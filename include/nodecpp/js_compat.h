/* -------------------------------------------------------------------------------
* Copyright (c) 2020, OLogN Technologies AG
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of the OLogN Technologies AG nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL OLogN Technologies AG BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
* -------------------------------------------------------------------------------*/

#ifndef NODECPP_JS_COMPAT_H
#define NODECPP_JS_COMPAT_H

#include "common.h"
#include <typeinfo>
#include <typeindex>

namespace nodecpp::js {

	class JSVar;
	class JSObject;
	class Value;
	class JSArray;
	class JSIndexRet;

	class JSOwnObj
	{
		friend class JSVar;

		using owningptr2jsobj = nodecpp::safememory::owning_ptr<JSObject>;
		using owningptr2jsarr = nodecpp::safememory::owning_ptr<JSArray>;
		owningptr2jsobj ptr;

	public:
		JSOwnObj() {}
		JSOwnObj( const JSOwnObj& other ) = delete;
		JSOwnObj( JSOwnObj&& other ) { ptr = std::move( other.ptr ); }
		JSOwnObj( owningptr2jsobj&& ptr_ ) { ptr = std::move( ptr_ ); }
		JSOwnObj( owningptr2jsarr&& ptr_ ) { owningptr2jsobj tmp(std::move( ptr_ )); ptr = std::move( tmp ); }

		~JSOwnObj() {}

		JSOwnObj& operator = ( const JSOwnObj& other ) = delete;
		JSOwnObj& operator = ( JSOwnObj&& other ) { if ( this == &other ) return *this ; ptr = std::move( other.ptr ); return *this; }
		JSOwnObj& operator = ( owningptr2jsobj&& ptr_ ) { ptr = std::move( ptr_ ); return *this; }
		JSOwnObj& operator = ( owningptr2jsarr&& ptr_ ) { ptr = std::move( ptr_ ); return *this; }

		JSIndexRet operator [] ( const JSVar& var );
		JSIndexRet operator [] ( double idx );
		JSIndexRet operator [] ( int idx );
		JSIndexRet operator [] ( const nodecpp::string& key );
		JSIndexRet operator [] ( const char* key );

		nodecpp::string toString() const;
		bool operator !() const { return ptr == nullptr; }
		operator nodecpp::string () const { return toString(); }

		bool has( const JSOwnObj& other ) const;
		bool has( size_t idx ) const;
		bool has( double num ) const;
		bool has( nodecpp::string str ) const;

		bool in( const JSOwnObj& collection ) const { return collection.has( *this ); }
	
		void forEach( std::function<void(nodecpp::string)> cb );
	};

	inline
	nodecpp::string typeOf( const JSOwnObj& obj )
	{
		return "object";
	}

	class JSVarBase
	{
	protected:
		friend class JSVar;
		friend class JSObject;
		friend class JSArray;
		friend nodecpp::string typeOf( const JSVarBase& );

		enum Type { undef, boolean, num, string, softptr, fn/*, fn0, fn1, fn2, fn3, fn4*/ };
		Type type = Type::undef;
		static constexpr size_t memsz = sizeof( nodecpp::string ) > 16 ? sizeof( nodecpp::string ) : 16;
		uintptr_t basemem[memsz/sizeof(uintptr_t)]; // note: we just cause it to be uintptr_t-aligned

		static constexpr size_t fnSize = sizeof( std::function<double(double)> );
		static_assert( fnSize == 64 );

		using softptr2jsobj = nodecpp::safememory::soft_ptr<JSObject>;
		using softptr2jsarr = nodecpp::safememory::soft_ptr<JSArray>;

		bool* _asBool() { return reinterpret_cast<bool*>( basemem ); }
		double* _asNum() { return reinterpret_cast<double*>( basemem ); }
		nodecpp::string* _asStr() { return reinterpret_cast<nodecpp::string*>( basemem ); }
		softptr2jsobj* _asSoft() { return reinterpret_cast<softptr2jsobj*>( basemem ); }

		const bool* _asBool() const { return reinterpret_cast<const bool*>( basemem ); }
		const double* _asNum() const { return reinterpret_cast<const double*>( basemem ); }
		const nodecpp::string* _asStr() const { return reinterpret_cast<const nodecpp::string*>( basemem ); }
		const softptr2jsobj* _asSoft() const { return reinterpret_cast<const softptr2jsobj*>( basemem ); }

		void deinit()
		{
			switch ( type )
			{
				case Type::undef:
				case Type::boolean:
				case Type::num:
					break;
				case Type::string:
					_asStr()->~basic_string();
					break;
				case Type::softptr:
					_asSoft()->~softptr2jsobj();
					break;
				default:
					NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
			}
			type = Type::undef;
		}
		void init( bool b )
		{
			if ( type == Type::boolean )
				*_asBool() = b;
			else
			{
				deinit();
				type = Type::boolean;
				*_asBool() = b;
			}
		}
		void init( double d )
		{
			if ( type == Type::num )
				*_asNum() = d;
			else
			{
				deinit();
				type = Type::num;
				*_asNum() = d;
			}
		}
		void init( const nodecpp::string& str )
		{
			if ( type == Type::string )
				*_asStr() = str;
			else
			{
				deinit();
				type = Type::string;
				new(_asStr())nodecpp::string( str );
			}
		}
		void init( const softptr2jsobj ptr )
		{
			if ( type == Type::softptr )
				*_asSoft() = ptr;
			else
			{
				deinit();
				type = Type::softptr;
				new(_asSoft())softptr2jsobj( ptr );
			}
		}
		void init( const softptr2jsarr ptr )
		{
			if ( type == Type::softptr )
				*_asSoft() = ptr;
			else
			{
				deinit();
				type = Type::softptr;
				new(_asSoft())softptr2jsarr( ptr );
			}
		}
		void init( const JSVarBase& other )
		{
			if ( type == other.type )
			{
				switch ( other.type )
				{
					case Type::undef:
						break;
					case Type::boolean:
						*_asBool() = *(other._asBool());
						break;
					case Type::num:
						*_asNum() = *(other._asNum());
						break;
					case Type::string:
						*_asStr() = *(other._asStr());
						break;
					case Type::softptr:
						*_asSoft() = *(other._asSoft());
						break;
					default:
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
				}
			}
			else
			{
				deinit();
				type = other.type;
				switch ( other.type )
				{
					case Type::undef:
						break;
					case Type::boolean:
						*_asBool() = *(other._asBool());
						break;
					case Type::num:
						*_asNum() = *(other._asNum());
						break;
					case Type::string:
						new(_asStr())nodecpp::string( *(other._asStr()) );
						break;
					case Type::softptr:
						new(_asSoft())softptr2jsobj( *(other._asSoft()) );
						break;
					default:
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
				}
			}
		}

	};

	class JSIndexRet
	{
		friend class JSVar;
		friend class JSObject;
		friend class JSArray;

		enum Type { undef, value, var };
		Type type = Type::undef;
		using OwnedT = Value*;
		static constexpr size_t memsz = sizeof( OwnedT ) > sizeof( JSVarBase ) ? sizeof( OwnedT ) : sizeof( JSVarBase );
		uintptr_t basemem[ memsz / sizeof( uintptr_t ) ];
		JSVar& _asVar() { return *reinterpret_cast<JSVar*>( basemem ); }
		OwnedT& _asValue() { return *reinterpret_cast<OwnedT*>( basemem ); }
		const JSVar& _asVar() const { return *reinterpret_cast<const JSVar*>( basemem ); }
		const OwnedT& _asValue() const { return *reinterpret_cast<const OwnedT*>( basemem ); }

		JSIndexRet() {}
		JSIndexRet( Value& v ) {
			_asValue() = &v;
			type = Type::value;
		}
		JSIndexRet( const JSVar& var );
		JSIndexRet& operator = ( Value& obj );

	public:
		JSIndexRet( const JSIndexRet& other);
		JSIndexRet& operator = ( const JSIndexRet& other );
		JSIndexRet& operator = ( const JSVar& var );
//		JSIndexRet& operator = ( const JSOwnObj& obj );
		JSIndexRet& operator = ( JSOwnObj&& obj );

		JSIndexRet operator [] ( const JSVar& var );
		JSIndexRet operator [] ( double idx );
		JSIndexRet operator [] ( int idx );
		JSIndexRet operator [] ( const nodecpp::string& key );
		JSIndexRet operator [] ( const char* key );

		~JSIndexRet();

		bool operator !() const;
		operator nodecpp::string () const { return toString(); }
		operator JSVar () const;
		nodecpp::string toString() const;
	};

	class JSVar : protected JSVarBase
	{
		friend class JSObject;
		friend class JSArray;
		friend nodecpp::string typeOf( const JSVarBase& );

		void init( const JSVar& other ) { JSVarBase::init( other ); }

	public:
		JSVar() {}
		JSVar( const JSVar& other ) { init( other );}
		JSVar( const JSOwnObj& other ) { const nodecpp::js::JSVarBase::softptr2jsobj tmp = other.ptr; JSVarBase::init( tmp );}
		JSVar( bool b ) { JSVarBase::init( b ); }
		JSVar( double d ) { JSVarBase::init( d ); }
		JSVar( int n ) { JSVarBase::init( (double)n ); }
		JSVar( const nodecpp::string& str ) { JSVarBase::init( str ); }
		JSVar( const char* str ) { nodecpp::string str_( str ); JSVarBase::init( str_ ); }
		JSVar( const softptr2jsobj ptr ) { JSVarBase::init( ptr ); }
		JSVar( const softptr2jsarr ptr ) { JSVarBase::init( ptr ); }

		~JSVar() { deinit(); }

		JSVar& operator = ( const JSVar& other ) { init( other ); return *this; }
		JSVar& operator = ( const JSOwnObj& other ) { const nodecpp::js::JSVarBase::softptr2jsobj tmp = other.ptr; JSVarBase::init( tmp ); return *this; }
		JSVar& operator = ( bool b ) { JSVarBase::init( b ); return *this; }
		JSVar& operator = ( double d ) { JSVarBase::init( d ); return *this; }
		JSVar& operator = ( int n ) { JSVarBase::init( (double)n ); return *this; }
		JSVar& operator = ( const nodecpp::string& str ) { JSVarBase::init( str ); return *this; }
		JSVar& operator = ( const char* str ) { nodecpp::string str_( str ); JSVarBase::init( str_ ); return * this; }
		JSVar& operator = ( softptr2jsobj ptr ) { JSVarBase::init( ptr ); return *this; }
		JSVar& operator = ( softptr2jsarr ptr ) { JSVarBase::init( ptr ); return *this; }

		JSIndexRet operator [] ( const JSVar& var );
		JSIndexRet operator [] ( double num );
		JSIndexRet operator [] ( int num );
		JSIndexRet operator [] ( const nodecpp::string& key );
		JSIndexRet operator [] ( const char* key ) { nodecpp::string s(key); return operator [] (s); }

		nodecpp::string toString() const;
		bool operator !() const
		{
			// TODO: make sure we report right values!!!
			switch ( type )
			{
				case Type::undef:
					return true; 
				case Type::boolean:
					return !*_asBool();
				case Type::num:
					return _asNum() == 0;
				case Type::string:
					return _asStr()->size() == 0 || *_asStr() == "false";
				case Type::softptr:
					return *_asSoft() != nullptr;
				default:
					NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
			}
		}
		operator nodecpp::string () const { return toString(); }

		bool has( const JSVar& other ) const;
		bool has( size_t idx ) const;
		bool has( double num ) const;
		bool has( nodecpp::string str ) const;

		bool in( const JSVar& collection ) const { return collection.has( *this ); }
		void forEach( std::function<void(nodecpp::string)> cb );
	};
	static_assert( sizeof(JSVarBase) == sizeof(JSVar), "no data memebers at JSVar itself!" );

	inline bool jsIn( const JSVar& var, const JSVar& collection )  { return collection.has( var ); }
	inline bool jsIn( size_t idx, const JSVar& collection )  { return collection.has( idx ); }
	inline bool jsIn( nodecpp::string str, const JSVar& collection )  { return collection.has( str ); }

	inline
	nodecpp::string typeOf( const JSVarBase& var )
	{
		switch ( var.type )
		{
			case JSVarBase::Type::undef:
				return "undefined";
			case JSVarBase::Type::boolean:
				return "boolean";
			case JSVarBase::Type::num:
				return "number";
			case JSVarBase::Type::string:
				return "string";
			case JSVarBase::Type::softptr:
				return "object";
			case JSVarBase::Type::fn:
				return "function";
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)(var.type) ); 
		}
	}

	class JSInit
	{
		friend class JSIndexRet;

		enum Type { undef, obj, var };
		Type type = Type::undef;
		using OwnedT = JSOwnObj;
		static constexpr size_t memsz = sizeof( OwnedT ) > sizeof( JSVar ) ? sizeof( OwnedT ) : sizeof( JSVar );
		uintptr_t basemem[ memsz / sizeof( uintptr_t ) ];
		JSVar& _asVar() { return *reinterpret_cast<JSVar*>( basemem ); }
		OwnedT& _asPtr() { return *reinterpret_cast<OwnedT*>( basemem ); }
		const JSVar& _asVar() const { return *reinterpret_cast<const JSVar*>( basemem ); }
		const OwnedT& _asPtr() const { return *reinterpret_cast<const OwnedT*>( basemem ); }
	public:
		JSInit() {}
		JSInit( const JSInit& other);
		JSInit( JSInit&& other);
		JSInit( JSOwnObj&& obj ) {
			new(&(_asPtr()))OwnedT( std::move( obj ) );
			type = Type::obj;
		}
		JSInit( const JSOwnObj& obj ) {
			new(&(_asVar()))JSVar( obj );
			type = Type::var;
		}
		JSInit( const JSVar& var ) {
			new(&(_asVar()))JSVar( var );
			type = Type::var;
		}
		JSInit( nodecpp::safememory::owning_ptr<JSObject>&& obj ) {
			new(&(_asPtr()))OwnedT( std::move( obj ) );
			type = Type::obj;
		}
		JSInit( nodecpp::safememory::owning_ptr<JSArray>&& arr ) {
			new(&(_asPtr()))OwnedT( std::move( arr ) );
			type = Type::obj;
		}
		JSInit( int num ) {
			new(&(_asVar()))JSVar( num );
			type = Type::var;
		}
		JSInit( const char* str ) {
			new(&(_asVar()))JSVar( str );
			type = Type::var;
		}
		JSInit& operator = ( const JSInit& other );
		JSInit& operator = ( JSInit&& other );
		/*JSInit& operator = ( JSOwnObj&& obj );
		JSInit& operator = ( const JSOwnObj& obj );
		JSInit& operator = ( const JSVar& var );
		JSInit& operator = ( int num ) { return operator = (JSVar( num ) ); }*/

		~JSInit() {
			if ( type == Type::var )
				_asVar().~JSVar();
			else if ( type == Type::obj )
				_asPtr().~JSOwnObj();
		}

		Value toValue() const;
	};

	class Value
	{
		friend class JSIndexRet;
		friend class JSInit;

		enum Type { undef, obj, var };
		Type type = Type::undef;
		using OwnedT = JSOwnObj;
		static constexpr size_t memsz = sizeof( OwnedT ) > sizeof( JSVar ) ? sizeof( OwnedT ) : sizeof( JSVar );
		uintptr_t basemem[ memsz / sizeof( uintptr_t ) ];
		JSVar& _asVar() { return *reinterpret_cast<JSVar*>( basemem ); }
		OwnedT& _asPtr() { return *reinterpret_cast<OwnedT*>( basemem ); }
		const JSVar& _asVar() const { return *reinterpret_cast<const JSVar*>( basemem ); }
		const OwnedT& _asPtr() const { return *reinterpret_cast<const OwnedT*>( basemem ); }
	public:
		Value() {}
		Value( Value&& other);
		Value( const Value& other) = delete;
		Value( JSOwnObj&& obj ) {
			new(&(_asPtr()))OwnedT( std::move( obj ) );
			type = Type::obj;
		}
		Value( const JSVar& var ) {
			new(&(_asVar()))JSVar( var );
			type = Type::var;
		}
		/*Value( nodecpp::safememory::owning_ptr<JSObject>&& obj ) {
			new(&(_asPtr()))OwnedT( std::move( obj ) );
			type = Type::obj;
		}
		Value( nodecpp::safememory::owning_ptr<JSArray>&& arr ) {
			new(&(_asPtr()))OwnedT( std::move( arr ) );
			type = Type::obj;
		}
		Value( int num ) {
			new(&(_asVar()))JSVar( num );
			type = Type::var;
		}
		Value( const char* str ) {
			new(&(_asVar()))JSVar( str );
			type = Type::var;
		}*/
//		Value( std::initializer_list<std::pair<nodecpp::string, int>> l );
		/*Value( std::initializer_list<int> l ) { 
			JSOwnObj tmp(l); 
			new(&(_asPtr()))OwnedT( std::move( tmp ) );
			type = Type::obj;
		}
		Value& operator = ( std::initializer_list<int> l ) { 
			JSOwnObj tmp(l); 
			new(&(_asPtr()))OwnedT( std::move( tmp ) );
			type = Type::obj;
			return *this;
		}
		Value( std::initializer_list<const char*> l ) { 
			JSOwnObj tmp(l); 
			new(&(_asPtr()))OwnedT( std::move( tmp ) );
			type = Type::obj;
		}
		Value& operator = ( std::initializer_list<const char*> l ) { 
			JSOwnObj tmp(l); 
			new(&(_asPtr()))OwnedT( std::move( tmp ) );
			type = Type::obj;
			return *this;
		}*/
		Value& operator = ( Value&& other );
		Value& operator = ( const Value& other ) = delete;
//		Value& operator = ( const JSOwnObj& obj );
//		Value& operator = ( const JSVar& var );
//		Value& operator = ( int num ) { return operator = (JSVar( num ) ); }

		~Value() {
			if ( type == Type::var )
				_asVar().~JSVar();
			else if ( type == Type::obj )
				_asPtr().~JSOwnObj();
		}

		bool operator !() const;
//		operator JSVar () const;
//		operator JSOwnObj () const;
		nodecpp::string toString() const;
	};

	class JSObject
	{
		friend class JSOwnObj;

	protected:
		nodecpp::map<nodecpp::string, Value> pairs;
	
		void toString_( nodecpp::string& ret, const nodecpp::string offset, const nodecpp::string separator ) const { 
			if ( pairs.size() == 0 )
				return;
			for ( auto& entry : pairs )
			{
				nodecpp::string entrystr = nodecpp::format( "{}{}: {}{}", offset, entry.first, entry.second.toString(), separator );
				ret += entrystr;
			}
			ret.erase( ret.size() - separator.size(), separator.size() );
		}

		JSIndexRet findOrAdd ( nodecpp::string s ) {
			auto f = pairs.find( s );
			if ( f != pairs.end() )
			{
				return f->second;
			}
			else
			{
				auto insret = pairs.insert( std::make_pair( s, Value() ) );
				return insret.first->second;
			}
		}

	public:
		JSObject() {}
		JSObject(std::initializer_list<std::pair<nodecpp::string, JSInit>> l)
		{
			for ( auto& p : l )
				pairs.insert( make_pair( p.first, std::move( p.second.toValue() ) ) );
		}
	public:
		virtual ~JSObject() {}
		virtual JSIndexRet operator [] ( const JSVar& var )
		{
			nodecpp::string s = var.toString(); // TODO: revise implementation!!!
			return findOrAdd( s );
		}
		virtual JSIndexRet operator [] ( size_t idx )
		{
			nodecpp::string s = nodecpp::format( "{}", idx );
			return findOrAdd( s );
		}
		virtual JSIndexRet operator [] ( const nodecpp::string& key )
		{
			return findOrAdd( key );
		}
		virtual JSIndexRet operator [] ( const nodecpp::string_literal& key ) {
			nodecpp::string s( key.c_str() );
			return findOrAdd( s );
		}
		virtual JSIndexRet operator [] ( const char* key ) {
			nodecpp::string s( key );
			return findOrAdd( s );
		}
		virtual nodecpp::string toString() const { 
			nodecpp::string ret = "{ \n";
			toString_( ret, "  ", ",\n" );
			ret += " }";
			return ret; 
		}
		virtual void forEach( std::function<void(nodecpp::string)> cb )
		{
			for ( auto& e: pairs )
				cb( e.first );
		}
		virtual bool has( const JSVar& var ) const
		{
			auto f = pairs.find( var.toString() );
			return f != pairs.end();
		}
		virtual bool has( size_t idx ) const
		{
			auto f = pairs.find( nodecpp::format( "{}", idx ) );
			return f != pairs.end();
		}
		virtual bool has( double num ) const
		{
			auto f = pairs.find( nodecpp::format( "{}", num ) );
			return f != pairs.end();
		}
		virtual bool has( nodecpp::string str ) const
		{
			auto f = pairs.find( str );
			return f != pairs.end();
		}
	};
	inline owning_ptr<JSObject> makeJSObject() { return make_owning<JSObject>(); }
	inline  owning_ptr<JSObject> makeJSObject(std::initializer_list<std::pair<nodecpp::string, JSInit>> l) { return make_owning<JSObject>(l); }

	class JSArray : public JSObject
	{
		friend class JSOwnObj;

		nodecpp::vector<Value> elems;
	public:
		JSArray() {}
		JSArray(std::initializer_list<JSInit> l)
		{
			for ( auto& p : l )
				elems.push_back( p.toValue() );
		}
	public:
		virtual JSIndexRet operator [] ( const JSVar& var )
		{
			// TODO: revise implementation!!!
			if ( var.type == JSVarBase::Type::num )
				return operator [] ( *(var._asNum()) );
			nodecpp::string s = var.toString(); 
			return findOrAdd( s );
		}
		virtual JSIndexRet operator [] ( size_t idx )
		{
			if ( idx < elems.size() )
				return elems[ idx ];
			else
			{
				elems.reserve( idx + 1 );
				for ( size_t i=elems.size(); i<elems.capacity(); ++i )
					elems.push_back( Value() );
				return elems[ idx ];
			}
		}
		virtual JSIndexRet operator [] ( const nodecpp::string& strIdx )
		{
			if ( strIdx.size() && strIdx[0] >= '0' && strIdx[0] <= '9' )
			{
				char* end;
				size_t idx = strtol( strIdx.c_str(), &end, 10 );
				if ( end - strIdx.c_str() == strIdx.size() )
					return operator [] ( idx );
			}
			return JSObject::operator[](strIdx);
		}
		virtual nodecpp::string toString() const { 
			nodecpp::string ret = "[ ";
			if ( pairs.size() )
			{
				toString_( ret, "", ", " );
				for ( auto& x : elems )
					ret += nodecpp::format( ", {}", x.toString() );
			}
			else if ( elems.size() )
			{
				for ( auto& x : elems )
					ret += nodecpp::format( "{}, ", x.toString() );
				ret.erase( ret.end() - 2 );
			}
			ret += " ]";
			return ret; 
		}
		virtual bool has( size_t idx ) const
		{
			auto f = pairs.find( nodecpp::format( "{}", idx ) );
			return f != pairs.end();
		}
	};
	inline owning_ptr<JSArray> makeJSArray() { return make_owning<JSArray>(); }
	inline owning_ptr<JSArray> makeJSArray(std::initializer_list<JSInit> l) { return make_owning<JSArray>(l); } // TODO: ownership of args

	////////////////////////////////////////////////////////////////////////////////////////////////


	class JSModule
	{
	public:
		virtual ~JSModule() {}
	};

	template<auto value>
	struct JSModule2JSVar : public JSModule {};

	template<typename Class, typename Result, Result Class::* member>
	struct JSModule2JSVar<member> : public Class
	{
		//JSVar& operator = ( soft_ptr<JSVar> other ) { (this->*member)->operator = ( other ); return (this->*member); } // TODO: ensure necessity and semantic
		JSVar& operator = ( bool b ) { (this->*member)->operator = ( b ); return (this->*member); }
		JSVar& operator = ( double d ) { (this->*member)->operator = ( d ); return (this->*member); }
		JSVar& operator = ( const nodecpp::string& str ) { (this->*member)->operator = ( str ); return (this->*member); }
		JSVar& operator = ( nodecpp::safememory::owning_ptr<JSObject>&& ptr ) { (this->*member)->operator = ( ptr ); return (this->*member); }
		JSVar& operator = ( nodecpp::safememory::soft_ptr<JSObject> ptr ) { (this->*member)->operator = ( ptr ); return (this->*member); }
		JSVar& operator = ( nodecpp::safememory::owning_ptr<JSArray>&& ptr ) { (this->*member)->operator = ( ptr ); return (this->*member); }
		JSVar& operator = ( nodecpp::safememory::soft_ptr<JSArray> ptr ) { (this->*member)->operator = ( ptr ); return (this->*member); }

		operator JSVar () { return (this->*member); }

		JSVar operator [] ( size_t idx ) { return (this->*member)->operator [] (idx ); }
		JSVar operator [] ( const nodecpp::string& key ) { return (this->*member)->operator [] (key ); }

		operator nodecpp::string () const  { return (this->*member)->operator nodecpp::string (); }
		bool operator !() const  { return (this->*member)->operator ! (); }

		bool has( const JSVar& other ) const { return (this->*member)->has(other ); }
		bool has( size_t idx ) const { return (this->*member)->has(idx ); }
		bool has( double num ) const { return (this->*member)->has(num ); }
		bool has( nodecpp::string str ) const { return (this->*member)->has(str ); }

		bool in( const JSVar& collection ) const { return (this->*member)->in(collection ); }

		nodecpp::string toString() const { return (this->*member)->toString();}
	};

	class JSModuleMap
	{
		using MapType = nodecpp::map<std::type_index, owning_ptr<JSModule>>;
#ifndef NODECPP_THREADLOCAL_INIT_BUG_GCC_60702
		MapType _classModuleMap;
		MapType& classModuleMap() { return _classModuleMap; }
#else
		uint8_t mapbytes[sizeof(MapType)];
		MapType& classModuleMap() { return *reinterpret_cast<MapType*>(mapbytes); }
#endif
		template<class UserClass>
		std::pair<bool, nodecpp::safememory::soft_ptr<UserClass>> getJsModuleExported_( std::type_index idx )
		{
			auto pattern = classModuleMap().find( idx );
			if ( pattern != classModuleMap().end() )
			{
				nodecpp::safememory::soft_ptr<JSModule> svar0 = pattern->second;
				nodecpp::safememory::soft_ptr<UserClass> svar = soft_ptr_static_cast<UserClass>(svar0);
				return std::make_pair( true, svar );
			}
			else
				return std::make_pair( false, nodecpp::safememory::soft_ptr<UserClass>() );
		}
		template<class UserClass>
		std::pair<bool, nodecpp::safememory::soft_ptr<UserClass>> addJsModuleExported_( std::type_index idx, nodecpp::safememory::owning_ptr<JSModule>&& pvar )
		{
			auto check = classModuleMap().insert( std::make_pair( idx, std::move( pvar ) ) );
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, check.second, "failed to insert exported value to map; insertion already done for this type" ); 
//			nodecpp::safememory::soft_ptr<UserClass> svar = check.first->second;
			nodecpp::safememory::soft_ptr<JSModule> svar0 = check.first->second;
			nodecpp::safememory::soft_ptr<UserClass> svar = soft_ptr_static_cast<UserClass>(svar0);
			return std::make_pair( true, svar );
		}
	public:
#ifdef NODECPP_THREADLOCAL_INIT_BUG_GCC_60702
		void init()
		{
			new(&(classModuleMap()))MapType();
		}
		void destroy()
		{
			classModuleMap().~MapType();
		}
#endif
		template<class UserClass>
		std::pair<bool, nodecpp::safememory::soft_ptr<UserClass>> getJsModuleExported()
		{
			return getJsModuleExported_<UserClass>( std::type_index(typeid(UserClass)) );
		}
		template<class UserClass>
		std::pair<bool, nodecpp::safememory::soft_ptr<UserClass>> addJsModuleExported( nodecpp::safememory::owning_ptr<JSModule>&& pvar )
		{
			return addJsModuleExported_<UserClass>( std::type_index(typeid(UserClass)), std::move( pvar ) );
		}
	};
	extern thread_local JSModuleMap jsModuleMap;

	template<class T>
	T& require()
	{
		auto trial = jsModuleMap.getJsModuleExported<T>();
		if ( trial.first )
			return *(trial.second);
		owning_ptr<JSModule> pt = nodecpp::safememory::make_owning<T>();
		auto ret = jsModuleMap.addJsModuleExported<T>( std::move( pt ) );
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, ret.first ); 
		return *(ret.second);
	}

} //namespace nodecpp::js

#endif // NODECPP_JS_COMPAT_H

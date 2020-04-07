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
//		JSOwnObj( JSOwnObj&& other ) { ptr = std::move( other.ptr ); }
		JSOwnObj( const JSOwnObj& other ) { ptr = std::move( *const_cast<owningptr2jsobj*>( &(other.ptr) ) ); }
		JSOwnObj( owningptr2jsobj ptr_ ) { ptr = std::move( ptr_ ); }
//		JSOwnObj( const owningptr2jsarr ptr ) { ptr = std::move( ptr ); }
		JSOwnObj( std::initializer_list<double> l ) { ptr = make_owning<JSArray>(l); }
		JSOwnObj( std::initializer_list<int> l ) { ptr = make_owning<JSArray>(l); }

		~JSOwnObj() {}

//		JSOwnObj& operator = ( JSOwnObj&& other ) { if ( this == &other ) return *this; ptr = std::move( other.ptr ); return *this; }
		JSOwnObj& operator = ( const JSOwnObj& other ) { if ( this == &other ) return *this ; ptr = std::move( *const_cast<owningptr2jsobj*>( &(other.ptr) ) ); return *this; }
		JSOwnObj& operator = ( owningptr2jsobj ptr ) { ptr = std::move( ptr ); return *this; }
//		JSOwnObj& operator = ( owningptr2jsarr ptr ) { ptr = std::move( ptr ); return *this; }
		JSOwnObj& operator = ( std::initializer_list<double> l ) { ptr = make_owning<JSArray>(l); return *this; }
		JSOwnObj& operator = ( std::initializer_list<int> l ) { ptr = make_owning<JSArray>(l); return *this; }

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

		enum Type { undef, boolean, num, string, softptr, fn };
		Type type = Type::undef;
		static constexpr size_t memsz = sizeof( nodecpp::string ) > 16 ? sizeof( nodecpp::string ) : 16;
		uintptr_t basemem[memsz/sizeof(uintptr_t)]; // note: we just cause it to be uintptr_t-aligned

		struct FunctionBase
		{
		};

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
		JSVar( const softptr2jsobj ptr ) { JSVarBase::init( ptr ); }
		JSVar( const softptr2jsarr ptr ) { JSVarBase::init( ptr ); }
//		JSVar( std::initializer_list<double> l ) { auto arr = make_owning<JSArray>(l); JSVarBase::init( std::move( arr ) ); }
//		JSVar( std::initializer_list<int> l ) { auto arr = make_owning<JSArray>(l); JSVarBase::init( std::move( arr ) ); }

		~JSVar() { deinit(); }

		JSVar& operator = ( const JSVar& other ) { init( other ); return *this; }
		JSVar& operator = ( const JSOwnObj& other ) { const nodecpp::js::JSVarBase::softptr2jsobj tmp = other.ptr; JSVarBase::init( tmp ); return *this; }
		JSVar& operator = ( bool b ) { JSVarBase::init( b ); return *this; }
		JSVar& operator = ( double d ) { JSVarBase::init( d ); return *this; }
		JSVar& operator = ( const nodecpp::string& str ) { JSVarBase::init( str ); return *this; }
		JSVar& operator = ( softptr2jsobj ptr ) { JSVarBase::init( ptr ); return *this; }
		JSVar& operator = ( softptr2jsarr ptr ) { JSVarBase::init( ptr ); return *this; }

		JSVar operator [] ( const JSVar& var );
		JSVar operator [] ( double num );
		JSVar operator [] ( int num );
		JSVar operator [] ( const nodecpp::string& key );
		JSVar operator [] ( const char* key ) { nodecpp::string s(key); return operator [] (s); }

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

	class Value
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
		Value();
		Value( const Value& other);
		Value( JSOwnObj&& obj ) {
			new(&(_asPtr()))OwnedT( std::move( obj ) );
			type = Type::obj;
		}
		Value( const JSVar& var ) {
			new(&(_asVar()))JSVar( var );
			type = Type::var;
		}
		Value& operator = ( const Value& other );

		Value& operator = ( JSOwnObj&& obj );
		Value& operator = ( const JSVar& var );
		operator JSVar () const;
		nodecpp::string toString() const;
	};

	class JSIndexRet
	{
		enum Type { undef, value, var };
		Type type = Type::undef;
		using OwnedT = Value*;
		static constexpr size_t memsz = sizeof( OwnedT ) > sizeof( JSVar ) ? sizeof( OwnedT ) : sizeof( JSVar );
		uintptr_t basemem[ memsz / sizeof( uintptr_t ) ];
		JSVar& _asVar() { return *reinterpret_cast<JSVar*>( basemem ); }
		OwnedT& _asValue() { return *reinterpret_cast<OwnedT*>( basemem ); }
		const JSVar& _asVar() const { return *reinterpret_cast<const JSVar*>( basemem ); }
		const OwnedT& _asValue() const { return *reinterpret_cast<const OwnedT*>( basemem ); }
	public:
		JSIndexRet( const JSIndexRet& other);
		JSIndexRet( Value& v ) {
			_asValue() = &v;
			type = Type::value;
		}
		JSIndexRet( const JSVar& var ) {
			new(&(_asVar()))JSVar( var );
			type = Type::var;
		}
		JSIndexRet& operator = ( const JSIndexRet& other );
		JSIndexRet& operator = ( Value& obj );
		JSIndexRet& operator = ( const JSVar& var );

		JSIndexRet operator [] ( const JSVar& var );
		JSIndexRet operator [] ( double idx );
		JSIndexRet operator [] ( int idx );
		JSIndexRet operator [] ( const nodecpp::string& key );
		JSIndexRet operator [] ( const char* key );

		operator JSVar () const;
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
				return f->second;
			else
			{
				auto insret = pairs.insert( std::make_pair( s, Value() ) );
				return insret.first->second;
			}
		}

	public:
		JSObject() {}
		JSObject(std::initializer_list<std::pair<nodecpp::string, JSVar>> l)
		{
			for ( auto& p : l )
				pairs.insert( p );
		}
		JSObject(std::initializer_list<std::pair<nodecpp::string, JSOwnObj>> l)
		{
			for ( auto& p : l )
				pairs.insert( std::move(*const_cast<std::pair<nodecpp::string, JSOwnObj>*>(&p)) );
		}
	public:
		virtual ~JSObject() {}
		static owning_ptr<JSObject> makeJSObject() { return make_owning<JSObject>(); }
//		static owning_ptr<JSObject> makeJSObject(std::initializer_list<std::pair<nodecpp::string, JSVar>> l) { return make_owning<JSObject>(l); }
		static owning_ptr<JSObject> makeJSObject(std::initializer_list<std::pair<nodecpp::string, JSOwnObj>> l) { return make_owning<JSObject>(l); }
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

	class JSArray : public JSObject
	{
		friend class JSOwnObj;

		nodecpp::vector<Value> elems;
	public:
		JSArray() {}
		JSArray(std::initializer_list<Value> l)
		{
			for ( auto& p : l )
				elems.push_back( p );
		}
		JSArray(std::initializer_list<double> l)
		{
			for ( auto& d : l )
				elems.push_back( JSVar( d ) );
		}
		JSArray(std::initializer_list<int> l)
		{
			for ( auto& d : l )
				elems.push_back( JSVar( d ) );
		}
		JSArray(std::initializer_list<nodecpp::string> l)
		{
			for ( auto& str : l )
				elems.push_back( Value(JSVar( str )) );
		}
	public:
		static owning_ptr<JSArray> makeJSArray() { return make_owning<JSArray>(); }
		static owning_ptr<JSArray> makeJSArray(std::initializer_list<Value> l) { return make_owning<JSArray>(l); } // TODO: ownership of args
		static owning_ptr<JSArray> makeJSArray(std::initializer_list<double> l) { return make_owning<JSArray>(l); }
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
					elems.push_back( JSVar() );
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

	////////////////////////////////////////////////////////////////////////////////////////////////
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

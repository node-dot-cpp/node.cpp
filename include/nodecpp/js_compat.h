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

namespace nodecpp::js {

	class JSObject;
	class JSArray;
	class JSVar
	{
		enum Type { undef, boolean, num, string, ownptr, softptr };
		Type type = Type::undef;
		static constexpr size_t memsz = sizeof( nodecpp::string ) > 16 ? sizeof( nodecpp::string ) : 16;
		uint8_t basemem[memsz];

		using owningptr2jsobj = nodecpp::safememory::owning_ptr<JSObject>;
		using softptr2jsobj = nodecpp::safememory::soft_ptr<JSObject>;

		bool* _asBool() { return reinterpret_cast<bool*>( basemem ); }
		double* _asNum() { return reinterpret_cast<double*>( basemem ); }
		nodecpp::string* _asStr() { return reinterpret_cast<nodecpp::string*>( basemem ); }
		owningptr2jsobj* _asOwn() { return reinterpret_cast<owningptr2jsobj*>( basemem ); }
		softptr2jsobj* _asSoft() { return reinterpret_cast<softptr2jsobj*>( basemem ); }

		void deinit()
		{
			switch ( type )
			{
				case Type::undef:
				case Type::boolean:
				case Type::num:
					break;
				case string:
					_asStr()->~basic_string();
				case ownptr:
					_asOwn()->~owningptr2jsobj();
				case softptr:
					_asSoft()->~softptr2jsobj();
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
		/*void init( int d )
		{
			if ( type == Type::num )
				*_asNum() = d;
			else
			{
				deinit();
				type = Type::num;
				*_asNum() = d;
			}
		}*/
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
		void init( owningptr2jsobj&& ptr )
		{
			if ( type == Type::ownptr )
				*_asOwn() = std::move( ptr );
			else
			{
				deinit();
				type = Type::ownptr;
				new(_asOwn())owningptr2jsobj( std::move( ptr ) );
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

	public:
		JSVar() {}
		JSVar( bool b ) { init( b ); }
		JSVar( double d ) { init( d ); }
		JSVar( const nodecpp::string& str ) { init( str ); }
		JSVar( owningptr2jsobj&& ptr ) { init( std::move( ptr ) ); }
//		JSVar( const softptr2jsobj ptr ) { init( ptr ); }

	public:
		static owning_ptr<JSVar> makeJSVar(bool b) { return make_owning<JSVar>(b); }
		static owning_ptr<JSVar> makeJSVar(double l) { return make_owning<JSVar>(l); }
		static owning_ptr<JSVar> makeJSVar(const nodecpp::string& str) { return make_owning<JSVar>(str); }
		static owning_ptr<JSVar> makeJSVar(owningptr2jsobj&& ptr) { return make_owning<JSVar>( std::move( ptr ) ); }
//		static owning_ptr<JSVar> makeJSVar(const softptr2jsobj ptr) { return make_owning<JSVar>(ptr); }
//		static owning_ptr<JSVar> makeJSVar(std::initializer_list<int> l) { return make_owning<JSVar>(l); }
		static owning_ptr<JSVar> makeJSVar(std::initializer_list<double> l) { auto arr = make_owning<JSArray>(l); return make_owning<JSVar>( std::move( arr ) ); }

		JSVar& operator = ( bool b ) { init( b ); }
		JSVar& operator = ( double d ) { init( d ); }
		JSVar& operator = ( const nodecpp::string& str ) { init( str ); }
		JSVar& operator = ( owningptr2jsobj&& ptr ) { init( std::move( ptr ) ); }
		JSVar& operator = ( softptr2jsobj ptr ) { init( ptr ); }

		nodecpp::string toString();
	};

	class JSObject
	{
	protected:
		nodecpp::map<nodecpp::string, owning_ptr<JSVar>> pairs;

		owning_ptr<JSVar> none;
	
		void toString_( nodecpp::string& ret, const nodecpp::string separator ) { 
			if ( pairs.size() == 0 )
				return;
			for ( auto& entry : pairs )
			{
				nodecpp::string entrystr = nodecpp::format( "  {}: {}{}", entry.first, entry.second->toString(), separator );
				ret += entrystr;
			}
			ret.erase( ret.size() - separator.size(), separator.size() );
		}

	public:
		JSObject() {}
		JSObject(std::initializer_list<std::pair<nodecpp::string, owning_ptr<JSVar>>> l)
		{
			for ( auto& p : l )
				pairs.insert( std::move(*const_cast<std::pair<nodecpp::string, owning_ptr<JSVar>>*>(&p)) );
		}
	public:
		virtual ~JSObject() {}
		static owning_ptr<JSObject> makeJSObject() { return make_owning<JSObject>(); }
		static owning_ptr<JSObject> makeJSObject(std::initializer_list<std::pair<nodecpp::string, owning_ptr<JSVar>>> l) { return make_owning<JSObject>(l); }
		soft_ptr<JSVar> operator [] ( const nodecpp::string& key )
		{
			auto f = pairs.find( key );
			if ( f != pairs.end() )
				return f->second;
			return none;
		}
		soft_ptr<JSVar> operator [] ( const nodecpp::string_literal& key ) {
			nodecpp::string s( key.c_str() );
			auto f = pairs.find( s );
			if ( f != pairs.end() )
				return f->second;
			return none;
		}
		soft_ptr<JSVar> operator [] ( const char* key ) {
			nodecpp::string s( key );
			auto f = pairs.find( s );
			if ( f != pairs.end() )
				return f->second;
			return none;
		}
		virtual nodecpp::string toString() { 
			nodecpp::string ret = "{ \n";
			toString_( ret, ",\n" );
			ret += "\n}";
			return ret; 
		}
	};

	class JSArray : public JSObject
	{
		nodecpp::vector<owning_ptr<JSVar>> elems;
	public:
		JSArray() {}
		JSArray(std::initializer_list<owning_ptr<JSVar>> l)
		{
			for ( auto& p : l )
				elems.push_back( std::move(*const_cast<owning_ptr<JSVar>*>(&p)) );
		}
		JSArray(std::initializer_list<double> l)
		{
			for ( auto& d : l )
				elems.push_back( std::move( JSVar::makeJSVar( d ) ) );
		}
	public:
		static owning_ptr<JSArray> makeJSArray() { return make_owning<JSArray>(); }
		static owning_ptr<JSArray> makeJSArray(std::initializer_list<owning_ptr<JSVar>> l) { return make_owning<JSArray>(l); }
		static owning_ptr<JSArray> makeJSArray(std::initializer_list<double> l) { return make_owning<JSArray>(l); }
		soft_ptr<JSVar> operator [] ( size_t idx )
		{
			if ( idx < elems.size() )
				return elems[ idx ];
			return none;
		}
		virtual nodecpp::string toString() { 
			nodecpp::string ret = "[ ";
			if ( pairs.size() )
			{
				toString_( ret, ", " );
				for ( auto& x : elems )
					ret += nodecpp::format( ", {}", x->toString() );
			}
			else if ( elems.size() )
			{
				for ( auto& x : elems )
					ret += nodecpp::format( "{}, ", x->toString() );
				ret.erase( ret.end() - 2 );
			}
			ret += "]";
			return ret; 
		}
	};

	inline
	nodecpp::string JSVar::toString()
	{
		switch ( type )
		{
			case Type::undef:
				return nodecpp::string( "undefined" );
			case Type::boolean:
				return nodecpp::format( "{}", *_asBool() );
			case Type::num:
				return nodecpp::format( "{}", *_asNum() );
			case Type::string:
				return nodecpp::format( "{}", *_asStr() );
			case Type::ownptr:
				return (*_asOwn())->toString();
			case Type::softptr:
				return (*_asSoft())->toString();
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
				return nodecpp::string( "undefined" );
				break;
		}
	}

} //namespace nodecpp::js

#endif // NODECPP_JS_COMPAT_H

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
		void init( softptr2jsobj ptr )
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
		JSVar( softptr2jsobj ptr ) { init( ptr ); }

		JSVar& operator = ( bool b ) { init( b ); }
		JSVar& operator = ( double d ) { init( d ); }
		JSVar& operator = ( const nodecpp::string& str ) { init( str ); }
		JSVar& operator = ( owningptr2jsobj&& ptr ) { init( std::move( ptr ) ); }
		JSVar& operator = ( softptr2jsobj ptr ) { init( ptr ); }

		nodecpp::string toString();
	};

	class JSObject
	{
		nodecpp::map<nodecpp::string, owning_ptr<JSVar>> pairs;
		owning_ptr<JSVar> none;
	
	public:
		static owning_ptr<JSObject> create() { return make_owning<JSObject>(); }
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
		nodecpp::string toString() { 
			nodecpp::string ret = "{ ";
			for ( auto& entry : pairs )
			{
				nodecpp::string entrystr = nodecpp::format( "  {}: {}\n", entry.first, entry.second->toString() );
				ret += entrystr;
			}
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
				break;
		}
	}

} //namespace nodecpp::js

#endif // NODECPP_JS_COMPAT_H

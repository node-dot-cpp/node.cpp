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

#include "js_compat.h"

thread_local nodecpp::js::JSModuleMap nodecpp::js::jsModuleMap;

namespace nodecpp::js {

	////////////////////////////////////////////////////////////   JSOwnObj ////

	JSIndexRet JSOwnObj::operator [] ( const JSVar& var )
	{
		return ptr->operator[]( var );
	}

	JSIndexRet JSOwnObj::operator [] ( double d )
	{
		return ptr->operator[]( d );
	}

	JSIndexRet JSOwnObj::operator [] ( int idx )
	{
		return ptr->operator[]( idx );
	}

	JSIndexRet JSOwnObj::operator [] ( const nodecpp::string& key )
	{
		return ptr->operator[]( key );
	}

	JSIndexRet JSOwnObj::operator [] ( const char* key )
	{
		nodecpp::string s( key );
		return ptr->operator[]( s );
	}

	nodecpp::string JSOwnObj::toString() const
	{
		if ( ptr )
			return ptr->toString();
		else
			return nodecpp::string( "undefined" );
	}

	bool JSOwnObj::has( const JSOwnObj& other ) const
	{
		JSVar tmp = other;
		return ptr->has( tmp );
	}

	bool JSOwnObj::has( size_t idx ) const
	{
		return ptr->has( idx );
	}

	bool JSOwnObj::has( double num ) const
	{
		return ptr->has( num );
	}

	bool JSOwnObj::has( nodecpp::string str ) const
	{
		return ptr->has( str );
	}

	void JSOwnObj::forEach( std::function<void(nodecpp::string)> cb )
	{
		ptr->forEach( std::move( cb ) );
	}


	////////////////////////////////////////////////////////////   JSVar ////
	
	JSIndexRet JSVar::operator [] ( const JSVar& var )
	{
		switch ( type )
		{
			case Type::undef:
				throw; // cannot set property to "undefined"
			case Type::boolean:
			case Type::num:
				return JSVar();
			case Type::string:
			{
				if ( var.type == Type::num )
				{
					auto pstr = _asStr();
					size_t idx = *( var._asNum() );
					if ( idx < pstr->size() )
						return JSVar( nodecpp::string( pstr->substr( idx, 1 ) ) );
					else
						return JSVar(); // TODO: mignt be something else!!!
				}
				else if ( var.type == Type::string )
				{
					const nodecpp::string& str = *(var._asStr());
					if ( str.size() && str[0] >= '0' && str[0] <= '9' )
					{
						auto pstr = _asStr();
						char* end;
						size_t idx = strtol( str.c_str(), &end, 10 );
						if ( end - str.c_str() == str.size() )
							return JSVar( nodecpp::string( pstr->substr( idx, 1 ) ) );
						else
							return JSVar(); // TODO: mignt be something else!!!
					}
					else
						return JSVar(); // TODO: mignt be something else!!!
				}
				else
					return JSVar(); // TODO: mignt be something else!!!
			}
			case Type::softptr:
				return (*_asSoft())->operator[]( var );
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
					return JSVar(); // TODO: mignt be something else!!!
		}
	}

	JSIndexRet JSVar::operator [] ( int idx )
	{
		switch ( type )
		{
			case Type::undef:
				throw; // cannot set property to "undefined"
			case Type::boolean:
			case Type::num:
				return JSVar(); // TODO: mignt be something else!!!
			case Type::string:
			{
				auto pstr = _asStr();
				if ( idx < pstr->size() )
					return JSVar( nodecpp::string( pstr->substr( idx, 1 ) ) ); // TODO: ownership
				else
				return JSVar(); // TODO: mignt be something else!!!
			}
			case Type::softptr:
				return (*_asSoft())->operator[]( idx );
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
				return JSVar(); // TODO: mignt be something else!!!
		}
	}

	JSIndexRet JSVar::operator [] ( double idx )
	{
		switch ( type )
		{
			case Type::undef:
				throw; // cannot set property to "undefined"
			case Type::boolean:
			case Type::num:
				return JSVar(); // TODO: mignt be something else!!!
			case Type::string:
			{
				auto pstr = _asStr();
				if ( idx < pstr->size() )
					return JSVar( nodecpp::string( pstr->substr( idx, 1 ) ) ); // TODO: ownership
				else
				return JSVar(); // TODO: mignt be something else!!!
			}
			case Type::softptr:
				return (*_asSoft())->operator[]( idx );
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
				return JSVar(); // TODO: mignt be something else!!!
		}
	}

	JSIndexRet JSVar::operator [] ( const nodecpp::string& key )
	{
		switch ( type )
		{
			case Type::undef:
				throw; // cannot set property to "undefined"
			case Type::boolean:
			case Type::num:
				return JSVar(); // TODO: mignt be something else!!!
			case Type::string:
			{
				if ( key.size() && key[0] >= '0' && key[0] <= '9' )
				{
					auto pstr = _asStr();
					char* end;
					size_t idx = strtol( key.c_str(), &end, 10 );
					if ( end - key.c_str() == key.size() )
						return JSVar( nodecpp::string( pstr->substr( idx, 1 ) ) ); // TODO: ownership
				}
				return JSVar(); // TODO: mignt be something else!!!
			}
			case Type::softptr:
				return (*_asSoft())->operator[]( key );
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
				return JSVar(); // TODO: mignt be something else!!!
		}
	}

	nodecpp::string JSVar::toString() const
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
			case Type::softptr:
				return (*_asSoft())->toString();
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
				return nodecpp::string( "undefined" );
				break;
		}
	}

	bool JSVar::has( const JSVar& other ) const
	{
		switch ( type )
		{
			case Type::undef:
			case Type::boolean:
			case Type::num:
				return false;
			case Type::string:
				return false; // TODO: ensure we report a right value
			case Type::softptr:
				return (*_asSoft())->has( other );
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
				return false;
		}
	}

	bool JSVar::has( size_t idx ) const
	{
		switch ( type )
		{
			case Type::undef:
			case Type::boolean:
			case Type::num:
				return false;
			case Type::string:
				return false; // TODO: ensure we report a right value
			case Type::softptr:
				return (*_asSoft())->has( idx );
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
				return false;
		}
	}

	bool JSVar::has( double num ) const
	{
		switch ( type )
		{
			case Type::undef:
			case Type::boolean:
			case Type::num:
				return false;
			case Type::string:
				return false; // TODO: ensure we report a right value
			case Type::softptr:
				return (*_asSoft())->has( num );
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
				return false;
		}
	}

	bool JSVar::has( nodecpp::string str ) const
	{
		switch ( type )
		{
			case Type::undef:
			case Type::boolean:
			case Type::num:
				return false;
			case Type::string:
				return false; // TODO: ensure we report a right value
			case Type::softptr:
				return (*_asSoft())->has( str );
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
				return false;
		}
	}

	void JSVar::forEach( std::function<void(nodecpp::string)> cb )
	{
		switch ( type )
		{
			case Type::undef:
			case Type::boolean:
			case Type::num:
				return;
			case Type::string:
				return; // TODO: revise!
			case Type::softptr:
				return (*_asSoft())->forEach( std::move( cb ) );
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
				return;
		}
	}

	////////////////////////////////////////////////////////////   JSInit ////

	JSInit::JSInit( const JSInit& other) {
		type = other.type;
		switch ( other.type )
		{
			case Type::undef:
				break;
			case Type::var:
				new(&(_asVar()))JSVar( other._asVar() );
				break;
			case Type::obj:
				new(&(_asPtr()))OwnedT( other._asPtr() );
				break;
		}
	}

	JSInit& JSInit::operator = ( const JSOwnObj& obj ) {
		switch ( type )
		{
			case Type::undef:
				new(&(_asPtr()))OwnedT( obj );
				break;
			case Type::var:
				_asVar().~JSVar();
				new(&(_asPtr()))OwnedT( obj );
				break;
			case Type::obj:
				_asPtr() = std::move( obj );
				break;
		}
		type = Type::obj;
		return *this;
	}

	JSInit& JSInit::operator = ( const JSVar& var ) {
		switch ( type )
		{
			case Type::undef:
				new(&(_asVar()))JSVar( var );
				break;
			case Type::var:
				_asVar() = var;
				break;
			case Type::obj:
				_asPtr().~OwnedT();
				new(&(_asVar()))JSVar( var );
				break;
		}
		type = Type::var;
		return *this;
	}

	JSInit& JSInit::operator = ( const JSInit& other ) {
		switch ( type )
		{
			case Type::undef:
				switch( other.type )
				{
					case Type::undef:
						break;
					case Type::var:
						new(&(_asVar()))JSVar( other._asVar() );
						break;
					case Type::obj:
						new(&(_asPtr()))OwnedT( other._asPtr() );
						break;
				}
				break;
			case Type::var:
				switch( other.type )
				{
					case Type::undef:
						_asVar().~JSVar();
						break;
					case Type::var:
						_asVar() = other._asVar();
						break;
					case Type::obj:
						_asVar().~JSVar();
						new(&(_asPtr()))OwnedT( other._asPtr() );
						break;
				}
				break;
			case Type::obj:
				switch( other.type )
				{
					case Type::undef:
						_asPtr().~OwnedT();
						break;
					case Type::var:
						_asPtr().~OwnedT();
						new(&(_asVar()))JSVar( other._asVar() );
						break;
					case Type::obj:
						_asPtr() = other._asPtr();
						break;
				}
				break;
		}
		type = other.type;
		return *this;
	}

	Value JSInit::toValue() const
	{
		switch ( type )
		{
			case Type::undef:
				return Value();
			case Type::var:
				return Value(_asVar());
			case Type::obj:
				return Value( std::move(_asPtr()));
		}
	}


	////////////////////////////////////////////////////////////   Value ////

	Value::Value() {
//		new(&(_asPtr()))OwnedT( JSObject::makeJSObject() );
		type = Type::undef;
	}

	/*Value::Value( std::initializer_list<std::pair<nodecpp::string, int>> l ) {
		new(&(_asPtr()))OwnedT( JSOwnObj( std::move( JSObject::makeJSObject( l ) ) ) );
		type = Type::obj;
	}*/

	Value::Value( const Value& other) {
		type = other.type;
		switch ( other.type )
		{
			case Type::undef:
				break;
			case Type::var:
				new(&(_asVar()))JSVar( other._asVar() );
				break;
			case Type::obj:
				new(&(_asPtr()))OwnedT( other._asPtr() );
				break;
		}
	}

	Value& Value::operator = ( const JSOwnObj& obj ) {
		switch ( type )
		{
			case Type::undef:
				new(&(_asPtr()))OwnedT( obj );
				break;
			case Type::var:
				_asVar().~JSVar();
				new(&(_asPtr()))OwnedT( obj );
				break;
			case Type::obj:
				_asPtr() = std::move( obj );
				break;
		}
		type = Type::obj;
		return *this;
	}

	Value& Value::operator = ( const JSVar& var ) {
		switch ( type )
		{
			case Type::undef:
				new(&(_asVar()))JSVar( var );
				break;
			case Type::var:
				_asVar() = var;
				break;
			case Type::obj:
				_asPtr().~OwnedT();
				new(&(_asVar()))JSVar( var );
				break;
		}
		type = Type::var;
		return *this;
	}

	Value& Value::operator = ( const Value& other ) {
		switch ( type )
		{
			case Type::undef:
				switch( other.type )
				{
					case Type::undef:
						break;
					case Type::var:
						new(&(_asVar()))JSVar( other._asVar() );
						break;
					case Type::obj:
						new(&(_asPtr()))OwnedT( other._asPtr() );
						break;
				}
				break;
			case Type::var:
				switch( other.type )
				{
					case Type::undef:
						_asVar().~JSVar();
						break;
					case Type::var:
						_asVar() = other._asVar();
						break;
					case Type::obj:
						_asVar().~JSVar();
						new(&(_asPtr()))OwnedT( other._asPtr() );
						break;
				}
				break;
			case Type::obj:
				switch( other.type )
				{
					case Type::undef:
						_asPtr().~OwnedT();
						break;
					case Type::var:
						_asPtr().~OwnedT();
						new(&(_asVar()))JSVar( other._asVar() );
						break;
					case Type::obj:
						_asPtr() = other._asPtr();
						break;
				}
				break;
		}
		type = other.type;
		return *this;
	}

	Value::operator JSVar () const
	{
		switch ( type )
		{
			case Type::undef:
				return JSVar();
			case Type::var:
				return _asVar();
			case Type::obj:
				return JSVar( _asPtr() );
		}
	}

	/*Value::operator JSOwnObj () const
	{
		switch ( type )
		{
			case Type::undef:
				throw;
				return JSOwnObj();
			case Type::var:
				throw;
				return JSOwnObj();
			case Type::obj:
				return JSVar( _asPtr() );
		}
	}*/

	nodecpp::string Value::toString() const
	{
		switch ( type )
		{
			case Type::undef:
				return "undefined";
			case Type::var:
				return _asVar().toString();
			case Type::obj:
				return _asPtr().toString();
		}
	}

	////////////////////////////////////////////////////////////   JSIndexRet ////

	JSIndexRet::JSIndexRet( const JSIndexRet& other) { 
		type = other.type;
		switch( other.type )
		{
			case Type::undef:
				break;
			case Type::var:
				new(&(_asVar()))JSVar( other._asVar() );
				break;
			case Type::value:
				_asValue() = other._asValue();
				break;
		}
	}

	JSIndexRet::JSIndexRet( const JSVar& var ) {
		new(&(_asVar()))JSVar( var );
		type = Type::var;
	}

	JSIndexRet& JSIndexRet::operator = ( const JSIndexRet& other ) {
		switch ( type )
		{
			case Type::undef:
				switch( other.type )
				{
					case Type::undef:
						break;
					case Type::var:
						new(&(_asVar()))JSVar( other._asVar() );
						break;
					case Type::value:
						_asValue() = other._asValue();
						break;
				}
				break;
			case Type::var:
				switch( other.type )
				{
					case Type::undef:
						_asVar().~JSVar();
						break;
					case Type::var:
						_asVar() = other._asVar();
						break;
					case Type::value:
						_asVar().~JSVar();
						_asValue() = other._asValue();
						break;
				}
				break;
			case Type::value:
				switch( other.type )
				{
					case Type::undef:
						break;
					case Type::var:
						new(&(_asVar()))JSVar( other._asVar() );
						break;
					case Type::value:
						_asValue() = other._asValue();
						break;
				}
				break;
		}
		type = other.type;
		return *this;
	}

	JSIndexRet& JSIndexRet::operator = ( Value& obj ) {
		switch ( type )
		{
			case Type::undef:
				throw;
				_asValue() = &obj;
				break;
			case Type::var:
				throw;
				_asVar().~JSVar();
				_asValue() = &obj;
				break;
			case Type::value:
				*_asValue() = obj;
				break;
		}
		type = Type::value;
		return *this;
	}

	JSIndexRet& JSIndexRet::operator = ( const JSOwnObj& obj ) {
		switch ( type )
		{
			case Type::undef:
				throw;
				*(_asValue()) = obj;
				break;
			case Type::var:
				throw;
//				_asVar().~JSVar();
//				*(_asValue()) = obj;
				_asVar() = obj; // effectively storing soft_ptr to what's owned by obj
				break;
			case Type::value:
				*(_asValue()) = obj;
				break;
		}
//		type = Type::value;
		return *this;
	}

	JSIndexRet& JSIndexRet::operator = ( const JSVar& var ) {
		switch ( type )
		{
			case Type::undef:
				throw; // TODO: reconsider
//				new(&(_asVar()))JSVar( var );
//				type = Type::var;
				break;
			case Type::var:
				_asVar() = var;
				break;
			case Type::value:
//				new(&(_asVar()))JSVar( var );
//				type = Type::var;
				*(_asValue()) = var;
				break;
		}
		return *this;
	}

	JSIndexRet::~JSIndexRet() {
		if ( type == Type::var )
			_asVar().~JSVar();
	}

	JSIndexRet::operator JSVar () const
	{
		switch ( type )
		{
			case Type::undef:
				return JSVar();
			case Type::var:
				return _asVar();
			case Type::value:
			{
				auto& v = *(_asValue());
				switch ( v.type )
				{
					case Value::Type::undef:
						return JSVar();
					case Value::Type::var:
						return v._asVar();
					case Value::Type::obj:
						JSVar ret = v._asPtr();
						return ret;
				}
			}
		}
	}

	JSIndexRet JSIndexRet::operator [] ( const JSVar& var )
	{
		switch ( type )
		{
			case Type::undef:
				return JSVar();
			case Type::var:
				return _asVar().operator[]( var );
			case Type::value:
			{
				auto& v = *(_asValue());
				switch ( v.type )
				{
					case Value::Type::undef:
						return JSVar();
					case Value::Type::var:
						return v._asVar().operator[]( var );
					case Value::Type::obj:
						return v._asPtr().operator[]( var );
				}
			}
		}
	}

	JSIndexRet JSIndexRet::operator [] ( double d )
	{
		switch ( type )
		{
			case Type::undef:
				return JSVar();
			case Type::var:
				return _asVar().operator[]( d );
			case Type::value:
			{
				auto& v = *(_asValue());
				switch ( v.type )
				{
					case Value::Type::undef:
						return JSVar();
					case Value::Type::var:
						return v._asVar().operator[]( d );
					case Value::Type::obj:
						return v._asPtr().operator[]( d );
				}
			}
		}
	}

	JSIndexRet JSIndexRet::operator [] ( int idx )
	{
		switch ( type )
		{
			case Type::undef:
				return JSVar();
			case Type::var:
				return _asVar().operator[]( idx );
			case Type::value:
			{
				auto& v = *(_asValue());
				switch ( v.type )
				{
					case Value::Type::undef:
						return JSVar();
					case Value::Type::var:
						return v._asVar().operator[]( idx );
					case Value::Type::obj:
						return v._asPtr().operator[]( idx );
				}
			}
		}
	}

	JSIndexRet JSIndexRet::operator [] ( const nodecpp::string& key )
	{
		switch ( type )
		{
			case Type::undef:
				return JSVar();
			case Type::var:
				return _asVar().operator[]( key );
			case Type::value:
			{
				auto& v = *(_asValue());
				switch ( v.type )
				{
					case Value::Type::undef:
						return JSVar();
					case Value::Type::var:
						return v._asVar().operator[]( key );
					case Value::Type::obj:
						return v._asPtr().operator[]( key );
				}
			}
		}
	}

	JSIndexRet JSIndexRet::operator [] ( const char* key )
	{
		nodecpp::string s( key );
		return JSIndexRet::operator [] ( s );
	}

	nodecpp::string JSIndexRet::toString() const
	{
		switch ( type )
		{
			case Type::undef:
				return "undefined";
			case Type::var:
				return _asVar().toString();
			case Type::value:
				return _asValue()->toString();
		}
	}

} // namespace nodecpp::js
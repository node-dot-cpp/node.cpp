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

	JSRLValue JSOwnObj::operator [] ( const JSVar& var )
	{
		return ptr->operator[]( var );
	}

	JSRLValue JSOwnObj::operator [] ( double d )
	{
		return ptr->operator[]( d );
	}

	JSRLValue JSOwnObj::operator [] ( int idx )
	{
		return ptr->operator[]( idx );
	}

	JSRLValue JSOwnObj::operator [] ( const nodecpp::string& key )
	{
		return ptr->operator[]( key );
	}

	JSRLValue JSOwnObj::operator [] ( const char* key )
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
	
	JSRLValue JSVar::operator [] ( const JSVar& var )
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

	JSRLValue JSVar::operator [] ( int idx )
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

	JSRLValue JSVar::operator [] ( double idx )
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

	JSRLValue JSVar::operator [] ( const nodecpp::string& key )
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

	JSInit JSVar::operator()() { 
		if ( type == Type::fn0 ) 
			return (_asFn0()->fn)(); 
		else 
			throw;
	}

	JSInit JSVar::operator()( JSVarOrOwn obj ) { 
		if ( type == Type::fn1 ) 
			return (_asFn1()->fn)( std::move( obj ) ); 
		else 
			throw;
	}

	JSInit JSVar::operator()( JSVarOrOwn obj1, JSVarOrOwn obj2 ) { 
		if ( type == Type::fn2 ) 
			return (_asFn2()->fn)( std::move( obj1 ), std::move( obj2 ) ); 
		else 
			throw;
	}

	JSInit JSVar::operator()( JSVarOrOwn obj1, JSVarOrOwn obj2, JSVarOrOwn obj3 ) { 
		if ( type == Type::fn3 ) 
			return (_asFn3()->fn)(std::move( obj1 ), std::move( obj2 ),  std::move( obj3 ) ); 
		else 
			throw;
	}

	JSInit JSVar::operator()( JSVarOrOwn obj1, JSVarOrOwn obj2, JSVarOrOwn obj3, JSVarOrOwn obj4 ) { 
		if ( type == Type::fn4 ) 
			return (_asFn4()->fn)( std::move( obj1 ), std::move( obj2 ), std::move( obj3 ), std::move( obj4 )  ); 
		else 
			throw;
	}

	JSInit JSVar::operator()( JSVarOrOwn obj1, JSVarOrOwn obj2, JSVarOrOwn obj3, JSVarOrOwn obj4, JSVarOrOwn obj5 ) { 
		if ( type == Type::fn5 ) 
			return (_asFn5()->fn)( std::move( obj1 ), std::move( obj2 ), std::move( obj3 ), std::move( obj4 ), std::move( obj5 ) ); 
		else 
			throw;
	}

	JSInit JSVar::operator()( JSVarOrOwn obj1, JSVarOrOwn obj2, JSVarOrOwn obj3, JSVarOrOwn obj4, JSVarOrOwn obj5, JSVarOrOwn obj6 ) { 
		if ( type == Type::fn6 ) 
			return (_asFn6()->fn)( std::move( obj1 ), std::move( obj2 ), std::move( obj3 ), std::move( obj4 ), std::move( obj5 ), std::move( obj6 ) ); 
		else 
			throw;
	}

	JSInit JSVar::operator()( JSVarOrOwn obj1, JSVarOrOwn obj2, JSVarOrOwn obj3, JSVarOrOwn obj4, JSVarOrOwn obj5, JSVarOrOwn obj6, JSVarOrOwn obj7 ) { 
		if ( type == Type::fn7 ) 
			return (_asFn7()->fn)( std::move( obj1 ), std::move( obj2 ), std::move( obj3 ), std::move( obj4 ), std::move( obj5 ), std::move( obj6 ), std::move( obj7 ) ); 
		else 
			throw;
	}

	JSInit JSVar::operator()( JSVarOrOwn obj1, JSVarOrOwn obj2, JSVarOrOwn obj3, JSVarOrOwn obj4, JSVarOrOwn obj5, JSVarOrOwn obj6, JSVarOrOwn obj7, JSVarOrOwn obj8 ) { 
		if ( type == Type::fn8 ) 
			return (_asFn8()->fn)( std::move( obj1 ), std::move( obj2 ), std::move( obj3 ), std::move( obj4 ), std::move( obj5 ), std::move( obj6 ), std::move( obj7 ), std::move( obj8 ) ); 
		else 
			throw;
	}

	JSInit JSVar::operator()( JSVarOrOwn obj1, JSVarOrOwn obj2, JSVarOrOwn obj3, JSVarOrOwn obj4, JSVarOrOwn obj5, JSVarOrOwn obj6, JSVarOrOwn obj7, JSVarOrOwn obj8, JSVarOrOwn obj9 ) { 
		if ( type == Type::fn9 ) 
			return (_asFn9()->fn)( std::move( obj1 ), std::move( obj2 ), std::move( obj3 ), std::move( obj4 ), std::move( obj5 ), std::move( obj6 ), std::move( obj7 ), std::move( obj8 ), std::move( obj9 ) ); 
		else 
			throw;
	}

	JSInit JSVar::operator()( JSVarOrOwn obj1, JSVarOrOwn obj2, JSVarOrOwn obj3, JSVarOrOwn obj4, JSVarOrOwn obj5, JSVarOrOwn obj6, JSVarOrOwn obj7, JSVarOrOwn obj8, JSVarOrOwn obj9, JSVarOrOwn obj10 ) { 
		if ( type == Type::fn10 ) 
			return (_asFn10()->fn)( std::move( obj1 ), std::move( obj2 ), std::move( obj3 ), std::move( obj4 ), std::move( obj5 ), std::move( obj6 ), std::move( obj7 ), std::move( obj8 ), std::move( obj9 ), std::move( obj10 )  ); 
		else 
			throw;
	}

	bool JSVar::operator !() const
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
			case JSVarBase::Type::fn0:
			case JSVarBase::Type::fn1:
			case JSVarBase::Type::fn2:
			case JSVarBase::Type::fn3:
			case JSVarBase::Type::fn4:
			case JSVarBase::Type::fn5:
			case JSVarBase::Type::fn6:
			case JSVarBase::Type::fn7:
			case JSVarBase::Type::fn8:
			case JSVarBase::Type::fn9:
			case JSVarBase::Type::fn10:
				throw;
				return false;
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
				return false;
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
			{
				// TODO: think about a right way
				double d = *_asNum();
				if ( (double)((int64_t)d) == d )
					return nodecpp::format( "{}", (int64_t)d);
				else
					return nodecpp::format( "{}", d);
			}
			case Type::string:
				return nodecpp::format( "{}", *_asStr() );
			case Type::softptr:
				return (*_asSoft())->toString();
			case Type::fn0:
			case Type::fn1:
			case Type::fn2:
			case Type::fn3:
			case Type::fn4:
			case Type::fn5:
			case Type::fn6:
			case Type::fn7:
			case Type::fn8:
			case Type::fn9:
			case Type::fn10:
				return nodecpp::string( "function" );
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
			case Type::fn0:
			case Type::fn1:
			case Type::fn2:
			case Type::fn3:
			case Type::fn4:
			case Type::fn5:
			case Type::fn6:
			case Type::fn7:
			case Type::fn8:
			case Type::fn9:
			case Type::fn10:
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
			case Type::fn0:
			case Type::fn1:
			case Type::fn2:
			case Type::fn3:
			case Type::fn4:
			case Type::fn5:
			case Type::fn6:
			case Type::fn7:
			case Type::fn8:
			case Type::fn9:
			case Type::fn10:
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
			case Type::fn0:
			case Type::fn1:
			case Type::fn2:
			case Type::fn3:
			case Type::fn4:
			case Type::fn5:
			case Type::fn6:
			case Type::fn7:
			case Type::fn8:
			case Type::fn9:
			case Type::fn10:
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
			case Type::fn0:
			case Type::fn1:
			case Type::fn2:
			case Type::fn3:
			case Type::fn4:
			case Type::fn5:
			case Type::fn6:
			case Type::fn7:
			case Type::fn8:
			case Type::fn9:
			case Type::fn10:
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
			case Type::fn0:
			case Type::fn1:
			case Type::fn2:
			case Type::fn3:
			case Type::fn4:
			case Type::fn5:
			case Type::fn6:
			case Type::fn7:
			case Type::fn8:
			case Type::fn9:
			case Type::fn10:
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
				new(&(_asPtr()))OwnedT( std::move( *const_cast<OwnedT*>( &(other._asPtr()) ) ) );
				break;
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
		}
	}

	JSInit::JSInit( JSInit&& other) {
		type = other.type;
		switch ( other.type )
		{
			case Type::undef:
				break;
			case Type::var:
				new(&(_asVar()))JSVar( other._asVar() );
				break;
			case Type::obj:
				new(&(_asPtr()))OwnedT( std::move( other._asPtr()) );
				break;
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
		}
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
						new(&(_asPtr()))OwnedT( std::move( *const_cast<OwnedT*>( &(other._asPtr()) ) ) );
						break;
					default:
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
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
						new(&(_asPtr()))OwnedT( std::move( *const_cast<OwnedT*>( &(other._asPtr()) ) ) );
						break;
					default:
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
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
						_asPtr() = std::move( *const_cast<OwnedT*>( &(other._asPtr()) ) );
						break;
					default:
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
				}
				break;
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
		}
		type = other.type;
		return *this;
	}

	JSInit& JSInit::operator = ( JSInit&& other ) {
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
						new(&(_asPtr()))OwnedT( std::move( other._asPtr() ) );
						break;
					default:
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
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
						new(&(_asPtr()))OwnedT( std::move( other._asPtr() ) );
						break;
					default:
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
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
						_asPtr() = std::move( other._asPtr() );
						break;
					default:
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
				}
				break;
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
		}
		type = other.type;
		return *this;
	}

	/*JSInit& JSInit::operator = ( const JSOwnObj& obj ) {
		switch ( type )
		{
			case Type::undef:
				new(&(_asVar()))JSVar( obj );
				break;
			case Type::var:
				_asVar() = obj;
				break;
			case Type::obj:
				_asPtr().~OwnedT();
				new(&(_asVar()))JSVar( obj );
				break;
		}
		type = Type::var;
		return *this;
	}

	JSInit& JSInit::operator = ( JSOwnObj&& obj ) {
		switch ( type )
		{
			case Type::undef:
				new(&(_asPtr()))OwnedT( std::move( obj ) );
				break;
			case Type::var:
				_asVar().~JSVar();
				new(&(_asPtr()))OwnedT( std::move( obj ) );
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
	}*/

	JSVarOrOwn JSInit::toValue() const
	{
		switch ( type )
		{
			case Type::undef:
				return JSVarOrOwn();
			case Type::var:
				return JSVarOrOwn(_asVar() );
			case Type::obj:
				return JSVarOrOwn( std::move( *const_cast<OwnedT*>( &(_asPtr()) ) ) );
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
				return JSVarOrOwn();
		}
	}


	////////////////////////////////////////////////////////////   JSVarOrOwn ////

	JSVarOrOwn::JSVarOrOwn( JSVarOrOwn&& other) {
		type = other.type;
		switch ( other.type )
		{
			case Type::undef:
				break;
			case Type::var:
				new(&(_asVar()))JSVar( other._asVar() );
				break;
			case Type::obj:
				new(&(_asPtr()))OwnedT( std::move( other._asPtr() ) );
				break;
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
		}
	}

	JSVarOrOwn& JSVarOrOwn::operator = ( JSVarOrOwn&& other ) {
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
						new(&(_asPtr()))OwnedT( std::move( other._asPtr() ) );
						break;
					default:
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
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
						new(&(_asPtr()))OwnedT( std::move( other._asPtr() ) );
						break;
					default:
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
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
						_asPtr() = std::move( other._asPtr() );
						break;
					default:
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
				}
				break;
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
		}
		type = other.type;
		return *this;
	}

	/*JSVarOrOwn::operator JSVar () const
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

	JSVarOrOwn::operator JSOwnObj () const
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

	bool JSVarOrOwn::operator !() const
	{
		switch ( type )
		{
			case Type::undef:
				throw; // TODO: reconsider
				return true;
			case Type::var:
				return _asVar().operator!();
				break;
			case Type::obj:
				return _asPtr().operator !();
				break;
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
				return false;
		}
	}

	nodecpp::string JSVarOrOwn::toString() const
	{
		switch ( type )
		{
			case Type::undef:
				return "undefined";
			case Type::var:
				return _asVar().toString();
			case Type::obj:
				return _asPtr().toString();
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
				return "";
		}
	}

	////////////////////////////////////////////////////////////   JSRLValue ////

	JSRLValue::JSRLValue( const JSRLValue& other) { 
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
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
		}
	}

	JSRLValue::JSRLValue( const JSVar& var ) {
		new(&(_asVar()))JSVar( var );
		type = Type::var;
	}

	JSRLValue& JSRLValue::operator = ( const JSRLValue& other ) {
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
					default:
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
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
					default:
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
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
					default:
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
				}
				break;
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
		}
		type = other.type;
		return *this;
	}

	JSRLValue& JSRLValue::operator = ( JSVarOrOwn& obj ) {
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
				_asValue() = &obj;
				break;
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
		}
		type = Type::value;
		return *this;
	}

	JSRLValue& JSRLValue::operator = ( JSOwnObj&& obj ) {
		switch ( type )
		{
			case Type::undef:
				throw; // TODO: reconsider
				*(_asValue()) = std::move( obj );
				break;
			case Type::var:
				throw; // TODO: reconsider
				_asVar() = obj; // effectively storing soft_ptr to what's owned by obj
				break;
			case Type::value:
				*(_asValue()) = std::move( obj );
				break;
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
		}
//		type = Type::value;
		return *this;
	}

	JSRLValue& JSRLValue::operator = ( const JSVar& var ) {
		switch ( type )
		{
			case Type::undef:
				throw; // TODO: reconsider
				break;
			case Type::var:
				_asVar() = var;
				break;
			case Type::value:
				*(_asValue()) = var;
				break;
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
		}
		return *this;
	}

	JSRLValue::~JSRLValue() {
		if ( type == Type::var )
			_asVar().~JSVar();
	}

	JSInit JSRLValue::operator()() {
		switch ( type )
		{
			case Type::var: return _asVar()();
			case Type::value: if ( _asValue()->type == JSVarOrOwn::Type::var ) return _asValue()->_asVar()(); else throw;
			default: throw;
		}
	}
	JSInit JSRLValue::operator()( JSVarOrOwn obj1 ) {
		switch ( type )
		{
			case Type::var: 
				return _asVar()( std::move( obj1 ) );
			case Type::value: 
				if ( _asValue()->type == JSVarOrOwn::Type::var ) 
					return _asValue()->_asVar()( std::move( obj1 ) ); 
				else throw;
			default: throw;
		}
	}

	JSInit JSRLValue::operator()( JSVarOrOwn obj1, JSVarOrOwn obj2 ) {
		switch ( type )
		{
			case Type::var: 
				return _asVar()( std::move( obj1 ), std::move( obj2 ) );
			case Type::value: 
				if ( _asValue()->type == JSVarOrOwn::Type::var ) 
					return _asValue()->_asVar()( std::move( obj1 ), std::move( obj2 ) ); 
				else throw;
			default: throw;
		}
	}

	JSInit JSRLValue::operator()( JSVarOrOwn obj1, JSVarOrOwn obj2, JSVarOrOwn obj3 ) {
		switch ( type )
		{
			case Type::var: 
				return _asVar()( std::move( obj1 ), std::move( obj2 ), std::move( obj3 ) );
			case Type::value: 
				if ( _asValue()->type == JSVarOrOwn::Type::var ) 
					return _asValue()->_asVar()( std::move( obj1 ), std::move( obj2 ), std::move( obj3 ) ); 
				else throw;
			default: throw;
		}
	}

	JSInit JSRLValue::operator()( JSVarOrOwn obj1, JSVarOrOwn obj2, JSVarOrOwn obj3, JSVarOrOwn obj4 ) {
		switch ( type )
		{
			case Type::var: 
				return _asVar()( std::move( obj1 ), std::move( obj2 ), std::move( obj3 ), std::move( obj4 ) );
			case Type::value: 
				if ( _asValue()->type == JSVarOrOwn::Type::var ) 
					return _asValue()->_asVar()( std::move( obj1 ), std::move( obj2 ), std::move( obj3 ), std::move( obj4 ) ); 
				else throw;
			default: throw;
		}
	}

	JSInit JSRLValue::operator()( JSVarOrOwn obj1, JSVarOrOwn obj2, JSVarOrOwn obj3, JSVarOrOwn obj4, JSVarOrOwn obj5 ) {
		switch ( type )
		{
			case Type::var: 
				return _asVar()( std::move( obj1 ), std::move( obj2 ), std::move( obj3 ), std::move( obj4 ), std::move( obj5 ) );
			case Type::value: 
				if ( _asValue()->type == JSVarOrOwn::Type::var ) 
					return _asValue()->_asVar()( std::move( obj1 ), std::move( obj2 ), std::move( obj3 ), std::move( obj4 ), std::move( obj5 ) ); 
				else throw;
			default: throw;
		}
	}

	JSInit JSRLValue::operator()( JSVarOrOwn obj1, JSVarOrOwn obj2, JSVarOrOwn obj3, JSVarOrOwn obj4, JSVarOrOwn obj5, JSVarOrOwn obj6 ) {
		switch ( type )
		{
			case Type::var: 
				return _asVar()( std::move( obj1 ), std::move( obj2 ), std::move( obj3 ), std::move( obj4 ), std::move( obj5 ), std::move( obj6 ) );
			case Type::value: 
				if ( _asValue()->type == JSVarOrOwn::Type::var ) 
					return _asValue()->_asVar()( std::move( obj1 ), std::move( obj2 ), std::move( obj3 ), std::move( obj4 ), std::move( obj5 ), std::move( obj6 ) ); 
				else throw;
			default: throw;
		}
	}

	JSInit JSRLValue::operator()( JSVarOrOwn obj1, JSVarOrOwn obj2, JSVarOrOwn obj3, JSVarOrOwn obj4, JSVarOrOwn obj5, JSVarOrOwn obj6, JSVarOrOwn obj7 ) {
		switch ( type )
		{
			case Type::var: 
				return _asVar()( std::move( obj1 ), std::move( obj2 ), std::move( obj3 ), std::move( obj4 ), std::move( obj5 ), std::move( obj6 ), std::move( obj7 ) );
			case Type::value: 
				if ( _asValue()->type == JSVarOrOwn::Type::var ) 
					return _asValue()->_asVar()( std::move( obj1 ), std::move( obj2 ), std::move( obj3 ), std::move( obj4 ), std::move( obj5 ), std::move( obj6 ), std::move( obj7 ) ); 
				else throw;
			default: throw;
		}
	}

	JSInit JSRLValue::operator()( JSVarOrOwn obj1, JSVarOrOwn obj2, JSVarOrOwn obj3, JSVarOrOwn obj4, JSVarOrOwn obj5, JSVarOrOwn obj6, JSVarOrOwn obj7, JSVarOrOwn obj8 ) {
		switch ( type )
		{
			case Type::var: 
				return _asVar()( std::move( obj1 ), std::move( obj2 ), std::move( obj3 ), std::move( obj4 ), std::move( obj5 ), std::move( obj6 ), std::move( obj7 ), std::move( obj8 ) );
			case Type::value: 
				if ( _asValue()->type == JSVarOrOwn::Type::var ) 
					return _asValue()->_asVar()( std::move( obj1 ), std::move( obj2 ), std::move( obj3 ), std::move( obj4 ), std::move( obj5 ), std::move( obj6 ), std::move( obj7 ), std::move( obj8 ) ); 
				else throw;
			default: throw;
		}
	}

	JSInit JSRLValue::operator()( JSVarOrOwn obj1, JSVarOrOwn obj2, JSVarOrOwn obj3, JSVarOrOwn obj4, JSVarOrOwn obj5, JSVarOrOwn obj6, JSVarOrOwn obj7, JSVarOrOwn obj8, JSVarOrOwn obj9 ) {
		switch ( type )
		{
			case Type::var: 
				return _asVar()( std::move( obj1 ), std::move( obj2 ), std::move( obj3 ), std::move( obj4 ), std::move( obj5 ), std::move( obj6 ), std::move( obj7 ), std::move( obj8 ), std::move( obj9 ) );
			case Type::value: 
				if ( _asValue()->type == JSVarOrOwn::Type::var ) 
					return _asValue()->_asVar()( std::move( obj1 ), std::move( obj2 ), std::move( obj3 ), std::move( obj4 ), std::move( obj5 ), std::move( obj6 ), std::move( obj7 ), std::move( obj8 ), std::move( obj9 ) ); 
				else throw;
			default: throw;
		}
	}

	JSInit JSRLValue::operator()( JSVarOrOwn obj1, JSVarOrOwn obj2, JSVarOrOwn obj3, JSVarOrOwn obj4, JSVarOrOwn obj5, JSVarOrOwn obj6, JSVarOrOwn obj7, JSVarOrOwn obj8, JSVarOrOwn obj9, JSVarOrOwn obj10 ) {
		switch ( type )
		{
			case Type::var: 
				return _asVar()( std::move( obj1 ), std::move( obj2 ), std::move( obj3 ), std::move( obj4 ), std::move( obj5 ), std::move( obj6 ), std::move( obj7 ), std::move( obj8 ), std::move( obj9 ), std::move( obj10 ) );
			case Type::value: 
				if ( _asValue()->type == JSVarOrOwn::Type::var ) 
					return _asValue()->_asVar()( std::move( obj1 ), std::move( obj2 ), std::move( obj3 ), std::move( obj4 ), std::move( obj5 ), std::move( obj6 ), std::move( obj7 ), std::move( obj8 ), std::move( obj9 ), std::move( obj10 ) ); 
				else throw;
			default: throw;
		}
	}

	bool JSRLValue::operator !() const
	{
		switch ( type )
		{
			case Type::undef:
				throw; // TODO: reconsider
				return true;
			case Type::var:
				return _asVar().operator!();
			case Type::value:
				return _asValue()->operator !();
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
				return false;
		}
	}

	JSRLValue::operator JSVar () const
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
					case JSVarOrOwn::Type::undef:
						return JSVar();
					case JSVarOrOwn::Type::var:
						return v._asVar();
					case JSVarOrOwn::Type::obj:
					{
						JSVar ret = v._asPtr();
						return ret;
					}
					default:
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
						return JSVar();
				}
			}
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
				return JSVar();
		}
	}

	JSRLValue JSRLValue::operator [] ( const JSVar& var )
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
					case JSVarOrOwn::Type::undef:
						return JSVar();
					case JSVarOrOwn::Type::var:
						return v._asVar().operator[]( var );
					case JSVarOrOwn::Type::obj:
						return v._asPtr().operator[]( var );
					default:
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
						return JSVar();
				}
			}
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
				return JSVar();
		}
	}

	JSRLValue JSRLValue::operator [] ( double d )
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
					case JSVarOrOwn::Type::undef:
						return JSVar();
					case JSVarOrOwn::Type::var:
						return v._asVar().operator[]( d );
					case JSVarOrOwn::Type::obj:
						return v._asPtr().operator[]( d );
					default:
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
						return JSVar();
				}
			}
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
				return JSVar();
		}
	}

	JSRLValue JSRLValue::operator [] ( int idx )
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
					case JSVarOrOwn::Type::undef:
						return JSVar();
					case JSVarOrOwn::Type::var:
						return v._asVar().operator[]( idx );
					case JSVarOrOwn::Type::obj:
						return v._asPtr().operator[]( idx );
					default:
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
						return JSVar();
				}
			}
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
				return JSVar();
		}
	}

	JSRLValue JSRLValue::operator [] ( const nodecpp::string& key )
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
					case JSVarOrOwn::Type::undef:
						return JSVar();
					case JSVarOrOwn::Type::var:
						return v._asVar().operator[]( key );
					case JSVarOrOwn::Type::obj:
						return v._asPtr().operator[]( key );
					default:
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
						return JSVar();
				}
			}
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
				return JSVar();
		}
	}

	JSRLValue JSRLValue::operator [] ( const char* key )
	{
		nodecpp::string s( key );
		return JSRLValue::operator [] ( s );
	}

	nodecpp::string JSRLValue::toString() const
	{
		switch ( type )
		{
			case Type::undef:
				return "undefined";
			case Type::var:
				return _asVar().toString();
			case Type::value:
				return _asValue()->toString();
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
				return "";
		}
	}

} // namespace nodecpp::js
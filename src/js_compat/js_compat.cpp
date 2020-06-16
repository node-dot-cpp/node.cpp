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

#include <cmath>
#include <iterator>
#include <regex>
#include "../../include/nodecpp/js_compat.h"
#include "../../include/nodecpp/nls.h"

namespace nodecpp {

thread_local NLS threadLocalData;

} // namespace nodecpp


class JSStackMan // just a helper
{
	nodecpp::safememory::soft_ptr<nodecpp::js::JSArray> args;
public:
	JSStackMan( nodecpp::safememory::soft_ptr<nodecpp::js::JSArray> args_ ) {
		args = nodecpp::threadLocalData.currentArgs;
		nodecpp::threadLocalData.currentArgs = args_;
	}
	~JSStackMan() {nodecpp::threadLocalData.currentArgs = args;}
};


namespace nodecpp::js {

	JSConsole console;

	JSVar arguments() { return threadLocalData.currentArgs != nullptr ? threadLocalData.currentArgs : JSVar(); }

	////////////////////////////////////////////////////////////   JSOwnObj ////

	JSRLValue JSOwnObj::operator [] ( const JSVar& var ) const
	{
		return ptr->operator[]( var );
	}

	JSRLValue JSOwnObj::operator [] ( const JSRLValue& val ) const
	{
		return ptr->operator[]( val );
	}

	JSRLValue JSOwnObj::operator [] ( double d ) const
	{
		return ptr->operator[]( d );
	}

	JSRLValue JSOwnObj::operator [] ( int idx ) const
	{
		return ptr->operator[]( idx );
	}

	JSRLValue JSOwnObj::operator [] ( const nodecpp::string& key ) const
	{
		return ptr->operator[]( key );
	}

	JSRLValue JSOwnObj::operator [] ( const JSString& key ) const { return operator [] ( key.str() ); }
	JSRLValue JSOwnObj::operator [] ( const char8_t* key ) const { JSString s(key); return operator [] (s.str()); }
	JSRLValue JSOwnObj::operator [] ( const char* key ) const { nodecpp::string s(key); return operator [] (s); }

	nodecpp::string JSOwnObj::toString() const
	{
		if ( ptr )
			return ptr->toString();
		else
			return nodecpp::string( "undefined" );
	}

	double JSOwnObj::toNumber() const
	{
		if ( ptr )
			return ptr->toNumber();
		else
			return NAN;
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

	nodecpp::safememory::owning_ptr<JSArray> JSOwnObj::keys()
	{
		return ptr->keys();
	}

	double JSOwnObj::length() const { return ptr->length(); }
	void JSOwnObj::setLength( double ln ) { ptr->setLength( ln ); }


	////////////////////////////////////////////////////////////   JSRegExp ////

	JSRegExp::JSRegExp( JSVar regex_, nodecpp::string flags ) {
		if ( !(regex_.type == JSVarBase::Type::string || regex_.type == JSVarBase::Type::num || regex_.type == JSVarBase::Type::boolean ) )
			throw;
		regex = regex_.toString();
	}


	////////////////////////////////////////////////////////////   JSVar ////

	JSRLValue JSVar::operator [] ( const JSVar& var ) const
	{
		switch ( type )
		{
			case Type::undef:
				throw; // cannot set property to "undefined"
			case Type::boolean:
			case Type::num:
				return JSRLValue();
			case Type::string:
			{
				if ( var.type == Type::num )
				{
					auto pstr = _asStr();
					size_t idx = *( var._asNum() );
					if ( idx < pstr->size() )
						return JSRLValue::from( JSVar( JSString( pstr->charAt( idx ) ) ) );
					else
						return JSRLValue(); // TODO: mignt be something else!!!
				}
				else if ( var.type == Type::string )
				{
					const JSString& str = *(var._asStr());
					if ( str.size() && str.str()[0] >= '0' && str.str()[0] <= '9' )
					{
						auto pstr = _asStr();
						char* end;
						size_t idx = strtol( str.c_str(), &end, 10 );
						if ( end == str.c_str() + str.size() )
							return JSRLValue::from( JSVar( JSString( pstr->charAt( idx ) ) ) );
						else
							return JSRLValue(); // TODO: mignt be something else!!!
					}
					else
						return JSRLValue(); // TODO: mignt be something else!!!
				}
				else
					return JSRLValue(); // TODO: mignt be something else!!!
			}
			case Type::softptr:
				return (*_asSoft())->operator[]( var );
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
					return JSRLValue(); // TODO: mignt be something else!!!
		}
	}

	JSRLValue JSVar::operator [] ( const JSRLValue& val ) const
	{
		if ( val.type != JSRLValue::Type::var )
			return JSRLValue(); // TODO: check if we should throw or convert to string, etc, instead
		const JSVar& var = val._asVar();

		switch ( type )
		{
			case Type::undef:
				throw; // cannot set property to "undefined"
			case Type::boolean:
			case Type::num:
				return JSRLValue();
			case Type::string:
			{
				if ( var.type == Type::num )
				{
					auto pstr = _asStr();
					size_t idx = *( var._asNum() );
					if ( idx < pstr->size() )
						return JSRLValue::from( JSVar( JSString( pstr->charAt( idx ) ) ) );
					else
						return JSRLValue(); // TODO: mignt be something else!!!
				}
				else if ( var.type == Type::string )
				{
					const JSString& str = *(var._asStr());
					if ( str.size() && str.str()[0] >= '0' && str.str()[0] <= '9' )
					{
						auto pstr = _asStr();
						char* end;
						size_t idx = strtol( str.c_str(), &end, 10 );
						if ( end == str.c_str() + str.size() )
							return JSRLValue::from( JSVar( JSString( pstr->charAt( idx ) ) ) );
						else
							return JSRLValue(); // TODO: mignt be something else!!!
					}
					else
						return JSRLValue(); // TODO: mignt be something else!!!
				}
				else
					return JSRLValue(); // TODO: mignt be something else!!!
			}
			case Type::softptr:
				return (*_asSoft())->operator[]( var );
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
					return JSRLValue(); // TODO: mignt be something else!!!
		}
	}

	JSRLValue JSVar::operator [] ( int idx ) const
	{
		switch ( type )
		{
			case Type::undef:
				throw; // cannot set property to "undefined"
			case Type::boolean:
			case Type::num:
				return JSRLValue(); // TODO: mignt be something else!!!
			case Type::string:
			{
				auto pstr = _asStr();
				if ( (size_t)idx < pstr->size() )
					return JSRLValue::from( JSVar( JSString( pstr->charAt( idx ) ) ) ); // TODO: ownership
				else
				return JSRLValue(); // TODO: mignt be something else!!!
			}
			case Type::softptr:
				return (*_asSoft())->operator[]( idx );
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
				return JSRLValue(); // TODO: mignt be something else!!!
		}
	}

	JSRLValue JSVar::operator [] ( double idx ) const
	{
		switch ( type )
		{
			case Type::undef:
				throw; // cannot set property to "undefined"
			case Type::boolean:
			case Type::num:
				return JSRLValue(); // TODO: mignt be something else!!!
			case Type::string:
			{
				auto pstr = _asStr();
				if ( idx < pstr->size() )
					return JSRLValue::from( JSVar( JSString( pstr->charAt( idx ) ) ) );
				else
				return JSRLValue(); // TODO: mignt be something else!!!
			}
			case Type::softptr:
				return (*_asSoft())->operator[]( idx );
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
				return JSRLValue(); // formally
		}
	}

	JSRLValue JSVar::operator [] ( const nodecpp::string& key ) const
	{
		switch ( type )
		{
			case Type::undef:
				throw; // cannot set property to "undefined"
			case Type::boolean:
			case Type::num:
				return JSRLValue(); // TODO: mignt be something else!!!
			case Type::string:
			{
				if ( key.size() && key[0] >= '0' && key[0] <= '9' )
				{
					auto pstr = _asStr();
					char* end;
					size_t idx = strtol( key.c_str(), &end, 10 );
					if ( end == key.c_str() + key.size() )
						return JSRLValue::from( JSVar( JSString( pstr->charAt( idx ) ) ) );
				}
				return JSRLValue(); // TODO: mignt be something else!!!
			}
			case Type::softptr:
				return (*_asSoft())->operator[]( key );
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
				return JSRLValue(); // TODO: mignt be something else!!!
		}
	}

	JSVar JSVar::operator()() { 
		nodecpp::safememory::owning_ptr<JSArray> args = makeJSArray();
		JSStackMan restorer( args );
		switch ( type )
		{
			case Type::fn0: return (_asFn0()->fn)(); 
			case Type::fn1: return (_asFn1()->fn)(JSVar()); 
			case Type::fn2: return (_asFn2()->fn)(JSVar(), JSVar()); 
			case Type::fn3: return (_asFn3()->fn)(JSVar(), JSVar(), JSVar()); 
			case Type::fn4: return (_asFn4()->fn)(JSVar(), JSVar(), JSVar(), JSVar()); 
			case Type::fn5: return (_asFn5()->fn)(JSVar(), JSVar(), JSVar(), JSVar(), JSVar()); 
			case Type::fn6: return (_asFn6()->fn)(JSVar(), JSVar(), JSVar(), JSVar(), JSVar(), JSVar()); 
			case Type::fn7: return (_asFn7()->fn)(JSVar(), JSVar(), JSVar(), JSVar(), JSVar(), JSVar(), JSVar()); 
			case Type::fn8: return (_asFn8()->fn)(JSVar(), JSVar(), JSVar(), JSVar(), JSVar(), JSVar(), JSVar(), JSVar()); 
			case Type::fn9: return (_asFn9()->fn)(JSVar(), JSVar(), JSVar(), JSVar(), JSVar(), JSVar(), JSVar(), JSVar(), JSVar()); 
			case Type::fn10: return (_asFn10()->fn)(JSVar(), JSVar(), JSVar(), JSVar(), JSVar(), JSVar(), JSVar(), JSVar(), JSVar(), JSVar()); 
			default: throw; 
		}
	}

	JSVar JSVar::operator()( JSVar obj ) { 
		nodecpp::safememory::owning_ptr<JSArray> args = makeJSArray( { obj } );
		JSStackMan restorer( args );

		switch ( type )
		{
			case Type::fn0: return (_asFn0()->fn)(); 
			case Type::fn1: return (_asFn1()->fn)(obj); 
			case Type::fn2: return (_asFn2()->fn)(obj, JSVar()); 
			case Type::fn3: return (_asFn3()->fn)(obj, JSVar(), JSVar()); 
			case Type::fn4: return (_asFn4()->fn)(obj, JSVar(), JSVar(), JSVar()); 
			case Type::fn5: return (_asFn5()->fn)(obj, JSVar(), JSVar(), JSVar(), JSVar()); 
			case Type::fn6: return (_asFn6()->fn)(obj, JSVar(), JSVar(), JSVar(), JSVar(), JSVar()); 
			case Type::fn7: return (_asFn7()->fn)(obj, JSVar(), JSVar(), JSVar(), JSVar(), JSVar(), JSVar()); 
			case Type::fn8: return (_asFn8()->fn)(obj, JSVar(), JSVar(), JSVar(), JSVar(), JSVar(), JSVar(), JSVar()); 
			case Type::fn9: return (_asFn9()->fn)(obj, JSVar(), JSVar(), JSVar(), JSVar(), JSVar(), JSVar(), JSVar(), JSVar()); 
			case Type::fn10: return (_asFn10()->fn)(obj, JSVar(), JSVar(), JSVar(), JSVar(), JSVar(), JSVar(), JSVar(), JSVar(), JSVar()); 
			default: throw; 
		}
	}

	JSVar JSVar::operator()( JSVar obj1, JSVar obj2 ) { 
		nodecpp::safememory::owning_ptr<JSArray> args = makeJSArray( { obj1, obj2 } );
		JSStackMan restorer( args );

		switch ( type )
		{
			case Type::fn0: return (_asFn0()->fn)(); 
			case Type::fn1: return (_asFn1()->fn)(obj1); 
			case Type::fn2: return (_asFn2()->fn)(obj1, obj2); 
			case Type::fn3: return (_asFn3()->fn)(obj1, obj2, JSVar()); 
			case Type::fn4: return (_asFn4()->fn)(obj1, obj2, JSVar(), JSVar()); 
			case Type::fn5: return (_asFn5()->fn)(obj1, obj2, JSVar(), JSVar(), JSVar()); 
			case Type::fn6: return (_asFn6()->fn)(obj1, obj2, JSVar(), JSVar(), JSVar(), JSVar()); 
			case Type::fn7: return (_asFn7()->fn)(obj1, obj2, JSVar(), JSVar(), JSVar(), JSVar(), JSVar()); 
			case Type::fn8: return (_asFn8()->fn)(obj1, obj2, JSVar(), JSVar(), JSVar(), JSVar(), JSVar(), JSVar()); 
			case Type::fn9: return (_asFn9()->fn)(obj1, obj2, JSVar(), JSVar(), JSVar(), JSVar(), JSVar(), JSVar(), JSVar()); 
			case Type::fn10: return (_asFn10()->fn)(obj1, obj2, JSVar(), JSVar(), JSVar(), JSVar(), JSVar(), JSVar(), JSVar(), JSVar()); 
			default: throw; 
		}
	}

	JSVar JSVar::operator()( JSVar obj1, JSVar obj2, JSVar obj3 ) { 
		nodecpp::safememory::owning_ptr<JSArray> args = makeJSArray( { obj1, obj2, obj3 } );
		JSStackMan restorer( args );

		switch ( type )
		{
			case Type::fn0: return (_asFn0()->fn)(); 
			case Type::fn1: return (_asFn1()->fn)(obj1 ); 
			case Type::fn2: return (_asFn2()->fn)(obj1, obj2 ); 
			case Type::fn3: return (_asFn3()->fn)(obj1, obj2, obj3 ); 
			case Type::fn4: return (_asFn4()->fn)(obj1, obj2, obj3, JSVar()); 
			case Type::fn5: return (_asFn5()->fn)(obj1, obj2, obj3, JSVar(), JSVar()); 
			case Type::fn6: return (_asFn6()->fn)(obj1, obj2, obj3, JSVar(), JSVar(), JSVar()); 
			case Type::fn7: return (_asFn7()->fn)(obj1, obj2, obj3, JSVar(), JSVar(), JSVar(), JSVar()); 
			case Type::fn8: return (_asFn8()->fn)(obj1, obj2, obj3, JSVar(), JSVar(), JSVar(), JSVar(), JSVar()); 
			case Type::fn9: return (_asFn9()->fn)(obj1, obj2, obj3, JSVar(), JSVar(), JSVar(), JSVar(), JSVar(), JSVar()); 
			case Type::fn10: return (_asFn10()->fn)(obj1, obj2, obj3, JSVar(), JSVar(), JSVar(), JSVar(), JSVar(), JSVar(), JSVar()); 
			default: throw; 
		}
	}

	JSVar JSVar::operator()( JSVar obj1, JSVar obj2, JSVar obj3, JSVar obj4 ) { 
		nodecpp::safememory::owning_ptr<JSArray> args = makeJSArray( { obj1, obj2, obj3, obj4 } );
		JSStackMan restorer( args );

		switch ( type )
		{
			case Type::fn0: return (_asFn0()->fn)(); 
			case Type::fn1: return (_asFn1()->fn)(obj1 ); 
			case Type::fn2: return (_asFn2()->fn)(obj1, obj2 ); 
			case Type::fn3: return (_asFn3()->fn)(obj1, obj2, obj3 ); 
			case Type::fn4: return (_asFn4()->fn)(obj1, obj2, obj3, obj4 ); 
			case Type::fn5: return (_asFn5()->fn)(obj1, obj2, obj3, obj4, JSVar()); 
			case Type::fn6: return (_asFn6()->fn)(obj1, obj2, obj3, obj4, JSVar(), JSVar()); 
			case Type::fn7: return (_asFn7()->fn)(obj1, obj2, obj3, obj4, JSVar(), JSVar(), JSVar()); 
			case Type::fn8: return (_asFn8()->fn)(obj1, obj2, obj3, obj4, JSVar(), JSVar(), JSVar(), JSVar()); 
			case Type::fn9: return (_asFn9()->fn)(obj1, obj2, obj3, obj4, JSVar(), JSVar(), JSVar(), JSVar(), JSVar()); 
			case Type::fn10: return (_asFn10()->fn)(obj1, obj2, obj3, obj4, JSVar(), JSVar(), JSVar(), JSVar(), JSVar(), JSVar()); 
			default: throw; 
		}
	}

	JSVar JSVar::operator()( JSVar obj1, JSVar obj2, JSVar obj3, JSVar obj4, JSVar obj5 ) { 
		nodecpp::safememory::owning_ptr<JSArray> args = makeJSArray( { obj1, obj2, obj3, obj4, obj5 } );
		JSStackMan restorer( args );

		switch ( type )
		{
			case Type::fn0: return (_asFn0()->fn)(); 
			case Type::fn1: return (_asFn1()->fn)(obj1 ); 
			case Type::fn2: return (_asFn2()->fn)(obj1, obj2 ); 
			case Type::fn3: return (_asFn3()->fn)(obj1, obj2, obj3 ); 
			case Type::fn4: return (_asFn4()->fn)(obj1, obj2, obj3, obj4 ); 
			case Type::fn5: return (_asFn5()->fn)(obj1, obj2, obj3, obj4, obj5 ); 
			case Type::fn6: return (_asFn6()->fn)(obj1, obj2, obj3, obj4, obj5, JSVar()); 
			case Type::fn7: return (_asFn7()->fn)(obj1, obj2, obj3, obj4, obj5, JSVar(), JSVar()); 
			case Type::fn8: return (_asFn8()->fn)(obj1, obj2, obj3, obj4, obj5, JSVar(), JSVar(), JSVar()); 
			case Type::fn9: return (_asFn9()->fn)(obj1, obj2, obj3, obj4, obj5, JSVar(), JSVar(), JSVar(), JSVar()); 
			case Type::fn10: return (_asFn10()->fn)(obj1, obj2, obj3, obj4, obj5, JSVar(), JSVar(), JSVar(), JSVar(), JSVar()); 
			default: throw; 
		}
	}

	JSVar JSVar::operator()( JSVar obj1, JSVar obj2, JSVar obj3, JSVar obj4, JSVar obj5, JSVar obj6 ) { 
		nodecpp::safememory::owning_ptr<JSArray> args = makeJSArray( { obj1, obj2, obj3, obj4, obj5, obj6 } );
		JSStackMan restorer( args );

		switch ( type )
		{
			case Type::fn0: return (_asFn0()->fn)(); 
			case Type::fn1: return (_asFn1()->fn)(obj1 ); 
			case Type::fn2: return (_asFn2()->fn)(obj1, obj2 ); 
			case Type::fn3: return (_asFn3()->fn)(obj1, obj2, obj3 ); 
			case Type::fn4: return (_asFn4()->fn)(obj1, obj2, obj3, obj4 ); 
			case Type::fn5: return (_asFn5()->fn)(obj1, obj2, obj3, obj4, obj5 ); 
			case Type::fn6: return (_asFn6()->fn)(obj1, obj2, obj3, obj4, obj5, obj6 ); 
			case Type::fn7: return (_asFn7()->fn)(obj1, obj2, obj3, obj4, obj5, obj6, JSVar()); 
			case Type::fn8: return (_asFn8()->fn)(obj1, obj2, obj3, obj4, obj5, obj6, JSVar(), JSVar()); 
			case Type::fn9: return (_asFn9()->fn)(obj1, obj2, obj3, obj4, obj5, obj6, JSVar(), JSVar(), JSVar()); 
			case Type::fn10: return (_asFn10()->fn)(obj1, obj2, obj3, obj4, obj5, obj6, JSVar(), JSVar(), JSVar(), JSVar()); 
			default: throw; 
		}
	}

	JSVar JSVar::operator()( JSVar obj1, JSVar obj2, JSVar obj3, JSVar obj4, JSVar obj5, JSVar obj6, JSVar obj7 ) { 
		nodecpp::safememory::owning_ptr<JSArray> args = makeJSArray( { obj1, obj2, obj3, obj4, obj5, obj6, obj7 } );
		JSStackMan restorer( args );

		switch ( type )
		{
			case Type::fn0: return (_asFn0()->fn)(); 
			case Type::fn1: return (_asFn1()->fn)(obj1 ); 
			case Type::fn2: return (_asFn2()->fn)(obj1, obj2 ); 
			case Type::fn3: return (_asFn3()->fn)(obj1, obj2, obj3 ); 
			case Type::fn4: return (_asFn4()->fn)(obj1, obj2, obj3, obj4 ); 
			case Type::fn5: return (_asFn5()->fn)(obj1, obj2, obj3, obj4, obj5 ); 
			case Type::fn6: return (_asFn6()->fn)(obj1, obj2, obj3, obj4, obj5, obj6 ); 
			case Type::fn7: return (_asFn7()->fn)(obj1, obj2, obj3, obj4, obj5, obj6, obj7 ); 
			case Type::fn8: return (_asFn8()->fn)(obj1, obj2, obj3, obj4, obj5, obj6, obj7, JSVar()); 
			case Type::fn9: return (_asFn9()->fn)(obj1, obj2, obj3, obj4, obj5, obj6, obj7, JSVar(), JSVar()); 
			case Type::fn10: return (_asFn10()->fn)(obj1, obj2, obj3, obj4, obj5, obj6, obj7, JSVar(), JSVar(), JSVar()); 
			default: throw; 
		}
	}

	JSVar JSVar::operator()( JSVar obj1, JSVar obj2, JSVar obj3, JSVar obj4, JSVar obj5, JSVar obj6, JSVar obj7, JSVar obj8 ) { 
		nodecpp::safememory::owning_ptr<JSArray> args = makeJSArray( { obj1, obj2, obj3, obj4, obj5, obj6, obj7, obj8 } );
		JSStackMan restorer( args );

		switch ( type )
		{
			case Type::fn0: return (_asFn0()->fn)(); 
			case Type::fn1: return (_asFn1()->fn)(obj1 ); 
			case Type::fn2: return (_asFn2()->fn)(obj1, obj2 ); 
			case Type::fn3: return (_asFn3()->fn)(obj1, obj2, obj3 ); 
			case Type::fn4: return (_asFn4()->fn)(obj1, obj2, obj3, obj4 ); 
			case Type::fn5: return (_asFn5()->fn)(obj1, obj2, obj3, obj4, obj5 ); 
			case Type::fn6: return (_asFn6()->fn)(obj1, obj2, obj3, obj4, obj5, obj6 ); 
			case Type::fn7: return (_asFn7()->fn)(obj1, obj2, obj3, obj4, obj5, obj6, obj7 ); 
			case Type::fn8: return (_asFn8()->fn)(obj1, obj2, obj3, obj4, obj5, obj6, obj7, obj8 ); 
			case Type::fn9: return (_asFn9()->fn)(obj1, obj2, obj3, obj4, obj5, obj6, obj7, obj8, JSVar()); 
			case Type::fn10: return (_asFn10()->fn)(obj1, obj2, obj3, obj4, obj5, obj6, obj7, obj8, JSVar(), JSVar()); 
			default: throw; 
		}
	}

	JSVar JSVar::operator()( JSVar obj1, JSVar obj2, JSVar obj3, JSVar obj4, JSVar obj5, JSVar obj6, JSVar obj7, JSVar obj8, JSVar obj9 ) { 
		nodecpp::safememory::owning_ptr<JSArray> args = makeJSArray( { obj1, obj2, obj3, obj4, obj5, obj6, obj7, obj8, obj9 } );
		JSStackMan restorer( args );

		switch ( type )
		{
			case Type::fn0: return (_asFn0()->fn)(); 
			case Type::fn1: return (_asFn1()->fn)(obj1 ); 
			case Type::fn2: return (_asFn2()->fn)(obj1, obj2 ); 
			case Type::fn3: return (_asFn3()->fn)(obj1, obj2, obj3 ); 
			case Type::fn4: return (_asFn4()->fn)(obj1, obj2, obj3, obj4 ); 
			case Type::fn5: return (_asFn5()->fn)(obj1, obj2, obj3, obj4, obj5 ); 
			case Type::fn6: return (_asFn6()->fn)(obj1, obj2, obj3, obj4, obj5, obj6 ); 
			case Type::fn7: return (_asFn7()->fn)(obj1, obj2, obj3, obj4, obj5, obj6, obj7 ); 
			case Type::fn8: return (_asFn8()->fn)(obj1, obj2, obj3, obj4, obj5, obj6, obj7, obj8 ); 
			case Type::fn9: return (_asFn9()->fn)(obj1, obj2, obj3, obj4, obj5, obj6, obj7, obj8, obj9 ); 
			case Type::fn10: return (_asFn10()->fn)(obj1, obj2, obj3, obj4, obj5, obj6, obj7, obj8, obj9, JSVar()); 
			default: throw; 
		}
	}

	JSVar JSVar::operator()( JSVar obj1, JSVar obj2, JSVar obj3, JSVar obj4, JSVar obj5, JSVar obj6, JSVar obj7, JSVar obj8, JSVar obj9, JSVar obj10 ) { 
		nodecpp::safememory::owning_ptr<JSArray> args = makeJSArray( { obj1, obj2, obj3, obj4, obj5, obj6, obj7, obj8, obj9, obj10 } );
		JSStackMan restorer( args );

		switch ( type )
		{
			case Type::fn0: return (_asFn0()->fn)(); 
			case Type::fn1: return (_asFn1()->fn)(obj1 ); 
			case Type::fn2: return (_asFn2()->fn)(obj1, obj2 ); 
			case Type::fn3: return (_asFn3()->fn)(obj1, obj2, obj3 ); 
			case Type::fn4: return (_asFn4()->fn)(obj1, obj2, obj3, obj4 ); 
			case Type::fn5: return (_asFn5()->fn)(obj1, obj2, obj3, obj4, obj5 ); 
			case Type::fn6: return (_asFn6()->fn)(obj1, obj2, obj3, obj4, obj5, obj6 ); 
			case Type::fn7: return (_asFn7()->fn)(obj1, obj2, obj3, obj4, obj5, obj6, obj7 ); 
			case Type::fn8: return (_asFn8()->fn)(obj1, obj2, obj3, obj4, obj5, obj6, obj7, obj8 ); 
			case Type::fn9: return (_asFn9()->fn)(obj1, obj2, obj3, obj4, obj5, obj6, obj7, obj8, obj9 ); 
			case Type::fn10: return (_asFn10()->fn)(obj1, obj2, obj3, obj4, obj5, obj6, obj7, obj8, obj9, obj10 ); 
			default: throw; 
		}
	}

	bool JSVar::operator !() const
	{
		return isFalseValue();
	}

	JSVar JSVar::operator %( const JSVar& other ) const
	{
		// TODO: make sure we report right values!!!
		double me_ = toNumber();
		if ( std::isnan( me_ ) )
			return NAN;
		double other_ = other.toNumber();
		if ( std::isnan( other_ ) )
			return NAN;
		bool me_negative = me_ < 0;
		if ( me_negative )
			me_ = -me_;
		if ( other_ < 0 )
			other_ = - other_;
		double rat = me_ / other_;
		if ( std::isnan( rat ) )
			return NAN;
		uint64_t irat = (uint64_t)(rat);
		double rem = me_ - other_ * irat;
		if ( me_negative )
			rem = -rem;
		return rem;
	}

	JSVar JSVar::operator ||( const JSVar& other ) const
	{
		bool me_ = !isFalseValue();
		bool other_ = !other.isFalseValue();
		if ( me_ )
			return *this;
		else
			return other;
	}

	JSVar JSVar::operator &&( const JSVar& other ) const
	{
		bool me_ = !isFalseValue();
		bool other_ = !other.isFalseValue();
		if ( me_ )
			return other;
		else
			return *this;
	}

	bool JSVar::operator ==( const JSVar& other ) const
	{
		switch ( type )
		{
			case Type::undef:
				switch ( other.type )
				{
					case Type::undef:
						return true; 
					case Type::softptr:
						return *_asSoft() == nullptr;
					default:
						return false;
				}
			case Type::boolean:
				switch ( other.type )
				{
					case Type::undef:
						return !*_asBool(); 
					case Type::boolean:
						return *_asBool() == *(other._asBool());
					case Type::num:
						return (*_asBool() ? 1 : 0) == *(other._asNum());
					case Type::string:
						return other.toNumber() == (_asBool() ? 1 : 0);
					default:
						return false;
				}
			case Type::num:
			{
				switch ( other.type )
				{
					case Type::undef:
						return false; 
					case Type::boolean:
						return *_asNum() == (*(other._asBool()) ? 1 : 0);
					case Type::num:
						return *_asNum() == *(other._asNum());
					case Type::string:
						return *_asNum() == other.toNumber();
					default:
						return false;
				}
			}
			case Type::string:
			{
				switch ( other.type )
				{
					case Type::string:
						return _asStr()->str() == other._asStr()->str();
					case Type::boolean:
						return toNumber() == (*(other._asBool()) ? 1 : 0);
					case Type::num:
						return toNumber() == *(other._asNum());
					case Type::softptr:
						if ( *(other._asSoft()) == nullptr )
							return false;
						else
							return _asStr()->str() == other.toString();
					default:
						return false;
				}
			}
			case Type::softptr:
				switch ( other.type )
				{
					case Type::undef:
						return *_asSoft() == nullptr;
					case Type::softptr:
						return *_asSoft() == *(other._asSoft());
					default:
						return false;
				}
			case JSVarBase::Type::fn0:
				if ( other.type == fn0 )
					return false; // a mechanism is required to check whether we have copies of the same function
				else
					return false;
			case JSVarBase::Type::fn1:
				if ( other.type == fn1 )
					return false; // a mechanism is required to check whether we have copies of the same function
				else
					return false;
			case JSVarBase::Type::fn2:
				if ( other.type == fn2 )
					return false; // a mechanism is required to check whether we have copies of the same function
				else
					return false;
			case JSVarBase::Type::fn3:
				if ( other.type == fn3 )
					return false; // a mechanism is required to check whether we have copies of the same function
				else
					return false;
			case JSVarBase::Type::fn4:
				if ( other.type == fn4 )
					return false; // a mechanism is required to check whether we have copies of the same function
				else
					return false;
			case JSVarBase::Type::fn5:
				if ( other.type == fn5 )
					return false; // a mechanism is required to check whether we have copies of the same function
				else
					return false;
			case JSVarBase::Type::fn6:
				if ( other.type == fn6 )
					return false; // a mechanism is required to check whether we have copies of the same function
				else
					return false;
			case JSVarBase::Type::fn7:
				if ( other.type == fn7 )
					return false; // a mechanism is required to check whether we have copies of the same function
				else
					return false;
			case JSVarBase::Type::fn8:
				if ( other.type == fn8 )
					return false; // a mechanism is required to check whether we have copies of the same function
				else
					return false;
			case JSVarBase::Type::fn9:
				if ( other.type == fn9 )
					return false; // a mechanism is required to check whether we have copies of the same function
				else
					return false;
			case JSVarBase::Type::fn10:
				if ( other.type == fn10 )
					return false; // a mechanism is required to check whether we have copies of the same function
				else
					return false;
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
				return false;
		}
		return false;
	}

	bool JSVar::isStrictlyTheSame( const JSVar& other ) const
	{
		if ( type != other.type )
			return false;
		switch ( type )
		{
			case Type::undef:
				return true;
			case Type::boolean:
				return *_asBool() == *(other._asBool());
			case Type::num:
				return *_asNum() == *(other._asNum());
			case Type::string:
				return _asStr()->str() == other._asStr()->str();
			case Type::softptr:
				return *_asSoft() == *(other._asSoft());
			case JSVarBase::Type::fn0:
				return false; // a mechanism is required to check whether we have copies of the same function
			case JSVarBase::Type::fn1:
				return false; // a mechanism is required to check whether we have copies of the same function
			case JSVarBase::Type::fn2:
				return false; // a mechanism is required to check whether we have copies of the same function
			case JSVarBase::Type::fn3:
				return false; // a mechanism is required to check whether we have copies of the same function
			case JSVarBase::Type::fn4:
				return false; // a mechanism is required to check whether we have copies of the same function
			case JSVarBase::Type::fn5:
				return false; // a mechanism is required to check whether we have copies of the same function
			case JSVarBase::Type::fn6:
				return false; // a mechanism is required to check whether we have copies of the same function
			case JSVarBase::Type::fn7:
				return false; // a mechanism is required to check whether we have copies of the same function
			case JSVarBase::Type::fn8:
				return false; // a mechanism is required to check whether we have copies of the same function
			case JSVarBase::Type::fn9:
				return false; // a mechanism is required to check whether we have copies of the same function
			case JSVarBase::Type::fn10:
				return false; // a mechanism is required to check whether we have copies of the same function
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
				return false;
		}
		return false;
	}
	
	JSVar JSVar::operator += (const JSVar& other )
	{
		// TODO: check for correctness and completeness
		switch ( type )
		{
			case Type::boolean:
				switch ( other.type )
				{
					case Type::boolean:
					{
						double val = ( *(_asBool()) ? 1 : 0 ) + ( *(other._asBool()) ? 1 : 0 );
						init( val );
						return val;
					}
					case Type::num:
					{
						double val = ( *(_asBool()) ? 1 : 0 ) + *(other._asNum());
						init( val );
						return val;
					}
					case Type::string:
					{
						init( toString() + other._asStr()->str() );
						return *(_asStr());
					}
					default:
						throw;
				}
				break;
			case Type::num:
			{
				switch ( other.type )
				{
					case Type::boolean:
					{
						*(_asNum()) += ( *(other._asBool()) ? 1 : 0 );
						return *(_asNum());
					}
					case Type::num:
					{
						*(_asNum()) += *(other._asNum());
						return *(_asNum());
					}
					case Type::string:
					{
						init( toString() + other._asStr()->str() );
						return *(_asStr());
					}
					default:
						throw;
				}
				break;
			}
			case Type::string:
			{
				switch ( other.type )
				{
					case Type::boolean:
					case Type::num:
					{
						_asStr()->str() += other.toString();
						return *(_asStr());
					}
					case Type::string:
					{
						_asStr()->str() += other._asStr()->str();
						return *(_asStr());
					}
					default:
						throw;
				}
				break;
			}
			case Type::undef:
			case Type::softptr:
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
				throw;
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
				return JSVar();
		}
	}

	JSVar JSVar::operator + (const JSVar& other ) const
	{
		// TODO: check for correctness and completeness
		switch ( type )
		{
			case Type::boolean:
				switch ( other.type )
				{
					case Type::boolean:
						return ( *(_asBool()) ? 1 : 0 ) + ( *(other._asBool()) ? 1 : 0 );
					case Type::num:
						return ( *(_asBool()) ? 1 : 0 ) + *(other._asNum());
					case Type::string:
						return toString() + other._asStr()->str();
					default:
						throw;
				}
				break;
			case Type::num:
			{
				switch ( other.type )
				{
					case Type::boolean:
						return *(_asNum()) + ( *(other._asBool()) ? 1 : 0 );
					case Type::num:
						return *(_asNum()) + *(other._asNum());
					case Type::string:
						return toString() + other._asStr()->str();
					default:
						throw;
				}
				break;
			}
			case Type::string:
			{
				switch ( other.type )
				{
					case Type::boolean:
					case Type::num:
						return _asStr()->str() + other.toString();
					case Type::string:
						return _asStr()->str() + other._asStr()->str();
					default:
						throw;
				}
				break;
			}
			case Type::undef:
			case Type::softptr:
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
				throw;
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
				return JSVar();
		}
	}

	JSVar JSVar::operator *= (const JSVar& other )
	{
		// TODO: check for correctness and completeness
		switch ( type )
		{
			case Type::boolean:
				switch ( other.type )
				{
					case Type::boolean:
					{
						double val = ( *(_asBool()) ? 1 : 0 ) * ( *(other._asBool()) ? 1 : 0 );
						init( val );
						return val;
					}
					case Type::num:
					{
						double val = ( *(_asBool()) ? 1 : 0 ) * *(other._asNum());
						init( val );
						return val;
					}
					case Type::string:
						init( NAN );
						return NAN;
					default:
						throw;
				}
				break;
			case Type::num:
			{
				switch ( other.type )
				{
					case Type::boolean:
					{
						*(_asNum()) *= ( *(other._asBool()) ? 1 : 0 );
						return *(_asNum());
					}
					case Type::num:
					{
						*(_asNum()) *= *(other._asNum());
						return *(_asNum());
					}
					case Type::string:
						init( NAN );
						return NAN;
					default:
						throw;
				}
				break;
			}
			case Type::string:
				return NAN;
			case Type::undef:
			case Type::softptr:
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
				throw;
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
				return JSVar();
		}
	}

	JSVar JSVar::operator * (const JSVar& other ) const
	{
		// TODO: check for correctness and completeness
		switch ( type )
		{
			case Type::boolean:
				switch ( other.type )
				{
					case Type::boolean:
						return ( *(_asBool()) ? 1 : 0 ) * ( *(other._asBool()) ? 1 : 0 );
					case Type::num:
						return ( *(_asBool()) ? 1 : 0 ) * *(other._asNum());
					case Type::string:
						return NAN;
					default:
						throw;
				}
				break;
			case Type::num:
			{
				switch ( other.type )
				{
					case Type::boolean:
						return *(_asNum()) * ( *(other._asBool()) ? 1 : 0 );
					case Type::num:
						return *(_asNum()) * *(other._asNum());
					case Type::string:
						return NAN;
					default:
						throw;
				}
				break;
			}
			case Type::string:
				return NAN;
			case Type::undef:
			case Type::softptr:
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
				throw;
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
				return JSVar();
		}
	}

	JSVar JSVar::operator++()
	{
		double val = toNumber();
		if ( !std::isnan( val ) )
			JSVarBase::init( val + 1 );
		else
			JSVarBase::init( val );
		return *this;
	}

	JSVar JSVar::operator++(int)
	{
		double val = toNumber();
		if ( !std::isnan( val ) )
		{
			JSVarBase::init( val + 1 );
			return JSVar( val );
		}
		else
		{
			JSVarBase::init( val );
			return *this;
		}
	}
	
	JSVar JSVar::operator--()
	{
		double val = toNumber();
		if ( !std::isnan( val ) )
			JSVarBase::init( val - 1 );
		else
			JSVarBase::init( val );
		return *this;
	}

	JSVar JSVar::operator--(int)
	{
		double val = toNumber();
		if ( !std::isnan( val ) )
		{
			JSVarBase::init( val - 1 );
			return JSVar( val );
		}
		else
		{
			JSVarBase::init( val );
			return *this;
		}
	}

	bool JSVar::isFalseValue() const
	{
		switch ( type )
		{
			case Type::undef:
				return true; 
			case Type::boolean:
				return *_asBool() == false;
			case Type::num:
				return *_asNum() == 0 || std::isnan( *_asNum() );
			case Type::string:
				return _asStr()->size() == 0;
			case Type::softptr:
				return *_asSoft() == nullptr;
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
				return false; 
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
				return true;
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
				return _asStr()->str();
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
	
	double JSVar::toNumber() const
	{
		switch ( type )
		{
			case Type::num:
				return *_asNum();
			case Type::boolean:
				return *_asBool() ? 1 : 0;
			case Type::string:
			{
				char* end = nullptr;
				double ret = strtod(_asStr()->c_str(), &end);
				if ( end == _asStr()->c_str() + _asStr()->size() )
					return ret;
				return NAN;
			}
			case Type::undef:
			case Type::softptr:
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
				return NAN;
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
				return NAN;
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

	nodecpp::safememory::owning_ptr<JSArray> JSVar::keys()
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
			case Type::string:
				throw; // type error TODO: revise!
			case Type::softptr:
				return (*_asSoft())->keys();
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
				return nodecpp::safememory::owning_ptr<JSArray>();
		}
	}

	JSOwnObj JSVar::split() const
	{
		nodecpp::string str = toString();
		return makeJSArray( {JSVar( str )} );
	}

	JSOwnObj JSVar::split(  const JSVar& separator, JSVar maxCount ) const
	{
		// TODO: reimplement! (optimization, at minimum)
		// TODO: regexp
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, separator.type == Type::string && separator._asStr()->size() <= 1, "error: not implemented yet" ); 
		nodecpp::string str = toString();
		auto arr = makeJSArray();
		if ( separator._asStr()->size() == 0 )
		{
			for ( auto ch : str )
			{
				nodecpp::string fragment;
				fragment += ch;
				arr->arrayValues.push_back( JSVar( fragment ) );
			}
		}
		else
		{
			size_t mcnt = maxCount.toNumber();
			size_t cnt = 0;
			nodecpp::string separ = separator._asStr()->str();
			size_t start = 0;
			for ( ; cnt<mcnt; ++cnt )
			{
				size_t end = str.find( separ, start );
				if ( end == nodecpp::string::npos )
				{
					arr->arrayValues.push_back( JSVar( str.substr( start ) ) );
					break;
				}
				else
				{
					arr->arrayValues.push_back( JSVar( str.substr( start, end - start ) ) );
					start = end + separ.size();
				}
			}
		}
		return arr;
	}

	double JSVar::length() const
	{ 
		if ( type == Type::softptr )
			return (*_asSoft())->length();
		else
			throw; 
	}
	void JSVar::setLength( double ln )
	{ 
		if ( type == Type::softptr )
			(*_asSoft())->setLength( ln );
		else
			throw; 
	}

	JSVar JSVar::toLowerCase() const
	{
		// TODO: other than ASCII
		nodecpp::string ret;
		const nodecpp::string& str = toString();
		for ( size_t idx = 0; idx<str.size(); ++idx )
		{
			char ch = str[ idx ];
			if ( ch >= 'A' && ch <= 'Z' )
				ret += ch - ('A' - 'a');
			else
				ret += ch;
		}
		return ret;
	}

	JSVar JSVar::toUpperCase() const
	{
		// TODO: other than ASCII
		nodecpp::string ret;
		const nodecpp::string& str = toString();
		for ( size_t idx = 0; idx<str.size(); ++idx )
		{
			char ch = str[ idx ];
			if ( ch >= 'a' && ch <= 'z' )
				ret += ch + ('A' - 'a');
			else
				ret += ch;
		}
		return ret;
	}

	JSOwnObj JSVar::match( const JSRegExp& re ) const
	{
		if ( type != Type::string )
			throw;
		nodecpp::safememory::owning_ptr<JSArray> ret = makeJSArray();
//		const nodecpp::string& str = _asStr()->str();
		const char* str = _asStr()->str().c_str();
		if ( re.flags() & JSRegExp::Flags::g )
		{
//			std::regex stdre( re.re() );
			std::regex stdre( re.re().c_str() );
//			auto words_begin = std::sregex_iterator(str.begin(), str.end(), stdre);
			auto words_begin = std::cregex_iterator(str, str + _asStr()->str().size(), stdre);
//			auto words_end = std::sregex_iterator();
			auto words_end = std::cregex_iterator();
//			for (std::sregex_iterator i = words_begin; i != words_end; ++i)
			for (std::cregex_iterator i = words_begin; i != words_end; ++i)
			{
				nodecpp::string found = i->str().c_str();
				ret->arrayValues.push_back( JSVar( found ) );
			}
			return ret;
		}
		else
		{
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "error: not implemented" ); 
			return makeJSArray();
			// TODO: ...
		}
	}

	JSVar JSVar::replace( const JSRegExp& re, const JSVar replacement ) const
	{
		if ( type != Type::string )
			throw;
		if ( replacement.type != Type::string )
			throw;
		const nodecpp::string& str = _asStr()->str();
		const nodecpp::string& repl = replacement._asStr()->str();
		if ( re.flags() & JSRegExp::Flags::g )
		{
			std::regex stdre( re.re() );
			std::string out = std::regex_replace(std::string(str.c_str()), stdre, std::string(repl.c_str()));
			return nodecpp::string( out.c_str() );
		}
		else
		{
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "error: not implemented" ); 
			return JSVar();
			// TODO: ...
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

	double JSVarOrOwn::toNumber() const
	{
		switch ( type )
		{
			case Type::undef:
				return NAN;
			case Type::var:
				return _asVar().toNumber();
			case Type::obj:
				return _asPtr().toNumber();
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
				return NAN;
		}
	}

	double JSVarOrOwn::length() const
	{ 
		if ( type == Type::obj )
			return _asPtr().length();
		else
			throw; 
	}
	void JSVarOrOwn::setLength( double ln )
	{ 
		if ( type == Type::obj )
			_asPtr().setLength( ln );
		else
			throw; 
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

	JSRLValue::JSRLValue(JSRLValue&& other) {
		type = other.type;
		switch (other.type)
		{
		case Type::undef:
			break;
		case Type::var:
			new(&(_asVar()))JSVar(other._asVar());
			other.type = Type::undef;
			break;
		case Type::value:
			_asValue() = std::move(other._asValue());
			other.type = Type::undef;
			break;
		default:
			NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type);
		}
	}

	void JSRLValue::initBy( const JSVar& var ) {
		new(&(_asVar()))JSVar( var );
		type = Type::var;
	}

	JSRLValue JSRLValue::from( const JSVar& var ) {
		JSRLValue ret;
		ret.initBy( var );
		return ret;
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

	JSVar JSRLValue::operator()() {
		switch ( type )
		{
			case Type::var: return _asVar()();
			case Type::value: if ( _asValue()->type == JSVarOrOwn::Type::var ) return _asValue()->_asVar()(); else throw;
			default: throw;
		}
	}

	JSVar JSRLValue::operator()( JSVar obj1 ) {
		switch ( type )
		{
			case Type::var: 
				return _asVar()( obj1 );
			case Type::value: 
				if ( _asValue()->type == JSVarOrOwn::Type::var ) 
					return _asValue()->_asVar()( obj1 ); 
				else throw;
			default: throw;
		}
	}

	JSVar JSRLValue::operator()( JSVar obj1, JSVar obj2 ) {
		switch ( type )
		{
			case Type::var: 
				return _asVar()( obj1, obj2 );
			case Type::value: 
				if ( _asValue()->type == JSVarOrOwn::Type::var ) 
					return _asValue()->_asVar()( obj1, obj2 ); 
				else throw;
			default: throw;
		}
	}

	JSVar JSRLValue::operator()( JSVar obj1, JSVar obj2, JSVar obj3 ) {
		switch ( type )
		{
			case Type::var: 
				return _asVar()( obj1, obj2, obj3 );
			case Type::value: 
				if ( _asValue()->type == JSVarOrOwn::Type::var ) 
					return _asValue()->_asVar()( obj1, obj2, obj3 ); 
				else throw;
			default: throw;
		}
	}

	JSVar JSRLValue::operator()( JSVar obj1, JSVar obj2, JSVar obj3, JSVar obj4 ) {
		switch ( type )
		{
			case Type::var: 
				return _asVar()( obj1, obj2, obj3, obj4 );
			case Type::value: 
				if ( _asValue()->type == JSVarOrOwn::Type::var ) 
					return _asValue()->_asVar()( obj1, obj2, obj3, obj4 ); 
				else throw;
			default: throw;
		}
	}

	JSVar JSRLValue::operator()( JSVar obj1, JSVar obj2, JSVar obj3, JSVar obj4, JSVar obj5 ) {
		switch ( type )
		{
			case Type::var: 
				return _asVar()( obj1, obj2, obj3, obj4, obj5 );
			case Type::value: 
				if ( _asValue()->type == JSVarOrOwn::Type::var ) 
					return _asValue()->_asVar()( obj1, obj2, obj3, obj4, obj5 ); 
				else throw;
			default: throw;
		}
	}

	JSVar JSRLValue::operator()( JSVar obj1, JSVar obj2, JSVar obj3, JSVar obj4, JSVar obj5, JSVar obj6 ) {
		switch ( type )
		{
			case Type::var: 
				return _asVar()( obj1, obj2, obj3, obj4, obj5, obj6 );
			case Type::value: 
				if ( _asValue()->type == JSVarOrOwn::Type::var ) 
					return _asValue()->_asVar()( obj1, obj2, obj3, obj4, obj5, obj6 ); 
				else throw;
			default: throw;
		}
	}

	JSVar JSRLValue::operator()( JSVar obj1, JSVar obj2, JSVar obj3, JSVar obj4, JSVar obj5, JSVar obj6, JSVar obj7 ) {
		switch ( type )
		{
			case Type::var: 
				return _asVar()( obj1, obj2, obj3, obj4, obj5, obj6, obj7 );
			case Type::value: 
				if ( _asValue()->type == JSVarOrOwn::Type::var ) 
					return _asValue()->_asVar()( obj1, obj2, obj3, obj4, obj5, obj6, obj7 ); 
				else throw;
			default: throw;
		}
	}

	JSVar JSRLValue::operator()( JSVar obj1, JSVar obj2, JSVar obj3, JSVar obj4, JSVar obj5, JSVar obj6, JSVar obj7, JSVar obj8 ) {
		switch ( type )
		{
			case Type::var: 
				return _asVar()( obj1, obj2, obj3, obj4, obj5, obj6, obj7, obj8 );
			case Type::value: 
				if ( _asValue()->type == JSVarOrOwn::Type::var ) 
					return _asValue()->_asVar()( obj1, obj2, obj3, obj4, obj5, obj6, obj7, obj8 ); 
				else throw;
			default: throw;
		}
	}

	JSVar JSRLValue::operator()( JSVar obj1, JSVar obj2, JSVar obj3, JSVar obj4, JSVar obj5, JSVar obj6, JSVar obj7, JSVar obj8, JSVar obj9 ) {
		switch ( type )
		{
			case Type::var: 
				return _asVar()( obj1, obj2, obj3, obj4, obj5, obj6, obj7, obj8, obj9 );
			case Type::value: 
				if ( _asValue()->type == JSVarOrOwn::Type::var ) 
					return _asValue()->_asVar()( obj1, obj2, obj3, obj4, obj5, obj6, obj7, obj8, obj9 ); 
				else throw;
			default: throw;
		}
	}

	JSVar JSRLValue::operator()( JSVar obj1, JSVar obj2, JSVar obj3, JSVar obj4, JSVar obj5, JSVar obj6, JSVar obj7, JSVar obj8, JSVar obj9, JSVar obj10 ) {
		switch ( type )
		{
			case Type::var: 
				return _asVar()( obj1, obj2, obj3, obj4, obj5, obj6, obj7, obj8, obj9, obj10 );
			case Type::value: 
				if ( _asValue()->type == JSVarOrOwn::Type::var ) 
					return _asValue()->_asVar()( obj1, obj2, obj3, obj4, obj5, obj6, obj7, obj8, obj9, obj10 ); 
				else throw;
			default: throw;
		}
	}

	bool JSRLValue::operator !() const 
	{
		JSVar tmp = *this; return !tmp;
	}

	JSVar JSRLValue::operator %( const JSVar& other ) const
	{
		JSVar tmp = *this; return tmp % other;
	}

	JSVar JSRLValue::operator ||( const JSVar& other ) const
	{
		JSVar tmp = *this; return tmp || other;
	}

	JSVar JSRLValue::operator &&( const JSVar& other ) const
	{
		JSVar tmp = *this; return tmp && other;
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

	JSRLValue JSRLValue::operator [] ( const JSVar& var ) const
	{
		switch ( type )
		{
			case Type::undef:
				return JSRLValue();
			case Type::var:
				return _asVar().operator[]( var );
			case Type::value:
			{
				auto& v = *(_asValue());
				switch ( v.type )
				{
					case JSVarOrOwn::Type::undef:
						return JSRLValue();
					case JSVarOrOwn::Type::var:
						return v._asVar().operator[]( var );
					case JSVarOrOwn::Type::obj:
						return v._asPtr().operator[]( var );
					default:
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
						return JSRLValue();
				}
			}
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
				return JSRLValue();
		}
	}

	JSRLValue JSRLValue::operator [] ( const JSRLValue& val ) const
	{
		if ( val.type != Type::var )
			return JSRLValue(); // TODO: check if we should throw or convert to string, etc, instead
		const JSVar& var = val._asVar();
		switch ( type )
		{
			case Type::undef:
				return JSRLValue();
			case Type::var:
				return _asVar().operator[]( var );
			case Type::value:
			{
				auto& v = *(_asValue());
				switch ( v.type )
				{
					case JSVarOrOwn::Type::undef:
						return JSRLValue();
					case JSVarOrOwn::Type::var:
						return v._asVar().operator[]( var );
					case JSVarOrOwn::Type::obj:
						return v._asPtr().operator[]( var );
					default:
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
						return JSRLValue();
				}
			}
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
				return JSRLValue();
		}
	}

	JSRLValue JSRLValue::operator [] ( double d ) const
	{
		switch ( type )
		{
			case Type::undef:
				return JSRLValue();
			case Type::var:
				return _asVar().operator[]( d );
			case Type::value:
			{
				auto& v = *(_asValue());
				switch ( v.type )
				{
					case JSVarOrOwn::Type::undef:
						return JSRLValue();
					case JSVarOrOwn::Type::var:
						return v._asVar().operator[]( d );
					case JSVarOrOwn::Type::obj:
						return v._asPtr().operator[]( d );
					default:
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
						return JSRLValue();
				}
			}
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
				return JSRLValue();
		}
	}

	JSRLValue JSRLValue::operator [] ( int idx ) const
	{
		switch ( type )
		{
			case Type::undef:
				return JSRLValue();
			case Type::var:
				return _asVar().operator[]( idx );
			case Type::value:
			{
				auto& v = *(_asValue());
				switch ( v.type )
				{
					case JSVarOrOwn::Type::undef:
						return JSRLValue();
					case JSVarOrOwn::Type::var:
						return v._asVar().operator[]( idx );
					case JSVarOrOwn::Type::obj:
						return v._asPtr().operator[]( idx );
					default:
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
						return JSRLValue();
				}
			}
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
				return JSRLValue();
		}
	}

	JSRLValue JSRLValue::operator [] ( const nodecpp::string& key ) const
	{
		switch ( type )
		{
			case Type::undef:
				return JSRLValue();
			case Type::var:
				return _asVar().operator[]( key );
			case Type::value:
			{
				auto& v = *(_asValue());
				switch ( v.type )
				{
					case JSVarOrOwn::Type::undef:
						return JSRLValue();
					case JSVarOrOwn::Type::var:
						return v._asVar().operator[]( key );
					case JSVarOrOwn::Type::obj:
						return v._asPtr().operator[]( key );
					default:
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
						return JSRLValue();
				}
			}
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
				return JSRLValue();
		}
	}

	/*JSRLValue JSRLValue::operator [] ( const char* key )
	{
		nodecpp::string s( key );
		return JSRLValue::operator [] ( s );
	}*/

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

	double JSRLValue::toNumber() const
	{
		switch ( type )
		{
			case Type::undef:
				return NAN;
			case Type::var:
				return _asVar().toNumber();
			case Type::value:
				return _asValue()->toNumber();
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
				return NAN;
		}
	}

	double JSRLValue::length() const
	{ 
		if ( type == Type::value )
			return _asValue()->length();
		else
			throw; 
	}
	void JSRLValue::setLength( double ln )
	{ 
		if ( type == Type::value )
			_asValue()->setLength( ln );
		else
			throw; 
	}

	////////////////////////////////////////////////////////////   JSObject ////

	void JSObject::concat_impl_add_me( nodecpp::safememory::owning_ptr<JSArray>& ret )
	{
		nodecpp::safememory::soft_ptr<JSObject> myPtr = myThis.getSoftPtr<JSObject>(this);
		ret->arrayValues.push_back( JSVar( myPtr ) );
	}

	void JSObject::concat_impl_1( nodecpp::safememory::owning_ptr<JSArray>& ret, JSVar arr )
	{
		if ( arr.type == JSVar::Type::softptr )
			(*(arr._asSoft()))->concat_impl_add_me( ret );
		else
			ret->arrayValues.push_back( arr );
	}

	nodecpp::safememory::owning_ptr<JSArray> JSObject::keys()
	{
		nodecpp::safememory::owning_ptr<JSArray> ret = makeJSArray();
		for ( auto& elem : keyValuePairs )
			ret->arrayValues.push_back( JSVar( elem.first ) );
		return ret;
	}

	////////////////////////////////////////////////////////////   JSArray ////

	void JSArray::concat_impl_add_me( nodecpp::safememory::owning_ptr<JSArray>& ret )
	{
		for ( auto& el : arrayValues )
		{
			if ( el.type == JSVarOrOwn::Type::var )
				ret->arrayValues.push_back( el._asVar() );
			else
			{
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, el.type == JSVarOrOwn::Type::obj, "unexpected type: {}", (size_t)(el.type) ); 
				ret->arrayValues.push_back( JSVar( el._asPtr() ) );
			}
		}
	}

	////////////////////////////////////////////////////////////   JSMath ////

	JSVar JSMath::floor( JSVar var ) 
	{ 
		double num = var.toNumber(); 
		if ( !std::isnan( num ) )
			return std::floor( num );
		else
		{
			if ( var.type == JSVar::Type::softptr && *(var._asSoft()) == nullptr )
				return 0;
			return num;
		}
	}

	JSVar JSMath::random() 
	{ 
		return threadLocalData.rng.normalizedRnd();
	}

	////////////////////////////////////////////////////////////   UTF staff ////

	void utf8ToUc16( const char8_t* utfs, short* uc16buff, size_t uc16buffsz )
	{
		size_t idx = 0;
		size_t idxout = 0;
		size_t seq = 0;
		uint32_t code = 0;
		while ( utfs[idx] )
		{
			auto ch = utfs[idx++];
			if ( seq == 0 )
			{
				if ( (ch & 0x80) == 0 )
				{
					uc16buff[idxout++] = ch;
					code = 0;
				}
				else
				{
					if ( (ch & 0xe0) == 0xc0 )
					{
						seq = 1;
						code = ((uint32_t)(ch & 0x1f)) << 6;
					}
					else if ( (ch & 0xf0) == 0xe0 )
					{
						seq = 2;
						code = ((uint32_t)(ch & 0x0f)) << 12;
					}
					else if ( (ch & 0xf1) == 0xf0 )
					{
						seq = 3;
						code = ((uint32_t)(ch & 0x07)) << 18;
					}
			  }
			}
			else
			{
				code += ((uint32_t)(ch & 0x3f)) << ( --seq );
				if ( seq == 0 )
				{
					uc16buff[idxout++] = (short)(code);
					code = 0;
				}
			}
		}
		uc16buff[idxout] = 0;
	}

} // namespace nodecpp::js

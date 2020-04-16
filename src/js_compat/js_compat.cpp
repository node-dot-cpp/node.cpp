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
#include "nls.h"

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

	JSVar arguments() { return threadLocalData.currentArgs != nullptr ? threadLocalData.currentArgs : JSVar(); }

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

	void JSOwnObj::forEach( std::function<void(JSVar)> cb )
	{
		ptr->forEach( std::move( cb ) );
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

	JSRLValue JSVar::operator [] ( int idx ) const
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

	JSRLValue JSVar::operator [] ( double idx ) const
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

	JSRLValue JSVar::operator [] ( const nodecpp::string& key ) const
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

	JSVar JSVar::operator %( const JSVar& other ) const
	{
		// TODO: make sure we report right values!!!
		double me_ = toNumber();
		if ( me_ == NAN )
			return NAN;
		double other_ = other.toNumber();
		if ( other_ == NAN )
			return NAN;
		bool me_negative = me_ < 0;
		if ( me_negative )
			me_ = -me_;
		if ( other_ < 0 )
			other_ = - other_;
		double rat = me_ / other_;
		if ( rat == NAN )
			return NAN;
		uint64_t irat = (uint64_t)(rat);
		double rem = me_ - other_ * irat;
		if ( me_negative )
			rem = -rem;
		return rem;
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
				if ( end - _asStr()->c_str() == _asStr()->size() )
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

	void JSVar::forEach( std::function<void(JSVar)> cb )
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

} // namespace nodecpp::js
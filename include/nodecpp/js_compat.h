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
//#include <typeinfo>
//#include <typeindex>
#include "nls.h"

namespace nodecpp::js {

	class JSVar;
	class JSObject;
	class JSVarOrOwn;
	class JSArray;
	class JSRLValue;
	class JSInit;

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

		JSRLValue operator [] ( const JSVar& var );
		JSRLValue operator [] ( const JSRLValue& val );
		JSRLValue operator [] ( double idx );
		JSRLValue operator [] ( int idx );
		JSRLValue operator [] ( const nodecpp::string& key );
		JSRLValue operator [] ( const char* key );

		nodecpp::string toString() const;
		double toNumber() const;
		bool operator !() const { return ptr == nullptr; }
		operator nodecpp::string () const { return toString(); }

		bool has( const JSOwnObj& other ) const;
		bool has( size_t idx ) const;
		bool has( double num ) const;
		bool has( nodecpp::string str ) const;

		bool in( const JSOwnObj& collection ) const { return collection.has( *this ); }
	
		void forEach( std::function<void(JSVar)> cb );

		template<class ArrT1, class ... ArrTX>
		JSOwnObj concat( ArrT1 arr1, ArrTX ... args );

		double length() const;
		void setLength( double ln ) ;
	};

	class JSVarBase
	{
	protected:
		friend class JSVar;
		friend class JSObject;
		friend class JSArray;
		friend class JSRLValue;
		friend nodecpp::string typeOf( const JSVarBase& );
		friend nodecpp::string typeOf( const JSVarOrOwn& );
		friend nodecpp::string typeOf( const JSRLValue& );

		using Fn0T = std::function<JSVar()>;
		using Fn1T = std::function<JSVar( JSVar )>;
		using Fn2T = std::function<JSVar( JSVar, JSVar )>;
		using Fn3T = std::function<JSVar( JSVar, JSVar, JSVar )>;
		using Fn4T = std::function<JSVar( JSVar, JSVar, JSVar, JSVar )>;
		using Fn5T = std::function<JSVar( JSVar, JSVar, JSVar, JSVar, JSVar )>;
		using Fn6T = std::function<JSVar( JSVar, JSVar, JSVar, JSVar, JSVar, JSVar )>;
		using Fn7T = std::function<JSVar( JSVar, JSVar, JSVar, JSVar, JSVar, JSVar, JSVar )>;
		using Fn8T = std::function<JSVar( JSVar, JSVar, JSVar, JSVar, JSVar, JSVar, JSVar, JSVar )>;
		using Fn9T = std::function<JSVar( JSVar, JSVar, JSVar, JSVar, JSVar, JSVar, JSVar, JSVar, JSVar )>;
		using Fn10T = std::function<JSVar( JSVar, JSVar, JSVar, JSVar, JSVar, JSVar, JSVar, JSVar, JSVar, JSVar )>;

		struct Fn0Struct { Fn0T fn; };
		struct Fn1Struct { Fn1T fn; };
		struct Fn2Struct { Fn2T fn; };
		struct Fn3Struct { Fn3T fn; };
		struct Fn4Struct { Fn4T fn; };
		struct Fn5Struct { Fn5T fn; };
		struct Fn6Struct { Fn6T fn; };
		struct Fn7Struct { Fn7T fn; };
		struct Fn8Struct { Fn8T fn; };
		struct Fn9Struct { Fn9T fn; };
		struct Fn10Struct { Fn10T fn; };

		static constexpr size_t fnsz = sizeof( Fn0Struct );
		static_assert( sizeof( Fn1Struct ) == fnsz );
		static_assert( sizeof( Fn2Struct ) == fnsz );
		static_assert( sizeof( Fn3Struct ) == fnsz );
		static_assert( sizeof( Fn4Struct ) == fnsz );
		static_assert( sizeof( Fn5Struct ) == fnsz );
		static_assert( sizeof( Fn6Struct ) == fnsz );
		static_assert( sizeof( Fn7Struct ) == fnsz );
		static_assert( sizeof( Fn8Struct ) == fnsz );
		static_assert( sizeof( Fn9Struct ) == fnsz );
		static_assert( sizeof( Fn10Struct ) == fnsz );

		enum Type { undef, boolean, num, string, softptr, fn0, fn1, fn2, fn3, fn4, fn5, fn6, fn7, fn8, fn9, fn10 };
		Type type = Type::undef;
		static constexpr size_t memsz = fnsz > (sizeof( nodecpp::string ) > 16 ? sizeof( nodecpp::string ) : 16) ? fnsz : (sizeof( nodecpp::string ) > 16 ? sizeof( nodecpp::string ) : 16);
		uintptr_t basemem[memsz/sizeof(uintptr_t)]; // note: we just cause it to be uintptr_t-aligned

		static constexpr size_t fnSize = sizeof( std::function<double(double)> );
		static_assert( fnSize == 64 );

		using softptr2jsobj = nodecpp::safememory::soft_ptr<JSObject>;
		using softptr2jsarr = nodecpp::safememory::soft_ptr<JSArray>;

		bool* _asBool() { return reinterpret_cast<bool*>( basemem ); }
		double* _asNum() { return reinterpret_cast<double*>( basemem ); }
		nodecpp::string* _asStr() { return reinterpret_cast<nodecpp::string*>( basemem ); }
		softptr2jsobj* _asSoft() { return reinterpret_cast<softptr2jsobj*>( basemem ); }
		Fn0Struct* _asFn0() { return reinterpret_cast<Fn0Struct*>( basemem ); }
		Fn1Struct* _asFn1() { return reinterpret_cast<Fn1Struct*>( basemem ); }
		Fn2Struct* _asFn2() { return reinterpret_cast<Fn2Struct*>( basemem ); }
		Fn3Struct* _asFn3() { return reinterpret_cast<Fn3Struct*>( basemem ); }
		Fn4Struct* _asFn4() { return reinterpret_cast<Fn4Struct*>( basemem ); }
		Fn5Struct* _asFn5() { return reinterpret_cast<Fn5Struct*>( basemem ); }
		Fn6Struct* _asFn6() { return reinterpret_cast<Fn6Struct*>( basemem ); }
		Fn7Struct* _asFn7() { return reinterpret_cast<Fn7Struct*>( basemem ); }
		Fn8Struct* _asFn8() { return reinterpret_cast<Fn8Struct*>( basemem ); }
		Fn9Struct* _asFn9() { return reinterpret_cast<Fn9Struct*>( basemem ); }
		Fn10Struct* _asFn10() { return reinterpret_cast<Fn10Struct*>( basemem ); }

		const bool* _asBool() const { return reinterpret_cast<const bool*>( basemem ); }
		const double* _asNum() const { return reinterpret_cast<const double*>( basemem ); }
		const nodecpp::string* _asStr() const { return reinterpret_cast<const nodecpp::string*>( basemem ); }
		const softptr2jsobj* _asSoft() const { return reinterpret_cast<const softptr2jsobj*>( basemem ); }
		const Fn0Struct* _asFn0() const { return reinterpret_cast<const Fn0Struct*>( basemem ); }
		const Fn1Struct* _asFn1() const { return reinterpret_cast<const Fn1Struct*>( basemem ); }
		const Fn2Struct* _asFn2() const { return reinterpret_cast<const Fn2Struct*>( basemem ); }
		const Fn3Struct* _asFn3() const { return reinterpret_cast<const Fn3Struct*>( basemem ); }
		const Fn4Struct* _asFn4() const { return reinterpret_cast<const Fn4Struct*>( basemem ); }
		const Fn5Struct* _asFn5() const { return reinterpret_cast<const Fn5Struct*>( basemem ); }
		const Fn6Struct* _asFn6() const { return reinterpret_cast<const Fn6Struct*>( basemem ); }
		const Fn7Struct* _asFn7() const { return reinterpret_cast<const Fn7Struct*>( basemem ); }
		const Fn8Struct* _asFn8() const { return reinterpret_cast<const Fn8Struct*>( basemem ); }
		const Fn9Struct* _asFn9() const { return reinterpret_cast<const Fn9Struct*>( basemem ); }
		const Fn10Struct* _asFn10() const { return reinterpret_cast<const Fn10Struct*>( basemem ); }

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
				case fn0:
					_asFn0()->~Fn0Struct();
					break;
				case fn1:
					_asFn1()->~Fn1Struct();
					break;
				case fn2:
					_asFn2()->~Fn2Struct();
					break;
				case fn3:
					_asFn3()->~Fn3Struct();
					break;
				case fn4:
					_asFn4()->~Fn4Struct();
					break;
				case fn5:
					_asFn5()->~Fn5Struct();
					break;
				case fn6:
					_asFn6()->~Fn6Struct();
					break;
				case fn7:
					_asFn7()->~Fn7Struct();
					break;
				case fn8:
					_asFn8()->~Fn8Struct();
					break;
				case fn9:
					_asFn9()->~Fn9Struct();
					break;
				case fn10:
					_asFn10()->~Fn10Struct();
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
		void init( Fn0T&& cb )
		{
			if ( type == Type::fn0 )
				_asFn0()->fn = std::move( cb );
			else
			{
				deinit();
				type = Type::fn0;
				new(_asFn0())Fn0Struct();
				_asFn0()->fn = std::move( cb );
			}
		}
		void init( Fn1T&& cb )
		{
			if ( type == Type::fn1 )
				_asFn1()->fn = std::move( cb );
			else
			{
				deinit();
				type = Type::fn1;
				new(_asFn1())Fn0Struct();
				_asFn1()->fn = std::move( cb );
			}
		}
		void init( Fn2T&& cb )
		{
			if ( type == Type::fn2 )
				_asFn2()->fn = std::move( cb );
			else
			{
				deinit();
				type = Type::fn2;
				new(_asFn2())Fn0Struct();
				_asFn2()->fn = std::move( cb );
			}
		}
		void init( Fn3T&& cb )
		{
			if ( type == Type::fn3 )
				_asFn3()->fn = std::move( cb );
			else
			{
				deinit();
				type = Type::fn3;
				new(_asFn3())Fn0Struct();
				_asFn3()->fn = std::move( cb );
			}
		}
		void init( Fn4T&& cb )
		{
			if ( type == Type::fn4 )
				_asFn4()->fn = std::move( cb );
			else
			{
				deinit();
				type = Type::fn4;
				new(_asFn4())Fn0Struct();
				_asFn4()->fn = std::move( cb );
			}
		}
		void init( Fn5T&& cb )
		{
			if ( type == Type::fn5 )
				_asFn5()->fn = std::move( cb );
			else
			{
				deinit();
				type = Type::fn5;
				new(_asFn5())Fn0Struct();
				_asFn5()->fn = std::move( cb );
			}
		}
		void init( Fn6T&& cb )
		{
			if ( type == Type::fn6 )
				_asFn6()->fn = std::move( cb );
			else
			{
				deinit();
				type = Type::fn6;
				new(_asFn6())Fn0Struct();
				_asFn6()->fn = std::move( cb );
			}
		}
		void init( Fn7T&& cb )
		{
			if ( type == Type::fn7 )
				_asFn7()->fn = std::move( cb );
			else
			{
				deinit();
				type = Type::fn7;
				new(_asFn7())Fn0Struct();
				_asFn7()->fn = std::move( cb );
			}
		}
		void init( Fn8T&& cb )
		{
			if ( type == Type::fn8 )
				_asFn8()->fn = std::move( cb );
			else
			{
				deinit();
				type = Type::fn8;
				new(_asFn8())Fn0Struct();
				_asFn8()->fn = std::move( cb );
			}
		}
		void init( Fn9T&& cb )
		{
			if ( type == Type::fn9 )
				_asFn9()->fn = std::move( cb );
			else
			{
				deinit();
				type = Type::fn9;
				new(_asFn9())Fn0Struct();
				_asFn9()->fn = std::move( cb );
			}
		}
		void init( Fn10T&& cb )
		{
			if ( type == Type::fn10 )
				_asFn10()->fn = std::move( cb );
			else
			{
				deinit();
				type = Type::fn10;
				new(_asFn10())Fn0Struct();
				_asFn10()->fn = std::move( cb );
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
					case Type::fn0:
						_asFn0()->fn = other._asFn0()->fn;
						break;
					case Type::fn1:
						_asFn1()->fn = other._asFn1()->fn;
						break;
					case Type::fn2:
						_asFn2()->fn = other._asFn2()->fn;
						break;
					case Type::fn3:
						_asFn3()->fn = other._asFn3()->fn;
						break;
					case Type::fn4:
						_asFn4()->fn = other._asFn4()->fn;
						break;
					case Type::fn5:
						_asFn5()->fn = other._asFn5()->fn;
						break;
					case Type::fn6:
						_asFn6()->fn = other._asFn6()->fn;
						break;
					case Type::fn7:
						_asFn7()->fn = other._asFn7()->fn;
						break;
					case Type::fn8:
						_asFn8()->fn = other._asFn8()->fn;
						break;
					case Type::fn9:
						_asFn9()->fn = other._asFn9()->fn;
						break;
					case Type::fn10:
						_asFn10()->fn = other._asFn10()->fn;
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
					case Type::fn0:
						new(_asFn0())Fn0Struct();
						_asFn0()->fn = other._asFn0()->fn;
						break;
					case Type::fn1:
						new(_asFn1())Fn0Struct();
						_asFn1()->fn = other._asFn1()->fn;
						break;
					case Type::fn2:
						new(_asFn2())Fn0Struct();
						_asFn2()->fn = other._asFn2()->fn;
						break;
					case Type::fn3:
						new(_asFn3())Fn0Struct();
						_asFn3()->fn = other._asFn3()->fn;
						break;
					case Type::fn4:
						new(_asFn4())Fn0Struct();
						_asFn4()->fn = other._asFn4()->fn;
						break;
					case Type::fn5:
						new(_asFn5())Fn0Struct();
						_asFn5()->fn = other._asFn5()->fn;
						break;
					case Type::fn6:
						new(_asFn6())Fn0Struct();
						_asFn6()->fn = other._asFn6()->fn;
						break;
					case Type::fn7:
						new(_asFn7())Fn0Struct();
						_asFn7()->fn = other._asFn7()->fn;
						break;
					case Type::fn8:
						new(_asFn8())Fn0Struct();
						_asFn8()->fn = other._asFn8()->fn;
						break;
					case Type::fn9:
						new(_asFn9())Fn0Struct();
						_asFn9()->fn = other._asFn9()->fn;
						break;
					case Type::fn10:
						new(_asFn10())Fn0Struct();
						_asFn10()->fn = other._asFn10()->fn;
						break;
					default:
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
				}
			}
		}

	};

	class JSRLValue
	{
		friend class JSVar;
		friend class JSObject;
		friend class JSArray;
		friend nodecpp::string typeOf( const JSRLValue& );

		enum Type { undef, value, var };
		Type type = Type::undef;
		using OwnedT = JSVarOrOwn*;
		static constexpr size_t memsz = sizeof( OwnedT ) > sizeof( JSVarBase ) ? sizeof( OwnedT ) : sizeof( JSVarBase );
		uintptr_t basemem[ memsz / sizeof( uintptr_t ) ];
		JSVar& _asVar() { return *reinterpret_cast<JSVar*>( basemem ); }
		OwnedT& _asValue() { return *reinterpret_cast<OwnedT*>( basemem ); }
		const JSVar& _asVar() const { return *reinterpret_cast<const JSVar*>( basemem ); }
		const OwnedT& _asValue() const { return *reinterpret_cast<const OwnedT*>( basemem ); }

		JSRLValue() {}
		JSRLValue( JSVarOrOwn& v ) {
			_asValue() = &v;
			type = Type::value;
		}
		JSRLValue( const JSVar& var );
		JSRLValue& operator = ( JSVarOrOwn& obj );

	public:
		JSRLValue( const JSRLValue& other);
		JSRLValue& operator = ( const JSRLValue& other );
		JSRLValue& operator = ( const JSVar& var );
//		JSRLValue& operator = ( const JSOwnObj& obj );
		JSRLValue& operator = ( JSOwnObj&& obj );

		~JSRLValue();

		JSRLValue operator [] ( const JSVar& var );
		JSRLValue operator [] ( const JSRLValue& val );
		JSRLValue operator [] ( double idx );
		JSRLValue operator [] ( int idx );
		JSRLValue operator [] ( const nodecpp::string& key );
		JSRLValue operator [] ( const char* key );

		JSVar operator()();
		JSVar operator()( JSVar obj );
		JSVar operator()( JSVar obj1, JSVar obj2 );
		JSVar operator()( JSVar obj1, JSVar obj2, JSVar obj3 );
		JSVar operator()( JSVar obj1, JSVar obj2, JSVar obj3, JSVar obj4 );
		JSVar operator()( JSVar obj1, JSVar obj2, JSVar obj3, JSVar obj4, JSVar obj5 );
		JSVar operator()( JSVar obj1, JSVar obj2, JSVar obj3, JSVar obj4, JSVar obj5, JSVar obj6 );
		JSVar operator()( JSVar obj1, JSVar obj2, JSVar obj3, JSVar obj4, JSVar obj5, JSVar obj6, JSVar obj7 );
		JSVar operator()( JSVar obj1, JSVar obj2, JSVar obj3, JSVar obj4, JSVar obj5, JSVar obj6, JSVar obj7, JSVar obj8 );
		JSVar operator()( JSVar obj1, JSVar obj2, JSVar obj3, JSVar obj4, JSVar obj5, JSVar obj6, JSVar obj7, JSVar obj8, JSVar obj9 );
		JSVar operator()( JSVar obj1, JSVar obj2, JSVar obj3, JSVar obj4, JSVar obj5, JSVar obj6, JSVar obj7, JSVar obj8, JSVar obj9, JSVar obj10 );

		bool operator !() const;
		operator nodecpp::string () const { return toString(); }
		operator JSVar () const;
		nodecpp::string toString() const;
		double toNumber() const;

		template<class ArrT1, class ... ArrTX>
		JSOwnObj concat( ArrT1 arr1, ArrTX ... args );

		double length() const;
		void setLength( double ln ) ;
	};

	class JSRegExp
	{
	public:
		enum Flags { none = 0, g = 0x1, i=0x2 }; // TODO: subject for further revision (consider using std:: -related names and values)

	private:
		nodecpp::string regex;
		uint64_t flags_ = Flags::none;

	public:
		JSRegExp() {}
		JSRegExp( nodecpp::string regex_, nodecpp::string flags = "" ) : regex( regex_ )
		{
			for ( size_t i=0; i<flags.size(); ++i )
				switch ( flags[i] )
				{
					case 'g': flags_ |= Flags::g; break;
					case 'i': flags_ |= Flags::g; break;
					default:
						throw;
				}
		}
		const nodecpp::string& re() const { return regex; }
		uint64_t flags() const { return flags_; }
	};

	class JSVar : protected JSVarBase
	{
		friend class JSObject;
		friend class JSArray;
		friend nodecpp::string typeOf( const JSVarBase& );
		friend nodecpp::string typeOf( const JSVarOrOwn& );
		friend nodecpp::string typeOf( const JSRLValue& );
		friend class JSMath;

		void init( const JSVar& other ) { JSVarBase::init( other ); }

		bool isFalseValue() const;

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
		JSVar( Fn0T&& cb ) { JSVarBase::init( std::move( cb ) ); }
		JSVar( Fn1T&& cb ) { JSVarBase::init( std::move( cb ) ); }
		JSVar( Fn2T&& cb ) { JSVarBase::init( std::move( cb ) ); }
		JSVar( Fn3T&& cb ) { JSVarBase::init( std::move( cb ) ); }
		JSVar( Fn4T&& cb ) { JSVarBase::init( std::move( cb ) ); }
		JSVar( Fn5T&& cb ) { JSVarBase::init( std::move( cb ) ); }
		JSVar( Fn6T&& cb ) { JSVarBase::init( std::move( cb ) ); }
		JSVar( Fn7T&& cb ) { JSVarBase::init( std::move( cb ) ); }
		JSVar( Fn8T&& cb ) { JSVarBase::init( std::move( cb ) ); }
		JSVar( Fn9T&& cb ) { JSVarBase::init( std::move( cb ) ); }
		JSVar( Fn10T&& cb ) { JSVarBase::init( std::move( cb ) ); }

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
		JSVar& operator = ( Fn0T&& cb ) { JSVarBase::init( std::move( cb ) ); return *this; }
		JSVar& operator = ( Fn1T&& cb ) { JSVarBase::init( std::move( cb ) ); return *this; }
		JSVar& operator = ( Fn2T&& cb ) { JSVarBase::init( std::move( cb ) ); return *this; }
		JSVar& operator = ( Fn3T&& cb ) { JSVarBase::init( std::move( cb ) ); return *this; }
		JSVar& operator = ( Fn4T&& cb ) { JSVarBase::init( std::move( cb ) ); return *this; }
		JSVar& operator = ( Fn5T&& cb ) { JSVarBase::init( std::move( cb ) ); return *this; }
		JSVar& operator = ( Fn6T&& cb ) { JSVarBase::init( std::move( cb ) ); return *this; }
		JSVar& operator = ( Fn7T&& cb ) { JSVarBase::init( std::move( cb ) ); return *this; }
		JSVar& operator = ( Fn8T&& cb ) { JSVarBase::init( std::move( cb ) ); return *this; }
		JSVar& operator = ( Fn9T&& cb ) { JSVarBase::init( std::move( cb ) ); return *this; }
		JSVar& operator = ( Fn10T&& cb ) { JSVarBase::init( std::move( cb ) ); return *this; }

		JSRLValue operator [] ( const JSVar& var ) const;
		JSRLValue operator [] ( const JSRLValue& val ) const;
		JSRLValue operator [] ( double num ) const;
		JSRLValue operator [] ( int num ) const;
		JSRLValue operator [] ( const nodecpp::string& key ) const;
		JSRLValue operator [] ( const char* key ) const { nodecpp::string s(key); return operator [] (s); }

		JSVar operator()();
		JSVar operator()( JSVar obj );
		JSVar operator()( JSVar obj1, JSVar obj2 );
		JSVar operator()( JSVar obj1, JSVar obj2, JSVar obj3 );
		JSVar operator()( JSVar obj1, JSVar obj2, JSVar obj3, JSVar obj4 );
		JSVar operator()( JSVar obj1, JSVar obj2, JSVar obj3, JSVar obj4, JSVar obj5 );
		JSVar operator()( JSVar obj1, JSVar obj2, JSVar obj3, JSVar obj4, JSVar obj5, JSVar obj6 );
		JSVar operator()( JSVar obj1, JSVar obj2, JSVar obj3, JSVar obj4, JSVar obj5, JSVar obj6, JSVar obj7 );
		JSVar operator()( JSVar obj1, JSVar obj2, JSVar obj3, JSVar obj4, JSVar obj5, JSVar obj6, JSVar obj7, JSVar obj8 );
		JSVar operator()( JSVar obj1, JSVar obj2, JSVar obj3, JSVar obj4, JSVar obj5, JSVar obj6, JSVar obj7, JSVar obj8, JSVar obj9 );
		JSVar operator()( JSVar obj1, JSVar obj2, JSVar obj3, JSVar obj4, JSVar obj5, JSVar obj6, JSVar obj7, JSVar obj8, JSVar obj9, JSVar obj10 );

		nodecpp::string toString() const;
		double toNumber() const;

		operator nodecpp::string () const { return toString(); }
		bool operator !() const;
		JSVar operator %( const JSVar& other ) const;
		JSVar operator ||( const JSVar& other ) const;
		JSVar operator &&( const JSVar& other ) const;

		bool operator ==( const JSVar& other ) const;
		bool operator !=( const JSVar& other ) const { return !operator == ( other ); }
		bool isStrictlyTheSame( const JSVar& other ) const;
		bool isNotStrictlyTheSame( const JSVar& other ) const { return !isStrictlyTheSame( other ); }

		JSVar operator += (const JSVar& other );
		JSVar operator + (const JSVar& other ) const;

		JSVar operator *= (const JSVar& other );
		JSVar operator * (const JSVar& other ) const;

		JSVar operator++(); // prefix
		JSVar operator++(int); // postfix
		JSVar operator--(); // prefix
		JSVar operator--(int); // postfix

		bool has( const JSVar& other ) const;
		bool has( size_t idx ) const;
		bool has( double num ) const;
		bool has( nodecpp::string str ) const;

		bool in( const JSVar& collection ) const { return collection.has( *this ); }
		void forEach( std::function<void(JSVar)> cb );

		JSOwnObj split(  const JSVar& separator, JSVar maxCount ) const; // TODO: implement!
		JSOwnObj split( const JSVar& separator ) const { return split( separator, INT32_MAX ); }
		JSOwnObj split() const;

		template<class ArrT1, class ... ArrTX>
		JSOwnObj concat( ArrT1 arr1, ArrTX ... args );

		double length() const;
		void setLength( double ln ) ;

		JSVar toLowerCase() const;
		JSVar toUpperCase() const;

		JSOwnObj match( const JSRegExp& re ) const;
		JSVar replace( const JSRegExp& re, const JSVar replacement ) const;
	};
	static_assert( sizeof(JSVarBase) == sizeof(JSVar), "no data memebers at JSVar itself!" );

	JSVar arguments();

	inline bool jsIn( const JSVar& var, const JSVar& collection )  { return collection.has( var ); }
	inline bool jsIn( size_t idx, const JSVar& collection )  { return collection.has( idx ); }
	inline bool jsIn( nodecpp::string str, const JSVar& collection )  { return collection.has( str ); }


	template<class ArrT1, class ... ArrTX>
	JSOwnObj JSRLValue::concat( ArrT1 arr1, ArrTX ... args )
	{
		if ( type == Type::var )
			return _asVar().concat( arr1, args ... );
		else 
		{
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, type == Type::value, "unexpected type: {}", (size_t)type ); 
			return _asValue()->concat( arr1, args ... );
		}
	}

	class JSMath
	{
	public:
		static JSVar floor( JSVar var );
		static JSVar random();
	};

	class JSInit
	{
		friend class JSRLValue;

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

		~JSInit() {
			if ( type == Type::var )
				_asVar().~JSVar();
			else if ( type == Type::obj )
				_asPtr().~JSOwnObj();
		}

		JSVarOrOwn toValue() const;
	};

	class JSVarOrOwn
	{
		friend class JSRLValue;
		friend class JSInit;
		friend class JSArray;
		friend nodecpp::string typeOf( const JSVarOrOwn& );

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
		JSVarOrOwn() {}
		JSVarOrOwn( JSVarOrOwn&& other);
		JSVarOrOwn( const JSVarOrOwn& other) = delete;
		JSVarOrOwn( JSOwnObj&& obj ) {
			new(&(_asPtr()))OwnedT( std::move( obj ) );
			type = Type::obj;
		}
		JSVarOrOwn( const JSVar& var ) {
			new(&(_asVar()))JSVar( var );
			type = Type::var;
		}
		JSVarOrOwn& operator = ( JSVarOrOwn&& other );
		JSVarOrOwn& operator = ( const JSVarOrOwn& other ) = delete;

		~JSVarOrOwn() {
			if ( type == Type::var )
				_asVar().~JSVar();
			else if ( type == Type::obj )
				_asPtr().~JSOwnObj();
		}

		bool operator !() const;
		nodecpp::string toString() const;
		double toNumber() const;

		template<class ArrT1, class ... ArrTX>
		JSOwnObj concat( ArrT1 arr1, ArrTX ... args )
		{
			if ( type == Type::var )
				return _asVar().concat( arr1, args ... );
			else
			{
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, type == Type::obj, "unexpected type: {}", (size_t)type ); 
				return _asPtr().concat( arr1, args ... );
			}
		}

		double length() const;
		void setLength( double ln ) ;
	};

	class JSObject
	{
		friend class JSOwnObj;
		friend class JSArray;

	protected:
		nodecpp::safememory::soft_this_ptr<JSObject> myThis;
		nodecpp::map<nodecpp::string, JSVarOrOwn> pairs;

		virtual void concat_impl_add_me( nodecpp::safememory::owning_ptr<JSArray>& ret );

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

		JSRLValue findOrAdd ( nodecpp::string s ) {
			auto f = pairs.find( s );
			if ( f != pairs.end() )
			{
				return f->second;
			}
			else
			{
				auto insret = pairs.insert( std::make_pair( s, JSVarOrOwn() ) );
				return insret.first->second;
			}
		}

		void concat_impl_1( nodecpp::safememory::owning_ptr<JSArray>& ret, JSVar arr );

	public:
		JSObject() {}
		JSObject(std::initializer_list<std::pair<nodecpp::string, JSInit>> l)
		{
			for ( auto& p : l )
				pairs.insert( make_pair( p.first, std::move( p.second.toValue() ) ) );
		}
	public:
		virtual ~JSObject() {}
		virtual JSRLValue operator [] ( const JSVar& var )
		{
			nodecpp::string s = var.toString(); // TODO: revise implementation!!!
			return findOrAdd( s );
		}
		virtual JSRLValue operator [] ( const JSRLValue& val )
		{
			if ( val.type != JSRLValue::Type::var )
				return JSVar(); // TODO: check if we should throw or convert to string, etc, instead
			return operator []( val._asVar() );
		}
		virtual JSRLValue operator [] ( size_t idx )
		{
			nodecpp::string s = nodecpp::format( "{}", idx );
			return findOrAdd( s );
		}
		virtual JSRLValue operator [] ( const nodecpp::string& key )
		{
			return findOrAdd( key );
		}
		virtual JSRLValue operator [] ( const nodecpp::string_literal& key ) {
			nodecpp::string s( key.c_str() );
			return findOrAdd( s );
		}
		virtual JSRLValue operator [] ( const char* key ) {
			nodecpp::string s( key );
			return findOrAdd( s );
		}
		double toNumber() const { return NAN; }
		virtual nodecpp::string toString() const { 
			nodecpp::string ret = "{ \n";
			toString_( ret, "  ", ",\n" );
			ret += " }";
			return ret; 
		}
		virtual void forEach( std::function<void(JSVar)> cb )
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

		template<class ArrT, class ArrT1, class ... ArrTX>
		void concat_impl( nodecpp::safememory::owning_ptr<JSArray>& ret, ArrT arr, ArrT1 arr1, ArrTX ... args)
		{
			concat_impl_1( ret, arr );
			concat_impl( ret, arr1, args ... );
		}

		template<class ArrT>
		void concat_impl( nodecpp::safememory::owning_ptr<JSArray>& ret, ArrT arr )
		{
			concat_impl_1( ret, arr );
		}

		template<class ArrT1, class ... ArrTX>
		JSOwnObj concat( ArrT1 arr1, ArrTX ... args );

		virtual double length() const { throw; }
		virtual void setLength( double ln ) { throw; }
	};
	inline owning_ptr<JSObject> makeJSObject() { return make_owning<JSObject>(); }
	inline  owning_ptr<JSObject> makeJSObject(std::initializer_list<std::pair<nodecpp::string, JSInit>> l) { return make_owning<JSObject>(l); }

	class JSArray : public JSObject
	{
		friend class JSOwnObj;
		friend class JSObject;
		friend class JSVar; // for quick fill in split()

		nodecpp::vector<JSVarOrOwn> elems;

		virtual void concat_impl_add_me( nodecpp::safememory::owning_ptr<JSArray>& ret );

	public:
		JSArray() {}
		JSArray(std::initializer_list<JSInit> l)
		{
			for ( auto& p : l )
				elems.push_back( p.toValue() );
		}
	public:
		virtual JSRLValue operator [] ( const JSVar& var )
		{
			// TODO: revise implementation!!!
			if ( var.type == JSVarBase::Type::num )
				return operator [] ( *(var._asNum()) );
			nodecpp::string s = var.toString(); 
			return findOrAdd( s );
		}
		virtual JSRLValue operator [] ( const JSRLValue& val )
		{
			if ( val.type != JSRLValue::Type::var )
				return JSVar(); // TODO: check if we should throw or convert to string, etc, instead
			return operator []( val._asVar() );
		}
		virtual JSRLValue operator [] ( size_t idx )
		{
			if ( idx < elems.size() )
				return elems[ idx ];
			else
			{
				elems.reserve( idx + 1 );
				for ( size_t i=elems.size(); i<elems.capacity(); ++i )
					elems.push_back( JSVarOrOwn() );
				return elems[ idx ];
			}
		}
		virtual JSRLValue operator [] ( const nodecpp::string& strIdx )
		{
			if ( strIdx.size() && strIdx[0] >= '0' && strIdx[0] <= '9' )
			{
				char* end;
				size_t idx = strtol( strIdx.c_str(), &end, 10 ); // TODO: for floating ?
				if ( end - strIdx.c_str() == strIdx.size() )
					return operator [] ( idx );
			}
			return JSObject::operator[](strIdx);
		}
		virtual void forEach( std::function<void(JSVar)> cb )
		{
			for ( size_t idx = 0; idx<elems.size(); ++idx )
				cb( JSVar( (double)idx ) );
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
		virtual double length() const { return elems.size(); }
		virtual void setLength( double ln )
		{
			if ( ln >= 0 && ln <= UINT32_MAX )
			{
				size_t iln = (size_t)ln;
				if ( ln < elems.size() )
					elems.erase( elems.begin() + iln, elems.end() );
				else 
					for ( size_t n = elems.size(); n < iln; ++n )
						elems.push_back( JSVar() );
			}
			else
				throw;
		}
	};
	inline owning_ptr<JSArray> makeJSArray() { return make_owning<JSArray>(); }
	inline owning_ptr<JSArray> makeJSArray(std::initializer_list<JSInit> l) { return make_owning<JSArray>(l); } // TODO: ownership of args

	template<class ArrT1, class ... ArrTX>
	JSOwnObj JSOwnObj::concat( ArrT1 arr1, ArrTX ... args ) 
	{
		return ptr->concat( arr1, args ... );
	}

	template<class ArrT1, class ... ArrTX>
	JSOwnObj JSVar::concat( ArrT1 arr1, ArrTX ... args )
	{
		switch ( type )
		{
			case Type::softptr:
				return (*_asSoft())->concat( arr1, args ... );
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
				return makeJSArray();
		}
	}

	template<class ArrT1, class ... ArrTX>
	JSOwnObj JSObject::concat( ArrT1 arr1, ArrTX ... args )
	{
		auto ret = makeJSArray();
		concat_impl_add_me( ret );
		concat_impl( ret, arr1, args ... );
		return ret;
	}

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
				return "function";
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)(var.type) ); 
				return "undefined";
		}
	}

	inline
	nodecpp::string typeOf( const JSOwnObj& obj )
	{
		return "object";
	}

	inline
	nodecpp::string typeOf( const JSVarOrOwn& val )
	{
		switch ( val.type )
		{
			case JSVarOrOwn::Type::undef:
				return "undefined";
			case JSVarOrOwn::Type::obj:
				return typeOf( val._asPtr() );
			case JSVarOrOwn::Type::var:
				return typeOf( val._asVar() );
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)(val.type) ); 
				return "undefined";
		}
	}

	inline
	nodecpp::string typeOf( const JSRLValue& val )
	{
		switch ( val.type )
		{
			case JSRLValue::Type::undef:
				return "undefined";
			case JSRLValue::Type::value:
				return typeOf( *(val._asValue()) );
			case JSRLValue::Type::var:
				return typeOf( val._asVar() );
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)(val.type) ); 
				return "undefined";
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////

	template<class T>
	JSVar require()
	{
		auto trial = nodecpp::threadLocalData.jsModuleMap.getJsModuleExported<T>();
		if ( trial.first )
			return trial.second->exports();
		owning_ptr<JSModule> pt = nodecpp::safememory::make_owning<T>();
		auto ret = nodecpp::threadLocalData.jsModuleMap.addJsModuleExported<T>( std::move( pt ) );
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, ret.first ); 
		return ret.second->exports();
	}

	template<class T>
	T& import()
	{
		auto trial = nodecpp::threadLocalData.jsModuleMap.getJsModuleExported<T>();
		if ( trial.first )
			return *(trial.second);
		owning_ptr<JSModule> pt = nodecpp::safememory::make_owning<T>();
		auto ret = nodecpp::threadLocalData.jsModuleMap.addJsModuleExported<T>( std::move( pt ) );
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, ret.first ); 
		return *(ret.second);
	}

} //namespace nodecpp::js

#endif // NODECPP_JS_COMPAT_H

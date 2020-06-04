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
#include "nls.h"

#ifdef assert
#undef assert
#endif // assert
namespace nodecpp::js {

	class JSString;
	class JSOwnObj;
	class JSVar;
	class JSObject;
	class JSVarOrOwn;
	class JSArray;
	class JSRLValue;
	class JSInit;

	class JSChar
	{
		friend class JSString;
		uint32_t code = 0;
	public:
	};

	class JSString
	{
		nodecpp::string str_;

		static const char* utf8ToJSChar( const char* utfs, JSChar& jschar )
		{
			jschar.code = 0;
			size_t seq = 0;
			uint32_t code = 0;
			while ( *utfs )
			{
				auto ch = *utfs++;
				if ( seq == 0 )
				{
					if ( (ch & 0x80) == 0 )
					{
						jschar.code = ch;
						break;
					}
					else
					{
						if ( (ch & 0xe0) == 0xc0 )
						{
							seq = 1;
							jschar.code = ((uint32_t)(ch & 0x1f)) << 6;
						}
						else if ( (ch & 0xf0) == 0xe0 )
						{
							seq = 2;
							jschar.code = ((uint32_t)(ch & 0x0f)) << 12;
						}
						else if ( (ch & 0xf1) == 0xf0 )
						{
							seq = 3;
							jschar.code = ((uint32_t)(ch & 0x07)) << 18;
						}
					}
				}
				else
				{
					code += ((uint32_t)(ch & 0x3f)) << ( --seq );
					if ( seq == 0 )
						break;
				}
			}
			return utfs;
		}

		static char* utf8FromJSChar( JSChar jschar, char* utfs )
		{
			if ( jschar.code <= 0x7F )
			{
				if ( jschar.code > 0 )
				{
					*utfs = (char)(jschar.code);
					return utfs + 1;
				}
				else
				{
					*utfs = 0;
					return utfs;
				}
			}
			else if ( jschar.code <= 0x7FF )
			{
				utfs[0] = 0xC0 | ((char)(jschar.code >> 5) & 0x1F );
				utfs[1] = 0x80 | ((char)(jschar.code) & 0x3F );
				return utfs + 2;
			}
			else if ( jschar.code <= 0xFFFF )
			{
				utfs[0] = 0xE0 | ((char)(jschar.code >> 11) & 0x1F );
				utfs[1] = 0x80 | ((char)(jschar.code >> 6) & 0x3F );
				utfs[2] = 0x80 | ((char)(jschar.code) & 0x3F );
				return utfs + 3;
			}
			else if ( jschar.code <= 0x1FFFFF )
			{
				utfs[0] = 0xF0 | ((char)(jschar.code >> 17) & 0x1F );
				utfs[1] = 0x80 | ((char)(jschar.code >> 12) & 0x3F );
				utfs[2] = 0x80 | ((char)(jschar.code >> 6) & 0x3F );
				utfs[3] = 0x80 | ((char)(jschar.code) & 0x3F );
				return utfs + 4;
			}
			else
				throw; // TODO: right way of error reporting
		}

	public:
		JSString() {};
		JSString( const JSString& ) = default;
		JSString( JSString&& ) = default;
		JSString( const char* str ) { str_ = str; }
		JSString( const char8_t* str ) { str_ = (const char*)str; }
		JSString( nodecpp::string str ) { str_ = str; }
		JSString( JSChar ch ) { char buff[5]; *(utf8FromJSChar( ch, buff )) = 0; str_ = buff; }

		JSString& operator = ( const JSString& ) = default;
		JSString& operator = ( JSString&& ) = default;
		JSString& operator = ( const char* str ) { str_ = str; return *this; }
		JSString& operator = ( const char8_t* str ) { str_ = (const char*)str; return *this; }
		JSString& operator = ( nodecpp::string str ) { str_ = str; return *this; }
		JSString& operator = ( JSChar ch ) { char buff[5]; *(utf8FromJSChar( ch, buff )) = 0; str_ = buff; return *this; }

		JSChar charAt( size_t index ) const
		{
			JSChar ret;
			const char* ustr = str_.c_str();
			const char* end = ustr + str_.size();
			for ( size_t idx=0; idx<index && ustr<end; ++idx )
				ustr = utf8ToJSChar( ustr, ret );
			return ret;
		}
		size_t size() const { return str_.size(); }
		const char* c_str() const { return str_.c_str(); }
		nodecpp::string& str() {return str_; }
		const nodecpp::string& str() const {return str_; }
		nodecpp::string toString() const {return str_; }
	};

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

		JSRLValue operator [] ( const JSVar& var ) const;
		JSRLValue operator [] ( const JSRLValue& val ) const;
		JSRLValue operator [] ( double idx ) const;
		JSRLValue operator [] ( int idx ) const;
		JSRLValue operator [] ( const nodecpp::string& key ) const;
		JSRLValue operator [] ( const JSString& key ) const;
		JSRLValue operator [] ( const char8_t* key ) const;
		JSRLValue operator [] ( const char* key ) const;

		nodecpp::string toString() const;
		double toNumber() const;
		bool operator !() const { return ptr == nullptr; }
		operator nodecpp::string () const { return toString(); }

		bool has( const JSOwnObj& other ) const;
		bool has( size_t idx ) const;
		bool has( double num ) const;
		bool has( nodecpp::string str ) const;

		bool in( const JSOwnObj& collection ) const { return collection.has( *this ); }
	
		template<class callback> // expected: td::function<void(JSRLValue)>, td::function<void(JSRLValue, JSVar)>, td::function<void(JSRLValue, size_t)>, td::function<void(JSRLValue, JSVar, JSVar)>, td::function<void(JSRLValue, size_t, JSVar)>
		void forEach( callback cb );

		nodecpp::safememory::owning_ptr<JSArray> keys();

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
		friend class JSRegExp;
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

		static constexpr size_t memsz1 = fnsz > (sizeof( JSString ) > 16 ? sizeof( JSString ) : 16) ? fnsz : (sizeof( JSString ) > 16 ? sizeof( JSString ) : 16);
		static constexpr size_t memsz2 = sizeof(nodecpp::safememory::owning_ptr<int>) > memsz1 ? sizeof(nodecpp::safememory::owning_ptr<int>) : memsz1;
		static constexpr size_t memsz3 = sizeof(nodecpp::safememory::soft_ptr<int>) > memsz2 ? sizeof(nodecpp::safememory::soft_ptr<int>) : memsz2;
		static constexpr size_t memsz = ((memsz3 - 1)/sizeof(uintptr_t)+1)*sizeof(uintptr_t);
		uint8_t basemem[ memsz ];

		enum Type { undef, boolean, num, string, softptr, fn0, fn1, fn2, fn3, fn4, fn5, fn6, fn7, fn8, fn9, fn10, type_max };
		Type type = Type::undef;

		using softptr2jsobj = nodecpp::safememory::soft_ptr<JSObject>;
		using softptr2jsarr = nodecpp::safememory::soft_ptr<JSArray>;

		bool* _asBool() { return reinterpret_cast<bool*>( basemem ); }
		double* _asNum() { return reinterpret_cast<double*>( basemem ); }
		JSString* _asStr() { return reinterpret_cast<JSString*>( basemem ); }
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
		const JSString* _asStr() const { return reinterpret_cast<const JSString*>( basemem ); }
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
					_asStr()->~JSString();
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
				new(_asStr())JSString( str );
			}
		}
		void init( const JSString& str )
		{
			if ( type == Type::string )
				*_asStr() = str;
			else
			{
				deinit();
				type = Type::string;
				new(_asStr())JSString( str );
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
		void initAsFn( Fn0T&& cb )
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
		void initAsFn( Fn1T&& cb )
		{
			if ( type == Type::fn1 )
				_asFn1()->fn = std::move( cb );
			else
			{
				deinit();
				type = Type::fn1;
				new(_asFn1())Fn1Struct();
				_asFn1()->fn = std::move( cb );
			}
		}
		void initAsFn( Fn2T&& cb )
		{
			if ( type == Type::fn2 )
				_asFn2()->fn = std::move( cb );
			else
			{
				deinit();
				type = Type::fn2;
				new(_asFn2())Fn2Struct();
				_asFn2()->fn = std::move( cb );
			}
		}
		void initAsFn( Fn3T&& cb )
		{
			if ( type == Type::fn3 )
				_asFn3()->fn = std::move( cb );
			else
			{
				deinit();
				type = Type::fn3;
				new(_asFn3())Fn3Struct();
				_asFn3()->fn = std::move( cb );
			}
		}
		void initAsFn( Fn4T&& cb )
		{
			if ( type == Type::fn4 )
				_asFn4()->fn = std::move( cb );
			else
			{
				deinit();
				type = Type::fn4;
				new(_asFn4())Fn4Struct();
				_asFn4()->fn = std::move( cb );
			}
		}
		void initAsFn( Fn5T&& cb )
		{
			if ( type == Type::fn5 )
				_asFn5()->fn = std::move( cb );
			else
			{
				deinit();
				type = Type::fn5;
				new(_asFn5())Fn5Struct();
				_asFn5()->fn = std::move( cb );
			}
		}
		void initAsFn( Fn6T&& cb )
		{
			if ( type == Type::fn6 )
				_asFn6()->fn = std::move( cb );
			else
			{
				deinit();
				type = Type::fn6;
				new(_asFn6())Fn6Struct();
				_asFn6()->fn = std::move( cb );
			}
		}
		void initAsFn( Fn7T&& cb )
		{
			if ( type == Type::fn7 )
				_asFn7()->fn = std::move( cb );
			else
			{
				deinit();
				type = Type::fn7;
				new(_asFn7())Fn7Struct();
				_asFn7()->fn = std::move( cb );
			}
		}
		void initAsFn( Fn8T&& cb )
		{
			if ( type == Type::fn8 )
				_asFn8()->fn = std::move( cb );
			else
			{
				deinit();
				type = Type::fn8;
				new(_asFn8())Fn8Struct();
				_asFn8()->fn = std::move( cb );
			}
		}
		void initAsFn( Fn9T&& cb )
		{
			if ( type == Type::fn9 )
				_asFn9()->fn = std::move( cb );
			else
			{
				deinit();
				type = Type::fn9;
				new(_asFn9())Fn9Struct();
				_asFn9()->fn = std::move( cb );
			}
		}
		void initAsFn( Fn10T&& cb )
		{
			if ( type == Type::fn10 )
				_asFn10()->fn = std::move( cb );
			else
			{
				deinit();
				type = Type::fn10;
				new(_asFn10())Fn10Struct();
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
						new(_asStr())JSString( *(other._asStr()) );
						break;
					case Type::softptr:
						new(_asSoft())softptr2jsobj( *(other._asSoft()) );
						break;
					case Type::fn0:
						new(_asFn0())Fn0Struct();
						_asFn0()->fn = other._asFn0()->fn;
						break;
					case Type::fn1:
						new(_asFn1())Fn1Struct();
						_asFn1()->fn = other._asFn1()->fn;
						break;
					case Type::fn2:
						new(_asFn2())Fn2Struct();
						_asFn2()->fn = other._asFn2()->fn;
						break;
					case Type::fn3:
						new(_asFn3())Fn3Struct();
						_asFn3()->fn = other._asFn3()->fn;
						break;
					case Type::fn4:
						new(_asFn4())Fn4Struct();
						_asFn4()->fn = other._asFn4()->fn;
						break;
					case Type::fn5:
						new(_asFn5())Fn5Struct();
						_asFn5()->fn = other._asFn5()->fn;
						break;
					case Type::fn6:
						new(_asFn6())Fn6Struct();
						_asFn6()->fn = other._asFn6()->fn;
						break;
					case Type::fn7:
						new(_asFn7())Fn7Struct();
						_asFn7()->fn = other._asFn7()->fn;
						break;
					case Type::fn8:
						new(_asFn8())Fn8Struct();
						_asFn8()->fn = other._asFn8()->fn;
						break;
					case Type::fn9:
						new(_asFn9())Fn9Struct();
						_asFn9()->fn = other._asFn9()->fn;
						break;
					case Type::fn10:
						new(_asFn10())Fn10Struct();
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

		using OwnedT = JSVarOrOwn*;
		static constexpr size_t memsz1 = sizeof( OwnedT ) > sizeof( JSVarBase ) ? sizeof( OwnedT ) : sizeof( JSVarBase );
		static constexpr size_t memsz = ((memsz1 - 1)/sizeof(uintptr_t)+1)*sizeof(uintptr_t);
		uint8_t basemem[ memsz ];

		enum Type { undef, value, var, type_max };
		Type type = Type::undef;

		JSVar& _asVar() { return *reinterpret_cast<JSVar*>( basemem ); }
		OwnedT& _asValue() { return *reinterpret_cast<OwnedT*>( basemem ); }
		const JSVar& _asVar() const { return *reinterpret_cast<const JSVar*>( basemem ); }
		const OwnedT& _asValue() const { return *reinterpret_cast<const OwnedT*>( basemem ); }

		JSRLValue() {}
		JSRLValue( JSVarOrOwn& v ) {
			_asValue() = &v;
			type = Type::value;
		}
//		JSRLValue( const JSVar& var );
		void initBy( const JSVar& var );
		static JSRLValue from( const JSVar& var );
		JSRLValue& operator = ( JSVarOrOwn& obj );

	public:
		JSRLValue( const JSRLValue& other);
		JSRLValue& operator = ( const JSRLValue& other );
		JSRLValue& operator = ( const JSVar& var );
		JSRLValue& operator = ( JSOwnObj&& obj );

		~JSRLValue();

		JSRLValue operator [] ( const JSVar& var ) const;
		JSRLValue operator [] ( const JSRLValue& val ) const;
		JSRLValue operator [] ( double idx ) const;
		JSRLValue operator [] ( int idx ) const;
		JSRLValue operator [] ( const nodecpp::string& key ) const;
		JSRLValue operator [] ( const JSString& key ) const { return operator [] ( key.str() ); }
		JSRLValue operator [] ( const char8_t* key ) const { JSString s(key); return operator [] (s.str()); }
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

		operator JSVar () const;
		bool operator !() const;
		JSVar operator %( const JSVar& other ) const;
		JSVar operator ||( const JSVar& other ) const;
		JSVar operator &&( const JSVar& other ) const;
		operator nodecpp::string () const { return toString(); }
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

		void setFlags( nodecpp::string flags )
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

	public:
		JSRegExp() {}
		JSRegExp( JSVar regex_, nodecpp::string flags = "" );
		JSRegExp( nodecpp::string regex_, nodecpp::string flags = "" ) : regex( regex_ ) { setFlags( flags ); }
		JSRegExp( const char* regex_, nodecpp::string flags = "" ) : regex( regex_ ) { setFlags( flags ); }

		const nodecpp::string& re() const { return regex; }
		uint64_t flags() const { return flags_; }
	};

	class JSVar : protected JSVarBase
	{
		friend class JSObject;
		friend class JSArray;
		friend class JSMath;
		friend class JSRegExp;
		friend nodecpp::string typeOf( const JSVarBase& );
		friend nodecpp::string typeOf( const JSVarOrOwn& );
		friend nodecpp::string typeOf( const JSRLValue& );

		void init( const JSVar& other ) { JSVarBase::init( other ); }

		bool isFalseValue() const;

	public:
		JSVar() {}
		JSVar( const JSVar& other ) { init( other );}
		JSVar( const JSOwnObj& other ) { const nodecpp::js::JSVarBase::softptr2jsobj tmp = other.ptr; JSVarBase::init( tmp );}
//		JSVar( bool b ) { JSVarBase::init( b ); }
//		JSVar( double d ) { JSVarBase::init( d ); }
//		JSVar( int n ) { JSVarBase::init( (double)n ); }
//		JSVar( const nodecpp::string& str ) { JSVarBase::init( str ); }
//		JSVar( const JSString& str ) { JSVarBase::init( str ); }
		JSVar( const char* str ) { nodecpp::string str_( str ); JSVarBase::init( str_ ); }
		JSVar( const char8_t* str ) { JSString str_( str ); JSVarBase::init( str_ ); }
		JSVar( const softptr2jsobj ptr ) { JSVarBase::init( ptr ); }
		JSVar( const softptr2jsarr ptr ) { JSVarBase::init( ptr ); }
		/*JSVar( Fn0T&& cb ) { JSVarBase::initAsFn( std::move( cb ) ); }
		JSVar( Fn1T&& cb ) { JSVarBase::initAsFn( std::move( cb ) ); }
		JSVar( Fn2T&& cb ) { JSVarBase::initAsFn( std::move( cb ) ); }
		JSVar( Fn3T&& cb ) { JSVarBase::initAsFn( std::move( cb ) ); }
		JSVar( Fn4T&& cb ) { JSVarBase::initAsFn( std::move( cb ) ); }
		JSVar( Fn5T&& cb ) { JSVarBase::initAsFn( std::move( cb ) ); }
		JSVar( Fn6T&& cb ) { JSVarBase::initAsFn( std::move( cb ) ); }
		JSVar( Fn7T&& cb ) { JSVarBase::initAsFn( std::move( cb ) ); }
		JSVar( Fn8T&& cb ) { JSVarBase::initAsFn( std::move( cb ) ); }
		JSVar( Fn9T&& cb ) { JSVarBase::initAsFn( std::move( cb ) ); }
		JSVar( Fn10T&& cb ) { JSVarBase::initAsFn( std::move( cb ) ); }*/
		template<class T>
		JSVar( T val ) { 
			if constexpr ( std::is_same<T, bool>::value )
				JSVarBase::init( val );
			else if constexpr ( std::is_same<T, float>::value )
				JSVarBase::init( (double)val );
			else if constexpr ( std::is_same<T, double>::value )
				JSVarBase::init( val );
			else if constexpr ( std::is_same<T, int>::value )
				JSVarBase::init( (double)val );
			else if constexpr ( std::is_same<T, const JSVar&>::value )
				JSVarBase::init( val );
			else if constexpr ( std::is_same<T, JSVar>::value )
				JSVarBase::init( val );
			else if constexpr ( std::is_same<T, const JSOwnObj&>::value )
				JSVarBase::init( val );
			else if constexpr ( std::is_same<T, JSOwnObj>::value )
				JSVarBase::init( val );
			else if constexpr ( std::is_same<T, const nodecpp::string&>::value )
				JSVarBase::init( val );
			else if constexpr ( std::is_same<T, nodecpp::string>::value )
				JSVarBase::init( val );
			else if constexpr ( std::is_same<T, const JSString&>::value )
				JSVarBase::init( val );
			else if constexpr ( std::is_same<T, JSString>::value )
				JSVarBase::init( val );
			else
				JSVarBase::initAsFn( std::move( val ) );
		}

		~JSVar() { deinit(); }

		JSVar& operator = ( const JSVar& other ) { init( other ); return *this; }
		JSVar& operator = ( const JSOwnObj& other ) { const nodecpp::js::JSVarBase::softptr2jsobj tmp = other.ptr; JSVarBase::init( tmp ); return *this; }
//		JSVar& operator = ( bool b ) { JSVarBase::init( b ); return *this; }
//		JSVar& operator = ( double d ) { JSVarBase::init( d ); return *this; }
//		JSVar& operator = ( int n ) { JSVarBase::init( (double)n ); return *this; }
//		JSVar& operator = ( const nodecpp::string& str ) { JSVarBase::init( str ); return *this; }
//		JSVar& operator = ( const JSString& str ) { JSVarBase::init( str ); return *this; }
		JSVar& operator = ( const char* str ) { nodecpp::string str_( str ); JSVarBase::init( str_ ); return * this; }
		JSVar& operator = ( const char8_t* str ) { JSString str_( str ); JSVarBase::init( str_ ); return * this; }
		JSVar& operator = ( softptr2jsobj ptr ) { JSVarBase::init( ptr ); return *this; }
		JSVar& operator = ( softptr2jsarr ptr ) { JSVarBase::init( ptr ); return *this; }
		/*JSVar& operator = ( Fn0T&& cb ) { JSVarBase::initAsFn( std::move( cb ) ); return *this; }
		JSVar& operator = ( Fn1T&& cb ) { JSVarBase::initAsFn( std::move( cb ) ); return *this; }
		JSVar& operator = ( Fn2T&& cb ) { JSVarBase::initAsFn( std::move( cb ) ); return *this; }
		JSVar& operator = ( Fn3T&& cb ) { JSVarBase::initAsFn( std::move( cb ) ); return *this; }
		JSVar& operator = ( Fn4T&& cb ) { JSVarBase::initAsFn( std::move( cb ) ); return *this; }
		JSVar& operator = ( Fn5T&& cb ) { JSVarBase::initAsFn( std::move( cb ) ); return *this; }
		JSVar& operator = ( Fn6T&& cb ) { JSVarBase::initAsFn( std::move( cb ) ); return *this; }
		JSVar& operator = ( Fn7T&& cb ) { JSVarBase::initAsFn( std::move( cb ) ); return *this; }
		JSVar& operator = ( Fn8T&& cb ) { JSVarBase::initAsFn( std::move( cb ) ); return *this; }
		JSVar& operator = ( Fn9T&& cb ) { JSVarBase::initAsFn( std::move( cb ) ); return *this; }
		JSVar& operator = ( Fn10T&& cb ) { JSVarBase::initAsFn( std::move( cb ) ); return *this; }*/
		template<class T>
		JSVar& operator = ( T val ) { 
			if constexpr ( std::is_same<T, bool>::value )
				JSVarBase::init( val );
			else if constexpr ( std::is_same<T, float>::value )
				JSVarBase::init( (double)val );
			else if constexpr ( std::is_same<T, double>::value )
				JSVarBase::init( val );
			else if constexpr ( std::is_same<T, int>::value )
				JSVarBase::init( (double)val );
			else if constexpr ( std::is_same<T, const JSVar&>::value )
				JSVarBase::init( val );
			else if constexpr ( std::is_same<T, JSVar>::value )
				JSVarBase::init( val );
			else if constexpr ( std::is_same<T, const JSOwnObj&>::value )
				JSVarBase::init( val );
			else if constexpr ( std::is_same<T, JSOwnObj>::value )
				JSVarBase::init( val );
			else if constexpr ( std::is_same<T, const nodecpp::string&>::value )
				JSVarBase::init( val );
			else if constexpr ( std::is_same<T, nodecpp::string>::value )
				JSVarBase::init( val );
			else if constexpr ( std::is_same<T, const JSString&>::value )
				JSVarBase::init( val );
			else if constexpr ( std::is_same<T, JSString>::value )
				JSVarBase::init( val );
			else
				JSVarBase::initAsFn( std::move( val ) );
			return *this;
		}

		JSRLValue operator [] ( const JSVar& var ) const;
		JSRLValue operator [] ( const JSRLValue& val ) const;
		JSRLValue operator [] ( double num ) const;
		JSRLValue operator [] ( int num ) const;
		JSRLValue operator [] ( const nodecpp::string& key ) const;
		JSRLValue operator [] ( const JSString& key ) const { return operator [] ( key.str() ); }
		JSRLValue operator [] ( const char8_t* key ) const { JSString s(key); return operator [] (s.str()); }
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

		template<class callback> // expected: td::function<void(JSRLValue)>, td::function<void(JSRLValue, JSVar)>, td::function<void(JSRLValue, size_t)>, td::function<void(JSRLValue, JSVar, JSVar)>, td::function<void(JSRLValue, size_t, JSVar)>
		void forEach( callback cb );

		nodecpp::safememory::owning_ptr<JSArray> keys();

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

	class JSMath
	{
	public:
		static JSVar floor( JSVar var );
		static JSVar random();
	};

	class JSInit
	{
		friend class JSRLValue;

		using OwnedT = JSOwnObj;
		static constexpr size_t memsz1 = sizeof( OwnedT ) > sizeof( JSVar ) ? sizeof( OwnedT ) : sizeof( JSVar );
		static constexpr size_t memsz = ((memsz1 - 1)/sizeof(uintptr_t)+1)*sizeof(uintptr_t);
		uint8_t basemem[ memsz ];

		enum Type { undef, obj, var };
		Type type = Type::undef;

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
		friend class JSVar;
		friend class JSRLValue;
		friend class JSInit;
		friend class JSArray;
		friend nodecpp::string typeOf( const JSVarOrOwn& );

		using OwnedT = JSOwnObj;
		static constexpr size_t memsz1 = sizeof( OwnedT ) > sizeof( JSVar ) ? sizeof( OwnedT ) : sizeof( JSVar );
		static constexpr size_t memsz = ((memsz1 - 1)/sizeof(uintptr_t)+1)*sizeof(uintptr_t);
		uint8_t basemem[ memsz ];

		enum Type { undef, obj, var, type_max };
		Type type = Type::undef;

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
		nodecpp::map<nodecpp::string, JSVarOrOwn> keyValuePairs;

		virtual void concat_impl_add_me( nodecpp::safememory::owning_ptr<JSArray>& ret );

		void toString_( nodecpp::string& ret, const nodecpp::string offset, const nodecpp::string separator ) const { 
			if ( keyValuePairs.size() == 0 )
				return;
			for ( auto& entry : keyValuePairs )
			{
				nodecpp::string entrystr = nodecpp::format( "{}{}: {}{}", offset, entry.first, entry.second.toString(), separator );
				ret += entrystr;
			}
			ret.erase( ret.size() - separator.size(), separator.size() );
		}

		JSRLValue findOrAdd ( nodecpp::string s ) {
			auto f = keyValuePairs.find( s );
			if ( f != keyValuePairs.end() )
			{
				return f->second;
			}
			else
			{
				auto insret = keyValuePairs.insert( std::make_pair( s, JSVarOrOwn() ) );
				return insret.first->second;
			}
		}

		void concat_impl_1( nodecpp::safememory::owning_ptr<JSArray>& ret, JSVar arr );

	public:
		JSObject() {}
		JSObject(std::initializer_list<std::pair<nodecpp::string, JSInit>> l)
		{
			for ( auto& p : l )
				keyValuePairs.insert( make_pair( p.first, std::move( p.second.toValue() ) ) );
		}
		virtual ~JSObject() {}

		virtual JSRLValue operator [] ( const JSVar& var )
		{
			nodecpp::string s = var.toString(); // TODO: revise implementation!!!
			return findOrAdd( s );
		}
		virtual JSRLValue operator [] ( const JSRLValue& val )
		{
			if ( val.type != JSRLValue::Type::var ) // TODO: check if we should throw or convert to string, etc, instead
				return JSRLValue();
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
		virtual JSRLValue operator [] ( const JSString& key ) const { return operator [] ( key.str() ); }
		virtual JSRLValue operator [] ( const char8_t* key ) const { JSString s(key); return operator [] (s.str()); }
		virtual JSRLValue operator [] ( const char* key ) const { nodecpp::string s(key); return operator [] (s); }
		virtual JSRLValue operator [] ( const nodecpp::string_literal& key ) { nodecpp::string s( key.c_str() ); return operator [] (s); }

		double toNumber() const { return NAN; }
		virtual nodecpp::string toString() const { 
			nodecpp::string ret = "{ \n";
			toString_( ret, "  ", ",\n" );
			ret += " }";
			return ret; 
		}
		nodecpp::safememory::owning_ptr<JSArray> keys();
		virtual void forEach( std::function<void(JSRLValue)> cb ) { return; }
		virtual void forEach( std::function<void(JSRLValue, JSVar)> cb ) { return; }
		virtual void forEach( std::function<void(JSRLValue, size_t)> cb ) { return; }
		virtual void forEach( std::function<void(JSRLValue, JSVar, JSVar)> cb ) { return; }
		virtual void forEach( std::function<void(JSRLValue, size_t, JSVar)> cb ) { return; }
		virtual bool has( const JSVar& var ) const
		{
			auto f = keyValuePairs.find( var.toString() );
			return f != keyValuePairs.end();
		}
		virtual bool has( size_t idx ) const
		{
			auto f = keyValuePairs.find( nodecpp::format( "{}", idx ) );
			return f != keyValuePairs.end();
		}
		virtual bool has( double num ) const
		{
			auto f = keyValuePairs.find( nodecpp::format( "{}", num ) );
			return f != keyValuePairs.end();
		}
		virtual bool has( nodecpp::string str ) const
		{
			auto f = keyValuePairs.find( str );
			return f != keyValuePairs.end();
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

		nodecpp::vector<JSVarOrOwn> arrayValues;

		virtual void concat_impl_add_me( nodecpp::safememory::owning_ptr<JSArray>& ret );

	public:
		JSArray() {}
		JSArray(std::initializer_list<JSInit> l)
		{
			for ( auto& p : l )
				arrayValues.push_back( p.toValue() );
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
			if ( val.type != JSRLValue::Type::var ) // TODO: check if we should throw or convert to string, etc, instead
				return JSRLValue();
			return operator []( val._asVar() );
		}
		virtual JSRLValue operator [] ( size_t idx )
		{
			if ( idx < arrayValues.size() )
				return arrayValues[ idx ];
			else
			{
				arrayValues.reserve( idx + 1 );
				for ( size_t i=arrayValues.size(); i<arrayValues.capacity(); ++i )
					arrayValues.push_back( JSVarOrOwn() );
				return arrayValues[ idx ];
			}
		}
		virtual JSRLValue operator [] ( const nodecpp::string& strIdx )
		{
			if ( strIdx.size() && strIdx[0] >= '0' && strIdx[0] <= '9' )
			{
				char* end;
				size_t idx = strtol( strIdx.c_str(), &end, 10 ); // TODO: for floating ?
				if ( end == strIdx.c_str() + strIdx.size() )
					return operator [] ( idx );
			}
			return JSObject::operator[](strIdx);
		}
		virtual JSRLValue operator [] ( const JSString& key ) const { return operator [] ( key.str() ); }
		virtual JSRLValue operator [] ( const char8_t* key ) const { JSString s(key); return operator [] (s.str()); }
		virtual JSRLValue operator [] ( const char* key ) const { nodecpp::string s(key); return operator [] (s); }
		virtual JSRLValue operator [] ( const nodecpp::string_literal& key ) { nodecpp::string s( key.c_str() ); return operator [] (s); }

		virtual void forEach( std::function<void(JSRLValue)> cb )
		{
			for ( size_t idx = 0; idx<arrayValues.size(); ++idx )
				if ( arrayValues[idx].type != JSVarOrOwn::Type::undef )
					cb( arrayValues[idx] );
		}
		virtual void forEach( std::function<void(JSRLValue, JSVar idx)> cb )
		{
			for ( size_t idx = 0; idx<arrayValues.size(); ++idx )
				if ( arrayValues[idx].type != JSVarOrOwn::Type::undef )
					cb( arrayValues[idx], JSVar((double)idx) );
		}
		virtual void forEach( std::function<void(JSRLValue, size_t idx)> cb )
		{
			for ( size_t idx = 0; idx<arrayValues.size(); ++idx )
				if ( arrayValues[idx].type != JSVarOrOwn::Type::undef )
					cb( arrayValues[idx], idx );
		}
		virtual void forEach( std::function<void(JSRLValue, JSVar idx, JSVar arr)> cb )
		{
			nodecpp::safememory::soft_ptr<JSObject> myPtr = myThis.getSoftPtr<JSObject>(this);
			for ( size_t idx = 0; idx<arrayValues.size(); ++idx )
				if ( arrayValues[idx].type != JSVarOrOwn::Type::undef )
					cb( arrayValues[idx], JSVar((double)idx), JSVar( myPtr ) );
		}
		virtual void forEach( std::function<void(JSRLValue, size_t idx, JSVar arr)> cb )
		{
			nodecpp::safememory::soft_ptr<JSObject> myPtr = myThis.getSoftPtr<JSObject>(this);
			for ( size_t idx = 0; idx<arrayValues.size(); ++idx )
				if ( arrayValues[idx].type != JSVarOrOwn::Type::undef )
					cb( arrayValues[idx], idx, JSVar( myPtr ) );
		}
		virtual nodecpp::string toString() const { 
			nodecpp::string ret = "[ ";
			if ( keyValuePairs.size() )
			{
				toString_( ret, "", ", " );
				for ( auto& x : arrayValues )
					ret += nodecpp::format( ", {}", x.toString() );
			}
			else if ( arrayValues.size() )
			{
				for ( auto& x : arrayValues )
					ret += nodecpp::format( "{}, ", x.toString() );
				ret.erase( ret.end() - 2 );
			}
			ret += " ]";
			return ret; 
		}
		virtual bool has( size_t idx ) const
		{
			auto f = keyValuePairs.find( nodecpp::format( "{}", idx ) );
			return f != keyValuePairs.end();
		}
		virtual double length() const { return arrayValues.size(); }
		virtual void setLength( double ln )
		{
			if ( ln >= 0 && ln <= UINT32_MAX )
			{
				size_t iln = (size_t)ln;
				if ( ln < arrayValues.size() )
					arrayValues.erase( arrayValues.begin() + iln, arrayValues.end() );
				else 
					for ( size_t n = arrayValues.size(); n < iln; ++n )
						arrayValues.push_back( JSVar() );
			}
			else
				throw;
		}
	};
	inline owning_ptr<JSArray> makeJSArray() { return make_owning<JSArray>(); }
	inline owning_ptr<JSArray> makeJSArray(std::initializer_list<JSInit> l) { return make_owning<JSArray>(l); }

	template<class callback>
	void JSOwnObj::forEach( callback cb )
	{
		ptr->forEach( std::move( cb ) );
	}

	template<class callback>
	void JSVar::forEach( callback cb )
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

	////////////////////////////////////////////////////////////////////////////////////////////////


	class JSConsole // static functions only
	{
		template<class VarT1, class ... VarTX>
		static 
		void _stringify4logging( nodecpp::string& out, const VarT1& var1, const VarTX& ... args )
		{
			out += var1.toString();
			_stringify4logging( out, args... );
		}

		template<class VarT1>
		static 
		void _stringify4logging( nodecpp::string& out, const VarT1& var1 )
		{
			out += var1.toString();
		}

	public:

		template<class ... VarTX>
		static 
		void log( const VarTX& ... args )
		{
			// TODO: incrementing counters, etc
			nodecpp::string out;
			_stringify4logging( out, args... );
			nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "{}", out );
		}

		template<class ... VarTX>
		static 
		void log( nodecpp::string f, const VarTX& ... args )
		{
			nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "{}", nodecpp::format( f.c_str(), args... ) );
		}

		template<class ... VarTX>
		static 
		void log( const char* f, const VarTX& ... args )
		{
			nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "{}", nodecpp::format( f, args... ) );
		}

		template<class ... VarTX>
		static 
		void error( const VarTX& ... args )
		{
			// TODO: revise (printing to stderr)
			nodecpp::string out;
			_stringify4logging( out, args... );
			nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "{}", out );
		}

		template<class ... VarTX>
		static 
		void error( nodecpp::string f, const VarTX& ... args )
		{
			// TODO: revise (printing to stderr)
			nodecpp::log::default_log::error( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "{}", nodecpp::format( f.c_str(), args... ) );
		}

		template<class ... VarTX>
		static 
		void error( const char* f, const VarTX& ... args )
		{
			// TODO: revise (printing to stderr)
			nodecpp::log::default_log::error( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "{}", nodecpp::format( f, args... ) );
		}

		template<class ... VarTX>
		static 
		void assert( bool cond, const VarTX& ... args )
		{
			if ( cond )
				return;
			// TODO: revise (printing to stderr)
			nodecpp::string out;
			_stringify4logging( out, args... );
			nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "{}", out );
		}

		template<class ... VarTX>
		static 
		void assert( bool cond, nodecpp::string f, const VarTX& ... args )
		{
			if (!cond)
				nodecpp::log::default_log::error( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "{}", nodecpp::format( f.c_str(), args... ) );
		}

		template<class ... VarTX>
		static 
		void assert( bool cond, const char* f, const VarTX& ... args )
		{
			if (!cond)
				nodecpp::log::default_log::error( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "{}", nodecpp::format( f, args... ) );
		}

	};
	static_assert( sizeof(JSConsole) <= 1 );
	extern JSConsole console;

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

template<>
struct fmt::formatter<nodecpp::js::JSString>
{
	template<typename ParseContext> constexpr auto parse(ParseContext& ctx) {return ctx.begin();}
	template<typename FormatContext> auto format(nodecpp::js::JSString const& str, FormatContext& ctx) {return fmt::format_to(ctx.out(), "{}", str.str() );}
};	

template<>
struct fmt::formatter<nodecpp::js::JSOwnObj>
{
	template<typename ParseContext> constexpr auto parse(ParseContext& ctx) {return ctx.begin();}
	template<typename FormatContext> auto format(nodecpp::js::JSOwnObj const& obj, FormatContext& ctx) {return fmt::format_to(ctx.out(), "{}", obj.toString() );}
};	

template<>
struct fmt::formatter<nodecpp::js::JSVar>
{
	template<typename ParseContext> constexpr auto parse(ParseContext& ctx) {return ctx.begin();}
	template<typename FormatContext> auto format(nodecpp::js::JSVar const& v, FormatContext& ctx) {return fmt::format_to(ctx.out(), "{}", v.toString() );}
};	

template<>
struct fmt::formatter<nodecpp::js::JSVarOrOwn>
{
	template<typename ParseContext> constexpr auto parse(ParseContext& ctx) {return ctx.begin();}
	template<typename FormatContext> auto format(nodecpp::js::JSVarOrOwn const& v, FormatContext& ctx) {return fmt::format_to(ctx.out(), "{}", v.toString() );}
};	

template<>
struct fmt::formatter<nodecpp::js::JSRLValue>
{
	template<typename ParseContext> constexpr auto parse(ParseContext& ctx) {return ctx.begin();}
	template<typename FormatContext> auto format(nodecpp::js::JSRLValue const& v, FormatContext& ctx) {return fmt::format_to(ctx.out(), "{}", v.toString() );}
};	

#endif // NODECPP_JS_COMPAT_H

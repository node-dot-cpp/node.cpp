/* -------------------------------------------------------------------------------
* Copyright (c) 2019, OLogN Technologies AG
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

// STATUS: stub (work in progress)

#ifndef NODECPP_URL_H
#define NODECPP_URL_H

#include "common.h"

namespace nodecpp {

	class UrlQueryItem{
		nodecpp::vector<nodecpp::string> elems;
		nodecpp::string none;

	public:
		UrlQueryItem() {}
		UrlQueryItem( const UrlQueryItem& ) = delete;
		UrlQueryItem& operator = ( const UrlQueryItem& ) = delete;
		UrlQueryItem( UrlQueryItem&& other ) { elems = std::move( other.elems ); }
		UrlQueryItem& operator = ( UrlQueryItem&& other ) { elems = std::move( other.elems ); return *this; }

		void add( const nodecpp::string& val )
		{
			elems.push_back( val );
		}
		nodecpp::string toStr() const
		{
			nodecpp::string ret;
			if ( elems.empty() )
				return ret;
			ret.append( elems[0] );
			for ( size_t i=1; i<elems.size(); ++i )
			{
				ret.append( "," );
				ret.append( elems[i] );
			}
			return ret;
		}
	};

	class UrlQuery
	{
		nodecpp::map<nodecpp::string, UrlQueryItem> parsed;
		UrlQueryItem none;
	public:
		UrlQuery() {}
		UrlQuery( const UrlQuery& ) = delete;
		UrlQuery& operator = ( const UrlQuery& ) = delete;
		UrlQuery( UrlQuery&& other ) { parsed = std::move( other.parsed ); }
		UrlQuery& operator = ( UrlQuery&& other ) { parsed = std::move( other.parsed ); return *this; }

		void add( const nodecpp::string& key, const nodecpp::string& val )
		{
			auto f = parsed.find( key );
			if ( f != parsed.end() )
				f->second.add( val );
			else
			{
				UrlQueryItem it;
				it.add( val );
				auto ins = parsed.insert( nodecpp::make_pair( key, std::move( it ) ) );
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, ins.second );
			}
		}

		const UrlQueryItem& operator [] ( const nodecpp::string& key ) {
			auto f = parsed.find( key );
			if ( f != parsed.end() )
				return f->second;
			return none;
		}

	};

	class Url {
	public:
		static inline
		void parseUrlQueryString(const nodecpp::string& url, UrlQuery& q )
		{
			size_t start = url.find_first_of( "?" );
			if ( start == nodecpp::string::npos )
				return;
			++start;

			for(;;)
			{
				size_t endEq = url.find_first_of( "=", start );
				if ( endEq == nodecpp::string::npos )
				{
					q.add( url.substr( start, endEq-start ), nodecpp::string_literal("") );
					break;
				}
				size_t endAmp = url.find_first_of( "&", endEq+ 1  );
				if ( endAmp == nodecpp::string::npos )
				{
					q.add( url.substr( start, endEq-start ), url.substr( endEq + 1 ) );
					break;
				}
				else
				{
					q.add( url.substr( start, endEq-start ), url.substr( endEq + 1, endAmp - endEq - 1 ) ) ;
					start = endAmp + 1;
				}
			}
		}
		static inline
		UrlQuery parseUrlQueryString(const nodecpp::string& url )
		{
			UrlQuery q;
			size_t start = url.find_first_of( "?" );
			if ( start == nodecpp::string::npos )
				return q;
			++start;

			for(;;)
			{
				size_t endEq = url.find_first_of( "=", start );
				if ( endEq == nodecpp::string::npos )
				{
					q.add( url.substr( start, endEq-start ), nodecpp::string_literal("") );
					break;
				}
				size_t endAmp = url.find_first_of( "&", endEq+ 1  );
				if ( endAmp == nodecpp::string::npos )
				{
					q.add( url.substr( start, endEq-start ), url.substr( endEq + 1 ) );
					break;
				}
				else
				{
					q.add( url.substr( start, endEq-start ), url.substr( endEq + 1, endAmp - endEq - 1 ) ) ;
					start = endAmp + 1;
				}
			}
			return q;
		}
	};

} //namespace nodecpp

#endif // NODECPP_URL_H

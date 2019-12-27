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

	class UrlQuery
	{
		nodecpp::map<nodecpp::string, nodecpp::string> parsed;
		nodecpp::string none;
	public:
		UrlQuery() {}
		UrlQuery( const UrlQuery& ) = delete;
		UrlQuery& operator = ( const UrlQuery& ) = delete;
		UrlQuery( UrlQuery&& other ) { parsed = std::move( other.parsed ); }
		UrlQuery& operator = ( UrlQuery&& other ) { parsed = std::move( other.parsed ); }

		void add( const nodecpp::string& key, const nodecpp::string& val )
		{
			auto ins = parsed.insert( std::make_pair( key, val ) );
			if ( !ins.second )
			{
				ins.first->second.append( "," );
				ins.first->second.append( val );
			}
		}
		const nodecpp::string& operator [] ( const nodecpp::string& key ) {
			auto f = parsed.find( key );
			if ( f != parsed.end() )
				return f->second;
			return nodecpp::string("");
		}
		const nodecpp::string& operator [] ( const nodecpp::string_literal& key ) {
			nodecpp::string s( key.c_str() );
			auto f = parsed.find( s );
			if ( f != parsed.end() )
				return f->second;
			return nodecpp::string("");
		}
		const nodecpp::string& operator [] ( const char* key ) {
			nodecpp::string s( key );
			auto f = parsed.find( s );
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
					q.add( url.substr( start, endEq-start ), "" );
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
					q.add( url.substr( start, endEq-start ), "" );
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

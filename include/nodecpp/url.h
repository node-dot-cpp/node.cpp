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
	class Url {
	public:
		static inline
		nodecpp::vector<std::pair<nodecpp::string, nodecpp::string>> parseUrlQueryString(const nodecpp::string& url )
		{
			nodecpp::vector<std::pair<nodecpp::string, nodecpp::string>> queryValues;
			size_t start = url.find_first_of( "?" );
			if ( start == nodecpp::string::npos )
				return queryValues;
			++start;

			for(;;)
			{
				size_t endEq = url.find_first_of( "=", start );
				if ( endEq == nodecpp::string::npos )
				{
					queryValues.push_back( std::make_pair( url.substr( start, endEq-start ), "" ) );
					break;
				}
				size_t endAmp = url.find_first_of( "&", endEq+ 1  );
				if ( endAmp == nodecpp::string::npos )
				{
					queryValues.push_back( std::make_pair( url.substr( start, endEq-start ), url.substr( endEq + 1 ) ) );
					break;
				}
				else
				{
					queryValues.push_back( std::make_pair( url.substr( start, endEq-start ), url.substr( endEq + 1, endAmp - endEq - 1 ) ) );
					start = endAmp + 1;
				}
			}
			return queryValues;
		}
	};

} //namespace nodecpp

#endif // NODECPP_URL_H

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

#ifndef NODECPP_LOGGING_H
#define NODECPP_LOGGING_H

#include "common.h"


namespace nodecpp {
	
//	enum class Console { std_out = ::stdout, std_err = ::stderr };

    class Log;
	class LogTransport
	{
		friend class Log;
		FILE* target; // so far...
	public:
		LogTransport() {}
		LogTransport( nodecpp::string path ) { target = fopen( path.c_str(), "a" ); }
		LogTransport( FILE* f ) { target = f; }
		LogTransport( const LogTransport& ) = delete;
		LogTransport& operator = ( const LogTransport& ) = delete;
		LogTransport( LogTransport&& other ) { target = other.target; other.target = nullptr; }
		LogTransport& operator = ( LogTransport&& other ) { target = other.target; other.target = nullptr; return *this;}
		~LogTransport() { if ( target ) fclose( target ); }

	private:
		void writoToLog( nodecpp::string s, size_t severity );
	};

    class Log
	{
	public:
		enum class Level { error = 0, warn = 1, info = 2, http = 3, verbose = 4, debug = 5, silly = 6 };
		static constexpr const char* LogLevelNames[] = { "error", "warn", "info", "http", "verbose", "debug", "silly", "" };
		Level level = Level::info;

		nodecpp::vector<LogTransport> transports;
		bool silent = false;

		//TODO::add: format
		//TODO::add: exitOnError

	public:
		Log() {}
		virtual ~Log() {}

		template<class ... Objects>
		void log( Level l, nodecpp::string_literal format_str, Objects ... obj ) {
			if ( l <= level ) {
				nodecpp::string msg = nodecpp::format( format_str, obj ... );
				for ( auto& transport : transports )
					transport.writoToLog( msg, (size_t)l );
			}				 
		}
		template<class ... Objects>
		void error( nodecpp::string_literal format_str, Objects ... obj ) { log( Level::error, format_str, obj ... ); }
		template<class ... Objects>
		void warn( nodecpp::string_literal format_str, Objects ... obj ) { log( Level::warn, format_str, obj ... ); }
		template<class ... Objects>
		void info( nodecpp::string_literal format_str, Objects ... obj ) { log( Level::info, format_str, obj ... ); }
		template<class ... Objects>
		void http( nodecpp::string_literal format_str, Objects ... obj ) { log( Level::http, format_str, obj ... ); }
		template<class ... Objects>
		void verbose( nodecpp::string_literal format_str, Objects ... obj ) { log( Level::verbose, format_str, obj ... ); }
		template<class ... Objects>
		void debug( nodecpp::string_literal format_str, Objects ... obj ) { log( Level::debug, format_str, obj ... ); }
		template<class ... Objects>
		void silly( nodecpp::string_literal format_str, Objects ... obj ) { log( Level::silly, format_str, obj ... ); }

		void clear() { transports.clear(); }
		bool add( nodecpp::string path ) { transports.emplace_back( path ); return transports.back().target != nullptr; }
		bool add( FILE* cons ) { transports.emplace_back( cons ); return transports.back().target != nullptr; } // TODO: input param is a subject for revision
		//TODO::add: remove()
	};

	void LogTransport::writoToLog( nodecpp::string s, size_t severity ) {
		if ( target != nullptr )
			fprintf( target, "[%s] %s\n", Log::LogLevelNames[(size_t)severity], s.c_str() );
	}
} //namespace nodecpp

#endif // NODECPP_LOGGING_H

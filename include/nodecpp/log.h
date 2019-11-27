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
	class LogTransport;

	class LogBuffer
	{
		// NOTE: it is just a quick sketch
		// NOTE: so far it is only a thread-unsafe sketch
		friend class Log;
		friend class LogTransport;

		size_t pageSize; // consider making a constexpr (do we consider 2Mb pages?)
		uint8_t* buff = nullptr; // aming: a set of consequtive pages
		size_t buffSize = 0; // a multiple of page size
		uint64_t start = 0; // writable by a thread writing to a file; readable: all
		uint64_t end = 0; // writable: logging threads; readable: all
		static constexpr size_t skippedCntMsgSz = 10; // todo: calc actual size
		size_t skippedCount = 0; // accessible by log-writing threads

		FILE* target; // so far...

		void addSkippedNotification()
		{
			// TODO: prepare and inster a message; update properly the 'end'
			end += skippedCntMsgSz;
			skippedCount = 0;
		}

	public:
		LogBuffer() {}
		LogBuffer( const LogBuffer& ) = delete;
		LogBuffer& operator = ( const LogBuffer& ) = delete;
		LogBuffer( LogBuffer&& other ) {
			pageSize = other.pageSize;
			buff = other.buff;
			other.buff = nullptr;
			buffSize = other.buffSize;
			other.buffSize = 0;
			start = other.start;
			end = other.end;
			other.start = 0;
			other.end = 0;
			skippedCount = other.skippedCount;
		}
		LogBuffer& operator = ( LogBuffer&& other )
		{
			pageSize = other.pageSize;
			buff = other.buff;
			other.buff = nullptr;
			buffSize = other.buffSize;
			other.buffSize = 0;
			start = other.start;
			end = other.end;
			other.start = 0;
			other.end = 0;
			skippedCount = other.skippedCount;
			return *this;
		}
		~LogBuffer()
		{
			if ( target ) fclose( target );
			if ( buff ) nodecpp::dealloc( buff, buffSize );
		}
		void init( size_t pageSize_, size_t pageCnt, FILE* f )
		{
			if ( buff ) nodecpp::dealloc( buff, buffSize );
			pageSize = pageSize_;
			buffSize = pageSize * pageCnt;
			buff = nodecpp::alloc<uint8_t>( buffSize );

			if ( target ) fclose( target );
			target = f;
		}
		void init( size_t pageSize_, size_t pageCnt, const char* path )
		{
			FILE* f = fopen( path, "ab" );
			init( pageSize_, pageCnt, f );
		}

		bool addMsg( const char* msg, size_t sz )
		{
			// TODO: thread-sync
			size_t fullSzRequired = skippedCount == 0 ? sz : sz + skippedCntMsgSz;
			if ( end - start + fullSzRequired <= buffSize ) // can copy
			{
				if ( skippedCount )
					addSkippedNotification();
				if ( buffSize - end >= sz )
				{
					memcpy( buff + end, msg, sz );
					end += sz;
				}
				else
				{
					memcpy( buff + end, msg, buffSize - end );
					memcpy( buff, msg + buffSize - end, sz - (buffSize - end) );
					end = sz - (buffSize - end);
				}
				return true;
			}
			else
			{
				++skippedCount;
				return false;
			}
		}

		void flush()
		{
			// TODO: thread-sync
			// so far, for testing purposes we will sit single-threaded and do some kind of emulation
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, ( start & ( pageSize - 1) ) == 0 ); 
			size_t pageRoundedEnd = end & ~(pageSize -1);
			if ( start != pageRoundedEnd )
			{
				size_t startoff = start % buffSize;
				size_t endoff = pageRoundedEnd % buffSize;
				if ( endoff > startoff )
					fwrite( buff + startoff, 1, endoff - startoff, target );
				else
				{
					fwrite( buff + startoff, 1, buffSize - startoff, target );
					fwrite( buff, 1, endoff, target );
				}
				start = pageRoundedEnd;
			}
		}
	};

	class LogTransport
	{
		friend class Log;
		//FILE* target; // so far...
		LogBuffer lb;
	public:
		LogTransport() {}
//		LogTransport( nodecpp::string path ) { target = fopen( path.c_str(), "a" ); }
		LogTransport( nodecpp::string path ) { lb.init( 0x1000, 2, path.c_str() ); }
//		LogTransport( FILE* f ) { target = f; }
		LogTransport( FILE* f ) { lb.init( 0x1000, 2, f ); }
		LogTransport( const LogTransport& ) = delete;
		LogTransport& operator = ( const LogTransport& ) = delete;
//		LogTransport( LogTransport&& other ) { target = other.target; other.target = nullptr; }
//		LogTransport& operator = ( LogTransport&& other ) { target = other.target; other.target = nullptr; return *this;}
//		~LogTransport() { if ( target ) fclose( target ); }
		LogTransport( LogTransport&& other ) = default;
		LogTransport& operator = ( LogTransport&& other ) = default;

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
		bool add( nodecpp::string path ) { transports.emplace_back( path ); return transports.back().lb.target != nullptr; }
		bool add( FILE* cons ) { transports.emplace_back( cons ); return transports.back().lb.target != nullptr; } // TODO: input param is a subject for revision
		//TODO::add: remove()
	};

	void LogTransport::writoToLog( nodecpp::string s, size_t severity ) {
		if ( lb.target != nullptr )
		{
			//fprintf( target, "[%s] %s\n", Log::LogLevelNames[(size_t)severity], s.c_str() );
			nodecpp::string msgformatted = nodecpp::format( "[{}] {}\n", Log::LogLevelNames[(size_t)severity], s );
			lb.addMsg( msgformatted.c_str(), msgformatted.size() );
			lb.flush(); // just emulation
		}
	}
} //namespace nodecpp

#endif // NODECPP_LOGGING_H

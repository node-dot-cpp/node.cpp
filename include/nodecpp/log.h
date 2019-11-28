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
#include "cluster.h"
#include <mutex>


namespace nodecpp {
	
//	enum class Console { std_out = ::stdout, std_err = ::stderr };

	class Log;
	class LogTransport;

	struct LogBufferBaseData
	{
		size_t pageSize; // consider making a constexpr (do we consider 2Mb pages?)
		uint8_t* buff = nullptr; // aming: a set of consequtive pages
		size_t buffSize = 0; // a multiple of page size
		volatile int64_t start = 0; // writable by a thread writing to a file; readable: all
		volatile uint64_t end = 0; // writable: logging threads; readable: all
		static constexpr size_t skippedCntMsgSz = 32;
		size_t skippedCount = 0; // accessible by log-writing threads
		std::condition_variable waitLogger;
		std::condition_variable waitLWriter;
		std::mutex mx;
		std::mutex mxWriter;

		FILE* target = nullptr; // so far...

		void init( size_t pageSize_, size_t pageCnt, FILE* f )
		{
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, nodecpp::clusterIsMaster() ); 
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, buff == nullptr ); 
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, target == nullptr ); 
			pageSize = pageSize_;
			buffSize = pageSize * pageCnt;
			buff = nodecpp::alloc<uint8_t>( buffSize );

			target = f;
		}
		void init( size_t pageSize_, size_t pageCnt, const char* path )
		{
			FILE* f = fopen( path, "ab" );
			init( pageSize_, pageCnt, f );
		}
		void deinit()
		{
			if ( buff ) nodecpp::dealloc( buff, buffSize );
		}
	};
} // nodecpp


namespace nodecpp::logging_impl {
	extern void createLogWriterThread( ::nodecpp::LogBufferBaseData* );
//	extern thread_local ::nodecpp::vector<LogBufferBaseData*> logDataStructures;
	extern ::nodecpp::stdvector<LogBufferBaseData*> logDataStructures;
} // nodecpp::logging_impl


namespace nodecpp {
	class LogWriter
	{
		LogBufferBaseData* logData;
	public:
		LogWriter( LogBufferBaseData* logData_ ) : logData( logData_ ) {}

		void flush()
		{

			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, ( logData->start & ( logData->pageSize - 1) ) == 0 ); 
			size_t pageRoundedEnd = logData->end & ~(logData->pageSize -1);
			if ( logData->start != pageRoundedEnd )
			{
				size_t startoff = logData->start % logData->buffSize;
				size_t endoff = pageRoundedEnd % logData->buffSize;
				if ( endoff > startoff )
					fwrite( logData->buff + startoff, 1, endoff - startoff, logData->target );
				else
				{
					fwrite( logData->buff + startoff, 1, logData->buffSize - startoff, logData->target );
					fwrite( logData->buff, 1, endoff, logData->target );
				}
				logData->start = pageRoundedEnd;
			}
		}
	};

	class LogBuffer
	{
		// NOTE: it is just a quick sketch
		friend class Log;
		friend class LogTransport;

		LogBufferBaseData* logData;

	public:
		LogBuffer( LogBufferBaseData* data ) : logData( data ) {}
		LogBuffer( const LogBuffer& ) = delete;
		LogBuffer& operator = ( const LogBuffer& ) = delete;
		LogBuffer( LogBuffer&& other ) {
			logData = other.logData;
			other.logData = nullptr;
		}
		LogBuffer& operator = ( LogBuffer&& other )
		{
			logData = other.logData;
			other.logData = nullptr;
			return *this;
		}
		~LogBuffer()
		{
		}

		void addMsg_( const char* msg, size_t sz )
		{
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, sz <= logData->buffSize - (logData->end - logData->start) );
			size_t endoff = logData->end % logData->buffSize; // TODO: math
			if ( logData->buffSize - endoff >= sz )
			{
				memcpy( logData->buff + endoff, msg, sz );
			}
			else
			{
				memcpy( logData->buff + endoff, msg, logData->buffSize - endoff );
				memcpy( logData->buff, msg + logData->buffSize - endoff, sz - (logData->buffSize - endoff) );
			}
			logData->end += sz;
		}

		bool addMsg( const char* msg, size_t sz )
		{
			bool ret = true;
			bool wakeWriter = false;
			{
//				std::unique_lock<std::mutex> lock(logData->mx);
				size_t fullSzRequired = logData->skippedCount == 0 ? sz : sz + logData->skippedCntMsgSz;
				if ( logData->end - logData->start + fullSzRequired <= logData->buffSize ) // can copy
				{
					size_t endoff = logData->end % logData->buffSize; // TODO: math
					if ( logData->skippedCount )
					{
						nodecpp::string skippedMsg = nodecpp::format( "<skipped {} msgs>\n", logData->skippedCount );
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, skippedMsg.size() <= logData->skippedCntMsgSz );
						addMsg_( skippedMsg.c_str(), skippedMsg.size() );
						logData->skippedCount = 0;
					}
					addMsg_( msg, sz );
					wakeWriter = logData->end - logData->start >= logData->pageSize;
				}
				else
				{
					++(logData->skippedCount);
					ret = false;
					wakeWriter = true;
				}
			} // unlocking
			logData->waitLogger.notify_one(); // outside the lock!
//			if ( wakeWriter )
//				logData->waitLWriter.notify_one();
			return ret;
		}
	};

	class LogTransport
	{
		friend class Log;
		LogBuffer lb;

	public:
		LogTransport( LogBufferBaseData* data ) : lb( data ) {}
		LogTransport( const LogTransport& ) = delete;
		LogTransport& operator = ( const LogTransport& ) = delete;
		LogTransport( LogTransport&& other ) = default;
		LogTransport& operator = ( LogTransport&& other ) = default;
		~LogTransport() {}

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

		size_t transportIdx = 0; // for adding purposes in case of clustering

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
		bool add( nodecpp::string path ) 
		{
			if ( nodecpp::clusterIsMaster() )
			{
				LogBufferBaseData* data = nodecpp::stdalloc<LogBufferBaseData>( 1 );
				data->init( 0x1000, 2, path.c_str() );
				::nodecpp::logging_impl::createLogWriterThread( data );
				transports.emplace_back( data ); 
				::nodecpp::logging_impl::logDataStructures.push_back( data );
				return true; // TODO
			}
			else
			{
				if ( ::nodecpp::logging_impl::logDataStructures.size() <= transportIdx )
					return false;
				LogBufferBaseData* data = ::nodecpp::logging_impl::logDataStructures[ transportIdx ];
				transports.emplace_back( data ); 
				++transportIdx;
			}
		}
		bool add( FILE* cons ) // TODO: input param is a subject for revision
		{
			if ( nodecpp::clusterIsMaster() )
			{
				LogBufferBaseData* data = nodecpp::stdalloc<LogBufferBaseData>( 1 );
				data->init( 0x1000, 2, cons );
				::nodecpp::logging_impl::createLogWriterThread( data );
				transports.emplace_back( data ); 
				::nodecpp::logging_impl::logDataStructures.push_back( data );
				return true; // TODO
			}
			else
			{
				if ( ::nodecpp::logging_impl::logDataStructures.size() <= transportIdx )
					return false;
				LogBufferBaseData* data = ::nodecpp::logging_impl::logDataStructures[ transportIdx ];
				transports.emplace_back( data ); 
				++transportIdx;
			}
		}
		//TODO::add: remove()
	};

	inline
	void LogTransport::writoToLog( nodecpp::string s, size_t severity ) {
//		if ( lb.target != nullptr )
		{
			nodecpp::string msgformatted = nodecpp::format( "[{}] {}\n", Log::LogLevelNames[(size_t)severity], s );
			lb.addMsg( msgformatted.c_str(), msgformatted.size() );
		}
	}
} //namespace nodecpp

#endif // NODECPP_LOGGING_H

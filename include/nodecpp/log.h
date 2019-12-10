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
#include <iibmalloc_page_allocator.h>


namespace nodecpp {

	class ModuleID
	{
		const char* str;
	public:
		ModuleID( const char* str_) : str( str_ ) {}
		ModuleID( const string_literal& other ) : str( other.c_str() ) {}
		ModuleID( const ModuleID& other ) : str( other.str ) {}
		ModuleID& operator = ( const ModuleID& other ) {str = other.str; return *this;}
		ModuleID( ModuleID&& other ) = delete;
		ModuleID& operator = ( ModuleID&& other ) = delete;
		const char* id() const { return str; }
	};
	
	enum class LogLevel { emerg = 0, alert = 1, crit = 2, err = 3, warning = 4, notice = 5, info = 6, debug = 7 };
	static constexpr const char* LogLevelNames[] = { "emerg", "alert", "crit", "err", "warning", "notice", "info", "debug", "" };
	constexpr size_t log_level_count = sizeof( LogLevelNames ) / sizeof( const char*) - 1;

	class Log;
	class LogTransport;

	struct SkippedMsgCounters
	{
		uint32_t skippedCtrs[log_level_count];
		uint32_t fullCnt_;
		SkippedMsgCounters() {clear();}
		void clear() { memset( skippedCtrs, 0, sizeof( skippedCtrs ) ); fullCnt_ = 0; }
		void increment( LogLevel l ) { 
			++(skippedCtrs[(size_t)l]);
			++fullCnt_;
		}
		size_t fullCount() { return fullCnt_; }
		::nodecpp::string toStr() {
			::nodecpp::string ret = "<skipped:";
			for ( size_t i=0; i<log_level_count; ++i )
				if ( skippedCtrs[i] )
					ret += ::nodecpp::format( " {}: {}", LogLevelNames[i], skippedCtrs[i] );
			return ret;
		}
	};

	struct ChainedWaitingData
	{
		std::condition_variable w;
		std::mutex mx;
		SkippedMsgCounters skippedCtrs;
		bool canRun = false;
		ChainedWaitingData* next = nullptr;
	};

	struct LogBufferBaseData
	{
		LogLevel levelCouldBeSkipped = LogLevel::info;
		size_t pageSize; // consider making a constexpr (do we consider 2Mb pages?)
		static constexpr size_t pageCount = 4; // so far it is not obvious why we really need something else
		uint8_t* buff = nullptr; // aming: a set of consequtive pages
		size_t buffSize = 0; // a multiple of page size
		volatile int64_t start = 0; // writable by a thread writing to a file; readable: all
		volatile uint64_t writerPromisedNextStart = 0; // writable: logging threads; readable: all
		volatile uint64_t end = 0; // writable: logging threads; readable: all
		static constexpr size_t skippedCntMsgSz = 128;
		SkippedMsgCounters skippedCtrs; // accessible by log-writing threads
		std::condition_variable waitLogger;
		std::condition_variable waitWriter;
		std::mutex mx;

		ChainedWaitingData* firstToRelease = nullptr; // for writer
		ChainedWaitingData* nextToAdd = nullptr; // for loggers

		FILE* target = nullptr; // so far...

		void init( FILE* f )
		{
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, nodecpp::clusterIsMaster() ); 
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, buff == nullptr ); 
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, target == nullptr ); 
			size_t memPageSz = VirtualMemory::getPageSize();
			if ( memPageSz <= 0x2000 )
			{
				pageSize = memPageSz;
				buffSize = pageSize * pageCount;
				buff = reinterpret_cast<uint8_t*>( VirtualMemory::allocate( buffSize ) );
			}
			else // TODO: revise for Large pages!!!
			{
				pageSize = memPageSz / pageCount;
				buffSize = memPageSz;
				buff = reinterpret_cast<uint8_t*>( VirtualMemory::allocate( memPageSz ) );
			}
			if ( buff == nullptr )
				throw;

			target = f;
		}
		void init( size_t pageSize_, size_t pageCnt, const char* path )
		{
			FILE* f = fopen( path, "ab" );
			init( f );
		}
		void deinit()
		{
			if ( buff )
			{
				VirtualMemory::deallocate( buff, buffSize );
				buff = nullptr;
			}
			if ( target ) 
			{
				fclose( target );
				target = nullptr;
			}
		}
		size_t availableSize() { return buffSize - (end - start); }
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

		void runLoop()
		{
			for (;;)
			{
				std::unique_lock<std::mutex> lock(logData->mx);
				while ( (logData->end & ~( logData->pageSize - 1)) == logData->start )
					logData->waitWriter.wait(lock);
				lock.unlock();

				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, ( logData->start & ( logData->pageSize - 1) ) == 0 ); 

				// there are two things we can do here:
				// 1. flush one or more ready pages
				// 2. release a thread waiting for free size in buffer (if any)
				// first, collects data
				size_t pageRoundedEnd;
				ChainedWaitingData* p = nullptr;
				{
					std::unique_lock<std::mutex> lock(logData->mx);
					pageRoundedEnd = logData->end & ~( logData->pageSize - 1);
					logData->writerPromisedNextStart = pageRoundedEnd;

					if ( logData->start == pageRoundedEnd && logData->firstToRelease != nullptr )
					{
						p = logData->firstToRelease;
						logData->firstToRelease = nullptr;
					}
				} // unlocking
				if ( p )
				{
					{
						std::unique_lock<std::mutex> lock(p->mx);
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, !p->canRun );
						p->canRun = true;
					}
					p->w.notify_one();
					if ( logData->start == pageRoundedEnd )
						return;
				}

				while ( logData->start != pageRoundedEnd )
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

					ChainedWaitingData* p = nullptr;
					{
						std::unique_lock<std::mutex> lock(logData->mx);
						logData->start = pageRoundedEnd;
						if ( logData->firstToRelease )
						{
							p = logData->firstToRelease;
							logData->firstToRelease = nullptr;
						}
						pageRoundedEnd = logData->end & ~( logData->pageSize - 1);
						logData->writerPromisedNextStart = pageRoundedEnd;
					} // unlocking
					if ( p )
					{
						{
							std::unique_lock<std::mutex> lock(p->mx);
							NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, !p->canRun );
							p->canRun = true;
						}
						p->w.notify_one();
					}
				}
			}
		}
	};

	class LogTransport
	{
		// NOTE: it is just a quick sketch
		friend class Log;

		LogBufferBaseData* logData;

		void insertSingleMsg( const char* msg, size_t sz ) // under lock
		{
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, sz <= logData->availableSize() );
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

		void insertMessage( const char* msg, size_t sz, SkippedMsgCounters& ctrs ) // under lock
		{
			size_t endoff = logData->end % logData->buffSize; // TODO: math
			if ( ctrs.fullCount() )
			{
				nodecpp::string skippedMsg = logData->skippedCtrs.toStr();
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, skippedMsg.size() <= logData->skippedCntMsgSz );
				insertSingleMsg( skippedMsg.c_str(), skippedMsg.size() );
				ctrs.clear();
			}
			insertSingleMsg( msg, sz );
			if (logData->end - logData->writerPromisedNextStart >= logData->pageSize)
			{
				logData->waitWriter.notify_one();
			}
		}

		bool addMsg( const char* msg, size_t sz, LogLevel l )
		{
			bool ret = true;
			bool waitAgain = false;
			ChainedWaitingData d;
			{
				std::unique_lock<std::mutex> lock(logData->mx);
				size_t fullSzRequired = logData->skippedCtrs.fullCount() == 0 ? sz : sz + logData->skippedCntMsgSz;
				if ( logData->nextToAdd != nullptr)
				{
					if (l >= logData->levelCouldBeSkipped)
					{
						logData->nextToAdd->skippedCtrs.increment(l);
						ret = false;
					}
					else
					{
						logData->nextToAdd->next = &d;
						logData->nextToAdd = &d;
						waitAgain = true;
					}
				}
				else if ( logData->end + fullSzRequired <= logData->start + logData->buffSize ) // can copy
				{
					insertMessage( msg, sz, logData->skippedCtrs );
				}
				else
				{
					if ( l >= logData->levelCouldBeSkipped ) // skip
					{
						logData->skippedCtrs.increment(l);
						ret = false;
					}
					else // add to waiting list
					{
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, logData->nextToAdd == nullptr );
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, logData->firstToRelease == nullptr ); 
						logData->firstToRelease = &d;
//						d.canRun = true;
						logData->nextToAdd = &d;
						waitAgain = true;
					}
				}
			} // unlocking

			while ( waitAgain )
			{
				waitAgain = false;
				std::unique_lock<std::mutex> lock(d.mx);
				while (!d.canRun)
					d.w.wait(lock);
				d.canRun = false;
				lock.unlock();

				{
					std::unique_lock<std::mutex> lock(logData->mx);

					size_t fullSzRequired = logData->skippedCtrs.fullCount() == 0 ? sz : sz + logData->skippedCntMsgSz;
					if ( logData->end + fullSzRequired > logData->start + logData->buffSize )
					{
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, logData->firstToRelease == nullptr );
						logData->firstToRelease = &d;
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, logData->nextToAdd != nullptr );
						waitAgain = true;
						lock.unlock();
						continue;
					}

					insertMessage( msg, sz, d.skippedCtrs );

					if ( logData->nextToAdd == &d )
					{
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, d.next == nullptr ); 
						logData->nextToAdd = nullptr;
					}
					else
					{
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, d.next != nullptr ); 
					}
				} // unlocking

				if ( d.next )
				{
					{
						std::unique_lock<std::mutex> lock(d.next->mx);
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, !d.next->canRun );
						d.next->canRun = true;
					}
					d.next->w.notify_one();
				}
			}
			return ret;
		}

		void writoToLog( ModuleID mid, nodecpp::string s, LogLevel severity );

	public:
		LogTransport( LogBufferBaseData* data ) : logData( data ) {}
		LogTransport( const LogTransport& ) = delete;
		LogTransport& operator = ( const LogTransport& ) = delete;
		LogTransport( LogTransport&& other ) {
			logData = other.logData;
			other.logData = nullptr;
		}
		LogTransport& operator = ( LogTransport&& other )
		{
			logData = other.logData;
			other.logData = nullptr;
			return *this;
		}
		~LogTransport()
		{
		}
	};

	class Log
	{
	public:
		LogLevel level = LogLevel::info;

		nodecpp::vector<LogTransport> transports;
		bool silent = false;

		//TODO::add: format
		//TODO::add: exitOnError

		size_t transportIdx = 0; // for adding purposes in case of clustering

	public:
		Log() {}
		virtual ~Log() {}

		template<class ... Objects>
		void log( ModuleID mid, LogLevel l, nodecpp::string_literal format_str, Objects ... obj ) {
			if ( l <= level ) {
				nodecpp::string msg = nodecpp::format( format_str, obj ... );
				for ( auto& transport : transports )
					transport.writoToLog( mid, msg, l );
			}				 
		}

		template<class ... Objects>
		void log( LogLevel l, nodecpp::string_literal format_str, Objects ... obj ) {
			log( ModuleID( NODECPP_DEFAULT_LOG_MODULE ), l, format_str, obj ... );
		}

		template<class ... Objects>
		void emergency( nodecpp::string_literal format_str, Objects ... obj ) { log( LogLevel::emerg, format_str, obj ... ); }
		template<class ... Objects>
		void alert( nodecpp::string_literal format_str, Objects ... obj ) { log( LogLevel::alert, format_str, obj ... ); }
		template<class ... Objects>
		void critical( nodecpp::string_literal format_str, Objects ... obj ) { log( LogLevel::crit, format_str, obj ... ); }
		template<class ... Objects>
		void error( nodecpp::string_literal format_str, Objects ... obj ) { log( LogLevel::err, format_str, obj ... ); }
		template<class ... Objects>
		void warning( nodecpp::string_literal format_str, Objects ... obj ) { log( LogLevel::warning, format_str, obj ... ); }
		template<class ... Objects>
		void notice( nodecpp::string_literal format_str, Objects ... obj ) { log( LogLevel::notice, format_str, obj ... ); }
		template<class ... Objects>
		void info( nodecpp::string_literal format_str, Objects ... obj ) { log( LogLevel::info, format_str, obj ... ); }
		template<class ... Objects>
		void debug( nodecpp::string_literal format_str, Objects ... obj ) { log( LogLevel::debug, format_str, obj ... ); }

		template<class ... Objects>
		void emergency( ModuleID mid, nodecpp::string_literal format_str, Objects ... obj ) { log( mid, LogLevel::emerg, format_str, obj ... ); }
		template<class ... Objects>
		void alert( ModuleID mid, nodecpp::string_literal format_str, Objects ... obj ) { log( mid, LogLevel::alert, format_str, obj ... ); }
		template<class ... Objects>
		void critical( ModuleID mid, nodecpp::string_literal format_str, Objects ... obj ) { log( mid, LogLevel::crit, format_str, obj ... ); }
		template<class ... Objects>
		void error( ModuleID mid, nodecpp::string_literal format_str, Objects ... obj ) { log( mid, LogLevel::err, format_str, obj ... ); }
		template<class ... Objects>
		void warning( ModuleID mid, nodecpp::string_literal format_str, Objects ... obj ) { log( mid, LogLevel::warning, format_str, obj ... ); }
		template<class ... Objects>
		void notice( ModuleID mid, nodecpp::string_literal format_str, Objects ... obj ) { log( mid, LogLevel::notice, format_str, obj ... ); }
		template<class ... Objects>
		void info( ModuleID mid, nodecpp::string_literal format_str, Objects ... obj ) { log( mid, LogLevel::info, format_str, obj ... ); }
		template<class ... Objects>
		void debug( ModuleID mid, nodecpp::string_literal format_str, Objects ... obj ) { log( mid, LogLevel::debug, format_str, obj ... ); }

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
				data->init( cons );
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
	void LogTransport::writoToLog( ModuleID mid, nodecpp::string s, LogLevel severity ) {
		nodecpp::string msgformatted = mid.id() != nullptr ?
			nodecpp::format( "[{}][{}] {}\n", mid.id(), LogLevelNames[(size_t)severity], s ) :
			nodecpp::format( "[{}] {}\n", LogLevelNames[(size_t)severity], s );
		addMsg( msgformatted.c_str(), msgformatted.size(), severity );
	}
} //namespace nodecpp

#endif // NODECPP_LOGGING_H

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

#ifndef NODECPP_LOG_H
#define NODECPP_LOG_H

#include "common.h"
#include <log.h>



namespace nodecpp::logging_impl {
	extern nodecpp::stdvector<nodecpp::log::Log*> logs;
} // namespace nodecpp::logging_impl

namespace nodecpp {

	using LogLevel = ::nodecpp::log::LogLevel;
	using ModuleID = ::nodecpp::log::ModuleID;

	class Log
	{
		::nodecpp::log::Log* log_ = nullptr;
#ifdef NODECPP_ENABLE_CLUSTERING
		static thread_local size_t ordinalBase;
		size_t ordinal;
#endif // NODECPP_ENABLE_CLUSTERING

	public:
		size_t transportIdx = 0; // for adding purposes in case of clustering

	public:
		Log() {
#ifdef NODECPP_ENABLE_CLUSTERING
			if ( nodecpp::clusterIsMaster() )
			{
				log_ = ::nodecpp::alloc<::nodecpp::log::Log>( 1 );
				nodecpp::logging_impl::logs.push_back( log_ );
				ordinal = nodecpp::logging_impl::logs.size();
			}
			else
			{
				ordinal = ordinalBase++;
				if ( ordinal < nodecpp::logging_impl::logs.size() )
					log_ = nodecpp::logging_impl::logs[ordinal];
			}
#else
			log_ = ::nodecpp::alloc<::nodecpp::log::Log>( 1 );
#endif
		}
		virtual ~Log() {
#ifdef NODECPP_ENABLE_CLUSTERING
			if ( nodecpp::clusterIsMaster() && log_  )
				::nodecpp::dealloc<::nodecpp::log::Log>( log_, 1 );
#else
			if ( log_  )
				::nodecpp::dealloc<::nodecpp::log::Log>( log_, 1 );
#endif
		}

		template<class ... Objects>
		void log( ModuleID mid, LogLevel l, nodecpp::string_literal format_str, Objects ... obj ) {
			if ( log_ )
				log_->log( mid, l, format_str.c_str(), obj... );
		}

		template<class ... Objects>
		void log( LogLevel l, nodecpp::string_literal format_str, Objects ... obj ) {
			log( ModuleID( NODECPP_DEFAULT_LOG_MODULE ), l, format_str.c_str(), obj ... );
		}

		void setLevel( LogLevel l ) { if (log_ ) log_->level = l; }
		void setGuaranteedLevel( LogLevel l )
		{
#ifdef NODECPP_ENABLE_CLUSTERING
			if ( nodecpp::clusterIsMaster() && log_  )
				log_->setGuaranteedLevel( l );
#else
			if ( log_  )
				log_->setGuaranteedLevel( l );
#endif
		}
		void resetGuaranteedLevel() { 
#ifdef NODECPP_ENABLE_CLUSTERING
			if ( nodecpp::clusterIsMaster() && log_  )
				log_->resetGuaranteedLevel();
#else
			if ( log_  )
				log_->resetGuaranteedLevel();
#endif
		}

		template<class ... Objects>
		void fatal( nodecpp::string_literal format_str, Objects ... obj ) { log( LogLevel::fatal, format_str, obj ... ); }
		template<class ... Objects>
		void error( nodecpp::string_literal format_str, Objects ... obj ) { log( LogLevel::err, format_str, obj ... ); }
		template<class ... Objects>
		void warning( nodecpp::string_literal format_str, Objects ... obj ) { log( LogLevel::warning, format_str, obj ... ); }
		template<class ... Objects>
		void info( nodecpp::string_literal format_str, Objects ... obj ) { log( LogLevel::info, format_str, obj ... ); }
		template<class ... Objects>
		void debug( nodecpp::string_literal format_str, Objects ... obj ) { log( LogLevel::debug, format_str, obj ... ); }

		template<class ... Objects>
		void fatal( ModuleID mid, nodecpp::string_literal format_str, Objects ... obj ) { log( mid, LogLevel::fatal, format_str, obj ... ); }
		template<class ... Objects>
		void error( ModuleID mid, nodecpp::string_literal format_str, Objects ... obj ) { log( mid, LogLevel::err, format_str, obj ... ); }
		template<class ... Objects>
		void warning( ModuleID mid, nodecpp::string_literal format_str, Objects ... obj ) { log( mid, LogLevel::warning, format_str, obj ... ); }
		template<class ... Objects>
		void info( ModuleID mid, nodecpp::string_literal format_str, Objects ... obj ) { log( mid, LogLevel::info, format_str, obj ... ); }
		template<class ... Objects>
		void debug( ModuleID mid, nodecpp::string_literal format_str, Objects ... obj ) { log( mid, LogLevel::debug, format_str, obj ... ); }

		void clear() { if (log_ ) log_->clear(); }
		bool add( nodecpp::string path )
		{
#ifdef NODECPP_ENABLE_CLUSTERING
			if ( nodecpp::clusterIsMaster() && log_  )
				return log_->add( path );
#else
			if ( log_  )
				return log_->add( path );
#endif
			return false;
		}

		bool add( FILE* cons ) // TODO: input param is a subject for revision
		{
#ifdef NODECPP_ENABLE_CLUSTERING
			if ( nodecpp::clusterIsMaster() && log_  )
				return log_->add( cons );
#else
			if ( log_  )
				return log_->add( cons );
#endif
			return false;
		}
		//TODO::add: remove()
	};

} //namespace nodecpp

#endif // NODECPP_LOG_H

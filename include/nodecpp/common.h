/* -------------------------------------------------------------------------------
* Copyright (c) 2018, OLogN Technologies AG
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

#ifndef COMMON_H
#define COMMON_H

#ifdef _MSC_VER
//#pragma warning (disable:4800) // forcing value to bool 'true' or 'false' (performance warning)
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define NOMINMAX
#endif // _MSC_VER


#ifdef __GNUC__
#define NODECPP_UNUSED_VAR [[gnu::unused]]
#else
#define NODECPP_UNUSED_VAR 
#endif

#ifndef NODECPP_DEFAULT_LOG_MODULE
#define NODECPP_DEFAULT_LOG_MODULE nullptr
#endif

#include <stdint.h>
namespace nodecpp {
	constexpr uint64_t module_id = 4;
} // namespace ::nodecpp

/*#include <foundation.h>
#include <nodecpp_assert.h>
#include <iibmalloc.h>*/
#include <nodecpp/safe_ptr.h>
#include "awaitable.h"
#include "basic_collections.h"

#include <utility>

#include <vector>
#include <map>
#include <unordered_map>
#include <list>
#include <array>
#include <algorithm>

//#include <cassert>
//#include <fmt/format.h>
//#include "trace.h"
//#include "assert.h"
#include "mallocator.h"

#ifdef NODECPP_RECORD_AND_REPLAY
#include <nodecpp/record_and_replay.h>
#endif // NODECPP_RECORD_AND_REPLAY

namespace nodecpp
{
	constexpr const char* nodecpp_module_id = "nodecpp";
}


template<class T>
using GlobalObjectAllocator = Mallocator<T>;

namespace nodecpp
{
	enum CoroStandardOutcomes { ok, timeout, insufficient_buffer, in_progress, failed };

#ifndef NODECPP_NO_COROUTINES
	using handler_ret_type = ::nodecpp::awaitable<void>;
#else
	using handler_ret_type = void;
#endif // NODECPP_NO_COROUTINES

#ifndef NODECPP_NO_COROUTINES
	using awaitable_handle_t = std::experimental::coroutine_handle<>;
#else
	using awaitable_handle_t = void (*)();
#endif

#ifndef NODECPP_NO_COROUTINES
	handler_ret_type a_timeout(uint32_t ms);
	handler_ret_type a_sleep(uint32_t ms);
#endif

} // namespace nodecpp

namespace nodecpp::net {
	template< class ... args > class ServerTEmitter;
	template< class ... args > class SocketTEmitter;
} // namespace nodecpp::net

namespace nodecpp
{
	template<class DataParentT>
	class DataParent
	{
		DataParentT* parent = nullptr;
	public:
		DataParent() {}
		DataParent(DataParentT* parent_) { parent = parent_; }
		DataParentT* getDataParent() { return parent; }
	};
} // namespace nodecpp

class NodeBase
{
#ifdef NODECPP_RECORD_AND_REPLAY
public: // TODO: just for a while... of course, it should never be public... private only
	nodecpp::record_and_replay_impl::BinaryLog binLog;
#endif // NODECPP_RECORD_AND_REPLAY
public:
	NodeBase() {}
	virtual ~NodeBase() {}
};

#ifdef NODECPP_ENABLE_CLUSTERING
struct ThreadStartupData; // forward declaration
#endif

class RunnableBase
{
#ifdef NODECPP_RECORD_AND_REPLAY
public:
	nodecpp::record_and_replay_impl::BinaryLog::Mode replayMode = nodecpp::record_and_replay_impl::BinaryLog::Mode::not_using;
#endif // NODECPP_RECORD_AND_REPLAY

public:
	RunnableBase() {}
#ifdef NODECPP_ENABLE_CLUSTERING
	virtual void run( bool isMaster, ThreadStartupData* startupData ) = 0;
#else
	virtual void run() = 0;
#endif
};

class RunnableFactoryBase
{
public:
	RunnableFactoryBase() {}
	virtual RunnableBase* create() = 0;
};

void registerFactory( const char* name, RunnableFactoryBase* factory );

template<class RunnableT>
class NodeRegistrator
{
public:
public:
	NodeRegistrator( const char* name )
	{
		class NodeFactory : public RunnableFactoryBase
		{
		public:
			NodeFactory() {}
			virtual RunnableBase* create() override
			{
				return new RunnableT;
			}
		};
		NodeFactory* factory = new NodeFactory;
		registerFactory( name, reinterpret_cast<NodeFactory*>(factory) );
	}
};

/*template<class RunnableT,class Infra>
thread_local Infra* NodeRegistrator<RunnableT,Infra>::infraPtr;*/

extern nodecpp::stdvector<nodecpp::stdstring> argv;
inline const nodecpp::stdvector<nodecpp::stdstring>& getArgv() { return argv; }

#ifdef NODECPP_ENABLE_CLUSTERING
namespace nodecpp {
	extern bool clusterIsMaster();
}
#endif // NODECPP_ENABLE_CLUSTERING

// NODECPP_CHECKER_EXTENSIONS is defined internally by nodecpp-checker tool
#ifdef NODECPP_CHECKER_EXTENSIONS
#define NODECPP_MAY_EXTEND_TO_THIS [[nodecpp::may_extend_to_this]]
#define NODECPP_NO_AWAIT [[nodecpp::no_await]]
#define NODECPP_NAKED_STRUCT [[nodecpp::naked_struct]]
#else
#define NODECPP_MAY_EXTEND_TO_THIS
#define NODECPP_NO_AWAIT
#define NODECPP_NAKED_STRUCT
#endif




#endif //COMMON_H

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

#include "../include/nodecpp/common.h"
#include "q_based_infrastructure.h"

#ifdef NODECPP_ENABLE_ONSTACK_SOFTPTR_COUNTING
thread_local size_t nodecpp::safememory::onStackSafePtrCreationCount; 
thread_local size_t nodecpp::safememory::onStackSafePtrDestructionCount;
#endif // NODECPP_ENABLE_ONSTACK_SOFTPTR_COUNTING
thread_local void* nodecpp::safememory::thg_stackPtrForMakeOwningCall = 0;

#if defined NODECPP_USE_NEW_DELETE_ALLOC
thread_local void** nodecpp::safememory::zombieList_ = nullptr;
#ifndef NODECPP_DISABLE_ZOMBIE_ACCESS_EARLY_DETECTION
thread_local std::map<uint8_t*, size_t, std::greater<uint8_t*>> nodecpp::safememory::zombieMap;
thread_local bool nodecpp::safememory::doZombieEarlyDetection_ = true;
#endif // NODECPP_DISABLE_ZOMBIE_ACCESS_EARLY_DETECTION
#endif // NODECPP_USE_xxx_ALLOC

nodecpp::stdvector<nodecpp::stdstring> argv;

#include "../src/clustering_impl/clustering_impl.h"
#include "SimulationNode.h"

template<class NodeT>
class SimplePollLoop
{
public:
	class Initializer
	{
		ThreadStartupData data;
		friend class SimplePollLoop;
		void acquire() {
			preinitThreadStartupData( data );
		}
	public:
		Initializer() {}
		Initializer( const Initializer& ) = default;
		Initializer& operator = ( const Initializer& ) = default;
		Initializer( Initializer&& ) = default;
		Initializer& operator = ( Initializer&& ) = default;
	};

private:
//	ThreadID commAddress;
	ThreadStartupData loopStartupData;
	bool initialized = false;
	bool entered = false;

	//template<class ThreadStartupDataT, class NodeT>
	template<class NODET, class ThreadStartupDataT>
	static void nodeThreadMain( void* pdata )
	{
		ThreadStartupDataT* sd = reinterpret_cast<ThreadStartupDataT*>(pdata);
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, pdata != nullptr ); 
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, sd->threadCommID.slotId != 0 ); 
		ThreadStartupDataT startupData = *sd;
		nodecpp::stddealloc( sd, 1 );
		setThisThreadDescriptor( startupData );
	#ifdef NODECPP_USE_IIBMALLOC
		g_AllocManager.initialize();
	#endif
		nodecpp::logging_impl::currentLog = startupData.defaultLog;
		nodecpp::logging_impl::instanceId = startupData.threadCommID.slotId;
		nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "starting Node thread with threadID = {}", startupData.threadCommID.slotId );
		/*NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, NodeFactoryMap::getInstance().getFacoryMap()->size() == 1, "Indeed: {}. Current implementation supports exactly 1 node per thread. More nodes is a pending dev", NodeFactoryMap::getInstance().getFacoryMap()->size() );
		for ( auto f : *(NodeFactoryMap::getInstance().getFacoryMap()) )
			f.second->create()->run(false, &startupData);*/
		Runnable<NODET> r;
		r.run( false, &startupData );
	}

public:
	SimplePollLoop() {}
	SimplePollLoop( Initializer i ) { 
		loopStartupData = i.data;
		setThisThreadDescriptor( i.data ); 
#ifdef NODECPP_USE_IIBMALLOC
		g_AllocManager.initialize();
#endif
		nodecpp::logging_impl::currentLog = i.data.defaultLog;
		nodecpp::logging_impl::instanceId = i.data.threadCommID.slotId;
		nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "starting Node thread with threadID = {}", i.data.threadCommID.slotId );
		initialized = true;
	}

	static std::pair<Initializer, ThreadID> getInitializer()
	{
		Initializer i;
		i.acquire();
		return std::make_pair(i, i.data.threadCommID);
	}
	
	void run()
	{
		// note: startup data must be allocated using std allocator (reason: freeing memory will happen at a new thread)
		ThreadStartupData* startupData = nodecpp::stdalloc<ThreadStartupData>(1);
		if ( !initialized )
		{
			preinitThreadStartupData( loopStartupData );
			initialized = true;
		}
		*startupData = loopStartupData;
	//	startupData->IdWithinGroup = listeners.add( startupData->threadCommID );
		size_t threadIdx = startupData->threadCommID.slotId;
	//	nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id),"about to start Listener thread with threadID = {} and listenerID = {}...", threadIdx, startupData->IdWithinGroup );
		nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id),"about to start Listener thread with threadID = {}...", threadIdx );
		nodeThreadMain<NodeT, ThreadStartupData>( (void*)(startupData) );
		nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id),"...starting Listener thread with threadID = {} completed at Master thread side", threadIdx );
	}

	ThreadID getAddress() { return loopStartupData.threadCommID; }
};

template<class NodeT, class ThreadStartupDataT>
void nodeThreadMain( void* pdata )
{
	ThreadStartupDataT* sd = reinterpret_cast<ThreadStartupDataT*>(pdata);
	NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, pdata != nullptr ); 
//	NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, sd->threadCommID.slotId != 0 ); 
	ThreadStartupDataT startupData = *sd;
	nodecpp::stddealloc( sd, 1 );
	/* TODO: move to SimplePollLoop<>
//	setThisThreadDescriptor( startupData ); 
#ifdef NODECPP_USE_IIBMALLOC
	g_AllocManager.initialize();
#endif
	nodecpp::logging_impl::currentLog = startupData.defaultLog;
	nodecpp::logging_impl::instanceId = startupData.threadCommID.slotId;
	nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "starting Node thread with threadID = {}", startupData.threadCommID.slotId );*/
	/*NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, NodeFactoryMap::getInstance().getFacoryMap()->size() == 1, "Indeed: {}. Current implementation supports exactly 1 node per thread. More nodes is a pending dev", NodeFactoryMap::getInstance().getFacoryMap()->size() );
	for ( auto f : *(NodeFactoryMap::getInstance().getFacoryMap()) )
		f.second->create()->run(false, &startupData);*/
	SimplePollLoop<NodeT> r( startupData );
//	r.run( false, &startupData );
	r.run();
}

template<class NodeT>
ThreadID runNodeInAnotherThread()
{
	auto startupDataAndAddr = SimplePollLoop<NodeT>::getInitializer();
	using InitializerT = typename SimplePollLoop<NodeT>::Initializer;
	InitializerT* startupData = nodecpp::stdalloc<InitializerT>(1);
	*startupData = startupDataAndAddr.first;
	size_t threadIdx = startupDataAndAddr.second.slotId;
	nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id),"about to start Listener thread with threadID = {}...", threadIdx );
	std::thread t1( nodeThreadMain<NodeT, InitializerT>, (void*)(startupData) );
	// startupData is no longer valid
	startupData = nullptr;
	t1.detach();
	nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id),"...starting Listener thread with threadID = {} completed at Master thread side", threadIdx );
	return startupDataAndAddr.second;
}

int main( int argc, char *argv_[] )
{
	for ( int i=0; i<argc; ++i )
		argv.push_back( argv_[i] );

	auto addr = runNodeInAnotherThread<SampleSimulationNode>();
		
	nodecpp::platform::internal_msg::InternalMsg imsg;
	imsg.append( "Second message", sizeof("Second message") );
//		sendInterThreadMsg( std::move( imsg ), InterThreadMsgType::ConnAccepted, targetThreadId );
	ThreadID target;
	target.slotId = 1;
	target.reincarnation = 1;
//		sendInterThreadMsg( std::move( imsg ), InterThreadMsgType::Infrastructural, loop.getAddress() );
	sendInterThreadMsg( std::move( imsg ), InterThreadMsgType::Infrastructural, addr );

	SimplePollLoop<SampleSimulationNode> loop2;
	loop2.run();

	return 0;
}

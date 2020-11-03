// User-defined sample code
///////////////////////////

#include <common.h>
#include "q_based_infrastructure.h"
#include "SimulationNode.h"


template<class NodeT, class ThreadStartupDataT>
void nodeThreadMain( void* pdata )
{
	ThreadStartupDataT* sd = reinterpret_cast<ThreadStartupDataT*>(pdata);
	NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, pdata != nullptr ); 
	ThreadStartupDataT startupData = *sd;
	nodecpp::stddealloc( sd, 1 );
	SimplePollLoop<NodeT> r( startupData );
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

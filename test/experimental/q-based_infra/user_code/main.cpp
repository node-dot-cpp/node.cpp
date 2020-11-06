// User-defined sample code
///////////////////////////

#include <common.h>
#include <infrastructure/q_based_infrastructure.h>
#include "SimulationNode.h"

// NOTE: next two calls can be viewed as helper functions in case a loop is to be run in a separate thread with QueueBasedNodeLoop
//       Unless further customization is required they can be used as-is with desired Node type (see main() for caling code sample

template<class NodeT, class ThreadStartupDataT>
void nodeThreadMain( void* pdata )
{
	ThreadStartupDataT* sd = reinterpret_cast<ThreadStartupDataT*>(pdata);
	NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, pdata != nullptr ); 
	ThreadStartupDataT startupData = *sd;
	nodecpp::stddealloc( sd, 1 );
	QueueBasedNodeLoop<NodeT> r( startupData );
	r.init();
	r.run();
}

template<class NodeT>
NodeAddress runNodeInAnotherThread()
{
	auto startupDataAndAddr = QueueBasedNodeLoop<NodeT>::getInitializer(useQueuePostman()); // TODO: consider implementing q-based Postman (as lib-defined)
	using InitializerT = typename QueueBasedNodeLoop<NodeT>::Initializer;
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

#include <Windows.h>
int main( int argc, char *argv_[] )
{
	for ( int i=0; i<argc; ++i )
		argv.push_back( argv_[i] );

	auto addr = runNodeInAnotherThread<SampleSimulationNode>();
		
	nodecpp::platform::internal_msg::InternalMsg imsg;
	imsg.append( "Second message", sizeof("Second message") );
	postInterThreadMsg( std::move( imsg ), InterThreadMsgType::Infrastructural, addr );

	class Postman : public InterThreadMessagePostmanBase
	{
		NoNodeLoop<SampleSimulationNode>& loop;
	public: 
		Postman( NoNodeLoop<SampleSimulationNode>& loop_ ) : loop( loop_ ) {}
		void postMessage( InterThreadMsg&& msg ) override
		{
			auto riter = msg.msg.getReadIter();
			printf( "Postman: \"%s\"\n", riter.read( riter.availableSize()) );
		}
	};
	NoNodeLoop<SampleSimulationNode> loop2;
	Postman p( loop2 );
	int waitTime = loop2.init(&p);
	auto addr2 = loop2.getAddress();
	for (;;)
	{
		nodecpp::platform::internal_msg::InternalMsg imsg2;
		imsg.append( "Third message", sizeof("Third message") );
//		waitTime = loop2.onInfrastructureMessage( InterThreadMsg( std::move( imsg ), InterThreadMsgType::Infrastructural, NodeAddress(), NodeAddress() ) );
		postInterThreadMsg( std::move( imsg ), InterThreadMsgType::Infrastructural, addr2 );
		imsg.append( "Forth message", sizeof("Forth message") );
        loop2.onInfrastructureMessage( InterThreadMsg( std::move( imsg ), InterThreadMsgType::Infrastructural, NodeAddress(), NodeAddress() ) );
		if ( waitTime > 0 )
		Sleep( waitTime );
		imsg.append( "Second message", sizeof("Second message") );
		postInterThreadMsg( std::move( imsg ), InterThreadMsgType::Infrastructural, addr );
		Sleep(300);
	}
//	

	return 0;
}

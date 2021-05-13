// User-defined sample code
///////////////////////////

#include <nodecpp/common.h>
#include <infrastructure/q_based_infrastructure.h>
#include <chrono>
#include <thread>
#include "PublisherNode.h"
#include "SubscriberNode.h"
#include <infrastructure/node_thread_creation.h>


int main( int argc, char *argv_[] )
{
#ifdef NODECPP_USE_GMQUEUE
	using BufferT = GMQueueStatePublisherSubscriberTypeInfo::BufferT;
	using ComposerT = GMQueueStatePublisherSubscriberTypeInfo::ComposerT;

	// Init GMGueue
	gmqueue.template initStateConcentratorFactory<mtest::StateConcentratorFactory<BufferT, ComposerT>>();
	gmqueue.setAuthority( "" );
#endif

	for ( int i=0; i<argc; ++i )
		argv.push_back( argv_[i] );

	auto addr = runNodeInAnotherThread<PublisherNode>( useQueuePostman(), PublisherNodeName );
		
	auto startupDataAndAddr = QueueBasedNodeLoop<SubscriberNode>::getInitializer(useQueuePostman()); // TODO: consider implementing q-based Postman (as lib-defined)
	using InitializerT = typename QueueBasedNodeLoop<SubscriberNode>::Initializer;
	InitializerT startupData;
	startupData = startupDataAndAddr.first;
	size_t threadIdx = startupDataAndAddr.second.slotId;
#ifdef NODECPP_USE_GMQUEUE
	nodecpp::GMQThreadQueueTransport<GMQueueStatePublisherSubscriberTypeInfo> transport4node( gmqueue, threadQueues[threadIdx].queue, 0 ); // NOTE: recipientID = 0 is by default; TODO: revise
	startupData.transportData = transport4node.makeTransferrable();
#endif
	QueueBasedNodeLoop<SubscriberNode> r( startupData );
	r.init();
	r.run();

	return 0;
}

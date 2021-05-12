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


	/*nodecpp::platform::internal_msg::InternalMsg imsg;
	imsg.append( "Second message", sizeof("Second message") );
	postInterThreadMsg( std::move( imsg ), InterThreadMsgType::Infrastructural, addr );

	class Postman : public InterThreadMessagePostmanBase
	{
		NoNodeLoop<PublisherNode>& loop;
	public: 
		Postman( NoNodeLoop<PublisherNode>& loop_ ) : loop( loop_ ) {}
		void postMessage( InterThreadMsg&& msg ) override
		{
			auto riter = msg.msg.getReadIter();
			printf( "Postman: \"%s\"\n", riter.directRead( riter.directlyAvailableSize()) );
		}
	};
	NoNodeLoop<PublisherNode> loop2;
	Postman p( loop2 );
	int waitTime = loop2.init(1, &p);
	auto addr2 = loop2.getAddress();
	for (;;)
	{
		nodecpp::platform::internal_msg::InternalMsg imsg2;
		imsg.append( "Third message", sizeof("Third message") );

		auto ptr = imsg.convertToPointer();
		nodecpp::platform::internal_msg::InternalMsg imsg3;
		imsg3.restoreFromPointer( ptr );

//		waitTime = loop2.onInfrastructureMessage( InterThreadMsg( std::move( imsg ), InterThreadMsgType::Infrastructural, NodeAddress(), NodeAddress() ) );
		postInterThreadMsg( std::move( imsg3 ), InterThreadMsgType::Infrastructural, addr2 );
		imsg3.append( "Forth message", sizeof("Forth message") );
        loop2.onInfrastructureMessage( InterThreadMsg( std::move( imsg3 ), InterThreadMsgType::Infrastructural, NodeAddress(), NodeAddress() ) );
		if ( waitTime > 0 )
		std::this_thread::sleep_for(std::chrono::milliseconds( waitTime ));
		imsg3.append( "Second message", sizeof("Second message") );
		postInterThreadMsg( std::move( imsg3 ), InterThreadMsgType::Infrastructural, addr );
		std::this_thread::sleep_for(std::chrono::milliseconds(300));
	}*/
//	

	return 0;
}

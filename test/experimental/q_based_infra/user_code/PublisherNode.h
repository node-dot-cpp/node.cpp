// NetSocket.h : sample of user-defined code for an http server

#ifndef PUBLISHER_NODE_H
#define PUBLISHER_NODE_H

#include <nodecpp/common.h>
#include <log.h>
#include <nodecpp/fs.h>
#include "generated/publishable_state.h"

constexpr const char* PublisherNodeName = "PublisherNode";

using namespace nodecpp;

class PublisherNode : public NodeBase
{
	using PoolType = globalmq::marshalling::MetaPool<GMQueueStatePublisherSubscriberTypeInfo>;
	PoolType mqPool;
	struct PublishedState
	{
		std::string text;
		size_t id = 0;
	};
	mtest::publishable_sample_NodecppWrapperForPublisher<PublishedState, PoolType> publishedStateWrapper;

	log::Log log;

	int ctr = 0;

	void republish() // sample code with timeouts
	{
		publishedStateWrapper.set_id( ++ctr );
		publishedStateWrapper.set_text( fmt::format( " ---> {}. Hello Node.Cpp!", ctr ) );

		// GlobalMQ: at the end of each handler cause pools to post all updates
		mqPool.postAllUpdates();

		setTimeout( [this]() { 
			republish();
		}, 500 );
	}


public:
	PublisherNode() : publishedStateWrapper( mqPool ) {}

	handler_ret_type main()
	{
		log.level = log::LogLevel::info;
		log.add( stdout );
		logging_impl::currentLog = &log;

		mqPool.setTransport( getTransport() );

		// TODO: place some code here, for instance...

		republish();

		CO_RETURN;
	}

	/*void onInfrastructureMessage( NodeAddress requestingThreadId, nodecpp::platform::internal_msg::InternalMsg& msg )
	{
		nodecpp::platform::internal_msg::InternalMsg::ReadIter riter = msg.getReadIter();
		printf( "Got it!\n%s\n", riter.directRead( riter.directlyAvailableSize() ) );

		// GlobalMQ: at the end of each handler cause pools to post all updates
		mqPool.postAllUpdates();
	}*/

	void onGlobalMQMessage( Message& msg )
	{
		mqPool.onMessage( msg );
		// GlobalMQ: at the end of each handler cause pools to post all updates
		mqPool.postAllUpdates();
	}
};

#endif // PUBLISHER_NODE_H

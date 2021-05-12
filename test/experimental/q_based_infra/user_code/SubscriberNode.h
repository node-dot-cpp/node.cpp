// NetSocket.h : sample of user-defined code for an http server

#ifndef SUBSCRIBER_NODE_H
#define SUBSCRIBER_NODE_H

#include <nodecpp/common.h>
#include <log.h>
#include <nodecpp/fs.h>
#include "generated/publishable_state.h"

constexpr const char* SubscriberNodeName = "SubscriberNode";

using namespace nodecpp;

class SubscriberNode : public NodeBase
{
	using PoolType = globalmq::marshalling::MetaPool<GMQueueStatePublisherSubscriberTypeInfo>;
	PoolType mqPool;
	struct SubscriptionState
	{
		std::string text;
		size_t id = 0;
		void notifyUpdated_id() const { log::default_log::log( log::LogLevel::fatal, "id = {}\n", id ); }
		void notifyUpdated_text() const { log::default_log::log( log::LogLevel::fatal, "text = {}\n", text ); }
	};
	mtest::publishable_sample_NodecppWrapperForSubscriber<SubscriptionState, PoolType> subscribedStateWrapper;

	log::Log log;

public:
	SubscriberNode() : subscribedStateWrapper( mqPool ) {}

	handler_ret_type main()
	{
		log.level = log::LogLevel::info;
		log.add( stdout );
		logging_impl::currentLog = &log;

		mqPool.setTransport( getTransport() );

		globalmq::marshalling::GmqPathHelper::PathComponents pc;
		pc.authority = "";
		pc.nodeName = "PublisherNode";
		pc.statePublisherName = mtest::publishable_sample_NodecppWrapperForSubscriber<SubscriptionState, PoolType>::stringTypeID;
		GMQ_COLL string path = globalmq::marshalling::GmqPathHelper::compose( pc );

		subscribedStateWrapper.subscribe( path );

		CO_RETURN;
	}

	void onGlobalMQMessage( Message& msg )
	{
		mqPool.onMessage( msg );
		// GlobalMQ: at the end of each handler cause pools to post all updates
		mqPool.postAllUpdates();
	}
};

#endif // SUBSCRIBER_NODE_H

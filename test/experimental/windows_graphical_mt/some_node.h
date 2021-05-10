// some_node.h : sample of user-defined code for an http server

#ifndef SOME_NODE_H
#define SOME_NODE_H

#include <nodecpp/common.h>
#include <log.h>
#include <nodecpp/fs.h>
#include "nodecpp_publish_subscribe_support.h"
#include "marshalling/wg_marshalling.h"

using namespace nodecpp;

constexpr const char* SomeNodeName = "SomeNode";
class SomeNode : public NodeBase
{
	PoolForWorkerThreadT mqPool;
	struct Point
	{
		int x;
		int y;
	};
	struct MyPublishingState
	{
		Point screenPoint;
	};
	mtest::publishable_sample_NodecppWrapperForPublisher<MyPublishingState, PoolForWorkerThreadT> myPublishingStateWrapper;

	log::Log log;
	Point pt;

	void processPoint( NodeAddress srcNodeAddress, Point pt )
	{
		Message msg;
		mtest::infrastructural::composeMessage<mtest::infrastructural::ScreenPoint>( msg, mtest::x = pt.y, mtest::y = pt.x ); // just swap the coords
		postInfrastructuralMsg( std::move( msg ), srcNodeAddress );
	}

	void processPointAsPublishedObject( Point pt )
	{
		myPublishingStateWrapper.get4set_screenPoint().set_x( pt.x );
		myPublishingStateWrapper.get4set_screenPoint().set_y( pt.y );
	}

public:
	SomeNode() : myPublishingStateWrapper( mqPool ) {}

	handler_ret_type main()
	{
		log.level = log::LogLevel::info;
		log.add( stdout );
		logging_impl::currentLog = &log;

		mqPool.setTransport( getTransport() );

		CO_RETURN;
	}

	void onInfrastructureMessage( NodeAddress srcNodeAddress, Message& msg )
	{
		mtest::infrastructural::handleMessage( msg,
			mtest::makeMessageHandler<mtest::infrastructural::ScreenPoint>([&](auto& parser){ 
				mtest::STRUCT_ScreenPoint_parse( parser, mtest::x = &(pt.x), mtest::y = &(pt.y) );
//				processPoint( srcNodeAddress, pt );
				processPointAsPublishedObject( pt );
			}),
			mtest::makeDefaultMessageHandler([&](auto& parser, uint64_t msgID){ fmt::print( "Unhandled message {}\n", msgID ); })
		);

		// GlobalMQ: at the end of each handler cause pools to post all updates
		mqPool.postAllUpdates();
	}

	void onGlobalMQMessage( NodeAddress srcNodeAddress, Message& msg )
	{
		mqPool.onMessage( msg/*, srcNodeAddress*/ );
		// GlobalMQ: at the end of each handler cause pools to post all updates
		mqPool.postAllUpdates();
	}
};

#endif // SOME_NODE_H

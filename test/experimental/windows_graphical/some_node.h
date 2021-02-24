// some_node.h : sample of user-defined code for an http server

#ifndef SOME_NODE_H
#define SOME_NODE_H

#include <nodecpp/common.h>
#include <log.h>
#include <nodecpp/fs.h>
#include "marshalling/wg_marshalling.h"

using namespace nodecpp;


class SomeNode : public NodeBase
{
	log::Log log;
	struct Point
	{
		int x;
		int y;
	};
	Point pt;

	void processPoint( NodeAddress requestingThreadId, Point pt )
	{
		Message msg;
		m::infrastructural::composeMessage<m::infrastructural::ScreenPoint>( msg, m::x = pt.y, m::y = pt.x ); // just swap the coords
		postInfrastructuralMsg( std::move( msg ), requestingThreadId );
		setTimeout( [pt, requestingThreadId]() { 
			Message msg;
			m::infrastructural::composeMessage<m::infrastructural::ScreenPoint>( msg, m::x = pt.x, m::y = pt.y ); // just swap the coords
			postInfrastructuralMsg( std::move( msg ), requestingThreadId );
		}, 1000 );
	}

public:
	handler_ret_type main()
	{
		log.level = log::LogLevel::info;
		log.add( stdout );
		logging_impl::currentLog = &log;

		CO_RETURN;
	}

	void onInfrastructureMessage( NodeAddress requestingThreadId, Message& msg )
	{
		m::infrastructural::handleMessage( msg,
			m::makeMessageHandler<m::infrastructural::ScreenPoint>([&](auto& parser){ 
				m::STRUCT_ScreenPoint_parse( parser, m::x = &(pt.x), m::y = &(pt.y) );
				processPoint( requestingThreadId, pt );
			}),
			m::makeDefaultMessageHandler([&](auto& parser, uint64_t msgID){ fmt::print( "Unhandled message {}\n", msgID ); })
		);
	}
};

#endif // SOME_NODE_H

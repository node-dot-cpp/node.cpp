// NetSocket.h : sample of user-defined code for an http server

#ifndef HTTP_SERVER_SAMPLE_H
#define HTTP_SERVER_SAMPLE_H

#include <nodecpp/common.h>
#include <log.h>

using namespace nodecpp;

class MySampleTNode : public NodeBase
{
	nodecpp::log::Log log;

	int ctr = 5;

	void setHelloOrExit()
	{
		if ( ctr )
		{
			nodecpp::log::default_log::log( nodecpp::log::LogLevel::fatal, "{}. Hello Node.Cpp!", ctr );
			setTimeout( [this]() { 
				setHelloOrExit();
			}, 500 );
		}
		else
			nodecpp::log::default_log::log( nodecpp::log::LogLevel::fatal, "{}................", ctr );
		--ctr;
	}


public:
	nodecpp::handler_ret_type main()
	{
		log.level = nodecpp::log::LogLevel::info;
		log.add( stdout );
		nodecpp::logging_impl::currentLog = &log;

		// TODO: place some code here, for instance...

		for ( size_t i=0; i<3; ++i )
		{
			co_await a_sleep(300);
			nodecpp::log::default_log::log( nodecpp::log::LogLevel::fatal, "sleeping..." );
		}

		setHelloOrExit();
	}
};

#endif // HTTP_SERVER_SAMPLE_H

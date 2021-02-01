// NetSocket.h : sample of user-defined code for an http server

#ifndef SAMPLE_SIMULATION_NODE_H
#define SAMPLE_SIMULATION_NODE_H

#include <nodecpp/common.h>
#include <log.h>
#include <nodecpp/fs.h>

using namespace nodecpp;

class SampleSimulationNode : public NodeBase
{
	log::Log log;

	int ctr = 5;

	void setHelloOrExit() // sample code with timeouts
	{
		if ( ctr )
		{
			log::default_log::log( log::LogLevel::info, "{}. Hello Node.Cpp!", ctr );
			setTimeout( [this]() { 
				setHelloOrExit();
			}, 500 );
		}
		else
			log::default_log::log( log::LogLevel::fatal, "{}................", ctr );
		--ctr;
	}


public:
	handler_ret_type main()
	{
		log.level = log::LogLevel::info;
		log.add( stdout );
		logging_impl::currentLog = &log;

		// TODO: place some code here, for instance...

		for ( size_t i=0; i<3; ++i )
		{
			co_await a_sleep(300);
			log::default_log::log( log::LogLevel::fatal, "sleeping..." );
		}

		setHelloOrExit();

		string_literal path( "../user_code/main.cpp" );
		auto file = fs::openSync( path, std::optional<string>(), std::optional<string>() );
		Buffer b;
		size_t sz = fs::readSync( file, b, 0, 50, std::optional<size_t>(3) );
		b.appendUint8( 0 );
		log::default_log::log( log::LogLevel::fatal, "{}", b.begin() );
		log::default_log::log( log::LogLevel::fatal, "=========" );

		b = fs::readFileSync( path );
		log::default_log::log( log::LogLevel::fatal, "{}", b.begin() );
		log::default_log::log( log::LogLevel::fatal, "=========" );

		nodecpp::platform::internal_msg::InternalMsg imsg;
		imsg.append( "First message", sizeof("First message") );
//		postInterThreadMsg( std::move( imsg ), InterThreadMsgType::ConnAccepted, targetThreadId );
		NodeAddress target;
		target.slotId = 1;
		target.reincarnation = 1;
		postInterThreadMsg( std::move( imsg ), InterThreadMsgType::Infrastructural, target );
	}

	void onInfrastructureMessage( NodeAddress requestingThreadId, nodecpp::platform::internal_msg::InternalMsg& msg )
	{
		nodecpp::platform::internal_msg::InternalMsg::ReadIter riter = msg.getReadIter();
		printf( "Got it!\n%s\n", riter.directRead( riter.directlyAvailableSize() ) );
	}
};

#endif // SAMPLE_SIMULATION_NODE_H

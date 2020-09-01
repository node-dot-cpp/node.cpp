// NetSocket.h : sample of user-defined code for an http server

#ifndef HTTP_SERVER_SAMPLE_H
#define HTTP_SERVER_SAMPLE_H

#include <nodecpp/common.h>
#include <log.h>
#include <nodecpp/fs.h>

using namespace nodecpp;

class MySampleTNode : public NodeBase
{
	log::Log log;

	int ctr = 5;

	void setHelloOrExit()
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

		string_literal path( "../user_code/SimulationNode.cpp" );
		auto file = fs::FS::openSync( path, std::optional<string>(), std::optional<string>() );
		Buffer b;
		size_t sz = fs::FS::readSync( file, b, 0, 1024, std::optional<size_t>() );
		b.appendUint8( 0 );
		log::default_log::log( log::LogLevel::fatal, "{}", b.begin() );
		log::default_log::log( log::LogLevel::fatal, "=========" );
		b = fs::FS::readFileSync( path );
		log::default_log::log( log::LogLevel::fatal, "{}", b.begin() );
		log::default_log::log( log::LogLevel::fatal, "=========" );
	}
};

#endif // HTTP_SERVER_SAMPLE_H

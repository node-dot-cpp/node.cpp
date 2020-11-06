// some_node.h : sample of user-defined code for an http server

#ifndef SOME_NODE_H
#define SOME_NODE_H

#include <nodecpp/common.h>
#include <log.h>
#include <nodecpp/fs.h>

using namespace nodecpp;


class SomeNode : public NodeBase
{
	log::Log log;
	HWND hWnd = 0;

public:
	enum MsgTypes { set_wnd, input_point, input_point3d };

public:
	handler_ret_type main()
	{
		log.level = log::LogLevel::info;
		log.add( stdout );
		logging_impl::currentLog = &log;

		CO_RETURN;
	}

	void onInfrastructureMessage( NodeAddress requestingThreadId, nodecpp::platform::internal_msg::InternalMsg::ReadIter& riter )
	{
		// TODO: regular parsing
		uint32_t msgID = *(uint32_t*)(riter.read( 4 ) );
		switch ( msgID )
		{
			case set_wnd:
				hWnd = *(HWND*)(riter.read( sizeof( HWND ) ) );
				break;
			case input_point:
			{
				uint32_t x = *(uint32_t*)(riter.read( 4 ) );
				uint32_t y = *(uint32_t*)(riter.read( 4 ) );
				auto f = fmt::format( "x = {}\ny = {}", x, y );
				MessageBox( hWnd, f.c_str(), "Point", MB_OK );
				break;
			}
			case input_point3d:
			{
				uint32_t x = *(uint32_t*)(riter.read( 4 ) );
				uint32_t y = *(uint32_t*)(riter.read( 4 ) );
				uint32_t z = *(uint32_t*)(riter.read( 4 ) );
				auto f = fmt::format( "x = {}\ny = {}\nz = {}", x, y, z );
				MessageBox( hWnd, f.c_str(), "Point", MB_OK );
				break;
			}
			default:
			{
				auto f = fmt::format( "msgId = {}, remaining size = {}", msgID, riter.availableSize() );
				MessageBox( 0, f.c_str(), "--???--", MB_OK );
				break;
			}
		}
	}
};

#endif // SOME_NODE_H

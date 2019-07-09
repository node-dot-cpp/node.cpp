// NetSocket.h : sample of user-defined code

#ifndef NET_SOCKET_H
#define NET_SOCKET_H

#include <nodecpp/common.h>
#include <nodecpp/socket_type_list.h>
#include <nodecpp/socket_t_base.h>


using namespace nodecpp;
using namespace fmt;

class MySampleTNode : public NodeBase
{
	using SocketIdType = int;

public:
	MySampleTNode()
	{
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "MySampleTNode::MySampleTNode()" );
	}

	virtual nodecpp::handler_ret_type main()
	{
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "MySampleLambdaOneNode::main()" );

		clientSock = nodecpp::net::createSocket<ClientSockType>();
		*( clientSock->getExtra() ) = 17;
		clientSock->connect(2001, "127.0.0.1");

		CO_RETURN;
	}

	using ClientSockBaseType = nodecpp::net::SocketBase;

	class MySocketOne : public ClientSockBaseType
	{
		struct Stats
		{
			uint64_t recvSize = 0;
			uint64_t sentSize = 0;
			uint64_t rqCnt;
			uint64_t connCnt = 0;
		};
		void printStats(const Stats& stats)
		{
			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "{}, {}, {}, {}, {}", infraGetCurrentTime(), stats.connCnt, stats.recvSize, stats.sentSize, stats.rqCnt );
		}

		size_t recvSize = 0;
		size_t recvReplies = 0;
		size_t sentSize = 0;
		std::unique_ptr<uint8_t> ptr;
		size_t size = 64;// * 1024;
		bool letOnDrain = false;

	public:
		using NodeType = MySampleTNode;

	private:
//		Buffer buf;
		int extraData;

	public:
		MySocketOne() {
			ptr.reset(static_cast<uint8_t*>(malloc(size)));
		}
		virtual ~MySocketOne() {}

		int* getExtra() { return &extraData; }

		void onWhateverConnect() 
		{
			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "MySampleTNode::onWhateverConnect(), extra = {}", *(getExtra()) );

			uint8_t* buff = ptr.get();
			buff[0] = 2;
			buff[1] = 1;
			write(buff, 2);
		}
		void onWhateverData(nodecpp::Buffer& buffer)
		{
			if ( buffer.size() < sizeof( Stats ) )
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "{}, Failure (expected {} bytes, received {} bytes", infraGetCurrentTime(), sizeof( Stats ), buffer.size() );
			else
				printStats( *reinterpret_cast<Stats*>( buffer.begin() ) );
		
			getchar();

			++recvReplies;
			recvSize += buffer.size();
			uint8_t* buff = ptr.get();
			buff[0] = 2;
			buff[1] = (uint8_t)recvReplies | 1;
			write(buff, 2);
		}
	};

	using ClientSockType = MySocketOne;

	using clientConnect_1 = nodecpp::net::HandlerData<MySocketOne, &MySocketOne::onWhateverConnect>;
	using clientConnect = nodecpp::net::SocketHandlerDataList<MySocketOne, clientConnect_1>;
	using clientData_1 = nodecpp::net::HandlerData<MySocketOne, &MySocketOne::onWhateverData>;
	using clientData = nodecpp::net::SocketHandlerDataList<MySocketOne, clientData_1>;
	using clientSocketHD = nodecpp::net::SocketHandlerDescriptor< MySocketOne, nodecpp::net::SocketHandlerDescriptorBase<nodecpp::net::OnConnectT<clientConnect>, nodecpp::net::OnDataT<clientData> > >;

	using EmitterType = nodecpp::net::SocketTEmitter<clientSocketHD>;

	nodecpp::safememory::owning_ptr<ClientSockType> clientSock;
};

#endif // NET_SOCKET_H

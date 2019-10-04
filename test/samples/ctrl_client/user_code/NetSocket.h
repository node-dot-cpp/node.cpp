// NetSocket.h : sample of user-defined code

#ifndef NET_SOCKET_H
#define NET_SOCKET_H

#include <nodecpp/common.h>
#include <nodecpp/socket_type_list.h>


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

	public:
		using NodeType = MySampleTNode;

	private:
		Buffer sendBuff;
		int extraData;

	public:
		MySocketOne() {
			sendBuff.reserve( 2 );
			sendBuff.set_size( 2 );
		}
		virtual ~MySocketOne() {}

		int* getExtra() { return &extraData; }

		nodecpp::handler_ret_type onWhateverConnect() 
		{
			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "MySampleTNode::onWhateverConnect(), extra = {}", *(getExtra()) );

			sendBuff.begin()[0] = 2;
			sendBuff.begin()[1] = 1;
			write(sendBuff);

			CO_RETURN;
		}
		nodecpp::handler_ret_type onWhateverData(nodecpp::Buffer& buffer)
		{
			if ( buffer.size() < sizeof( Stats ) )
			{
				if ( dataForCommandProcessing.state == net::SocketBase::DataForCommandProcessing::LocalEnded && buffer.size() == 10 )
				{
					nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "   server finaly says: {}", (char*)(buffer.begin()) );
					CO_RETURN;
				}
				else
					nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "{}, Failure (expected {} bytes, received {} bytes", infraGetCurrentTime(), sizeof( Stats ), buffer.size() );
			}
			else
				printStats( *reinterpret_cast<Stats*>( buffer.begin() ) );
		
			++recvReplies;

#ifdef AUTOMATED_TESTING_ONLY
			if ( recvReplies > 3 )
			{
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "About to exit successfully in automated testing" );
				// test just once
				end();
				unref();
				CO_RETURN;
			}
			co_await nodecpp::a_timeout(1000);
#else
			getchar();
#endif

			recvSize += buffer.size();
			sendBuff.begin()[0] = 2;
			sendBuff.begin()[1] = (uint8_t)recvReplies | 1;
			write(sendBuff);

			CO_RETURN;
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

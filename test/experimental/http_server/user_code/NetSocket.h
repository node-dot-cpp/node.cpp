// NetSocket.h : sample of user-defined code

#ifndef NET_SOCKET_H
#define NET_SOCKET_H


#include <nodecpp/common.h>
#include <nodecpp/socket_type_list.h>
#include <nodecpp/server_type_list.h>

#include <algorithm>
#include <cctype>
#include <string>

using namespace std;
using namespace nodecpp;
using namespace fmt;

class MySampleTNode : public NodeBase
{
	struct Stats
	{
		uint64_t recvSize = 0;
		uint64_t sentSize = 0;
		uint64_t rqCnt;
		uint64_t connCnt = 0;
	};
	Stats stats;

	Buffer ctrlReplyBuff;

public:

#ifdef AUTOMATED_TESTING_ONLY
	bool stopAccepting = false;
	bool stopResponding = false;
	nodecpp::Timeout to;
#endif

	class HttpServer : public nodecpp::net::ServerSocket<MySampleTNode>
	{
	public:
		HttpServer() {}
		HttpServer(MySampleTNode* node) : nodecpp::net::ServerSocket<MySampleTNode>(node) {}
		virtual ~HttpServer() {}
	};

	class CtrlServer : public nodecpp::net::ServerBase
	{
	public:
		CtrlServer() {}
		virtual ~CtrlServer() {}
	};

	class HttpSocket : public nodecpp::net::SocketBase, public ::nodecpp::DataParent<MySampleTNode>
	{
		class DummyBuffer
		{
			Buffer base;
			size_t currpos = 0;
		public:
			DummyBuffer() : base(0x10000) {}
			void pushFragment(const Buffer& b) { base.append( b ); }
			bool popLine(Buffer& b) 
			{ 
				for ( ; currpos<base.size(); ++currpos )
					if ( *(base.begin() + currpos) == '\n' )
					{
						b.clear();
						b.append( base, 0, currpos+1 );
						base.popFront( currpos+1 );
						currpos = 0;
						return true;
					}
				return false;
			}
		};
		DummyBuffer dbuf;

		class DummyHttpMessage
		{
			static constexpr std::pair<const char *, size_t> MethodNames[] = { 
				std::make_pair( "GET ", sizeof( "GET " ) - 1 ),
				std::make_pair( "HEAD ", sizeof( "HEAD " ) - 1 ),
				std::make_pair( "POST ", sizeof( "POST " ) - 1 ),
				std::make_pair( "PUT ", sizeof( "PUT " ) - 1 ),
				std::make_pair( "DELETE ", sizeof( "DELETE " ) - 1 ),
				std::make_pair( "TRACE ", sizeof( "TRACE " ) - 1 ),
				std::make_pair( "OPTIONS ", sizeof( "OPTIONS " ) - 1 ),
				std::make_pair( "CONNECT ", sizeof( "CONNECT " ) - 1 ),
				std::make_pair( "PATCH ", sizeof( "PATCH " ) - 1 ) };
			static constexpr size_t MethodCount = sizeof( MethodNames ) / sizeof( std::pair<const char *, size_t> );

			struct Method // so far
			{
				std::string name;
				std::string value;
				void clear() { name.clear(); value.clear(); }
			};
			Method method;
			typedef std::map<std::string, std::string> header_t;
			header_t header;
			size_t contentLength = 0;
			nodecpp::Buffer body;
			enum ConnStatus { close, keep_alive };
			ConnStatus connStatus = ConnStatus::keep_alive;
			enum Status { noinit, in_hdr, in_body, completed };
			Status status = Status::noinit;

		private:
			std::string makeLower( std::string& str ) // quick and dirty; TODO: revise (see, for instance, discussion at https://stackoverflow.com/questions/313970/how-to-convert-stdstring-to-lower-case)
			{
				std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c){ return std::tolower(c); });
				return str;
			}

			void setCL()
			{
				auto cl = header.find( "content-length" );
				if ( cl != header.end() )
					contentLength = ::atol( cl->second.c_str() ); // quick and dirty; TODO: revise
				contentLength = 0;
			}

			void setConnStatus()
			{
				auto cs = header.find( "connection" );
				if ( cs != header.end() )
				{
					std::string val = cs->second.c_str();
					val = makeLower( val );
					if ( val == "keep alive" )
						connStatus = ConnStatus::keep_alive;
					else if ( val == "close" )
						connStatus = ConnStatus::close;
				}
				contentLength = 0;
			}

		public:
			DummyHttpMessage() {}
			DummyHttpMessage(const DummyHttpMessage&) = delete;
			DummyHttpMessage operator = (const DummyHttpMessage&) = delete;
			DummyHttpMessage(DummyHttpMessage&& other)
			{
				method = std::move( other.method );
				header = std::move( other.header );
				status = other.status;
				contentLength = other.contentLength;
				other.status = Status::noinit;
			}
			DummyHttpMessage& operator = (DummyHttpMessage&& other)
			{
				method = std::move( other.method );
				header = std::move( other.header );
				status = other.status;
				other.status = Status::noinit;
				contentLength = other.contentLength;
				other.contentLength = 0;
				return *this;
			}
			void clear() // TODO: ensure necessity (added for reuse purposes)
			{
				method.clear();
				header.clear();
				body.clear();
				contentLength = 0;
				status = Status::noinit;
			}

			bool setMethod( const std::string& line )
			{
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, status == Status::noinit ); 
				size_t start = line.find_first_not_of( " \t" );
				if ( start == std::string::npos || line[start] == '\r' || line[start] == '\n' )
					return false;
				for ( size_t i=0; i<MethodCount; ++i )
					if ( line.size() >= MethodNames[i].second + start && memcmp( line.c_str() + start, MethodNames[i].first, MethodNames[i].second ) == 0 ) // TODO: cthink about rfind(*,0)
					{
						method.name = MethodNames[i].first;
						start += MethodNames[i].second;
						start = line.find_first_not_of( " \t", start );
						size_t end = line.find_last_not_of(" \t\r\n" );
						method.value = line.substr( start, end - start );
						status = Status::in_hdr;
						return true;
					}
				return false;
			}

			bool addHeaderEntry( const std::string& line )
			{
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, status == Status::in_hdr ); 
				size_t end = line.find_last_not_of(" \t\r\n" );
				if ( end == std::string::npos )
				{
					if ( !( line.size() == 2 || line[0] == '\r' && line[1] == '\n' ) )
						return true;
					setCL();
					status = contentLength ? Status::in_body : Status::completed;
					return false;
				}
				size_t start = line.find_first_not_of( " \t" );
				size_t idx = line.find(':', start);
				if ( idx >= end )
					return false;
				size_t valStart = line.find_first_not_of( " \t", idx + 1 );
				std::string key = line.substr( start, idx-start );
				header.insert( std::make_pair( makeLower( key ), line.substr( valStart, end - valStart ) ));
				return true;
			}

			size_t getContentLength() const { return contentLength; }

			void dbgTrace()
			{
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "{} {}", method.name, method.value );
				for ( auto& entry : header )
					nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "{}: {}", entry.first, entry.second );
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "[CL = {}, Conn = {}]", getContentLength(), connStatus == ConnStatus::keep_alive ? "keep-alive" : "close" );
			}
		};

		size_t rqCnt = 0;

		nodecpp::handler_ret_type readLine(Buffer& lb)
		{
			nodecpp::Buffer r_buff(0x200);
			while ( !dbuf.popLine( lb ) )
			{
				co_await a_read( r_buff, 2 );
				dbuf.pushFragment( r_buff );
			}

			CO_RETURN;
		}

		nodecpp::handler_ret_type sendReply2()
		{
			Buffer reply;
			std::string replyHeadFormat = "HTTP/1.1 200 OK\r\n"
				"Content-Length: {}\r\n"
				"Content-Type: text/html\r\n"
				"Connection: keep-alive\r\n"
				"\r\n";
			std::string replyHtmlFormat = "<html>\r\n"
				"<body>\r\n"
				"<h1>Get reply! (# {})</h1>\r\n"
				"</body>\r\n"
				"</html>\r\n";
			std::string replyHtml = fmt::format( replyHtmlFormat.c_str(), getDataParent()->stats.rqCnt + 1 );
			std::string replyHead = fmt::format( replyHeadFormat.c_str(), replyHtml.size() );

			std::string r = replyHead + replyHtml;
			reply.append( r.c_str(), r.size() );
			write(reply);
//			end();

			++(getDataParent()->stats.rqCnt);

			CO_RETURN;
		}

		nodecpp::handler_ret_type sendReply()
		{
			Buffer reply;
			std::string replyBegin = "HTTP/1.1 200 OK\r\n"
			"Date: Mon, 27 Jul 2009 12:28:53 GMT\r\n"
			"Server: Apache/2.2.14 (Win32)\r\n"
			"Last-Modified: Wed, 22 Jul 2009 19:15:56 GMT\r\n"
			"Content-Length: 88\r\n"
			"Content-Type: text/html\r\n"
			"Connection: keep-alive\r\n"
			"\r\n\r\n"
			"<html>\r\n"
			"<body>\r\n";
						std::string replyEnd = "</body>\r\n"
			"</html>\r\n"
			"\r\n\r\n";
			std::string replyBody = fmt::format( "<h1>Get reply! (# {})</h1>\r\n", getDataParent()->stats.rqCnt + 1 );
			std::string r = replyBegin + replyBody + replyEnd;
			reply.append( r.c_str(), r.size() );
			write(reply);
			end();

			++(getDataParent()->stats.rqCnt);

			CO_RETURN;
		}

		nodecpp::handler_ret_type getRequest( DummyHttpMessage& message )
		{
			bool ready = false;
			Buffer lb;
			co_await readLine(lb);
			lb.appendUint8( 0 );
			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "line: {}", reinterpret_cast<char*>(lb.begin()) );
			if ( !message.setMethod( std::string( reinterpret_cast<char*>(lb.begin()) ) ) )
			{
				end();
				co_await sendReply();
			}

			do
			{
				lb.clear();
				co_await readLine(lb);
				lb.appendUint8( 0 );
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "line: {}", reinterpret_cast<char*>(lb.begin()) );
			}
			while ( message.addHeaderEntry( std::string( reinterpret_cast<char*>(lb.begin()) ) ) );

			if ( message.getContentLength() )
			{
				lb.clear();
				lb.reserve( message.getContentLength() );
				co_await a_read( lb, message.getContentLength() );
			}

			CO_RETURN;
		}

		nodecpp::handler_ret_type processRequest()
		{
			DummyHttpMessage message;
			co_await getRequest( message );
			message.dbgTrace();

			co_await sendReply();

			CO_RETURN;
		}

	public:
		using NodeType = MySampleTNode;
		friend class MySampleTNode;

	public:
		HttpSocket() {}
		HttpSocket(MySampleTNode* node) : nodecpp::net::SocketBase(), ::nodecpp::DataParent<MySampleTNode>(node) {}
		virtual ~HttpSocket() {}

		nodecpp::handler_ret_type processRequests()
		{
			co_await processRequest();
			++rqCnt;
			CO_RETURN;
		}

	};

	MySampleTNode()
	{
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "MySampleTNode::MySampleTNode()" );
	}


	virtual nodecpp::handler_ret_type main()
	{
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "MySampleLambdaOneNode::main()" );

		nodecpp::net::ServerBase::addHandler<ServerType, nodecpp::net::ServerBase::DataForCommandProcessing::UserHandlers::Handler::Connection, &MySampleTNode::onConnectionx>(this);
		nodecpp::net::ServerBase::addHandler<CtrlServerType, nodecpp::net::ServerBase::DataForCommandProcessing::UserHandlers::Handler::Connection, &MySampleTNode::onConnectionCtrl>(this);

		srv = nodecpp::net::createServer<ServerType, HttpSocket>(this);
		srvCtrl = nodecpp::net::createServer<CtrlServerType, nodecpp::net::SocketBase>();

		srv->listen(2000, "127.0.0.1", 5);
		srvCtrl->listen(2001, "127.0.0.1", 5);

#ifdef AUTOMATED_TESTING_ONLY
		to = std::move( nodecpp::setTimeout(  [this]() { 
			srv->close();
			srv->unref();
			srvCtrl->close();
			srvCtrl->unref();
			stopAccepting = true;
			to = std::move( nodecpp::setTimeout(  [this]() {stopResponding = true;}, 3000 ) );
		}, 3000 ) );
#endif

		CO_RETURN;
	}

	nodecpp::handler_ret_type onConnectionx(nodecpp::safememory::soft_ptr<HttpServer> server, nodecpp::safememory::soft_ptr<nodecpp::net::SocketBase> socket) { 
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server: onConnection()!");
		//srv.unref();
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, socket != nullptr ); 
		soft_ptr<HttpSocket> socketPtr = nodecpp::safememory::soft_ptr_static_cast<HttpSocket>(socket);

		socketPtr->processRequests();

		CO_RETURN;
	}

	nodecpp::handler_ret_type readLine(nodecpp::safememory::soft_ptr<nodecpp::net::SocketBase> socket, Buffer& buffer) {
	}

	nodecpp::handler_ret_type onConnectionCtrl(nodecpp::safememory::soft_ptr<CtrlServer> server, nodecpp::safememory::soft_ptr<net::SocketBase> socket) { 
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server: onConnectionCtrl()!");

		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, socket != nullptr ); 
		nodecpp::Buffer r_buff(0x200);
		for (;;)
		{
#ifdef AUTOMATED_TESTING_ONLY
			if ( stopResponding )
			{
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "About to exit successfully in automated testing (by timer)" );
				socket->end();
				socket->unref();
				break;
			}
#endif
			co_await socket->a_read( r_buff, 2 );
			co_await onDataCtrlServerSocket_(socket, r_buff);
		}
		CO_RETURN;
	}

	using SockTypeServerSocket = nodecpp::net::SocketBase;
	using SockTypeServerCtrlSocket = nodecpp::net::SocketBase;

	using ServerType = HttpServer;
	nodecpp::safememory::owning_ptr<ServerType> srv; 

	using CtrlServerType = CtrlServer;
	nodecpp::safememory::owning_ptr<CtrlServerType>  srvCtrl;

	using EmitterType = nodecpp::net::SocketTEmitter<>;
	using EmitterTypeForServer = nodecpp::net::ServerTEmitter<>;
	nodecpp::handler_ret_type onDataCtrlServerSocket_(nodecpp::safememory::soft_ptr<nodecpp::net::SocketBase> socket, Buffer& buffer) {

		size_t requestedSz = buffer.begin()[1];
		if (requestedSz)
		{
			Buffer reply(sizeof(stats));
			stats.connCnt = srv->getSockCount();
			uint32_t replySz = sizeof(Stats);
			ctrlReplyBuff.clear();
			ctrlReplyBuff.append( &stats, replySz); // naive marshalling will work for a limited number of cases
			socket->write(ctrlReplyBuff);
		}
		CO_RETURN;
	}


};

#endif // NET_SOCKET_H

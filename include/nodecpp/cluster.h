/* -------------------------------------------------------------------------------
* Copyright (c) 2019, OLogN Technologies AG
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of the OLogN Technologies AG nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL OLogN Technologies AG BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
* -------------------------------------------------------------------------------*/

#ifndef CLUSTER_H
#define CLUSTER_H

#include "net_common.h"
#include "server_common.h"
#include "../../src/clustering_impl/clustering_common.h"

namespace nodecpp
{
	class Cluster; // forward declaration
	class Worker
	{
		static constexpr size_t invalidID = (size_t)(-1);
		friend class Cluster;
		size_t id_ = invalidID;
		Worker() {}
		void setID( size_t id ) { 
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, id_ == invalidID, "indeed: id_ = {}", id_ ); 
			id_ = id; 
		}

		nodecpp::safememory::owning_ptr<nodecpp::net::SocketBase> ctrlServer; // TODO: this might be a temporary solution

	public:
		Worker(const Worker&) = delete;
		Worker& operator = (const Worker&) = delete;
		Worker(Worker&& other) {
			id_ = other.id_;
			other.id_ = invalidID;
			ctrlServer = std::move( other.ctrlServer );
		}
		size_t id() const { return id_; }
		void disconnect();
	};


	extern void preinitMasterThreadClusterObject();
	extern void preinitSlaveThreadClusterObject( size_t id);
	extern void postinitThreadClusterObject();


	class Cluster
	{
	public:
		static constexpr size_t InvalidThreadID = (size_t)(-1);

	private:
		static void serializeListeningRequest( size_t threadID, size_t requestID, size_t entryIndex, Ip4 ip, Port port, int backlog, std::string family, nodecpp::Buffer& b ) {
			ClusteringMsgHeader h;
			h.type = ClusteringMsgHeader::ClusteringMsgType::ServerListening;
			h.assignedThreadID = threadID;
			h.requestID = requestID;
			h.bodySize = sizeof(size_t) + 4 + 2 + sizeof(int) + family.size();
			h.serialize( b );

			uint32_t uip = ip.getNetwork();
			uint16_t uport = port.getNetwork();
			b.append( &entryIndex, sizeof(size_t) );
			b.append( &uip, 4 );
			b.append( &uport, 2 );
			b.append( &backlog, sizeof(int) );
			b.appendString( family.c_str(), family.size() );
		}
		static size_t deserializeListeningRequestBody( size_t& entryIndex, nodecpp::net::Address& addr, int& backlog, nodecpp::Buffer& b, size_t offset, size_t sz ) {
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, sz + offset <= b.size() );
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, sz > sizeof(size_t) + 4 + 2 + sizeof(int) );
			entryIndex = *reinterpret_cast<size_t*>(b.begin() + offset);
			addr.ip.fromNetwork( *reinterpret_cast<uint32_t*>(b.begin() + offset + sizeof(size_t)) );
			addr.port = *reinterpret_cast<uint16_t*>(b.begin() + offset + sizeof(size_t) + 4);
			backlog = *reinterpret_cast<int*>(b.begin() + offset + sizeof(size_t) + 6);
			addr.family = std::string( reinterpret_cast<char*>(b.begin() + offset + sizeof(size_t) + 6 + sizeof(int)), sz - sizeof(size_t) - 6 - sizeof(int) );
			return offset + sz;
		}


	public:
		class AgentServer;
		class MasterSocket : public nodecpp::net::SocketBase, public ::nodecpp::DataParent<Cluster>
		{
			friend class AgentServer;
		public:
			nodecpp::safememory::soft_this_ptr<MasterSocket> myThis;

		private:
			nodecpp::Buffer incompleteRqBuff;
			size_t assignedThreadID = Cluster::InvalidThreadID;

			nodecpp::handler_ret_type sendListeningEv( size_t requestID )
			{
				ClusteringMsgHeader rhReply;
				rhReply.type = ClusteringMsgHeader::ClusteringMsgType::ServerListening;
				rhReply.assignedThreadID = assignedThreadID;
				rhReply.requestID = requestID;
				rhReply.bodySize = 0;
				nodecpp::Buffer reply;
				rhReply.serialize( reply );
				co_await a_write( reply );
				CO_RETURN;
			}

			nodecpp::handler_ret_type sendConnAcceptedEv( size_t internalID, size_t requestID, uint64_t socket )
			{
				ClusteringMsgHeader rhReply;
				rhReply.type = ClusteringMsgHeader::ClusteringMsgType::ConnAccepted;
				rhReply.assignedThreadID = assignedThreadID;
				rhReply.requestID = requestID;
				rhReply.bodySize = sizeof(internalID) + sizeof(socket);
				nodecpp::Buffer reply;
				rhReply.serialize( reply );
				size_t internalID_ = internalID;
				uint64_t socket_ = socket;
				reply.append( &internalID_, sizeof(internalID) );
				reply.append( &socket_, sizeof(socket) );
				co_await a_write( reply );
				CO_RETURN;
			}

			nodecpp::handler_ret_type processRequest( ClusteringMsgHeader& mh, nodecpp::Buffer& b, size_t offset )
			{
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "MasterSocket to thread {} processRequest(): request type {}", mh.assignedThreadID, (size_t)(mh.type) );
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, mh.bodySize + offset <= b.size(), "{} + {} vs. {}", mh.bodySize, offset, b.size() ); 
				switch ( mh.type )
				{
					case ClusteringMsgHeader::ClusteringMsgType::ThreadStarted:
					{
						nodecpp::net::Address addr;
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, assignedThreadID == Cluster::InvalidThreadID, "indeed: {}", assignedThreadID ); 
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, mh.assignedThreadID != Cluster::InvalidThreadID ); 
						assignedThreadID = mh.assignedThreadID;
						break;
					}
					case ClusteringMsgHeader::ClusteringMsgType::ServerListening:
					{
						nodecpp::net::Address addr;
						int backlog;
						size_t entryIndex;
						deserializeListeningRequestBody( entryIndex, addr, backlog, b, offset, mh.bodySize );
						nodecpp::safememory::soft_ptr<MasterSocket> me = myThis.getSoftPtr<MasterSocket>(this);
						bool already = getDataParent()->processRequestForListeningAtMaster( me, mh, entryIndex, addr, backlog );
						if ( already )
							sendListeningEv( mh.requestID );
						break;
					}
					default:
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type {}", (size_t)(mh.type) ); 
						break;
				}
				CO_RETURN;
			}

		public:
			nodecpp::handler_ret_type onAccepted()
			{
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "MasterSocket onAccepted()" );
				CO_RETURN;
			}
			nodecpp::handler_ret_type onData( nodecpp::Buffer& b)
			{
				if ( assignedThreadID != Cluster::InvalidThreadID )
					nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "MasterSocket to thread {} onData({})", assignedThreadID, b.size() );
				else
					nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "MasterSocket to thread ??? onData({})", b.size() );
				ClusteringMsgHeader currentMH;
				// Performance NotE: optimization is possible
				size_t pos = 0;
				if ( incompleteRqBuff.size() )
				{
					incompleteRqBuff.append( b );
					while ( ClusteringMsgHeader::couldBeDeserialized(incompleteRqBuff, pos) )
					{
						size_t tmppos = currentMH.deserialize( incompleteRqBuff, pos );
						if ( tmppos + currentMH.bodySize <= incompleteRqBuff.size() )
						{
							co_await processRequest( currentMH, incompleteRqBuff, pos );
							pos = tmppos + currentMH.bodySize;
						}
						else
							break;
					}
					if ( incompleteRqBuff.size() > pos )
						incompleteRqBuff.popFront( pos );
				}
				else
				{
					while ( ClusteringMsgHeader::couldBeDeserialized( b, pos ) )
					{
						size_t tmppos = currentMH.deserialize( b, pos );
						if ( tmppos + currentMH.bodySize <= b.size() )
						{
							co_await processRequest( currentMH, b, tmppos );
							pos = tmppos + currentMH.bodySize;
						}
						else
							break;
					}
					if ( b.size() > pos )
						incompleteRqBuff.append( b, pos );
				}
				CO_RETURN;
			}
		};

		class SlaveSocket : public nodecpp::net::SocketBase
		{
			friend class Cluster;

		private:
			nodecpp::Buffer incompleteRespBuff;
			size_t assignedThreadID;
			size_t requestIdBase = 0;
			Buffer requestsBeforeConnection;

			nodecpp::handler_ret_type sendListeningRequest( size_t entryIndex, Ip4 ip, Port port, std::string family, int backlog)
			{
				Buffer b;
				Cluster::serializeListeningRequest( assignedThreadID, ++requestIdBase, entryIndex, ip, port, backlog, family, b );
				if ( connecting() )
					requestsBeforeConnection.append( b );
				else
					co_await a_write( b );
			}

			nodecpp::handler_ret_type processResponse( ClusteringMsgHeader& mh, nodecpp::Buffer& b, size_t offset );

		public:
			nodecpp::handler_ret_type onConnect()
			{
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "onConnect() to Master at thread {}", assignedThreadID );
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, assignedThreadID != Cluster::InvalidThreadID ); 
				ClusteringMsgHeader rhReply;
				rhReply.type = ClusteringMsgHeader::ClusteringMsgType::ThreadStarted;
				rhReply.assignedThreadID = assignedThreadID;
				rhReply.requestID = 0;
				rhReply.bodySize = 0;
				nodecpp::Buffer reply;
				rhReply.serialize( reply );
				co_await a_write( reply );
				if ( requestsBeforeConnection.size() )
				{
					co_await a_write( requestsBeforeConnection );
					requestsBeforeConnection.clear();
				}
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "onConnect() to Master at thread {}: ini data sent", assignedThreadID );
				CO_RETURN;
			}
			nodecpp::handler_ret_type onData( nodecpp::Buffer& b)
			{
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "MasterSocket to thread {} onData({})", assignedThreadID, b.size() );
				ClusteringMsgHeader currentMH;
				// Performance NotE: optimization is possible
				size_t pos = 0;
				if ( incompleteRespBuff.size() )
				{
					incompleteRespBuff.append( b );
					while ( ClusteringMsgHeader::couldBeDeserialized(incompleteRespBuff, pos) )
					{
						size_t tmppos = currentMH.deserialize( incompleteRespBuff, pos );
						if ( tmppos + currentMH.bodySize <= incompleteRespBuff.size() )
						{
							co_await processResponse( currentMH, incompleteRespBuff, pos );
							pos = tmppos + currentMH.bodySize;
						}
						else
							break;
					}
					if ( incompleteRespBuff.size() > pos )
						incompleteRespBuff.popFront( pos );
				}
				else
				{
					while ( ClusteringMsgHeader::couldBeDeserialized( b ) )
					{
						size_t tmppos = currentMH.deserialize( b, pos );
						if ( tmppos + currentMH.bodySize <= b.size() )
						{
							co_await processResponse( currentMH, incompleteRespBuff, tmppos );
							incompleteRespBuff.append( b, pos + currentMH.bodySize );
							pos = tmppos + currentMH.bodySize;
						}
						else
							break;
					}
					if ( b.size() > pos )
						incompleteRespBuff.append( b, pos );
				}
				CO_RETURN;
			}
		};

		class MasterServer : public nodecpp::net::ServerSocket<Cluster>
		{
		public:
			nodecpp::handler_ret_type onListening(size_t id, nodecpp::net::Address addr) { 
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("clustering ctrl server: onListening()!");
				CO_RETURN;
			}

			nodecpp::handler_ret_type onConnection(nodecpp::safememory::soft_ptr<nodecpp::net::SocketBase> socket) { 
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("clustering ctrl server: onConnection()!");
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, socket != nullptr ); 
				soft_ptr<MasterSocket> socketPtr = nodecpp::safememory::soft_ptr_static_cast<MasterSocket>(socket);

				CO_RETURN;
			}
		};

		class AgentServer
		{
			friend class Cluster;
			Cluster& myCluster;
			struct SlaveServerData
			{
				size_t entryIndex;
				nodecpp::safememory::soft_ptr<MasterSocket> socket;
			};
			std::vector<SlaveServerData> socketsToSlaves;
		public:
			nodecpp::safememory::soft_this_ptr<AgentServer> myThis;
		public:
			class DataForCommandProcessing {
			public:
				size_t index;
				bool refed = false;
				short fdEvents = 0;
				//SOCKET osSocket = INVALID_SOCKET;
				unsigned long long osSocket = 0;

				enum State { Unused, Listening, BeingClosed, Closed }; // TODO: revise!
				State state = State::Unused;

				DataForCommandProcessing() {}
				DataForCommandProcessing(const DataForCommandProcessing& other) = delete;
				DataForCommandProcessing& operator=(const DataForCommandProcessing& other) = delete;

				DataForCommandProcessing(DataForCommandProcessing&& other) = default;
				DataForCommandProcessing& operator=(DataForCommandProcessing&& other) = default;

				net::Address localAddress;
			};
			DataForCommandProcessing dataForCommandProcessing;
			size_t requestID = -1;

		public:
			nodecpp::handler_ret_type onListening() { 
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("clustering Agent server: onListening()!");

				CO_RETURN;
			}
			nodecpp::handler_ret_type onConnection(uint64_t socket) { 
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("clustering Agent server: onConnection()!");
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, socket != 0 ); 
				// TODO: selection between Slaves
				socketsToSlaves[0].socket->sendConnAcceptedEv( socketsToSlaves[0].entryIndex, requestID, socket );
				CO_RETURN;
			}
			nodecpp::handler_ret_type onError() { 
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("clustering Agent server: onListening()!");
				// TODO: forward to all related slaves

				CO_RETURN;
			}
			nodecpp::handler_ret_type onEnd(bool hasError) { 
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("clustering Agent server: onListening()!");
				// TODO: forward to all related slaves

				CO_RETURN;
			}

		public:
			void registerServer();

		public:
			AgentServer(Cluster& myCluster_) : myCluster( myCluster_ ) {
				nodecpp::safememory::soft_ptr<AgentServer> p = myThis.getSoftPtr<AgentServer>(this);
			}
			virtual ~AgentServer() {
				reportBeingDestructed();
			}

			void internalCleanupBeforeClosing()
			{
				//NODECPP_ASSERT( nodecpp::module_id, nodecpp::assert::AssertLevel::critical, getSockCount() == 0 ); 
				dataForCommandProcessing.state = DataForCommandProcessing::State::Closed;
				dataForCommandProcessing.index = 0;
			}

			const net::Address& address() const { return dataForCommandProcessing.localAddress; }

			void listen(uint16_t port, const char* ip, int backlog);
			void close();
			void ref();
			void unref();
			bool listening() const { return dataForCommandProcessing.state == DataForCommandProcessing::State::Listening; }
			void reportBeingDestructed();


		};
		std::vector<nodecpp::safememory::owning_ptr<AgentServer>> agentServers;

		nodecpp::safememory::soft_ptr<AgentServer> createAgentServer() {
			nodecpp::safememory::owning_ptr<AgentServer> newServer = nodecpp::safememory::make_owning<AgentServer>(*this);
			nodecpp::safememory::soft_ptr<AgentServer> ret = newServer;
			newServer->registerServer();
			for ( size_t i=0; i<agentServers.size(); ++i )
				if ( agentServers[i] == nullptr )
				{
					agentServers[i] = std::move( newServer );
					return ret;
				}
			agentServers.push_back( std::move( newServer ) );
			return ret;
		}

		bool processRequestForListeningAtMaster(nodecpp::safememory::soft_ptr<MasterSocket> requestingSocket, ClusteringMsgHeader& mh, size_t entryIndex, nodecpp::net::Address address, int backlog)
		{
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, isMaster() );
			bool alreadyListening = false;
			AgentServer::SlaveServerData slaveData;
			slaveData.entryIndex = entryIndex;
			slaveData.socket = requestingSocket;
			for ( auto& server : agentServers )
				if ( server != nullptr && 
					server->dataForCommandProcessing.localAddress == address &&
					server->requestID == mh.requestID )
				{
					server->socketsToSlaves.push_back( slaveData );
					return server->listening();
				}
			nodecpp::safememory::soft_ptr<AgentServer> server = createAgentServer();
			server->requestID = mh.requestID;
			server->socketsToSlaves.push_back( slaveData );
			server->listen( address.port, address.ip.toStr().c_str(), backlog );
			return false;
		}

		void acceptRequestForListeningAtSlave(size_t entryIndex, Ip4 ip, Port port, std::string family, int backlog)
		{
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, isWorker() );
			Buffer b;
			slaveSocket->sendListeningRequest( entryIndex, ip, port, family, backlog );
		}

	private:
		Worker thisThreadWorker;
		std::vector<Worker> workers_;
		uint64_t coreCtr = 0; // for: thread ID generation at master; requestID generation at Slave

		using CtrlServerT = MasterServer;
		nodecpp::safememory::owning_ptr<CtrlServerT> ctrlServer; // TODO: this might be a temporary solution
		nodecpp::safememory::owning_ptr<SlaveSocket> slaveSocket; // TODO: this might be a temporary solution

		friend void preinitMasterThreadClusterObject();
		friend void preinitSlaveThreadClusterObject(size_t);
		friend void postinitThreadClusterObject();

		void preinitMaster() { 
			thisThreadWorker.id_ = 0; 
		}
		void preinitSlave(size_t threadID) { 
			thisThreadWorker.id_ = threadID; 
		}
		void postinit() { 
			if ( isMaster() )
			{
				nodecpp::net::ServerBase::addHandler<MasterServer, nodecpp::net::ServerBase::DataForCommandProcessing::UserHandlers::Handler::Connection, &MasterServer::onConnection>();
				nodecpp::net::ServerBase::addHandler<MasterServer, nodecpp::net::ServerBase::DataForCommandProcessing::UserHandlers::Handler::Listen, &MasterServer::onListening>();
				nodecpp::net::SocketBase::addHandler<MasterSocket, nodecpp::net::SocketBase::DataForCommandProcessing::UserHandlers::Handler::Accepted, &MasterSocket::onAccepted>();
				nodecpp::net::SocketBase::addHandler<MasterSocket, nodecpp::net::SocketBase::DataForCommandProcessing::UserHandlers::Handler::Data, &MasterSocket::onData>();
				ctrlServer = nodecpp::net::createServer<CtrlServerT, MasterSocket>();
				ctrlServer->listen(21000, "127.0.0.1", 500);
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("Master thread Cluster is ready");
			}
			else
			{
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("Slave thread # {} Cluster is about to setup connection to Master", thisThreadWorker.id());
				nodecpp::net::SocketBase::addHandler<SlaveSocket, nodecpp::net::SocketBase::DataForCommandProcessing::UserHandlers::Handler::Connect, &SlaveSocket::onConnect>();
				nodecpp::net::SocketBase::addHandler<SlaveSocket, nodecpp::net::SocketBase::DataForCommandProcessing::UserHandlers::Handler::Data, &SlaveSocket::onData>();

				slaveSocket = nodecpp::net::createSocket<SlaveSocket>();
				slaveSocket->assignedThreadID = thisThreadWorker.id_;
				slaveSocket->connect(21000, "127.0.0.1");
			}
		}
	public:
		Cluster() {}
		Cluster(const Cluster&) = delete;
		Cluster& operator = (const Cluster&) = delete;
		Cluster(Cluster&&) = delete;
		Cluster& operator = (Cluster&&) = delete;

		bool isMaster() const { return thisThreadWorker.id() == 0; }
		bool isWorker() const { return thisThreadWorker.id() != 0; }
		const std::vector<Worker>& workers() const { return workers_; }
		const Worker& worker() const { return thisThreadWorker; }

		Worker& fork();
		void disconnect() { for ( auto& w : workers_ ) w.disconnect(); }

		// event handling (awaitable)
	};
	extern thread_local Cluster cluster;

	inline
	Cluster& getCluster() { return cluster; }
}
#endif //CLUSTER_H



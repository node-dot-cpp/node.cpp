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

#ifdef NODECPP_ENABLE_CLUSTERING

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
		uint16_t portToMaster = 0;
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
		static void serializeListeningRequest( size_t threadID, size_t requestID, size_t entryIndex, Ip4 ip, uint16_t port, int backlog, IPFAMILY family, nodecpp::Buffer& b ) {
			nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "Slave id = {}: serializing listening request for Addr = {}:{}, backlog = {}, entryIndex = {:x}", threadID, ip.toStr(), port, backlog, entryIndex );
			ClusteringMsgHeader h;
			h.type = ClusteringMsgHeader::ClusteringMsgType::ServerListening;
			h.assignedThreadID = threadID;
			h.requestID = requestID;
			h.entryIdx = entryIndex;
			h.bodySize = 4 + 2 + sizeof(int) + 4;
			h.serialize( b );
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::pedantic, sizeof( ClusteringMsgHeader ) == b.size() );

			uint32_t uip = ip.getNetwork();
			b.append( &uip, 4 );
			b.append( &port, 2 );
			b.append( &backlog, sizeof(int) );
			uint32_t familyNum = (uint32_t)(family.value());
			b.append( &familyNum, 4 );
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::pedantic, sizeof( ClusteringMsgHeader ) + h.bodySize == b.size() );
		}
		static size_t deserializeListeningRequestBody( nodecpp::net::Address& addr, int& backlog, nodecpp::Buffer& b, size_t offset, size_t sz ) {
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, sz + offset <= b.size() );
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, sz > 4 + 2 + sizeof(int) );
			addr.ip = Ip4::fromNetwork( *reinterpret_cast<uint32_t*>(b.begin() + offset) );
			addr.port = *reinterpret_cast<uint16_t*>(b.begin() + offset + 4);
			backlog = *reinterpret_cast<int*>(b.begin() + offset + 6);
			uint32_t numFamily = *reinterpret_cast<uint32_t*>(b.begin() + offset + 6 + sizeof(int));
			addr.family.fromNum( numFamily );
			return offset + sz;
		}

		static void serializeServerCloseRequest( size_t threadID, size_t requestID, size_t entryIndex, nodecpp::Buffer& b ) {
			nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "Slave id = {}: serializing ServerCloseRequest request for entryIndex = {:x}", threadID, entryIndex );
			ClusteringMsgHeader h;
			h.type = ClusteringMsgHeader::ClusteringMsgType::ServerListening;
			h.assignedThreadID = threadID;
			h.requestID = requestID;
			h.entryIdx = entryIndex;
			h.bodySize = 0;
			h.serialize( b );
		}


	public:
		class AgentServer;
		class MasterSocket : public nodecpp::net::Socket<Cluster>
		{
			friend class AgentServer;
			friend class Cluster;
		public:
			nodecpp::safememory::soft_this_ptr<MasterSocket> myThis;

		public:
			MasterSocket() {}
			MasterSocket(Cluster* cluster) : nodecpp::net::Socket<Cluster>(cluster) {}
			virtual ~MasterSocket() {}

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

			nodecpp::handler_ret_type sendConnAcceptedEv( size_t internalID, size_t requestID, uint64_t socket, Ip4& remoteIp, Port& remotePort )
			{
				ClusteringMsgHeader rhReply;
				rhReply.type = ClusteringMsgHeader::ClusteringMsgType::ConnAccepted;
				rhReply.assignedThreadID = assignedThreadID;
				rhReply.requestID = requestID;
				rhReply.bodySize = sizeof(internalID) + sizeof(socket) + 4 + 2;
				nodecpp::Buffer reply;
				rhReply.serialize( reply );
				size_t internalID_ = internalID;
				uint64_t socket_ = socket;
				uint32_t uip = remoteIp.getNetwork();
				uint16_t uport = remotePort.getNetwork();
				reply.append( &internalID_, sizeof(internalID) );
				reply.append( &socket_, sizeof(socket) );
				reply.append( &uip, sizeof(uip) );
				reply.append( &uport, sizeof(uport) );
				co_await a_write( reply );
				CO_RETURN;
			}

			nodecpp::handler_ret_type sendServerErrorEv( size_t internalID, size_t requestID, Error e )
			{
				ClusteringMsgHeader rhReply;
				rhReply.type = ClusteringMsgHeader::ClusteringMsgType::ServerError;
				rhReply.assignedThreadID = assignedThreadID;
				rhReply.requestID = requestID;
				rhReply.bodySize = sizeof(internalID); // TODO: be ready to pass more data
				nodecpp::Buffer reply;
				rhReply.serialize( reply );
				size_t internalID_ = internalID;
				reply.append( &internalID_, sizeof(internalID) );
				co_await a_write( reply );
				CO_RETURN;
			}

			nodecpp::handler_ret_type sendServerCloseNotification( size_t entryIdx, size_t requestID, bool hasError )
			{
				ClusteringMsgHeader rhReply;
				rhReply.type = ClusteringMsgHeader::ClusteringMsgType::ServerClosedNotification;
				rhReply.assignedThreadID = assignedThreadID;
				rhReply.requestID = requestID;
				rhReply.entryIdx = entryIdx;
				rhReply.bodySize = 1;
				nodecpp::Buffer reply;
				rhReply.serialize( reply );
				reply.appendUint8( hasError ? 1 : 0 );
				co_await a_write( reply );
				CO_RETURN;
			}

			nodecpp::handler_ret_type processRequest( ClusteringMsgHeader& mh, nodecpp::Buffer& b, size_t offset )
			{
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, mh.bodySize + offset <= b.size(), "{} + {} vs. {}", mh.bodySize, offset, b.size() ); 
				switch ( mh.type )
				{
					case ClusteringMsgHeader::ClusteringMsgType::ThreadStarted:
					{
						nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "MasterSocket: processing ThreadStarted({}) request (for thread id: {})", (size_t)(mh.type), mh.assignedThreadID );
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, assignedThreadID == Cluster::InvalidThreadID, "indeed: {}", assignedThreadID ); 
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, mh.assignedThreadID != Cluster::InvalidThreadID ); 
						assignedThreadID = mh.assignedThreadID;
						break;
					}
					case ClusteringMsgHeader::ClusteringMsgType::ServerListening:
					{
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, mh.assignedThreadID == assignedThreadID ); 
						nodecpp::net::Address addr;
						int backlog;
						deserializeListeningRequestBody( addr, backlog, b, offset, mh.bodySize );
						nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "MasterSocket: processing ServerListening({}) request (for thread id: {}). Addr = {}:{}, backlog = {}, entryIndex = {:x}", (size_t)(mh.type), mh.assignedThreadID, addr.ip.toStr(), addr.port, backlog, mh.entryIdx );
						nodecpp::safememory::soft_ptr<MasterSocket> me = myThis.getSoftPtr<MasterSocket>(this);
						bool already = getDataParent()->processRequestForListeningAtMaster( me, mh, addr, backlog );
						if ( already )
							sendListeningEv( mh.requestID );
						break;
					}
					case ClusteringMsgHeader::ClusteringMsgType::ServerCloseRequest:
					{
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, mh.assignedThreadID == assignedThreadID ); 
						nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "MasterSocket: processing ServerCloseRequest({}) request (for thread id: {}), entryIndex = {:x}", (size_t)(mh.type), mh.assignedThreadID, mh.entryIdx );
						nodecpp::safememory::soft_ptr<MasterSocket> me = myThis.getSoftPtr<MasterSocket>(this);
						getDataParent()->processRequestForServerCloseAtMaster( me, mh );
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
//				nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "MasterSocket onAccepted()" );
				CO_RETURN;
			}
			nodecpp::handler_ret_type onData( nodecpp::Buffer& b)
			{
				/*if ( assignedThreadID != Cluster::InvalidThreadID )
					nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "MasterSocket to thread {} onData({})", assignedThreadID, b.size() );
				else
					nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "MasterSocket to thread ??? onData({})", b.size() );*/
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

			nodecpp::handler_ret_type sendListeningRequest( size_t entryIndex, Ip4 ip, uint16_t port, IPFAMILY family, int backlog)
			{
				Buffer b;
				Cluster::serializeListeningRequest( assignedThreadID, ++requestIdBase, entryIndex, ip, port, backlog, family, b );
				if ( connecting() )
					requestsBeforeConnection.append( b );
				else
					co_await a_write( b );
			}

			nodecpp::handler_ret_type sendServerCloseRequest( size_t entryIndex )
			{
				Buffer b;
				Cluster::serializeServerCloseRequest( assignedThreadID, ++requestIdBase, entryIndex, b );
				if ( connecting() )
					requestsBeforeConnection.append( b );
				else
					co_await a_write( b );
			}

			nodecpp::handler_ret_type processResponse( ClusteringMsgHeader& mh, nodecpp::Buffer& b, size_t offset );

		public:
			nodecpp::handler_ret_type onConnect()
			{
//				nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "onConnect() to Master at thread {}", assignedThreadID );
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, assignedThreadID != Cluster::InvalidThreadID ); 
				ClusteringMsgHeader rhReply;
				rhReply.type = ClusteringMsgHeader::ClusteringMsgType::ThreadStarted;
				rhReply.assignedThreadID = assignedThreadID;
				rhReply.requestID = 0;
				rhReply.bodySize = 0;
				nodecpp::Buffer reply;
				rhReply.serialize( reply );
				co_await a_write( reply );
//				nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "onConnect() to Master at thread {}: ini data sent, size: {}", assignedThreadID, reply.size() );
				if ( requestsBeforeConnection.size() )
				{
					co_await a_write( requestsBeforeConnection );
//					nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "onConnect() to Master at thread {}: additional data sent, size: {}", assignedThreadID, requestsBeforeConnection.size() );
					requestsBeforeConnection.clear();
				}
				CO_RETURN;
			}
			nodecpp::handler_ret_type onData( nodecpp::Buffer& b)
			{
//				nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "MasterSocket to thread {} onData({})", assignedThreadID, b.size() );
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
					while ( ClusteringMsgHeader::couldBeDeserialized( b, pos ) )
					{
						size_t tmppos = currentMH.deserialize( b, pos );
						if ( tmppos + currentMH.bodySize <= b.size() )
						{
							co_await processResponse( currentMH, b, tmppos );
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
			MasterServer() {}
			MasterServer(Cluster* cluster) : nodecpp::net::ServerSocket<Cluster>(cluster) {}
			virtual ~MasterServer() {}
			nodecpp::handler_ret_type onListening(size_t id, nodecpp::net::Address addr) { 
				nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id),"clustering ctrl server: onListening()!");
				CO_RETURN;
			}

			nodecpp::handler_ret_type onConnection(nodecpp::safememory::soft_ptr<nodecpp::net::SocketBase> socket) { 
//				nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id),"clustering ctrl server: onConnection()!");
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, socket != nullptr ); 
				soft_ptr<MasterSocket> socketPtr = nodecpp::safememory::soft_ptr_static_cast<MasterSocket>(socket);

				CO_RETURN;
			}
		};

		class AgentServer
		{
			friend class Cluster;
//			Cluster& myCluster;
			struct SlaveServerData
			{
				size_t entryIndex;
				nodecpp::safememory::soft_ptr<MasterSocket> socket;
			};
			nodecpp::vector<SlaveServerData> socketsToSlaves;
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
			size_t nextStep = 0;
			size_t requestID = -1;

		public:
			nodecpp::handler_ret_type onListening() { 
				nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id),"clustering Agent server: onListening()!");

				CO_RETURN;
			}
			nodecpp::handler_ret_type onConnection(uint64_t socket, Ip4& remoteIp, Port& remotePort) { 
//				nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id),"clustering Agent server: onConnection()!");
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, socket != 0 ); 
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, socketsToSlaves.size() != 0 ); 
				// TODO: consider alternative ways selection between Slaves
				nextStep = nextStep % socketsToSlaves.size();
				socketsToSlaves[nextStep].socket->sendConnAcceptedEv( socketsToSlaves[nextStep].entryIndex, requestID, socket, remoteIp, remotePort );
				++nextStep;
				CO_RETURN;
			}
			nodecpp::handler_ret_type onError( nodecpp::Error& e ) { 
				nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id),"clustering Agent server: onError()!");
				for ( auto& slaveData : socketsToSlaves )
					slaveData.socket->sendServerErrorEv( socketsToSlaves[nextStep].entryIndex, requestID, e );

				CO_RETURN;
			}
			nodecpp::handler_ret_type onEnd(bool hasError) { 
				nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id),"clustering Agent server: onEnd({})!", hasError);

				CO_RETURN;
			}

		public:
			void registerServer();

		public:
			AgentServer(Cluster& myCluster_) /*: myCluster( myCluster_ )*/ {
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

			void listen(uint16_t port, nodecpp::Ip4 ip, int backlog);
			void close();
			void ref();
			void unref();
			bool listening() const { return dataForCommandProcessing.state == DataForCommandProcessing::State::Listening; }
			void reportBeingDestructed();


		};
		nodecpp::vector<nodecpp::safememory::owning_ptr<AgentServer>> agentServers;

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

		bool processRequestForListeningAtMaster(nodecpp::safememory::soft_ptr<MasterSocket> requestingSocket, ClusteringMsgHeader& mh, nodecpp::net::Address address, int backlog)
		{
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, isMaster() );
			bool alreadyListening = false;
			size_t entryIndex = mh.entryIdx;
			AgentServer::SlaveServerData slaveData;
			slaveData.entryIndex = entryIndex;
			slaveData.socket = requestingSocket;
			for ( auto& server : agentServers )
			{
				if ( server != nullptr )
					nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "processRequestForListeningAtMaster(): comparing to Addr {}:{}, and RequestID {}", server->dataForCommandProcessing.localAddress.ip.toStr(), server->dataForCommandProcessing.localAddress.port, server->requestID );
				if ( server != nullptr && 
					server->dataForCommandProcessing.localAddress == address &&
					server->requestID == mh.requestID )
				{
					server->socketsToSlaves.push_back( slaveData );
					return server->listening();
				}
			}
			nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "processRequestForListeningAtMaster() for thread id: {} Addr {}:{}, requestID {}, and entryIndex = {:x} requires actually creating a new Agent server (curr num of Agent servers: {})", mh.assignedThreadID, address.ip.toStr(), address.port, mh.requestID, entryIndex, agentServers.size() );
			nodecpp::safememory::soft_ptr<AgentServer> server = createAgentServer();
			server->requestID = mh.requestID;
			server->socketsToSlaves.push_back( slaveData );
			server->listen( address.port, address.ip, backlog );
			nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "processRequestForListeningAtMaster(): new Agent Server for Addr {}:{}, and RequestID {}", server->dataForCommandProcessing.localAddress.ip.toStr(), server->dataForCommandProcessing.localAddress.port, server->requestID );
			return false;
		}

		void processRequestForServerCloseAtMaster(nodecpp::safememory::soft_ptr<MasterSocket> requestingSocket, ClusteringMsgHeader& mh)
		{
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, isMaster() );
			bool alreadyListening = false;
			AgentServer::SlaveServerData slaveData;
			slaveData.entryIndex = mh.entryIdx;
			slaveData.socket = requestingSocket;
			nodecpp::safememory::soft_ptr<AgentServer> agent;
			for ( auto& server : agentServers )
			{
				if ( server != nullptr && 
					server->requestID == mh.requestID )
				{
					for ( auto& slave : server->socketsToSlaves )
						if ( slave.entryIndex == mh.entryIdx )
						{
							agent = server;
							break;
						}
				}
				if ( agent != nullptr )
					break;
			}
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, agent != nullptr );
			agent->close();
			for ( auto& slave : agent->socketsToSlaves )
				slave.socket->sendServerCloseNotification( slave.entryIndex, agent->requestID, false );
		}

		void acceptRequestForListeningAtSlave(size_t entryIndex, Ip4 ip, uint16_t port, IPFAMILY family, int backlog)
		{
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, isWorker() );
			Buffer b;
			slaveSocket->sendListeningRequest( entryIndex, ip, port, family, backlog );
		}

		void acceptRequestForServerCloseAtSlave(size_t entryIndex)
		{
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, isWorker() );
			Buffer b;
			slaveSocket->sendServerCloseRequest( entryIndex );
		}

	private:
		Worker thisThreadWorker;
		nodecpp::vector<Worker> workers_;
		uint64_t coreCtr = 0; // for: thread ID generation at master; requestID generation at Slave

		using CtrlServerT = MasterServer;
		nodecpp::safememory::owning_ptr<CtrlServerT> ctrlServer; // TODO: this might be a temporary solution
		nodecpp::safememory::owning_ptr<SlaveSocket> slaveSocket; // TODO: this might be a temporary solution

		friend void preinitMasterThreadClusterObject();
		friend void preinitSlaveThreadClusterObject(ThreadStartupData& startupData);
		friend void postinitThreadClusterObject();

		void preinitMaster() { 
			thisThreadWorker.id_ = 0; 
		}
		void preinitSlave(ThreadStartupData& startupData) { 
			thisThreadWorker.id_ = startupData.assignedThreadID;
			thisThreadWorker.portToMaster = startupData.commPort;
		}
		void postinit() { 
			if ( isMaster() )
			{
				nodecpp::net::ServerBase::addHandler<MasterServer, nodecpp::net::ServerBase::DataForCommandProcessing::UserHandlers::Handler::Connection, &MasterServer::onConnection>();
				nodecpp::net::ServerBase::addHandler<MasterServer, nodecpp::net::ServerBase::DataForCommandProcessing::UserHandlers::Handler::Listen, &MasterServer::onListening>();
				nodecpp::net::SocketBase::addHandler<MasterSocket, nodecpp::net::SocketBase::DataForCommandProcessing::UserHandlers::Handler::Accepted, &MasterSocket::onAccepted>();
				nodecpp::net::SocketBase::addHandler<MasterSocket, nodecpp::net::SocketBase::DataForCommandProcessing::UserHandlers::Handler::Data, &MasterSocket::onData>();
				ctrlServer = nodecpp::net::createServer<CtrlServerT, MasterSocket>(this);
				ctrlServer->listen(0, "127.0.0.1", 64);
				nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id),"Master thread Cluster is ready and listens clients on port {}", ctrlServer->dataForCommandProcessing.localAddress.port );
			}
			else
			{
				nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id),"Slave thread # {} Cluster is about to setup connection to Master", thisThreadWorker.id());
				nodecpp::net::SocketBase::addHandler<SlaveSocket, nodecpp::net::SocketBase::DataForCommandProcessing::UserHandlers::Handler::Connect, &SlaveSocket::onConnect>();
				nodecpp::net::SocketBase::addHandler<SlaveSocket, nodecpp::net::SocketBase::DataForCommandProcessing::UserHandlers::Handler::Data, &SlaveSocket::onData>();

				slaveSocket = nodecpp::net::createSocket<SlaveSocket>();
				slaveSocket->assignedThreadID = thisThreadWorker.id_;
				slaveSocket->connect(thisThreadWorker.portToMaster, "127.0.0.1");
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
		const nodecpp::vector<Worker>& workers() const { return workers_; }
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

#endif // NODECPP_ENABLE_CLUSTERING

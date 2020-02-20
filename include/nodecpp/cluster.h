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
#include "../../src/clustering_impl/interthread_comm.h"

// ad-hoc marchalling between Master and Slave threads
struct ClusteringMsgHeader
{
	enum ClusteringMsgType { ThreadStarted, ThreadTerminate, ServerListening, ConnAccepted, ServerError, ServerCloseRequest, ServerClosedNotification };
	size_t bodySize;
	ClusteringMsgType type;
	size_t requestID;
	size_t entryIdx;
	void serialize( nodecpp::Buffer& b ) { b.append( this, sizeof( ClusteringMsgHeader ) ); }
	size_t deserialize( const nodecpp::Buffer& b, size_t pos ) { 
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, pos + sizeof( ClusteringMsgHeader ) <= b.size(), "indeed: {} + {} vs. {}", pos, sizeof( ClusteringMsgHeader ), b.size() ); 
		memcpy( this, b.begin() + pos, sizeof( ClusteringMsgHeader ) );
		return pos + sizeof( ClusteringMsgHeader );
	}
	void deserialize( const uint8_t* buff, size_t size ) { 
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, sizeof( ClusteringMsgHeader ) <= size, "indeed: {} vs. {}", sizeof( ClusteringMsgHeader ), size ); 
		memcpy( this, buff, sizeof( ClusteringMsgHeader ) );
	}
	static bool couldBeDeserialized( const nodecpp::Buffer& b, size_t pos = 0 ) { return b.size() >= pos + sizeof( ClusteringMsgHeader ); }
	static size_t serializationSize() { return sizeof( ClusteringMsgHeader ); }
};

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
	extern void postinitThreadClusterObject();


	class Cluster
	{
	public:
		static constexpr size_t InvalidThreadID = (size_t)(-1);

	private:
		class MasterProcessor
		{
		public:
			MasterProcessor() {}
			virtual ~MasterProcessor() {}

//		private:
			static void sendListeningEv( ThreadID targetThreadId, size_t requestID )
			{
				ClusteringMsgHeader rhReply;
				rhReply.type = ClusteringMsgHeader::ClusteringMsgType::ServerListening;
				rhReply.requestID = requestID;
				rhReply.bodySize = 0;
				nodecpp::Buffer reply;
				rhReply.serialize( reply );

				nodecpp::platform::internal_msg::InternalMsg msg;
				msg.append( reply.begin(), reply.size() );
				sendInterThreadMsg( std::move( msg ), ClusteringMsgHeader::ClusteringMsgType::ServerListening, targetThreadId );
			}

			static void sendConnAcceptedEv( ThreadID targetThreadId, size_t internalID, size_t requestID, uint64_t socket, Ip4& remoteIp, Port& remotePort )
			{
				ClusteringMsgHeader rhReply;
				rhReply.type = ClusteringMsgHeader::ClusteringMsgType::ConnAccepted;
				rhReply.requestID = requestID;
				rhReply.bodySize = sizeof(internalID) + sizeof(socket) + 4 + 2;
				nodecpp::Buffer reply;
				rhReply.serialize( reply );
				size_t internalID_ = internalID;
				uint64_t socket_ = socket;
				uint32_t uip = remoteIp.getNetwork();
				uint16_t uport = remotePort.getNetwork();
				reply.append( &internalID_, sizeof(internalID_) );
				reply.append( &socket_, sizeof(socket) );
				reply.append( &uip, sizeof(uip) );
				reply.append( &uport, sizeof(uport) );

				nodecpp::platform::internal_msg::InternalMsg msg;
				msg.append( reply.begin(), reply.size() );
				sendInterThreadMsg( std::move( msg ), ClusteringMsgHeader::ClusteringMsgType::ConnAccepted, targetThreadId );
			}

			static void sendServerErrorEv( ThreadID targetThreadId, size_t requestID, Error e )
			{
				ClusteringMsgHeader rhReply;
				rhReply.type = ClusteringMsgHeader::ClusteringMsgType::ServerError;
				rhReply.requestID = requestID;
				rhReply.bodySize = 0;
				nodecpp::Buffer reply;
				rhReply.serialize( reply );

				nodecpp::platform::internal_msg::InternalMsg msg;
				msg.append( reply.begin(), reply.size() );
				sendInterThreadMsg( std::move( msg ), ClusteringMsgHeader::ClusteringMsgType::ServerError, targetThreadId );
			}

			static void deserializeListeningRequestBody( nodecpp::net::Address& addr, int& backlog, nodecpp::platform::internal_msg::InternalMsg::ReadIter& riter, size_t bodySz ) {
				//nodecpp::Buffer& b, size_t offset, size_t sz
				size_t sz = riter.availableSize();
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, sz >= bodySz );
				const uint8_t* buff = riter.read( bodySz );
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, sz > 4 + 2 + sizeof(int) );
				addr.ip = Ip4::fromNetwork( *reinterpret_cast<const uint32_t*>(buff) );
				addr.port = *reinterpret_cast<const uint16_t*>(buff + 4);
				backlog = *reinterpret_cast<const int*>(buff + 6);
				uint32_t numFamily = *reinterpret_cast<const uint32_t*>(buff + 6 + sizeof(int));
				addr.family.fromNum( numFamily );
			}

		public:
			static void sendServerCloseNotification( ThreadID targetThreadId, size_t entryIdx, size_t requestID, bool hasError )
			{
				ClusteringMsgHeader rhReply;
				rhReply.type = ClusteringMsgHeader::ClusteringMsgType::ServerClosedNotification;
				rhReply.requestID = requestID;
				rhReply.entryIdx = entryIdx;
				rhReply.bodySize = 1;
				nodecpp::Buffer reply;
				rhReply.serialize( reply );
				reply.appendUint8( hasError ? 1 : 0 );

				nodecpp::platform::internal_msg::InternalMsg msg;
				msg.append( reply.begin(), reply.size() );
				sendInterThreadMsg( std::move( msg ), ClusteringMsgHeader::ClusteringMsgType::ServerClosedNotification, targetThreadId );
			}

		public:
		};


		nodecpp::handler_ret_type processInterthreadRequest( ThreadID requestingThreadId, ClusteringMsgHeader& mh, nodecpp::platform::internal_msg::InternalMsg::ReadIter& riter )
		{
	//		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, mh.bodySize + offset <= b.size(), "{} + {} vs. {}", mh.bodySize, offset, b.size() ); 
			switch ( mh.type )
			{
				case ClusteringMsgHeader::ClusteringMsgType::ThreadStarted:
				{
					nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "MasterSocket: processing ThreadStarted({}) request (for thread id: {})", (size_t)(mh.type), requestingThreadId.slotId );
//						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, assignedThreadID == Cluster::InvalidThreadID, "indeed: {}", assignedThreadID ); 
//						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, mh.assignedThreadID != Cluster::InvalidThreadID ); 
	//				assignedThreadID = mh.assignedThreadID;
					break;
				}
				case ClusteringMsgHeader::ClusteringMsgType::ServerListening:
				{
//						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, mh.assignedThreadID == assignedThreadID ); 
					nodecpp::net::Address addr;
					int backlog;
					MasterProcessor::deserializeListeningRequestBody( addr, backlog, riter, mh.bodySize );
					nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "MasterSocket: processing ServerListening({}) request (for thread id: {}). Addr = {}:{}, backlog = {}, entryIndex = {:x}", (size_t)(mh.type), requestingThreadId.slotId, addr.ip.toStr(), addr.port, backlog, mh.entryIdx );
//					nodecpp::safememory::soft_ptr<MasterSocket> me = myThis.getSoftPtr<MasterSocket>(this);
					bool already = processRequestForListeningAtMaster( requestingThreadId, mh, addr, backlog );
					if ( already )
						MasterProcessor::sendListeningEv( requestingThreadId, mh.requestID );
					break;
				}
				case ClusteringMsgHeader::ClusteringMsgType::ServerCloseRequest:
				{
//					NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, mh.assignedThreadID == assignedThreadID ); 
					nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "MasterSocket: processing ServerCloseRequest({}) request (for thread id: {}), entryIndex = {:x}", (size_t)(mh.type), requestingThreadId.slotId, mh.entryIdx );
//					nodecpp::safememory::soft_ptr<MasterSocket> me = myThis.getSoftPtr<MasterSocket>(this);
					processRequestForServerCloseAtMaster( requestingThreadId, mh );
					break;
				}
				default:
					NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type {}", (size_t)(mh.type) ); 
					break;
			}
			CO_RETURN;
		}

		public:
		nodecpp::handler_ret_type onInterthreadMessage( InterThreadMsg& msg )
		{
			// NOTE: in present quick-and-dirty implementation we assume that the message total size is less than a single page
			nodecpp::platform::internal_msg::InternalMsg::ReadIter riter = msg.msg.getReadIter();
			size_t sz = riter.availableSize();
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, ClusteringMsgHeader::serializationSize() <= sz, "indeed: {} vs. {}", ClusteringMsgHeader::serializationSize(), sz ); 
			const uint8_t* page = riter.read( ClusteringMsgHeader::serializationSize() );
			ClusteringMsgHeader mh;
			mh.deserialize( page, ClusteringMsgHeader::serializationSize() );
			processInterthreadRequest( msg.sourceThreadID, mh, riter );
			CO_RETURN;
		}




		static void serializeAndSendListeningRequest( ThreadID targetThreadId, size_t requestID, size_t entryIndex, Ip4 ip, uint16_t port, int backlog, IPFAMILY family ) {
			nodecpp::Buffer b;
//			nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "Slave id = {}: serializing listening request for Addr = {}:{}, backlog = {}, entryIndex = {:x}", threadID, ip.toStr(), port, backlog, entryIndex );
			nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "Slave id = [...]: serializing listening request for Addr = {}:{}, backlog = {}, entryIndex = {:x}", ip.toStr(), port, backlog, entryIndex );
			ClusteringMsgHeader h;
			h.type = ClusteringMsgHeader::ClusteringMsgType::ServerListening;
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

			nodecpp::platform::internal_msg::InternalMsg msg;
			msg.append( b.begin(), b.size() );
			sendInterThreadMsg( std::move( msg ), ClusteringMsgHeader::ClusteringMsgType::ServerListening, targetThreadId );
		}

		static void serializeAndSendServerCloseRequest( ThreadID targetThreadId, size_t requestID, size_t entryIndex ) {
//			nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "Slave id = {}: serializing ServerCloseRequest request for entryIndex = {:x}", threadID, entryIndex );
			nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "Slave id = [...]: serializing ServerCloseRequest request for entryIndex = {:x}", entryIndex );
			nodecpp::Buffer b;
			ClusteringMsgHeader h;
			h.type = ClusteringMsgHeader::ClusteringMsgType::ServerListening;
			h.requestID = requestID;
			h.entryIdx = entryIndex;
			h.bodySize = 0;
			h.serialize( b );

			nodecpp::platform::internal_msg::InternalMsg msg;
			msg.append( b.begin(), b.size() );
			sendInterThreadMsg( std::move( msg ), ClusteringMsgHeader::ClusteringMsgType::ServerListening, targetThreadId );
		}


	public:
		class AgentServer;

		class SlaveProcessor
		{
			friend class Cluster;

		private:
			size_t assignedThreadID;
			size_t requestIdBase = 0;

			void sendListeningRequest( size_t entryIndex, Ip4 ip, uint16_t port, IPFAMILY family, int backlog)
			{
				Cluster::serializeAndSendListeningRequest( ThreadID({0,0}), ++requestIdBase, entryIndex, ip, port, backlog, family );
			}

			void sendServerCloseRequest( size_t entryIndex )
			{
				Cluster::serializeAndSendServerCloseRequest( ThreadID({0,0}), ++requestIdBase, entryIndex );
			}

			nodecpp::handler_ret_type processResponse( ThreadID requestingThreadId, ClusteringMsgHeader& mh, nodecpp::platform::internal_msg::InternalMsg::ReadIter& riter );

		public:
			nodecpp::handler_ret_type reportThreadStarted()
			{
//				nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "reportThreadStarted() to Master at thread {}", assignedThreadID );
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, assignedThreadID != Cluster::InvalidThreadID ); 
				ClusteringMsgHeader rhReply;
				rhReply.type = ClusteringMsgHeader::ClusteringMsgType::ThreadStarted;
				rhReply.requestID = 0;
				rhReply.bodySize = 0;
				nodecpp::Buffer reply;
				rhReply.serialize( reply );

				nodecpp::platform::internal_msg::InternalMsg msg;
				msg.append( reply.begin(), reply.size() );
				sendInterThreadMsg( std::move( msg ), ClusteringMsgHeader::ClusteringMsgType::ThreadStarted, ThreadID({0, 0}) );
				CO_RETURN;
			}

			public:
			nodecpp::handler_ret_type onInterthreadMessage( InterThreadMsg& msg )
			{
				// NOTE: in present quick-and-dirty implementation we assume that the message total size is less than a single page
				auto riter = msg.msg.getReadIter();
				size_t sz = riter.availableSize();
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, ClusteringMsgHeader::serializationSize() <= sz, "indeed: {} vs. {}", ClusteringMsgHeader::serializationSize(), sz ); 
				const uint8_t* page = riter.read( ClusteringMsgHeader::serializationSize() );
				ClusteringMsgHeader mh;
				mh.deserialize( page, ClusteringMsgHeader::serializationSize() );
				processResponse( msg.sourceThreadID, mh, riter );
				CO_RETURN;
			}
		};
		SlaveProcessor slaveProcessor;

		class AgentServer
		{
			friend class Cluster;
//			Cluster& myCluster;
			struct SlaveServerData
			{
				size_t entryIndex;
				ThreadID targetThreadId;
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
				MasterProcessor::sendConnAcceptedEv( socketsToSlaves[nextStep].targetThreadId, socketsToSlaves[nextStep].entryIndex, requestID, socket, remoteIp, remotePort ); // TODO-ITC: upgrade
				++nextStep;
				CO_RETURN;
			}
			nodecpp::handler_ret_type onError( nodecpp::Error& e ) { 
				nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id),"clustering Agent server: onError()!");
				for ( auto& slaveData : socketsToSlaves )
					MasterProcessor::sendServerErrorEv( socketsToSlaves[nextStep].targetThreadId, requestID, e ); // TODO-ITC: upgrade

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

		bool processRequestForListeningAtMaster(ThreadID targetThreadId, ClusteringMsgHeader& mh, nodecpp::net::Address address, int backlog)
		{
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, isMaster() );
			bool alreadyListening = false;
			size_t entryIndex = mh.entryIdx;
			AgentServer::SlaveServerData slaveData;
			slaveData.entryIndex = entryIndex;
			slaveData.targetThreadId = targetThreadId;
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
			nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "processRequestForListeningAtMaster() for thread id: {} Addr {}:{}, requestID {}, and entryIndex = {:x} requires actually creating a new Agent server (curr num of Agent servers: {})", targetThreadId.slotId, address.ip.toStr(), address.port, mh.requestID, entryIndex, agentServers.size() );
			nodecpp::safememory::soft_ptr<AgentServer> server = createAgentServer();
			server->requestID = mh.requestID;
			server->socketsToSlaves.push_back( slaveData );
			server->listen( address.port, address.ip, backlog );
			nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "processRequestForListeningAtMaster(): new Agent Server for Addr {}:{}, and RequestID {}", server->dataForCommandProcessing.localAddress.ip.toStr(), server->dataForCommandProcessing.localAddress.port, server->requestID );
			return false;
		}

		void processRequestForServerCloseAtMaster(ThreadID targetThreadId, ClusteringMsgHeader& mh)
		{
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, isMaster() );
			bool alreadyListening = false;
			AgentServer::SlaveServerData slaveData;
			slaveData.entryIndex = mh.entryIdx;
			slaveData.targetThreadId = targetThreadId;
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
//				slave.socket->sendServerCloseNotification( slave.entryIndex, agent->requestID, false ); // TODO-ITC: upgrade
				MasterProcessor::sendServerCloseNotification( slave.targetThreadId, slave.entryIndex, agent->requestID, false ); // TODO-ITC: upgrade
		}

		void acceptRequestForListeningAtSlave(size_t entryIndex, Ip4 ip, uint16_t port, IPFAMILY family, int backlog)
		{
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, isWorker() );
			Buffer b;
			slaveProcessor.sendListeningRequest( entryIndex, ip, port, family, backlog );
		}

		void acceptRequestForServerCloseAtSlave(size_t entryIndex)
		{
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, isWorker() );
			Buffer b;
			slaveProcessor.sendServerCloseRequest( entryIndex );
		}

	private:
		Worker thisThreadWorker;
		nodecpp::vector<Worker> workers_;
		uint64_t coreCtr = 0; // for: thread ID generation at master; requestID generation at Slave

		friend void preinitMasterThreadClusterObject();
		friend void preinitSlaveThreadClusterObject(ThreadStartupData& startupData);
		friend void postinitThreadClusterObject();

		void preinitMaster() { 
			thisThreadWorker.id_ = 0; 
		}
		void preinitSlave( size_t assignedThreadId ) { 
			thisThreadWorker.id_ = assignedThreadId;
		}
		void postinit() { 
			if ( isMaster() )
			{
			}
			else
			{
				slaveProcessor.reportThreadStarted();
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

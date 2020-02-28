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

namespace nodecpp
{
	struct ListeningEvMsg
	{
		size_t requestID;
	};

	struct ConnAcceptedEvMsg
	{
		size_t requestID;
		size_t serverIdx;
		uintptr_t socket;
		Ip4 ip;
		Port uport;
	};

	struct ServerErrorEvMsg
	{
		size_t requestID;
		Error e;
	};

	struct ServerCloseNotificationMsg
	{
		size_t requestID;
		size_t entryIdx;
		bool hasError;
	};

	struct ListeningRequestMsg
	{
		size_t requestID;
		size_t entryIndex;
		Ip4 ip;
		uint16_t port;
		int backlog;
		IPFAMILY family;
	};

	struct ServerCloseRequest
	{
		size_t requestID;
		size_t entryIdx;
	};

	struct ThreadStartedReportMsg
	{
		size_t requestID;
	};

	struct RequestToListenerThread
	{
		enum Type { Undefined, AddServerSocket, CreateServerSocket, RemoveServerSocket, CloseServerSocket };
		Type type = Type::Undefined;
		uintptr_t socket;
		Ip4 ip;
		Port port;
		int backlog;
	};

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

			static void sendListeningEv( ThreadID targetThreadId, size_t requestID )
			{
				ListeningEvMsg msg;
				msg.requestID = requestID;

				nodecpp::platform::internal_msg::InternalMsg imsg;
				imsg.append( &msg, sizeof(msg) );
				sendInterThreadMsg( std::move( imsg ), InterThreadMsgType::ServerListening, targetThreadId );
			}

			static void sendConnAcceptedEv( ThreadID targetThreadId, size_t internalID, size_t requestID, uintptr_t socket, Ip4& remoteIp, Port& remotePort )
			{
				ConnAcceptedEvMsg msg;
				msg.requestID = requestID;
				msg.serverIdx = internalID;
				msg.socket = socket;
				msg.ip = remoteIp;
				msg.uport = remotePort;

				nodecpp::platform::internal_msg::InternalMsg imsg;
				imsg.append( &msg, sizeof(msg) );
				sendInterThreadMsg( std::move( imsg ), InterThreadMsgType::ConnAccepted, targetThreadId );
			}

			static void sendServerErrorEv( ThreadID targetThreadId, size_t requestID, Error e )
			{
				ServerErrorEvMsg msg;
				msg.requestID = requestID;
				msg.e = e;

				nodecpp::platform::internal_msg::InternalMsg imsg;
				imsg.append( &msg, sizeof(msg) );
				sendInterThreadMsg( std::move( imsg ), InterThreadMsgType::ServerError, targetThreadId );
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
				ServerCloseNotificationMsg msg;
				msg.requestID = requestID;
				msg.entryIdx = entryIdx;
				msg.hasError = hasError;

				nodecpp::platform::internal_msg::InternalMsg imsg;
				imsg.append( &msg, sizeof(msg) );
				sendInterThreadMsg( std::move( imsg ), InterThreadMsgType::ServerClosedNotification, targetThreadId );
			}

		public:
		};


		nodecpp::handler_ret_type processInterthreadRequest( ThreadID requestingThreadId, InterThreadMsgType msgtype, nodecpp::platform::internal_msg::InternalMsg::ReadIter& riter )
		{
			size_t sz = riter.availableSize();
			switch ( msgtype )
			{
				case InterThreadMsgType::ThreadStarted:
				{
					nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "MasterSocket: processing ThreadStarted({}) request (for thread id: {})", (size_t)(msgtype), requestingThreadId.slotId );
//						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, assignedThreadID == Cluster::InvalidThreadID, "indeed: {}", assignedThreadID ); 
//						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, mh.assignedThreadID != Cluster::InvalidThreadID ); 
					break;
				}
				case InterThreadMsgType::ServerListening:
				{
					NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, sizeof( ListeningRequestMsg ) <= sz, "{} vs. {}", sizeof( ListeningRequestMsg ), sz ); 
					const ListeningRequestMsg* msg = reinterpret_cast<const ListeningRequestMsg*>( riter.read( sizeof( ListeningRequestMsg ) ) );

					nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "MasterSocket: processing ServerListening({}) request (for thread id: {}). Addr = {}:{}, backlog = {}, entryIndex = {:x}", (size_t)(msgtype), requestingThreadId.slotId, msg->ip.toStr(), msg->port, msg->backlog, msg->entryIndex );
					nodecpp::net::Address addr;
					addr.ip = msg->ip;
					addr.port = msg->port;
					addr.family = msg->family;
					bool already = processRequestForListeningAtMaster( requestingThreadId, msg->entryIndex, msg->requestID, addr, msg->backlog );
					if ( already )
						MasterProcessor::sendListeningEv( requestingThreadId, msg->requestID );
					break;
				}
				case InterThreadMsgType::ServerCloseRequest:
				{
					NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, sizeof( ServerCloseRequest ) <= sz, "{} vs. {}", sizeof( ServerCloseRequest ), sz ); 
					const ServerCloseRequest* msg = reinterpret_cast<const ServerCloseRequest*>( riter.read( sizeof( ServerCloseRequest ) ) );
					nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "MasterSocket: processing ServerCloseRequest({}) request (for thread id: {}), entryIndex = {:x}", (size_t)(msgtype), requestingThreadId.slotId, msg->entryIdx );
					processRequestForServerCloseAtMaster( requestingThreadId, *msg );
					break;
				}
				default:
					NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type {}", (size_t)(msgtype) ); 
					break;
			}
			CO_RETURN;
		}

		public:
		nodecpp::handler_ret_type onInterthreadMessage( InterThreadMsg& msg )
		{
			// NOTE: in present quick-and-dirty implementation we assume that the message total size is less than a single page
			nodecpp::platform::internal_msg::InternalMsg::ReadIter riter = msg.msg.getReadIter();
			processInterthreadRequest( msg.sourceThreadID, msg.msgType, riter );
			CO_RETURN;
		}

		static void serializeAndSendListeningRequest( ThreadID targetThreadId, size_t requestID, size_t entryIndex, Ip4 ip, uint16_t port, int backlog, IPFAMILY family ) {
//			nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "Slave id = {}: serializing listening request for Addr = {}:{}, backlog = {}, entryIndex = {:x}", threadID, ip.toStr(), port, backlog, entryIndex );
			nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "Slave id = [...]: serializing listening request for Addr = {}:{}, backlog = {}, entryIndex = {:x}", ip.toStr(), port, backlog, entryIndex );

			ListeningRequestMsg msg;
			msg.requestID = requestID;
			msg.entryIndex = entryIndex;
			msg.ip = ip;
			msg.port = port;
			msg.backlog = backlog;
			msg.family = family;

			nodecpp::platform::internal_msg::InternalMsg imsg;
			imsg.append( &msg, sizeof(msg) );
			sendInterThreadMsg( std::move( imsg ), InterThreadMsgType::ServerListening, targetThreadId );
		}

		static void serializeAndSendServerCloseRequest( ThreadID targetThreadId, size_t requestID, size_t entryIndex ) {
//			nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "Slave id = {}: serializing ServerCloseRequest request for entryIndex = {:x}", threadID, entryIndex );
			nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "Slave id = [...]: serializing ServerCloseRequest request for entryIndex = {:x}", entryIndex );
			ServerCloseRequest msg;
			msg.requestID = requestID;
			msg.entryIdx = entryIndex;

			nodecpp::platform::internal_msg::InternalMsg imsg;
			imsg.append( &msg, sizeof(msg) );
			sendInterThreadMsg( std::move( imsg ), InterThreadMsgType::ServerListening, targetThreadId );
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

			nodecpp::handler_ret_type processResponse( ThreadID requestingThreadId, InterThreadMsgType msgType, nodecpp::platform::internal_msg::InternalMsg::ReadIter& riter );

		public:
			nodecpp::handler_ret_type reportThreadStarted()
			{
//				nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "reportThreadStarted() to Master at thread {}", assignedThreadID );
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, assignedThreadID != Cluster::InvalidThreadID ); 
				ThreadStartedReportMsg msg;
				msg.requestID = 0;

				nodecpp::platform::internal_msg::InternalMsg imsg;
				imsg.append( &msg, sizeof(msg) );
				sendInterThreadMsg( std::move( imsg ), InterThreadMsgType::ThreadStarted, ThreadID({0, 0}) );
				CO_RETURN;
			}

			public:
			nodecpp::handler_ret_type onInterthreadMessage( InterThreadMsg& msg )
			{
				// NOTE: in present quick-and-dirty implementation we assume that the message total size is less than a single page
				auto riter = msg.msg.getReadIter();
				processResponse( msg.sourceThreadID, msg.msgType, riter );
				CO_RETURN;
			}
		};
		SlaveProcessor slaveProcessor;

		class AgentServer
		{
			friend class Cluster;
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

		bool processRequestForListeningAtMaster(ThreadID targetThreadId, size_t entryIndex, size_t requestID, nodecpp::net::Address address, int backlog)
		{
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, isMaster() );
			bool alreadyListening = false;
			AgentServer::SlaveServerData slaveData;
			slaveData.entryIndex = entryIndex;
			slaveData.targetThreadId = targetThreadId;
			for ( auto& server : agentServers )
			{
				if ( server != nullptr )
					nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "processRequestForListeningAtMaster(): comparing to Addr {}:{}, and RequestID {}", server->dataForCommandProcessing.localAddress.ip.toStr(), server->dataForCommandProcessing.localAddress.port, server->requestID );
				if ( server != nullptr && 
					server->dataForCommandProcessing.localAddress == address &&
					server->requestID == requestID )
				{
					server->socketsToSlaves.push_back( slaveData );
					return server->listening();
				}
			}
			nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "processRequestForListeningAtMaster() for thread id: {} Addr {}:{}, requestID {}, and entryIndex = {:x} requires actually creating a new Agent server (curr num of Agent servers: {})", targetThreadId.slotId, address.ip.toStr(), address.port, requestID, entryIndex, agentServers.size() );
			nodecpp::safememory::soft_ptr<AgentServer> server = createAgentServer();
			server->requestID = requestID;
			server->socketsToSlaves.push_back( slaveData );
			server->listen( address.port, address.ip, backlog );
			nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "processRequestForListeningAtMaster(): new Agent Server for Addr {}:{}, and RequestID {}", server->dataForCommandProcessing.localAddress.ip.toStr(), server->dataForCommandProcessing.localAddress.port, server->requestID );

			// now, unles an error happened, we have a socket alredy good for ploing
			RequestToListenerThread rq;
			rq.type = RequestToListenerThread::Type::AddServerSocket;
			rq.ip = address.ip;
			rq.port.fromHost( address.port );
			rq.backlog = backlog;
			rq.socket = server->dataForCommandProcessing.osSocket;

			auto listeners = getListeners();
			for ( size_t i=0; i<listeners.second; ++i )
			{
				nodecpp::platform::internal_msg::InternalMsg imsg;
				imsg.append( &rq, sizeof(rq) );
				sendInterThreadMsg( std::move( imsg ), InterThreadMsgType::ConnAccepted, listeners.first[ i ].threadID );
			}
			return false;
		}

		void processRequestForServerCloseAtMaster(ThreadID targetThreadId, const ServerCloseRequest& mh)
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

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
		class MasterSocket : public nodecpp::net::SocketBase, public ::nodecpp::DataParent<Cluster>
		{
		public:
			nodecpp::handler_ret_type onAccepted()
			{
				CO_RETURN;
			}
			nodecpp::handler_ret_type onData( nodecpp::Buffer& b)
			{
				CO_RETURN;
			}
		};

		class SlaveSocket : public nodecpp::net::SocketBase
		{
		public:
			nodecpp::handler_ret_type onConnect()
			{
				CO_RETURN;
			}
			nodecpp::handler_ret_type onData( nodecpp::Buffer& b)
			{
				CO_RETURN;
			}
		};

		class MasterServer : public nodecpp::net::ServerSocket<Cluster>
		{
			nodecpp::handler_ret_type onConnection(nodecpp::safememory::soft_ptr<MasterSocket> socket) { 
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("clustering ctrl server: onConnection()!");
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, socket != nullptr ); 

				CO_RETURN;
			}

		};

		class AgentServer
		{
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

		public:
			nodecpp::handler_ret_type onListening() { 
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("clustering Agent server: onListening()!");

				CO_RETURN;
			}
			nodecpp::handler_ret_type onConnection(uint64_t socket) { 
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("clustering Agent server: onConnection()!");
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, socket != 0 ); 
				// TODO: forward to one of related slaves
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
			AgentServer() {
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



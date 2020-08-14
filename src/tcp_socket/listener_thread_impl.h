/* -------------------------------------------------------------------------------
* Copyright (c) 2018, OLogN Technologies AG
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


#ifndef TCP_SOCKET_FOR_LISTENER_THREAD_H
#define TCP_SOCKET_FOR_LISTENER_THREAD_H

#ifdef NODECPP_ENABLE_CLUSTERING

#include "tcp_socket_base.h"
#include "../clustering_impl/clustering_impl.h"
#include "../clustering_impl/interthread_comm.h"
#include "../../include/nodecpp/timers.h"


using namespace nodecpp;


class ListenerThreadWorker
{
private:
	static void sendConnAcceptedEv( ThreadID targetThreadId, size_t internalID, uint64_t socket, Ip4& remoteIp, Port& remotePort )
	{
		ConnAcceptedEvMsg msg;
		msg.requestID = 0; // TODO:
		msg.serverIdx = internalID;
		msg.socket = socket;
		msg.ip = remoteIp;
		msg.uport = remotePort;

		nodecpp::platform::internal_msg::InternalMsg imsg;
		imsg.append( &msg, sizeof(msg) );
		sendInterThreadMsg( std::move( imsg ), InterThreadMsgType::ConnAccepted, targetThreadId );
	}

	static void sendServerErrorEv( ThreadID targetThreadId, Error e )
	{
		ServerErrorEvMsg msg;
		msg.requestID = 0; // TODO:
		msg.e = e;

		nodecpp::platform::internal_msg::InternalMsg imsg;
		imsg.append( &msg, sizeof(msg) );
		sendInterThreadMsg( std::move( imsg ), InterThreadMsgType::ServerError, targetThreadId );
	}

	static void sendServerCloseNotification( ThreadID targetThreadId, size_t entryIdx, bool hasError )
	{
		ServerCloseNotificationMsg msg;
		msg.requestID = 0; // TODO:
		msg.entryIdx = entryIdx;
		msg.hasError = hasError;

		nodecpp::platform::internal_msg::InternalMsg imsg;
		imsg.append( &msg, sizeof(msg) );
		sendInterThreadMsg( std::move( imsg ), InterThreadMsgType::ServerClosedNotification, targetThreadId );
	}

	void reportThreadStarted()
	{
		ThreadStartedReportMsg msg;
		msg.requestID = 0;

		nodecpp::platform::internal_msg::InternalMsg imsg;
		imsg.append( &msg, sizeof(msg) );
		sendInterThreadMsg( std::move( imsg ), InterThreadMsgType::ThreadStarted, ThreadID({0, 0}) );
	}

	void processInterthreadRequest( ThreadID requestingThreadId, InterThreadMsgType msgType, nodecpp::platform::internal_msg::InternalMsg::ReadIter& riter );

public:
	void onInterthreadMessage( InterThreadMsg& msg )
	{
		// NOTE: in present quick-and-dirty implementation we assume that the message total size is less than a single page
		nodecpp::platform::internal_msg::InternalMsg::ReadIter riter = msg.msg.getReadIter();
		processInterthreadRequest( msg.sourceThreadID, msg.msgType, riter );
	}

	class AgentServer
	{
		friend class ListenerThreadWorker;
		size_t entryIndexAtSlave; // temporary solution TODO: further elaboration!!!
		/*struct SlaveServerData
		{
			size_t entryIndex;
			ThreadID targetThreadId;
		};
		nodecpp::vector<SlaveServerData> socketsToSlaves;
		size_t nextStep = 0;*/

	public:
		nodecpp::soft_this_ptr<AgentServer> myThis;
	public:
		class DataForCommandProcessing {
		public:
			size_t index;
			bool refed = false;
			short fdEvents = 0;
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
		void onListening() { 
			nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id),"clustering Agent server: onListening()!");
		}
		void onConnection(uint64_t socket, Ip4& remoteIp, Port& remotePort) { 
//				nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id),"clustering Agent server: onConnection()!");
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, socket != 0 ); 
			/*NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, socketsToSlaves.size() != 0 ); 
			// TODO: consider alternative ways selection between Slaves
			nextStep = nextStep % socketsToSlaves.size();
			sendConnAcceptedEv( socketsToSlaves[nextStep].targetThreadId, socketsToSlaves[nextStep].entryIndex, socket, remoteIp, remotePort ); // TODO-ITC: upgrade
			++nextStep;*/
			ThreadID id = getLeastLoadedWorkerAndIncrementLoad();
			//nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id),"listener thread: accepted connection is being sent to thread id {}", id.slotId);
			sendConnAcceptedEv( id, entryIndexAtSlave, socket, remoteIp, remotePort );
			// TODO: assert is good
		}
		void onError( nodecpp::Error& e ) { 
			nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id),"clustering Agent server: onError()!");
			/*for ( auto& slaveData : socketsToSlaves )
				sendServerErrorEv( socketsToSlaves[nextStep].targetThreadId, e ); // TODO-ITC: upgrade*/
		}
		void onEnd(bool hasError) { 
			nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id),"clustering Agent server: onEnd({})!", hasError);
		}

	public:
		void addServerSocketAndStartListening( SOCKET socket);
		void acquireSharedServerSocketAndStartListening(int backlog);

	public:
		AgentServer() {
			nodecpp::soft_ptr<AgentServer> p = myThis.getSoftPtr<AgentServer>(this);
		}
		virtual ~AgentServer() {
			//reportBeingDestructed();
		}

		void internalCleanupBeforeClosing()
		{
			//NODECPP_ASSERT( nodecpp::module_id, nodecpp::assert::AssertLevel::critical, getSockCount() == 0 ); 
			dataForCommandProcessing.state = DataForCommandProcessing::State::Closed;
			dataForCommandProcessing.index = 0;
		}

		const net::Address& address() const { return dataForCommandProcessing.localAddress; }

		bool listening() const { return dataForCommandProcessing.state == DataForCommandProcessing::State::Listening; }


	};

private:
	nodecpp::vector<nodecpp::owning_ptr<AgentServer>> agentServers;

	nodecpp::soft_ptr<AgentServer> createAgentServerWithSharedSocket(size_t entryIndex, nodecpp::Ip4 ip, uint16_t port, int backlog) {
		nodecpp::owning_ptr<AgentServer> newServer = nodecpp::make_owning<AgentServer>();
		nodecpp::soft_ptr<AgentServer> ret = newServer;
		newServer->entryIndexAtSlave = entryIndex;
		newServer->dataForCommandProcessing.localAddress.ip = ip;
		newServer->dataForCommandProcessing.localAddress.port = port;
		newServer->acquireSharedServerSocketAndStartListening(backlog);
		for ( size_t i=0; i<agentServers.size(); ++i )
			if ( agentServers[i] == nullptr )
			{
				agentServers[i] = std::move( newServer );
				return ret;
			}
		agentServers.push_back( std::move( newServer ) );
		return ret;
	}

	nodecpp::soft_ptr<AgentServer> createAgentServerWithExistingSocket( size_t entryIndex, SOCKET sock, nodecpp::Ip4 ip, uint16_t port ) {
		nodecpp::owning_ptr<AgentServer> newServer = nodecpp::make_owning<AgentServer>();
		nodecpp::soft_ptr<AgentServer> ret = newServer;
		newServer->entryIndexAtSlave = entryIndex;
		newServer->dataForCommandProcessing.localAddress.ip = ip;
		newServer->dataForCommandProcessing.localAddress.port = port;
		newServer->addServerSocketAndStartListening(sock);
		for ( size_t i=0; i<agentServers.size(); ++i )
			if ( agentServers[i] == nullptr )
			{
				agentServers[i] = std::move( newServer );
				return ret;
			}
		agentServers.push_back( std::move( newServer ) );
		return ret;
	}

public:
	ListenerThreadWorker() {}
	ListenerThreadWorker(const ListenerThreadWorker&) = delete;
	ListenerThreadWorker& operator = (const ListenerThreadWorker&) = delete;
	ListenerThreadWorker(ListenerThreadWorker&&) = delete;
	ListenerThreadWorker& operator = (ListenerThreadWorker&&) = delete;
	void preinit() { 
		//thisThreadWorker.id_ = 0; 
	}
	void postinit() { 
		reportThreadStarted();
	}
};
extern thread_local ListenerThreadWorker listenerThreadWorker;


static constexpr uint64_t TimeOutNever = std::numeric_limits<uint64_t>::max();

class NetSocketEntryForListenerThread {
	// TODO: revise everything around being 'refed'
	enum State { Unused, SockIssued, SockAssociated, SockClosed }; // TODO: revise!
	State state = State::Unused;
	nodecpp::soft_ptr<ListenerThreadWorker::AgentServer> ptr;

public:
	size_t index;
	bool refed = false;

	NetSocketEntryForListenerThread(size_t index) : state(State::Unused), index(index) {}
	NetSocketEntryForListenerThread(size_t index, nodecpp::soft_ptr<ListenerThreadWorker::AgentServer> ptr_) : state(State::SockIssued), index(index), ptr( ptr_ ) {ptr->dataForCommandProcessing.index = index;NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, ptr->dataForCommandProcessing.osSocket > 0 );}
	
	NetSocketEntryForListenerThread(const NetSocketEntryForListenerThread& other) = delete;
	NetSocketEntryForListenerThread& operator=(const NetSocketEntryForListenerThread& other) = delete;

	NetSocketEntryForListenerThread(NetSocketEntryForListenerThread&& other) = default;
	NetSocketEntryForListenerThread& operator=(NetSocketEntryForListenerThread&& other) = default;

	bool isUsed() const { NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, (state == State::Unused) || (state != State::Unused && ptr != nullptr) ); return state != State::Unused; }
	bool isAssociated() const { NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, (state == State::Unused) || (state != State::Unused && ptr != nullptr) ); return state == State::SockAssociated; }
	void setAssociated() {NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, state == State::SockIssued && ptr != nullptr ); state = State::SockAssociated;}
//	void setSocketClosed() {NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, state != State::Unused ); state = State::SockClosed;}
//	void setUnused() {state = State::Unused; index = 0;}

	nodecpp::soft_ptr<ListenerThreadWorker::AgentServer> getAgentServer() const { NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, ptr != nullptr); return ptr; }
	ListenerThreadWorker::AgentServer::DataForCommandProcessing* getAgentServerData() const { NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, ptr != nullptr); return ptr != nullptr ? &( ptr->dataForCommandProcessing ) : nullptr; }

	void updateIndex( size_t idx ) {
		index = idx;
		auto* pdata = getAgentServerData();
		if ( pdata )
			pdata->index = idx;
	}
};

class NetSocketsForListenerThread
{
private:
	using NetSocketEntryVectorT = nodecpp::vector<NetSocketEntryForListenerThread>;
	NetSocketEntryVectorT ourSide;
	nodecpp::vector<pollfd> osSide;
	size_t associatedCount = 0;
	size_t usedCount = 0;
public:
	//mb: xxxSide[0] is always reserved and invalid.
	//di: xxxSide[1] is always reserved (separate handling for awaker socket)
	static constexpr size_t awakerSockIdx = 1;
	static constexpr size_t reserved_capacity = 2;
private:
	static constexpr size_t capacity_ = 1 + reserved_capacity; // just a temporary workaround to prevent reallocation at arbitrary time; TODO: address properly!
	static constexpr size_t compactionMinSize = 32;

public:

	NetSocketsForListenerThread() {
		ourSide.reserve(capacity_); 
		osSide.reserve(capacity_);
		ourSide.emplace_back(0);
		osSide.emplace_back();

		// for awaker
		ourSide.emplace_back(0);
		osSide.emplace_back();
		osSide[awakerSockIdx].fd = INVALID_SOCKET;
	}

	bool isUsed(size_t idx) {
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, idx >= reserved_capacity ); 
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, idx < ourSide.size() );
		return ourSide[idx].isUsed(); 
	}
	NetSocketEntryForListenerThread& at(size_t idx) {
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, idx >= reserved_capacity ); 
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, idx < ourSide.size() );
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, ourSide[idx].isUsed() ); 
		return ourSide.at(idx);
	}
	const NetSocketEntryForListenerThread& at(size_t idx) const {
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, idx >= reserved_capacity ); 
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, idx < ourSide.size() );
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, ourSide[idx].isUsed() ); 
		return ourSide.at(idx);
	}
	short reventsAt(size_t idx) const {
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, idx != 0 ); 
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, idx < osSide.size() );
		return osSide.at(idx).revents;
	}
	SOCKET socketsAt(size_t idx) const { 
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, idx != 0 ); 
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, idx < osSide.size() );
		return osSide.at(idx).fd;
	}

	SOCKET getAwakerSockSocket() { return osSide[awakerSockIdx].fd; }

	size_t size() const {return ourSide.size() - 1; }
	bool isValidId( size_t idx ) { return idx >= reserved_capacity && idx < ourSide.size(); }

	void addEntry(nodecpp::soft_ptr<ListenerThreadWorker::AgentServer> ptr) {
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, ourSide.size() == osSide.size() );
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, ptr->dataForCommandProcessing.osSocket > 0 );
		for (size_t i = reserved_capacity; i != ourSide.size(); ++i) // skip ourSide[0]
		{
			if (!ourSide[i].isUsed())
			{
				NetSocketEntryForListenerThread entry(i, ptr);
				ourSide[i] = std::move(entry);
				osSide[i].fd = (SOCKET)(-((int64_t)(ptr->dataForCommandProcessing.osSocket)));
				osSide[i].events = 0;
				osSide[i].revents = 0;
				++usedCount;
				return;
			}
		}

		size_t ix = ourSide.size();
		ourSide.emplace_back(ix, ptr);
		pollfd p;
		p.fd = (SOCKET)(-((int64_t)(ptr->dataForCommandProcessing.osSocket)));
		p.events = 0;
		p.revents = 0;
		osSide.push_back( p );
		++usedCount;

		return;
	}

	void setAwakerSocket( SOCKET sock )
	{
		osSide[awakerSockIdx].fd = sock;
		osSide[awakerSockIdx].events |= POLLIN;
		osSide[awakerSockIdx].revents = 0;
		++usedCount;
		++associatedCount;
		return;
	}

	void makeCompactIfNecessary() {
		if ( ourSide.size() <= compactionMinSize || usedCount < ourSide.size() / 2 )
			return;
		NetSocketEntryVectorT ourSideNew;
		nodecpp::vector<pollfd> osSideNew;
		ourSideNew.reserve(capacity_); 
		osSideNew.reserve(capacity_);
		ourSideNew.emplace_back(0); 
		osSideNew.emplace_back();
		size_t usedCountNew = 0;
		// copy reserverd part
		ourSideNew.emplace_back(std::move(ourSide[awakerSockIdx]));
		osSideNew.push_back( osSide[awakerSockIdx] );
		if ( osSide[awakerSockIdx].fd != INVALID_SOCKET )
			++usedCountNew;

		for (size_t i = reserved_capacity; i != ourSide.size(); ++i) // skip reserved part
		{
			if (ourSide[i].isUsed())
			{
				size_t idx = ourSideNew.size();
				ourSide[i].updateIndex( idx );
				ourSideNew.emplace_back(std::move(ourSide[i]));
				osSideNew.push_back( osSide[i] );
				++usedCountNew;
			}
		}
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, usedCountNew == usedCount, "{} vs. {}", usedCountNew, usedCount );
		ourSide.swap( ourSideNew );
		osSide.swap( osSideNew );
	}

	void setAssociated( size_t idx/*, pollfd p*/ ) {
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, idx >= reserved_capacity ); 
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, idx < ourSide.size() );
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, ourSide[idx].isUsed() ); 
		ourSide[idx].setAssociated();
		osSide[idx].fd = (SOCKET)(-((int64_t)(osSide[idx].fd)));
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, osSide[idx].events == 0, "indeed: {}", osSide[idx].events );
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, osSide[idx].revents == 0, "indeed: {}", osSide[idx].revents );
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, osSide[idx].fd > 0 );
		++associatedCount;
	}
	void setPollin( size_t idx ) {
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, idx >= reserved_capacity ); 
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, idx < ourSide.size() );
		osSide[idx].events |= POLLIN; 
	}
	/*void unsetPollin( size_t idx ) {
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, idx >= reserved_capacity ); 
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, idx < ourSide.size() );
		osSide[idx].events &= ~POLLIN; 
	}*/
	void setRefed( size_t idx, bool refed ) {
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, idx >= reserved_capacity ); 
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, idx < ourSide.size() );
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, !refed || ourSide[idx].isUsed() ); 
		ourSide[idx].refed = refed;
	}
	std::pair<pollfd*, size_t> getPollfd() { 
		return osSide.size() > 1 ? ( associatedCount > 0 ? std::make_pair( &(osSide[1]), osSide.size() - 1 ) : std::make_pair( nullptr, 0 ) ) : std::make_pair( nullptr, 0 ); 
	}
	std::pair<bool, int> wait( int timeoutToUse ) {
		if ( associatedCount == 0 ) // if (refed == false && refedSocket == false) return false; //stop here'
			return std::make_pair(false, 0);
#ifdef _MSC_VER
		int retval = WSAPoll(&(osSide[1]), static_cast<ULONG>(osSide.size() - 1), timeoutToUse);
#else
		int retval = poll(&(osSide[1]), static_cast<nfds_t>(osSide.size() - 1), timeoutToUse);
#endif
		return std::make_pair(true, retval);
	}
};

class NetServerManagerForListenerThread : protected OSLayer
{
	friend class ListenerThreadWorker;

protected:
	NetSocketsForListenerThread ioSockets; // TODO: improve
	nodecpp::vector<size_t> pendingAcceptedEvents;
	//mb: ioSockets[0] is always reserved and invalid.
	nodecpp::vector<Error> errorStore;
	nodecpp::vector<size_t> pendingListenEvents;
	bool running = true;


public:
	NetServerManagerForListenerThread() {}

public:
	bool getAcceptedSockData(SOCKET s, OpaqueSocketData& osd, Ip4& remoteIp, Port& remotePort )
	{
		SocketRiia newSock(internal_usage_only::internal_tcp_accept(remoteIp, remotePort, s));
		if (newSock.get() == INVALID_SOCKET)
			return false;
		osd.s = std::move(newSock);
		return true;
	}

protected:
	nodecpp::IPFAMILY family = nodecpp::string_literal( "IPv4" );

public:
	void appAddAgentServerSocketAndStartListening( nodecpp::soft_ptr<ListenerThreadWorker::AgentServer> ptr, SOCKET socket)
	{
		ptr->dataForCommandProcessing.osSocket = socket;
		ioSockets.addEntry( ptr );
		ptr->dataForCommandProcessing.refed = true;
		ptr->dataForCommandProcessing.localAddress.family = family;
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, ptr->dataForCommandProcessing.index != 0 );
		ioSockets.setAssociated(ptr->dataForCommandProcessing.index);
		ioSockets.setPollin(ptr->dataForCommandProcessing.index);
		ioSockets.setRefed(ptr->dataForCommandProcessing.index, true);
	}

	void acquireSharedServerSocketAndStartListening( nodecpp::soft_ptr<ListenerThreadWorker::AgentServer> ptr, int backlog)
	{
		SocketRiia s(internal_usage_only::internal_make_shared_tcp_socket());
		if (!s)
		{
			throw Error();
		}
		ptr->dataForCommandProcessing.osSocket = s.release();

		Port myPort = Port::fromHost(ptr->dataForCommandProcessing.localAddress.port);
		if (!internal_usage_only::internal_bind_socket(ptr->dataForCommandProcessing.osSocket, ptr->dataForCommandProcessing.localAddress.ip, myPort)) {
			throw Error();
		}
		if (!internal_usage_only::internal_listen_tcp_socket(ptr->dataForCommandProcessing.osSocket, backlog)) {
			throw Error();
		}
		ioSockets.addEntry( ptr );
		ptr->dataForCommandProcessing.refed = true;
		ptr->dataForCommandProcessing.localAddress.family = family;
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, ptr->dataForCommandProcessing.index != 0 );
		ioSockets.setAssociated(ptr->dataForCommandProcessing.index);
		ioSockets.setPollin(ptr->dataForCommandProcessing.index);
		ioSockets.setRefed(ptr->dataForCommandProcessing.index, true);
	}

private:
	void infraCheckPollFdSet(NetSocketEntryForListenerThread& current, short revents)
	{
		if ((revents & (POLLERR | POLLNVAL)) != 0) // check errors first
		{
//!!//			nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id),"POLLERR event at {}", current.getServerSocketData()->osSocket);
			internal_usage_only::internal_getsockopt_so_error(current.getAgentServerData()->osSocket);
			nodecpp::Error e;
			current.getAgentServer()->onError( e );
			// TODO: consider removing socket
		}
		else if ((revents & POLLIN) != 0)
		{
//!!//			nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id),"POLLIN event at {}", current.getServerSocketData()->osSocket);
			OpaqueSocketData osd( false );
			Ip4 remoteIp;
			Port remotePort;
			if ( !getAcceptedSockData(current.getAgentServerData()->osSocket, osd, remoteIp, remotePort) )
				return;
			SOCKET osSocket = osd.s.release();
			current.getAgentServer()->onConnection( osSocket, remoteIp, remotePort );
		}
		else if (revents != 0)
		{
			nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id),"Unexpected event at {}, value {:x}", current.getAgentServerData()->osSocket, revents);
			internal_usage_only::internal_getsockopt_so_error(current.getAgentServerData()->osSocket);
			nodecpp::Error e;
			current.getAgentServer()->onError( e );
			// TODO: consider removing socket
		}
	}

	bool pollPhase2()
	{
#ifdef USE_TEMP_PERF_CTRS
extern thread_local size_t waitTime;
size_t now1 = infraGetCurrentTime();
		auto ret = ioSockets.wait( TimeOutNever );
waitTime += infraGetCurrentTime() - now1;
#else
		auto ret = ioSockets.wait( TimeOutNever );
#endif

		if ( !ret.first )
		{
			return false;
		}

		int retval = ret.second;

		if (retval < 0)
		{
#ifdef _MSC_VER
			int error = WSAGetLastError();
			//		if ( error == WSAEWOULDBLOCK )
			nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id),"error {}", error);
#else
			perror("select()");
			//		int error = errno;
			//		if ( error == EAGAIN || error == EWOULDBLOCK )
#endif
			/*        return WAIT_RESULTED_IN_TIMEOUT;*/
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical,false);
			nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id),"COMMLAYER_RET_FAILED");
			return false;
		}
		else if (retval == 0)
		{
			//timeout, just return with empty queue
			return true; 
		}
		else //if(retval)
		{
			int processed = 0;
			for ( size_t i=0; processed<retval; ++i)
			{
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::pedantic, i<ioSockets.size() );
				short revents = ioSockets.reventsAt( 1 + i );
#ifdef NODECPP_LINUX
				if ( revents )
				{
					NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::pedantic, (int64_t)(ioSockets.socketsAt(i + 1)) > 0, "indeed: {}", (int64_t)(ioSockets.socketsAt(i + 1)) );
#else
				if ( revents && (int64_t)(ioSockets.socketsAt(i + 1)) > 0 ) // on Windows WSAPoll() may set revents to a non-zero value despite the socket is invalid
				{
#endif
					++processed;
					if ( 1 + i >= ioSockets.reserved_capacity )
					{
						NetSocketEntryForListenerThread& current = ioSockets.at( 1 + i );
						if ( current.isAssociated() )
						{
							infraCheckPollFdSet(current, revents);
							break;
						}
					}
					else if ( 1 + i == ioSockets.awakerSockIdx )
					{
						// TODO: see infraCheckPollFdSet() for more details to be implemented
						if ((revents & POLLIN) != 0)
						{
							static constexpr size_t maxMsgCnt = 8;
							uint8_t recvBuffer[maxMsgCnt];
							size_t actaulFromSock = 0;
							bool res = OSLayer::infraGetPacketBytes(recvBuffer, maxMsgCnt, actaulFromSock, ioSockets.getAwakerSockSocket());
							if (res)
							{
								NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, actaulFromSock <= maxMsgCnt, "{} vs. {}", actaulFromSock, maxMsgCnt );
								InterThreadMsg thq[maxMsgCnt];
								size_t actualFromQueue = popFrontFromThisThreadQueue( thq, actaulFromSock );
								NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, actualFromQueue == actaulFromSock, "{} vs. {}", actualFromQueue, actaulFromSock );
								for ( size_t i=0; i<actualFromQueue; ++i )
									listenerThreadWorker.onInterthreadMessage( thq[i] );
							}
							else
							{
								internal_usage_only::internal_getsockopt_so_error(ioSockets.getAwakerSockSocket());
								// TODO: process error
							}
						}
					}
				}
			}
			return true;
		}
	}

public:
	void runLoop( uintptr_t listenerReadHandle )
	{
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical,isNetInitialized());
		ioSockets.setAwakerSocket( listenerReadHandle );
		listenerThreadWorker.postinit();
		while (running)
		{
			uint64_t now = infraGetCurrentTime();
			reportTimes( now );
			ioSockets.makeCompactIfNecessary();
			bool refed = pollPhase2();
			if(!refed)
				return;
		}
	}
};

extern thread_local NetServerManagerForListenerThread netServerManagerBaseForListenerThread;


#endif // NODECPP_ENABLE_CLUSTERING

#endif // TCP_SOCKET_FOR_LISTENER_THREAD_H


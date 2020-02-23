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

using namespace nodecpp;


class ListenerThreadWorker
{
private:
	size_t assignedThreadID;

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

	nodecpp::handler_ret_type reportThreadStarted()
	{
//				nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "reportThreadStarted() to Master at thread {}", assignedThreadID );
//				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, assignedThreadID != Cluster::InvalidThreadID ); 
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

	nodecpp::handler_ret_type processResponse( ThreadID requestingThreadId, ClusteringMsgHeader& mh, nodecpp::platform::internal_msg::InternalMsg::ReadIter& riter );

	/*nodecpp::handler_ret_type onInterthreadMessage( InterThreadMsg& msg )
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
	}*/


	nodecpp::handler_ret_type processInterthreadRequest( ThreadID requestingThreadId, ClusteringMsgHeader& mh, nodecpp::platform::internal_msg::InternalMsg::ReadIter& riter )
	{
//		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, mh.bodySize + offset <= b.size(), "{} + {} vs. {}", mh.bodySize, offset, b.size() ); 
		switch ( mh.type )
		{
			case ClusteringMsgHeader::ClusteringMsgType::ServerCloseRequest:
			{
//					NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, mh.assignedThreadID == assignedThreadID ); 
				nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "MasterSocket: processing ServerCloseRequest({}) request (for thread id: {}), entryIndex = {:x}", (size_t)(mh.type), requestingThreadId.slotId, mh.entryIdx );
//					nodecpp::safememory::soft_ptr<MasterSocket> me = myThis.getSoftPtr<MasterSocket>(this);
//				processRequestForServerCloseAtMaster( requestingThreadId, mh );
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


public:
	class AgentServer
	{
		friend class ListenerThreadWorker;
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
			sendConnAcceptedEv( socketsToSlaves[nextStep].targetThreadId, socketsToSlaves[nextStep].entryIndex, requestID, socket, remoteIp, remotePort ); // TODO-ITC: upgrade
			++nextStep;
			CO_RETURN;
		}
		nodecpp::handler_ret_type onError( nodecpp::Error& e ) { 
			nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id),"clustering Agent server: onError()!");
			for ( auto& slaveData : socketsToSlaves )
				sendServerErrorEv( socketsToSlaves[nextStep].targetThreadId, requestID, e ); // TODO-ITC: upgrade

			CO_RETURN;
		}
		nodecpp::handler_ret_type onEnd(bool hasError) { 
			nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id),"clustering Agent server: onEnd({})!", hasError);

			CO_RETURN;
		}

	public:
		void registerServer();

	public:
		AgentServer(ListenerThreadWorker& myCluster_) /*: myCluster( myCluster_ )*/ {
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

//private:

	friend void preinitMasterThreadClusterObject();
	friend void preinitSlaveThreadClusterObject(ThreadStartupData& startupData);
	friend void postinitThreadClusterObject();

	void preinit() { 
		//thisThreadWorker.id_ = 0; 
	}
	void postinit() { 
		reportThreadStarted();
	}
public:
	ListenerThreadWorker() {}
	ListenerThreadWorker(const ListenerThreadWorker&) = delete;
	ListenerThreadWorker& operator = (const ListenerThreadWorker&) = delete;
	ListenerThreadWorker(ListenerThreadWorker&&) = delete;
	ListenerThreadWorker& operator = (ListenerThreadWorker&&) = delete;

	// event handling (awaitable)
};
extern thread_local ListenerThreadWorker listenerThreadWorker;


static constexpr uint64_t TimeOutNever = std::numeric_limits<uint64_t>::max();

class NetSocketEntryForListenerThread {
	// TODO: revise everything around being 'refed'
	enum State { Unused, SockIssued, SockAssociated, SockClosed }; // TODO: revise!
	State state = State::Unused;
	nodecpp::safememory::soft_ptr<ListenerThreadWorker::AgentServer> ptr;

public:
	size_t index;
	bool refed = false;

	NetSocketEntryForListenerThread(size_t index) : state(State::Unused), index(index) {}
	NetSocketEntryForListenerThread(size_t index, nodecpp::safememory::soft_ptr<ListenerThreadWorker::AgentServer> ptr_) : state(State::SockIssued), index(index), ptr( ptr_ ) {ptr->dataForCommandProcessing.index = index;NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, ptr->dataForCommandProcessing.osSocket > 0 );}
	
	NetSocketEntryForListenerThread(const NetSocketEntryForListenerThread& other) = delete;
	NetSocketEntryForListenerThread& operator=(const NetSocketEntryForListenerThread& other) = delete;

	NetSocketEntryForListenerThread(NetSocketEntryForListenerThread&& other) = default;
	NetSocketEntryForListenerThread& operator=(NetSocketEntryForListenerThread&& other) = default;

	bool isUsed() const { NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, (state == State::Unused) || (state != State::Unused && ptr != nullptr) ); return state != State::Unused; }
	bool isAssociated() const { NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, (state == State::Unused) || (state != State::Unused && ptr != nullptr) ); return state == State::SockAssociated; }
	void setAssociated() {NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, state == State::SockIssued && ptr != nullptr ); state = State::SockAssociated;}
	void setSocketClosed() {NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, state != State::Unused ); state = State::SockClosed;}
	void setUnused() {state = State::Unused; index = 0;}

	nodecpp::safememory::soft_ptr<ListenerThreadWorker::AgentServer> getAgentServer() const { NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, ptr != nullptr); return ptr; }
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
	NetSocketEntryVectorT ourSideAccum;
	nodecpp::vector<pollfd> osSide;
	nodecpp::vector<pollfd> osSideAccum;
	size_t associatedCount = 0;
	size_t usedCount = 0;
public:
	//mb: xxxSide[0] is always reserved and invalid.
	//di: in clustering mode xxxSide[1] is always reserved and invalid (separate handling for awaker socket)
	//di: in clustering mode xxxSide[2] is always reserved and invalid (separate handling for InterThreadCommServer socket)
	static constexpr size_t awakerSockIdx = 1;
	static constexpr size_t reserved_capacity = 2;
private:
	static constexpr size_t capacity_ = 1 + reserved_capacity; // just a temporary workaround to prevent reallocation at arbitrary time; TODO: address properly!
	static constexpr size_t compactionMinSize = 32;

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
		if ( idx < ourSide.size() )
			return ourSide[idx].isUsed(); 
		else
		{
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, ourSide.size() == ourSide.capacity() && idx >= ourSide.size() ); 
			idx -= ourSide.capacity();
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, idx < ourSideAccum.size() );
			return ourSideAccum.at(idx).isUsed();
		}
	}
	NetSocketEntryForListenerThread& at(size_t idx) {
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, idx >= reserved_capacity ); 
		if ( idx < ourSide.size() )
		{
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, ourSide[idx].isUsed() ); 
			return ourSide.at(idx);
		}
		else
		{
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, ourSide.size() == ourSide.capacity() && idx >= ourSide.size() ); 
			idx -= ourSide.capacity();
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, idx < ourSideAccum.size() );
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, ourSideAccum[idx].isUsed() ); 
			return ourSideAccum.at(idx);
		}
	}
	const NetSocketEntryForListenerThread& at(size_t idx) const {
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, idx >= reserved_capacity ); 
		if ( idx < ourSide.size() )
		{
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, ourSide[idx].isUsed() ); 
			return ourSide.at(idx);
		}
		else
		{
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, ourSide.size() == ourSide.capacity() && idx >= ourSide.size() ); 
			idx -= ourSide.capacity();
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, idx < ourSideAccum.size() );
			return ourSideAccum.at(idx);
		}
	}
	short reventsAt(size_t idx) const {
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, idx != 0 ); 
		if ( idx < osSide.size() )
		{
			return osSide.at(idx).revents;
		}
		else
		{
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, osSide.size() == osSide.capacity() && idx >= osSide.size() ); 
			idx -= osSide.capacity();
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, idx < osSideAccum.size() );
			return osSideAccum.at(idx).revents;
		}
	}
	SOCKET socketsAt(size_t idx) const { 
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, idx != 0 ); 
		if ( idx < osSide.size() )
		{
			return osSide.at(idx).fd;
		}
		else
		{
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, osSide.size() == osSide.capacity() && idx >= osSide.size() ); 
			idx -= osSide.capacity();
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, idx < osSideAccum.size(), "{} vs. {}", idx, osSideAccum.size() );
			return osSideAccum.at(idx).fd;
		}
	}

	SOCKET getAwakerSockSocket() { return osSide[awakerSockIdx].fd; }

	size_t size() const {return ourSide.size() - 1; }
	bool isValidId( size_t idx ) { return idx >= reserved_capacity && idx < ourSide.size() + ourSideAccum.size(); }

	template<class SocketType>
	void addEntry(nodecpp::safememory::soft_ptr<SocketType> ptr) {
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

		if ( ourSide.size() < ourSide.capacity() )
		{
			size_t ix = ourSide.size();
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, ix < ourSide.capacity() ); // just a temporary workaround to prevent reallocation
			ourSide.emplace_back(ix, ptr);
			pollfd p;
			p.fd = (SOCKET)(-((int64_t)(ptr->dataForCommandProcessing.osSocket)));
			p.events = 0;
			p.revents = 0;
			osSide.push_back( p );
			++usedCount;
		}
		else // reallocation of all the underlaying array would happen
		{
			size_t bazeSz = ourSide.size();
			size_t ix = bazeSz + ourSideAccum.size();
			ourSideAccum.emplace_back(ix, ptr);
			pollfd p;
			p.fd = (SOCKET)(-((int64_t)(ptr->dataForCommandProcessing.osSocket)));
			p.events = 0;
			p.revents = 0;
			osSideAccum.push_back( p );
			++usedCount;
		}

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

	void reworkIfNecessary()
	{
		if ( ourSideAccum.size() )
		{
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, osSideAccum.size() == ourSideAccum.size(), "indeed: {} vs. {}", osSideAccum.size(), ourSideAccum.size() );
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, ourSide.size() == ourSide.capacity(), "indeed: {} vs. {}", ourSide.size(), ourSide.capacity() );
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, osSide.size() == ourSide.size(), "indeed: {} vs. {}", osSide.size(), ourSide.size() );
			for (size_t i=0; i != ourSideAccum.size(); ++i)
			{
//				ourSideAccum[i].updateIndex( ourSide.size() );
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, ourSideAccum[i].index == ourSide.size(), "indeed: {} vs. {}", ourSideAccum[i].index, ourSide.size() );
				ourSide.emplace_back( std::move( ourSideAccum[i] ) );
				osSide.push_back( osSideAccum[i] );
			}
			ourSideAccum.clear();
			osSideAccum.clear();
		}
		makeCompactIfNecessary();
	}

	void setAssociated( size_t idx/*, pollfd p*/ ) {
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, idx >= reserved_capacity ); 
		if ( idx < ourSide.size() )
		{
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, ourSide[idx].isUsed() ); 
			ourSide[idx].setAssociated();
			osSide[idx].fd = (SOCKET)(-((int64_t)(osSide[idx].fd)));
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, osSide[idx].events == 0, "indeed: {}", osSide[idx].events );
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, osSide[idx].revents == 0, "indeed: {}", osSide[idx].revents );
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, osSide[idx].fd > 0 );
			++associatedCount;
		}
		else
		{
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, ourSide.size() == ourSide.capacity() && idx >= ourSide.size() ); 
			idx -= ourSide.capacity();
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, idx < ourSideAccum.size() );
			ourSideAccum[idx].setAssociated();
			osSideAccum[idx].fd = (SOCKET)(-((int64_t)(osSideAccum[idx].fd)));
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, osSideAccum[idx].events == 0, "indeed: {}", osSideAccum[idx].events );
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, osSideAccum[idx].revents == 0, "indeed: {}", osSideAccum[idx].revents );
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, osSideAccum[idx].fd > 0 );
			++associatedCount;
		}
	}
	void setPollout( size_t idx ) {
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, idx >= reserved_capacity ); 
		if ( idx < ourSide.size() )
			osSide[idx].events |= POLLOUT; 
		else
		{
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, ourSide.size() == ourSide.capacity() && idx >= ourSide.size() ); 
			idx -= ourSide.capacity();
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, idx < ourSideAccum.size() );
			osSideAccum[idx].events |= POLLOUT; 
		}
	}
	void unsetPollout( size_t idx ) {
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, idx >= reserved_capacity ); 
		if ( idx < ourSide.size() )
			osSide[idx].events &= ~POLLOUT; 
		else
		{
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, ourSide.size() == ourSide.capacity() && idx >= ourSide.size() ); 
			idx -= ourSide.capacity();
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, idx < ourSideAccum.size() );
			osSideAccum[idx].events &= ~POLLOUT; 
		}
	}
	void setPollin( size_t idx ) {
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, idx >= reserved_capacity ); 
		if ( idx < ourSide.size() )
			osSide[idx].events |= POLLIN; 
		else
		{
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, ourSide.size() == ourSide.capacity() && idx >= ourSide.size() ); 
			idx -= ourSide.capacity();
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, idx < ourSideAccum.size() );
			osSideAccum[idx].events |= POLLIN; 
		}
	}
	void unsetPollin( size_t idx ) {
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, idx >= reserved_capacity ); 
		if ( idx < ourSide.size() )
			osSide[idx].events &= ~POLLIN; 
		else
		{
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, ourSide.size() == ourSide.capacity() && idx >= ourSide.size() ); 
			idx -= ourSide.capacity();
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, idx < ourSideAccum.size() );
			osSideAccum[idx].events &= ~POLLIN; 
		}
	}
	void setRefed( size_t idx, bool refed ) {
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, idx >= reserved_capacity ); 
		if ( idx < ourSide.size() )
		{
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, !refed || ourSide[idx].isUsed() ); 
			ourSide[idx].refed = refed;
		}
		else
		{
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, ourSide.size() == ourSide.capacity() && idx >= ourSide.size() ); 
			idx -= ourSide.capacity();
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, idx < ourSideAccum.size() );
			ourSideAccum[idx].refed = refed;
		}
	}
	void setUnused( size_t idx ) {
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, idx >= reserved_capacity ); 
		if ( idx < ourSide.size() )
		{
			if ( osSide[idx].fd != INVALID_SOCKET )
			{
				osSide[idx].fd = INVALID_SOCKET; 
				--associatedCount;
			}
			ourSide[idx].setUnused();
			--usedCount;
		}
		else
		{
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, ourSide.size() == ourSide.capacity() && idx >= ourSide.size() ); 
			idx -= ourSide.capacity();
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, idx < ourSideAccum.size() );
			if ( osSideAccum[idx].fd != INVALID_SOCKET )
			{
				osSideAccum[idx].fd = INVALID_SOCKET; 
				--associatedCount;
			}
			ourSideAccum[idx].setUnused();
			--usedCount;
		}

	}
	void setSocketClosed( size_t idx ) {
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, idx >= reserved_capacity ); 
		if ( idx < ourSide.size() )
		{
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, ourSide[idx].isUsed() ); 
			if ( ourSide[idx].isAssociated() )
				--associatedCount;
			osSide[idx].fd = INVALID_SOCKET; 
			ourSide[idx].setSocketClosed();
		}
		else
		{
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, ourSide.size() == ourSide.capacity() && idx >= ourSide.size() ); 
			idx -= ourSide.capacity();
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, idx < ourSideAccum.size() );
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, ourSideAccum[idx].isUsed() ); 
			if ( ourSideAccum[idx].isAssociated() )
				--associatedCount;
			osSideAccum[idx].fd = INVALID_SOCKET; 
			ourSideAccum[idx].setSocketClosed();
		}
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
//	friend class OSLayer;
	friend class ListenerThreadWorker;

protected:
	NetSocketsForListenerThread ioSockets; // TODO: improve
	//nodecpp::vector<std::pair<size_t, std::pair<bool, Error>>> pendingCloseEvents;
	nodecpp::vector<size_t> pendingAcceptedEvents;
	//mb: ioSockets[0] is always reserved and invalid.
	//nodecpp::vector<std::pair<size_t, bool>> pendingCloseEvents;
	PendingEvQueue pendingEvents;
	nodecpp::vector<Error> errorStore;
	nodecpp::vector<size_t> pendingListenEvents;
	bool running;


public:
	NetServerManagerForListenerThread() {}

public:
	void infraAddAccepted(soft_ptr<net::SocketBase> ptr)
	{
		size_t id = ptr->dataForCommandProcessing.index;
		pendingAcceptedEvents.push_back(id);
		ptr->dataForCommandProcessing.state = net::SocketBase::DataForCommandProcessing::Connected;
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, ptr->dataForCommandProcessing.writeBuffer.used_size() == 0 );
		ioSockets.unsetPollout(id);
		ptr->dataForCommandProcessing.refed = true;
		auto& entry = appGetEntry(id);
		entry.refed = true; 
		ioSockets.setAssociated(id);
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical,ptr->dataForCommandProcessing.remoteEnded == false);
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical,ptr->dataForCommandProcessing.paused == false);
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical,ptr->dataForCommandProcessing.state != net::SocketBase::DataForCommandProcessing::Connecting);
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical,ptr->dataForCommandProcessing.writeBuffer.empty());
		ioSockets.setPollin(id);
	}

	/*void appAcquireSocket(nodecpp::safememory::soft_ptr<net::SocketBase> ptr)
	{
		SocketRiia s( OSLayer::appAcquireSocket() );
		registerAndAssignSocket(ptr, s);
	}*/

	/*void appAssignSocket(nodecpp::safememory::soft_ptr<net::SocketBase> ptr, OpaqueSocketData& sdata)
	{
		SocketRiia s( sdata.s.release() );
		registerAndAssignSocket(ptr, s);
	}*/

	static SocketRiia extractSocket(OpaqueSocketData& sdata)
	{
		return sdata.s.release();
	}

private:
	/*void registerAndAssignSocket(nodecpp::safememory::soft_ptr<net::SocketBase> ptr, SocketRiia& s)
	{
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical,ptr->dataForCommandProcessing.state == net::SocketBase::DataForCommandProcessing::Uninitialized);
		ptr->dataForCommandProcessing.osSocket = s.release();
		ioSockets.addEntry<net::SocketBase>(ptr);
	}*/

public:
	void appConnectSocket(net::SocketBase* sockPtr, const char* ip, uint16_t port)
	{
		// TODO: check sockPtr validity
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, (SOCKET)(sockPtr->dataForCommandProcessing.osSocket) != INVALID_SOCKET);
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical,sockPtr->dataForCommandProcessing.state == net::SocketBase::DataForCommandProcessing::Uninitialized);
		OSLayer::appConnectSocket(sockPtr->dataForCommandProcessing.osSocket, ip, port );
		sockPtr->dataForCommandProcessing.state = net::SocketBase::DataForCommandProcessing::Connecting;
		sockPtr->dataForCommandProcessing.refed = true;
		ioSockets.setRefed( sockPtr->dataForCommandProcessing.index, true );
		ioSockets.setAssociated(sockPtr->dataForCommandProcessing.index/*, p*/ );
		ioSockets.setPollin(sockPtr->dataForCommandProcessing.index);
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical,sockPtr->dataForCommandProcessing.state == net::SocketBase::DataForCommandProcessing::Connecting);
		ioSockets.setPollout(sockPtr->dataForCommandProcessing.index);
	}
	bool getAcceptedSockData(SOCKET s, OpaqueSocketData& osd, Ip4& remoteIp, Port& remotePort )
	{
		SocketRiia newSock(internal_usage_only::internal_tcp_accept(remoteIp, remotePort, s));
		if (newSock.get() == INVALID_SOCKET)
			return false;
		osd.s = std::move(newSock);
		return true;
	}

protected:
	NetSocketEntryForListenerThread& appGetEntry(size_t id) { return ioSockets.at(id); }
	const NetSocketEntryForListenerThread& appGetEntry(size_t id) const { return ioSockets.at(id); }

	void closeSocket(NetSocketEntryForListenerThread& entry) //app-infra neutral
	{
		/*if ( !( entry.getAgentServerData()->state == net::SocketBase::DataForCommandProcessing::Closing ||
				entry.getAgentServerData()->state == net::SocketBase::DataForCommandProcessing::ErrorClosing ||
				entry.getAgentServerData()->state == net::SocketBase::DataForCommandProcessing::Closed ) )
		{
			entry.getAgentServerData()->state = net::SocketBase::DataForCommandProcessing::Closing;
			pendingCloseEvents.push_back(std::make_pair( entry.index, std::make_pair( false, Error())));
		}*/
	}
	void errorCloseSocket(NetSocketEntryForListenerThread& entry, Error& err) //app-infra neutral
	{
		/*if ( !( entry.getAgentServerData()->state == net::SocketBase::DataForCommandProcessing::Closing ||
				entry.getAgentServerData()->state == net::SocketBase::DataForCommandProcessing::ErrorClosing ||
				entry.getAgentServerData()->state == net::SocketBase::DataForCommandProcessing::Closed ) )
		{
			entry.getAgentServerData()->state = net::SocketBase::DataForCommandProcessing::ErrorClosing;
			pendingCloseEvents.push_back(std::make_pair( entry.index, std::make_pair( true, err)));
		}*/
	}

public:
	void appRef(size_t id) { 
		ioSockets.setRefed( id, true );
	}
	void appUnref(size_t id) { 
		ioSockets.setRefed( id, false );
	}
	void appReportBeingDestructed(size_t id) { 
		ioSockets.setUnused( id );
	}

protected:
	struct AcceptedSocketData
	{
		size_t idx;
		SOCKET socket;
		Ip4 remoteIp;
		Port remotePort;
	};
	nodecpp::vector<AcceptedSocketData> acceptedSockets;
	nodecpp::vector<size_t> receivedListeningEvs;

	nodecpp::IPFAMILY family = nodecpp::string_literal( "IPv4" );

public:
	template<class DataForCommandProcessing>
	void appClose(DataForCommandProcessing& serverData) {
		size_t id = serverData.index;
		auto& entry = appGetEntry(id);
		if (!entry.isUsed())
		{
			nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id),"Unexpected id {} on NetServerManager::close", id);
			return;
		}
		internal_usage_only::internal_close(serverData.osSocket);
		ioSockets.setSocketClosed( entry.index );
		//pendingCloseEvents.emplace_back(entry.index, false); note: it will be finally closed only after all accepted connections are ended
	}
	void appReportAllAceptedConnectionsEnded(net::ServerBase::DataForCommandProcessing& serverData) {
		size_t id = serverData.index;
		auto& entry = appGetEntry(id);
		if (!entry.isUsed())
		{
			nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id),"Unexpected id {} on NetServerManager::close", id);
			return;
		}
		//pendingCloseEvents.emplace_back(id, false);
	}

	void appAddAgentServer(nodecpp::safememory::soft_ptr<ListenerThreadWorker::AgentServer> ptr) {
		SocketRiia s(internal_usage_only::internal_make_tcp_socket());
		if (!s)
		{
			throw Error();
		}
		ptr->dataForCommandProcessing.osSocket = s.release();
		addAgentServerEntry(ptr);
	}

	template<class DataForCommandProcessing>
	void appListen(DataForCommandProcessing& dataForCommandProcessing, nodecpp::Ip4 ip, uint16_t port, int backlog) { //TODO:CLUSTERING alt impl
		Port myPort = Port::fromHost(port);
		if (!internal_usage_only::internal_bind_socket(dataForCommandProcessing.osSocket, ip, myPort)) {
			throw Error();
		}
		if ( port == 0 )
		{
			port = internal_usage_only::internal_port_of_tcp_socket(dataForCommandProcessing.osSocket);
			if ( port == 0 )
				throw Error();
		}
		if (!internal_usage_only::internal_listen_tcp_socket(dataForCommandProcessing.osSocket, backlog)) {
			throw Error();
		}
		dataForCommandProcessing.refed = true;
		dataForCommandProcessing.localAddress.ip = ip;
		dataForCommandProcessing.localAddress.port = port;
		dataForCommandProcessing.localAddress.family = family;
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, dataForCommandProcessing.index != 0 );
		/*pollfd p;
		p.fd = dataForCommandProcessing.osSocket;
		p.events = POLLIN;*/
		ioSockets.setAssociated(dataForCommandProcessing.index/*, p*/);
		ioSockets.setPollin(dataForCommandProcessing.index);
		ioSockets.setRefed(dataForCommandProcessing.index, true);
		pendingListenEvents.push_back( dataForCommandProcessing.index );
	}
	template<class DataForCommandProcessing>
	void appListen(DataForCommandProcessing& dataForCommandProcessing, const char* ip, uint16_t port, int backlog) { //TODO:CLUSTERING alt impl
		Ip4 ip4 = Ip4::parse(ip);
		appListen<DataForCommandProcessing>(dataForCommandProcessing, ip4, port, backlog);
	}
	template<class DataForCommandProcessing>
	void appRef(DataForCommandProcessing& dataForCommandProcessing) { 
		dataForCommandProcessing.refed = true;
	}
	template<class DataForCommandProcessing>
	void appUnref(DataForCommandProcessing& dataForCommandProcessing) { 
		dataForCommandProcessing.refed = true;
	}
	template<class DataForCommandProcessing>
	void appReportBeingDestructed(DataForCommandProcessing& dataForCommandProcessing) {
		size_t id = dataForCommandProcessing.index;
			ioSockets.setUnused(id); 
	}

	//TODO quick workaround until definitive life managment is in place
	Error& infraStoreError(Error err) {
		errorStore.push_back(std::move(err));
		return errorStore.back();
	}

	void infraClearStores() {
		errorStore.clear();
	}

	// to help with 'poll'
	//size_t infraGetPollFdSetSize() const { return ioSockets.size(); }
	void infraGetPendingEvents(EvQueue& evs) { pendingEvents.toQueue(evs); }

protected:
	void addAgentServerEntry(nodecpp::safememory::soft_ptr<ListenerThreadWorker::AgentServer> ptr);

public:
	void infraEmitListeningEvents()
	{
		while ( pendingListenEvents.size() )
		{
			nodecpp::vector<size_t> currentPendingListenEvents = std::move( pendingListenEvents );
			for (auto& current : currentPendingListenEvents)
			{
				if (ioSockets.isValidId(current))
				{
					auto& entry = ioSockets.at(current);
					if (entry.isUsed())
					{
						entry.getAgentServerData()->state = ListenerThreadWorker::AgentServer::DataForCommandProcessing::State::Listening;
						entry.getAgentServer()->onListening();
					}
				}
			}
		}
	}

	void infraCheckPollFdSet(NetSocketEntryForListenerThread& current, short revents)
	{
		if ((revents & (POLLERR | POLLNVAL)) != 0) // check errors first
		{
//!!//			nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id),"POLLERR event at {}", current.getServerSocketData()->osSocket);
			internal_usage_only::internal_getsockopt_so_error(current.getAgentServerData()->osSocket);
			infraMakeErrorEventAndClose(current);
		}
		else if ((revents & POLLIN) != 0)
		{
//!!//			nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id),"POLLIN event at {}", current.getServerSocketData()->osSocket);
			infraProcessAcceptEvent(current);
		}
		else if (revents != 0)
		{
			nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id),"Unexpected event at {}, value {:x}", current.getAgentServerData()->osSocket, revents);
			internal_usage_only::internal_getsockopt_so_error(current.getAgentServerData()->osSocket);
			infraMakeErrorEventAndClose(current);
		}
	}

private:
	void infraProcessAcceptEvent(NetSocketEntryForListenerThread& entry) //TODO:CLUSTERING alt impl
	{
		OpaqueSocketData osd( false );

		Ip4 remoteIp;
		Port remotePort;
		if ( !getAcceptedSockData(entry.getAgentServerData()->osSocket, osd, remoteIp, remotePort) )
			return;
		SOCKET osSocket = extractSocket( osd ).release();
		entry.getAgentServer()->onConnection( osSocket, remoteIp, remotePort );
		return;
	}

	void infraMakeErrorEventAndClose(NetSocketEntryForListenerThread& entry)
	{
		Error e;
		// TODO: special Clustering treatment
		return;
	}

	bool pollPhase2()
	{
		auto ret = ioSockets.wait( TimeOutNever );

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
							Buffer recvBuffer(maxMsgCnt);
							bool res = OSLayer::infraGetPacketBytes(recvBuffer, ioSockets.getAwakerSockSocket());
							if (res)
							{
								NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, recvBuffer.size() <= maxMsgCnt, "{} vs. {}", recvBuffer.size(), maxMsgCnt );
								InterThreadMsg thq[maxMsgCnt];
								size_t actual = popFrontFromThisThreadQueue( thq, recvBuffer.size() );
								NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, actual == recvBuffer.size(), "{} vs. {}", actual, recvBuffer.size() );
								for ( size_t i=0; i<actual; ++i )
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

	void doBasicInitialization()
	{
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical,isNetInitialized());

		ioSockets.reworkIfNecessary();
	}

	void runLoop()
	{
		while (running)
		{
			EvQueue queue;

			infraGetPendingEvents(queue);
			infraEmitListeningEvents();
			queue.emit();

			bool refed = pollPhase2();
			if(!refed)
				return;

			queue.emit();

			infraClearStores();

			ioSockets.reworkIfNecessary();
		}
	}

	void run()
	{
		listenerThreadWorker.postinit();
		doBasicInitialization();
		runLoop();
	}
};

extern thread_local NetServerManagerForListenerThread netServerManagerBase;


#endif // NODECPP_ENABLE_CLUSTERING

#endif // TCP_SOCKET_FOR_LISTENER_THREAD_H

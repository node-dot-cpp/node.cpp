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


#ifndef TCP_SOCKET_H
#define TCP_SOCKET_H

#include "tcp_socket_base.h"
#include "clustering_impl/clustering_common.h"

using namespace nodecpp;

class NetSocketEntry {
	// TODO: revise everything around being 'refed'
	enum State { Unused, SockIssued, SockAssociated, SockClosed }; // TODO: revise!
	State state = State::Unused;

public:
	size_t index;
	bool refed = false;
	OpaqueEmitter emitter;

	NetSocketEntry(size_t index) : state(State::Unused), index(index) {}
	NetSocketEntry(size_t index/*, NodeBase* node*/, nodecpp::safememory::soft_ptr<net::SocketBase> ptr, int type) : state(State::SockIssued), index(index), emitter(OpaqueEmitter::ObjectType::ClientSocket/*, node*/, ptr, type) {ptr->dataForCommandProcessing.index = index;NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, ptr->dataForCommandProcessing.osSocket > 0 );}
	NetSocketEntry(size_t index/*, NodeBase* node*/, nodecpp::safememory::soft_ptr<net::ServerBase> ptr, int type) : state(State::SockIssued), index(index), emitter(OpaqueEmitter::ObjectType::ServerSocket/*, node*/, ptr, type) {
		ptr->dataForCommandProcessing.index = index;
#ifndef NODECPP_ENABLE_CLUSTERING
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, ptr->dataForCommandProcessing.osSocket > 0 );
#else
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, getCluster().isWorker() || ptr->dataForCommandProcessing.osSocket > 0 );
#endif // NODECPP_ENABLE_CLUSTERING
	}
	NetSocketEntry(size_t index/*, NodeBase* node*/, nodecpp::safememory::soft_ptr<Cluster::AgentServer> ptr, int type) : state(State::SockIssued), index(index), emitter(OpaqueEmitter::ObjectType::AgentServer/*, node*/, ptr, type) {ptr->dataForCommandProcessing.index = index;NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, ptr->dataForCommandProcessing.osSocket > 0 );}
	
	NetSocketEntry(const NetSocketEntry& other) = delete;
	NetSocketEntry& operator=(const NetSocketEntry& other) = delete;

	NetSocketEntry(NetSocketEntry&& other) = default;
	NetSocketEntry& operator=(NetSocketEntry&& other) = default;

	OpaqueEmitter::ObjectType getObjectType() {return emitter.objectType; }

	bool isUsed() const { NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, (state == State::Unused) || (state != State::Unused && emitter.isValid()) ); return state != State::Unused; }
	bool isAssociated() const { NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, (state == State::Unused) || (state != State::Unused && emitter.isValid()) ); return state == State::SockAssociated; }
	void setAssociated() {NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, state == State::SockIssued && emitter.isValid() ); state = State::SockAssociated;}
	void setSocketClosed() {NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, state != State::Unused ); state = State::SockClosed;}
	void setUnused() {state = State::Unused; index = 0;}

	const OpaqueEmitter& getEmitter() const { return emitter; }
	nodecpp::safememory::soft_ptr<net::SocketBase> getClientSocket() const { NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical,emitter.isValid()); NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, emitter.objectType == OpaqueEmitter::ObjectType::ClientSocket); return emitter.getClientSocketPtr(); }
	nodecpp::safememory::soft_ptr<net::ServerBase> getServerSocket() const { NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical,emitter.isValid()); NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, emitter.objectType == OpaqueEmitter::ObjectType::ServerSocket); return emitter.getServerSocketPtr(); }
	net::SocketBase::DataForCommandProcessing* getClientSocketData() const { NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical,emitter.isValid()); NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, emitter.objectType == OpaqueEmitter::ObjectType::ClientSocket); return emitter.getClientSocketPtr() ? &( emitter.getClientSocketPtr()->dataForCommandProcessing ) : nullptr; }
	net::ServerBase::DataForCommandProcessing* getServerSocketData() const { NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical,emitter.isValid()); NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, emitter.objectType == OpaqueEmitter::ObjectType::ServerSocket); return emitter.getServerSocketPtr() ? &( emitter.getServerSocketPtr()->dataForCommandProcessing ) : nullptr; }

#ifdef NODECPP_ENABLE_CLUSTERING
	nodecpp::safememory::soft_ptr<Cluster::AgentServer> getAgentServer() const { NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical,emitter.isValid()); NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, emitter.objectType == OpaqueEmitter::ObjectType::AgentServer); return emitter.getAgentServerPtr(); }
	Cluster::AgentServer::DataForCommandProcessing* getAgentServerData() const { NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical,emitter.isValid()); NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, emitter.objectType == OpaqueEmitter::ObjectType::AgentServer); return emitter.getAgentServerPtr() ? &( emitter.getAgentServerPtr()->dataForCommandProcessing ) : nullptr; }
#endif // NODECPP_ENABLE_CLUSTERING

	void updateIndex( size_t idx ) {
		index = idx;
		switch ( emitter.objectType ) {
			case OpaqueEmitter::ObjectType::ClientSocket: {
				auto* pdata = getClientSocketData();
				if ( pdata )
					pdata->index = idx;
				break;
			}
			case OpaqueEmitter::ObjectType::ServerSocket: {
				auto* pdata = getServerSocketData();
				if ( pdata )
					pdata->index = idx;
				break;
			}
#ifdef NODECPP_ENABLE_CLUSTERING
			case OpaqueEmitter::ObjectType::AgentServer: {
				auto* pdata = getAgentServerData();
				if ( pdata )
					pdata->index = idx;
				break;
			}
#endif // NODECPP_ENABLE_CLUSTERING
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type {}", (size_t)(emitter.objectType));
				break;
		}
	}
};

class NetSockets
{
	std::vector<NetSocketEntry> ourSide;
	std::vector<NetSocketEntry> ourSideAccum;
#ifdef NODECPP_ENABLE_CLUSTERING
	std::vector<NetSocketEntry> slaveServers;
	static constexpr size_t SlaveServerEntryMinIndex = (((size_t)~((size_t)0))>>1)+1;
#endif // NODECPP_ENABLE_CLUSTERING
	std::vector<pollfd> osSide;
	std::vector<pollfd> osSideAccum;
	size_t associatedCount = 0;
	size_t usedCount = 0;
	//mb: xxxSide[0] is always reserved and invalid.
	static const size_t capacity_ = 2; // just a temporary workaround to prevent reallocation at arbitrary time; TODO: address properly!
	static const size_t compactionMinSize = 32;

	void makeCompactIfNecessary() {
		if ( ourSide.size() <= compactionMinSize || usedCount < ourSide.size() / 2 )
			return;
		std::vector<NetSocketEntry> ourSideNew;
		std::vector<pollfd> osSideNew;
		ourSideNew.reserve(capacity_); 
		osSideNew.reserve(capacity_);
		ourSideNew.emplace_back(0); 
		osSideNew.emplace_back();
		size_t usedCountNew = 0;
		for (size_t i = 1; i != ourSide.size(); ++i) // skip ourSide[0]
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

	NetSockets() {ourSide.reserve(capacity_); osSide.reserve(capacity_); ourSide.emplace_back(0); osSide.emplace_back();}

	NetSocketEntry& at(size_t idx) {
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, idx != 0 ); 
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
	const NetSocketEntry& at(size_t idx) const {
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, idx != 0 ); 
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
		if ( idx < ourSide.size() )
		{
			return osSide.at(idx).revents;
		}
		else
		{
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, ourSide.size() == ourSide.capacity() && idx >= ourSide.size() ); 
			idx -= ourSide.capacity();
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, idx < ourSideAccum.size() );
			return osSideAccum.at(idx).revents;
		}
	}
	SOCKET socketsAt(size_t idx) const { 
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, idx != 0 ); 
		if ( idx < ourSide.size() )
		{
			return osSide.at(idx).fd;
		}
		else
		{
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, ourSide.size() == ourSide.capacity() && idx >= ourSide.size() ); 
			idx -= ourSide.capacity();
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, idx < ourSideAccum.size() );
			return osSideAccum.at(idx).fd;
		}
	}
	size_t size() const {return ourSide.size() - 1; }
	bool isValidId( size_t idx ) { return idx && idx < ourSide.size() + ourSideAccum.size(); }

	template<class SocketType>
	void addEntry(/*NodeBase* node, */nodecpp::safememory::soft_ptr<SocketType> ptr, int typeId) {
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, ourSide.size() == osSide.size() );
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, ptr->dataForCommandProcessing.osSocket > 0 );
		for (size_t i = 1; i != ourSide.size(); ++i) // skip ourSide[0]
		{
			if (!ourSide[i].isUsed())
			{
				NetSocketEntry entry(i/*, node*/, ptr, typeId);
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
			ourSide.emplace_back(ix/*, node*/, ptr, typeId);
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
			ourSideAccum.emplace_back(ix/*, node*/, ptr, typeId);
			pollfd p;
			p.fd = (SOCKET)(-((int64_t)(ptr->dataForCommandProcessing.osSocket)));
			p.events = 0;
			p.revents = 0;
			osSideAccum.push_back( p );
			++usedCount;
		}

		return;
	}
#ifdef NODECPP_ENABLE_CLUSTERING
	NetSocketEntry& slaveServerAt(size_t idx) {
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, idx >= SlaveServerEntryMinIndex ); 
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, idx < SlaveServerEntryMinIndex + slaveServers.size() ); 
		return slaveServers.at(idx - SlaveServerEntryMinIndex);
	}
	const NetSocketEntry& slaveServerAt(size_t idx) const {
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, idx >= SlaveServerEntryMinIndex ); 
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, idx < SlaveServerEntryMinIndex + slaveServers.size() ); 
		return slaveServers.at(idx - SlaveServerEntryMinIndex);
	}
	template<class SocketType>
	void addSlaveServerEntry(/*NodeBase* node, */nodecpp::safememory::soft_ptr<SocketType> ptr, int typeId) {
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, getCluster().isWorker() );
		for (size_t i = 0; i != slaveServers.size(); ++i) // skip ourSide[0]
		{
			if (!slaveServers[i].isUsed())
			{
				NetSocketEntry entry(SlaveServerEntryMinIndex + i/*, node*/, ptr, typeId);
				slaveServers[i] = std::move(entry);
//				++usedCount;
				return;
			}
		}

		size_t ix = slaveServers.size();
		slaveServers.emplace_back(SlaveServerEntryMinIndex + ix/*, node*/, ptr, typeId);
//		++usedCount;
		return;
	}
#endif // NODECPP_ENABLE_CLUSTERING
	void reworkIfNecessary()
	{
		if ( ourSideAccum.size() )
		{
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, osSideAccum.size() == ourSideAccum.size(), "indeed; {} vs. {}", osSideAccum.size(), ourSideAccum.size() );
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, ourSide.size() == ourSide.capacity(), "indeed; {} vs. {}", ourSide.size(), ourSide.capacity() );
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, osSide.size() == ourSide.size(), "indeed; {} vs. {}", osSide.size(), ourSide.size() );
			for (size_t i=0; i != ourSideAccum.size(); ++i)
			{
//				ourSideAccum[i].updateIndex( ourSide.size() );
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, ourSideAccum[i].index == ourSide.size(), "indeed; {} vs. {}", ourSideAccum[i].index, ourSide.size() );
				ourSide.emplace_back( std::move( ourSideAccum[i] ) );
				osSide.push_back( osSideAccum[i] );
			}
			ourSideAccum.clear();
			osSideAccum.clear();
		}
		makeCompactIfNecessary();
	}

	void setAssociated( size_t idx/*, pollfd p*/ ) {
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, idx != 0 ); 
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
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, idx != 0 ); 
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
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, idx != 0 ); 
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
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, idx != 0 ); 
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
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, idx != 0 ); 
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
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, idx != 0 ); 
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
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, idx != 0 ); 
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
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, idx != 0 ); 
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
#ifdef NODECPP_ENABLE_CLUSTERING
	void setSlaveSocketClosed( size_t idx ) {
		NetSocketEntry& entry = slaveServerAt(idx);
		entry.setSocketClosed();
	}
#endif // NODECPP_ENABLE_CLUSTERING
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

class NetSocketManagerBase : protected OSLayer
{
	friend class OSLayer;
//	std::vector<Buffer> bufferStore; // TODO: improve

public:
//	int typeIndexOfSocketO = -1;
//	int typeIndexOfSocketL = -1;

protected:
	NetSockets& ioSockets; // TODO: improve
	std::vector<std::pair<size_t, std::pair<bool, Error>>> pendingCloseEvents;
	std::vector<size_t> pendingAcceptedEvents;

public:
	static constexpr size_t MAX_SOCKETS = 100; //arbitrary limit

public:
	NetSocketManagerBase(NetSockets& ioSockets_) : ioSockets( ioSockets_) {}

	//TODO quick workaround until definitive life managment is in place
	/*Buffer& infraStoreBuffer(Buffer buff) {
		bufferStore.push_back(std::move(buff));
		return bufferStore.back();
	}

	void infraClearStores() {
		bufferStore.clear();
	}*/

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

	void appAcquireSocket(/*NodeBase* node, */nodecpp::safememory::soft_ptr<net::SocketBase> ptr, int typeId)
	{
		SocketRiia s( OSLayer::appAcquireSocket() );
		registerAndAssignSocket(/*node, */ptr, typeId, s);
	}

	void appAssignSocket(/*NodeBase* node, */nodecpp::safememory::soft_ptr<net::SocketBase> ptr, int typeId, OpaqueSocketData& sdata)
	{
		SocketRiia s( sdata.s.release() );
		registerAndAssignSocket(/*node, */ptr, typeId, s);
	}

	static SocketRiia extractSocket(OpaqueSocketData& sdata)
	{
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::pedantic, getCluster().isMaster() );
		return sdata.s.release();
	}

	static OpaqueSocketData createOpaqueSocketData( SOCKET socket ) { return OpaqueSocketData( socket ); }

private:
	void registerAndAssignSocket(/*NodeBase* node, */nodecpp::safememory::soft_ptr<net::SocketBase> ptr, int typeId, SocketRiia& s)
	{
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical,ptr->dataForCommandProcessing.state == net::SocketBase::DataForCommandProcessing::Uninitialized);
		ptr->dataForCommandProcessing.osSocket = s.release();
		ioSockets.addEntry<net::SocketBase>(/*node, */ptr, typeId);
	}

public:
	void appConnectSocket(net::SocketBase* sockPtr, const char* ip, uint16_t port) // TODO: think about template with type checking inside
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
	bool appWrite(net::SocketBase::DataForCommandProcessing& sockData, const uint8_t* data, uint32_t size);
	bool appWrite2(net::SocketBase::DataForCommandProcessing& sockData, Buffer& b );
	bool getAcceptedSockData(SOCKET s, OpaqueSocketData& osd, Ip4& remoteIp, Port& remotePort )
	{
		SocketRiia newSock(internal_usage_only::internal_tcp_accept(remoteIp, remotePort, s));
		if (newSock.get() == INVALID_SOCKET)
			return false;
		osd.s = std::move(newSock);
		return true;
	}

protected:
	NetSocketEntry& appGetEntry(size_t id) { return ioSockets.at(id); }
	const NetSocketEntry& appGetEntry(size_t id) const { return ioSockets.at(id); }

	void closeSocket(NetSocketEntry& entry) //app-infra neutral
	{
		entry.getClientSocketData()->state = net::SocketBase::DataForCommandProcessing::Closing;
		//entry.getClientSocketData()->refed = true;
		//entry.refed = false;

	//	evs.add(&net::Socket::emitError, entry.getPtr(), storeError(Error()));
	//	pendingCloseEvents.emplace_back(entry.index, std::function<void()>());
		pendingCloseEvents.push_back(std::make_pair( entry.index, std::make_pair( false, Error())));
	}
	void errorCloseSocket(NetSocketEntry& entry, Error& err) //app-infra neutral
	{
		entry.getClientSocketData()->state = net::SocketBase::DataForCommandProcessing::ErrorClosing;
		//entry.getClientSocketData()->refed = true;
		//entry.refed = false;

	//	std::function<void()> ev = std::bind(&net::Socket::emitError, entry.getPtr(), std::ref(err));
	//	std::function<void()> ev = std::bind(&net::SocketEmitter::emitError, entry.getEmitter(), std::ref(err));
	//	pendingCloseEvents.emplace_back(entry.index, std::move(ev));
		pendingCloseEvents.push_back(std::make_pair( entry.index, std::make_pair( true, err)));
	}

public:
	void appRef(size_t id) { 
		ioSockets.setRefed( id, true );
	}
	void appUnref(size_t id) { 
		ioSockets.setRefed( id, false );
	}
	void appPause(size_t id) { 
		auto& entry = appGetEntry(id);
		entry.getClientSocketData()->paused = true; }
	void appResume(size_t id) { 
		auto& entry = appGetEntry(id);
		entry.getClientSocketData()->paused = false; 
	}
	void appReportBeingDestructed(size_t id) { 
		/*auto& entry = appGetEntry(id);
		//entry.getClientSocketData()->refed = false; 
		entry.setUnused(); */
		ioSockets.setUnused( id );
	}

protected:
	enum ShouldEmit { EmitNone, EmitConnect, EmitDrain };
	ShouldEmit _infraProcessWriteEvent(net::SocketBase::DataForCommandProcessing& sockData);
};

extern thread_local NetSocketManagerBase* netSocketManagerBase;

template<class EmitterType>
class NetSocketManager : public NetSocketManagerBase {
	Buffer recvBuffer;
	static constexpr size_t recvBufferCapacity = 64 * 1024;

public:
	NetSocketManager(NetSockets& ioSockets) : NetSocketManagerBase(ioSockets), recvBuffer(recvBufferCapacity) {}

	// to help with 'poll'
	template<class Node>
	void infraGetCloseEvent(/*EvQueue& evs*/)
	{
		// if there is an issue with a socket, we may need to appClose it,
		// and push an event here to notify autom later.

		for (auto& current : pendingCloseEvents)
		{
			if (ioSockets.isValidId(current.first))
			{
				auto& entry = ioSockets.at(current.first);
				if (entry.isUsed())
				{
					bool err = entry.getClientSocketData()->state == net::SocketBase::DataForCommandProcessing::ErrorClosing;
					if ( (SOCKET)(entry.getClientSocketData()->osSocket) != INVALID_SOCKET)
					{
						if(err) // if error closing, then discard all buffers
							internal_usage_only::internal_linger_zero_socket(entry.getClientSocketData()->osSocket);

						internal_usage_only::internal_close(entry.getClientSocketData()->osSocket);
						ioSockets.setSocketClosed( entry.index );
					}

					if (err && entry.isUsed()) //if error closing, then first error event
					{
						entry.getClientSocket()->emitError(current.second.second);
						if constexpr ( !std::is_same<EmitterType, void>::value )
						{
							if ( EmitterType::template isErrorEmitter<Node>(entry.getEmitter(), current.second.second) )
								EmitterType::template emitError<Node>(entry.getEmitter(), current.second.second);
						}
						if (entry.getClientSocketData()->isErrorEventHandler())
							entry.getClientSocketData()->handleErrorEvent(entry.getClientSocket(), current.second.second);
					}
					if (entry.isUsed())
						entry.getClientSocket()->emitClose(err);
					if constexpr ( !std::is_same<EmitterType, void>::value )
					{
						if ( EmitterType::template isCloseEmitter<Node>(entry.getEmitter(), err) )
							EmitterType::template emitClose<Node>(entry.getEmitter(), err);
					}
					if (entry.getClientSocketData()->isCloseEventHandler())
						entry.getClientSocketData()->handleCloseEvent(entry.getClientSocket(), err);
					if (entry.isUsed())
						entry.getClientSocketData()->state = net::SocketBase::DataForCommandProcessing::Closed;
#ifdef USE_TEMP_PERF_CTRS
extern thread_local size_t sessionCreationtime;
extern uint64_t infraGetCurrentTime();
size_t now = infraGetCurrentTime();
					entry.getClientSocket()->onFinalCleanup();
sessionCreationtime += infraGetCurrentTime() - now;
#else
					entry.getClientSocket()->onFinalCleanup();
#endif // USE_TEMP_PERF_CTRS
				}
				entry = NetSocketEntry(current.first); 
			}
		}
		pendingCloseEvents.clear();
	}

	template<class Node>
	void infraProcessSockAcceptedEvents()
	{
		size_t inisz = pendingAcceptedEvents.size();
		for ( size_t i=0; i<inisz; ++i )
		{
			size_t idx = pendingAcceptedEvents[i];
			if (idx < ioSockets.size())
			{
				auto& entry = ioSockets.at(idx);
//				if (entry.isValid())
				if (entry.isUsed())
				{
					auto hr = entry.getClientSocketData()->ahd_accepted;
					if ( hr )
					{
						entry.getClientSocketData()->ahd_accepted = nullptr;
						hr();
					}
					else // TODO: make sure we never have both cases in the same time
					{
						entry.getClientSocket()->emitAccepted();
//						EmitterType::emitAccepted(entry.getEmitter());
						if constexpr ( !std::is_same<EmitterType, void>::value )
						{
							if ( EmitterType::template isAcceptedEmitter<Node>(entry.getEmitter()) )
								EmitterType::template emitAccepted<Node>(entry.getEmitter());
						}
						if (entry.getClientSocketData()->isAcceptedEventHandler())
							entry.getClientSocketData()->handleAcceptedEvent(entry.getClientSocket());
					}
					//entry.setAssociated();
					//ioSockets.setAssociated( idx );
				}
			}
		}
		if ( inisz == pendingAcceptedEvents.size() )
			pendingAcceptedEvents.clear();
		else
			pendingAcceptedEvents.erase( pendingAcceptedEvents.begin(), pendingAcceptedEvents.begin() + inisz );
	}

	template<class Node>
	void infraCheckPollFdSet(NetSocketEntry& current, short revents)
	{
		if ((revents & (POLLERR | POLLNVAL)) != 0) // check errors first
		{
//!!//			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("POLLERR event at {}", current.getClientSocketData()->osSocket);
			internal_usage_only::internal_getsockopt_so_error(current.getClientSocketData()->osSocket);
			//errorCloseSocket(current, storeError(Error()));
			Error e;
			errorCloseSocket(current, e);
		}
		else {
			/*
				on Windows when the other appEnd sends a FIN,
				POLLHUP goes up, if we still have data pending to read
				both POLLHUP and POLLIN are up. POLLHUP continues to be up
				for the socket as long as the socket lives.
				on Linux, when remote appEnd sends a FIN, we get a zero size read
				after all pending data is read.
			*/

			if ((revents & POLLIN) != 0)
			{
				if (!current.getClientSocketData()->paused)
				{
					//nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("POLLIN event at {}", begin[i].fd);
					infraProcessReadEvent<Node>(current/*, evs*/);
				}
			}
			else if ((revents & POLLHUP) != 0)
			{
//!!//				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("POLLHUP event at {}", current.getClientSocketData()->osSocket);
				infraProcessRemoteEnded<Node>(current/*, evs*/);
			}
				
			if ((revents & POLLOUT) != 0)
			{
//!!//				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("POLLOUT event at {}", current.getClientSocketData()->osSocket);
				infraProcessWriteEvent<Node>(current/*, evs*/);
			}
		}
		//else if (revents != 0)
		//{
		//	nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("Unexpected event at {}, value {:x}", begin[i].fd, revents);
		//	internal_getsockopt_so_error(entry.osSocket);
		//	errorCloseSocket(entry);
		//}
	}

private:
	template<class Node>
	void infraProcessReadEvent(NetSocketEntry& entry)
	{
		auto hr = entry.getClientSocketData()->ahd_read.h;
		if ( hr )
		{
			entry.getClientSocketData()->ahd_read.h = nullptr;
			size_t target_sz = entry.getClientSocketData()->ahd_read.min_bytes;
			bool read_ok = OSLayer::infraGetPacketBytes2(entry.getClientSocketData()->readBuffer, entry.getClientSocketData()->osSocket, target_sz);
			if ( !read_ok )
			{
				nodecpp::setException(hr, std::exception()); // TODO: switch to our exceptions ASAP!
				hr();
			}
			else if ( entry.getClientSocketData()->readBuffer.used_size() >= entry.getClientSocketData()->ahd_read.min_bytes )
			{
				hr();
			}
		}
		else
		{
			recvBuffer.clear();
			bool res = OSLayer::infraGetPacketBytes(recvBuffer, entry.getClientSocketData()->osSocket);
			if (res)
			{
				if (recvBuffer.size() != 0)
				{
					entry.getClientSocket()->emitData( recvBuffer);
					if constexpr ( !std::is_same<EmitterType, void>::value )
					{
						if ( EmitterType::template isDataEmitter<Node>(entry.getEmitter(), recvBuffer) )
							EmitterType::template emitData<Node>(entry.getEmitter(), recvBuffer);
					}
					if (entry.getClientSocketData()->isDataEventHandler())
						entry.getClientSocketData()->handleDataEvent(entry.getClientSocket(), recvBuffer);
					
					NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, recvBuffer.capacity() == recvBufferCapacity );
				}
				else //if (!entry.remoteEnded)
				{
					infraProcessRemoteEnded<Node>(entry);
				}
			}
			else
			{
				internal_usage_only::internal_getsockopt_so_error(entry.getClientSocketData()->osSocket);
				Error e;
				errorCloseSocket(entry, e);
			}
		}
	}

	template<class Node>
	void infraProcessRemoteEnded(NetSocketEntry& entry)
	{
		if (!entry.getClientSocketData()->remoteEnded)
		{
			entry.getClientSocketData()->remoteEnded = true;
			ioSockets.unsetPollin(entry.index); // if(!remoteEnded && !paused) events |= POLLIN;

			entry.getClientSocket()->emitEnd();
			if constexpr ( !std::is_same<EmitterType, void>::value )
			{
				if ( EmitterType::template isEndEmitter<Node>(entry.getEmitter()) )
					EmitterType::template emitEnd<Node>(entry.getEmitter());
			}
			if (entry.getClientSocketData()->isEndEventHandler())
				entry.getClientSocketData()->handleEndEvent(entry.getClientSocket());

			if (entry.getClientSocketData()->state == net::SocketBase::DataForCommandProcessing::LocalEnded)
			{
				//pendingCloseEvents.emplace_back(entry.index, false);
				closeSocket(entry);
			}
			else if (!entry.getClientSocketData()->allowHalfOpen && entry.getClientSocketData()->state != net::SocketBase::DataForCommandProcessing::LocalEnding)
			{
				if (!entry.getClientSocketData()->writeBuffer.empty())
				{
					entry.getClientSocketData()->state = net::SocketBase::DataForCommandProcessing::LocalEnding;
				}
				else
				{
//!!//				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("infraProcessRemoteEnded() leads to internal_shutdown_send()...");
					internal_usage_only::internal_shutdown_send(entry.getClientSocketData()->osSocket);
					entry.getClientSocketData()->state = net::SocketBase::DataForCommandProcessing::LocalEnded;
					closeSocket(entry);
				}
			}
		}
		else
		{
			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("Unexpected end on socket {}, already ended", entry.getClientSocketData()->osSocket);
		}

	}

	template<class Node>
	void infraProcessWriteEvent(NetSocketEntry& current)
	{
		NetSocketManagerBase::ShouldEmit status = this->_infraProcessWriteEvent(*current.getClientSocketData());
		switch ( status )
		{
			case NetSocketManagerBase::ShouldEmit::EmitConnect:
			{
				auto hr = current.getClientSocketData()->ahd_connect;
				if ( hr )
				{
					current.getClientSocketData()->ahd_connect = nullptr;
					hr();
				}
				else
				{
					current.getClientSocket()->emitConnect();
					if constexpr ( !std::is_same<EmitterType, void>::value )
					{
						if ( EmitterType::template isConnectEmitter<Node>(current.getEmitter()) )
							EmitterType::template emitConnect<Node>(current.getEmitter());
					}
					if (current.getClientSocketData()->isConnectEventHandler())
						current.getClientSocketData()->handleConnectEvent(current.getClientSocket());
				}
				break;
			}
			case NetSocketManagerBase::ShouldEmit::EmitDrain:
			{
				auto hr = current.getClientSocketData()->ahd_drain;
				if ( hr )
				{
					current.getClientSocketData()->ahd_drain = nullptr;
					hr();
				}
				else // TODO: make sure we never have both cases in the same time
				{
					current.getClientSocket()->emitDrain();
					if constexpr ( !std::is_same<EmitterType, void>::value )
					{
						if ( EmitterType::template isDrainEmitter<Node>(current.getEmitter()) )
							EmitterType::template emitDrain<Node>(current.getEmitter());
					}
					if (current.getClientSocketData()->isDrainEventHandler())
						current.getClientSocketData()->handleDrainEvent(current.getClientSocket());
				}
				break;
			}
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical,status == NetSocketManagerBase::ShouldEmit::EmitNone, "unexpected value {}", (size_t)status);
		}
	}
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////

class NetServerManagerBase
{
	friend class OSLayer;
	friend class Cluster;
protected:
	//mb: ioSockets[0] is always reserved and invalid.
	NetSockets& ioSockets; // TODO: improve
	std::vector<std::pair<size_t, bool>> pendingCloseEvents;
	PendingEvQueue pendingEvents;
	std::vector<Error> errorStore;
	std::vector<size_t> pendingListenEvents;

#ifdef NODECPP_ENABLE_CLUSTERING
	struct AcceptedSocketData
	{
		size_t idx;
		SOCKET socket;
		Ip4 remoteIp;
		Port remotePort;
	};
	std::vector<AcceptedSocketData> acceptedSockets;
	std::vector<size_t> receivedListeningEvs;
#endif // NODECPP_ENABLE_CLUSTERING

	std::string family = "IPv4";

public:
	int typeIndexOfServerO = -1;
	int typeIndexOfServerL = -1;

public:
	static constexpr size_t MAX_SOCKETS = 100; //arbitrary limit
	NetServerManagerBase(NetSockets& ioSockets_ ) : ioSockets( ioSockets_) {}

	template<class DataForCommandProcessing>
	void appClose(DataForCommandProcessing& serverData) {
		size_t id = serverData.index;
#ifndef NODECPP_ENABLE_CLUSTERING
		auto& entry = appGetEntry(id);
		if (!entry.isUsed())
		{
			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("Unexpected id {} on NetServerManager::close", id);
			return;
		}

		internal_usage_only::internal_close(serverData.osSocket);
		ioSockets.setSocketClosed( entry.index );

		//pendingCloseEvents.emplace_back(entry.index, false); note: it will be finally closed only after all accepted connections are ended
#else
		auto& entry = getCluster().isMaster() ? appGetEntry(id) : ioSockets.slaveServerAt(id);
		if (!entry.isUsed())
		{
			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("Unexpected id {} on NetServerManager::close", id);
			return;
		}
		if ( getCluster().isMaster() )
		{
			internal_usage_only::internal_close(serverData.osSocket);
			ioSockets.setSocketClosed( entry.index );
			//pendingCloseEvents.emplace_back(entry.index, false); note: it will be finally closed only after all accepted connections are ended
		}
		else
		{
			ioSockets.setSlaveSocketClosed(id);
		}
#endif // NODECPP_ENABLE_CLUSTERING
	}
	void appReportAllAceptedConnectionsEnded(net::ServerBase::DataForCommandProcessing& serverData) {
		size_t id = serverData.index;
		auto& entry = appGetEntry(id);
		if (!entry.isUsed())
		{
			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("Unexpected id {} on NetServerManager::close", id);
			return;
		}
		pendingCloseEvents.emplace_back(id, false);
	}
	void appAddServer(/*NodeBase* node, */nodecpp::safememory::soft_ptr<net::ServerBase> ptr, int typeId) { //TODO:CLUSTERING alt impl
#ifndef NODECPP_ENABLE_CLUSTERING
		SocketRiia s(internal_usage_only::internal_make_tcp_socket());
		if (!s)
		{
			throw Error();
		}
		ptr->dataForCommandProcessing.osSocket = s.release();
		addServerEntry(/*node, */ptr, typeId);
#else
		if ( getCluster().isMaster() )
		{
			SocketRiia s(internal_usage_only::internal_make_tcp_socket());
			if (!s)
			{
				throw Error();
			}
			ptr->dataForCommandProcessing.osSocket = s.release();
			addServerEntry(/*node, */ptr, typeId);
		}
		else
			addSlaveServerEntry(/*node, */ptr, typeId);
#endif // NODECPP_ENABLE_CLUSTERING
	}
#ifdef NODECPP_ENABLE_CLUSTERING
	void appAddAgentServer(/*NodeBase* node, */nodecpp::safememory::soft_ptr<Cluster::AgentServer> ptr, int typeId) {
		SocketRiia s(internal_usage_only::internal_make_tcp_socket());
		if (!s)
		{
			throw Error();
		}
		ptr->dataForCommandProcessing.osSocket = s.release();
		addAgentServerEntry(/*node, */ptr, typeId);
	}
#endif // NODECPP_ENABLE_CLUSTERING
	template<class DataForCommandProcessing>
	void appListen(DataForCommandProcessing& dataForCommandProcessing, const char* ip, uint16_t port, int backlog) { //TODO:CLUSTERING alt impl
		Ip4 myIp = Ip4::parse(ip);
		Port myPort = Port::fromHost(port);
#ifdef NODECPP_ENABLE_CLUSTERING
		if ( getCluster().isWorker() )
		{
			dataForCommandProcessing.localAddress.ip = nodecpp::Ip4::parse( ip );
			dataForCommandProcessing.localAddress.port = port;
			dataForCommandProcessing.localAddress.family = family;
			getCluster().acceptRequestForListeningAtSlave( dataForCommandProcessing.index, myIp, port, family, backlog );
			return;
		}
#endif // NODECPP_ENABLE_CLUSTERING
		if (!internal_usage_only::internal_bind_socket(dataForCommandProcessing.osSocket, myIp, myPort)) {
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
		dataForCommandProcessing.localAddress.ip = nodecpp::Ip4::parse( ip );
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
#ifndef NODECPP_ENABLE_CLUSTERING
		auto& entry = appGetEntry(id);
#else
		if ( getCluster().isMaster() )
		{
			auto& entry = appGetEntry(id);
			entry.setUnused(); 
		}
		else
		{
			auto& entry = ioSockets.slaveServerAt(id);
			entry.setUnused(); 
		}
#endif
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

#ifdef NODECPP_ENABLE_CLUSTERING
	void addAcceptedSocket( size_t serverIdx, SOCKET socket, Ip4 remoteIp, Port remotePort ) {	
		NODECPP_ASSERT( nodecpp::module_id, nodecpp::assert::AssertLevel::critical, getCluster().isWorker() ); 
		acceptedSockets.push_back(AcceptedSocketData({serverIdx, socket, remoteIp, remotePort})); 
	}
	void addListeningServerEv( size_t serverIdx ) { 
		NODECPP_ASSERT( nodecpp::module_id, nodecpp::assert::AssertLevel::critical, getCluster().isWorker() ); 
		receivedListeningEvs.push_back(serverIdx);
	}
#endif // NODECPP_ENABLE_CLUSTERING

protected:
	void addServerEntry(/*NodeBase* node, */nodecpp::safememory::soft_ptr<net::ServerBase> ptr, int typeId);
#ifdef NODECPP_ENABLE_CLUSTERING
	void addSlaveServerEntry(/*NodeBase* node, */nodecpp::safememory::soft_ptr<net::ServerBase> ptr, int typeId);
	void addAgentServerEntry(/*NodeBase* node, */nodecpp::safememory::soft_ptr<Cluster::AgentServer> ptr, int typeId);
#endif // NODECPP_ENABLE_CLUSTERING
	NetSocketEntry& appGetEntry(size_t id) { return ioSockets.at(id); }
	const NetSocketEntry& appGetEntry(size_t id) const { return ioSockets.at(id); }
#ifdef NODECPP_ENABLE_CLUSTERING
	NetSocketEntry& appGetSlaveServerEntry(size_t id) { 
		NODECPP_ASSERT( nodecpp::module_id, nodecpp::assert::AssertLevel::critical, getCluster().isWorker() ); 
		return ioSockets.slaveServerAt(id);
	}
	const NetSocketEntry& appGetSlaveServerEntry(size_t id) const {
		NODECPP_ASSERT( nodecpp::module_id, nodecpp::assert::AssertLevel::critical, getCluster().isWorker() ); 
		return ioSockets.slaveServerAt(id);
	}
#endif // NODECPP_ENABLE_CLUSTERING
};

extern thread_local NetServerManagerBase* netServerManagerBase;


template<class EmitterType>
class NetServerManager : public NetServerManagerBase
{
	std::string family = "IPv4";

public:
	NetServerManager(NetSockets& ioSockets) : NetServerManagerBase(ioSockets) {}

	//TODO quick workaround until definitive life managment is in place
	Error& infraStoreError(Error err) {
		errorStore.push_back(std::move(err));
		return errorStore.back();
	}

	void infraClearStores() {
		errorStore.clear();
	}

	template<class Node>
	void infraGetCloseEvents(/*EvQueue& evs*/)
	{
		// if there is an issue with a socket, we may need to close it,
		// and push an event here to notify later.

		for (auto& current : pendingCloseEvents)
		{
			//first remove any pending event for this socket
			pendingEvents.remove(current.first);
			if (current.first < ioSockets.size())
			{
				auto& entry = ioSockets.at(current.first);
				entry.getServerSocket()->internalCleanupBeforeClosing();
//				if (entry.isValid())
				if (entry.isUsed())
				{
					NODECPP_ASSERT( nodecpp::module_id, nodecpp::assert::AssertLevel::critical, entry.getServerSocketData()->state == nodecpp::net::ServerBase::DataForCommandProcessing::State::BeingClosed ); 
//					if (entry.getServerSocketData()->osSocket != INVALID_SOCKET)
//						internal_usage_only::internal_close(entry.getServerSocketData()->osSocket);
//					NODECPP_ASSERT( nodecpp::module_id, nodecpp::assert::AssertLevel::critical, entry.getServerSocketData()->osSocket == INVALID_SOCKET ); 
					NODECPP_ASSERT( nodecpp::module_id, nodecpp::assert::AssertLevel::critical, ioSockets.socketsAt(current.first) == INVALID_SOCKET ); 
					//evs.add(&net::Server::emitClose, entry.getPtr(), current.second);
					auto hr = entry.getServerSocketData()->ahd_close;
					if ( hr != nullptr )
					{
						entry.getServerSocketData()->ahd_close = nullptr;
						if (current.second)
							nodecpp::setException(hr, std::exception()); // TODO: switch to our exceptions ASAP!
						hr();
					}
					else
					{
						//EmitterType::emitClose( entry.getEmitter(), current.second);
						entry.getServerSocket()->emitClose( current.second );
						if constexpr ( !std::is_same<EmitterType, void>::value )
						{
							if ( EmitterType::template isCloseEmitter<Node>(entry.getEmitter(), current.second) )
								EmitterType::template emitClose<Node>( entry.getEmitter(), current.second);
						}
						if (entry.getServerSocketData()->isCloseEventHandler())
							entry.getServerSocketData()->handleCloseEvent(entry.getServerSocket(), current.second);
						// TODO: what should we do with this event, if, at present, nobody is willing to process it?
					}
				}
				entry = NetSocketEntry(current.first);
			}
		}
		pendingCloseEvents.clear();
	}

	template<class Node>
	void infraEmitListeningEvents()
	{
		while ( pendingListenEvents.size() )
		{
			std::vector<size_t> currentPendingListenEvents = std::move( pendingListenEvents );
			for (auto& current : currentPendingListenEvents)
			{
				if (ioSockets.isValidId(current))
				{
					auto& entry = ioSockets.at(current);
					if (entry.isUsed())
					{
#ifdef NODECPP_ENABLE_CLUSTERING
						if ( entry.getObjectType() == OpaqueEmitter::ObjectType::AgentServer )
						{
							entry.getAgentServerData()->state = nodecpp::Cluster::AgentServer::DataForCommandProcessing::State::Listening;
							NODECPP_ASSERT( nodecpp::module_id, nodecpp::assert::AssertLevel::critical, getCluster().isMaster() ); 
							entry.getAgentServer()->onListening();
						}
						else
#endif // NODECPP_ENABLE_CLUSTERING
						{
							entry.getServerSocketData()->state = nodecpp::net::ServerBase::DataForCommandProcessing::State::Listening;
							auto hr = entry.getServerSocketData()->ahd_listen;
							if ( hr != nullptr )
							{
								entry.getServerSocketData()->ahd_listen = nullptr;
								hr();
							}
							else
							{
								//EmitterType::emitListening(entry.getEmitter(), current, entry.getServerSocketData()->localAddress);
								entry.getServerSocket()->emitListening(current, entry.getServerSocketData()->localAddress);
								if constexpr ( !std::is_same<EmitterType, void>::value )
								{
									if ( EmitterType::template isListeningEmitter<Node>(entry.getEmitter(), current, entry.getServerSocketData()->localAddress) )
										EmitterType::template emitListening<Node>(entry.getEmitter(), current, entry.getServerSocketData()->localAddress);
								}
								if (entry.getServerSocketData()->isListenEventHandler() )
									entry.getServerSocketData()->handleListenEvent(entry.getServerSocket(), current, entry.getServerSocketData()->localAddress);
								// TODO: what should we do with this event, if, at present, nobody is willing to process it?
							}
						}
					}
				}
			}
		}
	}

	template<class Node>
	void infraCheckPollFdSet(NetSocketEntry& current, short revents)
	{
		if ((revents & (POLLERR | POLLNVAL)) != 0) // check errors first
		{
//!!//			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("POLLERR event at {}", current.getServerSocketData()->osSocket);
			internal_usage_only::internal_getsockopt_so_error(current.getServerSocketData()->osSocket);
			infraMakeErrorEventAndClose<Node>(current/*, evs*/);
		}
		else if ((revents & POLLIN) != 0)
		{
//!!//			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("POLLIN event at {}", current.getServerSocketData()->osSocket);
			infraProcessAcceptEvent<Node>(current/*, evs*/);
		}
		else if (revents != 0)
		{
			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("Unexpected event at {}, value {:x}", current.getServerSocketData()->osSocket, revents);
			internal_usage_only::internal_getsockopt_so_error(current.getServerSocketData()->osSocket);
			infraMakeErrorEventAndClose<Node>(current/*, evs*/);
		}
	}

#ifdef NODECPP_ENABLE_CLUSTERING
	template<class Node>
	void infraEmitAcceptedSocketEventsReceivedfromMaster()
	{
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::pedantic, getCluster().isWorker() );
		for ( auto& info : acceptedSockets )
		{
			auto& entry = ioSockets.slaveServerAt( info.idx );
			OpaqueSocketData osd = NetSocketManagerBase::createOpaqueSocketData( info.socket );
			consumeAcceptedSocket<Node>(entry, osd, info.remoteIp, info.remotePort);
		}
		acceptedSockets.clear();
	}
#endif // NODECPP_ENABLE_CLUSTERING

private:
	template<class Node>
	void infraProcessAcceptEvent(NetSocketEntry& entry) //TODO:CLUSTERING alt impl
	{
		OpaqueSocketData osd( false );

#ifdef NODECPP_ENABLE_CLUSTERING
		OpaqueEmitter::ObjectType type = entry.getObjectType();
		Ip4 remoteIp;
		Port remotePort;
		if ( type == OpaqueEmitter::ObjectType::AgentServer ) // Clustering
		{
			if ( !netSocketManagerBase->getAcceptedSockData(entry.getAgentServerData()->osSocket, osd, remoteIp, remotePort) )
				return;
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::pedantic, getCluster().isMaster() );
			SOCKET osSocket = netSocketManagerBase->extractSocket( osd ).release();
			entry.getAgentServer()->onConnection( osSocket, remoteIp, remotePort );
			return;
		}
		else
		{
			if ( !netSocketManagerBase->getAcceptedSockData(entry.getServerSocketData()->osSocket, osd, remoteIp, remotePort) )
				return;
			consumeAcceptedSocket<Node>(entry, osd, remoteIp, remotePort);
		}
#else
		if ( !netSocketManagerBase->getAcceptedSockData(entry.getServerSocketData()->osSocket, osd, remoteIp, remotePort) )
			return;
		consumeAcceptedSocket<Node>(entry, osd, remoteIp, remotePort);
#endif // NODECPP_ENABLE_CLUSTERING
	}


	template<class Node>
	void consumeAcceptedSocket(NetSocketEntry& entry, OpaqueSocketData& osd, Ip4 remoteIp, Port remotePort)
	{
//		soft_ptr<net::SocketBase> ptr = EmitterType::makeSocket(entry.getEmitter(), osd);
#ifdef USE_TEMP_PERF_CTRS
extern thread_local int sessionCnt;
extern thread_local size_t sessionCreationtime;
extern uint64_t infraGetCurrentTime();
++sessionCnt;
size_t now = infraGetCurrentTime();
		auto ptr = EmitterType::makeSocket(entry.getEmitter(), osd);
sessionCreationtime += infraGetCurrentTime() - now;
#else
		auto ptr = EmitterType::makeSocket(entry.getEmitter(), osd);
#endif // USE_TEMP_PERF_CTRS
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, netSocketManagerBase != nullptr );
		netSocketManagerBase->infraAddAccepted(ptr);
		ptr->dataForCommandProcessing._remote.ip = remoteIp;
		ptr->dataForCommandProcessing._remote.port = remotePort.getHost();

		auto hr = entry.getServerSocketData()->ahd_connection.h;
		if ( hr )
		{
			entry.getServerSocketData()->ahd_connection.sock = ptr;
			entry.getServerSocketData()->ahd_connection.h = nullptr;
			hr();
		}
		else
		{
			//EmitterType::emitConnection(entry.getEmitter(), ptr); 
			entry.getServerSocket()->emitConnection(ptr);
			if constexpr ( !std::is_same<EmitterType, void>::value )
			{
				if ( EmitterType::template isConnectionEmitter<Node>(entry.getEmitter(), ptr) )
					EmitterType::template emitConnection<Node>(entry.getEmitter(), ptr); 
			}
			if (entry.getServerSocketData()->isConnectionEventHandler())
				entry.getServerSocketData()->handleConnectionEvent(entry.getServerSocket(), ptr);
			// TODO: what should we do with this event, if, at present, nobody is willing to process it?
		}

		return;
	}

	template<class Node>
	void infraMakeErrorEventAndClose(NetSocketEntry& entry)
	{
		Error e;
#ifdef NODECPP_ENABLE_CLUSTERING
		OpaqueEmitter::ObjectType type = entry.getObjectType();
		if ( type == OpaqueEmitter::ObjectType::AgentServer )
		{
			// TODO: special Clustering treatment
			return;
		}
#endif // NODECPP_ENABLE_CLUSTERING
//		evs.add(&net::Server::emitError, entry.getPtr(), std::ref(infraStoreError(Error())));
		entry.getServerSocket()->emitError( e );
		if constexpr ( !std::is_same<EmitterType, void>::value )
		{
			if ( EmitterType::template isErrorEmitter<Node>(entry.getEmitter(), e) )
				EmitterType::template emitError<Node>( entry.getEmitter(), e );
		}
		if (entry.getServerSocketData()->isErrorEventHandler())
			entry.getServerSocketData()->handleErrorEvent(entry.getServerSocket(), e);
		// TODO: what should we do with this event, if, at present, nobody is willing to process it?
		pendingCloseEvents.emplace_back(entry.index, true);
	}
};

template<>
class NetServerManager<void>
{
public:
	NetServerManager(NetSockets&) {}
};


#endif // TCP_SOCKET_H


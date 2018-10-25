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
*     * Neither the name of the <organization> nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
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
//#include "../../include/nodecpp/server_common.h"

using namespace nodecpp;

class NetSocketEntry {
	// TODO: revise everything around being 'refed'
	enum State { Unused, SockIssued, SockAssociated, SockClosed }; // TODO: revise!
	State state = State::Unused;

public:
	size_t index;
	bool refed = false;
	OpaqueEmitter emitter;

	NetSocketEntry(size_t index) : index(index), state(State::Unused) {}
	NetSocketEntry(size_t index, NodeBase* node, net::SocketTBase* ptr, int type) : index(index), state(State::SockIssued), emitter(OpaqueEmitter::ObjectType::ClientSocket, node, ptr, type) {ptr->dataForCommandProcessing.index = index;NODECPP_ASSERT( ptr->dataForCommandProcessing.osSocket > 0 );}
	NetSocketEntry(size_t index, NodeBase* node, net::ServerTBase* ptr, int type) : index(index), state(State::SockIssued), emitter(OpaqueEmitter::ObjectType::ServerSocket, node, ptr, type) {ptr->dataForCommandProcessing.index = index;NODECPP_ASSERT( ptr->dataForCommandProcessing.osSocket > 0 );}
	
	NetSocketEntry(const NetSocketEntry& other) = delete;
	NetSocketEntry& operator=(const NetSocketEntry& other) = delete;

	NetSocketEntry(NetSocketEntry&& other) = default;
	NetSocketEntry& operator=(NetSocketEntry&& other) = default;

	bool isUsed() const { NODECPP_ASSERT( (state == State::Unused) || (state != State::Unused && emitter.isValid()) ); return state != State::Unused; }
	bool isAssociated() const { NODECPP_ASSERT( (state == State::Unused) || (state != State::Unused && emitter.isValid()) ); return state == State::SockAssociated; }
	//void setSockIssued(SOCKET s) {NODECPP_ASSERT( state == State::Unused && emitter.ptr != nullptr ); state = State::SockIssued;}
	void setAssociated() {NODECPP_ASSERT( state == State::SockIssued && emitter.isValid() ); state = State::SockAssociated;}
	void setSocketClosed() {NODECPP_ASSERT( state != State::Unused ); state = State::SockClosed;}
	void setUnused() {state = State::Unused; }

	const OpaqueEmitter& getEmitter() const { return emitter; }
	net::SocketTBase::DataForCommandProcessing* getClientSocketData() const { NODECPP_ASSERT(emitter.isValid()); NODECPP_ASSERT( emitter.objectType == OpaqueEmitter::ObjectType::ClientSocket); return emitter.getClientSocketPtr() ? &( emitter.getClientSocketPtr()->dataForCommandProcessing ) : nullptr; }
	net::ServerTBase::DataForCommandProcessing* getServerSocketData() const { NODECPP_ASSERT(emitter.isValid()); NODECPP_ASSERT( emitter.objectType == OpaqueEmitter::ObjectType::ServerSocket); return emitter.getServerSocketPtr() ? &( emitter.getServerSocketPtr()->dataForCommandProcessing ) : nullptr; }
};

class NetSockets
{
	std::vector<NetSocketEntry> ourSide;
	std::vector<pollfd> osSide;
	size_t associatedCount = 0;
	//mb: xxxSide[0] is always reserved and invalid.
public:
	NetSockets() {ourSide.reserve(1000); osSide.reserve(1000); ourSide.emplace_back(0); osSide.emplace_back();}

	NetSocketEntry& at(size_t idx) { return ourSide.at(idx);}
	const NetSocketEntry& at(size_t idx) const { return ourSide.at(idx);}
	size_t size() const {return ourSide.size() - 1; }
	bool isValidId( size_t idx ) { return idx && idx < ourSide.size(); };

	template<class SocketType>
	size_t addEntry(NodeBase* node, SocketType* ptr, int typeId) {
		NODECPP_ASSERT( ourSide.size() == osSide.size() );
		NODECPP_ASSERT( ptr->dataForCommandProcessing.osSocket > 0 );
		for (size_t i = 1; i != ourSide.size(); ++i) // skip ourSide[0]
		{
			if (!ourSide[i].isUsed())
			{
				NetSocketEntry entry(i, node, ptr, typeId);
				ourSide[i] = std::move(entry);
				osSide[i].fd = (SOCKET)(-((int64_t)(ptr->dataForCommandProcessing.osSocket)));
				osSide[i].events = 0;
				osSide[i].revents = 0;
				return i;
			}
		}

		size_t ix = ourSide.size();
		ourSide.emplace_back(ix, node, ptr, typeId);
		pollfd p;
		p.fd = (SOCKET)(-((int64_t)(ptr->dataForCommandProcessing.osSocket)));
		p.events = 0;
		p.revents = 0;
		osSide.push_back( p );
		return ix;
	}
	//void setSockIssued( size_t idx ) {NODECPP_ASSERT( idx && idx <= ourSide.size() );}
	void setAssociated( size_t idx/*, pollfd p*/ ) {
		NODECPP_ASSERT( idx && idx <= ourSide.size() );
		ourSide[idx].setAssociated();
		osSide[idx].fd = (SOCKET)(-((int64_t)(osSide[idx].fd)));
		NODECPP_ASSERT( osSide[idx].events == 0 );
		NODECPP_ASSERT( osSide[idx].revents == 0 );
		NODECPP_ASSERT( osSide[idx].fd > 0 );
		++associatedCount;
		/*osSide[idx].events = p.events;
		osSide[idx].revents = p.revents;
		switch ( ourSide[idx].emitter.objectType ) {
			case OpaqueEmitter::ObjectType::ClientSocket:
				break;
			case OpaqueEmitter::ObjectType::ServerSocket:
				break;
			default:
				NODECPP_ASSERT( false );
		}*/

	}
	//short getEvents( size_t idx ) {NODECPP_ASSERT( idx && idx <= ourSide.size() ); return osSide[idx].events; }
	void setPollout( size_t idx ) {NODECPP_ASSERT( idx && idx <= ourSide.size() ); osSide[idx].events |= POLLOUT; }
	void unsetPollout( size_t idx ) {NODECPP_ASSERT( idx && idx <= ourSide.size() ); osSide[idx].events &= ~POLLOUT; }
	void setPollin( size_t idx ) {NODECPP_ASSERT( idx && idx <= ourSide.size() ); osSide[idx].events |= POLLIN; }
	void unsetPollin( size_t idx ) {NODECPP_ASSERT( idx && idx <= ourSide.size() ); osSide[idx].events &= ~POLLIN; }
	void setRefed( size_t idx, bool refed ) {NODECPP_ASSERT( idx && idx <= ourSide.size() ); ourSide[idx].refed = refed; }
	void setUnused( size_t idx ) {
		NODECPP_ASSERT( idx && idx <= ourSide.size() );
		if ( osSide[idx].fd != INVALID_SOCKET )
		{
			osSide[idx].fd = INVALID_SOCKET; 
			--associatedCount;
		}
		ourSide[idx].setUnused();
	}
	void setSocketClosed( size_t idx ) {
		NODECPP_ASSERT( idx && idx <= ourSide.size() ); 
		osSide[idx].fd = INVALID_SOCKET; 
		ourSide[idx].setSocketClosed();
		--associatedCount;
	}
	std::pair<pollfd*, size_t> getPollfd() { 
		return osSide.size() > 1 ? ( associatedCount > 0 ? std::make_pair( &(osSide[1]), osSide.size() - 1 ) : std::make_pair( nullptr, 0 ) ) : std::make_pair( nullptr, 0 ); 
	}
};

class NetSocketManagerBase : protected OSLayer
{
	friend class OSLayer;
	std::vector<Buffer> bufferStore; // TODO: improve

public:
	int typeIndexOfSocketO = -1;
	int typeIndexOfSocketL = -1;

protected:
	NetSockets& ioSockets; // TODO: improve
	std::vector<std::pair<size_t, std::pair<bool, Error>>> pendingCloseEvents;
	std::vector<size_t> pendingAcceptedEvents;

public:
	static constexpr size_t MAX_SOCKETS = 100; //arbitrary limit

public:
	NetSocketManagerBase(NetSockets& ioSockets_) : ioSockets( ioSockets_) {}

	//TODO quick workaround until definitive life managment is in place
	Buffer& infraStoreBuffer(Buffer buff) {
		bufferStore.push_back(std::move(buff));
		return bufferStore.back();
	}

	void infraClearStores() {
		bufferStore.clear();
	}

public:
//	size_t infraGetPollFdSetSize() const { return ioSockets.size(); }

	void infraAddAccepted(net::SocketTBase* ptr)
	{
		size_t id = ptr->dataForCommandProcessing.index;
		pendingAcceptedEvents.push_back(id);
		ptr->dataForCommandProcessing.state = net::SocketBase::DataForCommandProcessing::Connected;
		NODECPP_ASSERT( ptr->dataForCommandProcessing.writeBuffer.size() == 0 );
		ioSockets.unsetPollout(id);
		ptr->dataForCommandProcessing.refed = true;
		auto& entry = appGetEntry(id);
		entry.refed = true; 
		ioSockets.setAssociated(id);
		NODECPP_ASSERT(ptr->dataForCommandProcessing.remoteEnded == false);
		NODECPP_ASSERT(ptr->dataForCommandProcessing.paused == false);
		NODECPP_ASSERT(ptr->dataForCommandProcessing.state != net::SocketBase::DataForCommandProcessing::Connecting);
		NODECPP_ASSERT(ptr->dataForCommandProcessing.writeBuffer.empty());
		ioSockets.setPollin(id);
	}

#ifdef USING_T_SOCKETS
	size_t appAcquireSocket(NodeBase* node, net::SocketTBase* ptr, int typeId)
	{
		SocketRiia s( OSLayer::appAcquireSocket() );
		return registerAndAssignSocket(node, ptr, typeId, s);
	}

	size_t appAssignSocket(NodeBase* node, net::SocketTBase* ptr, int typeId, OpaqueSocketData& sdata)
	{
		SocketRiia s( sdata.s.release() );
		return registerAndAssignSocket(node, ptr, typeId, s);
	}

private:
	size_t registerAndAssignSocket(NodeBase* node, net::SocketTBase* ptr, int typeId, SocketRiia& s)
	{
		/*size_t id = addEntry(node, ptr, typeId);
		if (id == 0)
		{
			NODECPP_TRACE0("Failed to add entry on NetSocketManager::connect");
			throw Error();
		}

		auto& entry = appGetEntry(id);
		//ptr->dataForCommandProcessing.index = id;
		ptr->dataForCommandProcessing.osSocket = s.release();
		entry.setSockIssued();
		NODECPP_ASSERT(ptr->dataForCommandProcessing.state == net::SocketBase::DataForCommandProcessing::Uninitialized);*/

		NODECPP_ASSERT(ptr->dataForCommandProcessing.state == net::SocketBase::DataForCommandProcessing::Uninitialized);
		ptr->dataForCommandProcessing.osSocket = s.release();
		size_t id = ioSockets.addEntry(node, ptr, typeId);
		NODECPP_ASSERT(id != 0);
		return id;
	}

public:
	void appConnectSocket(net::SocketTBase* sockPtr, const char* ip, uint16_t port) // TODO: think about template with type checking inside
	{
		// TODO: check sockPtr validity
		NODECPP_ASSERT(sockPtr->dataForCommandProcessing.osSocket != INVALID_SOCKET);
		NODECPP_ASSERT(sockPtr->dataForCommandProcessing.state == net::SocketBase::DataForCommandProcessing::Uninitialized);
		OSLayer::appConnectSocket(sockPtr->dataForCommandProcessing.osSocket, ip, port );
		sockPtr->dataForCommandProcessing.state = net::SocketBase::DataForCommandProcessing::Connecting;
		sockPtr->dataForCommandProcessing.refed = true;
		/*auto& entry = appGetEntry(sockPtr->dataForCommandProcessing.index);
		entry.setAssociated();
		entry.refed = true; 
	//	entry.connecting = true;*/

		// TODOY: revise!
		/* p;
		p.fd = sockPtr->dataForCommandProcessing.osSocket;
		p.events = 0;
		if(!sockPtr->dataForCommandProcessing.remoteEnded && !sockPtr->dataForCommandProcessing.paused)
			p.events |= POLLIN;
		if (sockPtr->dataForCommandProcessing.state == net::SocketBase::DataForCommandProcessing::Connecting || !sockPtr->dataForCommandProcessing.writeBuffer.empty())
			p.events |= POLLOUT;*/
		ioSockets.setRefed( sockPtr->dataForCommandProcessing.index, true );
		ioSockets.setAssociated(sockPtr->dataForCommandProcessing.index/*, p*/ );
		ioSockets.setPollin(sockPtr->dataForCommandProcessing.index);
		NODECPP_ASSERT(sockPtr->dataForCommandProcessing.state == net::SocketBase::DataForCommandProcessing::Connecting);
		ioSockets.setPollout(sockPtr->dataForCommandProcessing.index);
	}
	bool appWrite(net::SocketBase::DataForCommandProcessing& sockData, const uint8_t* data, uint32_t size);
	bool getAcceptedSockData(SOCKET s, OpaqueSocketData& osd )
	{
		Ip4 remoteIp;
		Port remotePort;

		SocketRiia newSock(internal_usage_only::internal_tcp_accept(remoteIp, remotePort, s));
		if (newSock.get() == INVALID_SOCKET)
			return false;
		osd.s = std::move(newSock);
		return true;
	}
#endif // USING_T_SOCKETS

#if 0
	bool infraSetPollFdSet(pollfd* begin, const pollfd* end) const
	{
		size_t sz = end - begin;
		assert(sz >= ioSockets.size());
		bool anyRefed = false;
	
		for (size_t i = 0; i != sz; ++i)
		{
//			if(i < ioSockets.size() && ioSockets[i].isValid())
			if(i < ioSockets.size() && ioSockets[i].isAssociated())
			{
				const auto& current = ioSockets[i];
				NODECPP_ASSERT(current.getClientSocketData()->osSocket != INVALID_SOCKET);

//				anyRefed = anyRefed || current.getClientSocketData()->refed;
				anyRefed = anyRefed || current.refed;

				begin[i].fd = current.getClientSocketData()->osSocket;
				begin[i].events = 0;

				if(!current.getClientSocketData()->remoteEnded && !current.getClientSocketData()->paused)
					begin[i].events |= POLLIN;
				if (current.getClientSocketData()->state == net::SocketBase::DataForCommandProcessing::Connecting || !current.getClientSocketData()->writeBuffer.empty())
					begin[i].events |= POLLOUT;
			}
			else
				begin[i].fd = INVALID_SOCKET;
		}
		return anyRefed;
	}
#endif // 0

protected:
	/*size_t addEntry(NodeBase* node, net::SocketTBase* ptr, int typeId) //app-infra neutral
	{
		for (size_t i = 1; i != ioSockets.size(); ++i) // skip ioSockets[0]
		{
//			if (!ioSockets[i].isValid())
			if (!ioSockets[i].isUsed())
			{
				NetSocketEntry entry(node, i, ptr, typeId);
				ioSockets[i] = std::move(entry);
				return i;
			}
		}

		if (ioSockets.size() >= MAX_SOCKETS)
		{
			return 0;
		}

		size_t ix = ioSockets.size();
		ioSockets.emplace_back(node, ix, ptr, typeId);
		return ix;
	}*/

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
		auto& entry = appGetEntry(id);
		//entry.getClientSocketData()->refed = true;
		entry.refed = true; 
	}
	void appUnref(size_t id) { 
		auto& entry = appGetEntry(id);
		//entry.getClientSocketData()->refed = false; 
		entry.refed = false; 
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

public:
	NetSocketManager(NetSockets& ioSockets) : NetSocketManagerBase(ioSockets) {}

	// to help with 'poll'
	void infraGetCloseEvent(/*EvQueue& evs*/)
	{
		// if there is an issue with a socket, we may need to appClose it,
		// and push an event here to notify autom later.

		for (auto& current : pendingCloseEvents)
		{
//			if (current.first < ioSockets.size())
			if (ioSockets.isValidId(current.first))
			{
//				auto& entry = ioSockets[current.first];
				auto& entry = ioSockets.at(current.first);
//				if (entry.isValid())
				if (entry.isUsed())
				{
					bool err = entry.getClientSocketData()->state == net::SocketBase::DataForCommandProcessing::ErrorClosing;
					if (entry.getClientSocketData()->osSocket != INVALID_SOCKET)
					{
						if(err) // if error closing, then discard all buffers
							internal_usage_only::internal_linger_zero_socket(entry.getClientSocketData()->osSocket);

						internal_usage_only::internal_close(entry.getClientSocketData()->osSocket);
						ioSockets.setSocketClosed( entry.index );
					}

#if 0 // old version (note that emitClose is before emiterror in both cases; whether it is OK or not, is a separate question)
					if (err) //if error closing, then first error event
					{
	//					evs.add(std::move(current.second));
//						std::function<void()> ev = std::bind(&net::SocketEmitter::emitError, entry.getEmitter(), current.second.second);
						std::function<void()> ev = std::bind(&EmitterType::emitError, entry.getEmitter(), current.second.second);
						evs.add(std::move(ev));
					}

	//				evs.add(&net::Socket::emitClose, entry.getPtr(), err);
	//				entry.getPtr()->emitClose(err);
					EmitterType::emitClose(entry.getEmitter(), err);
					entry.getClientSocketData()->state = net::SocketBase::DataForCommandProcessing::Closed;
#else // new version
					EmitterType::emitClose(entry.getEmitter(), err);
					entry.getClientSocketData()->state = net::SocketBase::DataForCommandProcessing::Closed;
//					if (err && entry.isValid()) //if error closing, then first error event
					if (err && entry.isUsed()) //if error closing, then first error event
						EmitterType::emitError(entry.getEmitter(), current.second.second);
#endif // 0
				}
				entry = NetSocketEntry(current.first); 
			}
		}
		pendingCloseEvents.clear();
	}
	void infraProcessSockAcceptedEvents()
	{
		for ( auto idx:pendingAcceptedEvents )
		{
			if (idx < ioSockets.size())
			{
				auto& entry = ioSockets.at(idx);
//				if (entry.isValid())
				if (entry.isUsed())
				{
					EmitterType::emitAccepted(entry.getEmitter());
					//entry.setAssociated();
					ioSockets.setAssociated( idx );
				}
			}
			pendingAcceptedEvents.clear();
		}
	}
#if 0 // old code
	void infraCheckPollFdSet(const pollfd* begin, const pollfd* end)
	{
		assert(end - begin >= static_cast<ptrdiff_t>(ioSockets.size()));
		for (size_t i = 0; i != ioSockets.size(); ++i)
		{
			auto& current = ioSockets.at(i);
			if (begin[i].fd != INVALID_SOCKET)
			{
				if ((begin[i].revents & (POLLERR | POLLNVAL)) != 0) // check errors first
				{
					NODECPP_TRACE("POLLERR event at {}", begin[i].fd);
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

					if ((begin[i].revents & POLLIN) != 0)
					{
						if (!current.getClientSocketData()->paused)
						{
							//NODECPP_TRACE("POLLIN event at {}", begin[i].fd);
							infraProcessReadEvent(current/*, evs*/);
						}
					}
					else if ((begin[i].revents & POLLHUP) != 0)
					{
						NODECPP_TRACE("POLLHUP event at {}", begin[i].fd);
						infraProcessRemoteEnded(current/*, evs*/);
					}
				
					if ((begin[i].revents & POLLOUT) != 0)
					{
						NODECPP_TRACE("POLLOUT event at {}", begin[i].fd);
						infraProcessWriteEvent(current/*, evs*/);
					}
				}
				//else if (begin[i].revents != 0)
				//{
				//	NODECPP_TRACE("Unexpected event at {}, value {:x}", begin[i].fd, begin[i].revents);
				//	internal_getsockopt_so_error(entry.osSocket);
				//	errorCloseSocket(entry);
				//}
			}
		}
	}
#else
	void infraCheckPollFdSet(NetSocketEntry& current, pollfd p)
	{
//		assert(end - begin >= static_cast<ptrdiff_t>(ioSockets.size()));
//		for (size_t i = 0; i != ioSockets.size(); ++i)
		{
//			NetSocketEntry& current = ioSockets.at(i);
//			if (begin[i].fd != INVALID_SOCKET)
			{
				if ((p.revents & (POLLERR | POLLNVAL)) != 0) // check errors first
				{
					NODECPP_TRACE("POLLERR event at {}", p.fd);
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

					if ((p.revents & POLLIN) != 0)
					{
						if (!current.getClientSocketData()->paused)
						{
							//NODECPP_TRACE("POLLIN event at {}", begin[i].fd);
							infraProcessReadEvent(current/*, evs*/);
						}
					}
					else if ((p.revents & POLLHUP) != 0)
					{
						NODECPP_TRACE("POLLHUP event at {}", p.fd);
						infraProcessRemoteEnded(current/*, evs*/);
					}
				
					if ((p.revents & POLLOUT) != 0)
					{
						NODECPP_TRACE("POLLOUT event at {}", p.fd);
						infraProcessWriteEvent(current/*, evs*/);
					}
				}
				//else if (revents != 0)
				//{
				//	NODECPP_TRACE("Unexpected event at {}, value {:x}", begin[i].fd, revents);
				//	internal_getsockopt_so_error(entry.osSocket);
				//	errorCloseSocket(entry);
				//}
			}
		}
	}
#endif // 0

private:
	void infraProcessReadEvent(NetSocketEntry& entry)
	{
		auto res = OSLayer::infraGetPacketBytes(entry.getClientSocketData()->recvBuffer, entry.getClientSocketData()->osSocket);
		if (res.first)
		{
			if (res.second.size() != 0)
			{
	//			entry.ptr->emitData(std::move(res.second));

	//			evs.add(&net::Socket::emitData, entry.getPtr(), std::ref(infraStoreBuffer(std::move(res.second))));
//				entry.getEmitter().emitData(std::ref(infraStoreBuffer(std::move(res.second))));
				EmitterType::emitData(entry.getEmitter(), std::ref(infraStoreBuffer(std::move(res.second))));
			}
			else //if (!entry.remoteEnded)
			{
				infraProcessRemoteEnded(entry/*, evs*/);
			}
		}
		else
		{
			internal_usage_only::internal_getsockopt_so_error(entry.getClientSocketData()->osSocket);
	//		return errorCloseSocket(entry, storeError(Error()));
			Error e;
			errorCloseSocket(entry, e);
		}
	}

	void infraProcessRemoteEnded(NetSocketEntry& entry)
	{
		if (!entry.getClientSocketData()->remoteEnded)
		{
			entry.getClientSocketData()->remoteEnded = true;
			ioSockets.unsetPollin(entry.index); // if(!remoteEnded && !paused) events |= POLLIN;
	//		evs.add(&net::Socket::emitEnd, entry.getPtr());
			EmitterType::emitEnd(entry.getEmitter());
			if (entry.getClientSocketData()->state == net::SocketBase::DataForCommandProcessing::LocalEnded)
			{
				//pendingCloseEvents.emplace_back(entry.index, false);
				closeSocket(entry);
			}
			else if (!entry.getClientSocketData()->allowHalfOpen && entry.getClientSocketData()->state != net::SocketBase::DataForCommandProcessing::LocalEnding)
			{
				if (!entry.getClientSocketData()->writeBuffer.empty())
				{
	//				entry.pendingLocalEnd = true;
					entry.getClientSocketData()->state = net::SocketBase::DataForCommandProcessing::LocalEnding;
				}
				else
				{
					internal_usage_only::internal_shutdown_send(entry.getClientSocketData()->osSocket);
					entry.getClientSocketData()->state = net::SocketBase::DataForCommandProcessing::LocalEnded;
	//				entry.localEnded = true;
				}
			}
		}
		else
		{
			NODECPP_TRACE("Unexpected end on socket {}, already ended", entry.getClientSocketData()->osSocket);
		}

	}

	void infraProcessWriteEvent(NetSocketEntry& current)
	{
		NetSocketManagerBase::ShouldEmit status = this->_infraProcessWriteEvent(*current.getClientSocketData());
		switch ( status )
		{
			case NetSocketManagerBase::ShouldEmit::EmitConnect:
				EmitterType::emitConnect(current.getEmitter());
				break;
			case NetSocketManagerBase::ShouldEmit::EmitDrain:
				EmitterType::emitDrain(current.getEmitter());
				break;
			default:
				NODECPP_ASSERT(status == NetSocketManagerBase::ShouldEmit::EmitNone, "unexpected value {}", (size_t)status);
		}
	}
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////

class NetServerManagerBase
{
	friend class OSLayer;
protected:
	//mb: ioSockets[0] is always reserved and invalid.
	NetSockets& ioSockets; // TODO: improve
	std::vector<std::pair<size_t, bool>> pendingCloseEvents;
	PendingEvQueue pendingEvents;
	std::vector<Error> errorStore;
	std::vector<size_t> pendingListenEvents;

	std::string family = "IPv4";

public:
	int typeIndexOfServerO = -1;
	int typeIndexOfServerL = -1;

public:
	static constexpr size_t MAX_SOCKETS = 100; //arbitrary limit
	NetServerManagerBase(NetSockets& ioSockets_ ) : ioSockets( ioSockets_) {}

	void appClose(size_t id);
	void appAddServer(NodeBase* node, net::ServerTBase* ptr, int typeId) {
		SocketRiia s(internal_usage_only::internal_make_tcp_socket());
		if (!s)
		{
			throw Error();
		}
		ptr->dataForCommandProcessing.osSocket = s.release();
		size_t id = addServerEntry(node, ptr, typeId);
		NODECPP_ASSERT(id != 0);
	}
	void appListen(net::ServerTBase* ptr, const char* ip, uint16_t port, int backlog) {
		Ip4 myIp = Ip4::parse(ip);
		Port myPort = Port::fromHost(port);
		if (!internal_usage_only::internal_bind_socket(ptr->dataForCommandProcessing.osSocket, myIp, myPort)) {
			throw Error();
		}
		if (!internal_usage_only::internal_listen_tcp_socket(ptr->dataForCommandProcessing.osSocket)) {
			throw Error();
		}
		ptr->dataForCommandProcessing.refed = true;
		NODECPP_ASSERT( ptr->dataForCommandProcessing.index != 0 );
		/*pollfd p;
		p.fd = ptr->dataForCommandProcessing.osSocket;
		p.events = POLLIN;*/
		ioSockets.setAssociated(ptr->dataForCommandProcessing.index/*, p*/);
		ioSockets.setPollin(ptr->dataForCommandProcessing.index);
		ioSockets.setRefed(ptr->dataForCommandProcessing.index, true);
		pendingListenEvents.push_back( ptr->dataForCommandProcessing.index );
	}

	void appRef(size_t id) { appGetEntry(id).getServerSocketData()->refed = true; }
	void appUnref(size_t id) { 
		auto& entry = appGetEntry(id);
		entry.getServerSocketData()->refed = false; 
	}
	void appReportBeingDestructed(size_t id) { 
		auto& entry = appGetEntry(id);
		entry.setUnused(); 
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
	size_t addServerEntry(NodeBase* node, net::ServerTBase* ptr, int typeId);
	NetSocketEntry& appGetEntry(size_t id) { return ioSockets.at(id); }
	const NetSocketEntry& appGetEntry(size_t id) const { return ioSockets.at(id); }
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

	// to help with 'poll'
//	size_t infraGetPollFdSetSize() const;
	/*
	* TODO: for performace reasons, the poll data should be cached inside NetSocketManager
	* and updated at the same time that StreamSocketEntry.
	* Avoid to have to write it all over again every time
	*
	*/
#if 0
	bool infraSetPollFdSet(pollfd* begin, const pollfd* end) const
	{ 
		size_t sz = end - begin;
		assert(sz >= ioSockets.size());
		bool anyRefed = false;

		for (size_t i = 0; i != sz; ++i)
		{
//			if (i < ioSockets.size() && ioSockets[i].isValid())
			if (i < ioSockets.size() && ioSockets[i].isAssociated())
			{
				const auto& current = ioSockets[i];
				NODECPP_ASSERT(current.getServerSocketData()->osSocket != INVALID_SOCKET);

				anyRefed = anyRefed || current.getServerSocketData()->refed;

				begin[i].fd = current.getServerSocketData()->osSocket;
				begin[i].events = 0;

				bool f2 = true;
				if (f2)
					begin[i].events |= POLLIN;
			}
			else
				begin[i].fd = INVALID_SOCKET;
		}

		return anyRefed;
	}
#endif // 0
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
//				if (entry.isValid())
				if (entry.isUsed())
				{
					if (entry.getServerSocketData()->osSocket != INVALID_SOCKET)
						internal_usage_only::internal_close(entry.getServerSocketData()->osSocket);
					//			entry.getPtr()->emitClose(entry.second);
					//evs.add(&net::Server::emitClose, entry.getPtr(), current.second);
					EmitterType::emitClose( entry.getEmitter(), current.second);
				}
				entry = NetSocketEntry(current.first);
			}
		}
		pendingCloseEvents.clear();
	}
#if 0 // old version
	void infraCheckPollFdSet(const pollfd* begin, const pollfd* end)
	{
		assert(end - begin >= static_cast<ptrdiff_t>(ioSockets.size()));
		for (size_t i = 0; i != ioSockets.size(); ++i)
		{
			auto& current = ioSockets[i];
			if (begin[i].fd != INVALID_SOCKET)
			{
				if ((begin[i].revents & (POLLERR | POLLNVAL)) != 0) // check errors first
				{
					NODECPP_TRACE("POLLERR event at {}", begin[i].fd);
					internal_usage_only::internal_getsockopt_so_error(current.getServerSocketData()->osSocket);
					infraMakeErrorEventAndClose(current/*, evs*/);
				}
				else if ((begin[i].revents & POLLIN) != 0)
				{
					NODECPP_TRACE("POLLIN event at {}", begin[i].fd);
					infraProcessAcceptEvent(current/*, evs*/);
				}
				else if (begin[i].revents != 0)
				{
					NODECPP_TRACE("Unexpected event at {}, value {:x}", begin[i].fd, begin[i].revents);
					internal_usage_only::internal_getsockopt_so_error(current.getServerSocketData()->osSocket);
					infraMakeErrorEventAndClose(current/*, evs*/);
				}
			}
		}
	}
#else
	void infraCheckPollFdSet(NetSocketEntry& current, pollfd p)
	{
//		assert(end - begin >= static_cast<ptrdiff_t>(ioSockets.size()));
//		for (size_t i = 0; i != ioSockets.size(); ++i)
		{
//			auto& current = ioSockets[i];
//			if (begin[i].fd != INVALID_SOCKET)
			{
				if ((p.revents & (POLLERR | POLLNVAL)) != 0) // check errors first
				{
					NODECPP_TRACE("POLLERR event at {}", p.fd);
					internal_usage_only::internal_getsockopt_so_error(current.getServerSocketData()->osSocket);
					infraMakeErrorEventAndClose(current/*, evs*/);
				}
				else if ((p.revents & POLLIN) != 0)
				{
					NODECPP_TRACE("POLLIN event at {}", p.fd);
					infraProcessAcceptEvent(current/*, evs*/);
				}
				else if (p.revents != 0)
				{
					NODECPP_TRACE("Unexpected event at {}, value {:x}", p.fd, p.revents);
					internal_usage_only::internal_getsockopt_so_error(current.getServerSocketData()->osSocket);
					infraMakeErrorEventAndClose(current/*, evs*/);
				}
			}
		}
	}
#endif
private:
	void infraProcessAcceptEvent(NetSocketEntry& entry)
	{
		OpaqueSocketData osd( false );
		if ( !netSocketManagerBase->getAcceptedSockData(entry.getServerSocketData()->osSocket, osd) )
			return;
		net::SocketTBase* ptr = EmitterType::makeSocket(entry.getEmitter(), osd);
		NODECPP_ASSERT( netSocketManagerBase != nullptr );
		netSocketManagerBase->infraAddAccepted(ptr);
		EmitterType::emitConnection( entry.getEmitter(), ptr );

		return;
	}

	void infraMakeErrorEventAndClose(NetSocketEntry& entry)
	{
//		evs.add(&net::Server::emitError, entry.getPtr(), std::ref(infraStoreError(Error())));
		Error e;
		EmitterType::emitError( entry.getEmitter(), e );
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


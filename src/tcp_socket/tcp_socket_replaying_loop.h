/* -------------------------------------------------------------------------------
* Copyright (c) 2020, OLogN Technologies AG
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


#ifndef TCP_SOCKET_REPLAYING_LOOP_H
#define TCP_SOCKET_REPLAYING_LOOP_H

#include "tcp_socket_base.h"
#include "../clustering_impl/clustering_impl.h"
#include "../clustering_impl/interthread_comm.h"

using namespace nodecpp;

extern thread_local NodeBase* thisThreadNode;
class NodeReplayer
{
public:
	static void infraAddAccepted(soft_ptr<net::SocketBase> ptr)
	{
		ptr->dataForCommandProcessing.state = net::SocketBase::DataForCommandProcessing::Connected;
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, ptr->dataForCommandProcessing.writeBuffer.used_size() == 0 );
		ptr->dataForCommandProcessing.refed = true;
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical,ptr->dataForCommandProcessing.remoteEnded == false);
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical,ptr->dataForCommandProcessing.paused == false);
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical,ptr->dataForCommandProcessing.state != net::SocketBase::DataForCommandProcessing::Connecting);
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical,ptr->dataForCommandProcessing.writeBuffer.empty());
	}

	static void registerAndAssignSocket(nodecpp::safememory::soft_ptr<net::SocketBase> ptr)
	{
		if ( ::nodecpp::threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::replaying )
		{
			auto frame = threadLocalData.binaryLog->readNextFrame();
			if ( frame.type == record_and_replay_impl::BinaryLog::FrameType::sock_register_2 )
			{
				record_and_replay_impl::BinaryLog::ServerOrSocketRegisterFrameData* data = reinterpret_cast<record_and_replay_impl::BinaryLog::ServerOrSocketRegisterFrameData*>( frame.ptr );
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, data->type == record_and_replay_impl::BinaryLog::ServerOrSocketRegisterFrameData::ObjectType::ClientSocket );
				threadLocalData.binaryLog->addPointerMapping( data->ptr, &(*ptr) );
				ptr->dataForCommandProcessing.osSocket = data->index;
				ptr->dataForCommandProcessing.osSocket = data->socket;
//				ptr->dataForCommandProcessing.osSocket = reinterpret_cast<SocketRiia*>(reinterpret_cast<uint8_t*>(frame.ptr) + sizeof(record_and_replay_impl::BinaryLog::SocketEvent))->release();
//				NetSocketEntry entry( data->index, ptr );
			}
			else
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "UNEXPECTED FRAME TYPE {}", frame.type ); 
		}
		else
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "can be called in replay only" ); 
	}

	static void appConnectSocket(net::SocketBase* sockPtr, const char* ip, uint16_t port)
	{
		// TODO: check sockPtr validity
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, (SOCKET)(sockPtr->dataForCommandProcessing.osSocket) != INVALID_SOCKET);
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical,sockPtr->dataForCommandProcessing.state == net::SocketBase::DataForCommandProcessing::Uninitialized);
		auto frame = threadLocalData.binaryLog->readNextFrame();
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, frame.type == record_and_replay_impl::BinaryLog::FrameType::sock_connect, "UNEXPECTED FRAME TYPE  (indeed: {})", frame.type );
		sockPtr->dataForCommandProcessing.state = net::SocketBase::DataForCommandProcessing::Connecting;
		sockPtr->dataForCommandProcessing.refed = true;
	}

	static void closeSocket( net::SocketBase* sockPtr ) //app-infra neutral
	{
		if ( !( sockPtr->dataForCommandProcessing.state == net::SocketBase::DataForCommandProcessing::Closing ||
				sockPtr->dataForCommandProcessing.state == net::SocketBase::DataForCommandProcessing::ErrorClosing ||
				sockPtr->dataForCommandProcessing.state == net::SocketBase::DataForCommandProcessing::Closed ) )
		{
			sockPtr->dataForCommandProcessing.state = net::SocketBase::DataForCommandProcessing::Closing;
			auto frame = threadLocalData.binaryLog->readNextFrame();
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, frame.type == record_and_replay_impl::BinaryLog::FrameType::sock_closing, "UNEXPECTED FRAME TYPE  (indeed: {})", frame.type );
		}
	}
	static void errorCloseSocket(  net::SocketBase* sockPtr , Error& err ) //app-infra neutral
	{
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, ::nodecpp::threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::replaying );
		if ( !( sockPtr->dataForCommandProcessing.state == net::SocketBase::DataForCommandProcessing::Closing ||
				sockPtr->dataForCommandProcessing.state == net::SocketBase::DataForCommandProcessing::ErrorClosing ||
				sockPtr->dataForCommandProcessing.state == net::SocketBase::DataForCommandProcessing::Closed ) )
		{
			sockPtr->dataForCommandProcessing.state = net::SocketBase::DataForCommandProcessing::ErrorClosing;
			auto frame = threadLocalData.binaryLog->readNextFrame();
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, frame.type == record_and_replay_impl::BinaryLog::FrameType::sock_error_closing, "UNEXPECTED FRAME TYPE  (indeed: {})", frame.type );
		}
	}

//	enum ShouldEmit { EmitNone, EmitConnect, EmitDrain };
//	ShouldEmit _infraProcessWriteEvent(net::SocketBase::DataForCommandProcessing& sockData);

	static void appAddServer(nodecpp::safememory::soft_ptr<net::ServerBase> ptr)
	{
#ifndef NODECPP_ENABLE_CLUSTERING
		if ( ::nodecpp::threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::replaying )
		{
			auto frame = threadLocalData.binaryLog->readNextFrame();
			if ( frame.type == record_and_replay_impl::BinaryLog::FrameType::server_register_2 )
			{
				record_and_replay_impl::BinaryLog::ServerOrSocketRegisterFrameData* data = reinterpret_cast<record_and_replay_impl::BinaryLog::ServerOrSocketRegisterFrameData*>( frame.ptr );
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, data->type == record_and_replay_impl::BinaryLog::ServerOrSocketRegisterFrameData::ObjectType::ServerSocket );
				threadLocalData.binaryLog->addPointerMapping( data->ptr, &(*ptr) );
				threadLocalData.binaryLog->addPointerMapping( data->dataForCommProcPtr, &(ptr->dataForCommandProcessing) );
				ptr->dataForCommandProcessing.osSocket = data->socket;
				ptr->dataForCommandProcessing.index = data->index;
			}
			else
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "UNEXPECTED FRAME TYPE {}", frame.type ); 
		}
#else
#ifdef NODECPP_RECORD_AND_REPLAY
#error not(yet) implemented
#endif // NODECPP_RECORD_AND_REPLAY
		if ( getCluster().isMaster() )
		{
			SocketRiia s(internal_usage_only::internal_make_tcp_socket());
			if (!s)
			{
				throw Error();
			}
			ptr->dataForCommandProcessing.osSocket = s.release();
			addServerEntry(ptr);
		}
		else
			addSlaveServerEntry(ptr);
#endif // NODECPP_ENABLE_CLUSTERING
	}


	template<class DataForCommandProcessing>
	static void appListen(DataForCommandProcessing& dataForCommandProcessing, nodecpp::Ip4 ip, uint16_t port, int backlog) { //TODO:CLUSTERING alt impl
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, ::nodecpp::threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::replaying );
		Port myPort = Port::fromHost(port);
#ifdef NODECPP_ENABLE_CLUSTERING
#ifdef NODECPP_RECORD_AND_REPLAY
#error not(yet) implemented
#endif // NODECPP_RECORD_AND_REPLAY
		if ( getCluster().isWorker() )
		{
			dataForCommandProcessing.localAddress.ip = ip;
			dataForCommandProcessing.localAddress.port = port;
			dataForCommandProcessing.localAddress.family = family;
			getCluster().acceptRequestForListeningAtSlave( dataForCommandProcessing.index, ip, port, family, backlog );
			return;
		}
#endif // NODECPP_ENABLE_CLUSTERING
		auto frame = threadLocalData.binaryLog->readNextFrame();
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, frame.type == record_and_replay_impl::BinaryLog::FrameType::server_listen, "indeed type = {}", frame.type );
		record_and_replay_impl::BinaryLog::SocketEvent* edata = reinterpret_cast<record_and_replay_impl::BinaryLog::SocketEvent*>( frame.ptr );
		DataForCommandProcessing* dfcp = reinterpret_cast<DataForCommandProcessing*>( threadLocalData.binaryLog->mapPointer( edata->ptr ) );

		nodecpp::IPFAMILY family = nodecpp::string_literal( "IPv4" );

		dfcp->refed = true;
		dfcp->localAddress.ip = ip;
		dfcp->localAddress.port = port;
		dfcp->localAddress.family = family;
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, dfcp->index != 0 );
	}

	static void runReplayingLoop()
	{
		record_and_replay_impl::BinaryLog::FrameData fd;
		for(;;)
		{
			fd = threadLocalData.binaryLog->readNextFrame();
			switch ( fd.type )
			{
				case record_and_replay_impl::BinaryLog::FrameType::sock_error_closing:
				{
					record_and_replay_impl::BinaryLog::SocketCloseEvent* edata = reinterpret_cast<record_and_replay_impl::BinaryLog::SocketCloseEvent*>( fd.ptr );
					net::SocketBase* sockPtr = reinterpret_cast<net::SocketBase*>( threadLocalData.binaryLog->mapPointer( edata->ptr ) );
					nodecpp::safememory::soft_ptr<net::SocketBase> sockSoftPtr = sockPtr->myThis.getSoftPtr<net::SocketBase>(sockPtr);
					if ( edata->err && edata->used ) //if error closing, then first error event
					{
						Error e; // TODO-REPLAY: we may need to save an actual value of 'current.second.second' in the recorded call
						sockPtr->emitError( e );
						if ( sockPtr->dataForCommandProcessing.isErrorEventHandler())
							sockPtr->dataForCommandProcessing.handleErrorEvent( sockSoftPtr, e );
					}
					if ( edata->used )
						sockPtr->emitClose( edata->err );
					if ( sockPtr->dataForCommandProcessing.isCloseEventHandler())
						sockPtr->dataForCommandProcessing.handleCloseEvent( sockSoftPtr, edata->err );
					if ( edata->used )
						sockPtr->dataForCommandProcessing.state = net::SocketBase::DataForCommandProcessing::Closed;
					sockPtr->onFinalCleanup();
					break;
				}

				case record_and_replay_impl::BinaryLog::FrameType::sock_accepted:
				{
					record_and_replay_impl::BinaryLog::SocketEvent* edata = reinterpret_cast<record_and_replay_impl::BinaryLog::SocketEvent*>( fd.ptr );
					net::SocketBase* sockPtr = reinterpret_cast<net::SocketBase*>( threadLocalData.binaryLog->mapPointer( edata->ptr ) );
					nodecpp::safememory::soft_ptr<net::SocketBase> sockSoftPtr = sockPtr->myThis.getSoftPtr<net::SocketBase>(sockPtr);
					auto hr = sockPtr->dataForCommandProcessing.ahd_accepted;
					if ( hr )
					{
						sockPtr->dataForCommandProcessing.ahd_accepted = nullptr;
						hr();
					}
					else // TODO: make sure we never have both cases in the same time
					{
						sockPtr->emitAccepted();
						if (sockPtr->dataForCommandProcessing.isAcceptedEventHandler())
							sockPtr->dataForCommandProcessing.handleAcceptedEvent( sockSoftPtr );
					}
					break;
				}

				case record_and_replay_impl::BinaryLog::FrameType::sock_read_failed_event_crh:
				{
					record_and_replay_impl::BinaryLog::SocketEvent* edata = reinterpret_cast<record_and_replay_impl::BinaryLog::SocketEvent*>( fd.ptr );
					net::SocketBase* sockPtr = reinterpret_cast<net::SocketBase*>( threadLocalData.binaryLog->mapPointer( edata->ptr ) );
					nodecpp::safememory::soft_ptr<net::SocketBase> sockSoftPtr = sockPtr->myThis.getSoftPtr<net::SocketBase>(sockPtr);
					auto hr = sockPtr->dataForCommandProcessing.ahd_read.h;
					NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, hr );
//					Error e;
//					errorCloseSocket( sockPtr, e );
					sockPtr->dataForCommandProcessing.ahd_read.h = nullptr;
					nodecpp::setException(hr, std::exception()); // TODO: switch to our exceptions ASAP!
					hr();
					break;
				}

				case record_and_replay_impl::BinaryLog::FrameType::sock_read_event_crh:
				{
					record_and_replay_impl::BinaryLog::SocketEvent* edata = reinterpret_cast<record_and_replay_impl::BinaryLog::SocketEvent*>( fd.ptr );
					net::SocketBase* sockPtr = reinterpret_cast<net::SocketBase*>( threadLocalData.binaryLog->mapPointer( edata->ptr ) );
					nodecpp::safememory::soft_ptr<net::SocketBase> sockSoftPtr = sockPtr->myThis.getSoftPtr<net::SocketBase>(sockPtr);
					auto hr = sockPtr->dataForCommandProcessing.ahd_read.h;
					NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, hr );
					sockPtr->dataForCommandProcessing.ahd_read.h = nullptr;
					hr();
					break;
				}

				case record_and_replay_impl::BinaryLog::FrameType::sock_read_event_call:
				{
					record_and_replay_impl::BinaryLog::SocketEvent* edata = reinterpret_cast<record_and_replay_impl::BinaryLog::SocketEvent*>( fd.ptr );
					net::SocketBase* sockPtr = reinterpret_cast<net::SocketBase*>( threadLocalData.binaryLog->mapPointer( edata->ptr ) );
					nodecpp::safememory::soft_ptr<net::SocketBase> sockSoftPtr = sockPtr->myThis.getSoftPtr<net::SocketBase>(sockPtr);
					Buffer recvBuffer;
					recvBuffer.append( reinterpret_cast<uint8_t*>( fd.ptr ) + sizeof( record_and_replay_impl::BinaryLog::SocketEvent ), fd.size - sizeof( record_and_replay_impl::BinaryLog::SocketEvent ) );
					sockPtr->emitData( recvBuffer);
					if (sockPtr->dataForCommandProcessing.isDataEventHandler())
						sockPtr->dataForCommandProcessing.handleDataEvent(sockSoftPtr, recvBuffer);
					break;
				}

				case record_and_replay_impl::BinaryLog::FrameType::sock_remote_ended_call:
				{
					record_and_replay_impl::BinaryLog::SocketEvent* edata = reinterpret_cast<record_and_replay_impl::BinaryLog::SocketEvent*>( fd.ptr );
					net::SocketBase* sockPtr = reinterpret_cast<net::SocketBase*>( threadLocalData.binaryLog->mapPointer( edata->ptr ) );
					nodecpp::safememory::soft_ptr<net::SocketBase> sockSoftPtr = sockPtr->myThis.getSoftPtr<net::SocketBase>(sockPtr);
					sockPtr->dataForCommandProcessing.remoteEnded = true;
					sockPtr->emitEnd();
					if (sockPtr->dataForCommandProcessing.isEndEventHandler())
						sockPtr->dataForCommandProcessing.handleEndEvent(sockSoftPtr);
					break;
				}

				case record_and_replay_impl::BinaryLog::FrameType::sock_update_state:
				{
					record_and_replay_impl::BinaryLog::SocketUpdateState* edata = reinterpret_cast<record_and_replay_impl::BinaryLog::SocketUpdateState*>( fd.ptr );
					net::SocketBase* sockPtr = reinterpret_cast<net::SocketBase*>( threadLocalData.binaryLog->mapPointer( edata->ptr ) );
					nodecpp::safememory::soft_ptr<net::SocketBase> sockSoftPtr = sockPtr->myThis.getSoftPtr<net::SocketBase>(sockPtr);
					sockPtr->dataForCommandProcessing.state = (net::SocketBase::DataForCommandProcessing::State)( edata->state );
					break;
				}




				case record_and_replay_impl::BinaryLog::FrameType::sock_connect_event_crh:
				{
					record_and_replay_impl::BinaryLog::SocketEvent* edata = reinterpret_cast<record_and_replay_impl::BinaryLog::SocketEvent*>( fd.ptr );
					net::SocketBase* sockPtr = reinterpret_cast<net::SocketBase*>( threadLocalData.binaryLog->mapPointer( edata->ptr ) );
					nodecpp::safememory::soft_ptr<net::SocketBase> sockSoftPtr = sockPtr->myThis.getSoftPtr<net::SocketBase>(sockPtr);
					auto hr = sockPtr->dataForCommandProcessing.ahd_connect;
					NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, hr );
					sockPtr->dataForCommandProcessing.ahd_connect = nullptr;
					hr();
					break;
				}

				case record_and_replay_impl::BinaryLog::FrameType::sock_connect_event_call:
				{
					record_and_replay_impl::BinaryLog::SocketEvent* edata = reinterpret_cast<record_and_replay_impl::BinaryLog::SocketEvent*>( fd.ptr );
					net::SocketBase* sockPtr = reinterpret_cast<net::SocketBase*>( threadLocalData.binaryLog->mapPointer( edata->ptr ) );
					nodecpp::safememory::soft_ptr<net::SocketBase> sockSoftPtr = sockPtr->myThis.getSoftPtr<net::SocketBase>(sockPtr);
					sockPtr->emitConnect();
					if (sockPtr->dataForCommandProcessing.isConnectEventHandler())
						sockPtr->dataForCommandProcessing.handleConnectEvent(sockSoftPtr);
					break;
				}

				case record_and_replay_impl::BinaryLog::FrameType::sock_drain_event_crh:
				{
					record_and_replay_impl::BinaryLog::SocketEvent* edata = reinterpret_cast<record_and_replay_impl::BinaryLog::SocketEvent*>( fd.ptr );
					net::SocketBase* sockPtr = reinterpret_cast<net::SocketBase*>( threadLocalData.binaryLog->mapPointer( edata->ptr ) );
					nodecpp::safememory::soft_ptr<net::SocketBase> sockSoftPtr = sockPtr->myThis.getSoftPtr<net::SocketBase>(sockPtr);
					auto hr = sockPtr->dataForCommandProcessing.ahd_drain;
					NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, hr );
					sockPtr->dataForCommandProcessing.ahd_drain = nullptr;
					hr();
					break;
				}

				case record_and_replay_impl::BinaryLog::FrameType::sock_drain_event_call:
				{
					record_and_replay_impl::BinaryLog::SocketEvent* edata = reinterpret_cast<record_and_replay_impl::BinaryLog::SocketEvent*>( fd.ptr );
					net::SocketBase* sockPtr = reinterpret_cast<net::SocketBase*>( threadLocalData.binaryLog->mapPointer( edata->ptr ) );
					nodecpp::safememory::soft_ptr<net::SocketBase> sockSoftPtr = sockPtr->myThis.getSoftPtr<net::SocketBase>(sockPtr);
					auto hr = sockPtr->dataForCommandProcessing.ahd_accepted;
					sockPtr->emitDrain();
					if (sockPtr->dataForCommandProcessing.isDrainEventHandler())
						sockPtr->dataForCommandProcessing.handleDrainEvent(sockSoftPtr);
					break;
				}

				case record_and_replay_impl::BinaryLog::FrameType::server_close_event_1:
				{
					record_and_replay_impl::BinaryLog::SocketEvent* edata = reinterpret_cast<record_and_replay_impl::BinaryLog::SocketEvent*>( fd.ptr );
					net::ServerBase* serverPtr = reinterpret_cast<net::ServerBase*>( threadLocalData.binaryLog->mapPointer( edata->ptr ) );
					nodecpp::safememory::soft_ptr<net::ServerBase> serverSoftPtr = serverPtr->myThis.getSoftPtr<net::ServerBase>(serverPtr);
					serverPtr->internalCleanupBeforeClosing();
					break;
				}

				case record_and_replay_impl::BinaryLog::FrameType::server_close_event_2_crh:
				{
					record_and_replay_impl::BinaryLog::ServerCloseEvent_2* edata = reinterpret_cast<record_and_replay_impl::BinaryLog::ServerCloseEvent_2*>( fd.ptr );
					net::ServerBase* serverPtr = reinterpret_cast<net::ServerBase*>( threadLocalData.binaryLog->mapPointer( edata->ptr ) );
					nodecpp::safememory::soft_ptr<net::ServerBase> serverSoftPtr = serverPtr->myThis.getSoftPtr<net::ServerBase>(serverPtr);
					auto hr = serverPtr->dataForCommandProcessing.ahd_close;
					NODECPP_ASSERT( nodecpp::module_id, nodecpp::assert::AssertLevel::critical, hr != nullptr ); 
					serverPtr->dataForCommandProcessing.ahd_close = nullptr;
					if ( edata->err )
						nodecpp::setException( hr, std::exception() ); // TODO: switch to our exceptions ASAP!
					hr();
					break;
				}

				case record_and_replay_impl::BinaryLog::FrameType::server_close_event_2_call:
				{
					record_and_replay_impl::BinaryLog::ServerCloseEvent_2* edata = reinterpret_cast<record_and_replay_impl::BinaryLog::ServerCloseEvent_2*>( fd.ptr );
					net::ServerBase* serverPtr = reinterpret_cast<net::ServerBase*>( threadLocalData.binaryLog->mapPointer( edata->ptr ) );
					nodecpp::safememory::soft_ptr<net::ServerBase> serverSoftPtr = serverPtr->myThis.getSoftPtr<net::ServerBase>(serverPtr);
					serverPtr->emitClose( edata->err );
					if (serverPtr->dataForCommandProcessing.isCloseEventHandler())
						serverPtr->dataForCommandProcessing.handleCloseEvent( serverSoftPtr, edata->err );
					break;
				}

				case record_and_replay_impl::BinaryLog::FrameType::server_listening_event_crh:
				{
					record_and_replay_impl::BinaryLog::SocketEvent* edata = reinterpret_cast<record_and_replay_impl::BinaryLog::SocketEvent*>( fd.ptr );
					net::ServerBase* serverPtr = reinterpret_cast<net::ServerBase*>( threadLocalData.binaryLog->mapPointer( edata->ptr ) );
					nodecpp::safememory::soft_ptr<net::ServerBase> serverSoftPtr = serverPtr->myThis.getSoftPtr<net::ServerBase>(serverPtr);
					auto hr = serverPtr->dataForCommandProcessing.ahd_listen;
					NODECPP_ASSERT( nodecpp::module_id, nodecpp::assert::AssertLevel::critical, hr != nullptr ); 
					serverPtr->dataForCommandProcessing.ahd_listen = nullptr;
					hr();
					break;
				}

				case record_and_replay_impl::BinaryLog::FrameType::server_listening_event_call:
				{
					record_and_replay_impl::BinaryLog::SocketEvent* edata = reinterpret_cast<record_and_replay_impl::BinaryLog::SocketEvent*>( fd.ptr );
					net::ServerBase* serverPtr = reinterpret_cast<net::ServerBase*>( threadLocalData.binaryLog->mapPointer( edata->ptr ) );
					nodecpp::safememory::soft_ptr<net::ServerBase> serverSoftPtr = serverPtr->myThis.getSoftPtr<net::ServerBase>(serverPtr);
					// TODO: revise around serverPtr->dataForCommandProcessing.index
					serverPtr->emitListening(serverPtr->dataForCommandProcessing.index, serverPtr->dataForCommandProcessing.localAddress);
					if ( serverPtr->dataForCommandProcessing.isListenEventHandler() )
						serverPtr->dataForCommandProcessing.handleListenEvent(serverSoftPtr, serverPtr->dataForCommandProcessing.index, serverPtr->dataForCommandProcessing.localAddress);
					break;
				}

				case record_and_replay_impl::BinaryLog::FrameType::server_make_socket_input:
				{
					record_and_replay_impl::BinaryLog::SocketEvent* edata = reinterpret_cast<record_and_replay_impl::BinaryLog::SocketEvent*>( fd.ptr );
					net::ServerBase* serverPtr = reinterpret_cast<net::ServerBase*>( threadLocalData.binaryLog->mapPointer( edata->ptr ) );
					nodecpp::safememory::soft_ptr<net::ServerBase> serverSoftPtr = serverPtr->myThis.getSoftPtr<net::ServerBase>(serverPtr);
					OpaqueSocketData& osd = *reinterpret_cast<OpaqueSocketData*>( reinterpret_cast<uint8_t*>(fd.ptr) + sizeof(record_and_replay_impl::BinaryLog::SocketEvent));
					auto ptr = serverPtr->makeSocket( osd );
					break;
				}

				case record_and_replay_impl::BinaryLog::FrameType::server_make_socket_output:
				{
					record_and_replay_impl::BinaryLog::ServerMakeSocketOutput* edata = reinterpret_cast<record_and_replay_impl::BinaryLog::ServerMakeSocketOutput*>( fd.ptr );
					net::SocketBase* sockPtr = reinterpret_cast<net::SocketBase*>( threadLocalData.binaryLog->mapPointer( edata->sockPtr ) );
					nodecpp::safememory::soft_ptr<net::SocketBase> sockSoftPtr = sockPtr->myThis.getSoftPtr<net::SocketBase>(sockPtr);
					infraAddAccepted( sockSoftPtr );
					sockPtr->dataForCommandProcessing._remote.ip = edata->remoteIp;
					sockPtr->dataForCommandProcessing._remote.port = edata->remotePort.getHost();
					break;
				}

				case record_and_replay_impl::BinaryLog::FrameType::server_consume_socket_event_crh:
				{
					record_and_replay_impl::BinaryLog::serverConsumeSocket* edata = reinterpret_cast<record_and_replay_impl::BinaryLog::serverConsumeSocket*>( fd.ptr );
					net::ServerBase* serverPtr = reinterpret_cast<net::ServerBase*>( threadLocalData.binaryLog->mapPointer( edata->ptr ) );
//					nodecpp::safememory::soft_ptr<net::ServerBase> serverSoftPtr = serverPtr->myThis.getSoftPtr<net::ServerBase>(serverPtr);
					net::SocketBase* socketPtr = reinterpret_cast<net::SocketBase*>( threadLocalData.binaryLog->mapPointer( edata->sockPtr ) );
					nodecpp::safememory::soft_ptr<net::SocketBase> socketSoftPtr = socketPtr->myThis.getSoftPtr<net::SocketBase>(socketPtr);
					auto hr = serverPtr->dataForCommandProcessing.ahd_connection.h;
					NODECPP_ASSERT( nodecpp::module_id, nodecpp::assert::AssertLevel::critical, hr != nullptr ); 
					serverPtr->dataForCommandProcessing.ahd_connection.sock = socketSoftPtr;
					serverPtr->dataForCommandProcessing.ahd_connection.h = nullptr;
					hr();
					break;
				}

				case record_and_replay_impl::BinaryLog::FrameType::server_consume_socket_event_call:
				{
					record_and_replay_impl::BinaryLog::serverConsumeSocket* edata = reinterpret_cast<record_and_replay_impl::BinaryLog::serverConsumeSocket*>( fd.ptr );
					net::ServerBase* serverPtr = reinterpret_cast<net::ServerBase*>( threadLocalData.binaryLog->mapPointer( edata->ptr ) );
					nodecpp::safememory::soft_ptr<net::ServerBase> serverSoftPtr = serverPtr->myThis.getSoftPtr<net::ServerBase>(serverPtr);
					net::SocketBase* socketPtr = reinterpret_cast<net::SocketBase*>( threadLocalData.binaryLog->mapPointer( edata->sockPtr ) );
					nodecpp::safememory::soft_ptr<net::SocketBase> socketSoftPtr = socketPtr->myThis.getSoftPtr<net::SocketBase>(socketPtr);
					serverPtr->emitConnection( socketSoftPtr );
					if (serverPtr->dataForCommandProcessing.isConnectionEventHandler())
						serverPtr->dataForCommandProcessing.handleConnectionEvent( serverSoftPtr, socketSoftPtr );
					break;
				}

				case record_and_replay_impl::BinaryLog::FrameType::server_error_event_call:
				{
					record_and_replay_impl::BinaryLog::SocketEvent* edata = reinterpret_cast<record_and_replay_impl::BinaryLog::SocketEvent*>( fd.ptr );
					net::ServerBase* serverPtr = reinterpret_cast<net::ServerBase*>( threadLocalData.binaryLog->mapPointer( edata->ptr ) );
					nodecpp::safememory::soft_ptr<net::ServerBase> serverSoftPtr = serverPtr->myThis.getSoftPtr<net::ServerBase>(serverPtr);
					Error e;
					serverPtr->emitError( e );
					if (serverPtr->dataForCommandProcessing.isErrorEventHandler())
						serverPtr->dataForCommandProcessing.handleErrorEvent( serverSoftPtr, e );
					break;
				}
			}
		}
	}

	template<class Node>
	static void run()
	{
		interceptNewDeleteOperators(true);
		{
#ifdef NODECPP_THREADLOCAL_INIT_BUG_GCC_60702
			nodecpp::net::SocketBase::DataForCommandProcessing::userHandlerClassPattern.init();
			nodecpp::net::ServerBase::DataForCommandProcessing::userHandlerClassPattern.init();
#endif // NODECPP_THREADLOCAL_INIT_BUG_GCC_60702

#ifdef NODECPP_ENABLE_CLUSTERING
#error not yet implemented
			if ( isMaster )
			{
				uintptr_t readHandle = initInterThreadCommSystemAndGetReadHandleForMainThread();
				infra.ioSockets.setAwakerSocket( readHandle );
			}
			else
			{
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, startupData != nullptr );
				infra.ioSockets.setAwakerSocket( startupData->readHandle );
			}
#endif
			// from now on all internal structures are ready to use; let's run their "users"
#ifdef NODECPP_ENABLE_CLUSTERING
#error not yet implemented
			nodecpp::postinitThreadClusterObject();
			if ( isMaster )
			{
				size_t listenerCnt = 1;
				auto argv = getArgv();
				for ( size_t i=1; i<argv.size(); ++i )
				{
					if ( argv[i].size() > 13 && argv[i].substr(0,13) == "numlisteners=" )
						listenerCnt = atol(argv[i].c_str() + 13);
				}
				for ( size_t i=0; i<listenerCnt; ++i )
					createListenerThread();
			}
#endif // NODECPP_ENABLE_CLUSTERING

			owning_ptr<Node> node;
			node = make_owning<Node>();
			thisThreadNode = &(*node); 
			node->binLog.initForReplaying();
			::nodecpp::threadLocalData.binaryLog = &(node->binLog);
			// NOTE!!! 
			// By coincidence it so happened that both void Node::main() and nodecpp::handler_ret_type Node::main() are currently treated in the same way.
			// If, for any reason, treatment should be different, to check exactly which one is present, see, for instance
			// http://www.gotw.ca/gotw/071.htm and 
			// https://stackoverflow.com/questions/87372/check-if-a-class-has-a-member-function-of-a-given-signature
			node->main();
			runReplayingLoop();
			node = nullptr;

#ifdef NODECPP_THREADLOCAL_INIT_BUG_GCC_60702
			nodecpp::net::SocketBase::DataForCommandProcessing::userHandlerClassPattern.destroy();
			nodecpp::net::ServerBase::DataForCommandProcessing::userHandlerClassPattern.destroy();
#endif // NODECPP_THREADLOCAL_INIT_BUG_GCC_60702
		}
		killAllZombies();
		interceptNewDeleteOperators(false);
	}
};


#endif // TCP_SOCKET_REPLAYING_LOOP_H


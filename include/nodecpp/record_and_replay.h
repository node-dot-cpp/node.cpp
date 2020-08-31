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

#ifndef RECORD_AND_REPLAY_H
#define RECORD_AND_REPLAY_H

#include <nodecpp/common.h>
#include <page_allocator.h>
#include <nodecpp/ip_and_port.h>

namespace nodecpp::record_and_replay_impl {

class BinaryLog
{
public:
	enum Mode { not_using, recording, replaying };
	enum FrameType { incomplete = 0, 
		// node
		node_main_call,
		// client socket
		sock_register, sock_register_2, sock_connect, sock_connect_crh_ok, sock_connect_crh_timeout, sock_connect_crh_except, sock_accepted, 
		sock_read_event_crh, sock_read_failed_event_crh, sock_read_event_call, sock_read_crh_ok, sock_read_crh_except, sock_read_crh_timeout, 
		sock_connect_event_crh, sock_connect_event_call, sock_drain_event_crh_ok, sock_drain_event_crh_except, sock_drain_event_crh_timeout, sock_drain_event_call, 
		sock_closing, sock_error_preclosing, sock_error_closing, sock_process_close_event, sock_remote_ended_call, 
		sock_update_state, 
		// server socket
		server_register, server_register_2, server_listen, server_listen_crh_except, server_listen_crh_timeout, server_listen_crh_ok,
		server_make_socket_input, server_make_socket_output, server_consume_socket_event_crh, server_consume_socket_event_call,
//		server_all_conn_ended, 
		server_close_event_1, server_close_event_2_crh, server_close_event_2_call, server_close_crh_except, server_close_crh_timeout, server_close_crh_ok,
		server_error_event_call,
		server_listening_event_crh, server_listening_event_call, 
		server_conn_crh_except, server_conn_crh_ok, 
		// http support
		http_sock_read_data_crh_ok, http_sock_read_data_crh_except, 
		// common
		coro_await_ready_res, 
		// control
		type_max
	};

	struct FrameData // ret value while reading
	{
		void* ptr = nullptr;
		size_t size = 0;
		uint32_t type = 0;
	};

	// some helper structures for particualr frame types

	struct SocketEvent
	{
		uintptr_t ptr;
	};

	struct ServerOrSocketRegisterFrameData
	{
		enum ObjectType { Undefined, ClientSocket, ServerSocket, AgentServer };
		ObjectType type;
		size_t index;
		uintptr_t ptr;
		uintptr_t dataForCommProcPtr;
		uint64_t socket;
	};

	struct SocketUpdateState
	{
		uintptr_t ptr;
		int state;
	};

	struct SocketCloseEvent
	{
		uintptr_t ptr;
		bool err;
		bool used;
	};

	struct serverRegister
	{
		uintptr_t ptr;
		uintptr_t commProcDataPtr;
	};

	struct ServerListen
	{
		uintptr_t ptr;
		Ip4 ip;
		uint16_t port;
		int backlog;
	};

	struct ServerMakeSocketOutput
	{
		uintptr_t sockPtr;
		Ip4 remoteIp;
		Port remotePort;
	};

	struct serverConsumeSocket
	{
		uintptr_t ptr;
		uintptr_t sockPtr;
	};

	struct ServerCloseEvent_2
	{
		uintptr_t ptr;
		bool err;
	};

private:
	struct LogHeader
	{
		uint8_t signature[16];
		uint64_t size;
		uint64_t exeCheckSum; // size TBD
		uint64_t hashAlgo;
	};
	struct FrameHeader // for writing
	{
		uint32_t type;
		uint32_t size;
	};

	using PtrMapT = ::std::map<uintptr_t, void*, std::less<uintptr_t>, nodecpp::stdallocator<std::pair<const uintptr_t, void*>>>;
	PtrMapT pointerMap;

private:
	Mode mode_ = Mode::not_using;
	uint8_t* mem = nullptr;
	size_t size = 0;
	FrameHeader* fh = nullptr;

	uint8_t* writePos = nullptr;
	size_t currentFrameSize = 0;
	uint32_t currentFrameType = 0;

	size_t frameIdx = 0;
public:
	BinaryLog() {}
	~BinaryLog() { deinit(); }

	void initForRecording( size_t szExp ) {
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, mode_ == Mode::not_using, "indeed: {}", (size_t)mode_ ); 
		mode_ = Mode::recording;
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, szExp >= 16, "szExp = {}", szExp ); 
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, szExp <= 48, "szExp = {}", szExp ); 
		size = ((size_t)1) << szExp;
		mem = (uint8_t*)(nodecpp::VirtualMemory::allocate( size ) );
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, mem != nullptr ); 
		// TODO: init header
		writePos = mem + sizeof( LogHeader );
		FrameHeader* h = (FrameHeader*)writePos;
		h->type = 0; // no records yet
	}
	size_t dbgWalkThrougFrames()
	{
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, mode_ == Mode::replaying, "indeed: {}", (size_t)mode_ ); 
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, mem ); 
		size_t ret = 0;
		LogHeader* lh = (LogHeader*)(mem);
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, lh->size <= size, "indeed: {} vs. {}", lh->size, size ); 
		FrameHeader* frame = (FrameHeader*)(lh + 1);
		for ( ; ; ++ret )
		{
			if ( (uint8_t*)frame + sizeof( FrameHeader ) < mem + size && frame->type && (uint8_t*)frame + sizeof( FrameHeader ) + frame->size <= mem + size) {
//				printf( "[%zd] frame type %d, size %d\n", ret, frame->type, frame->size );
				frame = (FrameHeader*)( (uint8_t*)(frame + 1) + frame->size );
			}
			else
			{
				printf( " -> NO MORE FRAMES\n" );
				break;
			}
		}
		return ret;
	}
	void initForReplaying() { // TODO: subject for revision (currently serves rather for immediate development and debugging purposes)
		constexpr size_t maxBuffsize = 1 << 26;
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, mode_ == Mode::not_using, "indeed: {}", (size_t)mode_ ); 
		mode_ = Mode::replaying;
		void* buff = nullptr;
		size_t sz = 0;
		buff = nodecpp::VirtualMemory::allocate( maxBuffsize );
		FILE* f = fopen( "binlog.dat", "rb" );
		if ( f != 0 )
			sz = fread( buff, 1, maxBuffsize, f );
		else
		{
			nodecpp::VirtualMemory::deallocate( buff, maxBuffsize );
			buff = nullptr;
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "Failed to open a default binary log for replaying" ); 
		}
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, sz > 0 ); 
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, buff != nullptr ); 
		size = sz;
		mem = (uint8_t*)(buff);
		// TODO: complete header validation
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, buff != nullptr ); 
		LogHeader* lh = (LogHeader*)(mem);
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, lh->size <= sz, "indeed: {} vs. {}", lh->size, sz ); 
		fh = (FrameHeader*)(lh + 1);
printf( " Binary buffer initialized. Buffer size: %zd bytes, %zd frames\n", sz, dbgWalkThrougFrames() );
	}
	void deinit() {
		if ( mode_ == Mode::recording )
		{
			// TODO: subject for revision (currently serves rather for immediate development and debugging purposes)
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, mem != nullptr );
			FILE* f = fopen( "binlog.dat", "wb" );
			if ( f != 0 )
				fwrite( mem, 1, writePos - mem, f );
		}
		if ( mem )
			nodecpp::VirtualMemory::deallocate( mem, size );
		mode_ = Mode::not_using;
	}

	void addFrame( uint32_t frameType, void* data, uint32_t sz) {
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, mode_ == Mode::recording, "indeed: {}", (size_t)mode_ ); 
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, currentFrameSize == 0, "indeed: {}", currentFrameSize ); 
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, currentFrameType == 0, "indeed: {}", currentFrameType ); 
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, fh == nullptr ); 
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, frameType != 0 ); 
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, data != nullptr || ( data == nullptr && sz == 0 ) ); 
		FrameHeader* h = (FrameHeader*)writePos;
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, h->type == 0 ); 
		if ( data != nullptr )
		{
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, mem + size >= writePos + sizeof( FrameHeader ) + sz ); 
			memcpy( h + 1, data, sz );
			h->size = sz;
		}
		else
		{
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, mem + size >= writePos + sz ); 
			h->size = 0;
		}
		writePos = (uint8_t*)(h+1) + sz;
		FrameHeader* hnext = (FrameHeader*)writePos;
		hnext->type = 0;
		h->type = frameType;
//printf( "  [+%zd] -> frame type %d, size %d\n", frameIdx++, h->type, h->size );
	}
	void startAddingFrame( uint32_t frameType, void* data, uint32_t sz) {
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, mode_ == Mode::recording, "indeed: {}", (size_t)mode_ ); 
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, frameType != 0 ); 
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, data != nullptr ); 
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, mem + size >= writePos + sizeof( FrameHeader ) + sz ); 
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, currentFrameSize == 0, "indeed: {}", currentFrameSize ); 
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, currentFrameType == 0, "indeed: {}", currentFrameType ); 
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, fh == nullptr ); 
		currentFrameType = frameType;
		fh = (FrameHeader*)writePos;
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, fh->type == 0 ); 
		memcpy( fh + 1, data, sz );
		writePos = (uint8_t*)(fh+1) + sz;
		currentFrameSize = sz;
	}
	void continueAddingFrame( void* data, uint32_t sz) {
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, mode_ == Mode::recording, "indeed: {}", (size_t)mode_ ); 
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, data != nullptr ); 
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, mem + size >= writePos + sz ); 
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, currentFrameSize != 0 ); 
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, currentFrameType != 0 ); 
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, fh != nullptr ); 
		memcpy( writePos, data, sz );
		writePos += sz;
		currentFrameSize += sz;
	}
	void addingFrameDone() {
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, mode_ == Mode::recording, "indeed: {}", (size_t)mode_ ); 
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, currentFrameSize != 0 ); 
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, currentFrameType != 0 ); 
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, fh != nullptr ); 
		FrameHeader* hnext = (FrameHeader*)writePos;
		hnext->type = 0;
		fh->size = currentFrameSize;
		fh->type = currentFrameType;
//printf( "  [+%zd] -> frame type %d, size %d\n", frameIdx++, fh->type, fh->size );
		fh = nullptr;
		currentFrameSize = 0;
		currentFrameType = 0;
	}

	FrameData readNextFrame() {
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, mode_ == Mode::replaying, "indeed: {}", (size_t)mode_ ); 
		FrameData fd;
		if ( (uint8_t*)fh + sizeof( FrameHeader ) < mem + size && fh->type && (uint8_t*)fh + sizeof( FrameHeader ) + fh->size <= mem + size) {
//printf( "  [-%zd] -> frame type %d, size %d\n", frameIdx++, fh->type, fh->size );
			fd.size = fh->size;
			fd.type = fh->type;
			fd.ptr = fh + 1;
			fh = (FrameHeader*)( (uint8_t*)(fh + 1) + fh->size );
			++frameIdx;
			return fd;
		}
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "NO MORE FRAMES (already read: {})", frameIdx ); 
		return fd;
	}
	void addPointerMapping( uintptr_t oldValue, void* newValue ) {
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, mode_ == Mode::replaying, "indeed: {}", (size_t)mode_ ); 
		auto insret = pointerMap.insert( std::make_pair( oldValue, newValue ) );
		if ( !insret.second )
			insret.first->second = newValue;
	}
	void* mapPointer( uintptr_t oldVal ) {
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, mode_ == Mode::replaying, "indeed: {}", (size_t)mode_ ); 
		auto f = pointerMap.find( oldVal );
		if ( f != pointerMap.end() )
			return f->second;
		return (void*)oldVal;
	}

	uint32_t nextFrameType() {
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, mode_ == Mode::replaying, "indeed: {}", (size_t)mode_ ); 
		if ( (uint8_t*)fh + sizeof( FrameHeader ) < mem + size )
			return fh->type;
		return 0;
	}

	Mode mode() { return mode_; }
};

}

#endif // RECORD_AND_REPLAY_H

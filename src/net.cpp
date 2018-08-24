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

#include "../include/nodecpp/net.h"

#include "infrastructure.h"

using namespace std;
using namespace nodecpp;
using namespace nodecpp::net;




void SocketBase::destroy() { OSLayer::appDestroy(dataForCommandProcessing); }
void SocketBase::end() { OSLayer::appEnd(dataForCommandProcessing); }

//void SocketBase::pause() { OSLayer::appPause(id); }
//void SocketBase::ref() { OSLayer::appRef(id); }

//void SocketBase::resume() { OSLayer::appResume(id); }
//void SocketBase::unref() { OSLayer::appUnref(id); }

//size_t SocketBase::bufferSize() const { return OSLayer::appBufferSize(id); }

bool SocketBase::write(const uint8_t* data, uint32_t size)
{
	_bytesWritten += size;
	return OSLayer::appWrite(dataForCommandProcessing, data, size);
}

#ifdef USING_O_SOCKETS
void SocketO::connect(uint16_t port, const char* ip)
{
	state = CONNECTING;
//	dataForCommandProcessing.id = getInfra().getNetSocket().appConnect(this, ip, port);
	dataForCommandProcessing.id = connectToInfra(this, ip, port);
}

SocketO& SocketO::setNoDelay(bool noDelay)
{ 
	OSLayer::appSetNoDelay(dataForCommandProcessing, noDelay);
	return *this;
}

SocketO& SocketO::setKeepAlive(bool enable)
{
	OSLayer::appSetKeepAlive(dataForCommandProcessing, enable);
	return *this;
}
#endif // USING_O_SOCKETS

#ifdef USING_L_SOCKETS
void Socket::connect(uint16_t port, const char* ip)
{
	state = CONNECTING;
//	dataForCommandProcessing.id = getInfra().getNetSocket().appConnect(this, ip, port);
	dataForCommandProcessing.id = connectToInfra(this, ip, port);
}

Socket& Socket::setNoDelay(bool noDelay)
{ 
	OSLayer::appSetNoDelay(dataForCommandProcessing, noDelay);
	return *this;
}

Socket& Socket::setKeepAlive(bool enable)
{
	OSLayer::appSetKeepAlive(dataForCommandProcessing, enable);
	return *this;
}
#endif // USING_L_SOCKETS

#ifdef USING_T_SOCKETS
void SocketTBase::connect(uint16_t port, const char* ip)
{
	assert(false); // revise implementation
	state = CONNECTING;
//	dataForCommandProcessing.id = connectToInfra(this, ip, port);
}

SocketTBase& SocketTBase::setNoDelay(bool noDelay)
{ 
	OSLayer::appSetNoDelay(dataForCommandProcessing, noDelay);
	return *this;
}

SocketTBase& SocketTBase::setKeepAlive(bool enable)
{
	OSLayer::appSetKeepAlive(dataForCommandProcessing, enable);
	return *this;
}
#endif // USING_T_SOCKETS


///////////////////////////////////////////////////////////////////////////////

#ifndef NET_CLIENT_ONLY

void ServerBase::ref() { /*getInfra().*/getNetServer().appRef(id); }
void ServerBase::unref() { /*getInfra().*/getNetServer().appUnref(id); }

void ServerBase::close()
{
	/*getInfra().*/getNetServer().appClose(id);
}


void Server::listen(uint16_t port, const char* ip, int backlog)
{
	/*getInfra().*/getNetServer().appListen(this, port, ip, backlog);
}
#if 0 // be back later
void ServerO::listen(uint16_t port, const char* ip, int backlog)
{
//	/*getInfra().*/getNetServer().appListen(this, port, ip, backlog);
}
#endif

#endif // NO_SERVER_STAFF






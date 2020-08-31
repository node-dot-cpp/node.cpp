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

#include "../include/nodecpp/net_common.h"
#include "../include/nodecpp/socket_common.h"
#include "../include/nodecpp/server_common.h"
#include "../include/nodecpp/http_server.h"

#include "infrastructure.h"


using namespace std;
using namespace nodecpp;
using namespace nodecpp::net;

thread_local nodecpp::net::UserHandlerClassPatterns<nodecpp::net::SocketBase::DataForCommandProcessing::UserHandlersForDataCollecting> nodecpp::net::SocketBase::DataForCommandProcessing::userHandlerClassPattern;
thread_local nodecpp::net::UserHandlerClassPatterns<nodecpp::net::ServerBase::DataForCommandProcessing::UserHandlersForDataCollecting> nodecpp::net::ServerBase::DataForCommandProcessing::userHandlerClassPattern;
thread_local nodecpp::net::UserHandlerClassPatterns<nodecpp::net::HttpServerBase::DataForHttpCommandProcessing::UserHandlersForDataCollecting> nodecpp::net::HttpServerBase::DataForHttpCommandProcessing::userHandlerClassPattern;

thread_local NodeBase* thisThreadNode = nullptr;

//SocketBase::SocketBase(NodeBase* node_, OpaqueSocketData& sdata) {node = node_; registerMeAndAssignSocket(-1, sdata);}
//SocketBase::SocketBase(int typeID, NodeBase* node_, OpaqueSocketData& sdata) {node = node_; registerMeAndAssignSocket(typeID, sdata);}

void SocketBase::ref() {netSocketManagerBase->appRef(dataForCommandProcessing.index); }
void SocketBase::unref() { netSocketManagerBase->appUnref(dataForCommandProcessing.index); }
void SocketBase::resume() { netSocketManagerBase->appResume(dataForCommandProcessing.index); }
void SocketBase::pause() { netSocketManagerBase->appPause(dataForCommandProcessing.index); }
void SocketBase::reportBeingDestructed() { netSocketManagerBase->appReportBeingDestructed(dataForCommandProcessing.index); }

void SocketBase::destroy() { OSLayer::appDestroy(dataForCommandProcessing); }
void SocketBase::end() { OSLayer::appEnd(dataForCommandProcessing); }
bool SocketBase::write(const uint8_t* data, uint32_t size)
{
	_bytesWritten += size;
	return netSocketManagerBase->appWrite(dataForCommandProcessing, data, size);
}
bool SocketBase::write2(Buffer& b)
{
	_bytesWritten += b.size();
	return netSocketManagerBase->appWrite2(dataForCommandProcessing, b);
}
void SocketBase::registerMeAndAcquireSocket() {
	nodecpp::soft_ptr<SocketBase> p = myThis.getSoftPtr<SocketBase>(this);
	registerWithInfraAndAcquireSocket(p);
}
void SocketBase::registerMeAndAssignSocket(OpaqueSocketData& sdata) {
	nodecpp::soft_ptr<SocketBase> p = myThis.getSoftPtr<SocketBase>(this);
	registerWithInfraAndAssignSocket(p, sdata);
}

SocketBase& SocketBase::setNoDelay(bool noDelay) { OSLayer::appSetNoDelay(dataForCommandProcessing, noDelay); return *this; }
SocketBase& SocketBase::setKeepAlive(bool enable) { OSLayer::appSetKeepAlive(dataForCommandProcessing, enable); return *this; }

void SocketBase::connect(uint16_t port, const char* ip) {
	dataForCommandProcessing.userHandlers.from(SocketBase::DataForCommandProcessing::userHandlerClassPattern.getPatternForApplying( std::type_index(typeid(*this))), this);
	connectSocket(this, ip, port);
}

///////////////////////////////////////////////////////////////////////////////


ServerBase::ServerBase() {
	nodecpp::soft_ptr<ServerBase> p = myThis.getSoftPtr<ServerBase>(this);
//	registerServer(this->node, p);
}

/*ServerBase::ServerBase(int typeID) {
	nodecpp::soft_ptr<ServerBase> p = myThis.getSoftPtr<ServerBase>(this);
	registerServerByID(this->node, p, typeID);
}*/

/*ServerBase::ServerBase(acceptedSocketCreationRoutineType socketCreationCB) {
	nodecpp::soft_ptr<ServerBase> p = myThis.getSoftPtr<ServerBase>(this);
	acceptedSocketCreationRoutine = std::move( socketCreationCB );
	//	registerServer(this->node, p);
}*/

/*ServerBase::ServerBase(int typeID, acceptedSocketCreationRoutineType socketCreationCB) {
	nodecpp::soft_ptr<ServerBase> p = myThis.getSoftPtr<ServerBase>(this);
	acceptedSocketCreationRoutine = std::move( socketCreationCB );
	registerServerByID(this->node, p, typeID);
}*/

void ServerBase::registerServer(soft_ptr<net::ServerBase> t) { ::registerServer(t); }

void ServerBase::ref() { netServerManagerBase->appRef(dataForCommandProcessing); }
void ServerBase::unref() { netServerManagerBase->appUnref(dataForCommandProcessing); }
void ServerBase::reportBeingDestructed() { netServerManagerBase->appReportBeingDestructed(dataForCommandProcessing); }

void ServerBase::closingProcedure()
{
	netServerManagerBase->appClose(dataForCommandProcessing);
	dataForCommandProcessing.state = DataForCommandProcessing::State::BeingClosed;
	if ( getSockCount() == 0 )
		reportAllAceptedConnectionsEnded(); // posts close event
}

void ServerBase::close() {
#ifndef NODECPP_ENABLE_CLUSTERING
	closingProcedure();
#else
	getCluster().acceptRequestForServerCloseAtSlave(dataForCommandProcessing.index);
#endif // NODECPP_ENABLE_CLUSTERING
}

void ServerBase::reportAllAceptedConnectionsEnded()
{
	netServerManagerBase->appReportAllAceptedConnectionsEnded(dataForCommandProcessing);
}

void ServerBase::listen(uint16_t port, const char* ip, int backlog)
{
	nodecpp::soft_ptr<ServerBase> p = myThis.getSoftPtr<ServerBase>(this);
	dataForCommandProcessing.userHandlers.from(ServerBase::DataForCommandProcessing::userHandlerClassPattern.getPatternForApplying( std::type_index(typeid(*this))), this);
	netServerManagerBase->appListen(dataForCommandProcessing, ip, port, backlog);
}


#ifndef NODECPP_NO_COROUTINES
nodecpp::handler_ret_type nodecpp::a_timeout(uint32_t ms)
{
	co_await ::a_timeout_impl( ms );
	co_return;
}
nodecpp::handler_ret_type nodecpp::a_sleep(uint32_t ms)
{
	co_await ::a_timeout_impl( ms );
	co_return;
}
#endif


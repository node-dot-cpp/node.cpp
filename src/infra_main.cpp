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

#include "../include/nodecpp/common.h"

#ifdef NODECPP_ENABLE_ONSTACK_SOFTPTR_COUNTING
thread_local size_t nodecpp::safememory::onStackSafePtrCreationCount; 
thread_local size_t nodecpp::safememory::onStackSafePtrDestructionCount;
#endif // NODECPP_ENABLE_ONSTACK_SOFTPTR_COUNTING
thread_local void* nodecpp::safememory::thg_stackPtrForMakeOwningCall = 0;

#if defined NODECPP_USE_NEW_DELETE_ALLOC
thread_local void** nodecpp::safememory::zombieList_ = nullptr;
#ifndef NODECPP_DISABLE_ZOMBIE_ACCESS_EARLY_DETECTION
thread_local std::map<uint8_t*, size_t, std::greater<uint8_t*>> nodecpp::safememory::zombieMap;
thread_local bool nodecpp::safememory::doZombieEarlyDetection_ = true;
#endif // NODECPP_DISABLE_ZOMBIE_ACCESS_EARLY_DETECTION
#endif // NODECPP_USE_xxx_ALLOC

class NodeFactoryMap{
	typedef std::basic_string<char, std::char_traits<char>, GlobalObjectAllocator<char>> StringT;
	typedef std::map<StringT, RunnableFactoryBase*, std::less<StringT>, GlobalObjectAllocator<std::pair<const StringT, RunnableFactoryBase*>>> MapT;
	MapT* factoryMap = nullptr;
public:
	static NodeFactoryMap& getInstance(){
		static NodeFactoryMap instance;
		// volatile int dummy{};
		return instance;
	}
	void registerFactory( const char* name, RunnableFactoryBase* factory );
	
	MapT* getFacoryMap() { return factoryMap; }
	
private:
	NodeFactoryMap()= default;
	~NodeFactoryMap()= default;
	NodeFactoryMap(const NodeFactoryMap&)= delete;
	NodeFactoryMap& operator=(const NodeFactoryMap&)= delete;

};

void NodeFactoryMap::registerFactory( const char* name, RunnableFactoryBase* factory )
{
	if ( factoryMap == nullptr )
		factoryMap = new MapT;
	NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, factoryMap != nullptr );
	NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, factoryMap->find( name ) == factoryMap->end() );
	auto insRet NODECPP_UNUSED_VAR = factoryMap->insert( std::make_pair( name, factory ) );
	NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, insRet.second );
}

void registerFactory( const char* name, RunnableFactoryBase* factory )
{
	NodeFactoryMap::getInstance().registerFactory( name, factory );
}

nodecpp::stdvector<nodecpp::stdstring> argv;

#ifndef NODECPP_ENABLE_CLUSTERING

int main( int argc, char *argv_[] )
{
	for ( int i=0; i<argc; ++i )
		argv.push_back( argv_[i] );

#ifdef NODECPP_USE_IIBMALLOC
	g_AllocManager.initialize();
#endif
	nodecpp::log::Log log;
	log.level = nodecpp::log::LogLevel::info;
	log.add( nodecpp::string_literal( "default_log.txt" ) );
	nodecpp::logging_impl::currentLog = &log;

	NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, NodeFactoryMap::getInstance().getFacoryMap()->size() == 1, "Indeed: {}. Current implementation supports exactly 1 node per thread. More nodes is a pending dev", NodeFactoryMap::getInstance().getFacoryMap()->size() );
	for ( auto f : *(NodeFactoryMap::getInstance().getFacoryMap()) )
		f.second->create()->run();

	nodecpp::logging_impl::currentLog = nullptr; // TODO: this is thread-unsafe. Revise and make sure all other threads using nodecpp::logging_impl::currentLog has already exited

	return 0;
}

#else

#include "clustering_impl/clustering_impl.h"

namespace nodecpp {
extern void preinitMasterThreadClusterObject();
extern void preinitSlaveThreadClusterObject(ThreadStartupData& startupData);
}

void workerThreadMain( void* pdata )
{
	ThreadStartupData* sd = reinterpret_cast<ThreadStartupData*>(pdata);
	NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, pdata != nullptr ); 
	NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, sd->threadCommID.slotId != 0 ); 
	ThreadStartupData startupData = *sd;
	nodecpp::stddealloc( sd, 1 );
	setThisThreadDescriptor( startupData );
	addWorkerEntryForLoadTracking( startupData.threadCommID );
#ifdef NODECPP_USE_IIBMALLOC
	g_AllocManager.initialize();
#endif
	nodecpp::logging_impl::currentLog = startupData.defaultLog;
	nodecpp::logging_impl::instanceId = startupData.threadCommID.slotId;
	nodecpp::preinitSlaveThreadClusterObject( startupData );
	nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id),"starting Worker thread with threadID = {}", startupData.threadCommID.slotId );
	NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, NodeFactoryMap::getInstance().getFacoryMap()->size() == 1, "Indeed: {}. Current implementation supports exactly 1 node per thread. More nodes is a pending dev", NodeFactoryMap::getInstance().getFacoryMap()->size() );
	for ( auto f : *(NodeFactoryMap::getInstance().getFacoryMap()) )
		f.second->create()->run(false, &startupData);
}

int main( int argc, char *argv_[] )
{
	for ( int i=0; i<argc; ++i )
		argv.push_back( argv_[i] );

#ifdef NODECPP_USE_IIBMALLOC
	g_AllocManager.initialize();
#endif
	nodecpp::log::Log log;
//	log.level = nodecpp::log::LogLevel::info;
	log.add( stdout );
	nodecpp::logging_impl::currentLog = &log;
	nodecpp::logging_impl::instanceId = 0;

	nodecpp::preinitMasterThreadClusterObject();
	NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, NodeFactoryMap::getInstance().getFacoryMap()->size() == 1, "Indeed: {}. Current implementation supports exactly 1 node per thread. More nodes is a pending dev", NodeFactoryMap::getInstance().getFacoryMap()->size() );
	for ( auto f : *(NodeFactoryMap::getInstance().getFacoryMap()) )
		f.second->create()->run(true, nullptr );

	nodecpp::logging_impl::currentLog = nullptr; // TODO: this is thread-unsafe. Revise and make sure all other threads using nodecpp::logging_impl::currentLog has already exited

	return 0;
}

#endif // NODECPP_ENABLE_CLUSTERING

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

#ifndef NODECPP_ENABLE_CLUSTERING

int main()
{
#ifdef NODECPP_USE_IIBMALLOC
		g_AllocManager.initialize();
#endif
	for ( auto f : *(NodeFactoryMap::getInstance().getFacoryMap()) )
		f.second->create()->run(1);

	return 0;
}

#else

#include "clustering_impl/clustering_common.h"

namespace nodecpp {
extern void initCurrentThreadClusterObject(size_t id);
}

int worker_thread_main( void* pdata )
{
	ThreadStartupData* startupData = reinterpret_cast<ThreadStartupData*>(pdata);
#ifdef NODECPP_USE_IIBMALLOC
		g_AllocManager.initialize();
#endif
	NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, pdata != nullptr ); 
	NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, startupData->assignedThreadID != 0 ); 
	nodecpp::initCurrentThreadClusterObject( startupData->assignedThreadID );
	for ( auto f : *(NodeFactoryMap::getInstance().getFacoryMap()) )
		f.second->create()->run();

	return 0;
}

int main( int argc, char *argv[] )
{
#ifdef NODECPP_USE_IIBMALLOC
		g_AllocManager.initialize();
#endif
	size_t coreCnt = 1;
	if ( argc > 1 )
	{
		if ( strncmp( argv[1], "numcores=", 9 ) == 0 )
			coreCnt = atol(argv[1] + 9);
	}
	nodecpp::initCurrentThreadClusterObject( 0 );
	for ( auto f : *(NodeFactoryMap::getInstance().getFacoryMap()) )
		f.second->create()->run();

	return 0;
}

#endif // NODECPP_ENABLE_CLUSTERING

#include "../../../../include/nodecpp/common.h"
#include "../../../../include/nodecpp/loop.h"

#ifdef USING_T_SOCKETS

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
	NODECPP_ASSERT( factoryMap != nullptr );
	NODECPP_ASSERT( factoryMap->find( name ) == factoryMap->end() );
	auto insRet NODECPP_UNUSED_VAR = factoryMap->insert( std::make_pair( name, factory ) );
	NODECPP_ASSERT( insRet.second );
}

void registerFactory( const char* name, RunnableFactoryBase* factory )
{
	NodeFactoryMap::getInstance().registerFactory( name, factory );
}

extern void runLoop();

int main()
{
#ifdef USING_T_SOCKETS
//	runLoop();
	for ( auto f : *(NodeFactoryMap::getInstance().getFacoryMap()) )
		f.second->create()->run();
#else
	for ( auto f : *(NodeFactoryMap::getInstance().getFacoryMap()) )
		f.second->create()->main();

	nodecpp::runLoop();
#endif

	return 0;
}


#else

class NodeFactoryMap{
	typedef std::basic_string<char, std::char_traits<char>, GlobalObjectAllocator<char>> StringT;
	typedef std::map<StringT, NodeFactoryBase*, std::less<StringT>, GlobalObjectAllocator<std::pair<const StringT, NodeFactoryBase*>>> MapT;
	MapT* factoryMap = nullptr;
public:
	static NodeFactoryMap& getInstance(){
		static NodeFactoryMap instance;
		// volatile int dummy{};
		return instance;
	}
	void registerFactory( const char* name, NodeFactoryBase* factory );
	
	MapT* getFacoryMap() { return factoryMap; }
	
private:
	NodeFactoryMap()= default;
	~NodeFactoryMap()= default;
	NodeFactoryMap(const NodeFactoryMap&)= delete;
	NodeFactoryMap& operator=(const NodeFactoryMap&)= delete;

};

void NodeFactoryMap::registerFactory( const char* name, NodeFactoryBase* factory )
{
	if ( factoryMap == nullptr )
		factoryMap = new MapT;
	NODECPP_ASSERT( factoryMap != nullptr );
	NODECPP_ASSERT( factoryMap->find( name ) == factoryMap->end() );
	auto insRet NODECPP_UNUSED_VAR = factoryMap->insert( std::make_pair( name, factory ) );
	NODECPP_ASSERT( insRet.second );
}

void registerFactory( const char* name, NodeFactoryBase* factory )
{
	NodeFactoryMap::getInstance().registerFactory( name, factory );
}

extern void runLoop();

int main()
{
//	for ( auto f : *(NodeFactoryMap::getInstance().getFacoryMap()) )
//		f.second->create()->main();

//	nodecpp::runLoop();
	runLoop();

	return 0;
}

#endif USING_T_SOCKETS
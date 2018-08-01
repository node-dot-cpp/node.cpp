#include "../../../include/nodecpp/common.h"
#include <map>
#include <string>
#include <vector>
#include "socket.h"

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
	assert( factoryMap != nullptr );
	assert( factoryMap->find( name ) == factoryMap->end() );
	auto insRet = factoryMap->insert( std::make_pair( name, factory ) );
	assert( insRet.second );
}

void registerFactory( const char* name, NodeFactoryBase* factory )
{
	NodeFactoryMap::getInstance().registerFactory( name, factory );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////


class SocketVector{
	typedef std::vector<SocketO*, GlobalObjectAllocator<SocketO*>> VectorT;
	VectorT* sv = nullptr;
public:
	static SocketVector& getInstance(){
		static SocketVector instance;
		// volatile int dummy{};
		return instance;
	}
	void registerSocket( SocketO* sock );
	
	VectorT* getSockets() { return sv; }
	
private:
	SocketVector()= default;
	~SocketVector()= default;
	SocketVector(const SocketVector&)= delete;
	SocketVector& operator=(const SocketVector&)= delete;

};

void SocketVector::registerSocket( SocketO* sock )
{
	if ( sv == nullptr )
		sv = new VectorT;
	assert( sv != nullptr );
	sv->push_back( sock );
}

void registerSocket( SocketO* sock )
{
	SocketVector::getInstance().registerSocket( sock );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

int main()
{
	printf( "main()\n" );

	for ( auto f : *(NodeFactoryMap::getInstance().getFacoryMap()) )
		printf( "    - \"%s\"\n", f.first.c_str() );

	for ( auto f : *(NodeFactoryMap::getInstance().getFacoryMap()) )
		f.second->create()->main();

	for ( auto f : *(SocketVector::getInstance().getSockets()) )
		f->onConnect();

	for ( auto f : *(SocketVector::getInstance().getSockets()) )
		f->onClose(true);

    return 0;
}


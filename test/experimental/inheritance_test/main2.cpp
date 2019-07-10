#include "common.h"
#include <string>

class NodeFactoryHolder{
	typedef std::basic_string<char, std::char_traits<char>, GlobalObjectAllocator<char>> StringT;
	StringT fname;
	NodeFactoryBase* factory = nullptr;
public:
	static NodeFactoryHolder& getInstance(){
		static NodeFactoryHolder instance;
		// volatile int dummy{};
		return instance;
	}
	void registerFactory( const char* name, NodeFactoryBase* factory );
	
	NodeFactoryBase* getFactory() { return factory; }
	const char* getName() const { return fname.c_str(); }
	
private:
	NodeFactoryHolder()= default;
	~NodeFactoryHolder()= default;
	NodeFactoryHolder(const NodeFactoryHolder&)= delete;
	NodeFactoryHolder& operator=(const NodeFactoryHolder&)= delete;

};

void NodeFactoryHolder::registerFactory( const char* name, NodeFactoryBase* factory_ )
{
	assert( factory == nullptr );
	if ( factory == nullptr )
	{
		assert( fname.size() == 0 );
		factory = factory_;
		fname = name;
	}
}

void registerFactory( const char* name, NodeFactoryBase* factory )
{
	NodeFactoryHolder::getInstance().registerFactory( name, factory );
}

int main()
{
	printf( "main()\n" );

	printf( "    - \"%s\"\n", NodeFactoryHolder::getInstance().getName() );

	auto nodeObj = NodeFactoryHolder::getInstance().getFactory()->create();

	return 0;
}


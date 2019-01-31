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
#ifndef COMMON_H
#define COMMON_H

#ifdef _MSC_VER
//#pragma warning (disable:4800) // forcing value to bool 'true' or 'false' (performance warning)
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#define NOMINMAX
#endif // _MSC_VER


#ifdef __GNUC__
#define NODECPP_UNUSED_VAR [[gnu::unused]]
#else
#define NODECPP_UNUSED_VAR 
#endif

#include <foundation.h>
#include <nodecpp_assert.h>
#include <iibmalloc.h>
#include <safe_ptr.h>

#include <utility>

#include <vector>
#include <map>
#include <list>
#include <array>
#include <algorithm>

//#include <cassert>
#include <fmt/format.h>
//#include "trace.h"
//#include "assert.h"
#include "mallocator.h"

namespace nodecpp {
	constexpr uint64_t module_id = 4;
} // namespace ::nodecpp

using namespace ::nodecpp::safememory;

template<class T>
using GlobalObjectAllocator = Mallocator<T>;

class NodeBase
{
public:
	NodeBase() {}
	virtual void main() = 0;

	using EmitterType = void;
	using EmitterTypeForServer = void;
};

#ifdef USING_T_SOCKETS

class RunnableBase
{
public:
	RunnableBase() {}
	virtual void run() = 0;
};

class RunnableFactoryBase
{
public:
	RunnableFactoryBase() {}
	virtual RunnableBase* create() = 0;
};

void registerFactory( const char* name, RunnableFactoryBase* factory );

//template<class RunnableT>
template<class RunnableT>
class NodeRegistrator
{
public:
//	thread_local Infrastructure<typename RunnableT::NodeType::EmitterType>* infraPtr;
//	static thread_local Infra* infraPtr;

public:
	NodeRegistrator( const char* name )
	{
		//nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>(( "NodeRegistrator(\"{}\");", name );
		class NodeFactory : public RunnableFactoryBase
		{
		public:
			NodeFactory() {}
			virtual RunnableBase* create() override
			{
				return new RunnableT;
			}
		};
		NodeFactory* factory = new NodeFactory;
		registerFactory( name, reinterpret_cast<NodeFactory*>(factory) );
	}
};

/*template<class RunnableT,class Infra>
thread_local Infra* NodeRegistrator<RunnableT,Infra>::infraPtr;*/


#else

class NodeFactoryBase
{
public:
	NodeFactoryBase() {}
	virtual NodeBase* create() = 0;
};

void registerFactory( const char* name, NodeFactoryBase* factory );

template<class NodeT>
class NodeRegistrator
{
public:
	NodeRegistrator( const char* name )
	{
		//nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>(( "NodeRegistrator(\"{}\");", name );
		class NodeFactory : public NodeFactoryBase
		{
		public:
			NodeFactory() {}
			virtual NodeBase* create() override
			{
				return new NodeT;
			}
		};
		NodeFactory* factory = new NodeFactory;
		registerFactory( name, reinterpret_cast<NodeFactory*>(factory) );
	}
};

#endif // USING_T_SOCKETS

#endif //COMMON_H
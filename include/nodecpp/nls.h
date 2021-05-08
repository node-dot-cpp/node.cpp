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

#ifndef NLS_H
#define NLS_H

#include "common.h"
#include <typeinfo>
#include <typeindex>
#ifdef NODECPP_USE_GMQUEUE
#include <gmqueue.h>
#endif
#include <nodecpp/record_and_replay.h>

namespace nodecpp {

#ifdef NODECPP_ENABLE_JS_COMPATIBILITY_LAYER
	namespace js {
		class JSModule
		{
		public:
			JSModule() {}
			JSModule( const JSModule& ) = delete; // note: modules are not copied but only referenced. In particular, use auto& to assign ret value of required<>()
			JSModule& operator = ( const JSModule& ) = delete;
			virtual ~JSModule() {}
		};

		class JSArray;

		class JSModuleMap
		{
			using MapType = ::nodecpp::map<std::type_index, nodecpp::owning_ptr<js::JSModule>>;
	#ifndef NODECPP_THREADLOCAL_INIT_BUG_GCC_60702
			MapType _classModuleMap;
			MapType& classModuleMap() { return _classModuleMap; }
	#else
			uint8_t mapbytes[sizeof(MapType)];
			MapType& classModuleMap() { return *reinterpret_cast<MapType*>(mapbytes); }
	#endif
			template<class UserClass>
			std::pair<bool, nodecpp::soft_ptr<UserClass>> getJsModuleExported_( std::type_index idx )
			{
				auto pattern = classModuleMap().find( idx );
				if ( pattern != classModuleMap().end() )
				{
					nodecpp::soft_ptr<js::JSModule> svar0 = pattern->second;
					nodecpp::soft_ptr<UserClass> svar = nodecpp::soft_ptr_static_cast<UserClass>(svar0);
					return std::make_pair( true, svar );
				}
				else
					return std::make_pair( false, nodecpp::soft_ptr<UserClass>() );
			}
			template<class UserClass>
			std::pair<bool, nodecpp::soft_ptr<UserClass>> addJsModuleExported_( std::type_index idx, nodecpp::owning_ptr<js::JSModule>&& pvar )
			{
				auto check = classModuleMap().insert( std::make_pair( idx, std::move( pvar ) ) );
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, check.second, "failed to insert exported value to map; insertion already done for this type" ); 
	//			nodecpp::soft_ptr<UserClass> svar = check.first->second;
				nodecpp::soft_ptr<js::JSModule> svar0 = check.first->second;
				nodecpp::soft_ptr<UserClass> svar = nodecpp::soft_ptr_static_cast<UserClass>(svar0);
				return std::make_pair( true, svar );
			}
		public:
	#ifdef NODECPP_THREADLOCAL_INIT_BUG_GCC_60702
			void init()
			{
				new(&(classModuleMap()))MapType();
			}
			void destroy()
			{
				classModuleMap().~MapType();
			}
	#endif
			template<class UserClass>
			std::pair<bool, nodecpp::soft_ptr<UserClass>> getJsModuleExported()
			{
				return getJsModuleExported_<UserClass>( std::type_index(typeid(UserClass)) );
			}
			template<class UserClass>
			std::pair<bool, nodecpp::soft_ptr<UserClass>> addJsModuleExported( nodecpp::owning_ptr<js::JSModule>&& pvar )
			{
				return addJsModuleExported_<UserClass>( std::type_index(typeid(UserClass)), std::move( pvar ) );
			}
		};

		class LCG
		{
			// quick and dirty implementation
			// TODO: revise!
			static constexpr uint64_t a = 500713;
			static constexpr uint64_t c = 138041;
			uint64_t s;
	
		public:
			LCG() { s = 0x12345; }
			void seed( uint64_t seed_ ) { s = seed_; }
			uint32_t rnd() 
			{
				s = ( a * s + c );
				return (uint32_t)(s >> 16);
			}
			double normalizedRnd()
			{
				s = ( a * s + c );
				return ((uint32_t)(s >> 16)) / ((double)(((uint64_t)(1))<<32));
			}
		};

	} // namespace js
#endif // NODECPP_ENABLE_JS_COMPATIBILITY_LAYER

	struct NLS
	{
#ifdef NODECPP_RECORD_AND_REPLAY
		record_and_replay_impl::BinaryLog* binaryLog = nullptr;
#endif // NODECPP_RECORD_AND_REPLAY

#ifdef NODECPP_USE_GMQUEUE
	globalmq::marshalling::GMQTransportBase<GMQueueStatePublisherSubscriberTypeInfo>* transport;
#endif

#ifdef NODECPP_ENABLE_JS_COMPATIBILITY_LAYER
		js::JSModuleMap jsModuleMap;
		nodecpp::soft_ptr<nodecpp::js::JSArray> currentArgs;
		js::LCG rng;

#ifdef NODECPP_THREADLOCAL_INIT_BUG_GCC_60702
		void init()
		{
			jsModuleMap.init();
		}
		void destroy()
		{
			jsModuleMap.destroy();
		}
#endif // NODECPP_THREADLOCAL_INIT_BUG_GCC_60702

#else // NODECPP_ENABLE_JS_COMPATIBILITY_LAYER

#ifdef NODECPP_THREADLOCAL_INIT_BUG_GCC_60702
		void init() {}
		void destroy() {}
#endif // NODECPP_THREADLOCAL_INIT_BUG_GCC_60702

#endif // NODECPP_ENABLE_JS_COMPATIBILITY_LAYER
	};

//	extern thread_local NLS threadLocalData;
	extern thread_local void* nodeLocalData;

} // namespace nodecpp

#endif // NLS_H

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

namespace nodecpp {

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
			using MapType = nodecpp::map<std::type_index, owning_ptr<js::JSModule>>;
	#ifndef NODECPP_THREADLOCAL_INIT_BUG_GCC_60702
			MapType _classModuleMap;
			MapType& classModuleMap() { return _classModuleMap; }
	#else
			uint8_t mapbytes[sizeof(MapType)];
			MapType& classModuleMap() { return *reinterpret_cast<MapType*>(mapbytes); }
	#endif
			template<class UserClass>
			std::pair<bool, nodecpp::safememory::soft_ptr<UserClass>> getJsModuleExported_( std::type_index idx )
			{
				auto pattern = classModuleMap().find( idx );
				if ( pattern != classModuleMap().end() )
				{
					nodecpp::safememory::soft_ptr<js::JSModule> svar0 = pattern->second;
					nodecpp::safememory::soft_ptr<UserClass> svar = soft_ptr_static_cast<UserClass>(svar0);
					return std::make_pair( true, svar );
				}
				else
					return std::make_pair( false, nodecpp::safememory::soft_ptr<UserClass>() );
			}
			template<class UserClass>
			std::pair<bool, nodecpp::safememory::soft_ptr<UserClass>> addJsModuleExported_( std::type_index idx, nodecpp::safememory::owning_ptr<js::JSModule>&& pvar )
			{
				auto check = classModuleMap().insert( std::make_pair( idx, std::move( pvar ) ) );
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, check.second, "failed to insert exported value to map; insertion already done for this type" ); 
	//			nodecpp::safememory::soft_ptr<UserClass> svar = check.first->second;
				nodecpp::safememory::soft_ptr<js::JSModule> svar0 = check.first->second;
				nodecpp::safememory::soft_ptr<UserClass> svar = soft_ptr_static_cast<UserClass>(svar0);
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
			std::pair<bool, nodecpp::safememory::soft_ptr<UserClass>> getJsModuleExported()
			{
				return getJsModuleExported_<UserClass>( std::type_index(typeid(UserClass)) );
			}
			template<class UserClass>
			std::pair<bool, nodecpp::safememory::soft_ptr<UserClass>> addJsModuleExported( nodecpp::safememory::owning_ptr<js::JSModule>&& pvar )
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

		class JSConsole
		{
			template<class VarT1, class ... VarTX>
			void _stringify4logging( nodecpp::string& out, const VarT1& var1, const VarTX& ... args )
			{
				out += var1.toString();
				_stringify4logging( out, args... );
			}

			template<class VarT1>
			void _stringify4logging( nodecpp::string& out, const VarT1& var1 )
			{
				out += var1.toString();
			}

		public:

			template<class ... VarTX>
			void log( const VarTX& ... args )
			{
				// TODO: incrementing counters, etc
				nodecpp::string out;
				_stringify4logging( out, args... );
				nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "{}", out );
			}

			template<class ... VarTX>
			void log( nodecpp::string f, const VarTX& ... args )
			{
				nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "{}", nodecpp::format( f.c_str(), args... ) );
			}

			template<class ... VarTX>
			void log( const char* f, const VarTX& ... args )
			{
				nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "{}", nodecpp::format( f, args... ) );
			}

			template<class ... VarTX>
			void error( const VarTX& ... args )
			{
				// TODO: revise (printing to stderr)
				nodecpp::string out;
				_stringify4logging( out, args... );
				nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "{}", out );
			}

			template<class ... VarTX>
			void error( nodecpp::string f, const VarTX& ... args )
			{
				// TODO: revise (printing to stderr)
				nodecpp::log::default_log::error( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "{}", nodecpp::format( f.c_str(), args... ) );
			}

			template<class ... VarTX>
			void error( const char* f, const VarTX& ... args )
			{
				// TODO: revise (printing to stderr)
				nodecpp::log::default_log::error( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "{}", nodecpp::format( f, args... ) );
			}

			template<class ... VarTX>
			void assert( bool cond, const VarTX& ... args )
			{
				if ( cond )
					return;
				// TODO: revise (printing to stderr)
				nodecpp::string out;
				_stringify4logging( out, args... );
				nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "{}", out );
			}

			template<class ... VarTX>
			void assert( bool cond, nodecpp::string f, const VarTX& ... args )
			{
				if (!cond)
					nodecpp::log::default_log::error( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "{}", nodecpp::format( f.c_str(), args... ) );
			}

			template<class ... VarTX>
			void assert( bool cond, const char* f, const VarTX& ... args )
			{
				if (!cond)
					nodecpp::log::default_log::error( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "{}", nodecpp::format( f, args... ) );
			}

		};
	} // namespace js

	struct NLS
	{
		js::JSModuleMap jsModuleMap;
		nodecpp::safememory::soft_ptr<nodecpp::js::JSArray> currentArgs;
		js::LCG rng;
		js::JSConsole console;

#ifdef NODECPP_THREADLOCAL_INIT_BUG_GCC_60702
		void init()
		{
			jsModuleMap.init();
		}
		void destroy()
		{
			jsModuleMap.destroy();
		}
#endif
	};

	extern thread_local NLS threadLocalData;

	namespace js {
		inline
		JSConsole& console() { return threadLocalData.console; }
	} // namespace js
} // namespace nodecpp

#endif // NLS_H

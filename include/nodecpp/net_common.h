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

#ifndef NET_COMMON_H
#define NET_COMMON_H

#include "common.h"
#include "ip_and_port.h"
#include "timers.h"
#include "common_structs.h"
namespace nodecpp {

	class CircularByteBuffer
	{
		std::unique_ptr<uint8_t[]> buff; // TODO: switch to using safe memory objects ASAP
		size_t max_allowed_size_exp = 32;
		size_t size_exp;
		uint8_t* begin = nullptr;
		uint8_t* end = nullptr;

		bool resize_up_and_append( const uint8_t* data, size_t data_size) {
			// TODO: introduce upper limit and make this call bool
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, buff != nullptr );
			size_t total_sz = used_size() + data_size;
			size_t new_size_exp = size_exp + 1;
			while ( (((size_t)1) << new_size_exp) < total_sz + 1 )
			{
				// TODO: add upper bound control
				++new_size_exp;
				if ( new_size_exp > max_allowed_size_exp )
					return false;
			}
			size_t new_alloc_size = ((size_t)1) << new_size_exp;
			std::unique_ptr<uint8_t[]> new_buff( new uint8_t[new_alloc_size] );
			size_t sz = 0;
			if ( begin <= end )
			{
				sz = end - begin;
				memcpy( new_buff.get(), begin, sz );
			}
			else
			{
				sz = buff.get() + alloc_size() - begin;
				memcpy( new_buff.get(), begin, sz );
				memcpy( new_buff.get() + sz, buff.get(), end - buff.get() );
				sz += end - buff.get();
			}
			buff = std::move( new_buff );
			size_exp = new_size_exp;
			begin = buff.get();
			end = begin + sz;

			memcpy( end, data, data_size );
			end += data_size;

			return true;
		}

	public:
		CircularByteBuffer(size_t sz_exp = 16) { 
			size_exp = sz_exp; 
			buff = std::unique_ptr<uint8_t[]>(new uint8_t[alloc_size()]);
			begin = end = buff.get();
		}
		CircularByteBuffer( const CircularByteBuffer& ) = delete;
		CircularByteBuffer& operator = ( const CircularByteBuffer& ) = delete;
		CircularByteBuffer( CircularByteBuffer&& other ) : buff( std::move( other.buff ) ) {
			size_exp = other.size_exp;
			begin = other.begin;
			end = other.end;
		}
		CircularByteBuffer& operator = ( CircularByteBuffer&& other ) {
			size_exp = other.size_exp;
			begin = other.begin;
			end = other.end;
			return *this;
		}
		size_t used_size() const { return begin <= end ? end - begin : alloc_size() - (begin - end); }
		size_t remaining_capacity() const { return alloc_size() - 1 - used_size(); }
		bool empty() const { return begin == end; }
		size_t alloc_size() const { return ((size_t)1)<<size_exp; }

		// writer-related
		bool append( const uint8_t* ptr, size_t sz ) { 
			if ( sz > remaining_capacity() )
			{
				//return false;
				return resize_up_and_append( ptr, sz );
			}

			size_t fwd_free_sz = buff.get() + alloc_size() - end;
			if ( sz <= fwd_free_sz )
			{
				memcpy( end,  ptr, sz );
				end += sz;
				if ( buff.get() + alloc_size() == end )
					end = buff.get();
			}
			else
			{
				memcpy( end,  ptr, fwd_free_sz );
				memcpy( buff.get(),  ptr + fwd_free_sz, sz - fwd_free_sz );
				end = buff.get() + sz - fwd_free_sz;
			}
			return true; 
		}

		template<class Writer>
		void write( Writer& writer, size_t& bytesWritten ) {
			bytesWritten = 0;
			if ( begin < end )
			{
				writer.write( begin, end - begin, bytesWritten );
				begin += bytesWritten;
			}
			else if ( begin > end )
			{
				size_t sz2write = buff.get() + alloc_size() - begin;
				bool can_continue = writer.write( end, sz2write, bytesWritten );
				begin += bytesWritten;
				bool till_end = begin == (buff.get() + alloc_size());
				if( till_end )
					begin = buff.get();
				if (!can_continue || !till_end)
					return;
				NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, begin == buff.get() );
				if ( begin != end )
				{
					size_t bw = 0;
					writer.write( begin, end - begin, bw );
					begin += bw;
					bytesWritten += bw;
				}
			}
		}

		// reader-related
		/*bool get_ready_data( Buffer& b, size_t minsz ) {
			if ( used_size() < minsz )
				return false;
			get_ready_data( b );
			return true;
		}*/
		void get_ready_data( Buffer& b ) {
			get_ready_data( b, b.capacity() );
		}

		CoroStandardOutcomes read_ready_data_until_impl( Buffer& b, uint8_t what, uint8_t* workingEnd )
		{

			if ( begin == workingEnd )
				return CoroStandardOutcomes::in_progress;

			uint8_t curr;
			while ( begin < workingEnd && *begin != what && b.capacity() > b.size() )
			{
				curr = *begin++;
				b.appendUint8( curr );
			}
			if ( begin < workingEnd && *begin == what )
			{
				b.appendUint8( *begin++ );
				return CoroStandardOutcomes::ok;
			}
			else if ( b.capacity() == b.size() )
				return CoroStandardOutcomes::insufficient_buffer;
			else
			{
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::pedantic, begin == workingEnd );
				return CoroStandardOutcomes::in_progress;
			}
		}

		CoroStandardOutcomes read_ready_data_until( Buffer& b, uint8_t what )
		{
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, b.size() == 0 );
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, b.capacity() != 0 );

			if ( begin == end )
				return CoroStandardOutcomes::in_progress;
			if ( begin < end )
			{
				return read_ready_data_until_impl( b, what, end );
			}
			else
			{
				auto firstRet = read_ready_data_until_impl( b, what, buff.get() + alloc_size() );
				if ( begin == buff.get() + alloc_size() )
					begin = buff.get();
				if ( firstRet == CoroStandardOutcomes::ok || firstRet == CoroStandardOutcomes::insufficient_buffer )
					return firstRet;
				return read_ready_data_until_impl( b, what, end );
			}
		}

		void get_ready_data( Buffer& b, size_t bytes2read ) {
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, b.size() == 0 );
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, bytes2read <= b.capacity(), "indeed: {} vs. {}", bytes2read, b.capacity() );

			if ( begin <= end )
			{
				size_t diff = (size_t)(end - begin);
				size_t sz2copy = bytes2read >= diff ? diff : bytes2read;
				b.append( begin, sz2copy );
				begin += sz2copy;
			}
			else
			{
				size_t sz2copy = buff.get() + alloc_size() - begin;
				if ( sz2copy > bytes2read )
				{
					b.append( begin, bytes2read );
					begin += bytes2read;
					NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::pedantic, begin < buff.get() + alloc_size() );
				}
				else if ( sz2copy < bytes2read )
				{
					b.append( begin, sz2copy );
					NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::pedantic, begin + sz2copy == buff.get() + alloc_size() );
					begin = buff.get();
					size_t sz2copy2 = bytes2read - sz2copy;
					NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::pedantic, begin <= end );
					size_t diff = (size_t)(end - begin);
					if ( sz2copy2 > diff )
						sz2copy2 = diff;
					b.append( begin, sz2copy2 );
					begin += sz2copy2;
					NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::pedantic, begin <= end );
				}
				else
				{
					b.append( begin, sz2copy );
					begin = buff.get();
				}
			}
		}

		template<class Reader>
		void read( Reader& reader, size_t& bytesRead, size_t target_sz ) {
			bytesRead = 0;
			if ( begin > end )
			{
				reader.read( end, begin - end - 1, bytesRead );
				end += bytesRead;
			}
			else
			{
				uint8_t* endpoint = begin != buff.get() ? buff.get() + alloc_size() : buff.get() + alloc_size() - 1;
				size_t sz2read = endpoint - end;
				bool can_continue = reader.read( end, sz2read, bytesRead );
				end += bytesRead;
				bool till_end = end == (buff.get() + alloc_size());
				if( till_end )
					end = buff.get();
				if (!can_continue || !till_end )
					return;
				NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, end == buff.get() );
				if ( buff.get() + alloc_size() >= begin + target_sz )
					return;
				if ( begin - end > 1 )
				{
					size_t br = 0;
					reader.read( end, begin - end - 1, br );
					end += br;
					bytesRead += br;
				}
			}
		}

		std::pair<bool, uint8_t> try_read_byte()
		{
			if (!empty())
			{
				auto ret = std::make_pair( true, *begin );
				++begin;
				if ( begin != buff.get() + alloc_size() )
					return ret;
				begin = buff.get();
				return ret;
			}
			else
				return std::make_pair(false, 0 );
		}

		uint8_t read_byte()
		{
			NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::pedantic, begin != end );
			auto ret = *begin;
			++begin;
			if ( begin != buff.get() + alloc_size() )
				return ret;
			begin = buff.get();
			return ret;
		}
	};


	namespace net {


		struct Address {
			uint16_t port;
			nodecpp::IPFAMILY family;
			Ip4 ip;

			bool operator == ( const Address& other ) const { return port == other.port && ip == other.ip && family == other.family; }
		};

//		enum Mode { callable, awaitable };

		template<class ObjectT, auto memberFunc>
		struct HandlerData
		{
			using ObjT = ObjectT;
			static constexpr auto memberFn = memberFunc;
		};

		template<class FnT>
		struct UserDefHandlersBase
		{
			struct HandlerInstance
			{
				FnT handler = nullptr;
				void *object = nullptr;
			};
			nodecpp::vector<HandlerInstance> handlers;

			bool willHandle() { return handlers.size(); }
			void from(const UserDefHandlersBase<FnT>& patternUH, void* defaultObjPtr)
			{
				NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, handlers.size() == 0);
				handlers = patternUH.handlers;
				for (size_t i = 0; i < handlers.size(); ++i)
					if (handlers[i].object == nullptr)
						handlers[i].object = defaultObjPtr;
			}
		};

		template<class FnT>
		struct UserDefHandlersWithOptimizedStorage
		{
			struct HandlerInstance
			{
				FnT handler = nullptr;
				void *object = nullptr;
			};
			using HandlerInstanceVectorT = nodecpp::vector<HandlerInstance>;
			enum Type { uninitialized, zero, one, two, many };
			Type type = Type::uninitialized;
			static constexpr size_t fixed_size = 2;
			uint8_t basebytes[ sizeof( HandlerInstanceVectorT ) > sizeof( HandlerInstance ) * fixed_size ? sizeof( HandlerInstanceVectorT ) : sizeof( HandlerInstance ) * fixed_size ];
			HandlerInstance* handlers_a() { return reinterpret_cast<HandlerInstance*>(basebytes); }
			HandlerInstanceVectorT& handlers_v() { return *reinterpret_cast<HandlerInstanceVectorT*>(basebytes); }


			bool willHandle() { 
				return type != Type::uninitialized && type != Type::zero; 
			}
			void from(const UserDefHandlersBase<FnT>& patternUH, void* defaultObjPtr)
			{
				NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, type == Type::uninitialized);
				if ( patternUH.handlers.size() == 0 )
				{
					type = Type::zero;
					return;
				}
				else if ( patternUH.handlers.size() == 1 )
				{
					static_assert( fixed_size >= 1 );
					type = Type::one;
					handlers_a()[0].handler = patternUH.handlers[0].handler;
					handlers_a()[0].object = patternUH.handlers[0].object != nullptr ? patternUH.handlers[0].object : defaultObjPtr;
					return;
				}
				else if ( patternUH.handlers.size() == 2 )
				{
					static_assert( fixed_size >= 2 );
					type = Type::two;
					handlers_a()[0].handler = patternUH.handlers[0].handler;
					handlers_a()[0].object = patternUH.handlers[0].object != nullptr ? patternUH.handlers[0].object : defaultObjPtr;
					handlers_a()[1].handler = patternUH.handlers[1].handler;
					handlers_a()[1].object = patternUH.handlers[1].object != nullptr ? patternUH.handlers[1].object : defaultObjPtr;
					return;
				}
				else
				{
					static_assert( fixed_size <= 2 );
					type = Type::many;
					new(basebytes)HandlerInstanceVectorT();
					handlers_v().resize( patternUH.handlers.size() );
					for (size_t i = 0; i < handlers_v().size(); ++i)
					{
						handlers_v()[i].handler = patternUH.handlers[i].handler;
						handlers_v()[i].object = patternUH.handlers[i].object != nullptr ? patternUH.handlers[i].object : defaultObjPtr;
					}
				}
			}
			template<class ... ARGS>
			void execute(ARGS&& ... args) { 
				NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, type != Type::uninitialized );
				NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, type != Type::zero );
				if ( type == Type::one )
				{
					NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, handlers_a()[0].object != nullptr ); 
					handlers_a()[0].handler(handlers_a()[0].object, ::std::forward<ARGS>(args)...);
				}
				else if ( type == Type::two )
				{
					NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, handlers_a()[0].object != nullptr ); 
					handlers_a()[0].handler(handlers_a()[0].object, ::std::forward<ARGS>(args)...);
					NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, handlers_a()[1].object != nullptr ); 
					handlers_a()[1].handler(handlers_a()[1].object, ::std::forward<ARGS>(args)...);
				}
				else
				{
					NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, type == Type::many );
					for (auto h : handlers_v()) 
					{
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, h.object != nullptr ); 
						h.handler(h.object, ::std::forward<ARGS>(args)...);
					}
				}
			}
			~UserDefHandlersWithOptimizedStorage() {
				if ( type == Type::many ) {
					using myvectort = HandlerInstanceVectorT;
					handlers_v().~myvectort();
				}
			}
		};

		template<class FnT>
		struct UserDefHandlers : public UserDefHandlersBase<FnT>
		{
			template<class ObjectT>
			void add( ObjectT* object, FnT handler )
			{
				for (auto h : this->handlers)
					NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, h.handler != handler || h.object != object, "already added" );
				typename UserDefHandlersBase<FnT>::HandlerInstance inst;
				inst.object = object;
				inst.handler = handler;
				this->handlers.push_back(inst);
			}
			void add(FnT handler)
			{
				for (auto h : this->handlers)
					NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, h.handler != handler || h.object != nullptr, "already added");
				typename UserDefHandlersBase<FnT>::HandlerInstance inst;
				inst.object = nullptr;
				inst.handler = handler;
				this->handlers.push_back(inst);
			}
			template<class ObjectT>
			void remove(ObjectT* object, FnT handler)
			{
				for (size_t i=0; i<this->handlers.size(); ++i)
					if (this->handlers[i].handler == handler && this->handlers[i].object == object)
					{
						this->handlers.erase(this->handlers.begin() + i);
						return;
					}
			}
			void remove(FnT handler)
			{
				for (size_t i = 0; i < this->handlers.size(); ++i)
					if (this->handlers[i].handler == handler && this->handlers[i].object == nullptr)
					{
						this->handlers.erase(this->handlers.begin() + i);
						return;
					}
			}
		};

		template<class UserHandlerType>
		class UserHandlerClassPatterns
		{
			using MapType = nodecpp::map<std::type_index, std::pair<UserHandlerType, bool>>; 
#ifndef NODECPP_THREADLOCAL_INIT_BUG_GCC_60702
			MapType _patterns;
			MapType& patterns() { return _patterns; }
#else
			uint8_t mapbytes[sizeof(MapType)];
			MapType& patterns() { return *reinterpret_cast<MapType*>(mapbytes); }
#endif
			std::pair<UserHandlerType, bool>& getPattern( std::type_index idx )
			{
				auto pattern = patterns().find( idx );
				if ( pattern != patterns().end() )
					return pattern->second;
				else
				{
					auto ins = patterns().insert( make_pair( idx, std::make_pair(UserHandlerType(), false) ) );
					return ins.first->second;
				}
			}
		public:
#ifdef NODECPP_THREADLOCAL_INIT_BUG_GCC_60702
			void init()
			{
				new(&(patterns()))MapType();
			}
			void destroy()
			{
				patterns().~MapType();
			}
#endif
			template<class UserClass>
			UserHandlerType& getPatternForUpdate()
			{
				auto& ret = getPattern( std::type_index(typeid(UserClass)) );
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, !ret.second, "handlers must be added before any instance is created" );
				return ret.first;
			}
			template<class UserClass>
			const UserHandlerType& getPatternForApplying()
			{
				auto& ret = getPattern( std::type_index(typeid(UserClass)) );
				ret.second = true;
				return ret.first;
			}
			const UserHandlerType& getPatternForApplying(std::type_index idx)
			{
				auto& ret = getPattern( idx );
				ret.second = true;
				return ret.first;
			}
		};
	} //namespace net

} // namespace nodecpp

#endif // NET_COMMON_H

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

#ifndef EVENT_H
#define EVENT_H

#include <functional>
#include <vector>

#include "_error.h"
#include "net_common.h"

namespace nodecpp
{
	class Buffer;
	namespace net { class SocketBase; class IncomingHttpMessageAtServer; class HttpServerResponse; }


	struct event
	{
		struct Close
		{
			using callback = std::function<void(bool)>;
			static constexpr const char* name = "close";
		};
		static constexpr Close close = Close();

		struct Connect
		{
			using callback = std::function<void()>;
			static constexpr const char* name = "connect";
		};
		static constexpr Connect connect = Connect();

		struct Data
		{
			using callback = std::function<void(const Buffer&)>;
			static constexpr const char* name = "data";
		};
		static constexpr Data data = Data();

		struct Drain
		{
			using callback = std::function<void()>;
			static constexpr const char* name = "drain";
		};
		static constexpr Drain drain = Drain();

		struct End
		{
			using callback = std::function<void()>;
			static constexpr const char* name = "end";
		};
		static constexpr End end = End();

		struct Accepted
		{
			using callback = std::function<void()>;
			static constexpr const char* name = "accepted";
		};
		static constexpr Accepted accepted = Accepted();

		struct Error
		{
			using callback = std::function<void(nodecpp::Error&)>;
			static constexpr const char* name = "error";
		};
		static constexpr Error error = Error();


		struct Connection
		{
			using callback = std::function<void(soft_ptr<net::SocketBase>)>;
			static constexpr const char* name = "connection";
		};
		static constexpr Connection connection = Connection();

		struct Listening
		{
			using callback = std::function<void(size_t, net::Address)>;
			static constexpr const char* name = "listening";
		};
		static constexpr Listening listening = Listening();

		struct HttpRequest
		{
			using callback = std::function<void(nodecpp::net::IncomingHttpMessageAtServer&, nodecpp::net::HttpServerResponse&)>;
			static constexpr const char* name = "HttpRequest";
		};
		static constexpr HttpRequest httpRequest = HttpRequest();

	};

	class SocketListener
	{
		public:
			virtual void onClose(bool hadError) {}
			virtual void onConnect() {}
			virtual void onData(const Buffer& buffer) {}
			virtual void onDrain() {}
			virtual void onEnd() {}
			virtual void onAccepted() {}
			virtual void onError(Error& err) {}
			virtual ~SocketListener() {}
	};

#ifndef NODECPP_MSVC_BUG_379712_WORKAROUND_NO_LISTENER
	class ServerListener
	{
		public:
			virtual void onClose(bool hadError) {}
			virtual void onConnection(soft_ptr<net::SocketBase> socket) {}
			virtual void onListening(size_t id, net::Address addr) {}
			virtual void onError(Error& err) {}
			virtual ~ServerListener() {}
	};
#endif

	/*
		Two important things:
		1. handlers are called in the same order of registration, both regular and 'once'
		2. handlers are called syncronously, and they shouln't be able to afect current 
		event dispatch. So handler list must be copied before start calling them.
		If any handler modify the handler list, the modification will be efective on the next
		event dispatch
	*/
	template<class EV>
	class EventEmitter
	{
		nodecpp::vector<std::pair<bool, typename EV::callback>> callbacks;
	public:
		void on(typename EV::callback cb) {
			callbacks.emplace_back(false,cb);
		}
		void once(typename EV::callback cb) {
			callbacks.emplace_back(true, cb);
		}

		void prependListener(typename EV::callback cb) {
			callbacks.insert(callbacks.begin(), std::make_pair(false, cb));
		}

		void prependOnceListener(typename EV::callback cb) {
			callbacks.insert(std::make_pair(true, cb));
		}

		size_t listenerCount() const {
			return callbacks.size();
		}
		const char* eventName() const {
			return EV::name;
		}

		template<class... ARGS>
		void emit(ARGS&... args) {

			nodecpp::vector<std::pair<bool, typename EV::callback>> other;
			
			//first make a copy of handlers
			for (auto& current : callbacks) {
				if (!current.first)
					other.push_back(current);
			}
	
			std::swap(callbacks, other);

			for (auto& current : other) {
				current.second(args...);
			}
		}
	};

	template<class EV, class ListenerT, auto onMyEvent>
	class EventEmitterSupportingListeners
	{
//		static constexpr auto onMyEvent = F;
		struct Element
		{
			bool once;
			bool isLambda; // note: now we have only two options here: lambda and listeners
			typename EV::callback cb;
			nodecpp::soft_ptr<ListenerT> listener;
			Element(bool once_, typename EV::callback cb_) : once(once_), isLambda(true), cb(cb_) {}
			Element(bool once_, nodecpp::soft_ptr<ListenerT> listener_) : once(once_), isLambda(false), listener(std::move(listener_)) {}
		};
		nodecpp::vector<Element> callbacks;
	public:
		void on(typename EV::callback cb) {
			callbacks.emplace_back(false,cb);
		}
		void once(typename EV::callback cb) {
			callbacks.emplace_back(true, cb);
		}

		void prepend(typename EV::callback cb) {
			callbacks.insert(callbacks.begin(), std::make_pair(false, cb));
		}
		void prependOnce(typename EV::callback cb) {
			callbacks.insert(std::make_pair(true, cb));
		}

		void on(nodecpp::soft_ptr<ListenerT> listener) {
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, listener );
			callbacks.emplace_back(false,std::move(listener));
		}
		void once(nodecpp::soft_ptr<ListenerT> listener) {
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, listener );
			callbacks.emplace_back(true, std::move(listener));
		}

		void prepend(nodecpp::soft_ptr<ListenerT> listener) {
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, listener );
			callbacks.insert(callbacks.begin(), std::make_pair(false, std::move(listener)));
		}
		void prependOnce(nodecpp::soft_ptr<ListenerT> listener) {
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, listener );
			callbacks.insert(std::make_pair(true, std::move(listener)));
		}

		size_t listenerCount() const {
			return callbacks.size();
		}
		const char* eventName() const {
			return EV::name;
		}

		template<class... ARGS>
		void emit(ARGS... args) {

			nodecpp::vector<Element> other;
			
			//first make a copy of handlers
			for (auto& current : callbacks) {
				if (!current.once)
				{
					NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, current.isLambda || current.listener);
					other.push_back(current);
				}
			}
	
			std::swap(callbacks, other);

			for (auto& current : callbacks) {
				if ( current.isLambda )
					current.cb(args...);
				else
				{
#ifndef NODECPP_MSVC_BUG_379712_WORKAROUND_NO_LISTENER
					NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, current.listener );
//					current.listener->*onMyEvent(args...);
					auto ptr = &(*current.listener.get());
					(ptr->*onMyEvent)(args...);
#else
					NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false );
#endif
				}
			}
		}
	};
}
#endif //EVENT_H



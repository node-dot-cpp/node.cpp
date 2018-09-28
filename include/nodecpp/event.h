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
#ifndef EVENT_H
#define EVENT_H

#include <functional>
#include <vector>

#include "error.h"
#include "net_common.h"

namespace nodecpp
{
	class Buffer;
	namespace net { class SocketTBase; }


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
			using callback = std::function<void(Buffer&)>;
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
			using callback = std::function<void(net::SocketTBase*)>;
			static constexpr const char* name = "connection";
		};
		static constexpr Connection connection = Connection();

		struct Listening
		{
			using callback = std::function<void(size_t, net::Address)>;
			static constexpr const char* name = "listening";
		};
		static constexpr Listening listening = Listening();

	};

	class SocketListener
	{
		public:
			virtual void onClose(bool hadError) {}
			virtual void onConnect() {}
			virtual void onData(Buffer& buffer) {}
			virtual void onDrain() {}
			virtual void onEnd() {}
			virtual void onAccepted() {}
			virtual void onError(Error& err) {}
	};

	class ServerListener
	{
		public:
			virtual void onClose(bool hadError) {}
			virtual void onConnection(net::SocketTBase* socket) {}
			virtual void onListening(size_t id, net::Address addr) {}
			//virtual void onListening() {}
			virtual void onError(Error& err) {}
	};

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
		std::vector<std::pair<bool, typename EV::callback>> callbacks;
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
		void emit(ARGS... args) {

			std::vector<std::pair<bool, typename EV::callback>> other;
			
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
			ListenerT* listener;
			Element(bool once_, typename EV::callback cb_) : once(once_), isLambda(true), cb(cb_), listener(nullptr) {}
			Element(bool once_, ListenerT* listener_) : once(once_), isLambda(false), listener(listener_) {}
		};
		std::vector<Element> callbacks;
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

		void on(ListenerT* listener) {
			callbacks.emplace_back(false,listener);
		}
		void once(ListenerT* listener) {
			callbacks.emplace_back(true, listener);
		}

		void prepend(ListenerT* listener) {
			callbacks.insert(callbacks.begin(), std::make_pair(false, listener));
		}
		void prependOnce(ListenerT* listener) {
			callbacks.insert(std::make_pair(true, listener));
		}

		size_t listenerCount() const {
			return callbacks.size();
		}
		const char* eventName() const {
			return EV::name;
		}

		template<class... ARGS>
		void emit(ARGS... args) {

			std::vector<Element> other;
			
			//first make a copy of handlers
			for (auto& current : callbacks) {
				if (!current.once)
					other.push_back(current);
			}
	
			std::swap(callbacks, other);

			for (auto& current : other) {
				if ( current.isLambda )
					current.cb(args...);
				else
				{
					NODECPP_ASSERT( current.listener != nullptr );
//					current.listener->*onMyEvent(args...);
					(current.listener->*onMyEvent)(args...);
				}
			}
		}
	};
}
#endif //EVENT_H



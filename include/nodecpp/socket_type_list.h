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

#ifndef SOCKET_TYPE_LIST_H
#define SOCKET_TYPE_LIST_H

#include "template_common.h"
#include "socket_common.h"

namespace nodecpp {

	namespace net {


		// "iteration" over a list of handling functions for an event

		// checking handler presence (without envocation)

		template<class Node, class Socket, class HandlerDataT, class ... args>
		bool checkIsOnConnectHandlers( Node* nodePtr, nodecpp::safememory::soft_ptr<Socket> socketPtr ) { return true; }
		template<class Node, class Socket>
		bool checkIsOnConnectHandlers( Node* nodePtr, nodecpp::safememory::soft_ptr<Socket> socketPtr ) { return false; }

		template<class Node, class Socket, class HandlerDataT, class ... args>
		bool checkIsOnAcceptedHandlers( Node* nodePtr, nodecpp::safememory::soft_ptr<Socket> socketPtr ) { return true; }
		template<class Node, class Socket>
		bool checkIsOnAcceptedHandlers( Node* nodePtr, nodecpp::safememory::soft_ptr<Socket> ptr, bool hadError ) { return false; }

		template<class Node, class Socket, class HandlerDataT, class ... args>
		bool checkIsOnDataHandlers( Node* nodePtr, nodecpp::safememory::soft_ptr<Socket> socketPtr, nodecpp::Buffer& b ) { return true; }
		template<class Node, class Socket>
		bool checkIsOnDataHandlers( Node* nodePtr, nodecpp::safememory::soft_ptr<Socket> ptr, nodecpp::Buffer& b ) { return false; }

		template<class Node, class Socket, class HandlerDataT, class ... args>
		bool checkIsOnDrainHandlers( Node* nodePtr, nodecpp::safememory::soft_ptr<Socket> socketPtr ) { return true; }
		template<class Node, class Socket>
		bool checkIsOnDrainHandlers( Node* nodePtr, nodecpp::safememory::soft_ptr<Socket> ptr ) { return false; }

		template<class Node, class Socket, class HandlerDataT, class ... args>
		bool checkIsOnCloseSocketHandlers( Node* nodePtr, nodecpp::safememory::soft_ptr<Socket> socketPtr, bool hadError ) { return true; }
		template<class Node, class Socket>
		bool checkIsOnCloseSocketHandlers( Node* nodePtr, nodecpp::safememory::soft_ptr<Socket> ptr, bool hadError ) { return false; }

		template<class Node, class Socket, class HandlerDataT, class ... args>
		bool checkIsOnEndHandlers( Node* nodePtr, nodecpp::safememory::soft_ptr<Socket> socketPtr ) { return true; }
		template<class Node, class Socket>
		bool checkIsOnEndHandlers( Node* nodePtr, nodecpp::safememory::soft_ptr<Socket> ptr ) { return false; }

		template<class Node, class Socket, class HandlerDataT, class ... args>
		bool checkIsOnErrorSocketHandlers( Node* nodePtr, nodecpp::safememory::soft_ptr<Socket> socketPtr, nodecpp::Error& e ) { return true; }
		template<class Node, class Socket>
		bool checkIsOnErrorSocketHandlers( Node* nodePtr, nodecpp::safememory::soft_ptr<Socket> ptr, nodecpp::Error& e ) { return false; }


		// calling handlers

		template<class Node, class Socket, class HandlerDataT, class ... args>
		void callOnConnectHandlers( Node* nodePtr, nodecpp::safememory::soft_ptr<Socket> socketPtr )
		{
			if constexpr (std::is_same< Node, typename HandlerDataT::ObjT >::value)
				(nodePtr->*HandlerDataT::memberFn)(socketPtr);
			else
			{
				static_assert (std::is_same< Socket, typename HandlerDataT::ObjT >::value );
				((&(*socketPtr))->*HandlerDataT::memberFn)();
			}

			callOnConnectHandlers<Node, Socket, args...>(nodePtr, socketPtr);
		}

		template<class Node, class Socket>
		void callOnConnectHandlers( Node* nodePtr, nodecpp::safememory::soft_ptr<Socket> socketPtr )
		{
			return;
		}


		template<class Node, class Socket, class HandlerDataT, class ... args>
		void callOnAcceptedHandlers( Node* nodePtr, nodecpp::safememory::soft_ptr<Socket> socketPtr )
		{
			if constexpr (std::is_same< Node, typename HandlerDataT::ObjT >::value)
				(nodePtr->*HandlerDataT::memberFn)(socketPtr);
			else
			{
				static_assert (std::is_same< Socket, typename HandlerDataT::ObjT >::value );
				((&(*socketPtr))->*HandlerDataT::memberFn)();
			}

			callOnAcceptedHandlers<Node, Socket, args...>(nodePtr, socketPtr);
		}

		template<class Node, class Socket>
		void callOnAcceptedHandlers( Node* nodePtr, nodecpp::safememory::soft_ptr<Socket> ptr, bool hadError )
		{
			return;
		}


		template<class Node, class Socket, class HandlerDataT, class ... args>
		void callOnDataHandlers( Node* nodePtr, nodecpp::safememory::soft_ptr<Socket> socketPtr, nodecpp::Buffer& b )
		{
			if constexpr (std::is_same< Node, typename HandlerDataT::ObjT >::value)
				(nodePtr->*HandlerDataT::memberFn)( socketPtr, b );
			else
			{
				static_assert (std::is_same< Socket, typename HandlerDataT::ObjT >::value );
				((&(*socketPtr))->*HandlerDataT::memberFn)( b );
			}

			callOnDataHandlers<Node, Socket, args...>(nodePtr, socketPtr, b);
		}

		template<class Node, class Socket>
		void callOnDataHandlers( Node* nodePtr, nodecpp::safememory::soft_ptr<Socket> ptr, nodecpp::Buffer& b )
		{
			return;
		}


		template<class Node, class Socket, class HandlerDataT, class ... args>
		void callOnDrainHandlers( Node* nodePtr, nodecpp::safememory::soft_ptr<Socket> socketPtr )
		{
			if constexpr (std::is_same< Node, typename HandlerDataT::ObjT >::value)
				(nodePtr->*HandlerDataT::memberFn)(socketPtr);
			else
			{
				static_assert (std::is_same< Socket, typename HandlerDataT::ObjT >::value );
				((&(*socketPtr))->*HandlerDataT::memberFn)();
			}

			callOnDrainHandlers<Node, Socket, args...>(nodePtr, socketPtr);
		}

		template<class Node, class Socket>
		void callOnDrainHandlers( Node* nodePtr, nodecpp::safememory::soft_ptr<Socket> ptr )
		{
			return;
		}


		template<class Node, class Socket, class HandlerDataT, class ... args>
		void callOnCloseSocketHandlers( Node* nodePtr, nodecpp::safememory::soft_ptr<Socket> socketPtr, bool hadError )
		{
			if constexpr (std::is_same< Node, typename HandlerDataT::ObjT >::value)
				(nodePtr->*HandlerDataT::memberFn)( socketPtr, hadError );
			else
			{
				static_assert (std::is_same< Socket, typename HandlerDataT::ObjT >::value );
				((&(*socketPtr))->*HandlerDataT::memberFn)( hadError );
			}

			callOnCloseSocketHandlers<Node, Socket, args...>(nodePtr, socketPtr, hadError);
		}

		template<class Node, class Socket>
		void callOnCloseSocketHandlers( Node* nodePtr, nodecpp::safememory::soft_ptr<Socket> ptr, bool hadError )
		{
			return;
		}


		template<class Node, class Socket, class HandlerDataT, class ... args>
		void callOnEndHandlers( Node* nodePtr, nodecpp::safememory::soft_ptr<Socket> socketPtr )
		{
			if constexpr (std::is_same< Node, typename HandlerDataT::ObjT >::value)
				(nodePtr->*HandlerDataT::memberFn)(socketPtr);
			else
			{
				static_assert (std::is_same< Socket, typename HandlerDataT::ObjT >::value );
				((&(*socketPtr))->*HandlerDataT::memberFn)();
			}

			callOnEndHandlers<Node, Socket, args...>(nodePtr, socketPtr);
		}

		template<class Node, class Socket>
		void callOnEndHandlers( Node* nodePtr, nodecpp::safememory::soft_ptr<Socket> ptr )
		{
			return;
		}


		template<class Node, class Socket, class HandlerDataT, class ... args>
		void callOnErrorSocketHandlers( Node* nodePtr, nodecpp::safememory::soft_ptr<Socket> socketPtr, nodecpp::Error& e )
		{
			if constexpr (std::is_same< Node, typename HandlerDataT::ObjT >::value)
				(nodePtr->*HandlerDataT::memberFn)( socketPtr, e );
			else
			{
				static_assert (std::is_same< Socket, typename HandlerDataT::ObjT >::value );
				((&(*socketPtr))->*HandlerDataT::memberFn)( e );
			}

			callOnErrorSocketHandlers<Node, Socket, args...>(nodePtr, socketPtr, e);
		}

		template<class Node, class Socket>
		void callOnErrorSocketHandlers( Node* nodePtr, nodecpp::safememory::soft_ptr<Socket> ptr, nodecpp::Error& e )
		{
			return;
		}



		template<class Socket, class ... args> // 'args' are HandlerData<...>
		struct SocketHandlerDataList
		{
			using SocketT = Socket;

			// checking handler presence (without envocation)

			template<class Node>
			static bool isConnectEmitter( Node* node, nodecpp::safememory::soft_ptr<Socket> socket ) {
				return checkIsOnConnectHandlers<Node, Socket, args...>(node, socket); 
			}
			template<class Node>
			static bool isAcceptedEmitter( Node* node, nodecpp::safememory::soft_ptr<Socket> socket ) {
				return checkIsOnAcceptedHandlers<Node, Socket, args...>(node, socket); 
			}
			template<class Node>
			static bool isDataEmitter( Node* node, nodecpp::safememory::soft_ptr<Socket> socket, nodecpp::Buffer& b ) { 
				return checkIsOnDataHandlers<Node, Socket, args...>(node, socket, b);
			}
			template<class Node>
			static bool isDrainEmitter( Node* node, nodecpp::safememory::soft_ptr<Socket> socket ) { 
				return checkIsOnDrainHandlers<Node, Socket, args...>(node, socket);
			}
			template<class Node>
			static bool isCloseEmitter( Node* node, nodecpp::safememory::soft_ptr<Socket> socket, bool hadError ) {
				return checkIsOnCloseSocketHandlers<Node, Socket, args...>(node, socket, hadError);
			}
			template<class Node>
			static bool isEndEmitter( Node* node, nodecpp::safememory::soft_ptr<Socket> socket ) { 
				return checkIsOnEndHandlers<Node, Socket, args...>(node, socket);
			}
			template<class Node>
			static bool isErrorEmitter( Node* node, nodecpp::safememory::soft_ptr<Socket> socket, nodecpp::Error& e ) { 
				return checkIsOnErrorSocketHandlers<Node, Socket, args...>(node, socket, e);
			}

			// calling handlers

			template<class Node>
			static void emitConnect( Node* node, nodecpp::safememory::soft_ptr<Socket> socket ) {
				callOnConnectHandlers<Node, Socket, args...>(node, socket); 
			}
			template<class Node>
			static void emitAccepted( Node* node, nodecpp::safememory::soft_ptr<Socket> socket ) {
				callOnAcceptedHandlers<Node, Socket, args...>(node, socket); 
			}
			template<class Node>
			static void emitData( Node* node, nodecpp::safememory::soft_ptr<Socket> socket, nodecpp::Buffer& b ) { 
				callOnDataHandlers<Node, Socket, args...>(node, socket, b);
			}
			template<class Node>
			static void emitDrain( Node* node, nodecpp::safememory::soft_ptr<Socket> socket ) { 
				callOnDrainHandlers<Node, Socket, args...>(node, socket);
			}
			template<class Node>
			static void emitClose( Node* node, nodecpp::safememory::soft_ptr<Socket> socket, bool hadError ) {
				callOnCloseSocketHandlers<Node, Socket, args...>(node, socket, hadError);
			}
			template<class Node>
			static void emitEnd( Node* node, nodecpp::safememory::soft_ptr<Socket> socket ) { 
				callOnEndHandlers<Node, Socket, args...>(node, socket);
			}
			template<class Node>
			static void emitError( Node* node, nodecpp::safememory::soft_ptr<Socket> socket, nodecpp::Error& e ) { 
				callOnErrorSocketHandlers<Node, Socket, args...>(node, socket, e);
			}
		};


		template<class T>
		struct OnConnectT {};

		template<class T>
		struct OnAcceptedT {};

		template<class T>
		struct OnDataT {};

		template<class T>
		struct OnDrainT {};

		template<class T>
		struct OnEndT {};

		template<class T>
		struct OnCloseT {};

		template<class T>
		struct OnErrorT {};

		template<typename ... args>
		struct SocketHandlerDescriptorBase;

		//partial template specializations:
		template<class T, typename ... args>
		struct SocketHandlerDescriptorBase<OnConnectT<T>, args...>
			: public SocketHandlerDescriptorBase<args...> {
			using onConnectT = T;
		};

		template<class T, typename ... args>
		struct SocketHandlerDescriptorBase<OnAcceptedT<T>, args...>
			: public SocketHandlerDescriptorBase<args...> {
			using onAcceptedT = T;
		};

		template<class T, typename ... args>
		struct SocketHandlerDescriptorBase<OnDataT<T>, args...>
			: public SocketHandlerDescriptorBase<args...> {
			using onDataT = T;
		};

		template<class T, typename ... args>
		struct SocketHandlerDescriptorBase<OnDrainT<T>, args...>
			: public SocketHandlerDescriptorBase<args...> {
			using onDrainT = T;
		};

		template<class T, typename ... args>
		struct SocketHandlerDescriptorBase<OnCloseT<T>, args...>
			: public SocketHandlerDescriptorBase<args...> {
			using onCloseT = T;
		};

		template<class T, typename ... args>
		struct SocketHandlerDescriptorBase<OnEndT<T>, args...>
			: public SocketHandlerDescriptorBase<args...> {
			using onEndT = T;
		};

		template<class T, typename ... args>
		struct SocketHandlerDescriptorBase<OnErrorT<T>, args...>
			: public SocketHandlerDescriptorBase<args...> {
			using onErrorT = T;
		};

		template<> // forming template param list as onXxxST<HandlerDataList<...>>, ...
		struct SocketHandlerDescriptorBase<> {
			using onConnectT = void;
			using onAcceptedT = void;
			using onDataT = void;
			using onDrainT = void;
			using onCloseT = void;
			using onEndT = void;
			using onErrorT = void;
		};

		template<class SocketT, class HandlerDesciptorT>
		struct SocketHandlerDescriptor {
			using SocketType = SocketT;
			using HandlerDesciptorType = HandlerDesciptorT;
		};

		// type selection (for checkIsXXX() )

		template<class Node, class T, class T1, class ... args>
		bool checkIsOnConnect( Node* nodePtr, T* ptr, int type )
		{
			if ( type == 0 )
			{
				if constexpr ( std::is_same< typename T1::HandlerDesciptorType::onConnectT, void >::value )
					return false;
				else
				{
					soft_ptr<typename T1::SocketType> socketTypedPtr = nodecpp::safememory::soft_ptr_static_cast<typename T1::SocketType>(ptr->getPtr());
					return T1::HandlerDesciptorType::onConnectT::isConnectEmitter( nodePtr, socketTypedPtr );
				}
			}
			else
				return checkIsOnConnect<Node, T, args...>(nodePtr, ptr, type-1);
		}

		template<class Node, class T>
		bool checkIsOnConnect( Node* nodePtr, T* ptr, int type )
		{
			return false;
		}


		template<class Node, class T, class T1, class ... args>
		bool checkIsOnAccepted( Node* nodePtr, T* ptr, int type )
		{
			if ( type == 0 )
			{
				if constexpr ( std::is_same< typename T1::HandlerDesciptorType::onAcceptedT, void >::value )
					return false;
				else
				{
					soft_ptr<typename T1::SocketType> socketTypedPtr = nodecpp::safememory::soft_ptr_static_cast<typename T1::SocketType>(ptr->getPtr());
					return T1::HandlerDesciptorType::onAcceptedT::isAcceptedEmitter( nodePtr, socketTypedPtr );
				}
			}
			else
				return checkIsOnAccepted<Node, T, args...>(nodePtr, ptr, type-1);
		}

		template<class Node, class T>
		bool checkIsOnAccepted( Node* nodePtr, T* ptr, int type )
		{
			return false;
		}


		template<class Node, class T, class T1, class ... args>
		bool checkIsOnData( Node* nodePtr, T* ptr, int type, nodecpp::Buffer& b )
		{
			if ( type == 0 )
			{
				if constexpr ( std::is_same< typename T1::HandlerDesciptorType::onDataT, void >::value )
					return false;
				else
				{
					soft_ptr<typename T1::SocketType> socketTypedPtr = nodecpp::safememory::soft_ptr_static_cast<typename T1::SocketType>(ptr->getPtr());
					return T1::HandlerDesciptorType::onDataT::isDataEmitter( nodePtr, socketTypedPtr, b );
				}
			}
			else
				return checkIsOnData<Node, T, args...>(nodePtr, ptr, type-1, b);
		}

		template<class Node, class T>
		bool checkIsOnData( Node* nodePtr, T* ptr, int type, nodecpp::Buffer& b )
		{
			return false;
		}


		template<class Node, class T, class T1, class ... args>
		bool checkIsOnDrainSocket( Node* nodePtr, T* ptr, int type )
		{
			if ( type == 0 )
			{
				if constexpr ( std::is_same< typename T1::HandlerDesciptorType::onDrainT, void >::value )
					return false;
				else
				{
					soft_ptr<typename T1::SocketType> socketTypedPtr = nodecpp::safememory::soft_ptr_static_cast<typename T1::SocketType>(ptr->getPtr());
					return T1::HandlerDesciptorType::onDrainT::isDrainEmitter( nodePtr, socketTypedPtr );
				}
			}
			else
				return checkIsOnDrainSocket<Node, T, args...>(nodePtr, ptr, type-1);
		}

		template<class Node, class T>
		bool checkIsOnDrainSocket( Node* nodePtr, T* ptr, int type )
		{
			return false;
		}


		template<class Node, class T, class T1, class ... args>
		bool checkIsOnCloseSocket( Node* nodePtr, T* ptr, int type, bool hadError )
		{
			if ( type == 0 )
			{
				if constexpr ( std::is_same< typename T1::HandlerDesciptorType::onCloseT, void >::value )
					return false;
				else
				{
					soft_ptr<typename T1::SocketType> socketTypedPtr = nodecpp::safememory::soft_ptr_static_cast<typename T1::SocketType>(ptr->getPtr());
					return T1::HandlerDesciptorType::onCloseT::isCloseEmitter( nodePtr, socketTypedPtr, hadError );
				}
			}
			else
				return checkIsOnCloseSocket<Node, T, args...>(nodePtr, ptr, type-1, hadError);
		}

		template<class Node, class T>
		bool checkIsOnCloseSocket( Node* nodePtr, T* ptr, int type, bool hadError )
		{
			return false;
		}


		template<class Node, class T, class T1, class ... args>
		bool checkIsOnEnd( Node* nodePtr, T* ptr, int type )
		{
			if ( type == 0 )
			{
				if constexpr ( std::is_same< typename T1::HandlerDesciptorType::onEndT, void >::value )
					return false;
				else
				{
					soft_ptr<typename T1::SocketType> socketTypedPtr = nodecpp::safememory::soft_ptr_static_cast<typename T1::SocketType>(ptr->getPtr());
					return T1::HandlerDesciptorType::onEndT::isEndEmitter( nodePtr, socketTypedPtr );
				}
			}
			else
				return checkIsOnEnd<Node, T, args...>(nodePtr, ptr, type-1);
		}

		template<class Node, class T>
		bool checkIsOnEnd( Node* nodePtr, T* ptr, int type )
		{
			return false;
		}


		template<class Node, class T, class T1, class ... args>
		bool checkIsOnErrorSocket( Node* nodePtr, T* ptr, int type, nodecpp::Error& e )
		{
			if ( type == 0 )
			{
				if constexpr ( std::is_same< typename T1::HandlerDesciptorType::onErrorT, void >::value )
					return false;
				else
				{
					soft_ptr<typename T1::SocketType> socketTypedPtr = nodecpp::safememory::soft_ptr_static_cast<typename T1::SocketType>(ptr->getPtr());
					return T1::HandlerDesciptorType::onErrorT::isErrorEmitter( nodePtr, socketTypedPtr, e );
				}
			}
			else
				return checkIsOnErrorSocket<Node, T, args...>(nodePtr, ptr, type-1, e);
		}

		template<class Node, class T>
		bool checkIsOnErrorSocket( Node* nodePtr, T* ptr, int type, nodecpp::Error& e )
		{
			return false;
		}


		// type selection (for callXXX() )

		template<class Node, class T, class T1, class ... args>
		void callOnConnect( Node* nodePtr, T* ptr, int type )
		{
			assert(type!= -1);
			if ( type == 0 )
			{
				if constexpr ( !std::is_same< typename T1::HandlerDesciptorType::onConnectT, void >::value )
				{
					soft_ptr<typename T1::SocketType> socketTypedPtr = nodecpp::safememory::soft_ptr_static_cast<typename T1::SocketType>(ptr->getPtr());
					T1::HandlerDesciptorType::onConnectT::emitConnect( nodePtr, socketTypedPtr );
				}
			}
			else
				callOnConnect<Node, T, args...>(nodePtr, ptr, type-1);
		}

		template<class Node, class T>
		void callOnConnect( Node* nodePtr, T* ptr, int type )
		{
			assert( false );
		}


		template<class Node, class T, class T1, class ... args>
		void callOnAccepted( Node* nodePtr, T* ptr, int type )
		{
			if ( type == 0 )
			{
				if constexpr ( !std::is_same< typename T1::HandlerDesciptorType::onAcceptedT, void >::value )
				{
					soft_ptr<typename T1::SocketType> socketTypedPtr = nodecpp::safememory::soft_ptr_static_cast<typename T1::SocketType>(ptr->getPtr());
					T1::HandlerDesciptorType::onAcceptedT::emitAccepted( nodePtr, socketTypedPtr );
				}
			}
			else
				callOnAccepted<Node, T, args...>(nodePtr, ptr, type-1);
		}

		template<class Node, class T>
		void callOnAccepted( Node* nodePtr, T* ptr, int type )
		{
			assert( false );
		}


		template<class Node, class T, class T1, class ... args>
		void callOnData( Node* nodePtr, T* ptr, int type, nodecpp::Buffer& b )
		{
			if ( type == 0 )
			{
				if constexpr ( !std::is_same< typename T1::HandlerDesciptorType::onDataT, void >::value )
				{
					soft_ptr<typename T1::SocketType> socketTypedPtr = nodecpp::safememory::soft_ptr_static_cast<typename T1::SocketType>(ptr->getPtr());
					T1::HandlerDesciptorType::onDataT::emitData( nodePtr, socketTypedPtr, b );
				}
			}
			else
				callOnData<Node, T, args...>(nodePtr, ptr, type-1, b);
		}

		template<class Node, class T>
		void callOnData( Node* nodePtr, T* ptr, int type, nodecpp::Buffer& b )
		{
			assert( false );
		}


		template<class Node, class T, class T1, class ... args>
		void callOnDrainSocket( Node* nodePtr, T* ptr, int type )
		{
			if ( type == 0 )
			{
				if constexpr ( !std::is_same< typename T1::HandlerDesciptorType::onDrainT, void >::value )
				{
					soft_ptr<typename T1::SocketType> socketTypedPtr = nodecpp::safememory::soft_ptr_static_cast<typename T1::SocketType>(ptr->getPtr());
					T1::HandlerDesciptorType::onDrainT::emitDrain( nodePtr, socketTypedPtr );
				}
			}
			else
				callOnDrainSocket<Node, T, args...>(nodePtr, ptr, type-1);
		}

		template<class Node, class T>
		void callOnDrainSocket( Node* nodePtr, T* ptr, int type )
		{
			assert( false );
		}


		template<class Node, class T, class T1, class ... args>
		void callOnCloseSocket( Node* nodePtr, T* ptr, int type, bool hadError )
		{
			if ( type == 0 )
			{
				if constexpr ( !std::is_same< typename T1::HandlerDesciptorType::onCloseT, void >::value )
				{
					soft_ptr<typename T1::SocketType> socketTypedPtr = nodecpp::safememory::soft_ptr_static_cast<typename T1::SocketType>(ptr->getPtr());
					T1::HandlerDesciptorType::onCloseT::emitClose( nodePtr, socketTypedPtr, hadError );
				}
			}
			else
				callOnCloseSocket<Node, T, args...>(nodePtr, ptr, type-1, hadError);
		}

		template<class Node, class T>
		void callOnCloseSocket( Node* nodePtr, T* ptr, int type, bool hadError )
		{
			assert( false );
		}


		template<class Node, class T, class T1, class ... args>
		void callOnEnd( Node* nodePtr, T* ptr, int type )
		{
			if ( type == 0 )
			{
				if constexpr ( !std::is_same< typename T1::HandlerDesciptorType::onEndT, void >::value )
				{
					soft_ptr<typename T1::SocketType> socketTypedPtr = nodecpp::safememory::soft_ptr_static_cast<typename T1::SocketType>(ptr->getPtr());
					T1::HandlerDesciptorType::onEndT::emitEnd( nodePtr, socketTypedPtr );
				}
			}
			else
				callOnEnd<Node, T, args...>(nodePtr, ptr, type-1);
		}

		template<class Node, class T>
		void callOnEnd( Node* nodePtr, T* ptr, int type )
		{
			assert( false );
		}


		template<class Node, class T, class T1, class ... args>
		void callOnErrorSocket( Node* nodePtr, T* ptr, int type, nodecpp::Error& e )
		{
			if ( type == 0 )
			{
				if constexpr ( !std::is_same< typename T1::HandlerDesciptorType::onErrorT, void >::value )
				{
					soft_ptr<typename T1::SocketType> socketTypedPtr = nodecpp::safememory::soft_ptr_static_cast<typename T1::SocketType>(ptr->getPtr());
					T1::HandlerDesciptorType::onErrorT::emitError( nodePtr, socketTypedPtr, e );
				}
			}
			else
				callOnErrorSocket<Node, T, args...>(nodePtr, ptr, type-1, e);
		}

		template<class Node, class T>
		void callOnErrorSocket( Node* nodePtr, T* ptr, int type, nodecpp::Error& e )
		{
			assert( false );
		}



		template< class ... args >
		class SocketTEmitter
		{
		public:
			class Ptr
			{
				nodecpp::safememory::soft_ptr<SocketBase> ptr;
			public:
				Ptr( nodecpp::safememory::soft_ptr<SocketBase> ptr_ ) { ptr = ptr_; }
				nodecpp::safememory::soft_ptr<SocketBase> getPtr() const {return ptr;}
			};

		public:
			template<class Sock>
			static int getTypeIndex(Sock* s) { 
				static_assert( !std::is_same<typename Sock::NodeType, void>::value );
				return ::getTypeIndex<Sock,typename args::SocketType...>( s ); 
			}
			template<class Sock>
			static int softGetTypeIndexIfTypeExists() { return ::softGetTypeIndexIfTypeExists<Sock,typename args::SocketType...>(); }

			template<class Node>
			static Node* getThreadNode() { return static_cast<Node*>(thisThreadNode); }


			// checking presense of an emitter

			template<class Node>
			static bool isConnectEmitter( const OpaqueEmitter& emitter ) {
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, emitter.objectType == OpaqueEmitter::ObjectType::ClientSocket); 
				if ( emitter.type == -1 ) return false;
				Ptr emitter_ptr( nodecpp::safememory::soft_ptr_static_cast<SocketBase>(emitter.getClientSocketPtr()) ); 
				return checkIsOnConnect<Node, Ptr, args...>(getThreadNode<Node>(), &emitter_ptr, emitter.type); 
			}
			template<class Node>
			static bool isAcceptedEmitter( const OpaqueEmitter& emitter ) {
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, emitter.objectType == OpaqueEmitter::ObjectType::ClientSocket); 
				if ( emitter.type == -1 ) return false;
				Ptr emitter_ptr( nodecpp::safememory::soft_ptr_static_cast<SocketBase>(emitter.getClientSocketPtr()) ); 
				return checkIsOnAccepted<Node, Ptr, args...>(getThreadNode<Node>(), &emitter_ptr, emitter.type); 
			}
			template<class Node>
			static bool isDataEmitter( const OpaqueEmitter& emitter, nodecpp::Buffer& b ) { 
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, emitter.objectType == OpaqueEmitter::ObjectType::ClientSocket); 
				if ( emitter.type == -1 ) return false;
				Ptr emitter_ptr( nodecpp::safememory::soft_ptr_static_cast<SocketBase>(emitter.getClientSocketPtr()) ); 
				return checkIsOnData<Node, Ptr, args...>(getThreadNode<Node>(), &emitter_ptr, emitter.type, b);
			}
			template<class Node>
			static bool isDrainEmitter( const OpaqueEmitter& emitter ) { 
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, emitter.objectType == OpaqueEmitter::ObjectType::ClientSocket); 
				if ( emitter.type == -1 ) return false;
				Ptr emitter_ptr( nodecpp::safememory::soft_ptr_static_cast<SocketBase>(emitter.getClientSocketPtr()) ); 
				return checkIsOnDrainSocket<Node, Ptr, args...>(getThreadNode<Node>(), &emitter_ptr, emitter.type);
			}
			template<class Node>
			static bool isCloseEmitter( const OpaqueEmitter& emitter, bool hadError ) {
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, emitter.objectType == OpaqueEmitter::ObjectType::ClientSocket); 
				if ( emitter.type == -1 ) return false;
				Ptr emitter_ptr( nodecpp::safememory::soft_ptr_static_cast<SocketBase>(emitter.getClientSocketPtr()) ); 
				return checkIsOnCloseSocket<Node, Ptr, args...>(getThreadNode<Node>(), &emitter_ptr, emitter.type, hadError);
			}
			template<class Node>
			static bool isEndEmitter( const OpaqueEmitter& emitter ) { 
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, emitter.objectType == OpaqueEmitter::ObjectType::ClientSocket); 
				if ( emitter.type == -1 ) return false;
				Ptr emitter_ptr( nodecpp::safememory::soft_ptr_static_cast<SocketBase>(emitter.getClientSocketPtr()) ); 
				return checkIsOnEnd<Node, Ptr, args...>(getThreadNode<Node>(), &emitter_ptr, emitter.type);
			}
			template<class Node>
			static bool isErrorEmitter( const OpaqueEmitter& emitter, nodecpp::Error& e ) { 
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, emitter.objectType == OpaqueEmitter::ObjectType::ClientSocket); 
				if ( emitter.type == -1 ) return false;
				Ptr emitter_ptr( nodecpp::safememory::soft_ptr_static_cast<SocketBase>(emitter.getClientSocketPtr()) ); 
				return checkIsOnErrorSocket<Node, Ptr, args...>(getThreadNode<Node>(), &emitter_ptr, emitter.type, e);
			}

			// emiitting

			template<class Node>
			static void emitConnect( const OpaqueEmitter& emitter ) {
				if ( emitter.type == -1 ) return; // replace by assert(); should be checked externally before making this call
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, emitter.objectType == OpaqueEmitter::ObjectType::ClientSocket); 
				Ptr emitter_ptr( nodecpp::safememory::soft_ptr_static_cast<SocketBase>(emitter.getClientSocketPtr()) ); 
				callOnConnect<Node, Ptr, args...>(getThreadNode<Node>(), &emitter_ptr, emitter.type); 
			}
			template<class Node>
			static void emitAccepted( const OpaqueEmitter& emitter ) {
				if ( emitter.type == -1 ) return; // replace by assert(); should be checked externally before making this call
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, emitter.objectType == OpaqueEmitter::ObjectType::ClientSocket); 
				Ptr emitter_ptr( nodecpp::safememory::soft_ptr_static_cast<SocketBase>(emitter.getClientSocketPtr()) ); 
				callOnAccepted<Node, Ptr, args...>(getThreadNode<Node>(), &emitter_ptr, emitter.type); 
			}
			template<class Node>
			static void emitData( const OpaqueEmitter& emitter, nodecpp::Buffer& b ) { 
				if ( emitter.type == -1 ) return; // replace by assert(); should be checked externally before making this call
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, emitter.objectType == OpaqueEmitter::ObjectType::ClientSocket); 
				Ptr emitter_ptr( nodecpp::safememory::soft_ptr_static_cast<SocketBase>(emitter.getClientSocketPtr()) ); 
				callOnData<Node, Ptr, args...>(getThreadNode<Node>(), &emitter_ptr, emitter.type, b);
			}
			template<class Node>
			static void emitDrain( const OpaqueEmitter& emitter ) { 
				if ( emitter.type == -1 ) return; // replace by assert(); should be checked externally before making this call
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, emitter.objectType == OpaqueEmitter::ObjectType::ClientSocket); 
				Ptr emitter_ptr( nodecpp::safememory::soft_ptr_static_cast<SocketBase>(emitter.getClientSocketPtr()) ); 
				callOnDrainSocket<Node, Ptr, args...>(getThreadNode<Node>(), &emitter_ptr, emitter.type);
			}
			template<class Node>
			static void emitClose( const OpaqueEmitter& emitter, bool hadError ) {
				if ( emitter.type == -1 ) return; // replace by assert(); should be checked externally before making this call
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, emitter.objectType == OpaqueEmitter::ObjectType::ClientSocket); 
				Ptr emitter_ptr( nodecpp::safememory::soft_ptr_static_cast<SocketBase>(emitter.getClientSocketPtr()) ); 
				callOnCloseSocket<Node, Ptr, args...>(getThreadNode<Node>(), &emitter_ptr, emitter.type, hadError);
			}
			template<class Node>
			static void emitEnd( const OpaqueEmitter& emitter ) { 
				if ( emitter.type == -1 ) return; // replace by assert(); should be checked externally before making this call
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, emitter.objectType == OpaqueEmitter::ObjectType::ClientSocket); 
				Ptr emitter_ptr( nodecpp::safememory::soft_ptr_static_cast<SocketBase>(emitter.getClientSocketPtr()) ); 
				callOnEnd<Node, Ptr, args...>(getThreadNode<Node>(), &emitter_ptr, emitter.type);
			}
			template<class Node>
			static void emitError( const OpaqueEmitter& emitter, nodecpp::Error& e ) { 
				if ( emitter.type == -1 ) return; // replace by assert(); should be checked externally before making this call
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, emitter.objectType == OpaqueEmitter::ObjectType::ClientSocket); 
				Ptr emitter_ptr( nodecpp::safememory::soft_ptr_static_cast<SocketBase>(emitter.getClientSocketPtr()) ); 
				callOnErrorSocket<Node, Ptr, args...>(getThreadNode<Node>(), &emitter_ptr, emitter.type, e);
			}
		};
	} // namespace net

} // namespace nodecpp

#endif // SOCKET_TYPE_LIST_H

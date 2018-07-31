#ifndef SOCKET_H
#define SOCKET_H

#include <stdio.h>

class SocketO
{
public:
	SocketO() { printf( "SocketO::SocketO()\n" ); }
	virtual void onConnect() { printf( "SocketO::onConnect()\n" ); }
	virtual void onClose() { printf( "SocketO::onClose()\n" ); }
};

template<class T, class S = void>
class MySocket : public SocketO
{
	T* t;
public:
	MySocket( T* t_ ) : SocketO(), t(t_)
	{ 
		printf( "MySocket1::MySocket1() [2]\n" ); 
	}
	virtual void onConnect() 
	{ 
		if constexpr ( std::is_same< S, void >::value )
			t->onConnect();
		else
			(t->*(S::onConnect))(); 
	}
	virtual void onClose() 
	{ 
		if constexpr ( std::is_same< S, void >::value )
			t->onClose();
		else
			(t->*(S::onClose))(); 
	}
};

///////////////////////////////////////////////////////////////////////////////////////////

#if 0
template<class Node, class ... Handlers>
class SocketN;

template<class Node, class Extra, class ... Handlers>
class SocketN<Node, Extra, Handlers...>
: public SocketN<Node, Handlers...> {
public:
SocketN(Node* n) : SocketN<Node, Handlers...>(n) {}
};

template<auto x>
struct OnClose {};

template<auto x>
struct OnConnect {};

//partial template specialization:
template<class Node, class Extra, void(Node::*F)(const Extra*), class ... Handlers>
class SocketN<Node, Extra, OnClose<F>, Handlers...>
: public SocketN<Node, Extra, Handlers...> {
public:
SocketN(Node* n) : SocketN<Node, Extra,Handlers...>(n) {}
void onClose() override { (SocketN<Node,Extra>::node->*F)(&(this->extra)); }
};

//partial template specialization:
template<class Node/*, class Extra*/, void(Node::*F)(const void*), class ... Handlers>
class SocketN<Node/*, Extra*/, OnClose<F>, Handlers...>
: public SocketN<Node/*, void*/, Handlers...> {
public:
SocketN(Node* n) : SocketN<Node/*, void*/,Handlers...>(n) {}
void onClose() override { (SocketN<Node,void>::node->*F)(nullptr); }
};

//partial template specialization:
template<class Node, class Extra, void(Node::*F)(const Extra*), class ... Handlers>
class SocketN<Node, Extra, OnConnect<F>, Handlers...>
: public SocketN<Node, Extra, Handlers...> {
public:
SocketN(Node* n) : SocketN<Node, Extra,Handlers...>(n) {}
void onConnect() override { (SocketN<Node,Extra>::node->*F)(&(this->extra)); }
};

//partial template specialization:
template<class Node/*, class Extra*/, void(Node::*F)(const void*), class ... Handlers>
class SocketN<Node/*, Extra*/, OnConnect<F>, Handlers...>
: public SocketN<Node/*, void*/, Handlers...> {
public:
SocketN(Node* n) : SocketN<Node/*, void*/,Handlers...>(n) {}
void onConnect() override { (SocketN<Node,void>::node->*F)(nullptr); }
};
//create similar partial specializations for all the handlers

//partial template specialiazation to end recursion
template<class Node, class Extra>
class SocketN<Node, Extra>
: public SocketO {
public:
Node* node;
Extra extra;
SocketN(Node* node_) { node = node_;}
};

//partial template specialiazation to end recursion
template<class Node>
class SocketN<Node>
: public SocketO {
public:
Node* node;
//int* extra;
SocketN(Node* node_/*, Extra* e*/) { node = node_;/*extra = e;*/}
};

#else

template<auto x>
struct OnClose {};

template<auto x>
struct OnConnect {};

template<class Node, class Extra, class ... Handlers>
class SocketN;

//partial template specialization:
template<class Node, class Extra, void(Node::*F)(const Extra*), class ... Handlers>
class SocketN<Node, Extra, OnClose<F>, Handlers...>
: public SocketN<Node, Extra, Handlers...> {
public:
SocketN(Node* n) : SocketN<Node, Extra,Handlers...>(n) {}
void onClose() override { (SocketN<Node,Extra>::node->*F)(this->getExtra()); }
};

//partial template specialization:
template<class Node, class Extra, void(Node::*F)(const Extra*), class ... Handlers>
class SocketN<Node, Extra, OnConnect<F>, Handlers...>
: public SocketN<Node, Extra, Handlers...> {
public:
SocketN(Node* n) : SocketN<Node, Extra,Handlers...>(n) {}
void onConnect() override { (SocketN<Node,Extra>::node->*F)(this->getExtra()); }
};

//create similar partial specializations for all the handlers

//partial template specialiazation to end recursion
template<class Node, class Extra>
class SocketN<Node, Extra>
: public SocketO {
public:
Node* node;
Extra extra;
SocketN(Node* node_) { node = node_;}
Extra* getExtra() { return &extra; }
};

template<class Node>
class SocketN<Node, void>
: public SocketO {
public:
Node* node;
SocketN(Node* node_) { node = node_;}
void* getExtra() { return nullptr; }
};

#endif

#endif // SOCKET_H

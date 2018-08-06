#ifndef TRACE_H
#define TRACE_H


template< typename... ARGS >
void nodecppTrace(const char* formatStr, const ARGS& ... args) {
	fmt::print(stderr, formatStr, args...);
}

#define NODECPP_TRACE0(formatStr) (nodecppTrace(formatStr "\n"))
#define NODECPP_TRACE(formatStr, ...) (nodecppTrace(formatStr "\n", __VA_ARGS__))

#endif // TRACE_H

#ifndef TEST_EX_2_BASE_H
#define TEST_EX_2_BASE_H

#include <assert.h>
#include <experimental/coroutine>
#include <future>
#include <stdio.h>
#include <optional>
#include <iostream>
#include <experimental/resumable> 
using namespace std;


struct read_context
{
	bool used = false;
	bool dataAvailable = false;
	std::experimental::coroutine_handle<> awaiting;
};

static constexpr size_t max_h_count = 4;
static read_context g_callbacks[max_h_count];
static void init_read_context() { memset(g_callbacks, 0, sizeof( g_callbacks ) ); }
static size_t register_me() { printf( "register_me()\n"  ); for ( size_t i=0; i<max_h_count; ++i ) if ( !g_callbacks[i].used ) { g_callbacks[i].used = true; return i; } assert( false ); return (size_t)(-1); }
static void unregister( size_t idx ) { assert( idx < max_h_count ); g_callbacks[idx].used = false; }

static bool read_data(size_t idx) { bool ret = g_callbacks[idx].dataAvailable; g_callbacks[idx].dataAvailable = false; return ret; } // returns immediately
static void set_read_awaiting_handle(size_t idx, std::experimental::coroutine_handle<> awaiting) { g_callbacks[idx].awaiting = awaiting; } // returns immediately


#endif // TEST_EX_2_BASE_H

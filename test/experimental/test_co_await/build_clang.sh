clang++-7 test_co_await.cpp co_await_hierarchy.cpp ../../../safe_memory/library/gcc_lto_workaround/gcc_lto_workaround.cpp ../../../safe_memory/library/src/iibmalloc/src/iibmalloc.cpp  ../../../safe_memory/library/src/iibmalloc/src/foundation/page_allocator.cpp ../../../safe_memory/library/src/iibmalloc/src/foundation/src/log.cpp ../../../safe_memory/library/src/iibmalloc/src/foundation/3rdparty/fmt/src/format.cc ../../../safe_memory/library/src/iibmalloc/src/foundation/src/safe_memory_error.cpp ../../../safe_memory/library/src/safe_ptr.cpp ../../../safe_memory/library/src/iibmalloc/src/foundation/src/stack_info.cpp ../../../safe_memory/library/src/iibmalloc/src/foundation/src/std_error.cpp ../../../safe_memory/library/src/iibmalloc/src/foundation/src/tagged_ptr_impl.cpp -I../../../safe_memory/library/src/iibmalloc/src/foundation/include -I../../../safe_memory/library/src/iibmalloc/src/foundation/3rdparty/fmt/include -I../../../safe_memory/library/src/iibmalloc/src -I../../../safe_memory/library/src -I../../../safe_memory/library/include -I../../../include -I../../../src -std=gnu++17 -g -Wall -Wextra  -Wno-c++17-extensions -fcoroutines-ts -stdlib=libc++ -lc++experimental -Wno-unused-variable -Wno-unused-parameter -Wno-empty-body  -DNDEBUG -O3 -flto -lpthread -o test.bin
clang++-9 ../../../src/infra_main.cpp ../user_code/HttpServerWithHandlersSample.cpp ../../../src/net.cpp ../../../src/infrastructure.cpp ../../../src/tcp_socket/tcp_socket.cpp ../../../src/clustering_impl/clustering.cpp ../../../safe_memory/library/gcc_lto_workaround/gcc_lto_workaround.cpp ../../../safe_memory/library/src/iibmalloc/src/iibmalloc.cpp ../../../safe_memory/library/src/iibmalloc/src/foundation/src/page_allocator.cpp ../../../safe_memory/library/src/iibmalloc/src/foundation/src/nodecpp_assert.cpp ../../../safe_memory/library/src/iibmalloc/src/foundation/src/log.cpp ../../../safe_memory/library/src/iibmalloc/src/foundation/src/std_error.cpp ../../../safe_memory/library/src/iibmalloc/src/foundation/src/safe_memory_error.cpp ../../../safe_memory/library/src/iibmalloc/src/foundation/src/tagged_ptr_impl.cpp ../../../safe_memory/library/src/iibmalloc/src/foundation/3rdparty/fmt/src/format.cc -I../../../safe_memory/library/src/iibmalloc/src/foundation/include -I../../../safe_memory/library/src/iibmalloc/src/foundation/3rdparty/fmt/include -I../../../safe_memory/library/src/iibmalloc/src -I../../../safe_memory/library/src -I../../../include -I../../../src -std=c++2a -g -Wall -Wextra -Wno-unknown-attributes -Wno-c++2a-extensions -fcoroutines-ts -stdlib=libc++ -Wno-unused-variable -Wno-unused-parameter -Wno-empty-body -DNDEBUG -O3 -flto=thin -flto-jobs=0 -lpthread  -o server.bin
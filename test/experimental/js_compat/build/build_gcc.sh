g++ ../user_code/JsCompatMain.cpp ../../../../src/js_compat/js_compat.cpp ../../../../safe_memory/library/gcc_lto_workaround/gcc_lto_workaround.cpp ../../../../safe_memory/library/src/iibmalloc/src/iibmalloc.cpp  ../../../../safe_memory/library/src/iibmalloc/src/page_allocator_linux.cpp ../../../../safe_memory/library/src/iibmalloc/src/foundation/src/log.cpp ../../../../safe_memory/library/src/iibmalloc/src/foundation/src/std_error.cpp ../../../../safe_memory/library/src/iibmalloc/src/foundation/src/safe_memory_error.cpp ../../../../safe_memory/library/src/iibmalloc/src/foundation/src/tagged_ptr_impl.cpp ../../../../safe_memory/library/src/iibmalloc/src/foundation/3rdparty/fmt/src/format.cc -I../../../../safe_memory/library/src/iibmalloc/src/foundation/include -I../../../../safe_memory/library/src/iibmalloc/src/foundation/3rdparty/fmt/include -I../../../../safe_memory/library/src/iibmalloc/src -I../../../../safe_memory/library/src -I../../../../include -I../../../../src -std=c++17 -DUSING_T_SOCKETS -g -Wall -Wextra -Wno-unused-variable -Wno-unused-parameter -Wno-empty-body -DNDEBUG -O2 -flto -lpthread -o test.bin
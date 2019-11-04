

nodecpp-instrument ../user_code/NetSocket.cpp

set FILES=../user_code/NetSocket.dz.cpp
set FILES=%FILES% ../../../../src/infra_main.cpp
set FILES=%FILES% ../../../../src/net.cpp
set FILES=%FILES% ../../../../src/infrastructure.cpp
set FILES=%FILES% ../../../../src/tcp_socket/tcp_socket.cpp
set FILES=%FILES% ../../../../safe_memory/library/src/iibmalloc/src/iibmalloc_windows.cpp
set FILES=%FILES% ../../../../safe_memory/library/src/iibmalloc/src/page_allocator_windows.cpp
set FILES=%FILES% ../../../../safe_memory/library/src/iibmalloc/src/foundation/src/log.cpp
set FILES=%FILES% ../../../../safe_memory/library/src/iibmalloc/src/foundation/src/std_error.cpp
set FILES=%FILES% ../../../../safe_memory/library/src/iibmalloc/src/foundation/src/safe_memory_error.cpp
set FILES=%FILES% ../../../../safe_memory/library/src/iibmalloc/src/foundation/src/tagged_ptr_impl.cpp
set FILES=%FILES% ../../../../safe_memory/library/src/iibmalloc/src/foundation/3rdparty/fmt/src/format.cc

set INCL=/I../../../../safe_memory/library/src/iibmalloc/src/foundation/include
set INCL=%INCL% /I../../../../safe_memory/library/src/iibmalloc/src/foundation/3rdparty/fmt/include
set INCL=%INCL% /I../../../../safe_memory/library/src/iibmalloc/src
set INCL=%INCL% /I../../../../safe_memory/library/src
set INCL=%INCL% /I../../../../src
set INCL=%INCL% /I../../../../include

cl %FILES% %INCL% /W3 /EHa /MD /await /std:c++17 /DNDEBUG /DUSING_T_SOCKETS /Fe:dz-http-server.exe

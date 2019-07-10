cmake_minimum_required(VERSION 3.4)
project(node_cpp)

find_package(Threads)

set(CLIENT_NAME client.bin)
#set(CTRL_CLIENT_NAME ctrl_client.bin)
set(SERVER_NAME server.bin)
set(LIB_NAME gcc_lto_workaround)

set(SOURCE_EXE ${PROJECT_SOURCE_DIR}/src/infra_main.cpp 					
					${PROJECT_SOURCE_DIR}/src/loop.cpp 
					${PROJECT_SOURCE_DIR}/src/net.cpp 
					${PROJECT_SOURCE_DIR}/src/infrastructure.cpp 
					${PROJECT_SOURCE_DIR}/src/tcp_socket/tcp_socket.cpp )

set(CLIENT_SOURCE_EXE  ${PROJECT_SOURCE_DIR}/test/samples/o_awaitable_client/user_code/NetSocket.cpp)
#set(CTRL_CLIENT_SOURCE_EXE  ${PROJECT_SOURCE_DIR}/test/samples/ctrl_client/user_code/NetSocket.cpp)
set(SERVER_SOURCE_EXE  ${PROJECT_SOURCE_DIR}/test/samples/o_awaitable_server/user_code/NetSocket.cpp)

set(SOURCE_LIB ${PROJECT_SOURCE_DIR}/safe_memory/library/gcc_lto_workaround/gcc_lto_workaround.cpp)

add_subdirectory(${PROJECT_SOURCE_DIR}/safe_memory/library/src/iibmalloc build)

include_directories(include  ${PROJECT_SOURCE_DIR}/safe_memory/library/src/iibmalloc/src/foundation/3rdparty/fmt/include 
							 ${PROJECT_SOURCE_DIR}/safe_memory/library/src/iibmalloc/src/foundation/include 
							 ${PROJECT_SOURCE_DIR}/safe_memory/library/src/iibmalloc/src  							 
							 ${PROJECT_SOURCE_DIR}/safe_memory/library/src 
							 ${PROJECT_SOURCE_DIR}/include	
							 {PROJECT_SOURCE_DIR}/src )

if(MSVC OR MSYS OR MINGW)
	option(CMAKE_USE_WIN32_THREADS_INIT "using WIN32 threads" ON)
	add_definitions(/W3)
	add_definitions(/EHa)
	add_definitions(/MD)
else()
	enable_testing()
	add_definitions( -g )
	add_definitions( -Wall )
	add_definitions( -Wextra )
	add_definitions( -Wno-unused-variable )
	add_definitions( -Wno-unused-parameter )
	add_definitions( -Wno-empty-body )
	add_definitions(-lpthread)
	add_compile_options(-O2)
	set(CMAKE_CXX_STANDARD_REQUIRED on)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAG} -DNDEBUG -DUSING_T_SOCKETS ")
endif()

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/bin)

add_library( ${LIB_NAME} SHARED ${SOURCE_LIB})
add_executable(${CLIENT_NAME} ${SOURCE_EXE} ${CLIENT_SOURCE_EXE})
#add_executable(${CTRL_CLIENT_NAME} ${SOURCE_EXE} ${CTRL_CLIENT_SOURCE_EXE})
add_executable(${SERVER_NAME} ${SOURCE_EXE} ${SERVER_SOURCE_EXE})

target_link_libraries( ${CLIENT_NAME} iibmalloc ${LIB_NAME}  pthread)
target_link_libraries( ${SERVER_NAME} iibmalloc ${LIB_NAME}  pthread)

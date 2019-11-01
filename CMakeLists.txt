cmake_minimum_required(VERSION 3.4)
project(node_cpp)

find_package(Threads)

set(CLIENT_NAME client.bin)
#set(CTRL_CLIENT_NAME ctrl_client.bin)
set(SERVER_NAME server.bin)
set(LIB_NAME gcc_lto_workaround)

set(SOURCE_EXE ${PROJECT_SOURCE_DIR}/src/infra_main.cpp 					
					${PROJECT_SOURCE_DIR}/src/net.cpp 
					${PROJECT_SOURCE_DIR}/src/infrastructure.cpp 
					${PROJECT_SOURCE_DIR}/src/tcp_socket/tcp_socket.cpp )

set(CLIENT_SOURCE_EXE  ${PROJECT_SOURCE_DIR}/test/samples/client/user_code/NetSocket.cpp)
#set(CTRL_CLIENT_SOURCE_EXE  ${PROJECT_SOURCE_DIR}/test/samples/ctrl_client/user_code/NetSocket.cpp)
set(SERVER_SOURCE_EXE  ${PROJECT_SOURCE_DIR}/test/samples/server/user_code/NetSocket.cpp)

set(SOURCE_LIB ${PROJECT_SOURCE_DIR}/safe_memory/library/gcc_lto_workaround/gcc_lto_workaround.cpp)

add_subdirectory(${PROJECT_SOURCE_DIR}/safe_memory/library/src/iibmalloc build)

include_directories(include	${PROJECT_SOURCE_DIR}/safe_memory/library/src/iibmalloc/src/foundation/3rdparty/fmt/include 
							${PROJECT_SOURCE_DIR}/safe_memory/library/src/iibmalloc/src/foundation/include 
							${PROJECT_SOURCE_DIR}/safe_memory/library/src/iibmalloc/src  							 
							${PROJECT_SOURCE_DIR}/safe_memory/library/src
							${PROJECT_SOURCE_DIR}/include/nodecpp	
							${PROJECT_SOURCE_DIR}/include	
							${PROJECT_SOURCE_DIR}/src )

set(DZ_INCLUDES -isystem ${PROJECT_SOURCE_DIR}/safe_memory/library/src/iibmalloc/src/foundation/3rdparty/fmt/include
				-isystem ${PROJECT_SOURCE_DIR}/safe_memory/library/src/iibmalloc/src/foundation/include 
				-isystem ${PROJECT_SOURCE_DIR}/safe_memory/library/src/iibmalloc/src 
				-isystem ${PROJECT_SOURCE_DIR}/safe_memory/library/src
				-isystem ${PROJECT_SOURCE_DIR}/include/nodecpp
				-isystem ${PROJECT_SOURCE_DIR}/include
				-isystem ${PROJECT_SOURCE_DIR}/src
				)

set(DZ_DEFINES -DNDEBUG -DUSING_T_SOCKETS)


if(MSVC OR MSYS OR MINGW)
	option(CMAKE_USE_WIN32_THREADS_INIT "using WIN32 threads" ON)
	add_definitions(/W3)
	add_definitions(/EHa)
	add_definitions(/MD)
	add_definitions(/await)
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

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	add_library( ${LIB_NAME} SHARED ${SOURCE_LIB})
else()
	unset(LIB_NAME)
endif()

add_library( ${LIB_NAME} SHARED ${SOURCE_LIB})
add_executable(${CLIENT_NAME} ${SOURCE_EXE} ${CLIENT_SOURCE_EXE})
#add_executable(${CTRL_CLIENT_NAME} ${SOURCE_EXE} ${CTRL_CLIENT_SOURCE_EXE})
add_executable(${SERVER_NAME} ${SOURCE_EXE} ${SERVER_SOURCE_EXE})

if(MSVC)
	target_link_libraries( ${CLIENT_NAME} iibmalloc ${LIB_NAME})
	target_link_libraries( ${SERVER_NAME} iibmalloc ${LIB_NAME})
else()
	target_link_libraries( ${CLIENT_NAME} iibmalloc ${LIB_NAME} pthread)
	target_link_libraries( ${SERVER_NAME} iibmalloc ${LIB_NAME} pthread)
endif()


# -----------------------------------------------------------------------------
# Below this line targets to build intrumented (dezombiefied) client and server
# So far only tested with ninja generator under windows
# target names are 'dz-client.bin' and 'dz-server.bin'
# -----------------------------------------------------------------------------

function(make_dz_files OUT_VAR LIST1 LIST2)

	foreach(EACH IN LISTS ${LIST1} ${LIST2})

		get_filename_component(P ${EACH} DIRECTORY)
		get_filename_component(N ${EACH} NAME_WE)
		get_filename_component(E ${EACH} EXT)

		string(CONCAT DZ_NAME ${P} "/" ${N} ".dz" ${E})

		add_custom_command(
		    OUTPUT  ${DZ_NAME}
		    COMMAND nodecpp-instrument ${EACH} -- -fcoroutines-ts -fnew-alignment=16 -std=c++17 ${DZ_DEFINES} ${DZ_INCLUDES}
		)

		list(APPEND DZ_FILES ${DZ_NAME})

	endforeach()
	set(${OUT_VAR} ${DZ_FILES} PARENT_SCOPE)
endfunction()


make_dz_files(DZ_CLIENT_FILES SOURCE_EXE CLIENT_SOURCE_EXE)

add_executable(dz-client.bin ${DZ_CLIENT_FILES})

make_dz_files(DZ_SERVER_FILES SOURCE_EXE SERVER_SOURCE_EXE)

add_executable(dz-server.bin ${DZ_SERVER_FILES})


if(MSVC)
	target_link_libraries( dz-client.bin iibmalloc ${LIB_NAME})
	target_link_libraries( dz-server.bin iibmalloc ${LIB_NAME})
else()
	target_link_libraries( dz-client.bin iibmalloc ${LIB_NAME} pthread)
	target_link_libraries( dz-server.bin iibmalloc ${LIB_NAME} pthread)
endif()

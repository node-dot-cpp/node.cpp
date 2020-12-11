#-------------------------------------------------------------------------------------------
# Copyright (c) 2020, OLogN Technologies AG
#-------------------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.4)
project(NodeDotCpp CXX)
set(CMAKE_CXX_STANDARD 17)
include(CTest)

set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/safe_memory/library/src/iibmalloc/src/foundation/tools")
include(CommonCppFlags)


if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    # first thing so it applies to all dependencies
	if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        string( REPLACE "/EHsc" "/EHa" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" )
    endif()
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        # coroutine needs libc++ at this time
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++")
    endif()
endif()



add_subdirectory(safe_memory)

# just in case some project needs a different main
# we define three libraries, 'nodecpp', 'nodecpp_no_main', and 'nodecpp_no_main_q_based_infra'

add_library(nodecpp_no_main STATIC
   	src/infrastructure.cpp
  	src/net.cpp
#  	src/timers.cpp
    src/clustering_impl/clustering.cpp
#    src/logging_impl/logging.cpp
    src/tcp_socket/listener_thread.cpp
    src/tcp_socket/tcp_socket.cpp
    src/timeout_manager.cpp
    src/fs.cpp
)

target_compile_definitions(nodecpp_no_main PUBLIC NODECPP_ENABLE_CLUSTERING)
target_compile_definitions(nodecpp_no_main PUBLIC NODECPP_USE_SAFE_MEMORY_CONTAINERS)

target_include_directories(nodecpp_no_main
  PUBLIC include
  PUBLIC src
)

add_library(nodecpp_no_main_q_based_infra STATIC
	src/q_based_infrastructure.cpp
	src/timeout_manager.cpp
	src/common.cpp
	src/fs.cpp
)

target_compile_definitions(nodecpp_no_main_q_based_infra PUBLIC NODECPP_USE_Q_BASED_INFRA)
target_compile_definitions(nodecpp_no_main_q_based_infra PUBLIC NODECPP_USE_SAFE_MEMORY_CONTAINERS)

target_include_directories(nodecpp_no_main_q_based_infra
  PUBLIC include
  PUBLIC src
)

add_library(nodecpp STATIC
	src/infra_main.cpp
)


target_link_libraries(nodecpp_no_main safe_memory)
target_link_libraries(nodecpp_no_main_q_based_infra safe_memory)
target_link_libraries(nodecpp nodecpp_no_main)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
	if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
		target_compile_options(nodecpp_no_main PUBLIC /await)
		target_compile_options(nodecpp_no_main_q_based_infra PUBLIC /await)
	endif()
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")

	target_compile_definitions(nodecpp_no_main PUBLIC USING_T_SOCKETS)

	if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
		target_compile_definitions(nodecpp PUBLIC NODECPP_NO_STACK_INFO_IN_EXCEPTIONS)
		target_compile_options(nodecpp_no_main PUBLIC -Wno-unknown-attributes -Wno-c++17-extensions)
		target_compile_options(nodecpp_no_main PUBLIC -fcoroutines-ts)
#        set_target_properties(foundation PROPERTIES LINK_OPTIONS -lc++)
	endif()

elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")

	target_compile_definitions(nodecpp_no_main PUBLIC USING_T_SOCKETS)
	if (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
		target_compile_definitions(nodecpp PUBLIC NODECPP_NO_STACK_INFO_IN_EXCEPTIONS)
		target_compile_options(nodecpp_no_main PUBLIC -Wno-unknown-attributes -Wno-c++17-extensions)
	endif()
endif()


#-------------------------------------------------------------------------------------------
# Tests and samples
#-------------------------------------------------------------------------------------------


add_executable(HttpServerSample
	samples/http_server/user_code/HttpServerSample.cpp
)

target_link_libraries(HttpServerSample nodecpp)

add_executable(HttpServerWithHandlersSample
	samples/http_server_handlers/user_code/HttpServerWithHandlersSample.cpp
)

target_link_libraries(HttpServerWithHandlersSample nodecpp)

add_executable(TcpClientSample
	samples/tcp_client/user_code/TcpClientSample.cpp
)

target_link_libraries(TcpClientSample nodecpp)

add_executable(TcpClientWithHandlersSample
	samples/tcp_client_handlers/user_code/TcpClientWithHandlersSample.cpp
)

target_link_libraries(TcpClientWithHandlersSample nodecpp)

add_executable(TcpServerSample
	samples/tcp_server/user_code/TcpServerSample.cpp
)

target_link_libraries(TcpServerSample nodecpp)

add_executable(TcpServerWithHandlersSample
	samples/tcp_server_handlers/user_code/TcpServerWithHandlersSample.cpp
)

target_link_libraries(TcpServerWithHandlersSample nodecpp)

add_executable(QBasedInfraSample
	test/experimental/q_based_infra/user_code/main.cpp
)

target_link_libraries(QBasedInfraSample nodecpp_no_main_q_based_infra)



# -----------------------------------------------------------------------------
# Below this line targets to run checker and dezombiefier intrumentation
# So far only tested with ninja generator under windows
# -----------------------------------------------------------------------------

set(DZ_INCLUDES 
    # this five path are for ubuntu libc++
    -resource-dir /usr/lib/llvm-9/lib/clang/9.0.0
    -isystem /usr/lib/llvm-9/bin/../include/c++/v1
    -isystem /usr/lib/llvm-9/lib/clang/9.0.0/include

    #this two path are for MaxOS libc++
    -isystem /Library/Developer/CommandLineTools/usr/include/c++/v1
    -isystem /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include

    #all path below are from nodecpp
    -isystem ${CMAKE_CURRENT_SOURCE_DIR}/safe_memory/library/src/iibmalloc/src/foundation/3rdparty/fmt/include
    -isystem ${CMAKE_CURRENT_SOURCE_DIR}/safe_memory/library/src/iibmalloc/src/foundation/include 
    -isystem ${CMAKE_CURRENT_SOURCE_DIR}/safe_memory/library/src/iibmalloc/src 
    -isystem ${CMAKE_CURRENT_SOURCE_DIR}/safe_memory/library/include
    -isystem ${CMAKE_CURRENT_SOURCE_DIR}/safe_memory/library/src
    -isystem ${CMAKE_CURRENT_SOURCE_DIR}/include/nodecpp
    -isystem ${CMAKE_CURRENT_SOURCE_DIR}/include
    -isystem ${CMAKE_CURRENT_SOURCE_DIR}/src
)

set(DZ_DEFINES -DNDEBUG -DNODECPP_ENABLE_CLUSTERING)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
	set(DZ_FLAGS -std=c++17 -fcoroutines-ts)
else()
	set(DZ_FLAGS -std=c++17 -fcoroutines-ts -stdlib=libc++)
endif()


function(dz_add_executable ATARGET)

    get_target_property(TARGET_SOURCE_DIR ${ATARGET} SOURCE_DIR)
    get_target_property(TARGET_BINARY_DIR ${ATARGET} BINARY_DIR)
    get_target_property(TARGET_SOURCES ${ATARGET} SOURCES)

	foreach(EACH IN LISTS TARGET_SOURCES)

		get_filename_component(P ${EACH} DIRECTORY)
		get_filename_component(N ${EACH} NAME_WE)
		get_filename_component(E ${EACH} EXT)

#		string(CONCAT DZ_NAME ${TARGET_BINARY_DIR} "/" ${P} "/" ${N} ".dz" ${E})
# mb:use TARGET_SOURCE_DIR until we make local includes work
		string(CONCAT DZ_NAME ${TARGET_SOURCE_DIR} "/" ${P} "/" ${N} ".dz" ${E})
		string(CONCAT FULL_NAME ${TARGET_SOURCE_DIR} "/" ${EACH})

		add_custom_command(
		    OUTPUT  ${DZ_NAME}
		    COMMAND nodecpp-checker ${FULL_NAME} -- ${DZ_FLAGS} ${DZ_DEFINES} ${DZ_INCLUDES}
		    COMMAND nodecpp-instrument ${FULL_NAME} -o=${DZ_NAME} -- ${DZ_FLAGS} ${DZ_DEFINES} ${DZ_INCLUDES}
#			MAIN_DEPENDENCY ${FULL_NAME}
		)

		list(APPEND DZ_FILES ${DZ_NAME})

	endforeach()
	
    get_target_property(TARGET_NAME ${ATARGET} NAME)
    string(CONCAT DZ_TARGET_NAME ${TARGET_NAME} "-dz")
    
	add_executable(${DZ_TARGET_NAME} ${DZ_FILES})
	target_link_libraries(${DZ_TARGET_NAME} nodecpp)

endfunction()

option(NODECPP_CHECK_AND_DZ_SAMPLES "Check and Dezombiefy samples" OFF)

if(NODECPP_CHECK_AND_DZ_SAMPLES)
	dz_add_executable(HttpServerSample)
	dz_add_executable(HttpServerWithHandlersSample)
	dz_add_executable(TcpServerSample)
	dz_add_executable(TcpServerWithHandlersSample)
	dz_add_executable(TcpClientSample)
	dz_add_executable(TcpClientWithHandlersSample)
endif()

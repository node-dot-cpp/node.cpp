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
# we define two libraries, 'nodecpp' and 'nodecpp_no_main'

add_library(nodecpp_no_main STATIC
   	src/infrastructure.cpp
  	src/net.cpp
#  	src/timers.cpp
#    src/clustering_impl/clustering.cpp
#    src/logging_impl/logging.cpp
#    src/tcp_socket/listener_thread.cpp
    src/tcp_socket/tcp_socket.cpp
)

target_include_directories(nodecpp_no_main
  PUBLIC include
  PUBLIC src
)

add_library(nodecpp STATIC
	src/infra_main.cpp
)


target_link_libraries(nodecpp_no_main safe_memory)
target_link_libraries(nodecpp nodecpp_no_main)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
	if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
		target_compile_options(nodecpp_no_main PUBLIC /await)
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


add_executable(TcpClientSample
	samples/tcp_client/user_code/TcpClientSample.cpp
)

target_link_libraries(TcpClientSample nodecpp)

add_executable(TcpServerSample
	samples/tcp_server/user_code/TcpServerSample.cpp
)

target_link_libraries(TcpServerSample nodecpp)

add_executable(HttpServerSample
	samples/http_server/user_code/HttpServerSample.cpp
)

target_link_libraries(HttpServerSample nodecpp)


set(DZ_INCLUDES -isystem ${PROJECT_SOURCE_DIR}/safe_memory/library/src/iibmalloc/src/foundation/3rdparty/fmt/include
				-isystem ${PROJECT_SOURCE_DIR}/safe_memory/library/src/iibmalloc/src/foundation/include 
				-isystem ${PROJECT_SOURCE_DIR}/safe_memory/library/src/iibmalloc/src 
				-isystem ${PROJECT_SOURCE_DIR}/safe_memory/library/src
				-isystem ${PROJECT_SOURCE_DIR}/include/nodecpp
				-isystem ${PROJECT_SOURCE_DIR}/include
				-isystem ${PROJECT_SOURCE_DIR}/src
				)

set(DZ_DEFINES -DNDEBUG -DUSING_T_SOCKETS)




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
		    COMMAND nodecpp-instrument ${EACH} -- -fcoroutines-ts -std=c++17 ${DZ_DEFINES} ${DZ_INCLUDES}
		)

		list(APPEND DZ_FILES ${DZ_NAME})

	endforeach()
	set(${OUT_VAR} ${DZ_FILES} PARENT_SCOPE)
endfunction()


#make_dz_files(DZ_CLIENT_FILES SOURCE_EXE CLIENT_SOURCE_EXE)

#add_executable(dz-client.bin ${DZ_CLIENT_FILES})

#make_dz_files(DZ_SERVER_FILES SOURCE_EXE SERVER_SOURCE_EXE)

#add_executable(dz-server.bin ${DZ_SERVER_FILES})


if(MSVC)
#	target_link_libraries( dz-client.bin iibmalloc ${LIB_NAME})
#	target_link_libraries( dz-server.bin iibmalloc ${LIB_NAME})
else()
#	target_link_libraries( dz-client.bin iibmalloc ${LIB_NAME} pthread)
#	target_link_libraries( dz-server.bin iibmalloc ${LIB_NAME} pthread)
endif()

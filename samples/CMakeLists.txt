#-------------------------------------------------------------------------------------------
# Copyright (c) 2021, OLogN Technologies AG
#-------------------------------------------------------------------------------------------

#-------------------------------------------------------------------------------------------
# Samples
#-------------------------------------------------------------------------------------------

if(NODECPP_CHECKER_ON_SAMPLES)
#	set(CMAKE_CXX_CLANG_TIDY "safememory-checker --no-library-db")
	set(CMAKE_CXX_CLANG_TIDY "safememory-checker")
endif()

add_executable(HttpServerSample	http_server/user_code/HttpServerSample.cpp)

target_link_libraries(HttpServerSample nodecpp)

add_executable(HttpServerWithHandlersSample	http_server_handlers/user_code/HttpServerWithHandlersSample.cpp)

target_link_libraries(HttpServerWithHandlersSample nodecpp)

add_executable(TcpClientSample tcp_client/user_code/TcpClientSample.cpp)

target_link_libraries(TcpClientSample nodecpp)

add_executable(TcpClientWithHandlersSample tcp_client_handlers/user_code/TcpClientWithHandlersSample.cpp)

target_link_libraries(TcpClientWithHandlersSample nodecpp)

add_executable(TcpServerSample tcp_server/user_code/TcpServerSample.cpp)

target_link_libraries(TcpServerSample nodecpp)

add_executable(TcpServerWithHandlersSample tcp_server_handlers/user_code/TcpServerWithHandlersSample.cpp)

target_link_libraries(TcpServerWithHandlersSample nodecpp)


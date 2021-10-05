cmake_minimum_required(VERSION 3.19)
project(async_coRoutines)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++2a -pthread -fcoroutines")

set(CMAKE_BINARY_DIR ${PROJECT_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

set(default_build_type "Debug")
if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
    set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE STRING "Choose the type of build." FORCE)
endif()

add_compile_options(
        "-Wno-deprecated-declarations" "-Wall" "-Werror" "-Wno-error=unused-parameter" "-Wextra" "-g3" "-ggdb3"
        "$<$<CONFIG:Debug>:-O0;-DNRELEASE>"
        "$<$<CONFIG:Release>:-O3;-DRELEASE>"
)

include_directories(/usr/local/asio-1.18.2/include)

add_executable(tcp_server tcp_server.cpp common.h FifoCircularMessageBuffer.h CoRoutineSocketSenderAndReceiver.h)
add_executable(tcp_client tcp_client.cpp common.h FifoCircularMessageBuffer.h CoRoutineSocketSenderAndReceiver.h)
add_executable(udp_client udp_client.cpp common.h FifoCircularMessageBuffer.h CoRoutineSocketSenderAndReceiver.h)

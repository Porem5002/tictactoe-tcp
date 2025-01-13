cmake_minimum_required(VERSION 3.15)

project(tictactoe)

if(MSVC)
    add_compile_options("/MT")
endif()

set(WINSOCK_LIB ws2_32.lib)
set(QUARTZ_LIB "${CMAKE_SOURCE_DIR}/external/quartz/lib/quartz.lib")
set(EXTERNAL_PATH "external")

file(GLOB SHARED_SRCS "shared/*.c" "shared/*.h")
file(GLOB CLIENT_SRCS "client/*.c" "client/*.c")
file(GLOB SERVER_SRCS "server/*.c" "server/*.c")

add_executable(tictactoe ${CLIENT_SRCS} ${SHARED_SRCS})
add_executable(tictactoe-server ${SERVER_SRCS} ${SHARED_SRCS})

target_include_directories(tictactoe PRIVATE ${EXTERNAL_PATH})

target_link_libraries(tictactoe PRIVATE ${WINSOCK_LIB} ${QUARTZ_LIB})
target_link_libraries(tictactoe-server PRIVATE ${WINSOCK_LIB})
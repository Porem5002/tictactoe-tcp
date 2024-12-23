cmake_minimum_required(VERSION 3.15)

project(tictactoe)

set(WINSOCK_LIB ws2_32.lib)
set(QUARTZ_LIB "${CMAKE_SOURCE_DIR}/external/quartz.lib")

add_executable(tictactoe src/main.c src/scene.c src/menu.c src/localhost.c src/base.c src/ui.c src/ai.c)
add_executable(client net/client.c src/base.c src/ui.c net/net.c)
add_executable(server net/server.c src/base.c net/net.c)

target_link_libraries(tictactoe PRIVATE ${WINSOCK_LIB} ${QUARTZ_LIB})
target_link_libraries(client PRIVATE ${WINSOCK_LIB} ${QUARTZ_LIB})
target_link_libraries(server PRIVATE ${WINSOCK_LIB})
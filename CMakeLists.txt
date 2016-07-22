cmake_minimum_required(VERSION 2.8)

set(BIN server_make)

project( ${BIN} )

set (LIBS boost_system pthread boost_thread)

add_executable(${BIN} server.cpp)

target_link_libraries(${BIN} ${LIBS})

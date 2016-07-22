cmake_minimum_required(VERSION 2.8)

set(BIN final)

project( ${BIN} )

set( CMAKE_CXX_FLAGS "-O0 -std=c++11" ) 

set (LIBS boost_system pthread boost_thread)

add_executable(${BIN} server.cpp)

target_link_libraries(${BIN} ${LIBS})

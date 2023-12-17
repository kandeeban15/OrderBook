cmake_minimum_required(VERSION 3.26)
project(orderbook)

set(CMAKE_CXX_STANDARD 17)

add_executable(orderbook main.cpp)

target_link_libraries(orderbook pthread)

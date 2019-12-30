cmake_minimum_required (VERSION 3.0)

project (memory_ordering)

add_executable(memory_ordering sequential.cpp)
set_property(TARGET memory_ordering PROPERTY CXX_STANDARD 17)
SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-pthread")

find_package(Catch2 CONFIG REQUIRED)

target_link_libraries(memory_ordering PRIVATE Catch2::Catch2)
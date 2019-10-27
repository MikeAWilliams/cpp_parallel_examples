cmake_minimum_required (VERSION 3.0)
project (parallel_max_element)

add_executable(parallel_max_element main.cpp)
set_property(TARGET parallel_max_element PROPERTY CXX_STANDARD 17)
SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-pthread")

find_package(Catch2 CONFIG REQUIRED)
find_package(tl-function-ref CONFIG REQUIRED)

target_link_libraries(parallel_max_element PRIVATE Catch2::Catch2)
target_link_libraries(parallel_max_element PRIVATE tl::function-ref)
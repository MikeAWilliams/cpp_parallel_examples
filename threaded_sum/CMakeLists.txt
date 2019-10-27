cmake_minimum_required (VERSION 3.0)

project (parallel_sum)

add_executable(parallel_sum main.cpp)
set_property(TARGET parallel_sum PROPERTY CXX_STANDARD 17)
SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-pthread")

find_package(Catch2 CONFIG REQUIRED)
find_package(tl-function-ref CONFIG REQUIRED)

target_link_libraries(parallel_sum PRIVATE Catch2::Catch2)
target_link_libraries(parallel_sum PRIVATE tl::function-ref)
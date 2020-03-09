cmake_minimum_required (VERSION 3.0)

project (transform_reduce)

add_executable(transform_reduce main.cpp)
set_property(TARGET transform_reduce PROPERTY CXX_STANDARD 17)
SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-pthread")

find_package(Catch2 CONFIG REQUIRED)
find_package(tl-function-ref CONFIG REQUIRED)

target_link_libraries(transform_reduce PRIVATE Catch2::Catch2)
target_link_libraries(transform_reduce PRIVATE tl::function-ref)
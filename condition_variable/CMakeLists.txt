cmake_minimum_required (VERSION 3.0)

project (condition_variable)

add_executable(condition_variable main.cpp)
set_property(TARGET condition_variable PROPERTY CXX_STANDARD 17)
SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-pthread")

find_package(Catch2 CONFIG REQUIRED)
find_package(tl-function-ref CONFIG REQUIRED)

target_link_libraries(condition_variable PRIVATE Catch2::Catch2)
target_link_libraries(condition_variable PRIVATE tl::function-ref)
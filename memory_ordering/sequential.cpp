#define CATCH_CONFIG_MAIN 
#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include "catch2/catch.hpp"

#include <atomic>
#include <thread>

TEST_CASE("sequential")
{
    REQUIRE(false);
}
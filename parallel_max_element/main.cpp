#define CATCH_CONFIG_MAIN 
#include "catch2/catch.hpp"

#include <algorithm>
#include <vector>

// TODO after making this work as is, add overloads.
// take a comparison function
// take a number of threads allowed
// take a thread provider or manager
template<typename iterator>
iterator ParalellMaxElement(iterator begin, iterator end)
{
    return begin;
}

TEST_CASE("Happy path")
{
    const std::vector<int> input{4, 7, 3, 6};
    REQUIRE(*std::max_element(input.cbegin(), input.cend()) == *ParalellMaxElement(input.cbegin(), input.cend()));
}
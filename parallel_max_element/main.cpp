#define CATCH_CONFIG_MAIN 
#include "catch2/catch.hpp"

#include <algorithm>
#include <vector>

std::vector<double> GetTestData(const size_t elements, const double mean, const double standardDeviation)
{
    std::random_device device;
    std::mt19937 generator(device());
    std::normal_distribution<double> distribution(mean, standardDeviation);

    std::vector<double> result;
    result.reserve(elements);
    for(size_t i{0}; i < elements; ++i)
    {
        result.emplace_back(distribution(generator));
    }
    return result;
}
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
    const auto input {GetTestData(100'000, 500, 200)};
    REQUIRE(*std::max_element(input.cbegin(), input.cend()) == *ParalellMaxElement(input.cbegin(), input.cend()));
}
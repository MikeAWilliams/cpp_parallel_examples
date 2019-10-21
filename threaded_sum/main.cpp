#define CATCH_CONFIG_MAIN 
#include "catch2/catch.hpp"

#include <random>

std::vector<std::vector<bool>> GetTestData(const size_t vectors, const size_t elements)
{
    std::random_device device;
    std::mt19937 generator(device());
    std::uniform_int_distribution<int> distribution(0,1);

    std::vector<std::vector<bool>> result(vectors);
    for(auto & vec : result)
    {
        vec.reserve(elements);
        for(size_t i{0}; i < elements; ++i)
        {
            vec.emplace_back(1 == distribution(generator));
        }
    }
    return result;
}

size_t GetSerialResult(const std::vector<std::vector<bool>>& input)
{
    size_t result {0};
    for(const auto & vec : input)
    {
        for(const auto & item : vec)
        {
            if(item)
            {
                ++result;
            }
        }
    }
    return result;
}

size_t GetParallelResulMutex(const std::vector<std::vector<bool>>& input)
{
    size_t result {0};
    // spawn a number of threads equal to input.size() and have each one add to result
    // use a mutex to protect result
    return result;
}

size_t GetParallelResulAtomic(const std::vector<std::vector<bool>>& input)
{
    size_t result {0};
    // spawn a number of threads equal to input.size() and have each one add to result
    // change result to be atomic
    return result;
}

size_t GetParallelResulPartialSums(const std::vector<std::vector<bool>>& input)
{
    size_t result {0};
    // spawn a number of threads equal to input.size() and have each one add to result
    // instead of operating on result directly use one size_t per thread and sum the result at the end
    return result;
}

TEST_CASE("Run all the tests")
{
    auto testData {GetTestData(12, 100'000)};
    const auto expectedResult {GetSerialResult(testData)};

    SECTION("Mutex")
    {
        REQUIRE(expectedResult == GetParallelResulMutex(testData));
    }

    SECTION("Atomic")
    {
        REQUIRE(expectedResult == GetParallelResulAtomic(testData));
    }

    SECTION("PartialSum")
    {
        REQUIRE(expectedResult == GetParallelResulPartialSums(testData));
    }
}

// Now add performance tests to compare the three methods
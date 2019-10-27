#define CATCH_CONFIG_MAIN 
#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include "catch2/catch.hpp"
#include <tl/function_ref.hpp>

#include <atomic>
#include <functional>
#include <future>
#include <thread>
#include <mutex>
#include <random>
#include <vector>

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

void ComputeSumOnVector(const std::vector<bool>& data, const size_t dataIndex, tl::function_ref<void(const size_t)> sum_function)
{
    for(const auto & item : data)
    {
        if(item)
        {
            sum_function(dataIndex);
        }
    }
}

size_t GetSerialResult(const std::vector<std::vector<bool>>& input)
{
    size_t result {0};
    for(const auto & data : input)
    {
        ComputeSumOnVector(data, 0, [&result](const size_t)
        {
            ++result;
        });
    }
    return result;
}

void ComputeSumUsingVectorOfFutures(const std::vector<std::vector<bool>>& input, tl::function_ref<void(const size_t)> sum_function)
{
    std::vector<std::future<void>> futures;
    futures.reserve(input.size());
    for(size_t index{0}; index < input.size(); ++index)
    {
        const auto &data {input[index]};
        futures.emplace_back(std::async(std::launch::async, [&data, sum_function, index]()
        {
            ComputeSumOnVector(data, index, sum_function); 
        }));
    }
    for(auto & future : futures)
    {
        future.wait();
    }
}

size_t GetParallelResulMutex(const std::vector<std::vector<bool>>& input)
{
    size_t result {0};
    std::mutex mutex;

    ComputeSumUsingVectorOfFutures(input, [&result, &mutex](const size_t)
    {
        std::unique_lock<std::mutex> lock{mutex};
        ++result;
    });
    return result;
}

size_t GetParallelResulAtomic(const std::vector<std::vector<bool>>& input)
{
    std::atomic<size_t> result {0};

    ComputeSumUsingVectorOfFutures(input, [&result](const size_t)
    {
        ++result;
    });
    return result;
}

size_t GetParallelResulPartialSums(const std::vector<std::vector<bool>>& input)
{
    std::vector<size_t> partialSums(input.size(), 0);
    ComputeSumUsingVectorOfFutures(input, [&partialSums](const size_t dataIndex)
    {
        ++partialSums[dataIndex];
    });

    size_t result {0};
    for(const auto & partialSum : partialSums)
    {
        result += partialSum;
    }
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

TEST_CASE("benchmarks")
{
    auto testData {GetTestData(12, 100'000)};
    BENCHMARK("Mutex")
    {
      GetParallelResulMutex(testData);
    };

    BENCHMARK("Atomic")
    {
      GetParallelResulAtomic(testData);
    };

    BENCHMARK("PartialSum")
    {
      GetParallelResulPartialSums(testData);
    };
}
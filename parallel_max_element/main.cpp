#define CATCH_CONFIG_MAIN 
#include "catch2/catch.hpp"

#include <algorithm>
#include <thread>
#include <vector>
#include <iostream>

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

// based on Anthony Williams parallel_accumulate
template<typename iterator>
struct max_block
{
    iterator operator()(iterator first, iterator last, iterator& result)
    {
        result = std::max_element(first, last);
    }
};

int ComputeMaxThreads(const unsigned long length)
{
    const int min_per_thread = 25;
    const int max_threads = (length + min_per_thread - 1) / min_per_thread;

    const int hardware_threads = std::thread::hardware_concurrency();

    if(0 == hardware_threads)
    {
        return std::min(2, max_threads);
    }
    return std::min(hardware_threads, max_threads);
}

// TODO after making this work as is, add overloads.
// take a comparison function
// take a number of threads allowed
// take a thread provider or manager
template<typename iterator>
iterator ParallelMaxElement(iterator begin, iterator end)
{
    const int length = std::distance(begin, end);

    if(0 == length)
    {
        return begin;
    }

    const auto num_threads {ComputeMaxThreads(length)};
    const int block_size = length / num_threads;

    std::vector<iterator> results(num_threads);
    std::vector<std::thread>  threads(num_threads - 1);

    iterator block_start = begin;
    for(unsigned long i=0; i < (num_threads-1); ++i)
    {
        iterator block_end = block_start;
        std::advance(block_end, block_size);
        threads[i] = std::thread(
            max_block<iterator>(), block_start, block_end, std::ref(results[i]));
        block_start = block_end;
    }
    max_block<iterator>()(block_start, end, results[num_threads-1]);
    
    std::for_each(threads.begin(), threads.end(), std::mem_fn(&std::thread::join));

    return *std::max_element(results.begin(), results.end(), 
        [](iterator a, iterator b)
        {
            return *a < *b;
        });
}


TEST_CASE("Parallel max element")
{
    const auto input {GetTestData(100'000, 500, 200)};
    REQUIRE(*std::max_element(input.cbegin(), input.cend()) == *ParallelMaxElement(input.cbegin(), input.cend()));
}
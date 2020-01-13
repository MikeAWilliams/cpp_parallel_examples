#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include "catch2/catch.hpp"

#include <atomic>
#include <future>
#include <iostream>
#include <mutex>
#include <numeric>
#include <optional>
#include <random>
#include <thread>
#include <unordered_set>
#include <vector>

constexpr auto MAX_TO_INSERT {200};

void RemoveBFromA(std::unordered_set<int>& a, const std::unordered_set<int>& b)
{
    for(const auto& item : b)
    {
        auto findIter {a.find(item)};
        REQUIRE(a.end() != findIter);
        a.erase(findIter);
    }
}

template<class Container>
std::unordered_set<int> RemoveItems(Container& container)
{
    std::unordered_set<int> result;
    while(true)
    {
        auto item {container.Retrieve()};
        if(!item.has_value())
        {
            break;
        }
        result.insert(*item);
    }
    return result;
}

template<class Container>
void TestContainer()
{
    std::vector<int> data(MAX_TO_INSERT);
    std::iota(data.begin(), data.end(), 0);
    std::unordered_set<int> allToRemove{data.begin(), data.end()};

    Container testObject(std::move(data));
    auto consume1 {std::async(std::launch::async, [&testObject](){return RemoveItems(testObject);})};
    auto consume2 {std::async(std::launch::async, [&testObject](){return RemoveItems(testObject);})};

    auto removedSet1 {consume1.get()};
    auto removedSet2 {consume2.get()};

    RemoveBFromA(allToRemove, removedSet1);
    RemoveBFromA(allToRemove, removedSet2);
    REQUIRE(0 == allToRemove.size());
}

class ContainerMutex
{
private:
    std::vector<int> m_data;
    int m_index;
    std::mutex m_mutex;

public:
    ContainerMutex(std::vector<int> data)
        : m_data(std::move(data))
        , m_index {static_cast<int>(m_data.size() - 1)}
    {

    }

    std::optional<int> Retrieve()
    {
        std::lock_guard lock{m_mutex};
        if(m_index < 0)
        {
            return {};
        }
        int result {m_data[m_index]};
        --m_index;
        return result;
    }
};

TEST_CASE("Using mutex")
{
    TestContainer<ContainerMutex>();
}

class ContainerSeqCst
{
private:
    std::vector<int> m_data;
    std::atomic<int> m_index;

public:
    ContainerSeqCst(std::vector<int> data)
        : m_data(std::move(data))
        , m_index {static_cast<int>(m_data.size() - 1)}
    {

    }

    std::optional<int> Retrieve()
    {
        auto indexToUse {m_index.fetch_sub(1, std::memory_order_seq_cst)};
        if(indexToUse < 0)
        {
            return {};
        }
        return m_data[indexToUse];
    }
};

TEST_CASE("Using seqcst")
{
    TestContainer<ContainerSeqCst>();
}

class ContainerAcquire
{
private:
    std::vector<int> m_data;
    std::atomic<int> m_index;

public:
    ContainerAcquire(std::vector<int> data)
        : m_data(std::move(data))
        , m_index {static_cast<int>(m_data.size() - 1)}
    {

    }

    std::optional<int> Retrieve()
    {
        auto indexToUse {m_index.fetch_sub(1, std::memory_order_acquire)};
        if(indexToUse < 0)
        {
            return {};
        }
        return m_data[indexToUse];
    }
};

TEST_CASE("Using Acquire")
{
    TestContainer<ContainerAcquire>();
}

template<class Container>
void ExerciseContainer()
{
    std::vector<int> data(MAX_TO_INSERT);
    std::iota(data.begin(), data.end(), 0);
    std::unordered_set<int> allToRemove{data.begin(), data.end()};

    Container testObject(std::move(data));
    auto consume1 {std::async(std::launch::async, [&testObject](){return RemoveItems(testObject);})};
    auto consume2 {std::async(std::launch::async, [&testObject](){return RemoveItems(testObject);})};

    consume1.wait();
    consume2.wait();
}

TEST_CASE("benchmarks")
{
    BENCHMARK("mutex")
    {
        ExerciseContainer<ContainerMutex>();
    };

    BENCHMARK("SeqCst")
    {
        ExerciseContainer<ContainerSeqCst>();
    };

    BENCHMARK("Acquire")
    {
        ExerciseContainer<ContainerAcquire>();
    };
}
/* Typical results. Notice that memory_order_acquire does not win
benchmark name                                  samples       iterations    estimated
                                                mean          low mean      high mean
                                                std dev       low std dev   high std dev
-------------------------------------------------------------------------------
mutex                                                   100            5    85.166 ms
                                                 225.864 us    218.22 us   234.366 us
                                                  41.081 us    37.341 us    45.094 us

SeqCst                                                  100            5    83.448 ms
                                                 188.481 us   183.267 us   195.624 us
                                                   30.91 us    24.303 us    39.051 us

Acquire                                                 100            5    82.764 ms
                                                 190.682 us   184.139 us   198.706 us
                                                  36.909 us    31.205 us    42.673 us
*/
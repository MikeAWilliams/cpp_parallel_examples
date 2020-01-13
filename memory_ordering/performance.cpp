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

constexpr auto MAX_TO_INSERT {200'000};

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
mutex                                                   100            1    9.21604 s
                                                 91.4944 ms   91.0206 ms   91.9721 ms
                                                 2.43371 ms   2.22498 ms   2.68683 ms

SeqCst                                                  100            1    8.22779 s
                                                 82.5275 ms   82.3719 ms   82.6936 ms
                                                 818.591 us   721.944 us   978.407 us

Acquire                                                 100            1    8.29354 s
                                                 82.7256 ms    82.541 ms   82.9394 ms
                                                  1.0182 ms   860.456 us   1.24135 ms
*/
#include "catch2/catch.hpp"

#include <atomic>
#include <future>
#include <iostream>
#include <mutex>
#include <numeric>
#include <random>
#include <thread>
#include <unordered_set>
#include <vector>

constexpr auto MAX_SIZE {10'000};
constexpr auto MAX_TO_INSERT {200};

template<class stack>
void TestStack(stack& container)
{
    std::atomic<bool> done {false};
    auto populate {std::async(std::launch::async,
        [&container, &done]()
        {
            for(int i{0}; i < MAX_TO_INSERT; ++i)
            {
                container.Push(i);
            }
            done = true;
        })};

    auto consume {std::async(std::launch::async,
        [&container, &done]()
        {
            std::vector<int> toFillSet(MAX_TO_INSERT);
            std::iota(toFillSet.begin(), toFillSet.end(), 0);
            std::unordered_set<int> toFind{toFillSet.begin(), toFillSet.end()};
            while(true)
            {
                if(done)
                {
                    if(0 == container.Size())
                    {
                        break;
                    }
                }
                if(0 == container.Size())
                {
                    continue;
                }
                auto result {container.Pop()};
                auto findIter {toFind.find(result)};
                REQUIRE_FALSE(toFind.end() == findIter);
                toFind.erase(findIter);
            }
            REQUIRE(0 == toFind.size());
        })};
        populate.wait();
        consume.wait();
        REQUIRE(0 == container.Size());
}

class StackMutex
{
private:
    std::vector<int> m_data;
    int m_index;
    std::mutex m_mutex;

public:
    StackMutex()
        : m_data(MAX_SIZE)
        , m_index {-1}
    {

    }
    void Push(int item)
    {
        std::lock_guard lock{m_mutex};
        ++m_index;
        REQUIRE(m_index < MAX_SIZE);
        REQUIRE(m_index > -1);
        m_data[m_index] = item;
    }

    int Pop()
    {
        std::lock_guard lock{m_mutex};
        REQUIRE(m_index > -1);
        int result {m_data[m_index]};
        --m_index;
        return result;
    }

    int Size()
    {
        std::lock_guard lock{m_mutex};
        return m_index + 1;
    }
};

TEST_CASE("Using mutex")
{
    StackMutex container;
    TestStack(container);
}
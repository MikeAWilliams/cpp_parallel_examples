#include "catch2/catch.hpp"

#include <atomic>
#include <chrono>
#include <iostream>
#include <random>
#include <thread>

namespace relaxed
{
constexpr const int MAX_DELAY_MS {1};

std::atomic<bool> x, y;
std::atomic<int> z;

std::chrono::milliseconds GetRandomMilliseconds(int low, int high)
{
    std::random_device device;
    std::mt19937 generator(device());
    std::uniform_int_distribution<int> distribution(low, high);

    return std::chrono::milliseconds{distribution(generator)};
}

void write_x_then_y()
{
    x.store(true, std::memory_order_relaxed);
    y.store(true, std::memory_order_relaxed);
}

void read_y_then_x()
{
    while(!y.load(std::memory_order_relaxed));

    if(x.load(std::memory_order_relaxed))
    {
        ++z;
    }
}

TEST_CASE("relaxed")
{
    x = false;
    y = false;
    z = 0;

    std::thread b(read_y_then_x);
    std::thread a(write_x_then_y);

    a.join();
    b.join();

    // author claims this can be 0 but I am having a hard time seeing it
    std::cout << z << std::endl;
    REQUIRE(0 != z.load());
}
}
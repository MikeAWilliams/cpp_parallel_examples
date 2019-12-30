#define CATCH_CONFIG_MAIN 
#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include "catch2/catch.hpp"

#include <atomic>
#include <chrono>
#include <iostream>
#include <random>
#include <thread>

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

void write_x()
{
    std::this_thread::sleep_for(GetRandomMilliseconds(0, MAX_DELAY_MS));
    x.store(true, std::memory_order_seq_cst);
}

void write_y()
{
    std::this_thread::sleep_for(GetRandomMilliseconds(0, MAX_DELAY_MS));
    y.store(true, std::memory_order_seq_cst);
}

void read_x_then_y()
{
    while(!x.load(std::memory_order_seq_cst));

    if(y.load(std::memory_order_seq_cst))
    {
        ++z;
    }
}

void read_y_then_x()
{
    while(!y.load(std::memory_order_seq_cst));

    if(x.load(std::memory_order_seq_cst))
    {
        ++z;
    }
}
TEST_CASE("sequential")
{
    x = false;
    y = false;
    z = 0;

    std::thread a(write_y);
    std::thread b(write_x);
    std::thread c(read_y_then_x);
    std::thread d(read_x_then_y);

    a.join();
    b.join();
    c.join();
    d.join();

    std::cout << z << std::endl;
    REQUIRE(0 != z.load());
}
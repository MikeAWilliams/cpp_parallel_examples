// modified from examples in C++ Consurrency In Action Second Edition, Anthony Williams
#include "catch2/catch.hpp"

#include <atomic>
#include <chrono>
#include <iostream>
#include <random>
#include <thread>

namespace relaxed1
{

std::atomic<bool> x, y;
std::atomic<int> z;

std::atomic<bool> wait{true};

void write_x_then_y_relaxed()
{
    while(wait)
    {
        std::this_thread::yield();
    }
    x.store(true, std::memory_order_relaxed);
    y.store(true, std::memory_order_relaxed);
}

void read_y_then_x_relaxed()
{
    while(wait)
    {
        std::this_thread::yield();
    }
    while(!y.load(std::memory_order_relaxed));

    if(x.load(std::memory_order_relaxed))
    {
        ++z;
    }
}

TEST_CASE("relaxed_simple")
{
    wait = true;
    x = false;
    y = false;
    z = 0;

    std::thread b(read_y_then_x_relaxed);
    std::thread a(write_x_then_y_relaxed);

    wait = false;

    a.join();
    b.join();

    // author claims this can be 0 but I am having a hard time seeing it
    std::cout << z << std::endl;
    REQUIRE(0 != z.load());
}
}

namespace relaxded2
{
std::atomic<int> x(0),y(0),z(0);
std::atomic<bool> go(false);
unsigned const loop_count=10;

struct read_values
{
    int x,y,z;
};
read_values values1[loop_count];
read_values values2[loop_count];
read_values values3[loop_count];
read_values values4[loop_count];
read_values values5[loop_count];
void increment(std::atomic<int>* var_to_inc,read_values* values)
{
    while(!go)
        std::this_thread::yield();
    for(unsigned i=0;i<loop_count;++i)
    {
        values[i].x=x.load(std::memory_order_relaxed);
        values[i].y=y.load(std::memory_order_relaxed);
        values[i].z=z.load(std::memory_order_relaxed);
        var_to_inc->store(i+1,std::memory_order_relaxed);
        std::this_thread::yield();
    }
}

void read_vals(read_values* values)
{
    while(!go)
        std::this_thread::yield();
    for(unsigned i=0;i<loop_count;++i)
    {
        values[i].x=x.load(std::memory_order_relaxed);
        values[i].y=y.load(std::memory_order_relaxed);
        values[i].z=z.load(std::memory_order_relaxed);
        std::this_thread::yield();
    }
}

void print(read_values* v)
{
    for(unsigned i=0;i<loop_count;++i)
    {
        if(i)
            std::cout<<",";
        std::cout<<"("<<v[i].x<<","<<v[i].y<<","<<v[i].z<<")";
    }
    std::cout<<std::endl;
}
TEST_CASE("relaxed_5_thread")
{
    std::thread t1(increment,&x,values1);
    std::thread t2(increment,&y,values2);
    std::thread t3(increment,&z,values3);
    std::thread t4(read_vals,values4);
    std::thread t5(read_vals,values5);
    go=true;
    t5.join();
    t4.join();
    t3.join();
    t2.join();
    t1.join();
    print(values1);
    print(values2);
    print(values3);
    print(values4);
    print(values5);
}
}
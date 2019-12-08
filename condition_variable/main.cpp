#define CATCH_CONFIG_MAIN 
#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include "catch2/catch.hpp"
#include <tl/function_ref.hpp>

#include <condition_variable>
#include <future>
#include <iostream>
#include <mutex>
#include <optional>
#include <thread>
#include <queue>

using namespace std::chrono_literals;

std::mutex mutex;
std::queue<double> dataQueue;
std::condition_variable notifier;

int timesCalled {0};
bool moreDataToPrepare()
{
    ++timesCalled;
    return timesCalled <= 10;
}

double lastData{49};
double getData()
{
    ++lastData;
    return lastData;
}

bool stopAll {false};
bool isLastData(const double toTest)
{
    if(stopAll)
    {
        return true;
    }
    stopAll = toTest > 58;
    return stopAll;
}

void resetGlobals()
{
    timesCalled = 0;
    lastData = 49;
    stopAll = false;
}

void dataPreparationThread(tl::function_ref<void()> notifyFunction)
{
    std::cout << "Starting data prep" << std::endl;
    while(moreDataToPrepare())
    {
        const auto newData{getData()};
        {
            std::lock_guard<std::mutex> lock{mutex};
            std::cout << "New Data " << newData << std::endl;
            dataQueue.push(newData);
        }
        std::cout << "calling notify_one" << std::endl;
        notifyFunction();
        // sleep to let the other thread work. I am seeing all the data generated and then all processed with no interleaving
        // this sleep allow the other thread to get the data before this one just does all its work and finishes
        std::this_thread::sleep_for(1ms);
    }
}

void dataProcessingThread(tl::function_ref<void(const double)> processData)
{
    std::cout << "Starting data processing" << std::endl;
    while(true)
    {
        std::unique_lock<std::mutex> lock{mutex};
        notifier.wait_for(lock, 1s,
        //notifier.wait(lock,
            []()
            {
               auto result {!dataQueue.empty()};
               std::cout << "wait predicate " << result << std::endl;
               return result;
            });
        if(dataQueue.empty())
        {
            break;
        }

        const auto data {dataQueue.front()};
        dataQueue.pop();
        lock.unlock();
        processData(data);
        if(isLastData(data))
        {
            break;
        }
    }
}

TEST_CASE("Notify_one")
{
    resetGlobals();
    std::optional<double> lastData;
    int count {0};
    auto processFut {std::async(std::launch::async, dataProcessingThread, 
        [&lastData, &count](const double data)
        {
            std::cout << "Process data " << data << std::endl;
            ++count;
            if(lastData.has_value())
            {
                REQUIRE(data == (*lastData + 1));
            }
            lastData = data;
        })};

    auto prepFut {std::async(std::launch::async, dataPreparationThread,
        [&notifier](){
            notifier.notify_one();
        })};

    processFut.wait();
    REQUIRE(10 == count);
}

TEST_CASE("Notify_all")
{
    resetGlobals();
    int count1 {0};
    auto processFut1 {std::async(std::launch::async, dataProcessingThread, 
        [&count1](const double data)
        {
            std::cout << "Process data 1 " << data << std::endl;
            ++count1;
        })};

    int count2 {0};
    auto processFut2 {std::async(std::launch::async, dataProcessingThread, 
        [&count2](const double data)
        {
            std::cout << "Process data 2 " << data << std::endl;
            ++count2;
        })};

    auto prepFut {std::async(std::launch::async, dataPreparationThread,
        [&notifier](){
            notifier.notify_all();
        })};

    processFut1.wait();
    processFut2.wait();

    std::cout << count1 << " " << count2 << std::endl;

    REQUIRE(10 == (count1 + count2));
}
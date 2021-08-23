#include <eventpp/event_loop.hpp>
#include <eventpp/event_loop_thread_pool.hpp>
#include <eventpp/event_watcher.hpp>

#include <atomic>
#include <memory>
#include <mutex>
#include <set>
#include <thread>

#include "doctest.h"

static std::set<std::thread::id> g_working_tids;

static void OnWorkingThread()
{
    static std::mutex mutex;
    std::lock_guard<std::mutex> g(mutex);
    g_working_tids.insert(std::this_thread::get_id());
}


static std::atomic<int> g_count;
static void OnCount()
{
    g_count++;
    OnWorkingThread();
}

TEST_CASE("testEventLoopThreadPool")
{
    std::unique_ptr<eventpp::EventLoopThread> loop(new eventpp::EventLoopThread);
    loop->Start(true);

    int thread_num = 24;
    std::unique_ptr<eventpp::EventLoopThreadPool> pool(new eventpp::EventLoopThreadPool(loop->loop(), thread_num));
    REQUIRE(pool->Start(true));

    for (int i = 0; i < thread_num; i++)
    {
        pool->GetNextLoop()->RunInLoop(&OnCount);
    }

    usleep(1000 * 1000);
    pool->Stop(true);
    loop->Stop(true);
    usleep(1000 * 1000);
    REQUIRE((int)g_working_tids.size() == thread_num);
    pool.reset();
    loop.reset();
}


TEST_CASE("testEventLoopThreadPool2")
{
    std::unique_ptr<eventpp::EventLoopThread> loop(new eventpp::EventLoopThread);
    loop->Start(true);
    assert(loop->IsRunning());

    int thread_num = 24;
    for (int i = 0; i < thread_num; i++)
    {
        std::unique_ptr<eventpp::EventLoopThreadPool> pool(new eventpp::EventLoopThreadPool(loop->loop(), i));
        auto rc = pool->Start(true);
        REQUIRE(rc);
        REQUIRE(pool->IsRunning());
        pool->Stop(true);
        REQUIRE(pool->IsStopped());
        pool.reset();
    }

    assert(loop->IsRunning());
    loop->Stop(true);
    assert(loop->IsStopped());
    loop.reset();
}

#include <eventpp/event_loop.hpp>
#include <eventpp/event_watcher.hpp>
#include <eventpp/timestamp.hpp>

#include <stdio.h>

#include <memory>
#include <thread>

#include "test.h"

namespace evloop
{
static std::shared_ptr<eventpp::EventLoop> loop;
static eventpp::Duration delay(1.0);
static bool event_handler_called = false;
static void Handle(eventpp::InvokeTimerPtr t)
{
    event_handler_called = true;
    t->Cancel();
    loop->Stop();
}

static void MyEventThread()
{
    printf("EventLoop is running ...\n");
    loop->Run();

    // Make sure the loop object is delete in its own thread.
    loop.reset();
}

static int periodic_run_count = 0;
static void PeriodicFunc()
{
    periodic_run_count++;
    printf("PeriodicFunc is called , periodic_run_count=%d\n", periodic_run_count);
}

}

TEST_CASE("TestEventLoop1")
{
    using namespace evloop;
    loop = std::shared_ptr<eventpp::EventLoop>(new eventpp::EventLoop);
    std::thread th(MyEventThread);
    while (!loop->IsRunning())
    {
        usleep(static_cast<useconds_t>(delay.Microseconds()));
    }
    eventpp::Timestamp start = eventpp::Timestamp::Now();
    eventpp::InvokeTimerPtr t = loop->RunEvery(eventpp::Duration(0.3), &PeriodicFunc);
    loop->RunAfter(delay, std::bind(&Handle, t));
    th.join();
    t.reset();
    eventpp::Duration cost = eventpp::Timestamp::Now() - start;
    REQUIRE(delay <= cost);
    REQUIRE(event_handler_called);
    REQUIRE(periodic_run_count == 3);
    REQUIRE(eventpp::GetActiveEventCount() == 0);
}

void OnTimer(eventpp::EventLoop * loop)
{
}


TEST_CASE("TestEventLoop2")
{
    eventpp::EventLoop loop;
    auto timer = [&loop]() {
        auto close = [&loop]() { loop.Stop(); };
        loop.QueueInLoop(close);
    };
    loop.RunAfter(eventpp::Duration(0.5), timer);
    loop.Run();
    REQUIRE(eventpp::GetActiveEventCount() == 0);
}

// Test std::move of C++11
TEST_CASE("TestEventLoop3")
{
    eventpp::EventLoop loop;
    auto timer = [&loop]() {
        auto close = [&loop]() { loop.Stop(); };
        loop.QueueInLoop(close);
    };
    loop.RunAfter(eventpp::Duration(0.5), std::move(timer));
    loop.Run();
    REQUIRE(eventpp::GetActiveEventCount() == 0);
}


eventpp::EventLoop * loop = nullptr;
eventpp::InvokeTimerPtr invoke_timer;
int count = 0;
int active;

void Run()
{
    active--;
    printf("Running count=%d ...\n", count);
    if (count++ == 5)
    {
        invoke_timer->Cancel();
        loop->Stop();
    }
}

void NewEventLoop(struct event_base* base) {
    loop = new eventpp::EventLoop(base);
    invoke_timer = loop->RunEvery(eventpp::Duration(0.1), &Run);
}

// Test creating EventLoop from a exist event_base
TEST_CASE("TestEventLoop4")
{
    struct event_base* base = event_base_new();
    auto timer = std::make_shared<eventpp::TimerEventWatcher>(base, std::bind(&NewEventLoop, base), eventpp::Duration(1.0));
    active = 6;
    timer->Init();
    timer->AsyncWait();
    event_base_dispatch(base);
    event_base_free(base);
    delete loop;
    invoke_timer.reset();
    timer.reset();
    REQUIRE(active == 0);
    REQUIRE(eventpp::GetActiveEventCount() == 0);
}


// Test EventLoop::QueueInLoop() before EventLoop::Run()
TEST_CASE("TestEventLoop5")
{
    eventpp::EventLoop loop;
    auto fn = [&loop]() {
        printf("Entering fn\n");
        auto close = [&loop]() {
            printf("Entering close\n");
            loop.Stop();
        };
        loop.RunAfter(eventpp::Duration(1.0), close);
    };

    loop.QueueInLoop(fn);
    loop.Run();
}


// Test EventLoop's constructor and destructor
TEST_CASE("TestEventLoop6")
{
    eventpp::EventLoop * loop = new eventpp::EventLoop;
    delete loop;
}

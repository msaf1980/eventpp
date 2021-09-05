#include <eventpp/event_loop.hpp>
#include <eventpp/event_watcher.hpp>
#include <eventpp/invoke_timer.hpp>
#include <eventpp/timestamp.hpp>

#include <memory>
#include <thread>

#include "test.h"

namespace evloop
{
static std::shared_ptr<eventpp::EventLoop> loop;
static eventpp::Duration delay(1.0);
static eventpp::Duration cancel_delay(0.5);
static bool event_handler_called = false;
static void Close()
{
    loop->Stop();
}

static void Handle()
{
    event_handler_called = true;
}

static void MyEventThread()
{
    printf("EventLoop is running ...\n");
    loop->Run();
}
}

TEST_CASE("testInvokerTimerCancel")
{
    using namespace evloop;
    loop = std::shared_ptr<eventpp::EventLoop>(new eventpp::EventLoop);    
    std::thread th(MyEventThread);
    while (!loop->IsRunning())
    {
        usleep(delay.Microseconds());
    }
    eventpp::Timestamp start = eventpp::Timestamp::Now();
    loop->RunAfter(delay, &Close);
    eventpp::InvokeTimerPtr timer = loop->RunAfter(delay, &Handle);
    usleep(cancel_delay.Microseconds());
    timer->Cancel();
    th.join();
    timer.reset();
    loop.reset();
    eventpp::Duration cost = eventpp::Timestamp::Now() - start;
    REQUIRE(delay <= cost);
    REQUIRE(!event_handler_called);
    REQUIRE(eventpp::GetActiveEventCount() == 0);
}

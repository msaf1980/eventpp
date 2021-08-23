#include <signal.h>

#include <memory>
#include <thread>

//#include <eventpp/base.hpp>
#include <eventpp/event_loop.hpp>
#include <eventpp/event_loop_thread.hpp>
#include <eventpp/event_watcher.hpp>
#include <eventpp/timestamp.hpp>

#include "doctest.h"

//
// namespace evtimer {
// static eventpp::Duration g_timeout(1.0); // 1s
// static bool g_event_handler_called = false;
// static void Handle(struct event_base* base) {
//     g_event_handler_called = true;
//     event_base_loopexit(base, 0);
// }
//
// static void MyEventThread(struct event_base* base, eventpp::TimerEventWatcher* ev) {
//     ev->Init();
//     ev->AsyncWait();
//     event_base_loop(base, 0);
//     delete ev; // make sure to initialize and delete in the same thread.
// }
// }
//
// TEST_UNIT(testTimerEventWatcher) {
//     using namespace evtimer;
//     struct event_base* base = event_base_new();
//     eventpp::Timestamp start = eventpp::Timestamp::Now();
//     eventpp::TimerEventWatcher* ev(new eventpp::TimerEventWatcher(base, std::bind(&Handle, base), g_timeout));
//     std::thread th(MyEventThread, base, ev);
//     th.join();
//     eventpp::Duration cost = eventpp::Timestamp::Now() - start;
//     REQUIRE(g_timeout <= cost);
//     REQUIRE(g_event_handler_called);
//     event_base_free(base);
//     REQUIRE(eventpp::GetActiveEventCount() == 0);
// }


namespace evtimer
{
static eventpp::Duration g_timeout(1.0); // 1s
static bool g_event_handler_called = false;
static int active = 0;
static void Handle(eventpp::EventLoop * loop)
{
    active--;
    g_event_handler_called = true;
    loop->Stop();
}

static void MyEventThread(eventpp::EventLoop * loop, eventpp::TimerEventWatcher * ev)
{
    ev->Init();
    ev->AsyncWait();
    loop->Run();
    delete ev; // make sure to initialize and delete in the same thread.
}
}

TEST_CASE("testTimerEventWatcher")
{
    using namespace evtimer;
    active = 1;
    std::unique_ptr<eventpp::EventLoop> loop(new eventpp::EventLoop);
    eventpp::Timestamp start = eventpp::Timestamp::Now();
    eventpp::TimerEventWatcher * ev(new eventpp::TimerEventWatcher(loop.get(), std::bind(&Handle, loop.get()), g_timeout));
    std::thread th(MyEventThread, loop.get(), ev);
    th.join();
    eventpp::Duration cost = eventpp::Timestamp::Now() - start;
    REQUIRE(g_timeout <= cost);
    REQUIRE(g_event_handler_called);
    loop.reset();
    REQUIRE(active == 0);
}

TEST_CASE("testsocketpair")
{
    evpp_socket_t sockpair[2];
    memset(sockpair, 0, sizeof(sockpair[0] * 2));
    int r = evutil_socketpair(AF_UNIX, SOCK_STREAM, 0, sockpair);
    REQUIRE(r >= 0);
    REQUIRE(sockpair[0] > 0);
    REQUIRE(sockpair[1] > 0);
    REQUIRE(sockpair[0]);
    REQUIRE(sockpair[1]);
}

namespace evsignal
{
static eventpp::SignalEventWatcher * ev = nullptr;
static bool g_event_handler_called = false;
static int active = 0;

static void Handle(eventpp::EventLoopThread * thread)
{
    // LOG_INFO << "SIGINT caught.";
    active--;
    g_event_handler_called = true;
    thread->Stop();
    delete ev; // make sure to initialize and delete in the same thread.
    ev = nullptr;
}

static void WatchSignalInt()
{
    ev->Init();
    ev->AsyncWait();
}
}

TEST_CASE("testSignalEventWatcher")
{
    using namespace evsignal;
    active = 1;
    std::shared_ptr<eventpp::EventLoopThread> thread(new eventpp::EventLoopThread);
    thread->Start(true);
    eventpp::EventLoop * loop = thread->loop();
    ev = new eventpp::SignalEventWatcher(SIGINT, loop, std::bind(&Handle, thread.get()));
    loop->RunInLoop(&WatchSignalInt);
    auto f = []() {
        printf("Send SIGINT ...\n");
#ifdef H_OS_WINDOWS
        raise(SIGINT);
#else
        kill(getpid(), SIGINT);
#endif
    };
    loop->RunAfter(eventpp::Duration(0.1), f);
    while (!thread->IsStopped())
    {
        usleep(1);
    }
    thread.reset();
    REQUIRE(g_event_handler_called);
    REQUIRE(active == 0);
}

#include <eventpp/event_watcher.hpp>
#include <eventpp/event_loop.hpp>
#include <eventpp/event_loop_thread.hpp>
#include <eventpp/timestamp.hpp>

#include <atomic>
#include <memory>

#include "doctest.h"

static bool g_timeout = false;
static std::atomic<int> g_count;
static void OnTimeout() {
    g_timeout = true;
}

static void OnCount() {
    g_count++;
}


TEST_CASE("testEventLoopThread") {
    eventpp::Duration delay(double(1.0)); // 1s
    g_count.store(0);
    std::unique_ptr<eventpp::EventLoopThread> t(new eventpp::EventLoopThread);
    t->Start(true);
    usleep(1000);
    eventpp::Timestamp begin = eventpp::Timestamp::Now();
    t->loop()->RunAfter(delay, &OnTimeout);

    while (!g_timeout) {
        usleep(1);
    }

    eventpp::Duration cost = eventpp::Timestamp::Now() - begin;
    REQUIRE(delay <= cost);
    t->loop()->RunInLoop(&OnCount);
    t->loop()->RunInLoop(&OnCount);
    t->loop()->RunInLoop(&OnCount);
    t->loop()->RunInLoop(&OnCount);
    t->Stop(true);
    t.reset();
    REQUIRE(g_count == 4);
}

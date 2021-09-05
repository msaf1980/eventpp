
#include <thread>

#include <eventpp/event_loop.hpp>
#include <eventpp/event_watcher.hpp>

#include "test.h"

// namespace {
// static bool g_event_handler_called = false;
// static void Handle(struct event_base* base) {
//     g_event_handler_called = true;
//     event_base_loopexit(base, 0);
// }
//
// static void MyEventThread(struct event_base* base, eventpp::PipeEventWatcher* ev) {
//     if (ev->Init()) {
//         ev->AsyncWait();
//     }
//
//     event_base_loop(base, 0);
//     delete ev;// make sure to initialize and delete in the same thread.
// }
// }
//
// TEST_UNIT(testPipeEventWatcher) {
//     struct event_base* base = event_base_new();
//     eventpp::PipeEventWatcher* ev = new eventpp::PipeEventWatcher(base, std::bind(&Handle, base));
//     std::thread th(MyEventThread, base, ev);
//     ::usleep(1000 * 100);
//     ev->Notify();
//     th.join();
//     event_base_free(base);
//     REQUIRE(g_event_handler_called == true);
//     REQUIRE(eventpp::GetActiveEventCount() == 0);
// }

static bool g_event_handler_called = false;
static void Handle(eventpp::EventLoop * loop)
{
    g_event_handler_called = true;
    loop->Stop();
}

static void MyEventThread(eventpp::EventLoop * loop, eventpp::PipeEventWatcher * ev)
{
    if (ev->Init())
    {
        ev->AsyncWait();
    }

    loop->Run();
    delete ev; // make sure to initialize and delete in the same thread.
}

TEST_CASE("testPipeEventWatcher")
{
    std::unique_ptr<eventpp::EventLoop> loop(new eventpp::EventLoop);
    eventpp::PipeEventWatcher * ev = new eventpp::PipeEventWatcher(loop.get(), std::bind(&Handle, loop.get()));
    std::thread th(MyEventThread, loop.get(), ev);
    
    while (!loop->IsRunning())
    {
        ::usleep(100 * 100);
    }
    ev->Notify();
    th.join();
    loop.reset();
    REQUIRE(g_event_handler_called == true);
    REQUIRE(eventpp::GetActiveEventCount() == 0);
}

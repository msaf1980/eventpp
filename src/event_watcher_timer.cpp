#include "pch.h"

//#include "evpp/inner_pre.h"

#include <string.h>

#include <event2/event_struct.h>

#include <eventpp/base.hpp>
#include <eventpp/event_loop.hpp>
#include <eventpp/event_watcher_timer.hpp>

namespace eventpp
{
TimerEventWatcher::TimerEventWatcher(EventLoop * loop, const Handler & handler, Duration timeout)
    : EventWatcherBase(loop, handler), timeout_(timeout)
{
}

TimerEventWatcher::TimerEventWatcher(EventLoop * loop, Handler && h, Duration timeout)
    : EventWatcherBase(loop, std::move(h)), timeout_(timeout)
{
}

// TimerEventWatcher::TimerEventWatcher(struct event_base * loop, const Handler & handler, Duration timeout)
//     : EventWatcherBase(loop, handler), timeout_(timeout)
// {
// }

// TimerEventWatcher::TimerEventWatcher(struct event_base * loop, Handler && h, Duration timeout)
//     : EventWatcherBase(loop, std::move(h)), timeout_(timeout)
// {
// }

bool TimerEventWatcher::Init()
{
    int err = 0;
    int rc = evtimer_assign(event_, loop_->event_base(), &TimerEventWatcher::HandlerFn, this);
    if (rc == -1)
    {
        err = errno;
        Close();
        errno = err;
    }

    return rc == 0;
}

TimerEventWatcher::~TimerEventWatcher()
{
    Close();
    FreeEvent();
}

void TimerEventWatcher::Close() { }

void TimerEventWatcher::HandlerFn(eventpp_socket_t /*fd*/, short /*which*/, void * v)
{
    TimerEventWatcher * h = (TimerEventWatcher *)v;
    h->handler_();
}

bool TimerEventWatcher::AsyncWait()
{
    return Watch(timeout_);
}
}

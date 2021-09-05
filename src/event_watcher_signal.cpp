#include "pch.h"

//#include "evpp/inner_pre.h"

#include <string.h>

#include <event2/event_struct.h>

#include <eventpp/base.hpp>
#include <eventpp/event_loop.hpp>
#include <eventpp/event_watcher_signal.hpp>

namespace eventpp
{
SignalEventWatcher::SignalEventWatcher(signal_number_t signo, EventLoop * loop, const Handler & handler)
    : EventWatcherBase(loop, handler), signo_(signo)
{
    assert(signo_);
}

SignalEventWatcher::SignalEventWatcher(signal_number_t signo, EventLoop * loop, Handler && h)
    : EventWatcherBase(loop, std::move(h)), signo_(signo)
{
    assert(signo_);
}

bool SignalEventWatcher::Init()
{
    assert(signo_);
    int err = 0;
    int rc =  evsignal_assign(event_, loop_->event_base(), signo_, SignalEventWatcher::HandlerFn, this);
    if (rc == -1)
    {
        err = errno;
        Close();
        errno = err;
    }

    return rc == 0;
}

SignalEventWatcher::~SignalEventWatcher()
{
    Close();
    FreeEvent();
}

void SignalEventWatcher::Close() { }

void SignalEventWatcher::HandlerFn(signal_number_t /*sn*/, short /*which*/, void * v)
{
    SignalEventWatcher * h = (SignalEventWatcher *)v;
    h->handler_();
}

bool SignalEventWatcher::AsyncWait()
{
    return Watch(Duration());
}
}

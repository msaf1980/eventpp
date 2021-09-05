#include "pch.h"

//#include "evpp/inner_pre.h"

#include <string.h>

#include <event2/event_struct.h>

#include <eventpp/base.hpp>
#include <eventpp/event_loop.hpp>
#include <eventpp/event_watcher.hpp>

#include <eventpp/trace.hpp>

namespace eventpp
{
EventWatcher::EventWatcher(struct event_base * evbase, const Handler & handler) : evbase_(evbase), attached_(false), handler_(handler)
{
    event_ = new event;
    memset(event_, 0, sizeof(struct event));
}

EventWatcher::EventWatcher(struct event_base * evbase, Handler && handler) : evbase_(evbase), attached_(false), handler_(std::move(handler))
{
    event_ = new event;
    memset(event_, 0, sizeof(struct event));
}

EventWatcher::~EventWatcher()
{
    FreeEvent();
    Close();
}

bool EventWatcher::Init()
{
    if (!DoInit())
    {
        goto failed;
    }

    return true;

failed:
    Close();
    return false;
}


void EventWatcher::Close()
{
    DoClose();
}

bool EventWatcher::Watch(Duration timeout)
{
    struct timeval tv;
    struct timeval * timeoutval = nullptr;
    if (timeout.Nanoseconds() > 0)
    {
        timeout.To(&tv);
        timeoutval = &tv;
    }

    if (attached_)
    {
        // When InvokerTimer::periodic_ == true, EventWatcher::Watch will be called many times
        // so we need to remove it from event_base before we add it into event_base
        EventDel(event_);
        attached_ = false;
    }

    assert(!attached_);
    if (EventAdd(event_, timeoutval) != 0)
    {
        //LOG_ERROR << "event_add failed. fd=" << this->event_->ev_fd << " event_=" << event_;
        return false;
    }
    attached_ = true;
    return true;
}

void EventWatcher::FreeEvent()
{
    if (event_)
    {
        if (attached_)
        {
            EventDel(event_);
            attached_ = false;
        }

        delete event_;
        event_ = nullptr;
    }
}

void EventWatcher::Cancel()
{
    assert(event_);
    FreeEvent();

    if (cancel_callback_)
    {
        cancel_callback_();
    }
}

void EventWatcher::SetCancelCallback(const Handler & cb)
{
    cancel_callback_ = cb;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

PipeEventWatcher::PipeEventWatcher(EventLoop * loop, const Handler & handler) : EventWatcher(loop->event_base(), handler)
{
    pipe_[0] = -1;
    pipe_[1] = -1;
}

PipeEventWatcher::PipeEventWatcher(EventLoop * loop, Handler && h) : EventWatcher(loop->event_base(), std::move(h))
{
    pipe_[0] = -1;
    pipe_[1] = -1;
}


PipeEventWatcher::~PipeEventWatcher()
{
    Close();
}

bool PipeEventWatcher::DoInit()
{
    assert(pipe_[0] == -1);

    int err = 0;

    if (evutil_socketpair(AF_UNIX, SOCK_STREAM, 0, pipe_) < 0)
    {
        // LOG_ERROR << "create socketpair ERROR errno=" << err << " " << strerror(err);
        err = errno;
        goto FAILED;
    }

    if (evutil_make_socket_nonblocking(pipe_[0]) < 0 || evutil_make_socket_nonblocking(pipe_[1]) < 0)
    {
        err = errno;
        goto FAILED;
    }

    // DLOG_TRACE << "Init pipe whatch " << pipe_[1] << " " << pipe_[0] << std::endl;

    return event_assign(event_, evbase_, pipe_[1], EV_READ | EV_PERSIST, &PipeEventWatcher::HandlerFn, this) == 0;
FAILED:
    Close();
    errno = err;
    // DLOG_TRACE << "Init pipe whatch failed " << pipe_[1] << " " << pipe_[0] << std::endl;
    return false;
}

void PipeEventWatcher::DoClose()
{
    if (pipe_[0] > 0)
    {
        EVUTIL_CLOSESOCKET(pipe_[0]);
        EVUTIL_CLOSESOCKET(pipe_[1]);
        memset(pipe_, 0, sizeof(pipe_[0]) * 2);
    }
}

void PipeEventWatcher::HandlerFn(evpp_socket_t fd, short /*which*/, void * v)
{
    (void)fd;
    // LOG_INFO << "PipeEventWatcher::HandlerFn fd=" << fd << " v=" << v;
    PipeEventWatcher * e = (PipeEventWatcher *)v;
#ifdef H_BENCHMARK_TESTING
    // Every time we only read 1 byte for testing the IO event performance.
    // We use it in the benchmark test program
    //  1. evpp/benchmark/ioevent/evpp/
    //  1. evpp/benchmark/ioevent/fd_channel_vs_pipe_event_watcher/
    char buf[1];
#else
    char buf[128];
#endif

    if (recv(e->pipe_[1], buf, sizeof(buf), 0) > 0)
    {
        // DLOG_TRACE << "Pipe whatch notified " << e->pipe_[0] << " from " << e->pipe_[1] << std::endl;
        e->handler_();
    }
}

bool PipeEventWatcher::AsyncWait()
{
    return Watch(Duration());
}

void PipeEventWatcher::Notify()
{
    char c = '\0';

    if (send(pipe_[0], &c, sizeof(c), 0) < 0)
    {
        // DLOG_TRACE << "notify pipe failed: " << pipe_[0] << std:: endl;
        return;
    }

    // DLOG_TRACE << "notify pipe success: " << pipe_[0] << std:: endl;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
TimerEventWatcher::TimerEventWatcher(EventLoop * loop, const Handler & handler, Duration timeout)
    : EventWatcher(loop->event_base(), handler), timeout_(timeout)
{
}

TimerEventWatcher::TimerEventWatcher(EventLoop * loop, Handler && h, Duration timeout)
    : EventWatcher(loop->event_base(), std::move(h)), timeout_(timeout)
{
}

TimerEventWatcher::TimerEventWatcher(struct event_base * loop, const Handler & handler, Duration timeout)
    : EventWatcher(loop, handler), timeout_(timeout)
{
}

TimerEventWatcher::TimerEventWatcher(struct event_base * loop, Handler && h, Duration timeout)
    : EventWatcher(loop, std::move(h)), timeout_(timeout)
{
}

bool TimerEventWatcher::DoInit()
{
    return evtimer_assign(event_, evbase_, &TimerEventWatcher::HandlerFn, this) == 0;
}

void TimerEventWatcher::HandlerFn(evpp_socket_t /*fd*/, short /*which*/, void * v)
{
    TimerEventWatcher * h = (TimerEventWatcher *)v;
    h->handler_();
}

bool TimerEventWatcher::AsyncWait()
{
    return Watch(timeout_);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
SignalEventWatcher::SignalEventWatcher(signal_number_t signo, EventLoop * loop, const Handler & handler)
    : EventWatcher(loop->event_base(), handler), signo_(signo)
{
    assert(signo_);
}

SignalEventWatcher::SignalEventWatcher(signal_number_t signo, EventLoop * loop, Handler && h)
    : EventWatcher(loop->event_base(), std::move(h)), signo_(signo)
{
    assert(signo_);
}

bool SignalEventWatcher::DoInit()
{
    assert(signo_);
    return evsignal_assign(event_, evbase_, signo_, SignalEventWatcher::HandlerFn, this) == 0;
}

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

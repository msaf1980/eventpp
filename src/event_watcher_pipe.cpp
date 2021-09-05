#include "pch.h"

//#include "evpp/inner_pre.h"

#include <string.h>

#include <event2/event_struct.h>

#include <eventpp/base.hpp>
#include <eventpp/event_loop.hpp>
#include <eventpp/event_watcher_pipe.hpp>

namespace eventpp
{
PipeEventWatcher::PipeEventWatcher(EventLoop * loop, const Handler & handler) : EventWatcherBase(loop, handler)
{
    pipe_[0] = -1;
    pipe_[1] = -1;
}

PipeEventWatcher::PipeEventWatcher(EventLoop * loop, Handler && h) : EventWatcherBase(loop, std::move(h))
{
    pipe_[0] = -1;
    pipe_[1] = -1;
}


PipeEventWatcher::~PipeEventWatcher()
{
    Close();
    FreeEvent();
}

bool PipeEventWatcher::Init()
{
    assert(pipe_[0] == -1);

    int err = 0;
    int rc = 0;

    if ((rc = evutil_socketpair(AF_UNIX, SOCK_STREAM, 0, pipe_)) < 0)
    {
        err = errno;
        goto FAILED;
    }

    if (evutil_make_socket_nonblocking(pipe_[0]) < 0 || evutil_make_socket_nonblocking(pipe_[1]) < 0)
    {
        err = errno;
        goto FAILED;
    }

    rc = event_assign(event_, loop_->event_base(), pipe_[1], EV_READ | EV_PERSIST, &PipeEventWatcher::HandlerFn, this);
    if (rc == 0)
    {
        return true;
    }
    err = errno;

FAILED:
    Close();
    return false;
}

void PipeEventWatcher::Close()
{
    if (pipe_[0] > 0)
    {
        EVUTIL_CLOSESOCKET(pipe_[0]);
        EVUTIL_CLOSESOCKET(pipe_[1]);
        pipe_[0] = -1;
        pipe_[1] = -1;
    }
}

void PipeEventWatcher::HandlerFn(eventpp_socket_t fd, short /*which*/, void * v)
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

    if (::recv(e->pipe_[1], buf, sizeof(buf), 0) > 0)
    {
        e->handler_();
    }
}

bool PipeEventWatcher::AsyncWait()
{
    return Watch(Duration());
}

bool PipeEventWatcher::Notify()
{
    char c = '\0';

    return ::send(pipe_[0], &c, sizeof(c), 0) == 1;
}
}

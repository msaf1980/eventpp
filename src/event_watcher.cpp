#include "pch.h"

#ifdef HAS_LINUX_EVENTFD
#    include <sys/eventfd.h>
#endif

#include <string.h>

#include <event2/event_struct.h>

#include <eventpp/base.hpp>
#include <eventpp/event_loop.hpp>
#include <eventpp/event_watcher.hpp>
#include <eventpp/sys/platform_config.h>

namespace eventpp
{
EventWatcher::EventWatcher(EventLoop * loop, const Handler & handler) : EventWatcherBase(loop, handler)
{
    pipe_[0] = -1;
    pipe_[1] = -1;
}

EventWatcher::EventWatcher(EventLoop * loop, Handler && h) : EventWatcherBase(loop, std::move(h))
{
    pipe_[0] = -1;
    pipe_[1] = -1;
}


EventWatcher::~EventWatcher()
{
    Close();
    FreeEvent();
}

bool EventWatcher::Init()
{
    assert(pipe_[0] == -1);

    int err = 0;
    int rc = 0;
#ifdef HAS_LINUX_EVENTFD
    if ((pipe_[0] = eventfd(0, EFD_NONBLOCK)) == -1)
    {
        err = errno;
        goto FAILED;
    }
    rc = event_assign(event_, loop_->event_base(), pipe_[0], EV_READ | EV_PERSIST, &EventWatcher::HandlerFn, this);
#else
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
    rc = event_assign(event_, loop_->event_base(), pipe_[1], EV_READ | EV_PERSIST, &EventWatcher::HandlerFn, this);
#endif

    if (rc == 0)
    {
        return true;
    }
    err = errno;

FAILED:
    Close();
    return false;
}

void EventWatcher::Close()
{
    if (pipe_[0] > -1)
    {
        EVUTIL_CLOSESOCKET(pipe_[0]);
        pipe_[0] = -1;
#ifndef HAS_LINUX_EVENTFD
        EVUTIL_CLOSESOCKET(pipe_[1]);
        pipe_[1] = -1;
#endif
    }
}

void EventWatcher::HandlerFn(eventpp_socket_t fd, short /*which*/, void * v)
{
    (void)fd;
    // LOG_INFO << "EventWatcher::HandlerFn fd=" << fd << " v=" << v;
    EventWatcher * e = (EventWatcher *)v;
    ssize_t n;
#ifdef HAS_LINUX_EVENTFD
    eventfd_t u;
    if ((n = ::eventfd_read(e->pipe_[0], &u)) == 0)
    {
        n = 1;
    } 
#else
    char buf[128];
    n = ::recv(e->pipe_[1], buf, sizeof(buf), 0);
#endif
    if (n > 0)
    {
        e->handler_();
    }
}

bool EventWatcher::AsyncWait()
{
    return Watch(Duration());
}

bool EventWatcher::Notify()
{
#ifdef HAS_LINUX_EVENTFD
    eventfd_t u = 1;
    return ::eventfd_write(pipe_[0], u);
#else
    char c = '\0';
    return ::send(pipe_[0], &c, sizeof(c), 0) == 1;
#endif
}
}

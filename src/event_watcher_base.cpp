#include "pch.h"

#include <string.h>

#include <event2/event_struct.h>

#include <eventpp/base.hpp>
#include <eventpp/event_loop.hpp>
#include <eventpp/event_watcher_base.hpp>

namespace eventpp
{
EventWatcherBase::EventWatcherBase(EventLoop *loop, const Handler & handler) : loop_(loop), attached_(false), handler_(handler)
{
    event_ = new event;
    memset(event_, 0, sizeof(struct event));
}

EventWatcherBase::EventWatcherBase(EventLoop *loop, Handler && handler) : loop_(loop), attached_(false), handler_(std::move(handler))
{
    event_ = new event;
    memset(event_, 0, sizeof(struct event));
}

EventWatcherBase::~EventWatcherBase()
{
    FreeEvent();
}

bool EventWatcherBase::Watch(Duration timeout)
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
        // When InvokerTimer::periodic_ == true, EventWatcherBase::Watch will be called many times
        // so we need to remove it from event_base before we add it into event_base
        EventDel(event_);
        attached_ = false;
    }

    assert(!attached_);
    int rc = EventAdd(event_, timeoutval);
    if (rc != 0)
    {
        //LOG_ERROR << "event_add failed. fd=" << this->event_->ev_fd << " event_=" << event_;
        return false;
    }
    attached_ = true;
    return true;
}

void EventWatcherBase::FreeEvent()
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

void EventWatcherBase::Cancel()
{
    assert(event_);
    FreeEvent();

    if (cancel_callback_)
    {
        cancel_callback_();
    }
}

void EventWatcherBase::SetCancelCallback(const Handler & cb)
{
    cancel_callback_ = cb;
}
}

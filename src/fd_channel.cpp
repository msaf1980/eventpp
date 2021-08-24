#include "pch.h"

#include <string.h>

#include <event2/event_struct.h>

#include <eventpp/base.hpp>
#include <eventpp/event_loop.hpp>
#include <eventpp/fd_channel.hpp>

namespace eventpp
{
static_assert(FdChannel::kReadable == EV_READ, "");
static_assert(FdChannel::kWritable == EV_WRITE, "");

FdChannel::FdChannel(EventLoop * l, evpp_socket_t f, bool r, bool w) : loop_(l), attached_(false), event_(nullptr), fd_(f)
{
    // DLOG_TRACE << "fd=" << fd_;
    assert(fd_ > 0);
    events_ = (r ? kReadable : kNone) | (w ? kWritable : kNone);
    event_ = new event;
    memset(event_, 0, sizeof(struct event));
}

FdChannel::~FdChannel()
{
    // DLOG_TRACE << "fd=" << fd_;
    assert(event_ == nullptr);
}

void FdChannel::Close()
{
    // DLOG_TRACE << "fd=" << fd_;
    assert(event_);
    if (event_)
    {
        assert(!attached_);
        if (attached_)
        {
            event_del(event_);
            attached_ = false;
        }

        delete event_;
        event_ = nullptr;
    }
    read_fn_ = ReadEventCallback();
    write_fn_ = EventCallback();
}

bool FdChannel::AttachToLoop()
{
    assert(!IsNoneEvent());
    assert(loop_->IsInLoopThread());

    if (attached_)
    {
        // FdChannel::Update may be called many times
        // So doing this can avoid event_add will be called more than once.
        DetachFromLoop();
    }

    assert(!attached_);
    event_assign(event_, loop_->event_base(), fd_, events_ | EV_PERSIST, &FdChannel::HandleEvent, this);

    if (EventAdd(event_, nullptr) == 0)
    {
        // DLOG_TRACE << "fd=" << fd_ << " watching event " << EventsToString();
        attached_ = true;
    }
    // else
    // {
    //         LOG_ERROR << "this=" << this << " fd=" << fd_ << " with event " << EventsToString() << " attach to event loop failed";
    // }
      return attached_;
}

void FdChannel::EnableReadEvent()
{
    int events = events_;
    events_ |= kReadable;

    if (events_ != events)
    {
        Update();
    }
}

void FdChannel::EnableWriteEvent()
{
    int events = events_;
    events_ |= kWritable;

    if (events_ != events)
    {
        Update();
    }
}

void FdChannel::DisableReadEvent()
{
    int events = events_;
    events_ &= (~kReadable);

    if (events_ != events)
    {
        Update();
    }
}

void FdChannel::DisableWriteEvent()
{
    int events = events_;
    events_ &= (~kWritable);
    if (events_ != events)
    {
        Update();
    }
}

void FdChannel::DisableAllEvent()
{
    if (events_ == kNone)
    {
        return;
    }

    events_ = kNone;
    Update();
}

bool FdChannel::DetachFromLoop()
{
    assert(loop_->IsInLoopThread());
    assert(attached_);

    if (EventDel(event_) == 0)
    {
        attached_ = false;
        // DLOG_TRACE << "fd=" << fd_ << " detach from event loop";
    }
    // else
    // {
    // LOG_ERROR << "DetachFromLoop this=" << this << "fd=" << fd_ << " with event " << EventsToString()
    //           << " detach from event loop failed";
    // }

    return !attached_;
}

void FdChannel::Update()
{
    assert(loop_->IsInLoopThread());

    if (IsNoneEvent())
    {
        DetachFromLoop();
    }
    else
    {
        AttachToLoop();
    }
}

std::string FdChannel::EventsToString() const
{
    std::string s;

    if (events_ & kReadable)
    {
        s = "kReadable";
    }

    if (events_ & kWritable)
    {
        if (!s.empty())
        {
            s += "|";
        }

        s += "kWritable";
    }

    return s;
}

void FdChannel::HandleEvent(evpp_socket_t sockfd, short which, void * v)
{
    FdChannel * c = (FdChannel *)v;
    c->HandleEvent(sockfd, which);
}

void FdChannel::HandleEvent(evpp_socket_t sockfd, short which)
{
    assert(sockfd == fd_);
    // DLOG_TRACE << "fd=" << sockfd << " " << EventsToString();

    if ((which & kReadable) && read_fn_)
    {
        read_fn_();
    }

    if ((which & kWritable) && write_fn_)
    {
        write_fn_();
    }
}
}

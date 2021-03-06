#ifndef __EVENTPP_FD_CHANNEL_HPP__
#define __EVENTPP_FD_CHANNEL_HPP__


#include "event_watcher.hpp"

struct event;
struct event_base;

namespace eventpp {

class EventLoop;

// A selectable I/O fd channel.
//
// This class doesn't own the file descriptor.
// The file descriptor could be a socket,
// an eventfd, a timerfd, or a signalfd
class EVPP_EXPORT FdChannel {
public:
    enum EventType : short int {
        kNone = 0x00,
        kReadable = EV_READ,
        kWritable = EV_WRITE,
    };
    typedef std::function<void()> EventCallback;
    typedef std::function<void()> ReadEventCallback;

public:
    FdChannel(EventLoop* loop, evpp_socket_t fd,
              bool watch_read_event, bool watch_write_event);
    ~FdChannel();

    void Close();

    // Attach this FdChannel to EventLoop
    bool AttachToLoop();

    // Detach this FdChannel from EventLoop
    bool DetachFromLoop();

    bool attached() const {
        return attached_;
    }

    bool IsReadable() const {
        return (events_ & kReadable) != 0;
    }
    bool IsWritable() const {
        return (events_ & kWritable) != 0;
    }
    bool IsNoneEvent() const {
        return events_ == kNone;
    }

    void EnableReadEvent();
    void EnableWriteEvent();
    void DisableReadEvent();
    void DisableWriteEvent();
    void DisableAllEvent();

    evpp_socket_t fd() const {
        return fd_;
    }
    std::string EventsToString() const;

    void SetReadCallback(const ReadEventCallback& cb) {
        read_fn_ = cb;
    }

    void SetWriteCallback(const EventCallback& cb) {
        write_fn_ = cb;
    }

private:
    void HandleEvent(evpp_socket_t fd, short which);
    static void HandleEvent(evpp_socket_t fd, short which, void* v);

    void Update();

    ReadEventCallback read_fn_;
    EventCallback write_fn_;

    EventLoop* loop_;
    bool attached_; // A flag indicate whether this FdChannel has been attached to loop_

    struct event* event_;
    short int events_; // the bitwise OR of zero or more of the EventType flags

    evpp_socket_t fd_;
};

}



#endif // __EVENTPP_FD_CHANNEL_HPP__
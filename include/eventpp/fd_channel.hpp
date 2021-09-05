#ifndef FD_CHANNEL
#define FD_CHANNEL


#include "event_watcher.hpp"

struct event;
struct event_base;

namespace eventpp
{
class EventLoop;

// A base class for selectable I/O fd channel.
//
// This class doesn't own the file descriptor.
// The file descriptor could be a socket,
// an eventfd, a timerfd, or a signalfd
class EVENTPP_EXPORT FdChannel
{
public:
    enum EventType : short int
    {
        kNone = 0x00,
        kReadable = EV_READ,
        kWritable = EV_WRITE,
    };
    typedef std::function<void()> WriteEventCallback;
    typedef std::function<void()> ReadEventCallback;

protected:
    FdChannel(EventLoop * loop, eventpp_socket_t fd = INVALID_SOCKET, bool watch_read_event = false, bool watch_write_event = false);
public:
    ~FdChannel();

    void Close();

    // Attach this FdChannel to EventLoop
    bool AttachToLoop();

    // Detach this FdChannel from EventLoop
    bool DetachFromLoop();

    bool attached() const { return attached_; }

    bool IsReadable() const { return (events_ & kReadable) != 0; }
    bool IsWritable() const { return (events_ & kWritable) != 0; }
    bool IsNoneEvent() const { return events_ == kNone; }

    void EnableReadEvent();
    void EnableWriteEvent();
    void DisableReadEvent();
    void DisableWriteEvent();
    void DisableAllEvent();

    eventpp_socket_t fd() const { return fd_; }
    std::string EventsToString() const;

    void SetReadCallback(const ReadEventCallback & cb) { read_fn_ = cb; }

    void SetWriteCallback(const WriteEventCallback & cb) { write_fn_ = cb; }

    EventLoop * loop() const { return loop_; }
protected:
    EventLoop * loop_;
private:
    void HandleEvent(eventpp_socket_t fd, short which);
    static void HandleEvent(eventpp_socket_t fd, short which, void * v);

    void Update();

    ReadEventCallback read_fn_;
    WriteEventCallback write_fn_;

    bool attached_; // A flag indicate whether this FdChannel has been attached to loop_

    struct event * event_;
    short int events_; // the bitwise OR of zero or more of the EventType flags

protected:
    eventpp_socket_t fd_;
};

}


#endif /* FD_CHANNEL */

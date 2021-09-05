#ifndef __EVENTPP_EVENT_WATCHER_BASE_HPP__
#define __EVENTPP_EVENT_WATCHER_BASE_HPP__

#include "base.hpp"
#include "duration.hpp"
#include "sys/sys_sockets.h"

#include <functional>

namespace eventpp
{
class EventLoop;
class EVENTPP_EXPORT EventWatcherBase
{
public:
    typedef std::function<void()> Handler;

    ~EventWatcherBase();

    // @note It MUST be called in the event thread.
    void Cancel();

    // @brief :
    // @param[IN] const Handler& cb - The callback which will be called when this event is canceled.
    // @return void -
    void SetCancelCallback(const Handler & cb);

    void ClearHandler() { handler_ = Handler(); }

protected:
    // @note It MUST be called in the event thread.
    // @param timeout the maximum amount of time to wait for the event, or 0 to wait forever
    bool Watch(Duration timeout);

protected:
    EventWatcherBase(EventLoop *loop, const Handler & handler);
    EventWatcherBase(EventLoop *loop, Handler && handler);

    void FreeEvent();

protected:
    struct event * event_;
    EventLoop * loop_;
    bool attached_;
    Handler handler_;
    Handler cancel_callback_;
};
}


#endif // __EVENT_WATCHER_BASE_HPP__
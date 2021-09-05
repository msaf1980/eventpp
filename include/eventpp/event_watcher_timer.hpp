#ifndef __EVENTPP_EVENT_WATCHER_TIMER_HPP__
#define __EVENTPP_EVENT_WATCHER_TIMER_HPP__

#include "base.hpp"
#include "duration.hpp"
#include "event_watcher_base.hpp"
#include "sys/sys_sockets.h"

#include <functional>

namespace eventpp
{
class EventLoop;

class EVENTPP_EXPORT TimerEventWatcher : public EventWatcherBase
{
public:
    TimerEventWatcher(EventLoop * loop, const Handler & handler, Duration timeout);
    TimerEventWatcher(EventLoop * loop, Handler && handler, Duration timeout);
    // TimerEventWatcher(struct event_base * loop, const Handler & handler, Duration timeout);
    // TimerEventWatcher(struct event_base * loop, Handler && handler, Duration timeout);

    ~TimerEventWatcher();

    bool AsyncWait();
    bool Init();
    void Close();

private:
    static void HandlerFn(eventpp_socket_t fd, short which, void * v);

private:
    Duration timeout_;
};
}


#endif // __EVENT_WATCHER_TIMER_HPP__

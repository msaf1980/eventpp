#ifndef __EVENTPP_EVENT_WATCHER_SIGNAL_HPP__
#define __EVENTPP_EVENT_WATCHER_SIGNAL_HPP__

#include "base.hpp"
#include "duration.hpp"
#include "event_watcher_base.hpp"
#include "sys/sys_sockets.h"

#include <functional>

namespace eventpp
{
class EventLoop;
class EVENTPP_EXPORT SignalEventWatcher : public EventWatcherBase
{
public:
    SignalEventWatcher(signal_number_t signo, EventLoop * loop, const Handler & handler);
    SignalEventWatcher(signal_number_t signo, EventLoop * loop, Handler && handler);

    ~SignalEventWatcher();

    bool AsyncWait();
    bool Init();
    void Close();

private:
    static void HandlerFn(signal_number_t sn, short which, void * v);

    int signo_;
};
}


#endif // __EVENT_WATCHER_TIMER_HPP__

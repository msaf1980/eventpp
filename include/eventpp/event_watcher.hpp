#ifndef __EVENTPP_EVENT_WATCHER_HPP__
#define __EVENTPP_EVENT_WATCHER_HPP__

#include "base.hpp"
#include "duration.hpp"
#include "event_watcher_base.hpp"
#include "sys/sys_sockets.h"

#include <functional>

namespace eventpp
{
class EventLoop;

class EVENTPP_EXPORT EventWatcher : public EventWatcherBase
{
public:
    EventWatcher(EventLoop * loop, const Handler & handler);
    EventWatcher(EventLoop * loop, Handler && handler);
    ~EventWatcher();

    bool AsyncWait();
    bool Notify();

    void Close();
    static void HandlerFn(eventpp_socket_t fd, short which, void * v);

    bool Init();
private:
    eventpp_socket_t pipe_[2]; // Write to pipe_[0] , Read from pipe_[1]
};

}

#endif // __EVENT_WATCHER_HPP__

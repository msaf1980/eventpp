#ifndef INNER_PRE
#define INNER_PRE

#include <assert.h>
#include <stdint.h>

#include <functional>
#include <iostream>
#include <memory>

#include "sockets.hpp"
#include "sys/platform_config.h"
#include "sys/sys_addrinfo.h"
#include "sys/sys_sockets.h"
//#include "logging.h"

struct event;
#ifdef H_DEBUG_MODE
int EventAdd(struct event * ev, const struct timeval * timeout);
int EventDel(struct event *);
#else
#    define EventAdd(ev, timeout) event_add(ev, timeout)
#    define EventDel(ev) event_del(ev)
#endif

namespace eventpp
{
EVPP_EXPORT int GetActiveEventCount();
}


#endif /* INNER_PRE */

#ifndef __EVENTPP_TRACE_HPP__
#define __EVENTPP_TRACE_HPP__

// clang-format off
#ifdef DEBUG
#include <iostream>
#define DLOG_TRACE std::cout << __FILE__ << ":" << __LINE__ << " "
#else
#define DLOG_TRACE
#endif
// clang-format on

#endif /* __EVENTPP_TRACE_HPP__ */

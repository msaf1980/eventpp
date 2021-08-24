#ifndef __EVENTPP_PLATFORM_CONFIG_H__
#define __EVENTPP_PLATFORM_CONFIG_H__

#if defined(__APPLE__)
#define H_OS_MACOSX
#endif

#ifdef DEBUG
#ifndef H_DEBUG_MODE
#define H_DEBUG_MODE
#endif
#endif

#include "windows_port.h"

#endif // __EVENTPP_PLATFORM_CONFIG_H__

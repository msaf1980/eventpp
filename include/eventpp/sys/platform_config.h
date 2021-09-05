#ifndef __EVENTPP_PLATFORM_CONFIG_H__
#define __EVENTPP_PLATFORM_CONFIG_H__

#if defined(__APPLE__)
#    define H_OS_MACOSX
#endif

#ifdef DEBUG
#    ifndef H_DEBUG_MODE
#        define H_DEBUG_MODE
#    endif
#endif

#ifndef HAS_LINUX_EVENTFD
#    if __linux
#        include <features.h>
#        include <linux/version.h>
#        if __GLIBC_PREREQ(2, 8) && LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 30)
#            define HAS_LINUX_EVENTFD
#        endif
#    endif
#endif

#include "windows_port.h"

#endif // __EVENTPP_PLATFORM_CONFIG_H__

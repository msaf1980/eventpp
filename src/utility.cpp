#include "pch.h"

// #include "evpp/inner_pre.h"

#include <string.h>

namespace eventpp
{
static const std::string empty_string;

std::string strerror(int e)
{
#ifdef H_OS_WINDOWS
    LPVOID buf = nullptr;
    ::FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        e,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&buf,
        0,
        nullptr);

    if (buf)
    {
        std::string s = (char *)buf;
        LocalFree(buf);
        return s;
    }

#elif defined(H_OS_MACOSX)
    char buf[2048] = {};
    int rc = strerror_r(e, buf, sizeof(buf) - 1); // XSI-compliant
    if (rc == 0)
    {
        return std::string(buf);
    }
#else
    char buf[2048] = {};
#    if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600) && !_GNU_SOURCE
    int rc = strerror_r(e, buf, sizeof(buf) - 1); // XSI-compliant
    if (rc == 0)
    {
        return std::string(buf);
    }
#    else
    const char * s = strerror_r(e, buf, sizeof(buf) - 1); // GNU-specific
    if (s)
    {
        return std::string(s);
    }
#    endif
#endif
    return empty_string;
}

}

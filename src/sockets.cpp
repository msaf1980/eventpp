#include "pch.h"

#include <limits.h>

#include <eventpp/base.hpp>
#include <eventpp/duration.hpp>
#include <eventpp/sockets.hpp>

namespace eventpp
{
namespace sock
{
    static const std::string empty_string;

    evpp_socket_t CreateNonblockingSocket()
    {
        int serrno = 0;

        /* Create listen socket */
        evpp_socket_t fd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (fd == -1)
        {
            // serrno = errno;
            // LOG_ERROR << "socket error " << strerror(serrno);
            return INVALID_SOCKET;
        }

        if (evutil_make_socket_nonblocking(fd) < 0)
        {
            serrno = errno;
            goto out;
        }

#ifndef H_OS_WINDOWS
        if (fcntl(fd, F_SETFD, 1) == -1)
        {
            serrno = errno;
            // LOG_FATAL << "fcntl(F_SETFD)" << strerror(serrno);
            goto out;
        }
#endif

        SetKeepAlive(fd, true);
        SetReuseAddr(fd);
        SetReusePort(fd);
        return fd;
    out:
        EVUTIL_CLOSESOCKET(fd);
        errno = serrno;
        return INVALID_SOCKET;
    }

    evpp_socket_t CreateUDPServer(int port)
    {
        evpp_socket_t fd = socket(AF_INET, SOCK_DGRAM, 0);
        if (fd == -1)
        {
            // int serrno = errno;
            // LOG_ERROR << "socket error " << strerror(serrno);
            return INVALID_SOCKET;
        }
        SetReuseAddr(fd);
        SetReusePort(fd);

        std::string addr = std::string("0.0.0.0:") + std::to_string(port);
        struct sockaddr_storage local = ParseFromIPPort(addr.c_str());
        if (bind(fd, (struct sockaddr *)&local, sizeof(struct sockaddr)))
        {
            int serrno = errno;
            // LOG_ERROR << "socket bind error=" << serrno << " " << strerror(serrno);
            return INVALID_SOCKET;
        }

        return fd;
    }

    bool ToSockaddr(const std::string_view & host, unsigned short port, struct sockaddr_storage & ss)
    {
        memset(&ss, 0, sizeof(ss));
  
        short family = AF_INET;
        auto index = host.find(':');
        if (index != std::string::npos)
        {
            family = AF_INET6;
        }

        struct sockaddr_in * addr = sockaddr_in_cast(&ss);
        int rc = ::evutil_inet_pton(family, host.data(), &addr->sin_addr);
        if (rc == 0)
        {
            // LOG_INFO << "ParseFromIPPort evutil_inet_pton (AF_INET '" << host.data() << "', ...) rc=0. " << host.data() << " is not a valid IP address. Maybe it is a hostname.";
            return false;
        }
        else if (rc < 0)
        {
            // int serrno = errno;
            // if (serrno == 0) {
            //     LOG_INFO << "[" << host.data() << "] is not a IP address. Maybe it is a hostname.";
            // } else {
            //     LOG_WARN << "ParseFromIPPort evutil_inet_pton (AF_INET, '" << host.data() << "', ...) failed : " << strerror(serrno);
            // }
            return false;
        }

        addr->sin_family = family;
        addr->sin_port = htons(port);

        return true;
    }

    bool ParseFromIPPort(const char * address, struct sockaddr_storage & ss)
    {
        memset(&ss, 0, sizeof(ss));
        std::string host;
        unsigned short port;
        if (!SplitHostPort(address, host, port))
        {
            return false;
        }

        return ToSockaddr(host, port, ss);
    }

    bool SplitHostPort(const std::string_view address, std::string & host, unsigned short & port)
    {
        if (address.empty())
        {
            return false;
        }

        size_t index = address.rfind(':');
        if (index == std::string::npos)
        {
            // LOG_ERROR << "Address specified error <" << address << ">. Cannot find ':'";
            return false;
        }

        if (index == address.size() - 1)
        {
            return false;
        }

        // TODO: detect service instead of port number
        int port_ = std::atoi(&address.data()[index + 1]);
        if (port_ < 0 || port_ > USHRT_MAX) {
            return false;
        }
        port = static_cast<unsigned short>(port_);

        // TODO: reduce allocations
        host = address.substr(0, index);
        if (host[0] == '[')
        {
            if (*host.rbegin() != ']')
            {
                // LOG_ERROR << "Address specified error <" << address << ">. '[' ']' is not pair.";
                return false;
            }

            // trim the leading '[' and trail ']'
            host = std::string(host.data() + 1, host.size() - 2);
        }

        // Compatible with "fe80::886a:49f3:20f3:add2]:80"
        if (*host.rbegin() == ']')
        {
            // trim the trail ']'
            host = std::string(host.data(), host.size() - 1);
        }

        return true;
    }

    bool SplitHostPort(const char * address, std::string & host, unsigned short & port)
    {
        std::string_view a(address, strlen(address));

        return SplitHostPort(a, host, port);
    }

    struct sockaddr_storage GetLocalAddr(evpp_socket_t sockfd)
    {
        struct sockaddr_storage laddr;
        memset(&laddr, 0, sizeof laddr);
        socklen_t addrlen = static_cast<socklen_t>(sizeof laddr);
        if (::getsockname(sockfd, sockaddr_cast(&laddr), &addrlen) < 0)
        {
            // LOG_ERROR << "GetLocalAddr:" << strerror(errno);
            memset(&laddr, 0, sizeof laddr);
        }

        return laddr;
    }

    std::string ToIPPort(const struct sockaddr_storage * ss)
    {
        std::string saddr;
        int port = 0;

        if (ss->ss_family == AF_INET)
        {
            struct sockaddr_in * addr4 = const_cast<struct sockaddr_in *>(sockaddr_in_cast(ss));
            char buf[INET_ADDRSTRLEN] = {};
            const char * addr = ::evutil_inet_ntop(ss->ss_family, &addr4->sin_addr, buf, INET_ADDRSTRLEN);

            if (addr)
            {
                saddr = addr;
            }

            port = ntohs(addr4->sin_port);
        }
        else if (ss->ss_family == AF_INET6)
        {
            struct sockaddr_in6 * addr6 = const_cast<struct sockaddr_in6 *>(sockaddr_in6_cast(ss));
            char buf[INET6_ADDRSTRLEN] = {};
            const char * addr = ::evutil_inet_ntop(ss->ss_family, &addr6->sin6_addr, buf, INET6_ADDRSTRLEN);

            if (addr)
            {
                saddr = std::string("[") + addr + "]";
            }

            port = ntohs(addr6->sin6_port);
        }
        else
        {
            // LOG_ERROR << "unknown socket family connected";
            return empty_string;
        }

        if (!saddr.empty())
        {
            saddr.append(":", 1).append(std::to_string(port));
        }

        return saddr;
    }

    std::string ToIPPort(const struct sockaddr * ss) { return ToIPPort(sockaddr_storage_cast(ss)); }

    std::string ToIPPort(const struct sockaddr_in * ss) { return ToIPPort(sockaddr_storage_cast(ss)); }

    std::string ToIP(const struct sockaddr * s)
    {
        auto ss = sockaddr_storage_cast(s);
        if (ss->ss_family == AF_INET)
        {
            struct sockaddr_in * addr4 = const_cast<struct sockaddr_in *>(sockaddr_in_cast(ss));
            char buf[INET_ADDRSTRLEN] = {};
            const char * addr = ::evutil_inet_ntop(ss->ss_family, &addr4->sin_addr, buf, INET_ADDRSTRLEN);
            if (addr)
            {
                return std::string(addr);
            }
        }
        else if (ss->ss_family == AF_INET6)
        {
            struct sockaddr_in6 * addr6 = const_cast<struct sockaddr_in6 *>(sockaddr_in6_cast(ss));
            char buf[INET6_ADDRSTRLEN] = {};
            const char * addr = ::evutil_inet_ntop(ss->ss_family, &addr6->sin6_addr, buf, INET6_ADDRSTRLEN);
            if (addr)
            {
                return std::string(addr);
            }
        }
        // else
        // {
        //     LOG_ERROR << "unknown socket family connected";
        // }

        return empty_string;
    }

    int SetTimeout(evpp_socket_t fd, uint32_t timeout_ms)
    {
#ifdef H_OS_WINDOWS
        DWORD tv = timeout_ms;
#else
        struct timeval tv;
        tv.tv_sec = timeout_ms / 1000;
        tv.tv_usec = (timeout_ms % 1000) * 1000;
#endif
        return setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv));
    }

    int SetTimeout(evpp_socket_t fd, const Duration & timeout) { return SetTimeout(fd, (uint32_t)(timeout.Milliseconds())); }

    int SetKeepAlive(evpp_socket_t fd, bool on)
    {
        int optval = on ? 1 : 0;
        return setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, reinterpret_cast<const char *>(&optval), static_cast<socklen_t>(sizeof optval));
    }

    int SetReuseAddr(evpp_socket_t fd)
    {
        int optval = 1;
        return setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char *>(&optval), static_cast<socklen_t>(sizeof optval));
    }

    int SetReusePort(evpp_socket_t fd)
    {
#ifdef SO_REUSEPORT
        int optval = 1;
        return setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, reinterpret_cast<const char *>(&optval), static_cast<socklen_t>(sizeof optval));
#else
        errno = ENOTSUP;
        return -1;
#endif
    }


    int SetTCPNoDelay(evpp_socket_t fd, bool on)
    {
        int optval = on ? 1 : 0;
        return setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char *>(&optval), static_cast<socklen_t>(sizeof optval));
    }

}
}

#ifdef H_OS_WINDOWS
int readv(evpp_socket_t sockfd, struct iovec * iov, int iovcnt)
{
    DWORD readn = 0;
    DWORD flags = 0;

    if (::WSARecv(sockfd, iov, iovcnt, &readn, &flags, nullptr, nullptr) == 0)
    {
        return readn;
    }

    return -1;
}
#endif

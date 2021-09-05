#ifndef SOCKETS
#define SOCKETS

#include <string>

#include <string.h>

#include "sys/platform_config.h"
#include "sys/sys_addrinfo.h"
#include "sys/sys_sockets.h"

namespace eventpp
{
class Duration;

namespace sock
{
    EVENTPP_EXPORT eventpp_socket_t CreateNonblockingSocket();
    EVENTPP_EXPORT eventpp_socket_t CreateUDPServer(int port);
    EVENTPP_EXPORT int SetKeepAlive(eventpp_socket_t fd, bool on);
    EVENTPP_EXPORT int SetReuseAddr(eventpp_socket_t fd);
    EVENTPP_EXPORT int SetReusePort(eventpp_socket_t fd);
    EVENTPP_EXPORT int SetTCPNoDelay(eventpp_socket_t fd, bool on);
    EVENTPP_EXPORT int SetTimeout(eventpp_socket_t fd, uint32_t timeout_ms);
    EVENTPP_EXPORT int SetTimeout(eventpp_socket_t fd, const Duration & timeout);
    EVENTPP_EXPORT std::string ToIPPort(const struct sockaddr_storage * ss);
    EVENTPP_EXPORT std::string ToIPPort(const struct sockaddr * ss);
    EVENTPP_EXPORT std::string ToIPPort(const struct sockaddr_in * ss);
    EVENTPP_EXPORT std::string ToIP(const struct sockaddr * ss);

    // @brief Return an internet protocol family address from host address/port
    // @param[in] host - A network address of the form "host"
    // @param[in] port - Port
    // @return bool - false if parse failed.
    EVENTPP_EXPORT bool ToSockaddr(const std::string_view & host, unsigned short port, struct sockaddr_storage & ss);

    // @brief Parse a literal network address and return an internet protocol family address
    // @param[in] address - A network address of the form "host:port" or "[host]:port"
    // @return bool - false if parse failed.
    EVENTPP_EXPORT bool ParseFromIPPort(const char * address, struct sockaddr_storage & ss);

    inline struct sockaddr_storage ParseFromIPPort(const char * address)
    {
        struct sockaddr_storage ss;
        bool rc = ParseFromIPPort(address, ss);
        if (rc)
        {
            return ss;
        }
        else
        {
            memset(&ss, 0, sizeof(ss));
            return ss;
        }
    }

    // @brief Splits a network address of the form "host:port" or "[host]:port"
    //  into host and port. A literal address or host name for IPv6
    // must be enclosed in square brackets, as in "[::1]:80" or "[ipv6-host]:80"
    // @param[in] address - A network address of the form "host:port" or "[ipv6-host]:port"
    // @param[out] host -
    // @param[out] port - the port in local machine byte order
    // @return bool - false if the network address is invalid format
    EVENTPP_EXPORT bool SplitHostPort(const char * address, std::string & host, unsigned short & port);

    // @brief Splits a network address of the form "host:port" or "[host]:port"
    //  into host and port. A literal address or host name for IPv6
    // must be enclosed in square brackets, as in "[::1]:80" or "[ipv6-host]:80"
    // @param[in] address - A network address of the form "host:port" or "[ipv6-host]:port"
    // @param[out] host -
    // @param[out] port - the port in local machine byte order
    // @return bool - false if the network address is invalid format
    EVENTPP_EXPORT bool SplitHostPort(const std::string_view address, std::string & host, unsigned short & port);

    EVENTPP_EXPORT struct sockaddr_storage GetLocalAddr(eventpp_socket_t sockfd);

    inline bool IsZeroAddress(const struct sockaddr_storage * ss)
    {
        const char * p = reinterpret_cast<const char *>(ss);
        for (size_t i = 0; i < sizeof(*ss); ++i)
        {
            if (p[i] != 0)
            {
                return false;
            }
        }
        return true;
    }

    template <typename To, typename From>
    inline To implicit_cast(From const & f)
    {
        return f;
    }

    inline const struct sockaddr * sockaddr_cast(const struct sockaddr_in * addr)
    {
        return static_cast<const struct sockaddr *>(eventpp::sock::implicit_cast<const void *>(addr));
    }

    inline struct sockaddr * sockaddr_cast(struct sockaddr_in * addr)
    {
        return static_cast<struct sockaddr *>(eventpp::sock::implicit_cast<void *>(addr));
    }

    inline struct sockaddr * sockaddr_cast(struct sockaddr_storage * addr)
    {
        return static_cast<struct sockaddr *>(eventpp::sock::implicit_cast<void *>(addr));
    }

    inline const struct sockaddr_in * sockaddr_in_cast(const struct sockaddr * addr)
    {
        return static_cast<const struct sockaddr_in *>(eventpp::sock::implicit_cast<const void *>(addr));
    }

    inline struct sockaddr_in * sockaddr_in_cast(struct sockaddr * addr)
    {
        return static_cast<struct sockaddr_in *>(eventpp::sock::implicit_cast<void *>(addr));
    }

    inline struct sockaddr_in * sockaddr_in_cast(struct sockaddr_storage * addr)
    {
        return static_cast<struct sockaddr_in *>(eventpp::sock::implicit_cast<void *>(addr));
    }

    inline struct sockaddr_in6 * sockaddr_in6_cast(struct sockaddr_storage * addr)
    {
        return static_cast<struct sockaddr_in6 *>(eventpp::sock::implicit_cast<void *>(addr));
    }

    inline const struct sockaddr_in * sockaddr_in_cast(const struct sockaddr_storage * addr)
    {
        return static_cast<const struct sockaddr_in *>(eventpp::sock::implicit_cast<const void *>(addr));
    }

    inline const struct sockaddr_in6 * sockaddr_in6_cast(const struct sockaddr_storage * addr)
    {
        return static_cast<const struct sockaddr_in6 *>(eventpp::sock::implicit_cast<const void *>(addr));
    }

    inline const struct sockaddr_storage * sockaddr_storage_cast(const struct sockaddr * addr)
    {
        return static_cast<const struct sockaddr_storage *>(eventpp::sock::implicit_cast<const void *>(addr));
    }

    inline const struct sockaddr_storage * sockaddr_storage_cast(const struct sockaddr_in * addr)
    {
        return static_cast<const struct sockaddr_storage *>(eventpp::sock::implicit_cast<const void *>(addr));
    }

    inline const struct sockaddr_storage * sockaddr_storage_cast(const struct sockaddr_in6 * addr)
    {
        return static_cast<const struct sockaddr_storage *>(eventpp::sock::implicit_cast<const void *>(addr));
    }

}

}

#ifdef H_OS_WINDOWS
EVENTPP_EXPORT int readv(eventpp_socket_t sockfd, struct iovec * iov, int iovcnt);
#endif


#endif /* SOCKETS */

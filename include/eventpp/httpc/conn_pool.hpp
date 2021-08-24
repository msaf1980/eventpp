#ifndef __EVENTPP_CONN_POOL_HPP__
#define __EVENTPP_CONN_POOL_HPP__


#include <map>
#include <mutex>
#include <vector>

#include "../duration.hpp"
#include "../event_loop.hpp"

namespace eventpp
{
namespace httpc
{
    class Conn;
    typedef std::shared_ptr<Conn> ConnPtr;
    class EVPP_EXPORT ConnPool
    {
    public:
        ConnPool(const std::string & host, int port, Duration timeout, bool enable_ssl = false, size_t max_pool_size = 1024);
        ~ConnPool();

        ConnPtr Get(EventLoop * loop);
        void Put(const ConnPtr & c);

        // To make sure all Conn are released in it's own EventLoop
        void Clear();

        const std::string & host() const { return host_; }
        int port() const { return port_; }

        bool enable_ssl() const {
#if defined(EVENTPP_HTTP_CLIENT_SUPPORTS_SSL)
            return enable_ssl_;
#else
            return false;
#endif
        }

        Duration timeout() const { return timeout_; }

    private:
        std::string host_;
        int port_;
#if defined(EVENTPP_HTTP_CLIENT_SUPPORTS_SSL)
        bool enable_ssl_;
#endif
        Duration timeout_;
        size_t max_pool_size_; // The max size of the pool for every EventLoop

        std::mutex mutex_; // The guard of pools_
        std::map<EventLoop *, std::vector<ConnPtr>> pool_; // Every thread has its own pool which has a max size specified by max_pool_size_
    };
} // httpc
} // evpp

#endif // __EVENTPP_CONN_POOL_HPP__
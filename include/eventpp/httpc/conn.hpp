#ifndef __EVENTPP_HTTPC_CONN_HPP__
#define __EVENTPP_HTTPC_CONN_HPP__


#include "../base.hpp"
#include "../event_loop.hpp"

#if defined(EVENTPP_HTTP_CLIENT_SUPPORTS_SSL)
#    include <event2/bufferevent_ssl.h>
#    include <openssl/ssl.h>
#endif

struct evhttp_connection;
namespace eventpp
{
namespace httpc
{
    class ConnPool;
    class EVPP_EXPORT Conn
    {
    public:
        Conn(eventpp::EventLoop * loop, const std::string & host, int port, eventpp::Duration timeout, bool enable_ssl = false);
        ~Conn();

        bool Init();
        void Close();

        eventpp::EventLoop * loop() { return loop_; }
        struct evhttp_connection * evhttp_conn() { return evhttp_conn_; }
        const std::string & host() const { return host_; }
        int port() const { return port_; }

        bool enable_ssl() const { return enable_ssl_; }
        struct bufferevent * bufferevent() const
        {
#if defined(EVENTPP_HTTP_CLIENT_SUPPORTS_SSL)
            return bufferevent_;
#else
            return nullptr;
#endif
        }

        eventpp::Duration timeout() const { return timeout_; }

    private:
        friend class ConnPool;
        Conn(ConnPool * pool, eventpp::EventLoop * loop);
        ConnPool * pool() { return pool_; }

    private:
        eventpp::EventLoop * loop_;
        ConnPool * pool_;
        std::string host_;
        int port_;

        bool enable_ssl_;
#if defined(EVENTPP_HTTP_CLIENT_SUPPORTS_SSL)
        SSL * ssl_;
        struct bufferevent * bufferevent_;
#endif
        eventpp::Duration timeout_;
        struct evhttp_connection * evhttp_conn_;
    };
} // httpc
} // eventpp

#endif // __EVENTPP_HTTPC_CONN_HPP__

#ifndef __EVENTPP_LISTENER_HPP__
#define __EVENTPP_LISTENER_HPP__

#include "base.hpp"
#include "timestamp.hpp"

namespace eventpp {
class EventLoop;
class FdChannel;

class EVPP_EXPORT Listener {
public:
    typedef std::function <
    void(evpp_socket_t sockfd,
         const std::string& /*remote address with format "ip:port"*/,
         const struct sockaddr_in* /*remote address*/) >
    NewConnectionCallback;
    Listener(EventLoop* loop, const std::string_view & addr/*local listening address : ip:port*/);
    Listener(EventLoop* loop, const std::string_view & host, unsigned short port);
    ~Listener();

    // socket listen
    bool Listen(int backlog = SOMAXCONN);

    // nonblocking accept
    void Accept();

    void Stop();

    void SetNewConnectionCallback(NewConnectionCallback cb) {
        new_conn_fn_ = cb;
    }

private:
    void HandleAccept();

    evpp_socket_t fd_ = -1;// The listening socket fd
    EventLoop* loop_;
    std::string host_;
    unsigned short port_;
    std::unique_ptr<FdChannel> chan_;
    NewConnectionCallback new_conn_fn_;
};
}

#endif // __EVENTPP_LISTENER_HPP__

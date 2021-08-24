#include "pch.h"

#include <eventpp/event_loop.hpp>
#include <eventpp/fd_channel.hpp>
#include <eventpp/listener.hpp>
#include <eventpp/sockets.hpp>

namespace eventpp
{
Listener::Listener(EventLoop * l, const std::string & addr) : loop_(l), addr_(addr)
{
    // DLOG_TRACE << "addr=" << addr;
}

Listener::~Listener()
{
    // DLOG_TRACE << "fd=" << chan_->fd();
    chan_.reset();
    EVUTIL_CLOSESOCKET(fd_);
    fd_ = INVALID_SOCKET;
}

bool Listener::Listen(int backlog)
{
    fd_ = sock::CreateNonblockingSocket();
    if (fd_ < 0)
    {
        return false;
    }

    struct sockaddr_storage addr = sock::ParseFromIPPort(addr_.data());
    // TODO Add retry when failed
    int ret = bind(fd_, sock::sockaddr_cast(&addr), static_cast<socklen_t>(sizeof(struct sockaddr)));
    if (ret < 0)
    {
        return false;
    }

    return listen(fd_, backlog) == 0;
}

void Listener::Accept()
{
    chan_.reset(new FdChannel(loop_, fd_, true, false));
    chan_->SetReadCallback(std::bind(&Listener::HandleAccept, this));
    loop_->RunInLoop(std::bind(&FdChannel::AttachToLoop, chan_.get()));
}

void Listener::HandleAccept()
{
    // DLOG_TRACE << "A new connection is comming in";
    assert(loop_->IsInLoopThread());
    struct sockaddr_storage ss;
    socklen_t addrlen = sizeof(ss);
    int nfd = -1;
    if ((nfd = accept(fd_, sock::sockaddr_cast(&ss), &addrlen)) == -1)
    {
        int serrno = errno;
        if (serrno != EAGAIN && serrno != EINTR)
        {
            // LOG_WARN << __FUNCTION__ << " bad accept " << strerror(serrno);
        }
        return;
    }

    if (evutil_make_socket_nonblocking(nfd) < 0)
    {
        // LOG_ERROR << "set fd=" << nfd << " nonblocking failed.";
        EVUTIL_CLOSESOCKET(nfd);
        return;
    }

    sock::SetKeepAlive(nfd, true);

    std::string raddr = sock::ToIPPort(&ss);
    if (raddr.empty())
    {
        // LOG_ERROR << "sock::ToIPPort(&ss) failed.";
        EVUTIL_CLOSESOCKET(nfd);
        return;
    }

    // DLOG_TRACE << "accepted a connection from " << raddr << ", listen fd=" << fd_ << ", client fd=" << nfd;

    if (new_conn_fn_)
    {
        new_conn_fn_(nfd, raddr, sock::sockaddr_in_cast(&ss));
    }
}

void Listener::Stop()
{
    assert(loop_->IsInLoopThread());
    chan_->DisableAllEvent();
    chan_->Close();
}
}

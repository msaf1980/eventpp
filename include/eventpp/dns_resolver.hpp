#ifndef DNS_RESOLVER
#define DNS_RESOLVER

#include "inner_pre.hpp"
#include "duration.hpp"
#include "sys/sys_addrinfo.h"

struct evdns_base;
struct evdns_getaddrinfo_request;
namespace eventpp {
class EventLoop;
class TimerEventWatcher;
class EVENTPP_EXPORT DNSResolver : public std::enable_shared_from_this<DNSResolver> {
public:
    //TODO IPv6 DNS resolver
    typedef std::function<void(const std::vector<struct in_addr>& addrs, int err)> Functor;

    DNSResolver(EventLoop* evloop, const std::string& host, Duration timeout, const Functor& f);
    ~DNSResolver();
    void Start();
    void Cancel();
    const std::string& host() const {
        return host_;
    }
private:
    void SyncDNSResolve();
#if LIBEVENT_VERSION_NUMBER >= 0x02001500    
    void AsyncDNSResolve();
#endif
    void AsyncWait();
    void OnTimeout();
    void OnCanceled();
    void ClearTimer();
    void OnResolved(int errcode, struct addrinfo* addr);
    void OnResolved();
    static void OnResolved(int errcode, struct addrinfo* addr, void* arg);

    EventLoop* loop_;
    struct evdns_base* dnsbase_;
    struct evdns_getaddrinfo_request* dns_req_;
    std::string host_;
    Duration timeout_;
    Functor functor_;
    int err_;
    std::unique_ptr<TimerEventWatcher> timer_;
    std::vector<struct in_addr> addrs_;
};

}

#endif /* DNS_RESOLVER */

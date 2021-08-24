#include <eventpp/dns_resolver.hpp>
#include <eventpp/event_loop.hpp>
#include <eventpp/event_loop_thread.hpp>

#include <memory>

#include "doctest.h"

TEST_CASE("testDNSResolver")
{
    for (int i = 0; i < 6; i++)
    {
        std::atomic<int> resolved(0);
        bool deleted = false;
        std::vector<struct in_addr> addrs_resolved;
        int err_resolved = 0;

        auto fn_resolved = [&resolved, &addrs_resolved, &err_resolved](const std::vector<struct in_addr> & addrs, int err) {
            // LOG_INFO << "Entering fn_resolved";
            addrs_resolved = addrs;
            err_resolved = err;
            resolved.fetch_add(1);
        };

        eventpp::Duration delay(double(3.0)); // 3s
        std::unique_ptr<eventpp::EventLoopThread> t(new eventpp::EventLoopThread);
        t->Start(true);
        std::shared_ptr<eventpp::DNSResolver> dns_resolver(new eventpp::DNSResolver(t->loop(), "www.google.com", delay, fn_resolved));
        dns_resolver->Start();

        while (!resolved)
        {
            usleep(1);
        }
        REQUIRE(err_resolved == 0);
        REQUIRE(addrs_resolved.size() > 0);

        auto fn_deleter = [&deleted, dns_resolver]() {
            // LOG_INFO << "Entering fn_deleter";
            deleted = true;
        };

        t->loop()->QueueInLoop(fn_deleter);
        dns_resolver.reset();
        while (!deleted)
        {
            usleep(1);
        }

        t->Stop(true);
        t.reset();
        REQUIRE(eventpp::GetActiveEventCount() == 0);
    }
}

TEST_CASE("testDNSResolverTimeout")
{
    for (int i = 0; i < 6; i++)
    {
        std::atomic<int> resolved(0);
        bool deleted = false;
        std::vector<struct in_addr> addrs_resolved;
        int err_resolved = 0;

        auto fn_resolved = [&resolved, &addrs_resolved, &err_resolved](const std::vector<struct in_addr> & addrs, int err) {
            std::cerr << "Entering fn_resolved addrs.size=" << addrs.size();
            addrs_resolved = addrs;
            err_resolved = err;
            resolved.fetch_add(1);
        };

        // 1us to make it timeout immediately
        eventpp::Duration delay(double(0.0000001));
        std::unique_ptr<eventpp::EventLoopThread> t(new eventpp::EventLoopThread);
        t->Start(true);

        auto loop = t->loop();
        std::shared_ptr<eventpp::DNSResolver> dns_resolver(
            new eventpp::DNSResolver(loop, "wwwwwww.en.cppreference.com", delay, fn_resolved));
        dns_resolver->Start();

        while (!resolved)
        {
            usleep(1);
        }
        usleep(1);
        REQUIRE(err_resolved == EVUTIL_EAI_CANCEL);
        REQUIRE(addrs_resolved.size() == 0);

        auto fn_deleter = [&deleted, dns_resolver]() {
            // LOG_INFO << "Entering fn_deleter";
            deleted = true;
        };

        loop->QueueInLoop(fn_deleter);
        dns_resolver.reset();
        while (!deleted)
        {
            usleep(1);
        }

        loop->RunAfter(eventpp::Duration(0.05), [loop]() { loop->Stop(); });
        while (!t->IsStopped())
        {
            usleep(1);
        }
        t.reset();
        assert(resolved.load() == 1);
        REQUIRE(eventpp::GetActiveEventCount() == 0);
    }
}

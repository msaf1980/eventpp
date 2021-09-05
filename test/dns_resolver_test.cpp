#include <eventpp/dns_resolver.hpp>
#include <eventpp/event_loop.hpp>
#include <eventpp/event_loop_thread.hpp>

#include <memory>
#include <vector>

#include "test.h"

TEST_CASE("testDNSResolver")
{
    for (int i = 0; i < 6; i++)
    {
        std::atomic<int> resolved(0);
        std::atomic<bool> deleted = false;
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
        std::atomic<bool> deleted = false;
        std::vector<struct in_addr> addrs_resolved;
        int err_resolved = 0;

        auto fn_resolved = [&resolved, &addrs_resolved, &err_resolved](const std::vector<struct in_addr> & addrs, int err) {
            std::cerr << "Entering fn_resolved addrs.size=" << addrs.size() << std::endl;
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

TEST_CASE("TestDNSResolver (Up/Down)")
{
    for (int i = 0; i < 40; i++)
    {
        CAPTURE(i);
        std::atomic<bool> resolved = false;
        volatile int err = 0;
        std::atomic<bool> deleted = false;
        auto fn_resolved = [&resolved, &err](const std::vector<struct in_addr> & addrs, int error) {
            // std::cout << "Entering fn_resolved: " << gai_strerror(err) << std::endl;
            err = error;
            resolved = true;
        };

        eventpp::Duration delay(double(3.0)); // 3s
        std::unique_ptr<eventpp::EventLoopThread> t(new eventpp::EventLoopThread);
        t->Start(true);
        std::shared_ptr<eventpp::DNSResolver> dns_resolver(
            new eventpp::DNSResolver(t->loop(), "www.google.com", eventpp::Duration(1.0), fn_resolved));
        dns_resolver->Start();

        while (!resolved)
        {
            usleep(1);
        }
        usleep(1);
        REQUIRE_MESSAGE(err == 0, gai_strerror(err));

        auto fn_deleter = [&deleted, dns_resolver]() {
            // std::cout << "Entering fn_deleter" << std::endl;
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

TEST_CASE("TestDNSResolver (Scalability)")
{
    std::unique_ptr<eventpp::EventLoopThread> t(new eventpp::EventLoopThread);
    t->Start(true);

    std::atomic<int> resolved = 0;
    std::atomic<int> errors = 0;
    int count = 40;
    std::vector<std::shared_ptr<eventpp::DNSResolver>> queries;

    for (int i = 0; i < count; i++)
    {
        CAPTURE(i);
        auto fn_resolved = [&resolved, &errors](const std::vector<struct in_addr> & addrs, int err) {
            if (err == 0)
            {
                resolved.fetch_add(1);
            }
            else
            {
                std::cout << "Entering fn_resolved: " << gai_strerror(err) << std::endl;
                errors.fetch_add(1);
            }
        };

        eventpp::Duration delay(double(3.0)); // 3s
        std::shared_ptr<eventpp::DNSResolver> dns_resolver(
            new eventpp::DNSResolver(t->loop(), "www.google.com", eventpp::Duration(1.0), fn_resolved));
        dns_resolver->Start();
        queries.push_back(dns_resolver);
    }

    while (resolved.load() + errors.load() < count)
    {
        usleep(1);
    }

    std::atomic<bool> deleted = false;

    auto fn_deleter = [&deleted]() {
        // std::cout << "Entering fn_deleter" << std::endl;
        deleted = true;
    };

    t->loop()->QueueInLoop(fn_deleter);

    while (!deleted)
    {
        usleep(1);
    }
    usleep(1);

    for (int i = 0; i < queries.size(); ++i)
    {
        queries[i].reset();
    }

    REQUIRE(resolved.load() == count);
    REQUIRE(errors.load() == 0);

    t->Stop(true);
    t.reset();
    REQUIRE(eventpp::GetActiveEventCount() == 0);
}

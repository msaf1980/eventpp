#include <iostream>

#include <stdio.h>
#include <stdlib.h>

#include <eventpp/base.hpp>
#include <eventpp/event_loop_thread.hpp>
#include <eventpp/timestamp.hpp>

#include <eventpp/httpc/conn.hpp>
#include <eventpp/httpc/request.hpp>
#include <eventpp/httpc/response.hpp>

#include <eventpp/http/context.hpp>
#include <eventpp/http/http_server.hpp>
#include <eventpp/http/service.hpp>

#include "test.h"

static bool g_stopping = false;

static void
DefaultRequestHandler(eventpp::EventLoop * loop, const eventpp::http::ContextPtr & ctx, const eventpp::http::HTTPSendResponseCallback & cb)
{
    //std::cout << __func__ << " called ...\n";
    std::stringstream oss;
    oss << "func=" << __FUNCTION__ << "\n"
        << " ip=" << ctx->remote_ip() << "\n"
        << " uri=" << ctx->uri() << "\n"
        << " body=" << ctx->body().ToString() << "\n";

    if (ctx->uri().find("stop") != std::string::npos)
    {
        g_stopping = true;
    }

    cb(oss.str());
}

static void RequestHandlerHTTPClientRetry(
    eventpp::EventLoop * loop, const eventpp::http::ContextPtr & ctx, const eventpp::http::HTTPSendResponseCallback & cb)
{
    std::stringstream oss;
    oss << "func=" << __FUNCTION__ << " OK"
        << " ip=" << ctx->remote_ip() << "\n"
        << " uri=" << ctx->uri() << "\n"
        << " body=" << ctx->body().ToString() << "\n";
    static std::atomic<int> i(0);
    if (i++ == 0)
    {
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }
    cb(oss.str());
}

namespace
{
static std::vector<int> g_listening_port = {49000, 49001};

static std::string GetHttpServerURL()
{
    static int i = 0;
    std::ostringstream oss;
    oss << "http://127.0.0.1:" << g_listening_port[(i++ % g_listening_port.size())];
    return oss.str();
}

void testRequestHandlerRetry(eventpp::EventLoop * loop, int * finished)
{
    std::string uri = "/retry";
    std::string url = GetHttpServerURL() + uri;
    auto r = new eventpp::httpc::Request(loop, url, "", eventpp::Duration(2.0));
    r->set_retry_number(3);
    auto f = [r, finished](const std::shared_ptr<eventpp::httpc::Response> & response) {
        std::string result = response->body().ToString();
        REQUIRE(!result.empty());
        REQUIRE(result.find("uri=/retry") != std::string::npos);
        *finished += 1;
        delete r;
    };

    r->Execute(f);
}

void testStop(eventpp::EventLoop * loop, int * finished)
{
    std::string uri = "/mod/stop";
    std::string url = GetHttpServerURL() + uri;
    auto r = new eventpp::httpc::Request(loop, url, "", eventpp::Duration(10.0));
    auto f = [r, finished](const std::shared_ptr<eventpp::httpc::Response> & response) {
        std::string result = response->body().ToString();
        REQUIRE(!result.empty());
        REQUIRE(result.find("uri=/mod/stop") != std::string::npos);
        REQUIRE(result.find("func=DefaultRequestHandler") != std::string::npos);
        *finished += 1;
        delete r;
    };

    r->Execute(f);
}

static void TestHTTPClientRetry()
{
    eventpp::EventLoopThread t;
    t.Start(true);
    int finished = 0;
    testRequestHandlerRetry(t.loop(), &finished);
    testStop(t.loop(), &finished);

    while (true)
    {
        usleep(10);

        if (finished == 2)
        {
            break;
        }
    }

    t.Stop(true);
}

void testRequestHandlerRetryFailed(eventpp::EventLoop * loop, int * finished)
{
    std::string uri = "/retry";
    std::string url = GetHttpServerURL() + uri;
    auto r = new eventpp::httpc::Request(loop, url, "", eventpp::Duration(2.0));
    r->set_retry_number(3);
    auto f = [r, finished](const std::shared_ptr<eventpp::httpc::Response> & response) {
        std::string result = response->body().ToString();
        REQUIRE(result.empty());
        *finished += 1;
        delete r;
    };

    r->Execute(f);
}


static void TestHTTPClientRetryFailed()
{
    eventpp::EventLoopThread t;
    t.Start(true);
    int finished = 0;
    testRequestHandlerRetryFailed(t.loop(), &finished);

    while (true)
    {
        usleep(10);

        if (finished == 1)
        {
            break;
        }
    }

    t.Stop(true);
}
}

TEST_CASE("testHTTPClientRetry")
{
    for (int i = 0; i < 1; ++i)
    {
        // LOG_INFO << "Running testHTTPClientRetry i=" << i;
        eventpp::http::Server ph(i);
        ph.RegisterDefaultHandler(&DefaultRequestHandler);
        ph.RegisterHandler("/retry", &RequestHandlerHTTPClientRetry);
        bool r = ph.Init(g_listening_port) && ph.Start();
        REQUIRE(r);
        TestHTTPClientRetry();
        ph.Stop();
        usleep(1000 * 1000); // sleep a while to release the listening address and port
    }
}

TEST_CASE("testHTTPClientRetryFailed")
{
    for (int i = 0; i < 1; ++i)
    {
        // LOG_INFO << "Running testHTTPClientRetry i=" << i;
        TestHTTPClientRetryFailed();
        usleep(1000 * 1000); // sleep a while to release the listening address and port
    }
}

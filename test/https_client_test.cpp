#include <chrono>
#include <iostream>
#include <thread>

#include <eventpp/event_loop_thread.hpp>
#include <eventpp/httpc/conn.hpp>
#include <eventpp/httpc/request.hpp>
#include <eventpp/httpc/response.hpp>
#include <eventpp/ssl/ssl.hpp>

#include "test.h"

void InitSSLOnce()
{
    static std::once_flag flag;
    std::call_once(flag, []() { eventpp::InitSSL(); });
}

std::string HttpFetch(const std::string & url)
{
    InitSSLOnce();
    eventpp::EventLoopThread t;
    t.Start(true);
    eventpp::httpc::GetRequest * req = new eventpp::httpc::GetRequest(t.loop(), url, eventpp::Duration(1.0));
    volatile bool responsed = false;
    std::string ret;
    req->Execute([req, &ret, &responsed](const std::shared_ptr<eventpp::httpc::Response> & response) mutable {
        std::stringstream oss;
        oss << "http_code=" << response->http_code() << std::endl
            << "body [" << std::endl
            << response->body().ToString() << "]" << std::endl;
        ret = oss.str();
        responsed = true;
        // std::cout << ret << std::endl;
        delete req;
    });
    while (!responsed)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    t.Stop(true);

    return ret;
}

TEST_CASE("testHTTPResponse")
{
    try
    {
        std::string response = HttpFetch("http://httpbin.org/headers?show_env=1");
        REQUIRE(!response.empty());
        REQUIRE(response.find("http_code=200") != std::string::npos);
        REQUIRE(response.find("\"Host\": \"httpbin.org\"") != std::string::npos);
        REQUIRE(response.find("\"X-Forwarded-Port\": \"80\"") != std::string::npos);
        REQUIRE(response.find("\"X-Forwarded-Proto\": \"http\"") != std::string::npos);
        // REQUIRE(response.find("\"Connection\": \"close\"") != std::string::npos);
    }
    catch (const std::exception & e)
    {
        REQUIRE_MESSAGE(false, "fetch failed: ", e.what());
    }
}

TEST_CASE("testHTTPSResponse")
{
    try
    {
        std::string response = HttpFetch("https://httpbin.org/headers?show_env=1");
        REQUIRE(!response.empty());
        REQUIRE(response.find("http_code=200") != std::string::npos);
        REQUIRE(response.find("\"Host\": \"httpbin.org\"") != std::string::npos);
        REQUIRE(response.find("\"X-Forwarded-Port\": \"443\"") != std::string::npos);
        REQUIRE(response.find("\"X-Forwarded-Proto\": \"https\"") != std::string::npos);
        // REQUIRE(response.find("\"Connection\": \"close\"") != std::string::npos);
    }
    catch (std::exception & e)
    {
        auto s = e.what();
#ifdef EVENTPP_HTTP_CLIENT_SUPPORTS_SSL
        REQUIRE_MESSAGE(false, "fetch failed: %s", s);
#else
        REQUIRE(std::string(s) == std::string("eventpp compiled without ssl"));
#endif
    }
}

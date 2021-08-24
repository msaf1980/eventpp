#include <iostream>

#include <sys/wait.h>

#include <stdio.h>
#include <stdlib.h>

#include <eventpp/event_loop_thread.hpp>
#include <eventpp/timestamp.hpp>

#include <eventpp/httpc/conn.hpp>
#include <eventpp/httpc/request.hpp>
#include <eventpp/httpc/response.hpp>

#include <eventpp/http/context.hpp>
#include <eventpp/http/http_server.hpp>
#include <eventpp/http/service.hpp>

#define H_TEST_ASSERT assert

static bool g_stopping = false;
static void
RequestHandler(eventpp::EventLoop * loop, const eventpp::http::ContextPtr & ctx, const eventpp::http::HTTPSendResponseCallback & cb)
{
    std::stringstream oss;
    oss << "func=" << __FUNCTION__ << " OK"
        << " ip=" << ctx->remote_ip() << "\n"
        << " uri=" << ctx->uri() << "\n"
        << " body=" << ctx->body().ToString() << "\n";
    cb(oss.str());
}

static void
RequestHandler201(eventpp::EventLoop * loop, const eventpp::http::ContextPtr & ctx, const eventpp::http::HTTPSendResponseCallback & cb)
{
    std::stringstream oss;
    oss << "func=" << __FUNCTION__ << " OK"
        << " ip=" << ctx->remote_ip() << "\n"
        << " uri=" << ctx->uri() << "\n"
        << " body=" << ctx->body().ToString() << "\n";
    ctx->set_response_http_code(201);
    cb(oss.str());
}

static void
RequestHandler909(eventpp::EventLoop * loop, const eventpp::http::ContextPtr & ctx, const eventpp::http::HTTPSendResponseCallback & cb)
{
    std::cout << "RequestHandler909" << std::endl;
    std::stringstream oss;
    oss << "func=" << __FUNCTION__ << " OK"
        << " ip=" << ctx->remote_ip() << "\n"
        << " uri=" << ctx->uri() << "\n"
        << " body=" << ctx->body().ToString() << "\n";
    ctx->set_response_http_code(909);
    cb(oss.str());
}

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

void testDefaultHandler1(eventpp::EventLoop * loop, int * finished)
{
    std::string uri = "/status?a=1";
    std::string url = GetHttpServerURL() + uri;
    auto r = new eventpp::httpc::Request(loop, url, "", eventpp::Duration(10.0));
    auto f = [r, finished](const std::shared_ptr<eventpp::httpc::Response> & response) {
        std::string result = response->body().ToString();
        H_TEST_ASSERT(!result.empty());
        H_TEST_ASSERT(result.find("uri=/status") != std::string::npos);
        H_TEST_ASSERT(result.find("uri=/status?a=1") == std::string::npos);
        H_TEST_ASSERT(result.find("func=DefaultRequestHandler") != std::string::npos);
        *finished += 1;
        delete r;
    };

    r->Execute(f);
}

void testDefaultHandler2(eventpp::EventLoop * loop, int * finished)
{
    std::string uri = "/status";
    std::string body = "The http request body.";
    std::string url = GetHttpServerURL() + uri;
    auto r = new eventpp::httpc::Request(loop, url, body, eventpp::Duration(10.0));
    auto f = [body, r, finished](const std::shared_ptr<eventpp::httpc::Response> & response) {
        std::string result = response->body().ToString();
        H_TEST_ASSERT(!result.empty());
        H_TEST_ASSERT(result.find("uri=/status") != std::string::npos);
        H_TEST_ASSERT(result.find("func=DefaultRequestHandler") != std::string::npos);
        H_TEST_ASSERT(result.find(body.c_str()) != std::string::npos);
        *finished += 1;
        delete r;
    };

    r->Execute(f);
}

void testDefaultHandler3(eventpp::EventLoop * loop, int * finished)
{
    std::string uri = "/status/method/method2/xx";
    std::string url = GetHttpServerURL() + uri;
    auto r = new eventpp::httpc::Request(loop, url, "", eventpp::Duration(10.0));
    auto f = [r, finished](const std::shared_ptr<eventpp::httpc::Response> & response) {
        std::string result = response->body().ToString();
        H_TEST_ASSERT(!result.empty());
        H_TEST_ASSERT(result.find("uri=/status/method/method2/xx") != std::string::npos);
        H_TEST_ASSERT(result.find("func=DefaultRequestHandler") != std::string::npos);
        *finished += 1;
        delete r;
    };

    r->Execute(f);
}

void testPushBootHandler(eventpp::EventLoop * loop, int * finished)
{
    std::string uri = "/push/boot";
    std::string url = GetHttpServerURL() + uri;
    auto r = new eventpp::httpc::Request(loop, url, "", eventpp::Duration(10.0));
    auto f = [r, finished](const std::shared_ptr<eventpp::httpc::Response> & response) {
        std::string result = response->body().ToString();
        H_TEST_ASSERT(!result.empty());
        H_TEST_ASSERT(result.find("uri=/push/boot") != std::string::npos);
        H_TEST_ASSERT(result.find("func=RequestHandler") != std::string::npos);
        *finished += 1;
        delete r;
    };

    r->Execute(f);
}

void testRequestHandler201(eventpp::EventLoop * loop, int * finished)
{
    std::string uri = "/201";
    std::string url = GetHttpServerURL() + uri;
    auto r = new eventpp::httpc::Request(loop, url, "", eventpp::Duration(10.0));
    auto f = [r, finished](const std::shared_ptr<eventpp::httpc::Response> & response) {
        std::string result = response->body().ToString();
        H_TEST_ASSERT(!result.empty());
        H_TEST_ASSERT(response->http_code() == 201);
        H_TEST_ASSERT(result.find("uri=/201") != std::string::npos);
        H_TEST_ASSERT(result.find("func=RequestHandler201") != std::string::npos);
        *finished += 1;
        delete r;
    };

    r->Execute(f);
}

void testRequestHandler909(eventpp::EventLoop * loop, int * finished)
{
    std::string uri = "/909";
    std::string url = GetHttpServerURL() + uri;
    auto r = new eventpp::httpc::Request(loop, url, "", eventpp::Duration(10.0));
    auto f = [r, finished](const std::shared_ptr<eventpp::httpc::Response> & response) {
        std::string result = response->body().ToString();
        H_TEST_ASSERT(!result.empty());
        H_TEST_ASSERT(response->http_code() == 909);
        H_TEST_ASSERT(result.find("uri=/909") != std::string::npos);
        H_TEST_ASSERT(result.find("func=RequestHandler909") != std::string::npos);
        *finished += 1;
        delete r;
    };

    r->Execute(f);
}

void testRequestHandlerUriPathAndParam(eventpp::EventLoop * loop, int * finished)
{
    std::string uri = "/UriPathAndParam?key2=key2value&key1=key1value";
    std::string url = GetHttpServerURL() + uri;
    auto r = new eventpp::httpc::Request(loop, url, "", eventpp::Duration(10.0));
    auto f = [r, finished](const std::shared_ptr<eventpp::httpc::Response> & response) {
        std::string result = response->body().ToString();
        H_TEST_ASSERT(!result.empty());
        H_TEST_ASSERT(response->http_code() == 200);
        H_TEST_ASSERT(result.find("uri=/UriPathAndParam") != std::string::npos);
        H_TEST_ASSERT(result.find("key2=key2value") != std::string::npos);
        H_TEST_ASSERT(result.find("key1=key1value") != std::string::npos);
        H_TEST_ASSERT(result.find("notkey=\n") != std::string::npos);
        H_TEST_ASSERT(result.find("func=RequestHandlerUriPathAndParam") != std::string::npos);
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
        H_TEST_ASSERT(!result.empty());
        H_TEST_ASSERT(result.find("uri=/mod/stop") != std::string::npos);
        H_TEST_ASSERT(result.find("func=DefaultRequestHandler") != std::string::npos);
        *finished += 1;
        delete r;
    };

    r->Execute(f);
}

static void TestAll()
{
    eventpp::EventLoopThread t;
    t.Start(true);
    int finished = 0;
    testDefaultHandler1(t.loop(), &finished);
    testDefaultHandler2(t.loop(), &finished);
    testDefaultHandler3(t.loop(), &finished);
    testPushBootHandler(t.loop(), &finished);
    testRequestHandler201(t.loop(), &finished);
    testRequestHandler909(t.loop(), &finished);
    testStop(t.loop(), &finished);

    while (true)
    {
        usleep(10);

        if (finished == 7)
        {
            break;
        }
    }

    t.Stop(true);
}

static void Test909()
{
    eventpp::EventLoopThread t;
    t.Start(true);
    int finished = 0;
    testRequestHandler909(t.loop(), &finished);
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

}

int main()
{
    int i = 2;
    std::cout << "Running testHTTPServer i=" << i << std::endl;
    eventpp::http::Server ph(i);
    ph.RegisterDefaultHandler(&DefaultRequestHandler);
    ph.RegisterHandler("/909", &RequestHandler909);
    bool r = ph.Init(g_listening_port);
    auto pid = fork();
    if (pid != 0)
    {
        // In parent process
        std::cout << "In parent process. Starting" << std::endl;
        ph.Start();
        std::cout << "In parent process. Stopping" << std::endl;
        ph.Stop();
        std::cout << "In parent process. Stopped" << std::endl;
        int status = 0;
        int wpid = waitpid(-1, &status, 0);
        if (wpid == -1)
        {
            perror("waitpid");
            exit(EXIT_FAILURE);
        }
        int code = WEXITSTATUS(status);
        if (code == 0)
        {
            std::cout << "Success" << std::endl;
        }
        else
        {
            std::cout << "Failed (child exit eith code " << code << std::endl;
        }
        exit(code);
    }

    std::cout << "In child process. Doing AfterFork" << std::endl;
    ph.AfterFork();
    ph.Start();
    H_TEST_ASSERT(r);
    Test909();
    ph.Stop();
    usleep(1000 * 1000); // sleep a while to release the listening address and port
    return 0;
}

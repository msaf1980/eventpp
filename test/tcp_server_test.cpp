#include <memory>
#include <thread>

#include <eventpp/buffer.hpp>
#include <eventpp/event_loop.hpp>
#include <eventpp/event_loop_thread.hpp>
#include <eventpp/event_watcher.hpp>
#include <eventpp/tcp/tcp_client.hpp>
#include <eventpp/tcp/tcp_conn.hpp>
#include <eventpp/tcp/tcp_server.hpp>

#include "doctest.h"

static bool connected = false;
static bool message_recved = false;
const static std::string addr = "127.0.0.1:19099";


void OnClientConnection(const eventpp::TCPConnPtr & conn)
{
    if (conn->IsConnected())
    {
        conn->Send("hello");
        // LOG_INFO << "Send a message to server when connected.";
        connected = true;
    }
    else
    {
        // LOG_INFO << "Disconnected from " << conn->remote_addr();
    }
}

std::shared_ptr<eventpp::TCPClient> StartTCPClient(eventpp::EventLoop * loop)
{
    std::shared_ptr<eventpp::TCPClient> client(new eventpp::TCPClient(loop, addr, "TCPPingPongClient"));
    client->set_reconnect_interval(eventpp::Duration(0.1));
    client->SetConnectionCallback(&OnClientConnection);
    client->Connect();
    loop->RunAfter(eventpp::Duration(1.0), std::bind(&eventpp::TCPClient::Disconnect, client));
    return client;
}


TEST_CASE("testTCPServer1")
{
    //     std::unique_ptr<eventpp::EventLoopThread> tcp_client_thread(new eventpp::EventLoopThread);
    //     tcp_client_thread->set_name("TCPClientThread");
    //     tcp_client_thread->Start(true);
    //     std::unique_ptr<eventpp::EventLoop> loop(new eventpp::EventLoop);
    //     std::unique_ptr<eventpp::TCPServer> tsrv(new eventpp::TCPServer(loop.get(), addr, "tcp_server", 2));
    //     tsrv->SetMessageCallback([](const eventpp::TCPConnPtr& conn,
    //                                 eventpp::Buffer* msg) {
    //         message_recved = true;
    //     });
    //     bool rc = tsrv->Init();
    //     H_TEST_ASSERT(rc);
    //     rc = tsrv->Start();
    //     H_TEST_ASSERT(rc);
    //     loop->RunAfter(eventpp::Duration(1.4), [&tsrv]() { tsrv->Stop(); });
    //     loop->RunAfter(eventpp::Duration(1.6), [&loop]() { loop->Stop(); });
    //     std::shared_ptr<eventpp::TCPClient> client = StartTCPClient(tcp_client_thread->loop());
    //     loop->Run();
    //     tcp_client_thread->Stop(true);
    //     H_TEST_ASSERT(!loop->IsRunning());
    //     H_TEST_ASSERT(tcp_client_thread->IsStopped());
    //     H_TEST_ASSERT(connected);
    //     H_TEST_ASSERT(message_recved);
    //     tcp_client_thread.reset();
    //     loop.reset();
    //     tsrv.reset();
    //     H_TEST_ASSERT(eventpp::GetActiveEventCount() == 0);

    connected = false;
    message_recved = false;
    std::unique_ptr<eventpp::EventLoopThread> tcp_client_thread(new eventpp::EventLoopThread);
    tcp_client_thread->set_name("TCPClientThread");
    tcp_client_thread->Start(true);
    std::unique_ptr<eventpp::EventLoopThread> tcp_server_thread(new eventpp::EventLoopThread);
    tcp_server_thread->Start(true);
    auto loop = tcp_server_thread->loop();
    std::unique_ptr<eventpp::TCPServer> tsrv(new eventpp::TCPServer(loop, addr, "tcp_server", 2));
    tsrv->SetMessageCallback([](const eventpp::TCPConnPtr & conn, eventpp::Buffer * msg) { message_recved = true; });
    bool rc = tsrv->Init();
    assert(rc);
    rc = tsrv->Start();
    assert(rc);
    (void)rc;
    loop->RunAfter(eventpp::Duration(1.4), [&tsrv]() { tsrv->Stop(); });
    std::shared_ptr<eventpp::TCPClient> client = StartTCPClient(tcp_client_thread->loop());
    while (!tsrv->IsStopped())
    {
        usleep(1);
    }
    tcp_server_thread->Stop(true);
    tcp_client_thread->Stop(true);
    REQUIRE(tcp_client_thread->IsStopped());
    REQUIRE(connected);
    REQUIRE(message_recved);
    tcp_client_thread.reset();
    tcp_server_thread.reset();
    tsrv.reset();
    REQUIRE(eventpp::GetActiveEventCount() == 0);
}

TEST_CASE("testTCPServerSilenceShutdown1")
{
    std::unique_ptr<eventpp::EventLoop> loop(new eventpp::EventLoop);
    std::unique_ptr<eventpp::TCPServer> tsrv(new eventpp::TCPServer(loop.get(), addr, "tcp_server", 2));
    bool rc = tsrv->Init();
    REQUIRE(rc);
    rc = tsrv->Start();
    REQUIRE(rc);
    (void)rc;
    loop->RunAfter(eventpp::Duration(1.2), [&tsrv]() { tsrv->Stop(); });
    loop->RunAfter(eventpp::Duration(1.3), [&loop]() { loop->Stop(); });
    loop->Run();
    loop.reset();
    tsrv.reset();
    REQUIRE(eventpp::GetActiveEventCount() == 0);
}

TEST_CASE("testTCPServerSilenceShutdown2")
{
    std::unique_ptr<eventpp::EventLoop> loop(new eventpp::EventLoop);
    std::unique_ptr<eventpp::TCPServer> tsrv(new eventpp::TCPServer(loop.get(), addr, "tcp_server", 2));
    bool rc = tsrv->Init();
    REQUIRE(rc);
    rc = tsrv->Start();
    REQUIRE(rc);
    (void)rc;
    loop->RunAfter(eventpp::Duration(1.0), [&tsrv, &loop]() {
        tsrv->Stop();
        loop->Stop();
    });
    loop->Run();
    loop.reset();
    tsrv.reset();
    REQUIRE(eventpp::GetActiveEventCount() == 0);
}

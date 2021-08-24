#include <thread>
#include <memory>

#include <eventpp/event_watcher.hpp>
#include <eventpp/event_loop.hpp>
#include <eventpp/event_loop_thread.hpp>
#include <eventpp/tcp_server.hpp>
#include <eventpp/buffer.hpp>
#include <eventpp/tcp_conn.hpp>
#include <eventpp/tcp_client.hpp>

#include "doctest.h"

namespace {
static std::shared_ptr<eventpp::TCPServer> tsrv;
static std::atomic<int> connected_count(0);
static std::atomic<int> message_recved_count(0);
const static std::string addr = "127.0.0.1:19099";

void OnClientConnection(const eventpp::TCPConnPtr& conn) {
    if (conn->IsConnected()) {
        conn->Send("hello");
        // LOG_INFO << "Send a message to server when connected.";
        connected_count++;
    // } else {
    //     LOG_INFO << "Disconnected from " << conn->remote_addr();
    }
}

eventpp::TCPClient* StartTCPClient(eventpp::EventLoop* loop) {
    eventpp::TCPClient* client(new eventpp::TCPClient(loop, addr, "TCPPingPongClient"));
    client->set_reconnect_interval(eventpp::Duration(0.1));
    client->SetConnectionCallback(&OnClientConnection);
    client->Connect();
    return client;
}

}


TEST_CASE("testTCPClientReconnect") {
    std::unique_ptr<eventpp::EventLoopThread> tcp_client_thread(new eventpp::EventLoopThread);
    tcp_client_thread->set_name("TCPClientThread");
    tcp_client_thread->Start(true);
    std::unique_ptr<eventpp::EventLoopThread> tcp_server_thread(new eventpp::EventLoopThread);
    tcp_server_thread->set_name("TCPServerThread");
    tcp_server_thread->Start(true);
    eventpp::TCPClient* client = StartTCPClient(tcp_client_thread->loop());

    int test_count = 3;
    for (int i = 0; i < test_count; i++) {
        // LOG_INFO << "NNNNNNNNNNNNNNNN TestTCPClientReconnect i=" << i;
        tsrv.reset(new eventpp::TCPServer(tcp_server_thread->loop(), addr, "tcp_server", i));
        tsrv->SetMessageCallback([](const eventpp::TCPConnPtr& conn,
                                    eventpp::Buffer* msg) {
            message_recved_count++;
        });
        auto rc = tsrv->Init();
        rc = rc && tsrv->Start();
        REQUIRE(rc);
        usleep(eventpp::Duration(2.0).Microseconds());
        tsrv->Stop();
        while (!tsrv->IsStopped()) {
            usleep(1);
        }
        tsrv.reset();
    }
    // LOG_INFO << "XXXXXXXXXX connected_count=" << connected_count << " message_recved_count=" << message_recved_count;
    tcp_client_thread->loop()->RunInLoop([client]() {client->Disconnect(); });
    tcp_client_thread->loop()->RunAfter(eventpp::Duration(1.0), [client]() {delete client; });
    usleep(eventpp::Duration(2.0).Microseconds());
    client = nullptr;
    tcp_client_thread->Stop(true);
    tcp_server_thread->Stop(true);
    REQUIRE(tcp_client_thread->IsStopped());
    REQUIRE(tcp_server_thread->IsStopped());
    REQUIRE(connected_count == test_count);
    REQUIRE(message_recved_count == test_count);
    tcp_client_thread.reset();
    tcp_server_thread.reset();
    tsrv.reset();

    REQUIRE(eventpp::GetActiveEventCount() == 0);
}

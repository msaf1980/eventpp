#include <eventpp/buffer.hpp>
#include <eventpp/event_loop.hpp>
#include <eventpp/event_loop_thread.hpp>
#include <eventpp/event_watcher.hpp>
#include <eventpp/tcp/tcp_client.hpp>
#include <eventpp/tcp/tcp_conn.hpp>
#include <eventpp/tcp/tcp_server.hpp>

#include "test.h"

TEST_CASE("testTCPClientConnectFailed")
{
    std::shared_ptr<eventpp::EventLoop> loop(new eventpp::EventLoop);
    std::shared_ptr<eventpp::TCPClient> client(new eventpp::TCPClient(loop.get(), "127.0.0.1:39723", "TCPPingPongClient"));
    client->SetConnectionCallback([&loop, &client](const eventpp::TCPConnPtr & conn) {
        REQUIRE(!conn->IsConnected());
        client->Disconnect();
        loop->Stop();
    });
    client->set_auto_reconnect(false);
    client->Connect();
    loop->Run();
    client.reset();
    loop.reset();
    REQUIRE(eventpp::GetActiveEventCount() == 0);
}

TEST_CASE("testTCPClientDisconnectImmediately")
{
    std::shared_ptr<eventpp::EventLoop> loop(new eventpp::EventLoop);
    std::shared_ptr<eventpp::TCPClient> client(new eventpp::TCPClient(loop.get(), "cmake.org:80", "TCPPingPongClient"));
    client->SetConnectionCallback([loop, client](const eventpp::TCPConnPtr & conn) {
        REQUIRE(!conn->IsConnected());
        auto f = [loop]() { loop->Stop(); };
        loop->RunAfter(1.0, f);
    });
    client->set_auto_reconnect(false);
    client->Connect();
    client->Disconnect();
    loop->Run();
    client.reset();
    loop.reset();
    REQUIRE(eventpp::GetActiveEventCount() == 0);
}

TEST_CASE("testTCPClientConnectionTimeout")
{
    std::shared_ptr<eventpp::EventLoop> loop(new eventpp::EventLoop);
    std::shared_ptr<eventpp::TCPClient> client(new eventpp::TCPClient(loop.get(), "cmake.org:80", "TCPPingPongClient"));
    client->SetConnectionCallback([loop, client](const eventpp::TCPConnPtr & conn) { loop->Stop(); });
    client->set_auto_reconnect(false);
    client->set_connecting_timeout(eventpp::Duration(0.0001));
    client->Connect();
    loop->Run();
    client.reset();
    loop.reset();
    REQUIRE(eventpp::GetActiveEventCount() == 0);
}

struct NSQConn
{
    NSQConn(eventpp::EventLoop * loop) : loop_(loop)
    {
        client_ = std::make_shared<eventpp::TCPClient>(loop, "www.so.com:80", "TCPPingPongClient");
        client_->SetConnectionCallback([this](const eventpp::TCPConnPtr & conn) {
            REQUIRE(conn->IsConnected());
            REQUIRE(this->loop_->IsRunning());
            this->connected_ = true;
            client_->SetConnectionCallback(eventpp::ConnectionCallback());
        });
        client_->Connect();
    }

    void Disconnect()
    {
        if (!connected_)
        {
            loop_->RunAfter(100.0, [this]() { this->Disconnect(); });
            return;
        }

        // We call TCPClient::Disconnect and then delete the hold object of TCPClient immediately
        client_->Disconnect();
        client_.reset();
        connected_ = false;
        loop_->RunAfter(100.0, [this]() { this->loop_->Stop(); });
    }

    std::shared_ptr<eventpp::TCPClient> client_;
    bool connected_ = false;
    eventpp::EventLoop * loop_;
};

TEST_CASE("testTCPClientDisconnectAndDestruct")
{
    std::shared_ptr<eventpp::EventLoop> loop(new eventpp::EventLoop);
    NSQConn nc(loop.get());
    loop->RunAfter(100.0, [&nc]() { nc.Disconnect(); });
    loop->Run();
    loop.reset();
    REQUIRE(eventpp::GetActiveEventCount() == 0);
}

TEST_CASE("testTCPClientConnectLocalhost")
{
    eventpp::EventLoop loop;
    eventpp::TCPClient client(&loop, "localhost:39099", "TestClient");
    client.SetConnectionCallback([&loop, &client](const eventpp::TCPConnPtr & conn) {
        REQUIRE(!conn->IsConnected());
        client.Disconnect();
        loop.Stop();
    });
    client.SetMessageCallback([](const eventpp::TCPConnPtr & conn, eventpp::Buffer * buf) {});
    client.Connect();
    loop.Run();
    REQUIRE(eventpp::GetActiveEventCount() == 0);
}


TEST_CASE("TestTCPClientDisconnectImmediatelyIssue172")
{
    const std::string strAddr = "qup.f.360.cn:80";
    eventpp::EventLoop loop;
    eventpp::TCPClient client(&loop, strAddr, "TestClient");
    client.SetConnectionCallback([&loop, &client](const eventpp::TCPConnPtr & conn) {
        if (conn->IsConnected())
        {
            auto f = [&]() {
                client.Disconnect();
                loop.Stop();
            };
            loop.RunAfter(eventpp::Duration(1.0), f);
        }
    });
    client.SetMessageCallback([](const eventpp::TCPConnPtr & conn, eventpp::Buffer * buf) { std::string strMsg = buf->NextAllString(); });
    client.Connect();
    loop.Run();
}

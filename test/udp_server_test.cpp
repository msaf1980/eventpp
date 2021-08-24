#include <vector>

#include <eventpp/udp/sync_udp_client.hpp>
#include <eventpp/udp/udp_server.hpp>

#include "test.h"

static int g_count = 0;
static bool g_exit = false;
static uint64_t g_timeout_ms = 1000;
static void OnMessage(eventpp::udp::Server * udpsrv, eventpp::EventLoop * loop, const eventpp::udp::MessagePtr msg)
{
    g_count++;
    eventpp::udp::SendMessage(msg);
    usleep(100);
    if (memcmp(msg->data(), "stop", msg->size()) == 0)
    {
        g_exit = true;
    }
}

static void Init()
{
    g_count = 0;
    g_exit = false;
}

TEST_CASE("testUDPServer")
{
    // LOG_TRACE << __func__;
    Init();
    std::vector<int> ports(2, 0);
    ports[0] = 15368;
    ports[1] = 15369;
    eventpp::udp::Server * udpsrv = new eventpp::udp::Server;
    udpsrv->SetMessageHandler(std::bind(&OnMessage, udpsrv, std::placeholders::_1, std::placeholders::_2));
    REQUIRE(udpsrv->Init(ports));
    REQUIRE(udpsrv->Start());
    usleep(100); //wait udpsrv started
    // LOG_TRACE << "udpserver started.";

    int loop = 10;
    for (int i = 0; i < loop; ++i)
    {
        std::string req = "data xxx";
        std::string resp = eventpp::udp::sync::Client::DoRequest("127.0.0.1", ports[0], req, g_timeout_ms);
        REQUIRE(req == resp);
        resp = eventpp::udp::sync::Client::DoRequest("127.0.0.1", ports[1], req, g_timeout_ms);
        REQUIRE(req == resp);
    }

    REQUIRE(g_count == 2 * loop);
    eventpp::udp::sync::Client::DoRequest("127.0.0.1", ports[0], "stop", g_timeout_ms);
    REQUIRE(g_count == 2 * loop + 1);
    eventpp::udp::sync::Client::DoRequest("127.0.0.1", ports[1], "stop", g_timeout_ms);
    REQUIRE(g_count == 2 * loop + 2);

    while (!g_exit)
    {
        usleep(1);
    }

    REQUIRE(g_exit);
    udpsrv->Stop(true);
    REQUIRE(udpsrv->IsStopped());
    delete udpsrv;
}


TEST_CASE("testUDPServerGraceStop")
{
    Init();
    int port = 53669;
    eventpp::udp::Server * udpsrv = new eventpp::udp::Server;
    udpsrv->SetMessageHandler(std::bind(&OnMessage, udpsrv, std::placeholders::_1, std::placeholders::_2));
    REQUIRE(udpsrv->Init(port));
    REQUIRE(udpsrv->Start());
    usleep(100); //wait udpsrv started

    int loop = 10;
    for (int i = 0; i < loop; ++i)
    {
        std::string req = "data xxx";
        std::string resp = eventpp::udp::sync::Client::DoRequest("127.0.0.1", port, req, g_timeout_ms);
        REQUIRE(req == resp);
    }

    REQUIRE(g_count == loop);
    eventpp::udp::sync::Client::DoRequest("127.0.0.1", port, "stop", g_timeout_ms);
    REQUIRE(g_count == loop + 1);

    while (!g_exit)
    {
        usleep(1);
    }

    REQUIRE(g_exit);
    udpsrv->Stop(true);
    REQUIRE(udpsrv->IsStopped());
    delete udpsrv;
}

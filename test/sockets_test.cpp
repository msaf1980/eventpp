#include <eventpp/sockets.hpp>

#include <string>
#include <vector>

#include "test.h"

TEST_CASE("TestParseFromIPPort1")
{
    std::string dd[] = {
        "192.168.0.6:99",
        "101.205.216.65:60931",
        "127.0.0.1:19099",
    };

    for (size_t i = 0; i < H_ARRAYSIZE(dd); i++)
    {
        CAPTURE(i);
        struct sockaddr_storage ss;
        auto rc = eventpp::sock::ParseFromIPPort(dd[i].data(), ss);
        REQUIRE(rc);
        auto s = eventpp::sock::ToIPPort(&ss);
        rc = s == dd[i];
        REQUIRE(rc);
    }
}

TEST_CASE("TestParseFromIPPort2")
{
    std::vector<std::string> dd{
        "5353.168.0.6",
        "5353.168.0.6:",
        "5353.168.0.6:99",
        "1011.205.216.65:60931",
    };

    for (size_t i = 0; i < dd.size(); i++)
    {
        CAPTURE(i);
        struct sockaddr_storage ss;
        auto rc = eventpp::sock::ParseFromIPPort(dd[i].data(), ss);
        REQUIRE(!rc);
        rc = eventpp::sock::IsZeroAddress(&ss);
        REQUIRE(rc);
    }
}

// TODO IPv6 test failed
#if 0
TEST_CASE(TestParseFromIPPort3) {
    std::string dd[] = {
        "192.168.0.6:99",
        "101.205.216.65:60931",
        "[fe80::886a:49f3:20f3:add2:0]:80",
        "[fe80::c455:9298:85d2:f2b6:0]:8080",
    };

    for (size_t i = 0; i < H_ARRAYSIZE(dd); i++) {
        struct sockaddr_storage ss;
        auto rc = eventpp::sock::ParseFromIPPort(dd[i].data(), ss);
        REQUIRE(rc);
        auto s = eventpp::sock::ToIPPort(&ss);
        rc = s == dd[i];
        REQUIRE(rc);
    }
}

TEST_CASE(TestParseFromIPPort4) {
    std::string dd[] = {
        "5353.168.0.6",
        "5353.168.0.6:",
        "5353.168.0.6:99",
        "1011.205.216.65:60931",
        "[fe80::886a:49f3:20f3:add2]",
        "[fe80::886a:49f3:20f3:add2]:",
        "fe80::886a:49f3:20f3:add2]:80",
        "[fe80::c455:9298:85d2:f2b6:8080",
    };

    for (size_t i = 0; i < H_ARRAYSIZE(dd); i++) {
        struct sockaddr_storage ss;
        auto rc = eventpp::sock::ParseFromIPPort(dd[i].data(), ss);
        REQUIRE(!rc);
        rc = eventpp::sock::IsZeroAddress(&ss);
        REQUIRE(rc);
    }
}
#endif

TEST_CASE("TestSplitHostPort1")
{
    struct
    {
        std::string addr;
        std::string host;
        int port;
    } dd[] = {
        {"192.168.0.6:99", "192.168.0.6", 99},
        {"101.205.216.65:60931", "101.205.216.65", 60931},
        {"[fe80::886a:49f3:20f3:add2]:80", "fe80::886a:49f3:20f3:add2", 80},
        {"[fe80::c455:9298:85d2:f2b6]:8080", "fe80::c455:9298:85d2:f2b6", 8080},
        {"fe80::886a:49f3:20f3:add2]:80", "fe80::886a:49f3:20f3:add2", 80}, // This is OK
    };


    for (size_t i = 0; i < H_ARRAYSIZE(dd); i++)
    {
        std::string host;
        int port;
        auto rc = eventpp::sock::SplitHostPort(dd[i].addr.data(), host, port);
        REQUIRE(rc);
        REQUIRE(dd[i].host == host);
        REQUIRE(dd[i].port == port);
    }
}

TEST_CASE("TestSplitHostPort2")
{
    struct
    {
        std::string addr;
        std::string host;
        int port;
    } dd[] = {
        {"[fe80::c455:9298:85d2:f2b6:8080", "fe80::c455:9298:85d2:f2b6", 8080} // This is not OK
    };

    for (size_t i = 0; i < H_ARRAYSIZE(dd); i++)
    {
        std::string host;
        int port;
        auto rc = eventpp::sock::SplitHostPort(dd[i].addr.data(), host, port);
        REQUIRE(!rc);
    }
}

#include <eventpp/duration.hpp>
// #include <eventpp/gettimeofday.h>
#include <eventpp/timestamp.hpp>

#include "test.h"

TEST_CASE("testDuration")
{
    eventpp::Duration d0(0);
    eventpp::Duration d1(1);
    eventpp::Duration d2(2);
    eventpp::Duration d3(2);
    REQUIRE(d0 < d1);
    REQUIRE(d1 < d2);
    REQUIRE(d2 == d3);
    REQUIRE(d0.IsZero());
    REQUIRE(d0 <= d1);
    REQUIRE(d1 <= d2);
    REQUIRE(d2 <= d3);
    REQUIRE(d2 >= d3);
    REQUIRE(d1 > d0);
    REQUIRE(d2 > d1);
    REQUIRE(d1 >= d0);
    REQUIRE(d2 >= d1);
}

TEST_CASE("testTimestamp")
{
    int64_t c_s = time(nullptr);
    int64_t c_us = eventpp::utcmicrosecond();
    int64_t ts_ns = eventpp::Timestamp::Now().UnixNano();
    int64_t c11_us = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    REQUIRE(c_us / 1000000 == c11_us / 1000000);
    REQUIRE(c_s == c11_us / 1000000);
    REQUIRE(c_s == ts_ns / eventpp::duration::kSecond);
}

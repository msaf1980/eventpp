
#include <eventpp/http/context.hpp>
#include <eventpp/httpc/url_parser.hpp>

#include "test.h"

TEST_CASE("testURLParser")
{
    struct TestCase
    {
        std::string url;

        std::string protocol;
        std::string host;
        int port;
        std::string path;
        std::string query;
    };

    TestCase cases[] = {
        {"http://www.so.com/query?a=1", "http", "www.so.com", 80, "/query", "a=1"},
        {"http://www.so.com/", "http", "www.so.com", 80, "/", ""},
        {"http://www.so.com", "http", "www.so.com", 80, "", ""},
    };

    for (size_t i = 0; i < H_ARRAYSIZE(cases); i++)
    {
        eventpp::httpc::URLParser p(cases[i].url);
        REQUIRE(p.schema == cases[i].protocol);
        REQUIRE(p.host == cases[i].host);
        REQUIRE(p.port == cases[i].port);
        REQUIRE(p.path == cases[i].path);
        REQUIRE(p.query == cases[i].query);
    }
}

TEST_CASE("TestFindQueryFromURI")
{
    struct TestCase
    {
        std::string uri;
        std::string key;
        std::string value;
    };

    TestCase cases[] = {
        {"/query?aabc=1312&abc=3&ab=1&ccc=3", "ab", "1"},
        {"/query", "a", ""},
        {"/query?", "abc", ""},
        {"/query?a=1&bc=123", "abc", ""},
        {"/query?a=123", "abc", ""},
        {"/query?a=1", "a", "1"},
        {"/query?a=1&b=2&c=3", "a", "1"},
        {"/query?d=4&a=1&b=2&c=3", "a", "1"},
        {"/query?d=4&b=2&c=3&a=1123123", "a", "1123123"},
        {"/query?ab=1", "ab", "1"},
        {"/query?ab=1&b=2&c=3", "ab", "1"},
        {"/query?d=4&ab=1&b=2&c=3", "ab", "1"},
        {"/query?d=4&b=2&c=3&ab=1123123", "ab", "1123123"},
        {"/query?thekey=1", "thekey", "1"},
        {"/query?thekey=1&b=2&c=3", "thekey", "1"},
        {"/query?d=4&thekey=1&b=2&c=3", "thekey", "1"},
        {"/query?d=4&b=2&c=3&thekey=1123123", "thekey", "1123123"},
    };

    for (size_t i = 0; i < H_ARRAYSIZE(cases); i++)
    {
        std::string uri = cases[i].uri;
        std::string k = cases[i].key;
        std::string expected_value = cases[i].value;
        std::string v = eventpp::http::Context::FindQueryFromURI(uri, k);
        REQUIRE(v == expected_value);
    }
}

TEST_CASE("TestFindClientIPFromURI")
{
    struct TestCase
    {
        std::string uri;
        std::string ip;
    } cases[] = {
        {"/abc?clientip=", ""},
        {"/abc?clientip=123.1.1.9", "123.1.1.9"},
        {"/abc?clientip=123.1.1.9&a=b", "123.1.1.9"},
        {"/abc?c=d&clientip=123.1.1.9&a=b", "123.1.1.9"},
        {"/abc?c=d&xx=123.1.1.9&a=b", ""},
    };

    for (size_t i = 0; i < H_ARRAYSIZE(cases); i++)
    {
        std::string uri = cases[i].uri;
        std::string ip = eventpp::http::Context::FindClientIPFromURI(uri.data(), uri.size());
        REQUIRE(ip == cases[i].ip);
    }
}

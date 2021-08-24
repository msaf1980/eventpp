#pragma once

#include <iostream>
#include <string>
#include <algorithm>
#include <cctype>
#include <functional>
#include <iterator>

#include <eventpp/inner_pre.hpp>

namespace eventpp {
namespace httpc {
struct EVPP_EXPORT URLParser {
public:
    std::string schema;
    std::string host;
    int port;
    std::string path;
    std::string query;

    URLParser(const std::string& url);

private:
    int parse(const std::string& url);
};
} // httpc
} // evpp
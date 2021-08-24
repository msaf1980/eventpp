#ifndef __EVENT_RESPONSE_HPP__
#define __EVENT_RESPONSE_HPP__

#include <map>

#include "eventpp/base.hpp"
#include "eventpp/event_loop.hpp"
#include "eventpp/slice.hpp"

struct evhttp_request;
namespace eventpp {
namespace httpc {
class Request;
class EVPP_EXPORT Response {
public:
    typedef std::map<eventpp::Slice, eventpp::Slice> Headers;
    Response(Request* r, struct evhttp_request* evreq, bool had_ssl_error = false);

    ~Response();

    int http_code() const {
        return http_code_;
    }

    bool had_ssl_error() const {
        return had_ssl_error_;
    }

    const eventpp::Slice& body() const {
        return body_;
    }
    const Request* request() const {
        return request_;
    }
    const char* FindHeader(const char* key);
private:
    Request* request_;
    struct evhttp_request* evreq_;
    int http_code_;
    bool had_ssl_error_;
    eventpp::Slice body_;
};
}
}

#endif // __EVENT_RESPONSE_HPP__

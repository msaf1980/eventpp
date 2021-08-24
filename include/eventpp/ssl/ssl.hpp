#ifndef __EVENTPP_SSL_HPP__
#define __EVENTPP_SSL_HPP__

#ifdef EVENTPP_HTTP_CLIENT_SUPPORTS_SSL

#if HAVE_OPENSSL
#include "openssl.hpp"
#endif

#else

namespace eventpp {
bool InitSSL();
void CleanSSL();
#define SSL_CTX void
SSL_CTX* GetSSLCtx();
} // eventpp

#endif

#endif /* __EVENTPP_SSL_HPP__ */

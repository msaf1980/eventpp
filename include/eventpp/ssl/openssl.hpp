#ifndef __EVENTPP_OPENSSL_HPP__
#define __EVENTPP_OPENSSL_HPP__

#include <openssl/ssl.h>
#include <openssl/err.h>

namespace eventpp {
bool InitSSL();
void CleanSSL();
SSL_CTX* GetSSLCtx();
} // eventpp


#endif /* __EVENTPP_OPENSSL_HPP__ */

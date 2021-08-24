#include "../pch.h"

#include <eventpp/ssl/ssl.hpp>

#if defined(EVENTPP_HTTP_CLIENT_SUPPORTS_SSL)

#    if HAVE_OPENSSL
#        include "openssl.cpp"
#    endif

#else

namespace eventpp
{
bool InitSSL()
{
    return false;
}
void CleanSSL()
{
}

SSL_CTX * GetSSLCtx()
{
    return nullptr;
}

} // eventpp


#endif

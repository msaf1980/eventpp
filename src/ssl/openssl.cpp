#include <openssl/err.h>
#include <openssl/rand.h>

namespace eventpp
{
static SSL_CTX * g_ssl_ctx = nullptr;

bool InitSSL()
{
    SSL_library_init();
    ERR_load_crypto_strings();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
    int r = RAND_poll();
    if (r == 0)
    {
        LOG_ERROR << "RAND_poll failed";
        return false;
    }
    g_ssl_ctx = SSL_CTX_new(SSLv23_method());
    if (!g_ssl_ctx)
    {
        LOG_ERROR << "SSL_CTX_new failed";
        return false;
    }
    X509_STORE * store = SSL_CTX_get_cert_store(g_ssl_ctx);
    if (X509_STORE_set_default_paths(store) != 1)
    {
        LOG_ERROR << "X509_STORE_set_default_paths failed";
        return false;
    }
    return true;
}

void CleanSSL()
{
    if (g_ssl_ctx != nullptr)
    {
        SSL_CTX_free(g_ssl_ctx);
    }
    ERR_free_strings();
    EVP_cleanup();
    ERR_remove_thread_state(nullptr);
    CRYPTO_cleanup_all_ex_data();
}

SSL_CTX * GetSSLCtx()
{
    return g_ssl_ctx;
}
} // evpp

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

} // evpp

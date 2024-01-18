#pragma once

#include <memory>
#include <openssl/err.h>
#include <openssl/dh.h>
#include <openssl/ssl.h>
#include <openssl/conf.h>
#include <openssl/engine.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/x509.h>
#include <openssl/x509_vfy.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#define OPENSSL_NO_EC2M
#include <openssl/ec.h>

#define DEF_ANOTHER_SSL_AUTO_PTR(POINTED_TYPE) \
    template<> \
    struct ssl_deleter<POINTED_TYPE> { \
        void operator()(POINTED_TYPE *p) const { \
            if(p) { \
                POINTED_TYPE ## _free(p); \
            } \
        } \
    }

#define DEF_ANOTHER_SSL_CF_AUTO_PTR(POINTED_TYPE, FUNC_NAME) \
    template<> \
    struct ssl_deleter<POINTED_TYPE> { \
        void operator()(POINTED_TYPE *p) const { \
            if(p) { \
                FUNC_NAME(p); \
            } \
        } \
    }

namespace lins {

    namespace ssl {

        template<typename T>
        struct ssl_deleter;

        DEF_ANOTHER_SSL_AUTO_PTR(EVP_CIPHER_CTX);
        DEF_ANOTHER_SSL_AUTO_PTR(SSL_CTX);
        DEF_ANOTHER_SSL_AUTO_PTR(SSL);
        DEF_ANOTHER_SSL_CF_AUTO_PTR(BIO, BIO_free_all);
        DEF_ANOTHER_SSL_CF_AUTO_PTR(BIO_METHOD, BIO_meth_free);

        template<> struct ssl_deleter<BIGNUM> {
            void operator()(BIGNUM *p) const {
                if(p) {
                    ::BN_clear(p);
                    ::BN_free(p);
                }
            }
        };

        struct ssl_deleter_bio {
            void operator()(BIO *p) const {
                if(p) {
                    ::BIO_free(p);
                }
            }
        };

        struct ssl_deleter_bio_all {
            void operator()(BIO *p) const {
                if(p) {
                    ::BIO_free_all(p);
                }
            }
        };

        DEF_ANOTHER_SSL_AUTO_PTR(RSA);

        DEF_ANOTHER_SSL_AUTO_PTR(X509);
        DEF_ANOTHER_SSL_AUTO_PTR(X509_STORE_CTX);
        DEF_ANOTHER_SSL_AUTO_PTR(X509_STORE);

        DEF_ANOTHER_SSL_AUTO_PTR(EVP_PKEY);
        DEF_ANOTHER_SSL_AUTO_PTR(EVP_PKEY_CTX);

        DEF_ANOTHER_SSL_AUTO_PTR(EC_KEY);
        DEF_ANOTHER_SSL_AUTO_PTR(ECDSA_SIG);
        DEF_ANOTHER_SSL_AUTO_PTR(EC_GROUP);
        DEF_ANOTHER_SSL_CF_AUTO_PTR(EC_POINT, EC_POINT_clear_free);
        DEF_ANOTHER_SSL_AUTO_PTR(BN_CTX);
        DEF_ANOTHER_SSL_AUTO_PTR(EVP_MD_CTX);

        template<class T>
        using unique_ptr = std::unique_ptr<T, ssl_deleter<T>>;

        using unique_ptr_bio = std::unique_ptr<BIO, ssl_deleter_bio>;

        using unique_ptr_bio_all = std::unique_ptr<BIO, ssl_deleter_bio_all>;

        template<typename T>
        unique_ptr<T> NEW(std::function<T *()> &&f) {
            return unique_ptr<T>{f()};
        }

        static unique_ptr_bio NEW_BIO(std::function<BIO *()> &&f) {
            return unique_ptr_bio{f()};
        }

        static unique_ptr_bio_all NEW_BIO_ALL(std::function<BIO *()> &&f) {
            return unique_ptr_bio_all{f()};
        }

    }
}

template<typename T>
using ssl_ptr = lins::ssl::unique_ptr<T>;
using ssl_ptr_bio = lins::ssl::unique_ptr_bio;
using ssl_ptr_bio_all = lins::ssl::unique_ptr_bio_all;

#undef DEF_ANOTHER_SSL_AUTO_PTR
#undef DEF_ANOTHER_SSL_CF_AUTO_PTR

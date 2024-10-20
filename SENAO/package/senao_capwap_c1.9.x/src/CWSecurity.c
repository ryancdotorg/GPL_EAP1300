/*******************************************************************************************
 * Copyright (c) 2006-7 Laboratorio di Sistemi di Elaborazione e Bioingegneria Informatica *
 *                      Universita' Campus BioMedico - Italy                               *
 *                                                                                         *
 * This program is free software; you can redistribute it and/or modify it under the terms *
 * of the GNU General Public License as published by the Free Software Foundation; either  *
 * version 2 of the License, or (at your option) any later version.                        *
 *                                                                                         *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY         *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A 	   *
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.                *
 *                                                                                         *
 * You should have received a copy of the GNU General Public License along with this       *
 * program; if not, write to the:                                                          *
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,                    *
 * MA  02111-1307, USA.                                                                    *
 *                                                                                         *
 * --------------------------------------------------------------------------------------- *
 * Project:  Capwap                                                                        *
 *                                                                                         *
 * Author :  Ludovico Rossi (ludo@bluepixysw.com)                                          *
 *           Del Moro Andrea (andrea_delmoro@libero.it)                                    *
 *           Giovannini Federica (giovannini.federica@gmail.com)                           *
 *           Massimo Vellucci (m.vellucci@unicampus.it)                                    *
 *           Mauro Bisson (mauro.bis@gmail.com)                                            *
 *******************************************************************************************/

#include "CWCommon.h"
#include <openssl/pkcs12.h>
#include <openssl/x509v3.h>
#include <openssl/rand.h>

#ifdef DMALLOC
#include "../dmalloc-5.5.0/dmalloc.h"
#endif

#define	CW_DTLS_CERT_VERIFY_DEPTH	1
#define CW_COOKIE_SECRET_LENGTH    16

#if (OPENSSL_VERSION_NUMBER < 0x000908000)
#error "Must use CAPWAP Hacked OpenSSL 0.9.8a or later"
#endif

static char *gSecurityPassword;
static CWBool useCertificate;
static CWBool gIsClient;
static CWThreadMutex *mutexOpensslBuf = NULL;
static unsigned char cookieSecret[CW_COOKIE_SECRET_LENGTH];

CWBool CWSecurityVerifyPeerCertificateForCAPWAP(SSL *ssl, CWBool isClient);
static int CWDTLSPasswordCB(char *buf, int num, int rwflag, void *userdata);
int CWSecurityVerifyCB(int ok, X509_STORE_CTX *ctx);
/*Sigma added*/
#define CW_CERT_PATH_LENGTH     PATH_MAX
CWBool CWisDefaultCert(const char *defaultCertFile, X509 *currentCert);
CWBool CWVerifyIsIssuedByCA(X509 *cert, const char *CAfile);
CWBool CWIsCertExpired(const char *fileCert, const char *fileCA);
char gCWCertPath[CW_CERT_PATH_LENGTH];

unsigned int CWSecurityPSKClientCB(SSL *ssl,
                                   const char *hint,
                                   char *identity,
                                   unsigned int max_identity_len,
                                   unsigned char *psk,
                                   unsigned int max_psk_len);

unsigned int CWSecurityPSKServerCB(SSL *ssl,
                                   const char *identity,
                                   unsigned char *psk,
                                   unsigned int max_psk_len);

int psk_key2bn(const char *psk_key, unsigned char *psk, unsigned int max_psk_len);

#define CWSecurityGetErrorStr()				((const char *) ERR_error_string(ERR_get_error(), NULL))
#define CWDTLSGetError()				"Err"

#define CWSecurityRaiseError(error)			{						\
								char buf[256];				\
								ERR_error_string(ERR_get_error(), buf);	\
								CWErrorRaise(error, buf);		\
								return CW_FALSE;			\
							}

#define CWSecurityRaiseSystemError(error)		{						\
								char buf[256];				\
								strerror_r(errno, buf, 256);		\
								CWErrorRaise(error, buf);		\
								return CW_FALSE;			\
							}

#define CWSecurityManageSSLError(arg, session, stuff)	{						\
								char ___buf[256];			\
								int r;					\
								if((r=(arg)) <= 0) {			\
									{stuff}				\
									ERR_error_string_n(ERR_get_error(), ___buf, 256);	\
									CWLog(ERR_reason_error_string(ERR_get_error()));		\
									return CWErrorRaise(CW_ERROR_GENERAL, ___buf);		\
								}					\
							}

static void CWSslLockingFunc(int mode, int n, const char *file, int line)
{
    if(mode & CRYPTO_LOCK)
    {
        CWThreadMutexLock(&mutexOpensslBuf[n]);
    }
    else
    {
        CWThreadMutexUnlock(&mutexOpensslBuf[n]);
    }

    return;
}

static unsigned long CWSslIdFunction()
{
    //CWLog("CWSslIdFunction call");
    return (unsigned long) CWThreadSelf();
}

static int CWCookieGenerateCB(SSL *ssl, unsigned char
                              *cookie, unsigned int *cookieLen)
{
    unsigned char *buffer, result[EVP_MAX_MD_SIZE];
    unsigned int length, resultLength;
    CWNetworkLev4Address peer;

    /* Read peer information */
    BIO_ctrl(SSL_get_rbio(ssl), BIO_CTRL_DGRAM_GET_PEER, 0, (char *)&peer);

    /* Create buffer with peer's address and port */
    length = sizeof(peer);
    CW_CREATE_OBJECT_SIZE_ERR(buffer, length,
    {
        CWLog("out of memory\n");
        return 0;
    });

    CW_COPY_MEMORY(buffer, &peer, sizeof(peer));

    /* Calculate HMAC of buffer using the secret */
    HMAC(EVP_sha1(), &cookieSecret, CW_COOKIE_SECRET_LENGTH, buffer,
         length, (unsigned char *) &result, &resultLength);
    CW_FREE_OBJECT(buffer);

    CW_COPY_MEMORY(cookie, result, resultLength);
    *cookieLen = resultLength;
#if 0
    CWLog("CWGenerateCookieCB called cookieLen=%u cookie=", *cookieLen);
    int i;
    for(i = 0; i < *cookieLen; i++)
    {
        printf("%x ", cookie[i]);
    }
    printf("\n");
#endif
    return 1;
}

static int CWCookieVerifyCB(SSL *ssl, unsigned char
                            *cookie, unsigned int cookieLen)
{
    unsigned char *buffer, result[EVP_MAX_MD_SIZE];
    unsigned int length, resultLength;
    CWNetworkLev4Address peer;
#if 0
    CWLog("CWVerifyCookieCB called cookie_len=%u\n", cookieLen);
    int i;
    for(i = 0; i < cookieLen; i++)
    {
        printf("%x ", cookie[i]);
    }
    printf("\n");
#endif
    /* Read peer information */
    BIO_ctrl(SSL_get_rbio(ssl), BIO_CTRL_DGRAM_GET_PEER, 0, (char *)&peer);

    /* Create buffer with peer's address and port */
    length = sizeof(peer);
    CW_CREATE_OBJECT_SIZE_ERR(buffer, length,
    {
        CWLog("out of memory\n");
        return 0;
    });

    CW_COPY_MEMORY(buffer, &peer, sizeof(peer));

    /* Calculate HMAC of buffer using the secret */
    HMAC(EVP_sha1(), &cookieSecret, CW_COOKIE_SECRET_LENGTH, buffer,
         length, (unsigned char *) &result, &resultLength);
    CW_FREE_OBJECT(buffer);

    if(cookieLen == resultLength && memcmp(&result, cookie,
                                           resultLength) == 0)
    {
        return 1;
    }

    CWLog("Hello Cookie Verification failed\n");
    return 0;
}

void CWSslCleanUp()
{
    int i;

    if(mutexOpensslBuf == NULL)
    {
        return;
    }

    for(i = 0; i < CRYPTO_num_locks(); i++)
    {
        CWDestroyThreadMutex(&mutexOpensslBuf[i]);
    }

    CW_FREE_OBJECT(mutexOpensslBuf);
    mutexOpensslBuf = NULL;

    return;
}

CWBool CWSecurityInitLib()
{
    int i;

    SSL_load_error_strings();
    SSL_library_init();

    /* setup mutexes for openssl internal locking */
    CW_CREATE_ARRAY_ERR(mutexOpensslBuf,
                        CRYPTO_num_locks(),
                        CWThreadMutex,
                        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY,
                                            "Cannot create openssl mutexes"););

    CW_ZERO_MEMORY(mutexOpensslBuf, CRYPTO_num_locks() * sizeof(CWThreadMutex));

    for(i = 0; i < CRYPTO_num_locks(); i++)
    {
        if(CWCreateThreadMutex(&mutexOpensslBuf[i]) != CW_TRUE)
        {
            CWSslCleanUp();
            return CWErrorRaise(CW_ERROR_CREATING,
                                "Cannot create openssl mutexes");
        }
    }

    CRYPTO_set_id_callback(CWSslIdFunction);
    CRYPTO_set_locking_callback(CWSslLockingFunc);

    return CW_TRUE;
}

CWBool CWSecurityInitSessionClient(CWSocket sock,
                                   CWNetworkLev4Address *addrPtr,
                                   CWSecurityContext ctx,
                                   CWSecuritySession *sessionPtr,
                                   int *PMTUPtr,
                                   CWSecurityRxCB cbRx,
                                   void *cbRxArg,
                                   int controllerId)
{
    BIO *sbio = NULL;
    CWNetworkLev4Address peer;
    int peerlen = sizeof(peer);

    if(ctx == NULL || sessionPtr == NULL || PMTUPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    if((*sessionPtr = SSL_new(ctx)) == NULL)
    {
        CWSecurityRaiseError(CW_ERROR_CREATING);
    }

#ifdef CW_DEBUGGING
    CWDebugLog("My Certificate");
    if(gEnabledDebugLog)
    {
        PEM_write_X509(stdout, SSL_get_certificate(*sessionPtr));
    }
#endif

    if((sbio = BIO_new_memory(sock, addrPtr, cbRx, cbRxArg, controllerId)) == NULL)
    {
        SSL_free(*sessionPtr);
        *sessionPtr = NULL;
        CWSecurityRaiseError(CW_ERROR_CREATING);
    }

    if(getsockname(sock, (struct sockaddr *)&peer, (void *)&peerlen) < 0)
    {
        SSL_free(*sessionPtr);
        *sessionPtr = NULL;
        CWSecurityRaiseSystemError(CW_ERROR_GENERAL);
    }

#if (OPENSSL_VERSION_NUMBER >= 0x10100000L)
	if(BIO_ctrl_set_connected(sbio, &peer)) ; //do nothing if statement because of compile error
#else
	if(BIO_ctrl_set_connected(sbio, 1, &peer)) ; //do nothing if statement because of compile error
#endif


    /* BIO_ctrl(sbio, BIO_CTRL_DGRAM_MTU_DISCOVER, 0, NULL); // TO-DO (pass MTU?) */
    /*
     * TO-DO if we don't set a big MTU, the DTLS implementation will
     * not be able to use a big certificate
     */
    /* BIO_ctrl(sbio, BIO_CTRL_DGRAM_SET_MTU, 2000, NULL); */

    /* update PMTUPtr */
    *PMTUPtr = BIO_ctrl(sbio, BIO_CTRL_DGRAM_QUERY_MTU, 0, PMTUPtr);
    CWDebugLog("PMTU: %d", *PMTUPtr);

    /*
     * Let the verify_callback catch the verify_depth error so that we get
     * an appropriate error in the logfile.
     */
    SSL_set_verify_depth((*sessionPtr), CW_DTLS_CERT_VERIFY_DEPTH + 1);

    /* required by DTLS implementation to avoid data loss */
    SSL_set_read_ahead((*sessionPtr), 1);
    SSL_set_bio((*sessionPtr), sbio, sbio);
    SSL_set_connect_state((*sessionPtr));

    CWDebugLog("Before HS");
    CWSecurityManageSSLError(SSL_do_handshake(*sessionPtr),
                             *sessionPtr,
                             SSL_free(*sessionPtr); *sessionPtr = NULL;);
    CWDebugLog("After HS");

    if(SSL_get_verify_result(*sessionPtr) == X509_V_OK)
    {
        CWDebugLog("Certificate Verified");
    }
    else
    {
        CWDebugLog("Certificate Error (%d)",
                   SSL_get_verify_result(*sessionPtr));
    }

    if(useCertificate)
    {
        if(CWSecurityVerifyPeerCertificateForCAPWAP((*sessionPtr), CW_TRUE))
        {
            CWDebugLog("Certificate Ok for CAPWAP");
        }
        else
        {
            CWDebugLog("Certificate Not Ok for CAPWAP");
#ifndef CW_DEBUGGING
            SSL_free(*sessionPtr);
            *sessionPtr = NULL;
            return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Certificate Not Ok for CAPWAP");
#endif
        }
    }
    return CW_TRUE;

}

void CWSecurityCloseSession(CWSecuritySession *sPtr)
{
    SSL_free(*sPtr);
}

CWBool CWSecurityReceive(CWSecuritySession session,
                         char *buf,
                         int len,
                         int *readBytesPtr)
{
    CWSecurityManageSSLError((*readBytesPtr = SSL_read(session, buf, len)), session,);

    CWDebugLog("Received Secured Packet");

    return CW_TRUE;
}

CWBool CWSecuritySend(CWSecuritySession session, const char *buf, int len)
{
    if(buf == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    CWSecurityManageSSLError(SSL_write(session, buf, len), session,);

    CWDebugLog("Secured Packet Sent");
    return CW_TRUE;
}

CWBool CWSecurityInitSessionServer(CWSocket sock,
                                   CWNetworkLev4Address *addrPtr,
                                   CWSecurityContext ctx,
                                   CWSecuritySession *sessionPtr,
                                   int *PMTUPtr,
                                   CWSecurityRxCB cbRx,
                                   void *cbRxArg)
{
    BIO *sbio = NULL;

    if(ctx == NULL || sessionPtr == NULL || PMTUPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    if((*sessionPtr = SSL_new(ctx)) == NULL)
    {
        CWSecurityRaiseError(CW_ERROR_CREATING);
    }

    if((sbio = BIO_new_memory(sock, addrPtr, cbRx, cbRxArg, 0)) == NULL)
    {
        SSL_free(*sessionPtr);
        *sessionPtr = NULL;
        CWSecurityRaiseError(CW_ERROR_CREATING);
    }

    /* BIO_ctrl(sbio, BIO_CTRL_DGRAM_MTU_DISCOVER, 0, NULL); // TO-DO (pass MTU?) */
    /*
     * TO-DO if we don't set a big MTU, the DTLS implementation will
     * not be able to use a big certificate
     */
    //BIO_ctrl(sbio, BIO_CTRL_DGRAM_SET_MTU, 2000, NULL);

    *PMTUPtr = BIO_ctrl(sbio, BIO_CTRL_DGRAM_QUERY_MTU, 0, PMTUPtr);
    CWDebugLog("PMTU: %d", *PMTUPtr);

    /*
     * Let the verify_callback catch the verify_depth error so that we get
     * an appropriate error in the logfile.
     */
    SSL_set_verify_depth((*sessionPtr), CW_DTLS_CERT_VERIFY_DEPTH + 1);
    /* required by DTLS implementation to avoid data loss */
    SSL_set_read_ahead((*sessionPtr), 1);
    /* turn on cookie exchange */
    SSL_set_options((*sessionPtr), SSL_OP_COOKIE_EXCHANGE);
    /* set the same bio for reading and writing */
    SSL_set_bio((*sessionPtr), sbio, sbio);
    /* tell OpenSSL we are a server */
    SSL_set_accept_state((*sessionPtr));

    CWDebugLog("Before HS");
    CWSecurityManageSSLError(SSL_do_handshake(*sessionPtr),
                             *sessionPtr,
                             SSL_free(*sessionPtr); *sessionPtr = NULL;);
    CWDebugLog("After HS");

    if(SSL_get_verify_result(*sessionPtr) == X509_V_OK)
    {
        CWDebugLog("Certificate Verified");
    }
    else
    {
        CWDebugLog("Certificate Error (%d)", SSL_get_verify_result(*sessionPtr));
    }

    if(useCertificate)
    {
        if(CWSecurityVerifyPeerCertificateForCAPWAP((*sessionPtr), CW_FALSE))
        {

            CWDebugLog("Certificate Ok for CAPWAP");
        }
        else
        {
            CWDebugLog("Certificate Not Ok for CAPWAP");
#ifndef CW_DEBUGGING
            SSL_free(*sessionPtr);
            *sessionPtr = NULL;
            return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Certificate Not Ok for CAPWAP");
#endif
        }
    }

    return CW_TRUE;
}

/*
 *  NULL caList means that we want pre-shared keys
 */
CWBool CWSecurityInitContext(CWSecurityContext *ctxPtr,
                             const char *caList,
                             const char *certfile,
                             const char *keyfile,
                             const char *passw,
                             CWBool isClient,
                             int (*hackPtr)(void *))
{
    char currentCAPath[PATH_MAX] = {0};
    char currentCertPath[PATH_MAX] = {0};
    char currentPKeyPath[PATH_MAX] = {0};

    if(ctxPtr == NULL || (caList != NULL && (keyfile == NULL || passw == NULL)))
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    if(!caList)
    {
        return CWErrorRaise(CW_ERROR_NOT_SUPPORTED, "OpenSSL PrivateSharedKey is not supported");
    }

    gIsClient = isClient;

    sprintf(currentCAPath, "%s%s", gCWCertPath, caList);
    sprintf(currentCertPath, "%s%s", gCWCertPath, certfile);
    sprintf(currentPKeyPath, "%s%s", gCWCertPath, keyfile);

    if(((*ctxPtr) = SSL_CTX_new((isClient) ? DTLSv1_client_method() : DTLSv1_server_method())) == NULL)
    {
        CWSecurityRaiseError(CW_ERROR_CREATING);
    }

    /* certificates */
    if(caList != NULL)
    {
        useCertificate = CW_TRUE;
        /* load keys and certificates */
        CWDebugLog("SSL_CTX_use_certificate_file: %s", currentCertPath);
        //		if(!(SSL_CTX_use_certificate_file((*ctxPtr), keyfile, SSL_FILETYPE_PEM))) {
        if(!(SSL_CTX_use_certificate_chain_file((*ctxPtr), currentCertPath)))
        {
            SSL_CTX_free((*ctxPtr));
            (*ctxPtr) = NULL;
            CWSecurityRaiseError(CW_ERROR_GENERAL);
        }

        /* store password */
        CWDebugLog("SSL_CTX_set_default_passwd_cb: %s", passw);
        gSecurityPassword = (char *)passw;
        SSL_CTX_set_default_passwd_cb((*ctxPtr), CWDTLSPasswordCB);

        CWDebugLog("SSL_CTX_use_PrivateKey_file: %s", currentPKeyPath);
        if(!(SSL_CTX_use_PrivateKey_file((*ctxPtr), currentPKeyPath, SSL_FILETYPE_PEM)))
        {
            SSL_CTX_free((*ctxPtr));
            (*ctxPtr) = NULL;
            CWSecurityRaiseError(CW_ERROR_GENERAL);
        }

        if(!SSL_CTX_check_private_key((*ctxPtr)))
        {
            SSL_CTX_free((*ctxPtr));
            (*ctxPtr) = NULL;
            CWSecurityRaiseError(CW_ERROR_GENERAL);
        }

        /* load the CAs we trust */
        CWDebugLog("SSL_CTX_load_verify_locations: %s", currentCAPath);
        if(!(SSL_CTX_load_verify_locations((*ctxPtr), currentCAPath, 0)))
        {
            SSL_CTX_free((*ctxPtr));
            (*ctxPtr) = NULL;
            CWSecurityRaiseError(CW_ERROR_GENERAL);
        }

        SSL_CTX_set_default_verify_paths((*ctxPtr));

        if(!isClient)
        {
            /* require client authentication */
            SSL_CTX_set_verify((*ctxPtr),
                               SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT,
                               CWSecurityVerifyCB);

            /* Initialize a random secret */
            if(!RAND_bytes((unsigned char *) &cookieSecret,
                           CW_COOKIE_SECRET_LENGTH))
            {
                SSL_CTX_free((*ctxPtr));
                (*ctxPtr) = NULL;
                CWSecurityRaiseError(CW_ERROR_GENERAL);
            }

            /* Set DTLS cookie generation and verification callbacks */
            SSL_CTX_set_cookie_generate_cb((*ctxPtr), CWCookieGenerateCB);
            SSL_CTX_set_cookie_verify_cb((*ctxPtr), CWCookieVerifyCB);

        }
        else
        {
            SSL_CTX_set_verify((*ctxPtr),
                               SSL_VERIFY_PEER,
                               CWSecurityVerifyCB);
        }

        /*
         * 1. (TLS_RSA_WITH_AES_128_CBC_SHA) CAPWAP says: MUST be supported
         * 2. (TLS_RSA_WITH_3DES_EDE_CBC_SHA) CAPWAP says: MUST be supported
         * 3. (TLS_DH_RSA_WITH_AES_128_CBC_SHA) CAPWAP says: SHOULD be supported
         * 4. Not implemented in OpenSSL (TLS_DH_RSA_WITH_3DES_EDE_CBC_SHA)
         *    CAPWAP says: SHOULD be supported
         */
        /* set the ciphers supported by CAPWAP */
        SSL_CTX_set_cipher_list((*ctxPtr),
                                "AES128-SHA:DES-CBC3-SHA:DH-RSA-AES128-SHA");
    }
    else
    {
        /* TODO pre-shared keys */
        /*
        useCertificate = CW_FALSE;
        SSL_CTX_set_cipher_list( (*ctxPtr), "TLSv1");	// current implementation of PSK for OpenSSL doesn't support CAPWAP's cipher.
        												// Better than nothing.

        if(isClient) {
        	CWDebugLog("Client PSK");
        	SSL_CTX_set_psk_client_callback( (*ctxPtr), CWSecurityPSKClientCB);
        } else {
        	CWDebugLog("Server PSK");
        	SSL_CTX_set_psk_server_callback( (*ctxPtr), CWSecurityPSKServerCB);
        }
        */
    }

    /* needed for DTLS */
    SSL_CTX_set_read_ahead((*ctxPtr), 1);

    return CW_TRUE;
}

void CWSecurityDestroyContext(CWSecurityContext ctx)
{
    if(ctx != NULL)
    {
        SSL_CTX_free(ctx);
    }
}

void CWSecurityDestroySession(CWSecuritySession s)
{
    if(s != NULL)
    {
        SSL_free(s);
    }

    ERR_remove_state(0); /* prevent memory leak */
}

CWBool CWSecurityVerifyCertEKU(X509 *x509, const char *const expected_oid)
{
    EXTENDED_KEY_USAGE *eku = NULL;
    CWBool fFound = CW_FALSE;

    if((eku = (EXTENDED_KEY_USAGE *)X509_get_ext_d2i(x509, NID_ext_key_usage, NULL, NULL)) == NULL)
    {
        CWDebugLog("Certificate does not have extended key usage extension");
    }
    else
    {
        int i;

        CWDebugLog("Validating certificate extended key usage");
        for(i = 0; !fFound && i < sk_ASN1_OBJECT_num(eku); i++)
        {
            ASN1_OBJECT *oid = sk_ASN1_OBJECT_value(eku, i);
            char szOid[1024];

            if(!fFound && OBJ_obj2txt(szOid, sizeof(szOid), oid, 0) != -1)
            {
                CWDebugLog("Certificate has EKU (str) %s, expects %s", szOid, expected_oid);
                if(!strcmp(expected_oid, szOid))
                {
                    fFound = CW_TRUE;
                }
            }
            if(!fFound && OBJ_obj2txt(szOid, sizeof(szOid), oid, 1) != -1)
            {
                CWDebugLog("Certificate has EKU (oid) %s, expects %s", szOid, expected_oid);
                if(!strcmp(expected_oid, szOid))
                {
                    fFound = CW_TRUE;
                }
            }
        }
    }

    if(eku != NULL)
    {
        sk_ASN1_OBJECT_pop_free(eku, ASN1_OBJECT_free);
    }

    return fFound;
}

/*
 * modificare questa funzione
 */
CWBool CWSecurityVerifyPeerCertificateForCAPWAP(SSL *ssl, CWBool isClient)
{
    X509 *x509;
    CWBool ret;

    if(ssl == NULL)
    {
        return CW_FALSE;
    }

    x509 = SSL_get_peer_certificate(ssl);
    if(x509 == NULL)
    {
        CWLog("Cannot get peer certificat");
        return CW_FALSE;
    }

    if(!isClient)
    {
        ret = CWSecurityVerifyCertEKU(x509, "1.3.6.1.5.5.7.3.19"); /* value expected for WTP */
    }
    else
    {
        ret = CWSecurityVerifyCertEKU(x509, "1.3.6.1.5.5.7.3.18"); /* value expected for AC */
    }

    X509_free(x509); //20151013,Andy Hu: avoid memory leak

    return ret;
}


/*
 * callbacks
 */
static int CWDTLSPasswordCB(char *buf, int num, int rwflag, void *userdata)
{
    if(buf == NULL || num < strlen(gSecurityPassword) + 1)
    {
        return 0;
    }

    strcpy(buf, gSecurityPassword);

    return strlen(gSecurityPassword);
}


int CWSecurityVerifyCB(int ok, X509_STORE_CTX *ctx)
{
    int preverify_ok = 1;
    char currentCertPath[PATH_MAX] = {0};
    char currentCAPath[PATH_MAX] = {0};
    char    buf[256];
    X509   *cur_cert;
    int     err, depth;

    sprintf(currentCAPath, "%s%s", gCWCertPath, CW_AC_CA_NAME_CURRENT);
    sprintf(currentCertPath, "%s%s", gCWCertPath, CW_WTP_FILE_CERT_NEW);

    /* If AC's current certificate was expired, reject all DTLS handshake.*/
    if(!gIsClient && CWIsCertExpired(currentCertPath, currentCAPath))
    {
        CWDebugLog("Certificate was expired!");
        return 0;
    }

    CWDebugLog("%s : result: %d, thread id: %u", __FUNCTION__, ok, 	pthread_self());

    err = X509_STORE_CTX_get_error(ctx);
    CWDebugLog("error code = %d : %s", err, X509_verify_cert_error_string(err));

    depth = X509_STORE_CTX_get_error_depth(ctx);

    /*
     * Retrieve the pointer to the SSL of the connection currently treated
     * and the application specific data stored into the SSL object.
     */
    cur_cert = X509_STORE_CTX_get_current_cert(ctx);
	X509_NAME_oneline(X509_get_subject_name(cur_cert), buf, 256);
    CWDebugLog("subject name = %s", buf);

    /*
     * Catch a too long certificate chain. The depth limit set using
     * SSL_CTX_set_verify_depth() is by purpose set to "limit+1" so
     * That whenever the "depth>verify_depth" condition is met, we
     * have violated the limit and want to log this error condition.
     * We must do it here, because the CHAIN_TOO_LONG error would not
     * be found explicitly; only errors introduced by cutting off the
     * additional certificates would be logged.
     */

    if(depth > CW_DTLS_CERT_VERIFY_DEPTH)
    {
        preverify_ok = 0;
        err = X509_V_ERR_CERT_CHAIN_TOO_LONG;
        X509_STORE_CTX_set_error(ctx, err);
    }

    if(!preverify_ok)
    {
        CWDebugLog("verify error:num=%d:%s:depth=%d:%s\n", err,
                   X509_verify_cert_error_string(err), depth, buf);
    }
    else
    {
        CWDebugLog("depth=%d:%s\n", depth, buf);
    }

    /*
     * At this point, err contains the last verification error. We can use
     * it for something special
     */
    if(!preverify_ok && (err == X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT))
    {
        X509_NAME_oneline(X509_get_issuer_name(cur_cert), buf, 256);
        CWDebugLog("issuer= %s\n", buf);
    }

    /*
     * Sigma:
     * check the WTP certificate was issued by this AC's current CA or not;
     * if no, terminate current DTLS session immediately.
     */
    if(!gIsClient)
    {

        /*
         *  Sigma:
         *  check current WTP cert is issued by current/default CA or not.
         *  Sometimes CWSecurityVerifyCB will be called several times with different cert
         *  in one handshake, set certResetData.issuedBy if one of the certs can be recognized.
         */
        char defaultCAPath[PATH_MAX] = {0};

        sprintf(defaultCAPath, "%s%s", gCWCertPath, CW_AC_CA_NAME_DEFAULT);

        CWDebugLog("currentCAPath = %s", currentCAPath);
        if(CWVerifyIsIssuedByCA(cur_cert, currentCAPath) ||
                CWVerifyIsIssuedByCA(cur_cert, defaultCAPath))
        {
            CWDebugLog("Current cert was issued by current or default CA!");
            preverify_ok = 1;
        }
        else
        {
            CWDebugLog("Current cert can NOT be recognized!");
            preverify_ok = 0;
        }
    }

    return preverify_ok;
}

unsigned int CWSecurityPSKClientCB(SSL *ssl,
                                   const char *hint,
                                   char *identity,
                                   unsigned int max_identity_len,
                                   unsigned char *psk,
                                   unsigned int max_psk_len)
{
    if(snprintf(identity, max_identity_len, "CLient_identity") < 0)
    {
        return 0;
    }

    /* TO-DO load keys from... Plain-text config file? Leave them hard-coded? */
    return psk_key2bn("1a2b3c", psk, max_psk_len);
}

unsigned int CWSecurityPSKServerCB(SSL *ssl,
                                   const char *identity,
                                   unsigned char *psk,
                                   unsigned int max_psk_len)
{
    CWDebugLog("Identity: %s, PSK: %s", identity, psk);
    /* TO-DO load keys from... Plain-text config file? Leave them hard-coded? */
    return psk_key2bn("1a2b3c", psk, max_psk_len);
}

/*
 * Convert the PSK key (psk_key) in ascii to binary (psk).
 */
int psk_key2bn(const char *psk_key, unsigned char *psk, unsigned int max_psk_len)
{

    unsigned int psk_len = 0;
    int ret;
    BIGNUM *bn = NULL;

    ret = BN_hex2bn(&bn, psk_key);
    if(!ret)
    {
        printf("Could not convert PSK key '%s' to BIGNUM\n", psk_key);
        if(bn)
        {
            BN_free(bn);
        }
        return 0;
    }

    if(BN_num_bytes(bn) > max_psk_len)
    {

        printf("psk buffer of callback is too small (%d) for key (%d)\n",
               max_psk_len, BN_num_bytes(bn));
        BN_free(bn);
        return 0;
    }
    psk_len = BN_bn2bin(bn, psk);
    BN_free(bn);

    if(psk_len < 0)
    {
        goto out_err;
    }
    return psk_len;
out_err:
    return 0;
}

/*
 * Sigma added:
 * Check target certificate is default certificate or not.
 */
CWBool CWisDefaultCert(const char *defaultCertFile, X509 *currentCert)
{
    if((defaultCertFile == NULL) || (currentCert == NULL))
    {
        CWDebugLog("%s, %u : File name or cert is empty.", __FUNCTION__, __LINE__);
        return CW_FALSE;
    }

    /*Load default WTP certificate into a buffer*/
    FILE *defaultCert = fopen(defaultCertFile, "r");
    if(!defaultCert)
    {
        CWDebugLog("%s, %u : Open default cert file error.", __FUNCTION__, __LINE__);
        return CW_FALSE;
    }

    long lSize = 0;
    fseek(defaultCert , 0L , SEEK_END);
    lSize = ftell(defaultCert);
    rewind(defaultCert);

    char *bufferDefaultCert;
    bufferDefaultCert = (char *)CWMemZeroAlloc(lSize);
    if(!bufferDefaultCert)
    {
        fclose(defaultCert);
        CWDebugLog("%s, %u : Memory allocate failed.", __FUNCTION__, __LINE__);
        return CW_FALSE;
    }

    size_t totalRead;
    totalRead = fread(bufferDefaultCert, 1, lSize, defaultCert);
    if(totalRead != lSize)
    {
        fclose(defaultCert);
        CWMemFree(bufferDefaultCert);
        CWDebugLog("%s, %u : Read entire file failed.", __FUNCTION__, __LINE__);
        return CW_FALSE;
    }
    fclose(defaultCert);

    /*Load current WTP certificate into another buffer for compare*/
    FILE *wtpCert = tmpfile();
    PEM_write_X509(wtpCert, currentCert);

    fseek(wtpCert , 0L , SEEK_END);
    lSize = ftell(wtpCert);
    rewind(wtpCert);

    char *bufferWTPCert;
    bufferWTPCert = (char *)CWMemZeroAlloc(lSize);
    if(!bufferWTPCert)
    {
        CWMemFree(bufferDefaultCert);
        fclose(wtpCert);
        CWDebugLog("%s, %u : Memory allocate failed.", __FUNCTION__, __LINE__);
        return CW_FALSE;
    }

    totalRead = fread(bufferWTPCert, 1, lSize, wtpCert);
    if(totalRead != lSize)
    {
        CWMemFree(bufferDefaultCert);
        fclose(wtpCert);
        CWMemFree(bufferWTPCert);
        CWDebugLog("%s, %u : Read entire file failed.", __FUNCTION__, __LINE__);
        return CW_FALSE;
    }
    fclose(wtpCert);

    /*Find default WTP cert in current WTP cert*/
    char *result = strstr(bufferWTPCert, bufferDefaultCert);
    CWMemFree(bufferDefaultCert);
    CWMemFree(bufferWTPCert);
    if(result == NULL)
    {
        return CW_FALSE;
    }
    else
    {
        return CW_TRUE;
    }
}

/*
 * Sigma added:
 * Verify a certificate was issued by CA or not.
 */
CWBool CWVerifyIsIssuedByCA(X509 *cert, const char *CAfile)
{
    CWBool ret = CW_FALSE;
    X509_STORE *cert_ctx = NULL;
    X509_LOOKUP *lookup = NULL;
    X509_STORE_CTX *csc = NULL;

    do
    {
        cert_ctx = X509_STORE_new();
        if(cert_ctx == NULL)
        {
            CWDebugLog("%s, %u : X509_STORE_new failed.", __FUNCTION__, __LINE__);
            break;
        }

        lookup = X509_STORE_add_lookup(cert_ctx, X509_LOOKUP_file());
        if(lookup == NULL)
        {
            CWDebugLog("%s, %u : X509_STORE_add_lookup failed.", __FUNCTION__, __LINE__);
            break;
        }

        if(!X509_LOOKUP_load_file(lookup, CAfile, X509_FILETYPE_PEM))
        {
            CWDebugLog("%s, %u : X509_LOOKUP_load_file failed.", __FUNCTION__, __LINE__);
            break;
        }
        /*
                lookup = X509_STORE_add_lookup(cert_ctx, X509_LOOKUP_hash_dir());
                if(lookup == NULL)
                {
                    CWDebugLog("%s, %u : X509_STORE_add_lookup failed.", __FUNCTION__, __LINE__);
                    break;
                }
        */
        X509_LOOKUP_add_dir(lookup, NULL, X509_FILETYPE_DEFAULT);

        csc = X509_STORE_CTX_new();
        if(csc == NULL)
        {
            CWDebugLog("%s, %u : X509_STORE_CTX_new failed.", __FUNCTION__, __LINE__);
            break;
        }

        X509_STORE_set_flags(cert_ctx, 0);
        if(!X509_STORE_CTX_init(csc, cert_ctx, cert, 0))
        {
            CWDebugLog("%s, %u : X509_STORE_CTX_init failed.", __FUNCTION__, __LINE__);
            break;
        }

        ret = X509_verify_cert(csc) > 0 ? CW_TRUE : CW_FALSE;
        if(!ret)
        {
            CWDebugLog("%s, %u : X509_verify_cert error code =  %d.", __FUNCTION__, __LINE__, X509_STORE_CTX_get_error(csc));
        }

    }
    while(0);

    if(csc != NULL)
    {
        X509_STORE_CTX_free(csc);
    }

    if(cert_ctx != NULL)
    {
        X509_STORE_free(cert_ctx);
    }

    return ret;
}

CWBool CWIsCertExpired(const char *fileCert, const char *fileCA)
{
    CWBool ret = CW_FALSE;
    X509_STORE *cert_ctx = NULL;
    X509_LOOKUP *lookup = NULL;
    X509_STORE_CTX *csc = NULL;
    X509 *xCert = NULL;
    BIO *bioCert = NULL;

    do
    {
        cert_ctx = X509_STORE_new();
        if(cert_ctx == NULL)
        {
            CWDebugLog("%s, %u : X509_STORE_new failed.", __FUNCTION__, __LINE__);
            break;
        }

        lookup = X509_STORE_add_lookup(cert_ctx, X509_LOOKUP_file());
        if(lookup == NULL)
        {
            CWDebugLog("%s, %u : X509_STORE_add_lookup failed.", __FUNCTION__, __LINE__);
            break;
        }

        if(!X509_LOOKUP_load_file(lookup, fileCA, X509_FILETYPE_PEM))
        {
            CWDebugLog("%s, %u : X509_LOOKUP_load_file failed.", __FUNCTION__, __LINE__);
            break;
        }

        lookup = X509_STORE_add_lookup(cert_ctx, X509_LOOKUP_hash_dir());
        if(lookup == NULL)
        {
            CWDebugLog("%s, %u : X509_STORE_add_lookup failed.", __FUNCTION__, __LINE__);
            break;
        }

        X509_LOOKUP_add_dir(lookup, NULL, X509_FILETYPE_DEFAULT);

        /* Load current WTP certificate */
        if((bioCert = BIO_new(BIO_s_file())) == NULL)
        {
            CWDebugLog("%s, %u : BIO_new failed.", __FUNCTION__, __LINE__);
            break;
        }

        if(BIO_read_filename(bioCert, fileCert) <= 0)
        {
            CWDebugLog("%s, %u : BIO_read_filename failed.", __FUNCTION__, __LINE__);
            break;
        }

        xCert = PEM_read_bio_X509_AUX(bioCert, NULL, NULL, NULL);

        csc = X509_STORE_CTX_new();
        if(csc == NULL)
        {
            CWDebugLog("%s, %u : X509_STORE_CTX_new failed.", __FUNCTION__, __LINE__);
            break;
        }

        X509_STORE_set_flags(cert_ctx, 0);
        if(!X509_STORE_CTX_init(csc, cert_ctx, xCert, 0))
        {
            CWDebugLog("%s, %u : X509_STORE_CTX_init failed.", __FUNCTION__, __LINE__);
            break;
        }

        if(X509_verify_cert(csc) == 0)
        {
            if(X509_STORE_CTX_get_error(csc) == 9 || X509_STORE_CTX_get_error(csc) == 10)
            {
                ret = CW_TRUE;
                CWDebugLog("%s, %u : X509_verify_cert error code =  %d.", __FUNCTION__, __LINE__, X509_STORE_CTX_get_error(csc));
            }
        }
    }
    while(0);

    if(bioCert != NULL)
    {
        BIO_free(bioCert);
    }

    if(xCert != NULL)
    {
        X509_free(xCert);
    }

    if(csc != NULL)
    {
        X509_STORE_CTX_free(csc);
    }

    if(cert_ctx != NULL)
    {
        X509_STORE_free(cert_ctx);
    }

    return ret;
}

void CWSetCertDir(const char *certDir)
{
    memset(gCWCertPath, 0, CW_CERT_PATH_LENGTH);

    if(certDir == NULL)
    {
        return;
    }

    sprintf(gCWCertPath, "%s", certDir);
}

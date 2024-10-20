/*****************************************************************************
;
;   (C) Unpublished Work of SENAO Networks Incorporated.  All Rights Reserved.
;
;       THIS WORK IS AN UNPUBLISHED WORK AND CONTAINS CONFIDENTIAL,
;       PROPRIETARY AND TRADESECRET INFORMATION OF SENAO INCORPORATED.
;       ACCESS TO THIS WORK IS RESTRICTED TO (I) SENAO EMPLOYEES WHO HAVE A
;       NEED TO KNOW TO PERFORM TASKS WITHIN THE SCOPE OF THEIR ASSIGNMENTS
;       AND (II) ENTITIES OTHER THAN SENAO WHO HAVE ENTERED INTO APPROPRIATE
;       LICENSE AGREEMENTS.  NO PART OF THIS WORK MAY BE USED, PRACTICED,
;       PERFORMED, COPIED, DISTRIBUTED, REVISED, MODIFIED, TRANSLATED,
;       ABBRIDGED, CONDENSED, EXPANDED, COLLECTED, COMPILED, LINKED, RECAST,
;       TRANSFORMED OR ADAPTED WITHOUT THE PRIOR WRITTEN CONSENT OF SENAO.
;       ANY USE OR EXPLOITATION OF THIS WORK WITHOUT AUTHORIZATION COULD
;       SUBJECT THE PERPERTRATOR TO CRIMINAL AND CIVIL LIABILITY.
;
;------------------------------------------------------------------------------
;
;    Project :
;    Creator : mook
;    File    :
;    Abstract:
;
;
;       Modification History:
;       By              Date     Ver.   Modification Description
;       --------------- -------- -----  --------------------------------------
;	  mook		2009-0409	Newly Create
;
;*****************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        SYSTEM INCLUDES                                */
/*-----------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#if USE_OPENSSL
#include <openssl/ssl.h>
#include <openssl/err.h>
#elif USE_MATRIXSSL
#include "matrixssl_helper.h"
#endif /* USE_OPENSSL */

/*-----------------------------------------------------------------------*/
/*                        APPLICATION INCLUDES                           */
/*-----------------------------------------------------------------------*/
#include "http_ssl.h"
#include "appagents.h"

/*-----------------------------------------------------------------------*/
/*                        DEFINES + TYPEDEFINE                           */
/*-----------------------------------------------------------------------*/
#define DEFAULT_CERTFILE   "/etc/agent.pem"
#define DEFAULT_HTTPS_PORT 9091

typedef struct {
	SSL *ssl;
#if USE_OPENSSL
	SSL_CTX *ssl_ctx;
#else /* USE_MATRIXSSL */
	sslKeys_t *ssl_ctx;
#endif
} HttpSslData;

/*-----------------------------------------------------------------------*/
/*                        GLOBAL VARIABLES                               */
/*-----------------------------------------------------------------------*/
int do_ssl = 0;
int https_port = DEFAULT_HTTPS_PORT;


/*-----------------------------------------------------------------------*/
/*                        LOCAL VARIABLES                                */
/*-----------------------------------------------------------------------*/
static HttpSslData *ssldata = NULL;
static char *certfile = DEFAULT_CERTFILE;


/*-----------------------------------------------------------------------*/
/*                        FUNCTIONS                                      */
/*-----------------------------------------------------------------------*/

#if USE_OPENSSL
/*****************************************************************
* NAME: initialize_ctx
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:
* OUTPUT:   
* Author: mook
* Modify:   
******************************************************************/
static SSL_CTX *initialize_ctx(char *keyfile)
{
	int error = 0;
	SSL_CTX *ctx;

	/* Global system initialization*/
	SSL_library_init();
	SSL_load_error_strings();

	/* Create our context*/
	ctx = SSL_CTX_new(SSLv23_server_method());

	/* Load our keys and certificates*/
	if (!SSL_CTX_use_certificate_file(ctx, keyfile, SSL_FILETYPE_PEM))
	{
		printf("ERROR: load certificate\n");
		error = -1;
	}

	if (!error &&
		!SSL_CTX_use_PrivateKey_file(ctx, keyfile, SSL_FILETYPE_PEM))
	{
		printf("ERROR: private key\n");
		error = -2;
	}

	/* Load the CAs we trust*/
	if (!error && !SSL_CTX_check_private_key(ctx))
	{
		printf("ERROR: check private key\n");
		error = -3;
	}

	if (error < 0)
	{
		SSL_CTX_free(ctx);
		ctx = NULL;
#if HTTPD_SSL_DEBUG
		ERR_print_errors_fp(stderr);
#endif
	}

	return ctx;
}

#else /* USE_MATRIXSSL */

/*****************************************************************
* NAME: initialize_key
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author: mook
* Modify:   
******************************************************************/
static sslKeys_t *initialize_key(char *certfile)
{
	int error = 0;
	sslKeys_t *keys = NULL;

	if ((error = matrixSslOpen() < 0))
	{
		printf("ERROR: open matrixssl\n");
	}

	if (!error &&
		matrixSslReadKeys(&keys, certfile, certfile, NULL, NULL ) < 0)
	{
		printf("ERROR: load certificate\n");
		keys = NULL;
	}

	return keys;
}
#endif /* USE_OPENSSL */

/*****************************************************************
* NAME: bind_ssl
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author: mook
* Modify:   
******************************************************************/
#if USE_OPENSSL
static SSL *bind_ssl(int fd, SSL_CTX *ctx)
#else /* USE_MATRIXSSL */
static SSL *bind_ssl(int fd, sslKeys_t *ctx)
#endif
{
	int error = 0, ret;
	SSL *ssl;

	ssl = SSL_new(ctx);
	if (!ssl)
		return NULL;

#if USE_OPENSSL
	if (SSL_set_fd(ssl, fd) != 1)
		error = -1;
#else /* USE_MATRIXSSL */
	SSL_set_fd(ssl, fd);
#endif

	if (!error && (ret = SSL_accept(ssl)) <= 0)
		error = -2;

	if (error < 0)
	{
		/*
		 * SSL_accept() will call SSL_free() if error occurrs,
		 * only matrixssl do this.
		 */
#if USE_OPENSSL
		SSL_free(ssl);
#endif
		ssl = NULL;
		printf("ERROR: bind SSL with sockfd, error %d, SSL_accept %d\n",
			error, ret);
#if HTTPD_SSL_DEBUG && USE_OPENSSL
		ERR_print_errors_fp(stderr);
#endif
	}

	return ssl;
}

/*****************************************************************
* NAME: http_ssl_params
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author: mook
* Modify:   
******************************************************************/
int http_ssl_params(int argc, char **argv)
{
	int argn = 1;

	while (argn < argc && argv[argn][0] == '-')
	{
		if (!strcmp(argv[argn], "-s"))
		{
			do_ssl = 1;
		}
		else if (!strcmp(argv[argn], "-k"))
		{
			argn++;
			certfile = argv[argn];
		}
		else if (!strcmp(argv[argn], "-p"))
		{
			argn++;
			https_port = atoi(argv[argn]);
		}
		else if (!strcmp(argv[argn], "-d"))
		{
			do_dbg = 1;
		}
		argn++;
	}

	printf("Serving httpd as https ... %s\n", do_ssl ? "YES" : "NO");
	if (do_ssl)
	{
		printf(" - port %d\n", https_port);
		printf(" - using certificate file %s\n", certfile);
	}

	if (do_dbg)
		printf(" ###  debug: %d\n",do_dbg);
	return 0;
}

/*****************************************************************
* NAME: http_ssl_init
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author: mook
* Modify:   
******************************************************************/
int http_ssl_init()
{
	if (access(certfile, F_OK) < 0)
	{
		printf("ERROR: certificate file %s doesn't exist\n", certfile);
		return -1;
	}

	ssldata = (HttpSslData *)malloc(sizeof(HttpSslData));
	if (!ssldata)
		return -2;

#if USE_OPENSSL
	ssldata->ssl = NULL;
	ssldata->ssl_ctx = initialize_ctx(certfile);
#else /* USE_MATRIXSSL */
	ssldata->ssl_ctx = initialize_key(certfile);
#endif
	if (!ssldata->ssl_ctx)
	{
		free(ssldata);
		return -3;
	}

	return 0;
}

/*****************************************************************
* NAME: http_ssl_accept
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author: mook
* Modify:   
******************************************************************/
int http_ssl_accept(int fd)
{
	ssldata->ssl = bind_ssl(fd, ssldata->ssl_ctx);
	if (!ssldata->ssl)
		return -1;

	return 0;
}

/*****************************************************************
* NAME: http_ssl_freectx
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author: mook
* Modify:   
******************************************************************/
void http_ssl_freectx(void)
{
	if (!ssldata)
		return;

#if USE_OPENSSL
	SSL_CTX_free(ssldata->ssl_ctx);
#else
	matrixSslFreeKeys(ssldata->ssl_ctx);
	matrixSslClose();
#endif
	free(ssldata);
}

/*****************************************************************
* NAME: http_ssl_free
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author: mook
* Modify:   
******************************************************************/
void http_ssl_free(void)
{
	if (!ssldata)
		return;

	if (ssldata->ssl)
	{
		SSL_free(ssldata->ssl);
		ssldata->ssl = NULL;
	}
}

/*****************************************************************
* NAME: http_ssl_read
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author: mook
* Modify:   
******************************************************************/
ssize_t http_ssl_read(char *buf, int len)
{
	ssize_t bytes;

	bytes = SSL_read(ssldata->ssl, buf, len);
//printf("%s: bytes %d\n", __func__, bytes);
	return bytes;
}

/*****************************************************************
* NAME: http_ssl_write
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author: mook
* Modify:   
******************************************************************/
ssize_t http_ssl_write(char *buf, int len)
{
	ssize_t bytes;
	
	if(!ssldata->ssl || len <= 0)
	{
#ifdef DEBUG
		printf("[app_agentd] http_ssl_write --- fail \n");
#endif
		return -1;
	}

	bytes = SSL_write(ssldata->ssl, buf, len);
	//printf("%s: bytes %d\n", __func__, bytes);
	return bytes;
}

#if USE_MATRIXSSL
/*****************************************************************
* NAME: http_ssl_inbuffer
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author: mook
* Modify:   
******************************************************************/
int http_ssl_inbuffer(void)
{
	if (ssldata && SSL_dataBuffered(ssldata->ssl) > 0)
		return 1;
	return 0;
}
#endif /* USE_MATRIXSSL */

/******************************************************************
<<END>>
******************************************************************/

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
;    Creator :
;    File    :
;    Abstract:
;
;
;       Modification History:
;       By              Date     Ver.   Modification Description
;       --------------- -------- -----  --------------------------------------
;
;*****************************************************************************/
#ifndef _HTTP_SSL_H_
#define _HTTP_SSL_H_

/*-----------------------------------------------------------------------*/
/*                        EXPORT VARIABLES                               */
/*-----------------------------------------------------------------------*/
extern int do_ssl;
extern int https_port;
extern char *httpd_certfile;

/*-----------------------------------------------------------------------*/
/*                        EXPORT FUNCTIONS                               */
/*-----------------------------------------------------------------------*/
int http_ssl_params(int, char **);
int http_ssl_init(void);
int http_ssl_accept(int);
void http_ssl_freectx(void);
void http_ssl_free(void);
ssize_t http_ssl_read(char *, int);
ssize_t http_ssl_write(char *, int);
#if USE_MATRIXSSL
int http_ssl_inbuffer(void);
#endif /* USE_MATRIXSSL */
#endif /* _HTTP_SSL_H_ */

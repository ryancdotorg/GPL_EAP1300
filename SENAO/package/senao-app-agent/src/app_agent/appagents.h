#if __cplusplus
extern "C" {
#endif

#ifndef _APPAGENTS_H_
#define _APPAGENTS_H_


/*--------------------------------------------------------------------------*/
/*                           INCLUDE HEADER FILES                           */
/*--------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>

#include "ostypes.h"
#include "envcfg.h"
#include "sap_ostypes.h"


/**********************************
 constant and  data sturcture
**********************************/
#define REQUEST                     0
#define RESPONSE                    1

#define NORMAL 		                0
#define CR 		                    13
#define LF 		                    10
#define CRLF 		                1
#define CRLFCR 		                2
#define CRLFCRLF 	                3
#define CRCR                        4

#define HTTP_REQ_POSTSIZE           4096

/* OEM Require */
#define HTTP_CFG_PORT               80

/* server settings */
#define HTTP_GATEWAY_INTERFACE      "CGI/1.1"
#define HTTP_FULL_REQUEST           "HTTP/1.0"

#define MAX_FIELD_LEN	            256
#define MAX_HEADER_LEN	            2048
#define MAX_URI_LEN		            256

#define MAX_READ 		            512
#define MAX_ERROR_HEADER_LEN        512

/* error constants */
#define E_RECV			            4
#define E_SEND			            5

/* request.method */
#define M_GET 	                    0
#define M_HEAD	                    1
#define M_POST	                    2


/* request.status */
#define R_SILENT                    999		

#define R_BAD_REQUEST               400
#define R_FORBIDDEN                 403
#define R_NOT_FOUND                 404
#define R_METHOD_NA                 405
#define R_NONE_ACC                  406

#define R_ERROR                     500
#define	R_NOT_IMP                   501

typedef enum _XML_AGENT_PACKET_TYPE_
{
	NOT_XML_PACKET = 0,
	XML_LOG_IN,
	XML_ROUTER_INFO
} XML_AGENT_PACKET_TYPE;

/**********************************
  data structure                
**********************************/
typedef struct HTTPS_CB_S
{
    int fd;                                 /* client's socket fd */
    int method;                             /* M_GET, M_POST, M_HEAD */

    char request_uri[MAX_URI_LEN + 1];      /* uri */
    char header_in[MAX_HEADER_LEN + 1];     /* full raw header */

    int body_index;                         /* pts to first char after validated header in header_in */
    int body_cnt;                           /* number of bytes already read from body */

    int header_idx;                         /* index to next unread char */
    int header_end_idx;                     /* index to one char past last */

    int http_simple;                        /* true if HTTP version 0.9 (simple) */
    int status;                             /* R_OK et al. */
    int browsing;                           /* flag used for setting content-type 
                                               during a browse request */
    struct envcfg_t envcfg;
    int multipart;                          /* process multipart  */

    int json;
    char json_action[64];
#if HAS_MESH_JSON
    int mesh;
#endif
    int senao_app_client;
}
HTTPS_CB;

/**********************************
  function prototype                
**********************************/
void https(int s);

int http_store_data_to_buffer(char *format, ...);
int http_send_stored_data(int fd);
int do_dbg;

#endif  /* _HTTP_REQUEST_H_ */

#if __cplusplus
}
#endif

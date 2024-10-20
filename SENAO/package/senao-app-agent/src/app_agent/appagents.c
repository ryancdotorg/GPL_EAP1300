/****************************************************************************
;
;   (C) Unpublished Work of Senao Networks, Inc.  All Rights Reserved.
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
;----------------------------------------------------------------------------
;
;    Project : app_agent
;    Creator : Jerry Chen
;    File    : hnap_setting.c
;    Abstract:
;
;       Modification History:
;       By              Date        Ver.    Modification Description
;       --------------- --------    -----   -------------------------------------
;       Jerry           2012/09/10          First commit
;****************************************************************************/

#include "appagents.h"
#include "errno.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/in.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>
#include <string.h>
#include <time.h>
#if APP_AGENTD_HAS_SSL
#include "http_ssl.h"
#endif

#if HAS_ENCRYPT_PAYLOAD
#include "aes.h"
#include "base64.h"

extern int do_encrypt_payload;

#define ENCRYPT_DEBUG(fmt,args...) do{if(do_dbg)printf("%s: "fmt"\n",__FUNCTION__,##args);}while(0)
//#define ENCRYPT_DEBUG(args...)
//#define ENCRYPT_DEBUG2(fmt,args...) do{if(do_dbg)printf("%s: "fmt"\n",__FUNCTION__,##args);}while(0)
#define ENCRYPT_DEBUG2(args...)
#endif

/**********************************
 Definitions
***********************************/
#define HTTP_V_1_0                "HTTP/1.0"
#define HTTP_V_1_1                "HTTP/1.1"

#define JSON_POST_HEADER          "POST /json/"
#define JSON_GET_HEADER           "GET /json/"

#define USER_AGENT_HEADER         "User-Agent: "

#define SENAO_APP_CLIENT_NEWAGENT "SenaoAppClient/1.0"
#define SENAO_APP_CLIENT_OLDAGENT "Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.9.2.24) Gecko/20111108 Fedora/3.6.24-1.fc14 Firefox/3.6.24"

#if HAS_MESH_JSON
#define JSON_MESH_HEADER          "mesh/"
#endif

#define MAX_BUFFER_SIZE 64*1024

/**********************************
 Variables
***********************************/
static  int (*pHttpsCgiMain)() = 0;

static char msg_buf[MAX_BUFFER_SIZE] = {0}; //the szie for http_send_stored_data to PC
static char tmp_buf[MAX_BUFFER_SIZE]={0};
static int msg_len = 0;

/**********************************
 static function prototype
***********************************/
static void err_end(int, HTTPS_CB *);

static int check_for_end_of_header(HTTPS_CB *, int *, int *);
static int parse_header(HTTPS_CB *);
static int parse_method(HTTPS_CB *);
static int parse_uri(HTTPS_CB *);
static int read_header(HTTPS_CB *);
static void cleanup_uri(char * request_uri, HTTPS_CB *);
static int send_error_reply(HTTPS_CB * req);
static int build_header(HTTPS_CB * req);

int http_recv (int s, char *buf, int len, int flags);

static int parse_JSONAction(HTTPS_CB *);
static int parse_UserAgent(HTTPS_CB *);

/*---------------------------------------------------------------------------------
	Global Variables
----------------------------------------------------------------------------------*/
char *http_standard_env [] =

{
	"CONTENT_LENGTH",
	"CONTENT_TYPE",
	"PATH_INFO",
	"AUTHORIZATION",
	"IF_MODIFIED_SINCE",
	(char *) 0
};

/*----------------------------------------------------------------------
* ROUTINE NAME - err_end
*-----------------------------------------------------------------------
* DESCRIPTION: 
*   Socket error while sending or receiving header.
*
* INPUT:
*   error_code   -- error code number
*   req          -- pointer to request control block
*
* RETURN:    
*   none
*----------------------------------------------------------------------*/
static void err_end(int error_code, HTTPS_CB *req)
{
	switch(error_code)
	{
	case E_SEND:
		printf("htReqest.c:err_end: E_SEND error\n");
		break;

	case E_RECV:
		printf("htReqest.c:err_end: E_RECV error\n");
		break;
	}
}

/*----------------------------------------------------------------------
* ROUTINE NAME - parse_header
*-----------------------------------------------------------------------
* DESCRIPTION: 
*   Parse header and set appropriate env variables.
*
* INPUT:
*   req          -- pointer to request control block
*
* RETURN:    
*   0 on success, -1 on failure
*----------------------------------------------------------------------*/
static int parse_header(HTTPS_CB * req)
{
//    char *buffer;
	char *name;
	char *value;
	char *field;

	char *last_field;
	int i;
	int field_done;
	int header_done;


	/* the null-terminated raw header data is now in req->header */
	/* must be at least "GET ?<CR><LF><CR><LF>" or 9 chars (? = min. URI) */
	if(req->header_end_idx < 9)
	{
		req->status = R_BAD_REQUEST;
		return (ERROR);
	}
	/* we got at least 9 bytes here */

	/* Parse the Request-Line */

	if(parse_method(req) == ERROR)
	{
		return (ERROR);
	}

	if(parse_uri(req) == ERROR)
	{
		return (ERROR);
	}

	if (req->json && parse_JSONAction(req) == ERROR)
	{
		return(ERROR);
	}

	if (req->json && parse_UserAgent(req) == ERROR)
	{
		return(ERROR);
	}

	/* Parse the General-Header */
	/* Parse the Request-Header */

	/* req->header_idx now points to version of HTTP in header_in */

	field = req->header_in;
	do
	{
		if(req->header_in[req->header_idx++] == '\n')
		{
			req->header_in[req->header_idx - 1] = 0;
			break;
		}

	} while(req->header_idx < req->header_end_idx);

	/* req->header_idx is index to char past request-line in header_in */

	last_field = field;

	if(field[strlen(field) - 1] == '\r')
		field[strlen(field) - 1] = 0;

	/* don't do anything with 1st field, this was already parsed above */
	header_done = 0;
	do
	{
		field_done = 0;
		if(req->header_idx >= req->header_end_idx)
			break;


		field = &req->header_in[req->header_idx];

		while(1)
		{
			if(req->header_in[req->header_idx++] == '\n')
			{
				req->header_in[req->header_idx - 1] = 0;
				break;
			}

			if(req->header_idx >= req->header_end_idx)
			{
				field = NULL;
				header_done = 1;
				break;
			}
		};

		if(field == NULL)
			break;

		if(field[strlen(field) - 1] == '\r')
			field[strlen(field) - 1] = 0;

		/* check for field found, and that field found is
		   not same as last field found */
		if((strlen(field)) && (field != last_field))
		{
			last_field = field;

			/* seperate name & value from field and set ptrs */
			name = field;
			for(i = 0; i < strlen(field); i++)
			{
				if(field[i] == ':')
				{
					value = &field[i+1];
					field[i] = 0;
					break;
				}


			}

			/* Convert name to upper case and replace '-' with '_' */
			for(i = 0; i < strlen(name); i++)
			{
				if(name[i] == '-')
					name[i] = '_';
				else
					name[i] = toupper(name[i]);
			}

			/* Compare name with set of standard env fields; if found set env */
			for(i = 0; http_standard_env[i] != 0; i++)
			{
				if(!strcmp(name, http_standard_env[i]))
				{
					set_env(&req->envcfg, name, value);
					field_done = 1;
					break;
				}
			}
		}
		else
			header_done = 1;

	} while(!header_done);

	return (OK);
}

/*----------------------------------------------------------------------
* ROUTINE NAME - parse_method
*-----------------------------------------------------------------------
* DESCRIPTION: 
*   Determine method type

*
* INPUT:
*   req          -- pointer to request control block
*
* RETURN:    
*   OK on success, ERROR on failure
*----------------------------------------------------------------------*/
static int parse_method(HTTPS_CB * req)
{
	if(strncmp(req->header_in, "GET ", 4)==0)
	{
		req->method = M_GET;
		req->header_idx = 4;
	}
	else if(strncmp(req->header_in, "HEAD ", 5)==0)
	{
		req->method = M_HEAD;
		req->header_idx = 5;
	}
	else if(strncmp(req->header_in, "POST ", 5)==0)
	{
		req->method = M_POST;
		req->header_idx = 5;
	}
	else
	{	/* not a valid method */
		req->status = R_BAD_REQUEST;
		return (ERROR);
	}

	return (OK);
}

static int parse_JSONAction(HTTPS_CB * req)
{
	char *pJSONAction;
	char *pJSONAction_end;
	char *pHTTP_version;

	pJSONAction = NULL;

	if((pJSONAction = (char*)strcasestr(req->header_in, JSON_GET_HEADER)) !=NULL)
	{
		pJSONAction += strlen(JSON_GET_HEADER);
	}
    else if((pJSONAction = (char*)strcasestr(req->header_in, JSON_POST_HEADER)) !=NULL)
	{
		pJSONAction += strlen(JSON_POST_HEADER);
	}

	if(pJSONAction != NULL)
	{
#if HAS_MESH_JSON
		if(0 == strncmp(pJSONAction, JSON_MESH_HEADER, strlen(JSON_MESH_HEADER)))
		{
			req->mesh = 1;
			pJSONAction += strlen(JSON_MESH_HEADER);
		}
#endif
		pHTTP_version = strstr(pJSONAction, HTTP_V_1_1);
		pJSONAction_end = strchr(pJSONAction, '/');

		/* The case ((pHTTP_version - pJSONAction_end) > 0) are shown below.*/
		/* GET /json/function_name/ HTTP/1.1 */
		/* GET /json/function_name/?par1=123&par2=456 HTTP/1.1 */
		if((pHTTP_version - pJSONAction_end) < 0)
		{
			if(pHTTP_version == pJSONAction_end - 4)
				{
				/* GET /json/function_name HTTP/1.1 */
				/* Found character '/' is in HTTP_V_1_1 */
				pJSONAction_end = pHTTP_version - 1;

				/* Skip the redundant white space */
				while(*(--pJSONAction_end) == ' ');
			}
		}

		if(!((*pJSONAction_end == '/') || (*(++pJSONAction_end) == ' ')))
		{
			return(ERROR);
		}

		snprintf(req->json_action, pJSONAction_end-pJSONAction+1, "%s", pJSONAction); // +1 if for '\0'

//#ifdef DEBUG
#if 1
		printf("[app_agentd] JSONAction: [%s] \n", req->json_action);
#endif
	}

	return(OK);
}

static int parse_UserAgent(HTTPS_CB * req)
{
	char *pUserAgent;

	pUserAgent = NULL;
	req->senao_app_client = 0;

	if((pUserAgent = (char*)strcasestr(req->header_in, USER_AGENT_HEADER)) !=NULL)
	{
		pUserAgent += strlen(USER_AGENT_HEADER);
	}

	if(pUserAgent != NULL)
	{
		if( (0 == strncmp(SENAO_APP_CLIENT_NEWAGENT, pUserAgent, strlen(SENAO_APP_CLIENT_NEWAGENT)))
		 || (0 == strncmp(SENAO_APP_CLIENT_OLDAGENT, pUserAgent, strlen(SENAO_APP_CLIENT_OLDAGENT))))
		{
			req->senao_app_client = 1;
		}
	}

	return(OK);
}

/*----------------------------------------------------------------------
* ROUTINE NAME - parse_uri
*-----------------------------------------------------------------------

* DESCRIPTION: 
*   copy the uri from header_in to request_uri. update header_idx.
*   update http_simple.
*
* INPUT:
*   req          -- pointer to request control block
*
* RETURN:    
*   OK on success, ERROR on failure

*----------------------------------------------------------------------*/
static int parse_uri(HTTPS_CB * req)
{
	int uri_idx = 0;

	while((req->header_in[req->header_idx] != '\0') &&
		  (req->header_in[req->header_idx] != ' '))
	{
		req->request_uri[uri_idx] = req->header_in[req->header_idx];

		uri_idx++;              /* destination */
		req->header_idx++;      /* source */

		if(uri_idx >= MAX_URI_LEN)
		{
			/* URI is too long */

			req->status = R_BAD_REQUEST;
			return (ERROR);
		}
	}

	/* null-terminate the uri string */
	if(strncmp(req->header_in, JSON_GET_HEADER,  strlen(JSON_GET_HEADER))==0 ||
	   strncmp(req->header_in, JSON_POST_HEADER, strlen(JSON_POST_HEADER))==0)
	{
		req->json=1;
	}

	/* if at end of GET request header then it must be
	   a simple request */
//gwochern 0318
	if(req->header_in[req->header_idx] == '\0')
	{
		if(req->method == M_GET)
		{
			req->http_simple = TRUE;
			cleanup_uri(req->request_uri, req);
			req->request_uri[uri_idx] = '\0';
			return (OK);
		}
		else
		{  /* malformed Full-Request header */
			req->status = R_BAD_REQUEST;
			return (ERROR);
		}
	}

	req->http_simple = FALSE;

	/* now move index to HTTP-Version in header since we have one*/
	req->header_idx++;
	cleanup_uri(req->request_uri, req);
	set_env(&req->envcfg,"PATH_INFO", req->request_uri);
	return (OK);
}

/*----------------------------------------------------------------------
* ROUTINE NAME - cleanup_uri
*-----------------------------------------------------------------------
* DESCRIPTION: 
*   remove any ".." from a request uri for security.
*   Set env variable QUERY_STRING if a ? is found in the uri.
*
* INPUT:
*   request_uri  -- request uri string
*   req          -- pointer to request control block
*
* RETURN:    
*   none
*----------------------------------------------------------------------*/

static void cleanup_uri(char * request_uri, HTTPS_CB *req) 
{
	char *buffer;
	int i = 0;
	int src_idx = 0;

	int dest_idx = 0;
	int max_src_idx = strlen(request_uri) - 1;

	// allocate buffer
	if((buffer = (char *)malloc (MAX_URI_LEN)) == 0)
	{
		return;
	}

	/* strip off first '.' of a '..', resultant path may be bad  */

	/* loop until we find a '?' or until end of string */
	while((src_idx <= max_src_idx) && (request_uri[src_idx] != '?'))
	{
		if((request_uri[src_idx] == '.') && 
		   (request_uri[src_idx + 1] == '.'))
		{
			src_idx++;
		}
		else
		{
			request_uri[dest_idx] = request_uri[src_idx];
			dest_idx++;
			src_idx++;
		}
	}

	request_uri[dest_idx] = '\0';
	src_idx++;

	/* copy parameter list unaltered */

	while(src_idx <= max_src_idx)
	{
		buffer[i++] = request_uri[src_idx++];
	}
	buffer[i] = 0;	/* NULL terminate */

	if(i)
	{
		set_env(&req->envcfg,"QUERY_STRING", buffer);
	}

	// free buffer
	free (buffer);
}

/*----------------------------------------------------------------------
* ROUTINE NAME - read_header
*-----------------------------------------------------------------------
* DESCRIPTION: 
*   Read date from socket until find end of header or error occurs.
*
* INPUT:
*   req          -- pointer to request control block
*
* RETURN:    
*   OK on success, ERROR on failure
*----------------------------------------------------------------------*/
static int read_header(HTTPS_CB *req)
{
	char *buffer;
	int index;
	int prev_char;
	int header_size = 0;
	int rv;
	int nAsk;
	int i, num;
    char ipstr[64];

	index = 0;
	prev_char = NORMAL;

	//header_end_idx is char length of recv
	req->header_end_idx = http_recv(req->fd, req->header_in, MAX_HEADER_LEN, 0);

#ifdef DEBUG
	printf("[app_agentd] req->header_in \n---[\n%s\n]---\n", req->header_in);
#endif

#if 1 //gwochern 0318
	if(req->header_end_idx == -1)
	{
		if(errno == ECONNRESET)
		{
			req->status = R_SILENT;
			return (ERROR);
		}
		else
		{
			req->status = R_ERROR;
			if(do_dbg)
			{
				sprintf(ipstr, "%s", sysutil_get_peername(req->fd));
				printf("[app_agentd] [%s:%d] source ip [%s]\n", __func__, __LINE__, ipstr);
				printf("[app_agentd] [%s:%d] req->header_in \n---[\n%s\n]---\n", __func__, __LINE__, req->header_in);
			}
			return (ERROR);
		}
	}
#endif

	/* 2011-03-18: Norkay, Use isprint to check non-HTTP packet. If req is non-HTTP traffic return ERROR. */
	num = req->header_end_idx < 5 ? req->header_end_idx : 5;
	for(i = 0; i < num; ++i)
	{
		if(!isprint(req->header_in[i]))
			return(ERROR);
	}

	header_size += req->header_end_idx;
	rv = check_for_end_of_header(req, &index, &prev_char);

/*-------------------------------------------------------------------------------
	   if rv == 0, then we found end of header on first read

	   else, we need to see if more data is available to read.
	   If data is available on socket, do another recv, then look for end of header.
	   Repeat while loop until one of the following occurs:

			find end of header(CRLFCRLF or CRCR).           GOOD HDR
			read past MAX_HDR_LEN                           BAD HDR
---------------------------------------------------------------------------------*/

	// allocate variable
	if((buffer = (char *)malloc (MAX_HEADER_LEN)) == 0)
	{
		req->status = R_ERROR;
		if(do_dbg)
		{
			printf("[app_agentd] [%s:%d] req->header_in \n---[\n%s\n]---\n", __func__, __LINE__, req->header_in);
		}
		return (ERROR);
	}

	while(rv)
	{
		/* how much can read (1998-08-03, Zhong Qiyao) */
		nAsk = MAX_HEADER_LEN - header_size;

		if(nAsk == 0)
		{
			req->status = R_BAD_REQUEST;

			// free
			free (buffer);

			return (ERROR);
		}

		//memset(buffer, 0, sizeof(buffer));
		// the buffer lenth should be MAX_HEADER_LEN, sumei chung, 1999.07.16
		memset(buffer, 0, MAX_HEADER_LEN);

		req->header_end_idx = 0;

		req->header_end_idx = http_recv(req->fd, buffer, nAsk, 0);

/* when a telnet session disconnects before sending a complete header, recv returns 0 */
/* recv will continue to return 0 forever, so we look for 0 below to exit if needed */
/* recv should block until a datagram is available, or return -1 on disconnect */

		if((req->header_end_idx == -1) || (req->header_end_idx == 0))
		{
			if(errno == ECONNRESET)
			{
				req->status = R_SILENT;

				// free
				free (buffer);

				return (ERROR);
			}
			else
			{
				// free
				free (buffer);

				req->status = R_ERROR;
				if(do_dbg)
				{
					printf("[app_agentd] [%s:%d] req->header_in \n---[\n%s\n]---\n", __func__, __LINE__, req->header_in);
				}
				err_end(E_RECV, req);
				return (ERROR);
			}
		}

		memcpy (&req->header_in[header_size], buffer, req->header_end_idx);
		header_size += req->header_end_idx;		/* increment local header count */
		req->header_end_idx = header_size;		/* adj header end index */
		//strcat(req->header_in, buffer);


		rv = check_for_end_of_header(req, &index, &prev_char);
		if(index >= MAX_HEADER_LEN)
		{
			req->status = R_BAD_REQUEST;

			// free

			free (buffer);

			return (ERROR);
		}
	}

	/* the header is terminated properly   */

	/* index points to first char of body */
	req->body_index = index;
	req->body_cnt = header_size - index;

	req->header_end_idx = strlen(req->header_in);		/* adjust index to end of header */

	req->header_in[req->header_end_idx - 1] = '\n';	   /* leave a single LF at end for use with parse_header */

	// free
	free (buffer);

	return (OK);
}

/*----------------------------------------------------------------------
* ROUTINE NAME - check_for_end_of_header
*-----------------------------------------------------------------------
* DESCRIPTION: 
*   search through data to determine if we recv'd a full header
*   a complete header is terminated by a CRLFCRLF or CRCR
*   we will terminate the header leaving CRNULL
*
* INPUT:
*   req          -- pointer to request control block
*
* RETURN:    
*   OK on success, ERROR on failure
*----------------------------------------------------------------------*/
static int check_for_end_of_header(HTTPS_CB *req, int *index, int *prev_char)
{
	/* search through data to determine if we recv'd a full header */
	/* a complete header is terminated by a CRLFCRLF or CRCR       */
	/* we will terminate the header leaving CRNULL         */

	while(*index <  req->header_end_idx)
	{
		if(req->header_in[*index] == CR)
		{
			if(*prev_char == CR)
			{
				*prev_char = CRCR;
				req->header_in[*index] = 0;           /* NULL terminate header */
				(*index)++;
				return 0;                             /* found end of header */
			}
			else if(*prev_char == CRLF)
			{
				*prev_char = CRLFCR;
				(*index)++;
			}
			else
			{
				*prev_char = CR;
				(*index)++;
			}
		}
		else if(req->header_in[*index] == LF)
		{
			if(*prev_char == CR)
			{
				*prev_char = CRLF;
				(*index)++;
			}
			else if(*prev_char == CRLFCR)
			{
				*prev_char = CRLFCRLF;
				req->header_in[(*index) - 2] = 0;     /* NULL terminate header */
				(*index)++;
				return 0;                             /* found end of header */
			}
			else
			{
				*prev_char = NORMAL;
				(*index)++;
			}
		}
		else
		{
			*prev_char = NORMAL;
			(*index)++;
		}
	}
	return 1;                                         /* did not find end of header */
}

int http_store_data_to_buffer(char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	vsnprintf(tmp_buf, sizeof(tmp_buf), format, ap);

	strncat(msg_buf, tmp_buf, strlen(tmp_buf));
	msg_len += strlen(tmp_buf);
	return 1;
}


int http_send_stored_data(int fd)
{
	int res = 0, len = 0;
	char *len_pos = NULL, *rest_data_pos = NULL, *body_start_pos = NULL, *body_end_pos = NULL;

	if (fd < 0) return 0;

	body_start_pos = strchr(msg_buf, '{');
	body_end_pos = strrchr(msg_buf, '}');

	if((body_start_pos != NULL) && (body_end_pos != NULL))
	{
		len = body_end_pos - body_start_pos + 1;
	}

#if HAS_ENCRYPT_PAYLOAD
	if(do_encrypt_payload)
	{
		char *ciphertext = NULL, *base64text = NULL;
		int cipherlen, base64len;

		if(body_start_pos == NULL || len <= 0)
		{
			printf("%s: no encryption source.\n",__FUNCTION__);
		}
		else
		{
			ENCRYPT_DEBUG("try to encrypt data (%d):\n%s",len,body_start_pos);
			// "len + 1"  to fix oldversion app_agent (use previous openssl 1.1.1g)
			*(body_end_pos+1) = '\0';
			ciphertext = encrypt_data(body_start_pos, len+1, &cipherlen);
			ENCRYPT_DEBUG2("cipher data length: %d.",cipherlen);
		}
		if(ciphertext != NULL && cipherlen > 0)
		{
			ENCRYPT_DEBUG2("try to base64 data...%d (%d)",cipherlen,len);
			base64text = base64_encode(ciphertext, cipherlen, &base64len);
			ENCRYPT_DEBUG("base64 data length: %d.",base64len);
		}
		if(base64text != NULL && base64len > 0)
		{
			int newlen = msg_len - len + base64len;
			ENCRYPT_DEBUG2("get base64 data:\n%s",base64text);
			if(newlen >= MAX_BUFFER_SIZE)
				printf("%s: base64 text overflow (%d)!\n",__FUNCTION__,newlen);
			else
			{
				memcpy(body_start_pos,base64text,base64len);
				// suppose no more string beyond original end of body text
				msg_buf[newlen] = '\0';
				msg_len = strlen(msg_buf);
				len = base64len;
			}
		}
		if(base64text != NULL)
			free(base64text);
		if(ciphertext != NULL)
			free(ciphertext);
	}
#endif // HAS_ENCRYPT_PAYLOAD

	if (strstr(msg_buf, "Content-Length:") != NULL)
	{
		len_pos = strstr(msg_buf, "Content-Length:") + strlen("Content-Length: ");
		rest_data_pos = strstr(len_pos, "\r\n") + strlen("\r\n");

		sprintf(len_pos, "%d\r\n", len);
		strncat(msg_buf, rest_data_pos, msg_len);
	}

	if(do_dbg)
	{
		printf("[app_agentd] SEND to Phone \n");
		printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
		printf("%s\n", msg_buf);
		printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	}

#ifdef DEBUG
	printf("[app_agentd] SEND to Phone \n");
#ifdef XML_DEBUG
	printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	printf("%s\n", msg_buf);
#endif
#endif

	res = http_send(fd, msg_buf, msg_len, 0);
    
	// clear all the buffer data after transmitting.
	memset(msg_buf, 0, sizeof(msg_buf));
	msg_len = 0;

	return res;
}

/*----------------------------------------------------------------------

* ROUTINE NAME - send_error_reply
*-----------------------------------------------------------------------
* DESCRIPTION: 
*   Send an error reply message to client
*
* INPUT:
*   req          -- pointer to request control block
*
* RETURN:    
*   OK on success, ERROR on failure
*----------------------------------------------------------------------*/
static int send_error_reply(HTTPS_CB * req)
{
	char *buffer;

	// allocate buffer
	if((buffer = (char *)malloc (MAX_ERROR_HEADER_LEN)) == 0)
	{
		return (-1);
	}

	/* generate header and send relevant error message */
	switch(req->status)
	{
	case R_BAD_REQUEST:
		if(do_dbg)
			printf("- send_error_reply (R_BAD_REQUEST). \n");
		strcpy(buffer, "HTTP/1.0 400 Bad Request\r\n");
		strcat(buffer, "Content-Type: text/html\r\n\r\n");
		strcat(buffer, "<title>400 Bad Request</title><body>400 Bad Request</body>\0");
		break;

	case R_FORBIDDEN:
		if(do_dbg)
			printf("- send_error_reply (R_FORBIDDEN). \n");
		strcpy(buffer, "HTTP/1.0 403 Forbidden\n");

		strcat(buffer, "Content-Type: text/html\n\n");
		strcat(buffer, "<title>403 Forbidden</title><body>403 Forbidden</body>");
		break;

	case R_METHOD_NA:
		if(do_dbg)
			printf("- send_error_reply (R_METHOD_NA). \n");
		strcpy(buffer, "HTTP/1.0 405 Method Not Allowed\n\0");
		strcat(buffer, "Content-Type: text/html\n\n\0");
		strcat(buffer, "<title>405 Method Not Allowed</title><body>405 Method Not Allowed</body>\0");
		break;

	case R_NONE_ACC:
		if(do_dbg)
			printf("- send_error_reply (R_NONE_ACC). \n");
		strcpy(buffer, "HTTP/1.0 406 Not Acceptable\n\0");
		strcat(buffer, "Content-Type: text/html\n\n\0");
		strcat(buffer, "<title>406 Not Acceptable</title><body>406 Not Acceptable</body>\0");
		break;

	case R_NOT_FOUND:
		if(do_dbg)
			printf("- send_error_reply (R_NOT_FOUND). \n");
		strcpy(buffer, "HTTP/1.0 404 Not Found\r\n\0");
		strcat(buffer, "Content-Type: text/html\r\n\r\n\0");
		strcat(buffer, "<title>404 Not found</title><body>404 Not Found</body>\0");
		break;

	case R_NOT_IMP:
		if(do_dbg)
			printf("- send_error_reply (R_NOT_IMP). \n");
		buffer[0] = '\0'; /* empty message */
		break;

	case R_ERROR:
		if(do_dbg)
			printf("- send_error_reply (R_ERROR). \n");
		buffer[0] = '\0'; /* empty message */
		break;

	case R_SILENT:
		if(do_dbg)
			printf("- send_error_reply (R_SILENT). \n");
		break;

	default:
		if(do_dbg)
			printf("- send_error_reply (default). \n");
		buffer[0] = '\0'; /* empty message */
		break;
	}

	if(send(req->fd, buffer, strlen(buffer), 0) == ERROR)
	{
		// free buffer
		free (buffer);
		return (ERROR);
	}

	// free buffer
	free (buffer);
	return (OK);
}

/*----------------------------------------------------------------------
* ROUTINE NAME - http_recv
*-----------------------------------------------------------------------
* DESCRIPTION: 
*   Non-blocking version of "receive".

*
* INPUT:
*   s       -- Socket.
*   buf     -- Buffer.
*   len     -- How many to receive.
*   flags   -- Passed to "recv".
*
* RETURN:    
*   -1: Error.
*  >=0: How many bytes recieved.
*----------------------------------------------------------------------*/
int http_recv (int s, char *buf, int len, int flags)
{
#if APP_AGENTD_HAS_SSL
	if (do_ssl)
		return http_ssl_read(buf, len);
	else
#endif
		return recv(s, buf, len, flags);
}
/*----------------------------------------------------------------------
* ROUTINE NAME - httpd_process
*-----------------------------------------------------------------------
* DESCRIPTION:
*       This routine reads the request from the client into  the request
*       structure.  If it is a POST request, check whether it is a 
*       multi-part request.  For a multi-part request, let CGI process
*       the upload request.  Otherwise read body of the data and save
*       in the env variable "QUERY_STRING".
*           
*
*                   parse_request
*                       |
*                   read_header
*                       |               No
*                    ?M_POST -----------------------+----> END
*                       |       Yes                 |
*                  ?Multi-part -------> Set flag ---+
*                       |
*                   read file

*                       |
*               save in "QUERY_STRING"
*                       |
*                      END
*
*            
* INPUT:    req     -- requst structure to handle
* OUTPUT:
* RETURN:    
*----------------------------------------------------------------------*/
static int https_process(HTTPS_CB *pcb)
{
	char *pContentType;
	char *pMulti = "multipart/form-data"; 
	int  multi_len = strlen(pMulti); 
	int  i;

	int  cnt = 0;
	int  rc = 0;
	char *pPostText;
	int  nAsk;
	int  length;

	/* read header data */
	if(read_header(pcb) == ERROR)
	{
		printf("[app_agentd] read_header return error! ");
		return (ERROR);
	}

	/* parse header data */
	if(parse_header(pcb) == ERROR)
	{
		printf("[app_agentd] parse_header return error! ");
		return (ERROR);
	}

	/* parse the POST request */
	if(pcb->method == M_POST)
	{
		/*******************************************
		  Process the multipart request
		  Suemi, added for multipart--file upload---
		********************************************/
		pcb->multipart = 0;
		pContentType = get_env (&pcb->envcfg, "CONTENT_TYPE");   

		/* Extra check for ContentType */
		if (!pContentType)
		{
			// TRACE(MSG_DBG,"ContentType return error!\n");
			return (ERROR);
		}

#if 1   // 2011-03-18 Norkay, fix Chrome crash
		if(strlen(pContentType) >= multi_len)
		{
			if(strstr(pContentType, pMulti) != 0)
			{
				// proccess multi-part file
				pcb->multipart = 1;
				return OK;
			}
		}
#else 
		for(i=0; i<= (strlen(pContentType) - multi_len) ; i++)
		{
			if(memcmp(pContentType + i, pMulti, multi_len) == 0)
			{
				// proccess multi-part file
				pcb->multipart = 1;
				return OK;
			}
		}
#endif

		/*******************************************
		  Not multipart, read the file to POST
		 *******************************************/
		length = atoi(get_env(&pcb->envcfg,"CONTENT_LENGTH"));
		if(length > HTTP_REQ_POSTSIZE - 1)
			length = HTTP_REQ_POSTSIZE - 1;

		/* part of the body may already be in req->header_in */
		cnt = pcb->body_cnt;
		if(cnt > length)
			cnt = length;

		// allocate variable
		if((pPostText = (char *)malloc (HTTP_REQ_POSTSIZE)) == 0)
		{
			pcb->status = R_ERROR;
			return (ERROR);
		}

		/* copy to post-text */
		memset (pPostText, '0', sizeof(pPostText));
		memcpy (pPostText, &pcb->header_in[pcb->body_index], cnt);

#ifdef DEBUG
		printf("[app_agentd] RECV from Phone \n");
#ifdef XML_DEBUG
		printf("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
		printf("%s\n\n", pPostText);
#endif
#endif
		if(do_dbg)
			printf("[app_agentd] RECV from Phone \n");

		/* Read the length assigned by "CONTENT_LENGTH" */
		while(cnt < length)
		{
			nAsk = length - cnt;
			if(nAsk > MAX_READ)
				nAsk = MAX_READ;

			if(cnt + nAsk > HTTP_REQ_POSTSIZE - 1)
				nAsk = HTTP_REQ_POSTSIZE - 1 - cnt;

			rc = http_recv(pcb->fd, pPostText + cnt, nAsk, 0);
			if(rc == ERROR)
			{
				pcb->status = R_BAD_REQUEST;
				free (pPostText);
				printf("http_recv return error!\n");
				return (ERROR);
			}

			/* update index */
			cnt += rc;
		}

		/* end of string (cnt = length) */
		pPostText [cnt] = '\0';

		if(do_dbg)
		{
			printf("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
			printf("%s\n\n", pPostText);
			printf("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
		}
#if HAS_ENCRYPT_PAYLOAD
        if(do_encrypt_payload && cnt > 0)
        {
            char *plaintext, *ciphertext = NULL;
            int plainlen, cipherlen;
            ENCRYPT_DEBUG2("try to de-base64 data...(%d)",cnt);
            if((ciphertext = base64_decode(pPostText, cnt, &cipherlen)) == NULL)
            {
                printf("%s: de-base64 data failed!\n",__FUNCTION__);
                free (pPostText);
                return (ERROR);
            }
            ENCRYPT_DEBUG2("try to decrypt data...(%d)",cipherlen);
            if((plaintext = decrypt_data(ciphertext, cipherlen, &plainlen)) == NULL)
            {
                printf("%s: decrypt data failed!\n",__FUNCTION__);
                if(ciphertext != NULL)
                    free(ciphertext);
                free (pPostText);
                return (ERROR);
            }
            else
            {
                char *body_begin, *body_end;
                ENCRYPT_DEBUG("get plaintext (%d):",plainlen);
                // CKC: a quick check about validity of result plain text
                if(plainlen > 0)
                {
                    body_begin = strchr(plaintext, '{');
                    body_end = strchr(plaintext, '}');
                }
                if(plainlen > 0 && (body_begin == NULL || body_end == NULL || body_begin > body_end))
                {
                    printf("%s: illegal JSON format, clean up now...\n",__FUNCTION__);
                    strcpy(plaintext,"{}");
                    plaintext[2] = '\0';
                    plainlen = 2;
                }
                else if(plainlen >= HTTP_REQ_POSTSIZE) // CKC: in fact, this is impossible...
                {
                    char* newtext = (char*)realloc(pPostText, (plainlen+1)*sizeof(char));
                    if(newtext == NULL)
                    {
                        printf("%s: realloc post text (%d) failed!\n",__FUNCTION__,plainlen+1);
                        if(ciphertext != NULL)
                            free(ciphertext);
                        free (pPostText);
                        return (ERROR);
                    }
                    pPostText = newtext;
                }
                strncpy(pPostText,plaintext,plainlen);
                pPostText[plainlen] = '\0';
                ENCRYPT_DEBUG("%s\n", pPostText);
            }
            if(ciphertext != NULL)
                free(ciphertext);
        }
#endif // HAS_ENCRYPT_PAYLOAD

		set_env (&pcb->envcfg, "QUERY_STRING", pPostText);
		free (pPostText);
	}

	return (OK);
}

/*----------------------------------------------------------------------
* ROUTINE NAME - http_send
*-----------------------------------------------------------------------
* DESCRIPTION: 
*   Non-blocking version of "send".
*
* INPUT:
*   s       -- Socket.
*   buf     -- Buffer.
*   len     -- How many to send.
*   flags   -- Passed to "send".
*
* RETURN:    
*   -1: Error.
*  >=0: How many bytes sent.
*----------------------------------------------------------------------*/
int http_send(int s, char *buf, int len, int flags)
{
	/* mook 2009-0407: support https */
#if APP_AGENTD_HAS_SSL
	if (do_ssl)
	{
#ifdef DEBUG
		printf("[app_agentd] https_send \n");
#endif
		return http_ssl_write(buf, len);
	}
	else
#endif
	{
#ifdef DEBUG
		printf("[app_agentd] http_send \n");
#endif
		return send(s, buf, len, flags);
	}
}

/*----------------------------------------------------------------------
* ROUTINE NAME - https
*-----------------------------------------------------------------------
* DESCRIPTION: This routine processes the client request.
*
*                     https
*                       |
*                       |
*                   malloc(request)
*                   init_env

*                       |
*                       |
*                       |
*                  https_process
*                     /    \
*                    /      \
*                   /        \

*                  /          \
*      (*pHttpCgiMain)()   send_error_reply
*                  \          /
*                   \        /
*                    \      /
*                     \    /
*                    free_env
*                   free(request)
*            
* INPUT:    int s   -- socket to read the HTTP data
* OUTPUT:
* RETURN:    
*----------------------------------------------------------------------*/
void https (int s)
{
	HTTPS_CB    *pcb;

	//printf("https gets socket %d\n",s);
	/*******************************
	 Initialize the request context
	*******************************/
	if((pcb = (HTTPS_CB *)malloc(sizeof(HTTPS_CB))) == 0)
		return;

	memset(pcb, 0, sizeof(HTTPS_CB));

	init_env(&pcb->envcfg);
	pcb->fd = s;

	/*******************************
	  Parse the request
	*******************************/
	if(https_process(pcb) == OK)
	{
		char    *pFirstPkt = 0;
		int     pkt_len = 0;
		int     len;
		if(pcb->multipart == 1)
		{
			/* part of the body may already be in req->header_in */ 
			pkt_len = pcb->body_cnt;
			len = atoi(get_env(&pcb->envcfg, "CONTENT_LENGTH"));
			if(pkt_len > len)
				pkt_len = len;

			pFirstPkt = &pcb->header_in [pcb->body_index];
		}

/* if packet is JSON protocol, pass to the JSON handler. */
		if(pcb->json)
		{
#if 0 //for debug, print login array table data.
	int i;
	UINT32 has_login_ip, has_login_ip_time;
	for(i=0; i < NUM_APPAGENT_CONCURENT_LOGIN; i++)
    {
#if 1//SUPPORT_IPV6_SETTING
		AdminCfg_GetHasLoginIp(has_login_ip ,i);
#else
		AdminCfg_GetHasLoginIp(&has_login_ip ,i);
		AdminCfg_GetMultiLoginTime(&has_login_ip_time, i);
#endif
    }
#endif
			app_agent_process(s, pcb);
			goto HTTPS_END;
		}
		else
		{
			/* If packet's prototype is not HNAP, ignore it. */
#ifdef DEBUG
			printf("jerry debug ---> Packet's prototype is not HNAP/JSON, ignore it. \n");
#endif
			goto HTTPS_END;
		}
	}
	else
	{
		printf("send_error_reply. \n");
		send_error_reply(pcb);
	}

	/*******************************
	  Free the request context
	*******************************/

HTTPS_END:

	free_env(&pcb->envcfg);
	free (pcb);
	return;
}

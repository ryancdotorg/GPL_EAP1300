/*******************************************************************************************
 * Copyright (c) 2018 Ryan Hsu @ SenaoNetworks - Taiwan                                    *
 *                                                                                         *
 * This code has to be used in SenaoNetworks internally                                    *
 * Unauthorized copying of this file, via any medium is strictly prohibite                 *
 * Proprietary and confidential                                                            *
 * --------------------------------------------------------------------------------------- *
 * Project:  syseye                                                                        *
 *                                                                                         *
 * Author :  Ryan Hsu (ryan_hsu@hotmail.com.tw)                                            *
 *******************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include "libseipc.h"

static char sepkt_format[] =
"{"
"    \"Header\": {"
"        \"source\": \"\","
"        \"type\": 0"
"    },"
"    \"Data\": {}"
"}";

/* debug is disabled by default. */
static int debug = 0;
static FILE *debug_fp=NULL;

static int 
seipc_set_unix_conn(char *path)
{
    struct sockaddr_un address;
    int sockfd;
    size_t size;

    if (!path)
        return -1;

    if ( (sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1 ) {
		perror("socket:");
        return -1;
	}

    memset(&address, 0, sizeof(struct sockaddr_un));
    address.sun_family = AF_UNIX;
    snprintf(address.sun_path, sizeof(address.sun_path), "%s", path);

    size = (offsetof (struct sockaddr_un, sun_path)
            + strlen (address.sun_path));

    if (connect(sockfd, (struct sockaddr *)&address, size)<0){
        close(sockfd);
        return -1;
    }
    return sockfd;
}

static int seipc_set_inet_conn(char dest_ip, int dest_port)
{
    // TODO
}

static int 
seipc_pkt_send(int sockfd, JsonNode *pkt)
{
    unsigned char header[1024], chunkhdr[32], tail[32];
    int len1=0, len2=0, pktlen;
    char *pktstr;

    if(!pkt)
        return -1;

    if (debug){
        fprintf(debug_fp, ">>>>> seipc request\n");
        js_to_fp_hr(pkt, debug_fp);
        fprintf(debug_fp, "\n");
    }
    pktstr = js_to_str(pkt);
    pktlen = strlen(pktstr);

    len1 += snprintf(header, sizeof(header), "Transfer-Encoding: chunked\r\nContent-Type: plain/text\r\n\r\n");
    len1 += snprintf(chunkhdr, sizeof(chunkhdr), "%x\r\n", pktlen);
    len1 += pktlen;
    len1 += snprintf(tail, sizeof(tail), "\r\n0\r\n\r\n");

    len2 += send(sockfd, header, strlen(header), 0);
    len2 += send(sockfd, chunkhdr, strlen(chunkhdr), 0);
    len2 += send(sockfd, pktstr, pktlen, 0);
    len2 += send(sockfd, tail, strlen(tail), 0);
    free(pktstr);
    if (len1 != len2){
        return -1;
    }
    return len2;
}

/* Search string */
static unsigned char*
_memfind(unsigned char *haystack, int hsize, unsigned char *needle, int nsize)
{
	unsigned char *i;
	for (i = haystack; i < haystack + hsize - nsize + 1; i++) {
		if (!memcmp(i, needle, nsize))
			return i;
	}
	return NULL;
}

#define BUFSIZE	1024

static JsonNode*
seipc_pkt_recv(int sockfd)
{
	/* For first part */
	int ret=0, total=0, rblen=0;
	unsigned char *rcvbuf, tmpbuf[BUFSIZE];
	/* For second part */
	int idx=0, len=0, odd=0, idx2=0;
	unsigned char *jsbuf, *head, *tail;

	JsonNode *gx, *tmpgx;


	/* First: Receive the response. */
	rcvbuf = calloc(1, BUFSIZE);
	if (!rcvbuf)
		return NULL;

	rblen += BUFSIZE;

	while ((ret = read(sockfd, tmpbuf, sizeof(tmpbuf))) > 0) {
		memcpy(rcvbuf+total, tmpbuf, ret);
		total+=ret;
		if (_memfind(rcvbuf, total, "0\r\n\r\n", 5) != NULL)
			break;

		rblen += BUFSIZE;
		rcvbuf = realloc(rcvbuf, rblen);
		if (!rcvbuf)
			return NULL;
	}

        /* Second: Handle this rcvbuf. Reassemble the json response. */
	head = tail = rcvbuf;
	jsbuf = calloc(1, rblen);
	if (!jsbuf) {
		free(rcvbuf);
		return NULL;
	}

	while ((tail=_memfind(rcvbuf+idx, total, "\r\n", 2)) != NULL) {
		/* If parsing completes, break! */
		if (memcmp(tail, "\r\n\r\n", 4) == 0)
			break;
		len = tail-head+2;
		total -= len;
		idx += len;
		/* In our method, we don't care the chunk size. */
		odd++;
		if (odd%2!=0) {
			head = rcvbuf+idx;
			continue;
		}
		memcpy(jsbuf+idx2, head, tail-head);
		idx2 += tail-head;
		head = rcvbuf+idx;
	}

	/* Third: Parse the json response. */
	tmpgx = js_parse_str(jsbuf);
	if (!tmpgx) {
		printf("Cannot get the response from syseye\n");
	} else {
		gx = js_dup(tmpgx);
		js_free(tmpgx);
	}
	free(rcvbuf);
	free(jsbuf);
	if (gx) {
        if (debug){
            fprintf(debug_fp, ">>>>> seipc response\n");
            js_to_fp_hr(gx, debug_fp);
            fprintf(debug_fp, "\n");
        }
		return gx;
	}
	else
		return NULL;
}

void
seipc_debug_on(FILE *fp)
{
	debug = 1;
	debug_fp = fp;
}

void
seipc_debug_off(void)
{
	debug = 0;
	debug_fp = NULL;
}

struct seipc_t*
seipc_create(char *source, char *path)
{
    struct seipc_t *handle;
    int sockfd;

    if ((sockfd = seipc_set_unix_conn(path)) < 0) 
		return NULL;
	
	if (!(handle= malloc(sizeof(struct seipc_t)))) {
		close(sockfd);
		return NULL;
	}
	handle->sockfd = sockfd;
	handle->source = strdup(source);
	handle->result = NULL;
	return handle;
}

void 
seipc_close(struct seipc_t *handle)
{
	if (!handle)
		return;
	close(handle->sockfd);
	free(handle->source);
	if (handle->result)
		free(handle->result);
	free(handle);
}

int 
seipc_set_blk(struct seipc_t *handle, JsonNode *data)
{
	int ret=-1, len;
	JsonNode *reqpkt, *respkt;
	char fmt[512];

	if (!handle || !data)
		return -1;
	
	snprintf(fmt, sizeof(fmt), "%s", sepkt_format);
	reqpkt = js_parse_str(fmt);
	js_set_path_str(reqpkt, "Header/source", handle->source);
	js_set_path_int(reqpkt, "Header/type", SEPKT_SET);

	js_union(js_get_path(reqpkt, "Data"), data);
    //js_print_hr(reqpkt);
    //js_to_fp_hr(reqpkt, stderr);
	len = seipc_pkt_send(handle->sockfd, reqpkt);
	if (len < 0)
		goto failure;
	respkt = seipc_pkt_recv(handle->sockfd);
	if (!respkt) 
		goto failure;
	
	ret = js_get_path_int(respkt, "Data/Response/status");
	js_free(respkt);
failure:
	js_free(reqpkt);
	return (ret==0)?0:-1;
}

int 
seipc_set_str(struct seipc_t *handle, char *path, char *value)
{
	int ret=-1, len;
	JsonNode *reqpkt, *respkt;
	char fmt[512];
	
	if (!handle || !path || !value)
		return -1;

	snprintf(fmt, sizeof(fmt), "%s", sepkt_format);
	reqpkt = js_parse_str(fmt);

    js_set_path_str(reqpkt, "Header/source", handle->source);
    js_set_path_int(reqpkt, "Header/type", SEPKT_SET);
    js_idx_set_path_str(js_set_path(reqpkt, "Data"), path, value);

	len = seipc_pkt_send(handle->sockfd, reqpkt);
	if (len < 0) 
		goto failure;
	respkt = seipc_pkt_recv(handle->sockfd);
	if (!respkt)
		goto failure;

	ret = js_get_path_int(respkt, "Data/Response/status");
	js_free(respkt);
failure:
	js_free(reqpkt);
	return (ret==0)?0:-1;
}

int 
seipc_set_int(struct seipc_t *handle, char *path, int value)
{
	int ret=-1, len;
	JsonNode *reqpkt, *respkt;
	char fmt[512];

	if (!handle || !path) 
		return -1;

	snprintf(fmt, sizeof(fmt), "%s", sepkt_format);
	reqpkt = js_parse_str(fmt);
	js_set_path_str(reqpkt, "Header/source", handle->source);
	js_set_path_int(reqpkt, "Header/type", SEPKT_SET);
    js_idx_set_path_int(js_set_path(reqpkt, "Data"), path, value);

	len = seipc_pkt_send(handle->sockfd, reqpkt);
	if (len < 0) 
		goto failure;

	respkt = seipc_pkt_recv(handle->sockfd);

	if (!respkt) 
		goto failure;

	ret = js_get_path_int(respkt, "Data/Response/status");
	js_free(respkt);
failure:
	js_free(reqpkt);
	return (ret==0)?0:-1;
}

JsonNode * 
seipc_get_blk(struct seipc_t *handle, char *path)
{
	int len;
	JsonNode *reqpkt, *respkt, *resdata=NULL;
	char fmt[512];

	if (!handle || !path)
		return NULL;

	if (handle->result) {
		free(handle->result);
		handle->result = NULL;
	}
	
	snprintf(fmt, sizeof(fmt), "%s", sepkt_format);
	reqpkt = js_parse_str(fmt);
	js_set_path_str(reqpkt, "Header/source", handle->source);
	js_set_path_int(reqpkt, "Header/type", SEPKT_GET);
	js_set_path(js_get_path(reqpkt, "Data"), path);
	
	len = seipc_pkt_send(handle->sockfd, reqpkt);
	if (len < 0)
		goto failure;
	respkt = seipc_pkt_recv(handle->sockfd);
	if (!respkt)
		goto failure;

	resdata = js_dup(js_get_path(js_get_path(respkt, "Data"), path));
	
	js_free(respkt);
failure:
	js_free(reqpkt);
	return resdata;
}

char* 
seipc_get_str(struct seipc_t *handle, char *path)
{
	int len;
	JsonNode *reqpkt, *respkt;
	char fmt[512];

	if (!handle || !path)
		return NULL;

	if (handle->result) {
		free(handle->result);
		handle->result = NULL;
	}

	snprintf(fmt, sizeof(fmt), "%s", sepkt_format);
	reqpkt = js_parse_str(fmt);
	js_set_path_str(reqpkt, "Header/source", handle->source);
	js_set_path_int(reqpkt, "Header/type", SEPKT_GET);
	js_set_path(js_get_path(reqpkt, "Data"), path);
	
	len = seipc_pkt_send(handle->sockfd, reqpkt);
	if (len < 0)
		goto failure;
	respkt = seipc_pkt_recv(handle->sockfd);

	if (!respkt)
		goto failure;

	if (js_get_path_str(js_get_path(respkt, "Data"), path)) {
		handle->result = strdup(js_get_path_str(js_get_path(respkt, "Data"), path));
	}

	js_free(respkt);
failure:
	js_free(reqpkt);
	return handle->result;

}
struct seipc_t *
seipc_get_value(struct seipc_t *handle, char *path)
{
	int len;
	JsonNode *reqpkt, *respkt, *t;
	char fmt[512];

	if (!handle || !path)
		return NULL;

	if (handle->result) {
		free(handle->result);
		handle->result = NULL;
	}

	snprintf(fmt, sizeof(fmt), "%s", sepkt_format);
	reqpkt = js_parse_str(fmt);
	js_set_path_str(reqpkt, "Header/source", handle->source);
	js_set_path_int(reqpkt, "Header/type", SEPKT_GET);
	js_idx_set_path(js_get_path(reqpkt, "Data"), path);
	
	len = seipc_pkt_send(handle->sockfd, reqpkt);
	if (len < 0)
		goto failure;
	respkt = seipc_pkt_recv(handle->sockfd);
	if (!respkt)
		goto failure;
    
	t = js_idx_get_path(js_get_path(respkt, "Data"), path);
    if (!t)
        goto failure;
    handle->type = t->tag;
    switch(t->tag)
    {
		case ezJSON_BOOL:
            handle->result_number = t->bool_;
            break;
		case ezJSON_STRING:
            handle->result = strdup(t->string_);
            break;
		case ezJSON_NUMBER:
            handle->result_number = t->number_;
            break;
		case ezJSON_NULL:
		case ezJSON_ARRAY:
		case ezJSON_OBJECT:
		default:
            handle = NULL;
            break;
    }
	js_free(respkt);
failure:
	js_free(reqpkt);
	return handle;
}

int 
seipc_get_num_array(struct seipc_t *handle, char *path)
{
	int len, count = -1;
	JsonNode *reqpkt, *respkt, *t;
    char *p =  NULL, *e = NULL, *r = NULL;
	char fmt[512];
    char tmp[64];

	if (!handle || !path)
		return JS_FAIL;

	if (handle->result) {
		free(handle->result);
		handle->result = NULL;
	}
    // check path format, shoule be abc[]
    if ( (e = strchr(path, '['))==NULL || strchr(path, ']') == NULL){
        return JS_FAIL;
    }

	snprintf(fmt, sizeof(fmt), "%s", sepkt_format);
	reqpkt = js_parse_str(fmt);
	js_set_path_str(reqpkt, "Header/source", handle->source);
	js_set_path_int(reqpkt, "Header/type", SEPKT_GET_NUM_ARRAY);
	js_idx_set_path(js_get_path(reqpkt, "Data"), path);
	len = seipc_pkt_send(handle->sockfd, reqpkt);
	if (len < 0)
		goto failure;
	respkt = seipc_pkt_recv(handle->sockfd);
	if (!respkt)
		goto failure;

    // if query array is abc, the result would be num_abc in same layer
    // find path and the leaf node, to format num_abc
    p = path;
    if (e != NULL){
        *e = '\0';
    }
    e = strrchr(p, '/');
    if (e != NULL){
        r = e+1;
        *e = '\0';
        snprintf(tmp, sizeof(tmp), "%s/num_%s", p, r);
    }
    else{
        r = p;
        snprintf(tmp, sizeof(tmp), "num_%s", r);
    }

	count = js_idx_get_path_int(js_get_path(respkt, "Data"), tmp);
    if (count == JS_FAIL)
        goto failure;
    handle->type = ezJSON_NUMBER;
    handle->result_number = count;

	js_free(respkt);
failure:
	js_free(reqpkt);
	return count;
}

int 
seipc_reconf(struct seipc_t *handle, char *path)
{
	int ret=-1, len;
	JsonNode *reqpkt, *respkt;
	char fmt[512];

	if (!handle)
		return -1;

	snprintf(fmt, sizeof(fmt), "%s", sepkt_format);
	reqpkt = js_parse_str(fmt);
	js_set_path_str(reqpkt, "Header/source", handle->source);
	js_set_path_int(reqpkt, "Header/type", SEPKT_RECONF);
    // TODO: handle section here, check path
	js_set_path(js_get_path(reqpkt, "Data"), path);

	len = seipc_pkt_send(handle->sockfd, reqpkt);
	if (len < 0)
		goto failure;
	respkt = seipc_pkt_recv(handle->sockfd);
	if (!respkt)
		goto failure;

	ret = js_get_path_int(respkt, "Data/Response/status");
	js_free(respkt);
failure:
	js_free(reqpkt);
	return (ret==0)?0:-1;
}

int
seipc_commit(struct seipc_t *handle, char *path)
{
	int ret=-1, len;
	JsonNode *reqpkt, *respkt;
	char fmt[512];

	if (!handle)
		return -1;

	snprintf(fmt, sizeof(fmt), "%s", sepkt_format);
	reqpkt = js_parse_str(fmt);
	js_set_path_str(reqpkt, "Header/source", handle->source);
	js_set_path_int(reqpkt, "Header/type", SEPKT_COMMIT);
    // TODO: handle section here, check path
	js_set_path(js_get_path(reqpkt, "Data"), path);

	len = seipc_pkt_send(handle->sockfd, reqpkt);
	if (len < 0)
		goto failure;
	respkt = seipc_pkt_recv(handle->sockfd);
	if (!respkt)
		goto failure;

	ret = js_get_path_int(respkt, "Data/Response/status");
	js_free(respkt);
failure:
	js_free(reqpkt);
	return (ret==0)?0:-1;
}

int
seipc_prune(struct seipc_t *handle, char *path)
{
	int ret=-1, len;
	JsonNode *reqpkt, *respkt;
	char fmt[512];

	if (!handle || !path)
		return -1;

	snprintf(fmt, sizeof(fmt), "%s", sepkt_format);
	reqpkt = js_parse_str(fmt);
	js_set_path_str(reqpkt, "Header/source", handle->source);
	js_set_path_int(reqpkt, "Header/type", SEPKT_PRUNE);
	js_set_path(js_get_path(reqpkt, "Data"), path);

	len = seipc_pkt_send(handle->sockfd, reqpkt);
	if (len < 0)
		goto failure;
	respkt = seipc_pkt_recv(handle->sockfd);
	if (!respkt)
		goto failure;

	ret = js_get_path_int(respkt, "Data/Response/status");
	js_free(respkt);
failure:
	js_free(reqpkt);
	return (ret==0)?0:-1;
}

int 
seipc_prune_first(struct seipc_t *handle, char *path)
{
	int ret=-1, len;
	JsonNode *reqpkt, *respkt;
	char fmt[512];

	if (!handle || !path)
		return -1;

	snprintf(fmt, sizeof(fmt), "%s", sepkt_format);
	reqpkt = js_parse_str(fmt);
	js_set_path_str(reqpkt, "Header/source", handle->source);
	js_set_path_int(reqpkt, "Header/type", SEPKT_PRUNE_FIRST_ELEMENT);
	js_set_path(js_get_path(reqpkt, "Data"), path);

    //js_print_hr(reqpkt);
	len = seipc_pkt_send(handle->sockfd, reqpkt);
	if (len < 0)
		goto failure;
	respkt = seipc_pkt_recv(handle->sockfd);
	if (!respkt)
		goto failure;

	ret = js_get_path_int(respkt, "Data/Response/status");
	js_free(respkt);
failure:
	js_free(reqpkt);
	return (ret==0)?0:-1;
}

int
seipc_update_elem(struct seipc_t *handle, char *path)
{
	int ret=-1, len;
	JsonNode *reqpkt, *respkt;
	char fmt[512];

	if (!handle || !path)
		return -1;

	snprintf(fmt, sizeof(fmt), "%s", sepkt_format);
	reqpkt = js_parse_str(fmt);
	js_set_path_str(reqpkt, "Header/source", handle->source);
	js_set_path_int(reqpkt, "Header/type", SEPKT_UPDATE_ELEMENT);
	js_set_path(js_get_path(reqpkt, "Data"), path);

    //js_print_hr(reqpkt);
	len = seipc_pkt_send(handle->sockfd, reqpkt);
	if (len < 0)
		goto failure;
	respkt = seipc_pkt_recv(handle->sockfd);
	if (!respkt)
		goto failure;

	ret = js_get_path_int(respkt, "Data/Response/status");
	js_free(respkt);
failure:
	js_free(reqpkt);
	return (ret==0)?0:-1;
}

int
seipc_treeview(struct seipc_t *handle, char *path)
{
	int ret=-1, len;
	JsonNode *reqpkt, *respkt;
	char fmt[512];

	if (!handle || !path)
		return -1;
	
	snprintf(fmt, sizeof(fmt), "%s", sepkt_format);
	reqpkt = js_parse_str(fmt);
	js_set_path_str(reqpkt, "Header/source", handle->source);
	js_set_path_int(reqpkt, "Header/type", SEPKT_TREEVIEW);
	js_set_path(js_get_path(reqpkt, "Data"), path);

	len = seipc_pkt_send(handle->sockfd, reqpkt);
	if (len < 0)
		goto failure;
	respkt = seipc_pkt_recv(handle->sockfd);
	if (!respkt)
		goto failure;

	ret = js_get_path_int(respkt, "Data/Response/status");
	js_free(respkt);
failure:
	js_free(reqpkt);
	return (ret==0)?0:-1;
}

JsonNode *
seipc_action(struct seipc_t *handle, char *name, char *command)
{
	int 		ret=-1, len;
	JsonNode *reqpkt, *respkt, *gx=NULL;
	char 		fmt[512];
#define RBUF_SIZE 1024
	
	if (!handle || !name || !command)
		return NULL;

	snprintf(fmt, sizeof(fmt), "%s", sepkt_format);
	reqpkt = js_parse_str(fmt);
	js_set_path_str(reqpkt, "Header/source", handle->source);
	js_set_path_int(reqpkt, "Header/type", SEPKT_ACTION);
	gx = js_set_path(js_get_path(reqpkt, "Data"), "Action");
	
	js_set_path_str(gx, "name", name);
	js_set_path_str(gx, "command", command);

	len = seipc_pkt_send(handle->sockfd, reqpkt);
	if (len < 0) {
		js_free(reqpkt);
		return NULL;
	}
    //js_print(reqpkt);
	respkt = seipc_pkt_recv(handle->sockfd);
	if (!respkt) {
		js_free(reqpkt); 
		return NULL;
	}
//    printf("get:[\n");
//	js_print(respkt);
//    printf("]\n");
	gx = js_dup(js_get_path(respkt, "Data/Action"));

	js_free(respkt);
	js_free(reqpkt);
	return gx;
}

int
seipc_listaction(struct seipc_t *handle)
{
	int ret=-1, len;
	JsonNode *reqpkt, *respkt;
	char fmt[512];

	if (!handle)
		return -1;

	snprintf(fmt, sizeof(fmt), "%s", sepkt_format);
	reqpkt = js_parse_str(fmt);
	js_set_path_str(reqpkt, "Header/source", handle->source);
	js_set_path_int(reqpkt, "Header/type", SEPKT_LISTACTION);

	len = seipc_pkt_send(handle->sockfd, reqpkt);
	if (len < 0)
		goto failure;
	respkt = seipc_pkt_recv(handle->sockfd);
	if (!respkt)
		goto failure;

	ret = js_get_path_int(respkt, "Data/Response/status");
	js_free(respkt);
failure:
	js_free(reqpkt);
	return (ret==0)?0:-1;
}

JsonNode *
seipc_actions(struct seipc_t *handle, JsonNode *json)
{
	int 		ret=-1, len;
	JsonNode *reqpkt, *respkt, *gx=NULL;
	char 		fmt[512];
#define RBUF_SIZE 1024

	if (!handle || !json)
		return NULL;

	snprintf(fmt, sizeof(fmt), "%s", sepkt_format);
	reqpkt = js_parse_str(fmt);
	js_set_path_str(reqpkt, "Header/source", handle->source);
	js_set_path_int(reqpkt, "Header/type", SEPKT_ACTIONS);
	gx = js_get_path(reqpkt, "Data");
	js_join(gx, json);

	len = seipc_pkt_send(handle->sockfd, reqpkt);
	if (len < 0) {
		js_free(reqpkt);
		return NULL;
	}
//    js_print(reqpkt);
	respkt = seipc_pkt_recv(handle->sockfd);
	if (!respkt) {
		js_free(reqpkt);
		return NULL;
	}
//    printf("get:[\n");
//	js_print(respkt);
//    printf("]\n");
	gx = js_dup(js_get_path(respkt, "Data/Actions"));

	js_free(respkt);
	js_free(reqpkt);
	return gx;
}

int seipc_send_json(struct seipc_t *handle, JsonNode *json)
{
	int ret=-1, len;
	JsonNode *respkt;
	if (!handle || !json)
		return -1;

	len = seipc_pkt_send(handle->sockfd, json);
	if (len  < 0)
		goto failure;
	respkt = seipc_pkt_recv(handle->sockfd);
	if (!respkt)
		goto failure;

	ret = js_get_path_int(respkt, "Data/Response/status");
	js_free(respkt);
failure:
	return (ret==0)?0:-1;

}

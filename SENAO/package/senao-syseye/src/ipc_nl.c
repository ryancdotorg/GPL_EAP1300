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
#include <sys/socket.h>
#include <linux/netlink.h>
#include "global.h"
#include "ipc_nl.h"
#include "util.h"

#define NETLINK_SYSEYE NETLINK_USERSOCK
//#define NETLINK_SYSEYE 30
#define SENLC_REGISTER 17
#define MAX_PAYLOAD 1024

// private function
/**
 * check if the netlink message fits into the remaining bytes
 * @arg nlh             netlink message header
 * @arg remaining       number of bytes remaining in message stream
 */
int nlmsg_ok(const struct nlmsghdr *nlh, int remaining)
{
    return (remaining >= sizeof(struct nlmsghdr) &&
        nlh->nlmsg_len >= sizeof(struct nlmsghdr) &&
        nlh->nlmsg_len <= remaining);
}

/**
 * next netlink message in message stream
 * @arg nlh             netlink message header
 * @arg remaining       number of bytes remaining in message stream
 *
 * @returns the next netlink message in the message stream and
 * decrements remaining by the size of the current message.
 */
struct nlmsghdr *nlmsg_next(struct nlmsghdr *nlh, int *remaining)
{
    int totlen = NLMSG_ALIGN(nlh->nlmsg_len);
    *remaining -= totlen;
    return (struct nlmsghdr *) ((unsigned char *) nlh + totlen);
}

int nlmsg_datalen(struct nlmsghdr *hdr)
{
    return hdr->nlmsg_len - NLMSG_HDRLEN;
}

static void safe_mem_free(void **buf){
    if (buf!=NULL && *buf !=NULL){
        free(*buf);
        *buf = NULL;	
    }
}
#define safe_free(pointer) safe_mem_free((void **) &(pointer))

/**
 * nlmsg_msg_size - length of netlink message not including padding
 * @payload: length of message payload
 */
static inline int nlmsg_msg_size(int payload)
{
	return NLMSG_HDRLEN + payload;
}

/**
 * nlmsg_total_size - length of netlink message including padding
 * @payload: length of message payload
 */
static inline int nlmsg_total_size(int payload)
{
	return NLMSG_ALIGN(nlmsg_msg_size(payload));
}

/**
 * nlmsg_padlen - length of padding at the message's tail
 * @payload: length of message payload
 */
static inline int nlmsg_padlen(int payload)
{
	return nlmsg_total_size(payload) - nlmsg_msg_size(payload);
}

int nl_sock(void *data)
{
    nl_srv_data_t *srvdata;
    struct sockaddr_nl src_addr, dest_addr;
    struct nlmsghdr *nlh = NULL;
    struct iovec iov;
    struct msghdr msg;
    if (data == NULL){
        printf("create sock error\n");
        return -1;
    }
    srvdata = (nl_srv_data_t *) data;

    if ((srvdata->fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_SYSEYE)) < 0){
        perror("create socket:");
        return -1;
    }

    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = getpid(); /* kernel will input process pid, self pid */

    bind(srvdata->fd, (struct sockaddr *)&src_addr, sizeof(src_addr));
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.nl_family = AF_NETLINK;
    dest_addr.nl_pid = 0;   /* For Linux Kernel */
    dest_addr.nl_groups = 0; /* unicast */

    nlh = (struct nlmsghdr *)calloc(1, NLMSG_SPACE(MAX_PAYLOAD));
    nlh->nlmsg_len = NLMSG_LENGTH(sizeof(*nlh));
    nlh->nlmsg_type |= SENLC_REGISTER;
    nlh->nlmsg_pid = getpid();
    nlh->nlmsg_flags |= NLM_F_REQUEST;
    iov.iov_base = (void *)nlh;
    iov.iov_len = nlh->nlmsg_len;

    memset(&msg, 0, sizeof(msg));
    msg.msg_name = (void *)&dest_addr;
    msg.msg_namelen = sizeof(dest_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    sendmsg(srvdata->fd,&msg,0);
    safe_free(nlh);	    
    return srvdata->fd;
}

void nl_close(void *data)
{
	nl_srv_data_t *srvdata;
	if (data == NULL){
		printf("tcp_close, data not initialized\n");
	}
	srvdata = (nl_srv_data_t *) data;

	if (srvdata->fd > 0)
	{
		close(srvdata->fd);
	}

}

void nl_recv(void *srvdata)
{
    nl_srv_data_t *nl_srv_data;
    int n;
    int flags = 0;
    struct nlmsghdr *hdr = NULL;
    struct iovec iov;
    int sock_fd;
    struct sockaddr_nl nla = {0};
    int remaining_len = 0;
    struct msghdr msg = {
        .msg_name = (void *) &nla,
        .msg_namelen = sizeof(struct sockaddr_nl),
        .msg_iov = &iov,
        .msg_iovlen = 1,
        .msg_control = NULL,
        .msg_controllen = 0,
        .msg_flags = 0,
    };

    if (srvdata == NULL){
        printf("create sock error\n");
        return;
    }
    nl_srv_data = (nl_srv_data_t *) srvdata;
    sock_fd = nl_srv_data->fd;

    if (nl_srv_data->iov_len == 0)
        nl_srv_data->iov_len = getpagesize() * 4;

    iov.iov_len = nl_srv_data->iov_len;
    iov.iov_base = calloc(1, iov.iov_len);
    if(iov.iov_base == NULL) {
        sedbg("calloc fail!\n");
        perror("calloc:");
        return 0;
    }

    n = recvmsg(sock_fd, &msg, 0);
    //sedbg("recvmsg get %d bytes\n", n);
    if (!n){
        safe_free(iov.iov_base);
        return;
    }
    else if (n < 0){
        if (errno == EINTR){
            sedbg("recvmsg() return EINTR, should not happened, handled in mio\n");
            safe_free(iov.iov_base);
            return;
        } else if (errno = EAGAIN){
            sedbg("recvmsg() return EAGAIN, aborting\n");\
            safe_free(iov.iov_base);
            safe_free(nl_srv_data->nlmsg_buf);
            return;
        } else {
            sedbg("errno unknown: %d\n", errno);\
            safe_free(iov.iov_base);
            safe_free(nl_srv_data->nlmsg_buf);
            return; 
        }
    }

    if (iov.iov_len < n || msg.msg_flags & MSG_TRUNC) {
        /* Provided buffer is not long enough, enlarge it
         * and try again. */
        sedbg("provided buffer(%d) is not long enough(n:%d), enlarge it\n", iov.iov_len, n);
        nl_srv_data->iov_len *= 2;
        safe_free(iov.iov_base);
    } else if (flags != 0){
        /* Buffer is big enough, do the actual reading */
        sedbg("Buffer is big enough, do the actual reading, reset flag(%d) and read again\n", flags);
        flags = 0;
        safe_free(iov.iov_base);
        return;	
    }

    if (msg.msg_namelen != sizeof(struct sockaddr_nl)) {
        sedbg("errno: NNOADDR\n");	   
        safe_free(iov.iov_base);
        safe_free(nl_srv_data->nlmsg_buf);
        return;	   
    }
    // copy received data to nlsmsg_buf
    if (nl_srv_data->nlmsg_buflen > 0){
        nl_srv_data->nlmsg_buf = realloc(nl_srv_data->nlmsg_buf, nl_srv_data->nlmsg_buflen + n);
    } else {
        safe_free(nl_srv_data->nlmsg_buf);
        nl_srv_data->nlmsg_buf = calloc(1, n);

        if (nl_srv_data->nlmsg_buf == NULL){
            //perror("malloc/realloc:");
            sedbg("malloc/realloc fail!\n");
            safe_free(iov.iov_base);
            safe_free(nl_srv_data->nlmsg_buf);
	    return;
        }
    }
    memcpy(nl_srv_data->nlmsg_buf + nl_srv_data->nlmsg_buflen, iov.iov_base, n);
    nl_srv_data->nlmsg_buflen += n;

    safe_free(iov.iov_base);
    // handle nlmsg
    //sedbg("recvmsgs: Read %d bytes\n", nl_srv_data->nlmsg_buflen);
    for (hdr = (struct nlmsghdr *) nl_srv_data->nlmsg_buf, 
                remaining_len = nl_srv_data->nlmsg_buflen;
        nlmsg_ok(hdr, remaining_len);
	hdr = nlmsg_next(hdr, &remaining_len)) {

//        sedbg("len: %d, type:%d, flag:%d seq:%d  pid:%d datalen:%d remaining_len:%d DATA:[%s]\n",
//        hdr->nlmsg_len, hdr->nlmsg_type, hdr->nlmsg_flags, hdr->nlmsg_seq, hdr->nlmsg_pid, nlmsg_datalen(hdr), remaining_len, NLMSG_DATA(hdr));
//        sedbg("nlmsg_total_size:%d nlmsg_msg_size:%d nlmsg_pad_size:%d \n", nlmsg_total_size(nlmsg_datalen(hdr)), hdr->nlmsg_len, nlmsg_padlen(nlmsg_datalen(hdr)));
        // check sequence number
        if (hdr->nlmsg_seq != nl_srv_data->seq){
            sedbg("sequence number mismitch! drop packet!!\n"); 
            safe_free(nl_srv_data->nlmsg_buf);
            nl_srv_data->nlmsg_buflen = 0;
            safe_free(nl_srv_data->content);
            nl_srv_data->content_len = 0;
	    return;
        }
        nl_srv_data->seq++;

        if (nl_srv_data->content_len  > 0){
            nl_srv_data->content = realloc(nl_srv_data->content, nl_srv_data->content_len + nlmsg_datalen(hdr));
        } else {
            safe_free(nl_srv_data->content);
            nl_srv_data->content= calloc(1, nlmsg_datalen(hdr));
        }
        if (nl_srv_data->content == NULL){
            sedbg("malloc/realloc fail!\n");
            perror("malloc/realloc:");
            return;
        }
        memcpy(nl_srv_data->content + nl_srv_data->content_len, NLMSG_DATA(hdr), nlmsg_datalen(hdr));
        nl_srv_data->content_len += nlmsg_datalen(hdr);

	if (hdr->nlmsg_type == NLMSG_DONE) {
            //hexDump("recvmsgs:", nl_srv_data->content, nl_srv_data->content_len);
            if (nl_srv_data->nl_cmdfunc){
                nl_srv_data->content = realloc(nl_srv_data->content, nl_srv_data->content_len + 1);
                *(nl_srv_data->content + nl_srv_data->content_len) = '\0';
                nl_srv_data->nl_cmdfunc((void *)nl_srv_data->gdata, nl_srv_data->fd, nl_srv_data->content, nl_srv_data->content_len);
            }

            //sedbg("recvmsgs(%d):------\n%s\n------\n====\n", nl_srv_data->content_len, nl_srv_data->content);
            nl_srv_data->seq = 0;
            safe_free(nl_srv_data->nlmsg_buf);
            nl_srv_data->nlmsg_buflen = 0;
            safe_free(nl_srv_data->content);
            nl_srv_data->content_len = 0;
            nl_srv_data->iov_len = 0;
	    return;
	}
    }
//    sedbg("hdr addr: %x nlmsg_buf addr: %x diff:%d remining:%d buf_len:%d\n", (char *) hdr, (char *)nl_srv_data->nlmsg_buf, (char *)hdr - (char *)nl_srv_data->nlmsg_buf, remaining_len, nl_srv_data->nlmsg_buflen);
    memmove(nl_srv_data->nlmsg_buf, nl_srv_data->nlmsg_buf + (nl_srv_data->nlmsg_buflen - remaining_len), nl_srv_data->nlmsg_buflen - remaining_len);
    nl_srv_data->nlmsg_buflen = remaining_len;
}

static ipc_t ipc_nl =
{
	.name = "netlink",
	.access_point = "-1",
	.ipc_sockfunc = nl_sock,
	.ipc_recvfunc = nl_recv,
	.ipc_closesockfunc = nl_close,
};

void init_ipc_nl(cmdipc_t *cmdipc_pool, nl_cmdfunc cmdfunc, void *gdata)
{
    nl_srv_data_t *srvdata;
    srvdata = (nl_srv_data_t *)calloc(1,sizeof(nl_srv_data_t));
    if(!srvdata) {
        perror("calloc");
        return;
    }
    srvdata->gdata= gdata;
    strcpy(srvdata->access_point, ipc_nl.access_point);
    srvdata->nl_cmdfunc = cmdfunc;
    ipc_nl.cmdipc = add_cmdipc(cmdipc_pool, ipc_nl.name, ipc_nl.access_point, ipc_nl.ipc_sockfunc, ipc_nl.ipc_recvfunc, ipc_nl.ipc_closesockfunc, (void *)srvdata);
}

void uninit_ipc_nl(cmdipc_t *cmdipc_pool)
{
	cmdipc_t *cmdipc;
	if(!cmdipc_pool)
		return;
	cmdipc = ipc_nl.cmdipc;
	if (cmdipc == NULL)
		return;

	if (cmdipc->data != NULL){
		free(cmdipc->data); // srv_data, tcp_srv_data_t
		cmdipc->data = NULL;
	}
	remove_cmdipc(cmdipc_pool, cmdipc);
}

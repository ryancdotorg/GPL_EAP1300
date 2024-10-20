/*******************************************************************************************
 * Copyright (c) 2018 Ryan Hsu @ SenaoNetworks - Taiwan                                    *
 *                                                                                         *
 * This code has to be used in SenaoNetworks internally                                     *
 * Unauthorized copying of this file, via any medium is strictly prohibite                 *
 * Proprietary and confidential                                                            *
 * --------------------------------------------------------------------------------------- *
 * Project:  linkmon                                                                       *
 *                                                                                         *
 * Author :  Ryan Hsu (ryan_hsu@hotmail.com.tw)                                            *
 *******************************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <endian.h>
#include "mio.h"
#include "dns.h"
#include "util.h"

int dns_init(void *mdata, void *data);
void dns_close(void *data);
void dns_send(void *data);
void dns_recv(void *data);
int dns_status(void *data);
char *dns_getmsg(void *data);

static monitor_dns_t monitor_dns =
{
	.name = "dns",
	.sec = 5,
	.usec = 0,
	.lm_initfunc = dns_init,
	.lm_sendfunc = dns_send,
	.lm_recvfunc = dns_recv,
	.lm_statusfunc = dns_status,
	.lm_msgfunc = dns_getmsg,
	.lm_uninitfunc = dns_close,
	.dns_server = "8.8.8.8",
	.query_host = "aws.amazon.com",
};

#define T_A 1 //Ipv4 address
#define T_NS 2 //Nameserver
#define T_CNAME 5 // canonical name
#define T_SOA 6 /* start of authority zone */
#define T_PTR 12 /* domain name pointer */
#define T_MX 15 //Mail server

//DNS header structure
struct DNS_HEADER
{
	uint16_t id;
# if __BYTE_ORDER == __BIG_ENDIAN
	uint8_t qr:1;
	uint8_t opcode:4;
	uint8_t aa:1;
	uint8_t tc:1;
	uint8_t rd:1;
	uint8_t ra:1;
	uint8_t zero:1;
	uint8_t ad :1; // authenticated data
	uint8_t cd :1; // checking disabled
	uint8_t rcode:4;
# elif __BYTE_ORDER == __LITTLE_ENDIAN
	uint8_t rd:1;
	uint8_t tc:1;
	uint8_t aa:1;
	uint8_t opcode:4;
	uint8_t qr:1;
	uint8_t rcode:4;
	uint8_t cd :1; // checking disabled
	uint8_t ad :1; // authenticated data
	uint8_t zero:1;
	uint8_t ra:1;
# else
#  error "Adjust your <bits/endian.h> defines"
# endif
	uint16_t q_count;	// question count
	uint16_t ans_count;	// Answer record count
	uint16_t auth_count;	// Name Server (Autority Record) Count
	uint16_t add_count;	// Additional Record Count
};
//Constant sized fields of query structure
struct QUESTION
{
	unsigned short qtype;
	unsigned short qclass;
};

//Constant sized fields of the resource record structure
#pragma pack(push, 1)
struct R_DATA
{
	unsigned short type;
	unsigned short _class;
	unsigned int ttl;
	unsigned short data_len;
};
#pragma pack(pop)

//Pointers to resource record contents
struct RES_RECORD
{
	unsigned char *name;
	struct R_DATA *resource;
	unsigned char *rdata;
};

//Structure of a Query
typedef struct
{
	unsigned char *name;
	struct QUESTION *ques;
} QUERY;

#if TOOLCHAIN_MUSL
unsigned char* ReadName(unsigned char* reader,unsigned char* buffer,int* count)
#else
u_char* ReadName(unsigned char* reader,unsigned char* buffer,int* count)
#endif
{
	unsigned char *name = NULL;
	unsigned int p=0,jumped=0,offset;
	int i , j;

	*count = 1;
	if ((name = (unsigned char*)calloc(1,256)) == NULL){
		perror("calloc");
		return NULL;
	}


	//read the names in 3www6google3com format
	while(*reader!=0)
	{
		if(*reader>=192)
		{
			offset = (*reader)*256 + *(reader+1) - 49152; //49152 = 11000000 00000000 ;)
			reader = buffer + offset - 1;
			jumped = 1; //we have jumped to another location so counting wont go up!
		}
		else
		{
			name[p++]=*reader;
		}

		reader = reader+1;

		if(jumped==0)
		{
			*count = *count + 1; //if we havent jumped to another location then we can count up
		}
	}

	name[p]='\0'; //string complete
	if(jumped==1)
	{
		*count = *count + 1; //number of steps we actually moved forward in the packet
	}

	//now convert 3www6google3com0 to www.google.com
	for(i=0;i<(int)strlen((const char*)name);i++)
	{
		p=name[i];
		for(j=0;j<(int)p;j++)
		{
			name[i]=name[i+1];
			i=i+1;
		}
		name[i]='.';
	}
	name[i-1]='\0'; //remove the last dot
	return name;
}


/*
 * This will convert www.google.com to 3www6google3com
 * */
void ChangetoDnsNameFormat(unsigned char* dns,unsigned char* host)
{
	unsigned char *dst = (unsigned char*) dns
		, *src = (unsigned char*) host
		, *tick;

	for (tick=dst++;  *dst = *src++; dst++) {
		if (*dst == '.') { *tick = (dst-tick-1); tick = dst; }
	}
	*tick = (dst-tick-1);
}

void dns_clean_rcv(void *data)
{
	if (data == NULL){
		lmdbg("dns_clean_rcv, data not initialized\n");
		return;
	}
	dns_data_t *dnsdata;
	dnsdata = (dns_data_t *)data;
	// receive timeout
	if (dnsdata->mio_event_read){
		remove_event(dnsdata->mio_event_read);
		dnsdata->mio_event_read = NULL;
	}
	close(dnsdata->fd);
	dnsdata->fd = 0;
	strcpy(dnsdata->msg, "no response");
	dnsdata->status = DNS_NO_RESPONSE;
}

int dns_init(void *mdata, void *data)
{
	if (mdata == NULL || data == NULL){
		lmdbg("%s(%d) init error\n", __func__, __LINE__);
		return -1;
	}
	mio_data_t *mio_data;
	dns_data_t *dnsdata;
	mio_data = (mio_data_t *)mdata;
	dnsdata = (dns_data_t *)data;
	dnsdata->mio_data = (void *)mio_data;

	lmdbg("initial send timer\n");
	if (monitor_dns.lm_sendfunc){
		mio_timer_t *mt;
		mt = add_timer(&(mio_data->timer_pool), monitor_dns.sec, monitor_dns.usec, (timer_func)monitor_dns.lm_sendfunc, (void *)dnsdata, 1);
		dnsdata->mio_timer_snd = (void *) mt;
		return 0;
	}
	else {
		dnsdata->status = DNS_CREATE_ERR;
		return -1;
	}
}

void dns_close(void *data)
{
	if (data == NULL){
		lmdbg("dns_close, data not initialized\n");
		return;
	}
	dns_data_t *dnsdata;
	mio_event_t *mio_event;
	mio_timer_t *mio_timer;

	dnsdata = (dns_data_t *) data;
	mio_timer = (mio_timer_t *)dnsdata->mio_timer_snd;
	if (mio_timer != NULL){
		remove_timer(mio_timer);;
		dnsdata->mio_timer_snd = NULL;
	}

	if (dnsdata->fd > 0)
	{
		close(dnsdata->fd);
		dnsdata->fd = 0;
	}
}

void dns_send(void *data)
{
	if (data == NULL){
		lmdbg("send data broken error\n");
		return;
	}
	int fd;
	dns_data_t *dnsdata;
	mio_data_t *mio_data;
	dnsdata = (dns_data_t *)data;
	mio_data = (mio_data_t *)dnsdata->mio_data;

	if (dnsdata->fd > 0){
		lmdbg("previous fd job not finished, ignored. fd:[%d]\n", dnsdata->fd);
		return;
	}
	// initial send request
	lmdbg("socket fd set\n");
	if ((fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) <= 0){
		dnsdata->status = DNS_SEND_ERR;
		perror("socket error:");
		return;
	}

	(dnsdata->dest).sin_family = AF_INET;
	(dnsdata->dest).sin_port = htons(53);
	(dnsdata->dest).sin_addr.s_addr = dnsdata->dns_serv;

	//Set the DNS structure to standard queries
	unsigned char sendpacket[4096];
	struct DNS_HEADER *dns = NULL;
	struct QUESTION *qinfo = NULL;
	unsigned char *qname;

	memset(sendpacket, 0, sizeof(sendpacket));

	dns = (struct DNS_HEADER *)sendpacket;
	dns->id = (unsigned short) htons(getpid());
	dns->qr = 0; //This is a query
	dns->opcode = 0; //This is a standard query
	dns->aa = 0; //Not Authoritative
	dns->tc = 0; //This message is not truncated
	dns->rd = 1; //Recursion Desired
	dns->ra = 0; //Recursion not available! hey we dont have it (lol)
	dns->zero = 0;
	dns->ad = 0;
	dns->cd = 0;
	dns->rcode = 0;
	dns->q_count = htons(1); //we have only 1 question
	dns->ans_count = 0;
	dns->auth_count = 0;
	dns->add_count = 0;

	//DumpHex(sendpacket, sizeof(struct DNS_HEADER));
	//point to the query portion
	qname =(unsigned char*)&sendpacket[sizeof(struct DNS_HEADER)];

	ChangetoDnsNameFormat(qname , dnsdata->query_host);
	qinfo =(struct QUESTION*)&sendpacket[sizeof(struct DNS_HEADER) + (strlen((const char*)qname) + 1)]; //fill it
	qinfo->qtype = htons( T_A ); //type of the query , A , MX , CNAME , NS etc
	qinfo->qclass = htons(1); //its internet (lol)

	lmdbg("packet send\n");
	if (sendto(fd, sendpacket, sizeof(struct DNS_HEADER) + (strlen((const char*)qname)+1) + sizeof(struct QUESTION), 0, (struct sockaddr *)&(dnsdata->dest), sizeof(dnsdata->dest)) == -1)
	{
		lmdbg("sendto error\n");
		dnsdata->status = DNS_SEND_ERR;
		close(fd);
		return;
	}

	// register receive and receive timeout
	mio_event_t *er;
	mio_timer_t *mt_rcv;

	dnsdata->fd = fd;

	lmdbg("add recv event\n");
	er = add_event(&(mio_data->read_pool), dnsdata->fd, (event_func)monitor_dns.lm_recvfunc, (void *)dnsdata);
	dnsdata->mio_event_read=(void *)er;

	lmdbg("add timer for timeout recycling\n");
	mt_rcv = add_timer(&(mio_data->timer_pool), 3, 0, dns_clean_rcv, (void *)dnsdata, 0);
	dnsdata->mio_timer_rcv = mt_rcv;
	return;
}

void dns_recv(void *data)
{
	if (data == NULL){
		lmdbg("dns_recv, data not initialized\n");
		return;
	}
	dns_data_t *dnsdata;
	mio_event_t *mio_event;
	mio_timer_t *mio_timer_rcv;
	dnsdata = (dns_data_t *)data;
	mio_event = (mio_event_t *)dnsdata->mio_event_read;
	mio_timer_rcv = (mio_timer_t *)dnsdata->mio_timer_rcv;
	if (dnsdata->fd <= 0){
		dnsdata->status = DNS_RCV_ERR;
		return;
	}
	char recvpacket[4096];
	unsigned char *qname, *reader;
	struct DNS_HEADER *dns = NULL;
	int destlen;
	int res = 0, i, j;
	int stop;
	struct RES_RECORD answers[20],auth[20],addit[20]; //the replies from the DNS server
	memset(answers, 0, sizeof(struct RES_RECORD)*20);
	memset(auth, 0, sizeof(struct RES_RECORD)*20);
	memset(addit, 0, sizeof(struct RES_RECORD)*20);

        memset(&(dnsdata->dest), 0, sizeof(struct sockaddr_in));
	destlen = sizeof(dnsdata->dest);
	if ((res = recvfrom (dnsdata->fd, recvpacket, sizeof(recvpacket), 0,
			(struct sockaddr *) &(dnsdata->dest), &destlen)) < 0){
		dnsdata->status = DNS_RCV_ERR;
		close(dnsdata->fd);
		dnsdata->fd = 0;
		return;
	}

	dns = (struct DNS_HEADER*) recvpacket;
	qname =(unsigned char*)&recvpacket[sizeof(struct DNS_HEADER)];

	reader = &recvpacket[sizeof(struct DNS_HEADER) + (strlen((const char*)qname)+1) + sizeof(struct QUESTION)];

	stop=0;

	for(i=0;i<ntohs(dns->ans_count);i++)
	{
		answers[i].name=ReadName(reader,recvpacket,&stop);
		reader = reader + stop;

		answers[i].resource = (struct R_DATA*)(reader);
		reader = reader + sizeof(struct R_DATA);

		if(ntohs(answers[i].resource->type) == T_A) //if its an ipv4 address
		{
			answers[i].rdata = (unsigned char*)calloc(1, ntohs(answers[i].resource->data_len));
			if(answers[i].rdata==NULL){
				perror("calloc:");
				continue;
			}
			memcpy(answers[i].rdata, reader, ntohs(answers[i].resource->data_len));
			reader = reader + ntohs(answers[i].resource->data_len);

			struct sockaddr_in addr;
			addr.sin_addr.s_addr = *((long *)answers[i].rdata);
			//lmdbg("found: %s\n", inet_ntoa( addr.sin_addr));
			strcpy(dnsdata->msg, inet_ntoa( addr.sin_addr));
			dnsdata->status = STATUS_OK;
			close(dnsdata->fd);
			dnsdata->fd = 0;
			remove_event(mio_event);
			dnsdata->mio_event_read = NULL;

			// remove clean_conn timer
			if (mio_timer_rcv){
				remove_timer(mio_timer_rcv);
				dnsdata->mio_timer_rcv = NULL;
			}
			break;
		}
		else{
			answers[i].rdata = ReadName(reader,recvpacket,&stop);
			reader = reader + stop;
		}
	}
	// free
	for(i=0;i<ntohs(dns->ans_count);i++){
		if (answers[i].name)
			free(answers[i].name);
		if (answers[i].rdata)
			free(answers[i].rdata);
	}
	return;
}

int dns_status(void *data)
{
	if (data == NULL){
		return DNS_DATA_ERR;
	}
	dns_data_t *dnsdata;
	dnsdata = (dns_data_t *)data;
	return dnsdata->status;
}

char *dns_getmsg(void *data)
{
	if (data == NULL){
		return "";
	}
	dns_data_t *dnsdata;
	dnsdata = (dns_data_t *)data;
	return dnsdata->msg;
}

void init_dns(linkmon_t *linkmon_pool, char *dns_server, char *query_host)
{
	dns_data_t *dnsdata;
	dnsdata = (dns_data_t *)calloc(1,sizeof(dns_data_t));
	if(!dnsdata) {
		perror("calloc");
		return;
	}
	in_addr_t addr;
	if (*dns_server != '\0')
		addr = inet_addr(dns_server);
	else
		addr = inet_addr(monitor_dns.dns_server);
	dnsdata->dns_serv = addr;

	if (*query_host != '\0')
		strcpy(dnsdata->query_host, query_host);
	else
		strcpy(dnsdata->query_host, monitor_dns.query_host);
	dnsdata->status = LINK_INIT;

	monitor_dns.linkmon = add_linkmon(linkmon_pool, monitor_dns.name, monitor_dns.sec, monitor_dns.usec, monitor_dns.lm_initfunc, monitor_dns.lm_sendfunc, monitor_dns.lm_recvfunc, monitor_dns.lm_statusfunc, monitor_dns.lm_msgfunc, monitor_dns.lm_uninitfunc, (void *)dnsdata);
}

void uninit_dns(linkmon_t *linkmon_pool)
{
	linkmon_t *linkmon;
	if (!linkmon_pool) 
		return;

//	linkmon = find_linkmon_by_name(linkmon_pool, monitor_dns.name); 
	linkmon = monitor_dns.linkmon;
	if (linkmon == NULL)
		return;

	if (linkmon->data != NULL){ // dns_data_t
		free(linkmon->data);
		linkmon->data = NULL;
	}
	remove_linkmon(linkmon_pool, linkmon);
	monitor_dns.linkmon = NULL;
}


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
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h> // uint_8_t
#include <netinet/in.h>
#include <unistd.h>
#include <endian.h>
#include <sys/time.h>
#include "dns.h"
#include "statuscode.h"

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

static unsigned char* ReadName(unsigned char* reader,unsigned char* buffer,int* count)
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
static void ChangetoDnsNameFormat(unsigned char* dns,unsigned char* host)
{
	unsigned char *dst = (unsigned char*) dns
		, *src = (unsigned char*) host
		, *tick;

	for (tick=dst++;  *dst = *src++; dst++) {
		if (*dst == '.') { *tick = (dst-tick-1); tick = dst; }
	}
	*tick = (dst-tick-1);
}

int check_dns_status(char *dns_ip, char *query_host)
{
    int status = LINK_INIT;
	int fd;
	unsigned char ipBytes[4];
	//in_addr_t addr;
	uint32_t dns_serv;
	struct sockaddr_in dest;

	struct sockaddr_in from;
	socklen_t fromlen;

    fd_set readfds;
    struct timeval tv;
    int nselect = 1;
    int res = 0;

    if (dns_ip == NULL || query_host == NULL){
        status = DNS_DATA_ERR; 
        return status;
    }
	dns_serv = inet_addr(dns_ip);

	if ((fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) <= 0){
		perror("socket error:");
		status = DNS_CREATE_ERR;
		return status;
	}

	dest.sin_family = AF_INET;
	dest.sin_port = htons(53);
	dest.sin_addr.s_addr = dns_serv;

	//Set the DNS structure to standard queries
	unsigned char packet[4096];
	struct DNS_HEADER *dns = NULL;
	struct QUESTION *qinfo = NULL;
	unsigned char *qname;

	memset(packet, 0, sizeof(packet));

	dns = (struct DNS_HEADER *)packet;
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
	qname =(unsigned char*)&packet[sizeof(struct DNS_HEADER)];
	ChangetoDnsNameFormat(qname , query_host);

	qinfo =(struct QUESTION*)&packet[sizeof(struct DNS_HEADER) + (strlen((const char*)qname) + 1)]; //fill it
	qinfo->qtype = htons( T_A ); //type of the query , A , MX , CNAME , NS etc
	qinfo->qclass = htons(1); //its internet (lol)

	if (sendto(fd, packet, sizeof(struct DNS_HEADER) + (strlen((const char*)qname)+1) + sizeof(struct QUESTION), 0, (struct sockaddr *)&(dest), sizeof(dest)) == -1)
	{
		perror("sendto error\n");
		status = DNS_SEND_ERR;
		close(fd);
		return status;
	}

    unsigned char *reader;
    int destlen;
    int i, j;
    int stop;
    struct RES_RECORD answers[20],auth[20],addit[20]; //the replies from the DNS server
    struct sockaddr_in addr;

    FD_ZERO(&readfds); FD_SET(fd, &readfds);
    tv.tv_sec = 3;  tv.tv_usec = 0;
    while (1){
        status = LINK_INIT;
        nselect = select(fd + 1, &readfds, 
           NULL, NULL, &tv);
        if (nselect > 0){
            memset(answers, 0, sizeof(struct RES_RECORD)*20);
            memset(auth, 0, sizeof(struct RES_RECORD)*20);
            memset(addit, 0, sizeof(struct RES_RECORD)*20);
            memset(&(dest), 0, sizeof(struct sockaddr_in));
            destlen = sizeof(dest);
	        memset(packet, 0, sizeof(packet));

            if ((res = recvfrom (fd, packet, sizeof(packet), 0,
                            (struct sockaddr *) &(dest), &destlen)) < 0){
                continue;
            }
            dns = (struct DNS_HEADER*) packet;
            qname =(unsigned char*)&packet[sizeof(struct DNS_HEADER)];

            reader = &packet[sizeof(struct DNS_HEADER) + (strlen((const char*)qname)+1) + sizeof(struct QUESTION)];

            stop=0;

            for(i=0;i<ntohs(dns->ans_count);i++)
            {
                answers[i].name=ReadName(reader,packet,&stop);
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
                    //printf("found: %s\n", inet_ntoa( addr.sin_addr));
                    //strcpy(dnsdata->msg, inet_ntoa( addr.sin_addr));
                    status = STATUS_OK;
                    break;
                }
                else{
                    answers[i].rdata = ReadName(reader,packet,&stop);
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
            if (status != LINK_INIT) 
                break;
        }
        else if (nselect == 0){ // timeout without duplicated mac arp reply
            status = DNS_NO_RESPONSE;
            break;  
        }
    }
	close(fd);
    return status;
}

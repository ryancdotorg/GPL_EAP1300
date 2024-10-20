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
;    Creator : David Chang
;    File    : tcp.h
;    Abstract: Simple tcp protocol stack
;
;       Modification History:
;       By              Date     Ver.   Modification Description
;       --------------- -------- -----  --------------------------------------
;       David           20101011        Newly Create
;*****************************************************************************/
#ifndef _TCP_H
#define _TCP_H

/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
                    macros, defines, typedefs, enums
 ----------------------------------------------------------------------------*/
#define IMAGE_MEM_ADDR      0x82000000
#define BYTES_ALIGN(x)      ((x+3) & (~3))

/*-----------------------------------------------------------------------------
                    data declarations, extern, static, const
 ----------------------------------------------------------------------------*/
#if defined(Ralink_platform) || defined(Atheros_platform) || defined(CONFIG_UBOOT_AM335X_PLATFORM)

/*from /Realtek/bootcode/boot/include/asm/rtl8196.h*/
typedef unsigned long Int32;
typedef unsigned short Int16;
typedef unsigned char Int8;
typedef short INT16;
typedef unsigned short UINT16;
typedef int INT32;
typedef unsigned int UINT32;
typedef char CHAR8;
typedef unsigned char UCHAR8;

/*from /Realtek/bootcode/boot/include/etherboot.h*/
#define IP_TCP          06
#endif  /* if defined(CONFIG_UBOOT_AM335X_PLATFORM) */

#if defined(Realtek_platform)
#define IPPROTO_TCP     06
#define FRAME_IP        0x0800
#define ETH_ALEN        6   /* Size of Ethernet address */
#define TFTP_SERVER     0
#define TFTP_CLIENT     1
#define HTTPD_ARPENTRY  2

#endif  /* #if defined(Realtek_platform) */

#ifdef CONFIG_UBOOT_LITTLE_ENDIAN
#define _LITTLE_ENDIAN
#endif  /* #ifdef CONFIG_UBOOT_LITTLE_ENDIAN */

typedef union 
{
	unsigned long   num;
	unsigned char   numArray[4];
} int32_num;

typedef union 
{
	unsigned long   s_addr;
	unsigned char   ip[4];
} in_addr_tcp;

/* only support one http client
   for simply !!!we just assume there are no packet loss,no congestion,no dis-order
   Only One Session Exist. if two web browser connected together. first one succeed */
typedef struct utcpinfo
{
	Int8	sessionsaved;
	Int8	tcpi_state;
	Int8	node[6];    /*MAC address*/
	Int16	tcpi_sport;
	Int16	tcpi_dport;
	Int32	tcpi_ack;
	Int32 	tcpi_acked;
	Int32	tcpi_seq;
	Int32	next_seq;
	struct in_addr	dst;
	struct in_addr	src;
	Int8	uploadstarted;
	Int8	uploadfailed;
#ifdef REVERSE_SEQ_OVERWRITE
	Int8	retransmission;
#endif
} UTCPINFO_T, UTPCINFO_Tp;

/*TCP state*/
enum 
{
    TCP_ESTABLISHED = 1,
    TCP_SYN_SENT,
    TCP_SYN_RECV,
    TCP_FIN_WAIT1,
    TCP_FIN_WAIT2,
    TCP_TIME_WAIT,
    TCP_CLOSE,
    TCP_CLOSE_WAIT,
    TCP_LAST_ACK,
    TCP_LISTEN,
    TCP_CLOSING,	 /* now a valid state */
    
    TCP_MAX_STATES /* Leave at the end! */
};

/* TCP header definition */
struct tcphdr 
{
	unsigned short	source;
	unsigned short	dest;
	/*to avoid 32 bit not aligned*/
	Int32        	seq;
	Int32       	ack_seq;
#ifdef _LITTLE_ENDIAN   /*refer to kernel/include/linux/tcp.h*/
	/* ARM AM335x looks like LITTLE ENDIAN */
	union
	{
    	unsigned char 	flags[2];
        unsigned short  res1:4,
                        doff:4,
                        fin:1,
                        syn:1,
                        rst:1,
                        psh:1,
                        ack:1,
                        urg:1,
                        ece:1,
                        cwr:1;
	} tcpu;
#else
	/*for mips.Big Endian*/
	union
	{
    	unsigned char 	flags[2];
    	unsigned short	doff:4,
			res1:4,
			cwr:1,
			ece:1,
 			urg:1,
			ack:1,
			psh:1,
			rst:1,
			syn:1,
			fin:1;
	} tcpu;
/*!!!! BIG  Endian !!!!*/
#endif
	unsigned short	window;
	unsigned short	check;
	unsigned short	urg_ptr;
};

#define TCP_HDR_SIZE		(sizeof(struct tcphdr))

/*TCP flag*/
#define tcpflags    tcpu.flags
#define tcpdoff     tcpu.doff
#define tcpres1     tcpu.res1
#define tcpcwr      tcpu.cwr
#define tcpece      tcpu.ece
#define tcpurg      tcpu.urg
#define tcpack      tcpu.ack
#define tcppsh      tcpu.psh
#define tcprst      tcpu.rst
#define tcpsyn      tcpu.syn
#define tcpfin      tcpu.fin

/*TCP Mask*/
#define DOFF_MASK   0xF000
#define RES1_MASK   0x0F00
#define CWR_MASK    (1<<7)
#define ECE_MASK    (1<<6)
#define URG_MASK    (1<<5)
#define ACK_MASK    (1<<4)
#define PSH_MASK    (1<<3)
#define RST_MASK    (1<<2)
#define SYN_MASK    (1<<1)
#define FIN_MASK    (1<<0)

#define DOFF_OFFSET 12
#define RES1_OFFSET 8
#define CWR_OFFSET  7
#define ECE_OFFSET  6
#define URG_OFFSET  5
#define ACK_OFFSET  4
#define PSH_OFFSET  3
#define RST_OFFSET  2
#define SYN_OFFSET  1
#define FIN_OFFSET  0

/* extern global tcp information */
extern UTCPINFO_T g_tcp_info;

/*-----------------------------------------------------------------------------
                    functions declarations
 ----------------------------------------------------------------------------*/
extern void tcp_send_syn_ack(void);
extern void tcp_send_fin_ack(void);
extern void tcp_send_fin(void);
extern void tcp_ack(void);
extern void tcp_write(unsigned char *packet, int length);

extern void tcp_timeout_hdlr(void);
extern void tcp_handler(uchar *pkt, unsigned dest, struct in_addr sip, unsigned src, unsigned len);
extern void tcp_start(void);

extern void resend_tcp_sync_ack(void);

extern Int16 ip_hdr_chksum(Int16*ip,int len);

#if defined(Ralink_platform) || defined (Atheros_platform) || defined(CONFIG_UBOOT_AM335X_PLATFORM)
extern unsigned char g_pkt_big_buffer[];
#define G_PKT_BIG_BUF_SIZE  4096
#endif  /* defined(CONFIG_UBOOT_AM335X_RAINIER) */

#endif /* #ifndef _TCP_H */

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
;    File    : tcp.c
;    Abstract: Simple tcp protocol stack
;
;       Modification History:
;       By              Date     Ver.   Modification Description
;       --------------- -------- -----  --------------------------------------
;       David           20101011        Newly Create
;*****************************************************************************/
/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/
#include <common.h>
#include <command.h>
#include <net.h>
#include "bootp.h"
#include "tcp.h"
#include "httpd.h"

/*-----------------------------------------------------------------------------
                    macros, defines, typedefs, enums
 ----------------------------------------------------------------------------*/
#define SN_TIMEOUT          1
#define SN_TIMEOUT_COUNT    2		/* # of timeouts before giving up  */

/*dbg_tcp utility*/
//#define DBG_TCP
#ifdef	DBG_TCP
    #define dbg_tcp(fmt,args...)	printf (fmt ,##args)
    #define dbg_tcpx(level,fmt,args...) if (DBG_TCP>=level) printf(fmt,##args);
#else
    #define dbg_tcp(fmt,args...)
    #define dbg_tcpx(level,fmt,args...)
#endif

/*-----------------------------------------------------------------------------
                    data declarations, extern, static, const
 ----------------------------------------------------------------------------*/
UTCPINFO_T g_tcp_info;

/*for ip datagram sequence number*/
static unsigned short ip_seq=0;

#if defined(Ralink_platform) || defined (Atheros_platform) || defined(CONFIG_UBOOT_AM335X_PLATFORM)/* idleman-20140429 */
/*weird implementation for TCP protocol stack*/
unsigned char g_pkt_big_buffer[G_PKT_BIG_BUF_SIZE];
#endif  /* defined(CONFIG_UBOOT_AM335X_PLATFORM) */

#ifdef Realtek_platform
unsigned long load_addr = IMAGE_MEM_ADDR;  //senao
extern long NetBootFileXferSize; //senao
extern int autoBurn; //senao
struct arptable_t {
	in_addr ipaddr;
	unsigned char node[6];
};
extern struct arptable_t  arptable_tftp[3];
extern char eth0_mac_httpd[];
extern char eth0_ip_httpd[];
extern struct nic nic;
#endif

/*-----------------------------------------------------------------------------
                    functions declarations
 ----------------------------------------------------------------------------*/
/*static int dump_g_tcp_info(void);*//* defined but not used */
static void assign_ip(struct in_addr *iptoassign, struct in_addr *ipaddr);
static unsigned int chksum(void *dataptr, Int16 len);
static Int16 inet_chksum_pseudo(char *packet, struct in_addr src, struct in_addr dest, Int8 proto, Int32 proto_len);
static void tcp_checksum(unsigned char *packet, int length);
static void tcp_connected(void);
static int tcp_input_data(struct ip_hdr *ip_hdr, struct tcphdr *tcp_hdr);
static void tcp_reset(void);
static void tcp_output(struct tcphdr *tcp_hdr,unsigned char *packet, int length);
static void ip_output(unsigned char *packet, int length, Int8 protocol);
static void tcp_save_session(struct ip_hdr *ip_hdr, struct tcphdr *tcp_hdr);
static void tcp_clear_session(void);
static int is_same_session(struct ip_hdr *ip_hdr, struct tcphdr *tcp_hdr);

void tcp_start(void);

#ifdef Realtek_platform
extern void checkAutoFlashing(unsigned long startAddr, int len); // senao
#endif

/*-----------------------------------------------------------------------------
                    Static functions implementation
 ----------------------------------------------------------------------------*/
#ifdef Realtek_platform
static void writeImagetoflash(char *image,int len)
{
	/*reference the upgrade firmware*/
	if(autoBurn)
	{
		/*no need to auto reboot*/
	   	checkAutoFlashing((unsigned long)(image), len);
 	}     
}
#endif

/*-----------------------------------------------------------------------------
 * Name: dump_g_tcp_info
 *
 * Description: dump global tcp information
 *
 * Inputs:  
 *
 * Outputs: -
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
/* defined but not used */
/*
static int dump_g_tcp_info(void)
{
	dbg_tcp("Func: %s, sessionsaved=%d, tcpi_state=%d, tcpi_sport=%d, tcpi_dport=%d, tcpi_ack=%d, tcpi_acked=%d, tcpi_seq=%d\n", 
		 __FUNCTION__, g_tcp_info.sessionsaved, g_tcp_info.tcpi_state, g_tcp_info.tcpi_sport, g_tcp_info.tcpi_dport,
		g_tcp_info.tcpi_ack, g_tcp_info.tcpi_acked, g_tcp_info.tcpi_seq);
	return 0;
}
*/
/*-----------------------------------------------------------------------------
 * Name: dump_pkt
 *
 * Description: 
 *
 * Inputs:  
 *
 * Outputs: -
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
void dump_pkt(char *packet, int len)
{
	int i;
	dbg_tcp("\n");
	for(i=0;i<len;i++)
	{
		dbg_tcp("%02x ",*(unsigned char*)&packet[i]);
			if(!((i+1) % 16))
				dbg_tcp("\n");
	}
}

/*-----------------------------------------------------------------------------
 * Name: dump_tcp_info
 *
 * Description: 
 *
 * Inputs:  
 *
 * Outputs: -
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
void dump_tcp_info(struct tcphdr *tcp_hdr)
{
	dbg_tcp("----------->Func: %s, seq=%x, ack=%x\r\n",
		__FUNCTION__, ntohl(net_read_u32((u32 *)&tcp_hdr->seq)), ntohl(net_read_u32((u32 *)&tcp_hdr->acq_seq)));
}

/*-----------------------------------------------------------------------------
 * Name: assign_ip
 *
 * Description: 
 *
 * Inputs:  
 *
 * Outputs: -
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
static void assign_ip(struct in_addr *iptoassign, struct in_addr *ipaddr)
{
	//iptoassign->s_addr = ipaddr->s_addr;
	net_copy_ip((void *)&iptoassign->s_addr, &ipaddr->s_addr);
	return ;
}

/*-----------------------------------------------------------------------------
 * Name: chksum
 *
 * Description:  * Sums up all 16 bit words in a memory portion. Also includes any odd byte.
 * This function is used by the other checksum functions.
 *
 * For now, this is not optimized. Must be optimized for the particular processor
 * arcitecture on which it is to run. Preferebly coded in assembler.
 *
 * Inputs:  
 *
 * Outputs: -
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
static unsigned int chksum(void *dataptr, Int16 len)
{
	unsigned int acc;
	Int16 *dataptr16 = (Int16 *)dataptr;

	for(acc = 0; len > 1; len -= 2) 
	{
		acc += (unsigned int)(*(dataptr16++));      
	}
	
	/* add up any odd byte */
	if(len == 1) 
	{
#ifdef _LITTLE_ENDIAN
		acc += ((unsigned int)(*(dataptr16)) & 0xff);
#else
		acc += (((unsigned int)(*(dataptr16)) & 0xff) << 8);
#endif
	}

	return acc;
}

/*-----------------------------------------------------------------------------
 * Name: ip_hdr_chksum
 *
 * Description: ip header checksum
 *
 * Inputs:  
 *
 * Outputs: -
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
Int16 ip_hdr_chksum(Int16*ip,int len)
{
    Int32 sum = 0;

    len >>= 1;

    while (len--)
    {
        sum += *(ip++);
        if (sum > 0xFFFF)
        sum -= 0xFFFF;
    }                           /*Correct return 0*/

    return((~sum) & 0x0000FFFF);/*only 2 bytes*/
}

/*-----------------------------------------------------------------------------
 * Name: inet_chksum_pseudo
 *
 * Description: Calculates the pseudo Internet checksum used by TCP and UDP for a pbuf chain.
 *
 * Inputs:  
 *
 * Outputs: -
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
static Int16 inet_chksum_pseudo(
    char *packet, 
    struct in_addr src, 
    struct in_addr dest,
    Int8 proto, 
    Int32 proto_len)
{
    Int32 acc;

    acc = 0;
    acc += chksum(packet, proto_len);

    acc += (src.s_addr & 0xffff);
    acc += ((src.s_addr >> 16) & 0xffff);

    acc += (dest.s_addr & 0xffff);
    acc += ((dest.s_addr >> 16) & 0xffff);

    acc += (Int32)htons((Int16)proto);
    acc += (Int32)htons(proto_len); 

    while(acc >> 16) 
    {
        acc = (acc & 0xffff) + (acc >> 16);
    }

    return ~(acc & 0xffff);
}

/*-----------------------------------------------------------------------------
 * Name: tcp_checksum
 *
 * Description: 
 *
 * Inputs:  
 *
 * Outputs: -
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
static void tcp_checksum(unsigned char *packet, int length)
{
    struct ip_hdr *ip_hdr;
    struct tcphdr *tcp_hdr;

    ip_hdr = (struct ip_hdr *)packet;
    tcp_hdr = (struct tcphdr *)(packet+IP_HDR_SIZE);

    /* wried implementation, should do endian conversion?! */
#ifdef _LITTLE_ENDIAN
    tcp_hdr->check = inet_chksum_pseudo((char*)tcp_hdr,
                                        net_read_ip(&ip_hdr->ip_src), net_read_ip(&ip_hdr->ip_dst),
                                        IPPROTO_TCP, (length - IP_HDR_SIZE));
#else
    tcp_hdr->check = htons(inet_chksum_pseudo((char*)tcp_hdr,
                                              net_read_ip(&ip_hdr->ip_src), net_read_ip(&ip_hdr->ip_dst),
                                              IPPROTO_TCP, (length - IP_HDR_SIZE)));
#endif
    
    return;
}

/*-----------------------------------------------------------------------------
 * Name: tcp_connected
 *
 * Description: 
 *
 * Inputs:  
 *
 * Outputs: -
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
static void tcp_connected(void)
{
	/*receive ACK of syn ack. Connected*/
	/*seq add 1*/
	g_tcp_info.tcpi_seq += 1;
}

/*-----------------------------------------------------------------------------
 * Name: tcp_input_data
 *
 * Description: 
 *
 * Inputs:  
 *
 * Outputs: -
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
static int tcp_input_data(struct ip_hdr *ip_hdr, struct tcphdr *tcp_hdr)
{
	int length, doff, retval = 0;
	unsigned char *payload;

	doff = ((tcp_hdr->tcpflags[0] & 0xF0) >> 4) << 2;
	payload = (unsigned char *)((unsigned char*)tcp_hdr + doff);
	length = ntohs(ip_hdr->ip_len) - ((ip_hdr->ip_hl_v & 0xF) << 2) - doff;

	if(payload[0] =='G' && payload[1] == 'E' && payload[2] == 'T')
	{
		/*Get Method, send main page*/
		http_send_main_page();
		printf("Get Firmware Upgrade Request from WEB!!!\n");
		g_tcp_info.tcpi_state = TCP_FIN_WAIT1;
		tcp_send_fin_ack();
		retval = 1;
	}
	else if(payload[0] =='P' && payload[1] == 'O' && payload[2] == 'S' && payload[3] == 'T')					
	{
		/* Reset current tcp connection if the payload did not contain "multipart/form-data" identifier. */
		if(!strstr((const char *)payload, "multipart/form-data"))
		{
			retval=(-1);
			return retval;
		}
		
		retval = http_upload_file(payload, length, HTTP_POST_START);
		if(!g_tcp_info.uploadfailed)
		{
			printf("Start to Upload dlf file!!!\n");
			g_tcp_info.uploadstarted = 1;
			g_tcp_info.next_seq = ntohl(net_read_u32((u32 *)&tcp_hdr->seq)) + length;
		}
	}
	else
	{

		if(0 == length)
		{
			/*it's only a simple ACK no data*/
			retval = 1;
		}
		else if(g_tcp_info.uploadstarted)
		{
			retval = http_upload_file(payload, length, HTTP_POST_COND);
			g_tcp_info.next_seq = ntohl(net_read_u32((u32 *)&tcp_hdr->seq)) + length;
		}
		
	}

	/*Data Recived .Ack the Data. retval 0 means need to ack last  packet*/
	return retval;
}

/*-----------------------------------------------------------------------------
 * Name: tcp_reset
 *
 * Description: 
 *
 * Inputs:  
 *
 * Outputs: -
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
static void tcp_reset(void)
{
	unsigned char packet_buff[128];
	unsigned char *tmppacket=(unsigned char *)BYTES_ALIGN((unsigned long)packet_buff);
	struct tcphdr *tcp_hdr;

	memset(packet_buff,0,sizeof(packet_buff));
	tcp_hdr= (struct tcphdr *)(tmppacket + IP_HDR_SIZE);

	/*set rst*/
	tcp_hdr->tcpflags[1] = (1 << RST_OFFSET);

	/*set tcp_hdrlen*/
	tcp_hdr->tcpflags[0] = (20 >> 2) << 4;

	tcp_output(tcp_hdr,tmppacket, 0);
}

/*-----------------------------------------------------------------------------
 * Name: tcp_output
 *
 * Description: 
 *
 * Inputs:  
 *
 * Outputs: -
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
static void tcp_output(struct tcphdr *tcp_hdr,unsigned char *packet, int length)
{	
	//debug_cond(DEBUG_NET_PKT, "[tcp] tcp_output\n");
	/*fill tcp header*/
	tcp_hdr->source = htons(g_tcp_info.tcpi_dport);
	tcp_hdr->dest = htons(g_tcp_info.tcpi_sport);
	tcp_hdr->ack_seq = htonl(g_tcp_info.tcpi_ack);
	tcp_hdr->seq = htonl(g_tcp_info.tcpi_seq);
	g_tcp_info.tcpi_seq += length;

	/*set window size 2560 . reduce peer's burst sending*/
	tcp_hdr->window = htons(2560);

	/*checksum*/
	//tcp_hdr->check=0;
	length += (((tcp_hdr->tcpflags[0] & 0xF0) >> 4) << 2);
	ip_output(packet, length, IPPROTO_TCP);
}

/*-----------------------------------------------------------------------------
 * Name: ip_output
 *
 * Description: 
 *
 * Inputs:  
 *
 * Outputs: -
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
static void ip_output(unsigned char *packet, int length, Int8 protocol)
{
	/*fix ip header field*/
	struct ip_hdr *ip_hdr;
	volatile unsigned char *vptr_pkt;

	ip_hdr = (struct ip_hdr *)packet;

	/*header without options*/
	ip_hdr->ip_hl_v = 0x45;
	//ip_hdr->service=0x0
	ip_hdr->ip_len = htons(length + IP_HDR_SIZE);
	length = length + IP_HDR_SIZE;
	ip_hdr->ip_id = htons(ip_seq);
	ip_seq++;
	//ip_hdr->frags=htons(0x0);
	ip_hdr->ip_ttl = 0x80;
	ip_hdr->ip_p = protocol;

	/*ipchecksum*/
	ip_hdr->ip_sum = 0;
	ip_hdr->ip_off = htons(0x4000);
	assign_ip(&ip_hdr->ip_src, &g_tcp_info.dst);
	assign_ip(&ip_hdr->ip_dst, &g_tcp_info.src);
	ip_hdr->ip_sum = ip_hdr_chksum((Int16 *)packet, IP_HDR_SIZE);
	if(IPPROTO_TCP == protocol)
	{
		tcp_checksum(packet, length);
	}

	/*send it out*/
#if defined(Realtek_platform)
	prepare_txpkt(0, FRAME_IP,g_tcp_info.node, packet, length);
#elif defined(Ralink_platform) || defined(Atheros_platform) || defined(CONFIG_UBOOT_AM335X_PLATFORM)/* idleman-20140429 */
	/* We use memcpy to copy data to "net_tx_packet"
	 * Though it may affect the performance, the data we sent out is small
	 */
	vptr_pkt = net_tx_packet + net_eth_hdr_size();
	memcpy((void *)vptr_pkt, packet, length);

	NetSendTCPPacket(net_server_ethaddr, ip_hdr->ip_dst, g_tcp_info.tcpi_sport, g_tcp_info.tcpi_dport, length);
#endif  /* defined(CONFIG_UBOOT_AM335X_PLATFORM) */
}

/*-----------------------------------------------------------------------------
 * Name: tcp_save_session
 *
 * Description: save current session information
 *
 * Inputs:  
 *
 * Outputs: -
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
static void tcp_save_session(struct ip_hdr *ip_hdr, struct tcphdr *tcp_hdr)
{
#ifdef Realtek_platform
	/*Save client's mac*/
	/* David, check if we need MAC information*/
	memcpy(g_tcp_info.node, nic.packet+ETH_ALEN, ETH_ALEN);
#endif
	/*Save sip dip sport dport*/
	assign_ip(&g_tcp_info.src, &ip_hdr->ip_src);
	assign_ip(&g_tcp_info.dst, &ip_hdr->ip_dst);
	g_tcp_info.tcpi_dport = ntohs(tcp_hdr->dest);
	g_tcp_info.tcpi_sport = ntohs(tcp_hdr->source);
	g_tcp_info.sessionsaved = 1;
	g_tcp_info.tcpi_ack = ntohl(net_read_u32((u32 *)&tcp_hdr->seq));/* idleman-TODO */
	g_tcp_info.next_seq = g_tcp_info.tcpi_ack;
	g_tcp_info.tcpi_ack += 1;

	return;
}

/*-----------------------------------------------------------------------------
 * Name: tcp_clear_session
 *
 * Description: clean TCP session information
 *
 * Inputs:  
 *
 * Outputs: -
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
static void tcp_clear_session(void)
{
	memset(&g_tcp_info,0,sizeof(g_tcp_info));
	g_tcp_info.tcpi_state=TCP_CLOSE;
}

/*-----------------------------------------------------------------------------
 * Name: is_same_session
 *
 * Description: check pakcet belongs to the same session
 *
 * Inputs:  
 *
 * Outputs: -
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
static int is_same_session(struct ip_hdr *ip_hdr, struct tcphdr *tcp_hdr)
{
	int tcplen;
	struct in_addr ip_1 = net_read_ip(&ip_hdr->ip_src);
	struct in_addr ip_2 = net_read_ip(&g_tcp_info.src);

	dbg_tcp("File: %s, Func: %s, Line: %d\n", __FILE__, __FUNCTION__, __LINE__);

	if(g_tcp_info.sessionsaved)
	{
		/*check pakcet belongs to the same session.*/
		if ( (g_tcp_info.tcpi_dport == ntohs(tcp_hdr->dest)) &&
		     (g_tcp_info.tcpi_sport == ntohs(tcp_hdr->source)) &&
		     (ip_1.s_addr == ip_2.s_addr)
		   )
		{
			/*remember tcp seq*/
			tcplen = ntohs(ip_hdr->ip_len) - ((ip_hdr->ip_hl_v & 0xF) << 2) - (((tcp_hdr->tcpflags[0] & 0xF0) >> 4) << 2);
			g_tcp_info.tcpi_ack += tcplen;
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 1;
	}
}

/*-----------------------------------------------------------------------------
                    Extern functions implementation
 ----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 * Name: tcp_send_syn_ack
 *
 * Description: 
 *
 * Inputs:  
 *
 * Outputs: -
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
void tcp_send_syn_ack(void)
{
	/*SYN Recived. To Send SYN ACK packet*/
	unsigned char packet_buff[128];
	unsigned char *tmppacket=(unsigned char *)BYTES_ALIGN((unsigned long)packet_buff);
	struct tcphdr *tcp_hdr;
	unsigned char *option_field;

	memset(packet_buff, 0, sizeof(packet_buff));

	tcp_hdr = (struct tcphdr *)(tmppacket + IP_HDR_SIZE);
	/*fill syn acK*/
	tcp_hdr->tcpflags[1] = (1 << SYN_OFFSET | 1 << ACK_OFFSET);
	
	/*set mss to 1460*/
	option_field = tmppacket + IP_HDR_SIZE + 20;
	option_field[0] = 0x02;
	option_field[1] = 0x04;
	option_field[2] = 0x05;
	option_field[3] = 0xb4;
	option_field[4] = 0x01;
	option_field[5] = 0x01;
	option_field[6] = 0x04;
	option_field[7] = 0x02;
	
	/*set tcp_hdrlen*/
	tcp_hdr->tcpflags[0] = ((28 >> 2) << 4);

	/*set seq=0 and ack_seq=1*/	
	tcp_output(tcp_hdr, tmppacket, 0);
}

/*-----------------------------------------------------------------------------
 * Name: tcp_send_fin_ack
 *
 * Description: 
 *
 * Inputs:  
 *
 * Outputs: -
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
void tcp_send_fin_ack(void)
{
	unsigned char packet_buff[128];
	unsigned char *tmppacket = (unsigned char *)BYTES_ALIGN((unsigned long)packet_buff);
	struct tcphdr *tcp_hdr;

	memset(packet_buff, 0, sizeof(packet_buff));
	tcp_hdr = (struct tcphdr *)(tmppacket + IP_HDR_SIZE);

	dbg_tcp("File: %s, Func: %s, Line: %d\n", __FILE__, __FUNCTION__, __LINE__);

	/*set ack*/
	tcp_hdr->tcpflags[1] = (1<<ACK_OFFSET | 1<<FIN_OFFSET);

	/*set tcp_hdrlen*/
	tcp_hdr->tcpflags[0] = ((20 >> 2) << 4);

	tcp_output(tcp_hdr, tmppacket, 0);
}

/*-----------------------------------------------------------------------------
 * Name: tcp_send_fin
 *
 * Description: 
 *
 * Inputs:  
 *
 * Outputs: -
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
void tcp_send_fin(void)
{
	unsigned char packet_buff[128];
	unsigned char *tmppacket = (unsigned char *)BYTES_ALIGN((unsigned long)packet_buff);
	struct tcphdr *tcp_hdr;

	memset(packet_buff, 0, sizeof(packet_buff));
	tcp_hdr = (struct tcphdr *)(tmppacket + IP_HDR_SIZE);

	dbg_tcp("File: %s, Func: %s, Line: %d\n", __FILE__, __FUNCTION__, __LINE__);

	/*set ack*/
	tcp_hdr->tcpflags[1] = (1 << FIN_OFFSET);

	/*set tcp_hdrlen*/
	tcp_hdr->tcpflags[0] = ((20 >> 2) << 4);

	tcp_output(tcp_hdr, tmppacket, 0);
}

/*-----------------------------------------------------------------------------
 * Name: tcp_ack
 *
 * Description: 
 *
 * Inputs:  
 *
 * Outputs: -
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
void tcp_ack(void)
{
	unsigned char packet_buff[128];
	unsigned char *tmppacket = (unsigned char *)BYTES_ALIGN((unsigned long)packet_buff);
	struct tcphdr *tcp_hdr;

	memset(packet_buff, 0, sizeof(packet_buff));
	tcp_hdr = (struct tcphdr *)(tmppacket + IP_HDR_SIZE);

	dbg_tcp("File: %s, Func: %s, Line: %d\n", __FILE__, __FUNCTION__, __LINE__);

	/*set ack*/
	tcp_hdr->tcpflags[1] = 1 << ACK_OFFSET;

	/*set tcp_hdrlen*/
	tcp_hdr->tcpflags[0] = ((20 >> 2) << 4);

	tcp_output(tcp_hdr,tmppacket, 0);
}

/*-----------------------------------------------------------------------------
 * Name: tcp_write
 *
 * Description: now tcp write only can send tcp payload less than 1024.!!!
 *
 * Inputs:  
 *
 * Outputs: -
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
void tcp_write(unsigned char *packet, int length)
{
	unsigned char packet_buff[1400];
	unsigned char *tmppacket = (unsigned char *)BYTES_ALIGN((unsigned long)packet_buff);
	struct tcphdr* tcp_hdr;

	memset(packet_buff, 0, sizeof(packet_buff));

	dbg_tcp("File: %s, Func: %s, Line: %d\n", __FILE__, __FUNCTION__, __LINE__);

	tcp_hdr= (struct tcphdr *)(tmppacket + IP_HDR_SIZE);

	if(length + IP_HDR_SIZE + TCP_HDR_SIZE > 1400)
	{
		dbg_tcp("packet too long to send!\n");
	}

	memcpy(tmppacket + IP_HDR_SIZE + TCP_HDR_SIZE, packet, length);

	/*To ack*/
	tcp_hdr->tcpflags[1] = 1 << ACK_OFFSET;

	/*Set data offset*/
	tcp_hdr->tcpflags[0] = (20 >> 2) << 4;
	tcp_output(tcp_hdr, tmppacket, length);
}

#ifdef Realtek_platform
/*-----------------------------------------------------------------------------
 * Name: httpd_entry
 *
 * Description: 
 *
 * Inputs:  
 *
 * Outputs: -
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
void httpd_entry(void)
{
	memset(&g_tcp_info,0,sizeof(g_tcp_info));
	g_tcp_info.tcpi_state=TCP_CLOSE;
	if(memcmp(eth0_ip_httpd,"\x0\x0\x0\x0",4))
	{
		arptable_tftp[HTTPD_ARPENTRY].ipaddr.s_addr =*(unsigned long *)eth0_ip_httpd;
		if(memcmp(eth0_mac_httpd,"\x0\x0\x0\x0\x0\x0",4))
		{
			arptable_tftp[HTTPD_ARPENTRY].node[5]=eth0_mac_httpd[5];
			arptable_tftp[HTTPD_ARPENTRY].node[4]=eth0_mac_httpd[4];
			arptable_tftp[HTTPD_ARPENTRY].node[3]=eth0_mac_httpd[3];
			arptable_tftp[HTTPD_ARPENTRY].node[2]=eth0_mac_httpd[2];
			arptable_tftp[HTTPD_ARPENTRY].node[1]=eth0_mac_httpd[1];
			arptable_tftp[HTTPD_ARPENTRY].node[0]=eth0_mac_httpd[0];
		}
		else
		{
			memcpy(arptable_tftp[HTTPD_ARPENTRY].node,arptable_tftp[TFTP_SERVER].node,6);
		}
	}
	else
	{
		/* Sam Liang 2011-0701 Let HTTPD_ARPENTRY's IP address use DEFAULT_IP_ADDR*/
		arptable_tftp[HTTPD_ARPENTRY].ipaddr.s_addr = string_to_ip(DEFAULT_IP_ADDR);//my_config.mk
		memcpy(&arptable_tftp[HTTPD_ARPENTRY].node,&arptable_tftp[TFTP_SERVER].node,6);
		//memcpy(&arptable_tftp[HTTPD_ARPENTRY],&arptable_tftp[TFTP_SERVER],sizeof(struct arptable_t));
	}
}
#endif  /* #ifdef Realtek_platform */

/*-----------------------------------------------------------------------------
 * Name: tcp_input
 *
 * Description: 
 *
 * Inputs:  
 *
 * Outputs: -
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
void tcp_input(struct ip_hdr *ip_hdr, struct tcphdr *tcp_hdr)
{
    unsigned short check;
    unsigned long len;
    unsigned short ret = 0;

    if(HTTP_PORT == htons(tcp_hdr->dest))
    {
        len = ntohs(ip_hdr->ip_len) - ((ip_hdr->ip_hl_v & 0xF) << 2);
        check = inet_chksum_pseudo((char*)tcp_hdr,
                                   net_read_ip(&ip_hdr->ip_src), net_read_ip(&ip_hdr->ip_dst),
                                   IPPROTO_TCP, len);

        //debug_cond(DEBUG_NET_PKT, "[tcp] flag[1] = 0x%X, STATE = 0x%X, length = %d, check = 0x%lx\n", tcp_hdr->tcpflags[1], g_tcp_info.tcpi_state, len, check);
        if (check)
        {
            return;
        }

        /* If upload process was started, omit other connection. */
        if(g_tcp_info.uploadstarted && !is_same_session(ip_hdr, tcp_hdr))
        {
            return;
        }

        /* If upload process was started and expected next_seq did not equal to input seq, 
         * this means the packet was retransmitted. */
        if((g_tcp_info.uploadstarted == 1) && (g_tcp_info.next_seq != ntohl(net_read_u32((u32 *)&tcp_hdr->seq))))
        {
#ifdef REVERSE_SEQ_OVERWRITE
            /* Adjust the httpd_upload_mem_len to correct length and copy payload of input packet int buffer. */
            g_tcp_info.retransmission=1;
#else
            /* Just send ack packet and did not copy payload of input packet. */
            tcp_ack();
            return;
#endif
        }

        /* get tcp flags */
        if((tcp_hdr->tcpflags[1]) & SYN_MASK)
        {
            //reply with syn ack and window =2048
            g_tcp_info.tcpi_state = TCP_SYN_RECV;
            tcp_save_session(ip_hdr, tcp_hdr);
            tcp_send_syn_ack();
        }
        else if(tcp_hdr->tcpflags[1] & FIN_MASK)
        {
            /*This is our entry point, start write data into flash*/
                		 
            if(TCP_FIN_WAIT2 == g_tcp_info.tcpi_state)
            {
                g_tcp_info.tcpi_ack += 1;
                g_tcp_info.tcpi_seq += 1;
                tcp_ack();
                g_tcp_info.tcpi_state=TCP_CLOSE;
                tcp_clear_session();
            }
            else if(TCP_FIN_WAIT1 == g_tcp_info.tcpi_state)
            {
                if(tcp_hdr->tcpflags[1] & ACK_MASK)
                {
                    g_tcp_info.tcpi_ack += 1;
                    g_tcp_info.tcpi_seq += 1;
                    tcp_ack();
                    g_tcp_info.tcpi_state=TCP_CLOSE;
                    tcp_clear_session();
                }
                else
                {
                    /*Two FIN Send together.CLOSING same time*/
                    g_tcp_info.tcpi_ack += 1;
                    g_tcp_info.tcpi_seq += 1;
                    tcp_ack();
                    g_tcp_info.tcpi_state=TCP_CLOSING;
                }
            }
            else if(TCP_CLOSE_WAIT == g_tcp_info.tcpi_state)
            {
                /*never happen ?*/
            }
            else if(TCP_SYN_RECV == g_tcp_info.tcpi_state)
            {
                /*never happen ?*/
            }
            else if(TCP_ESTABLISHED == g_tcp_info.tcpi_state)
            {
                g_tcp_info.tcpi_state = TCP_CLOSE_WAIT;
                //send FIN ACK bk
                //increase 1 for FIN ack
                g_tcp_info.tcpi_ack += 1;
                g_tcp_info.tcpi_seq += 1;
                tcp_send_fin();
            }                	
        }
        else if((tcp_hdr->tcpflags[1]) & ACK_MASK)
        {
            if(TCP_SYN_RECV == g_tcp_info.tcpi_state)
            {
                g_tcp_info.tcpi_state = TCP_ESTABLISHED;
                tcp_connected();
            }
            else if(TCP_ESTABLISHED == g_tcp_info.tcpi_state)
            {
            	//Data Received
                ret = tcp_input_data(ip_hdr, tcp_hdr);
                if(0 == ret)
                {
                    tcp_ack();
                }            		  
                else if((-1) == ret)
                {
                    /* Un-expected POST data, just send reset packet to close tcp connection. */
                    tcp_clear_session();
                    tcp_reset();
                }
            }
            else if(TCP_CLOSE_WAIT == g_tcp_info.tcpi_state)
            {
                /*inactive close*/
                g_tcp_info.tcpi_state = TCP_CLOSE;
                tcp_clear_session();				
            } 
            else if(TCP_CLOSE ==g_tcp_info.tcpi_state)
            {
                /*Ack to FIN ACK. clear Session*/
                tcp_clear_session();
            }
            else if( TCP_FIN_WAIT1 == g_tcp_info.tcpi_state)
            {
                g_tcp_info.tcpi_state = TCP_FIN_WAIT2;
            }
            else if( TCP_FIN_WAIT2 == g_tcp_info.tcpi_state)
            {
                g_tcp_info.tcpi_state = TCP_FIN_WAIT2;
            }
            else if(TCP_CLOSING == g_tcp_info.tcpi_state)
            {
                /*Ack to pre send FIN. and peer's FIN received */
                g_tcp_info.tcpi_state = TCP_CLOSE;
                tcp_clear_session();
            }
            else
            {
                tcp_clear_session();
                tcp_reset();
            }
        }
        else
        {
            /*Just Reset the tcp connection*/
            if(TCP_CLOSE != g_tcp_info.tcpi_state)
            {
                tcp_clear_session();
                tcp_reset();
            }
        }
    }

    return;
} 

#if defined(Ralink_platform) || defined(Atheros_platform) || defined(CONFIG_UBOOT_AM335X_PLATFORM)/* idleman-20140429 */
/*-----------------------------------------------------------------------------
 * Name: tcp_timeout_hdlr
 *
 * Description: blinking the led
 *
 * Inputs:  
 *
 * Outputs: -
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
void tcp_timeout_hdlr(void)
{
    static int count = 0;
#if defined (RT3052_FPGA_BOARD) || defined (RT3052_ASIC_BOARD)
    unsigned long piodata_val;
#endif 
/* 
 * Fix me!!!! Init and config led for senao/customer style, port from board.c
 */ 
  
    /*dbg_tcp("tcp_timeout_hdlr, max_retry:%d\n", SN_TIMEOUT_COUNT);*/

    if (count==0)
    {
#if defined (RT3052_FPGA_BOARD) || defined (RT3052_ASIC_BOARD)
        /*set data*/
    	piodata_val = RALINK_REG_PIODATA;
    	piodata_val &= ~(1L << PWR_LED_GPIO_PIN); //Set Gpio to low 
    	piodata_val &= ~(1L << WPS_LED_GPIO_PIN); //Set Gpio to low 
    	RALINK_REG_W(RALINK_PIO_BASE + 0x20) = cpu_to_le32(piodata_val);
    	
#elif defined (RT3352_FPGA_BOARD) || defined (RT3352_ASIC_BOARD)
        /*Don't know if RT3352 need this?*/        
#elif defined (RT6855A_FPGA_BOARD) || defined (RT6855A_ASIC_BOARD)
        /*Don't know if RT6855A need this?*/        
#endif
        
        count = 1;        
    }
    else
    {
#if defined (RT3052_FPGA_BOARD) || defined (RT3052_ASIC_BOARD)
        /*set data*/
    	piodata_val = RALINK_REG_PIODATA;
    	piodata_val |= (1L << PWR_LED_GPIO_PIN); //Set Gpio to high
    	piodata_val |= (1L << WPS_LED_GPIO_PIN); //Set Gpio to high
    	RALINK_REG_W(RALINK_PIO_BASE + 0x20) = cpu_to_le32(piodata_val);
    	
#elif defined (RT3352_FPGA_BOARD) || defined (RT3352_ASIC_BOARD)
        /*Don't know if RT3352 need this?*/
#elif defined (RT6855A_FPGA_BOARD) || defined (RT6855A_ASIC_BOARD)
        /*Don't know if RT6855A need this?*/        
#endif

        count = 0;
    }
#if defined(CONFIG_UBOOT_AM335X_PLATFORM)/* idleman-20140429 */
    net_set_timeout_handler(30000000>>1, tcp_timeout_hdlr);
#else
    net_set_timeout_handler(CFG_HZ>>1, tcp_timeout_hdlr);
#endif  /* defined(CONFIG_UBOOT_AM335X_PLATFORM) */
}
#endif  /* defined(CONFIG_UBOOT_AM335X_PLATFORM) */

/*-----------------------------------------------------------------------------
 * Name: tcp_handler
 *
 * Description: 
 *
 * Inputs:  pkt points to tcp header, dest==src==len==0,
 *
 * Outputs: -
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
void tcp_handler(uchar *pkt, unsigned dest, struct in_addr sip, unsigned src, unsigned len)
{
    struct ip_hdr *ip_hdr;
    struct tcphdr *tcp_hdr;

#if defined(CONFIG_UBOOT_AM335X_PLATFORM)/* idleman-20140429 */
    if((((struct ip_udp_hdr *)pkt)->ip_p) != IPPROTO_TCP)
        return;
#else   /* #if defined(CONFIG_UBOOT_AM335X_PLATFORM) */
    if((((IP_t*)pkt)->ip_p)!=IPPROTO_TCP)
        return;
#endif   /* #if defined(CONFIG_UBOOT_AM335X_PLATFORM) */

#if defined(Ralink_platform) || defined (Atheros_platform) || defined(CONFIG_UBOOT_AM335X_PLATFORM)/* idleman-20140429 */
    /*weird implementation*/
    memset(g_pkt_big_buffer, 0, G_PKT_BIG_BUF_SIZE);
    memcpy((void *)g_pkt_big_buffer, pkt, (len > G_PKT_BIG_BUF_SIZE)?G_PKT_BIG_BUF_SIZE:len);
    /* get the ptr of ip_hdr and tcp_hdr */
    ip_hdr = (struct ip_hdr *)g_pkt_big_buffer;
    tcp_hdr = (struct tcphdr *)(g_pkt_big_buffer + IP_HDR_SIZE);
#elif defined(Realtek_platform)
    ip_hdr = (struct iphdr *)pkt;
    tcp_hdr = (struct tcphdr *)(pkt+sizeof(struct iphdr));
#endif  /* #if defined(CONFIG_UBOOT_AM335X_PLATFORM) */
    tcp_input(ip_hdr, tcp_hdr);    
}
 
/*-----------------------------------------------------------------------------
 * Name: tcp_start
 *
 * Description: 
 *
 * Inputs:  
 *
 * Outputs: -
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
void tcp_start(void)
{
	/*reset the global utcp information*/
	memset(&g_tcp_info, 0, sizeof(g_tcp_info));
	g_tcp_info.tcpi_state = TCP_CLOSE;

	/*update nethandler, timeout handler*/
	dbg_tcp("\nTcp start, Load address: 0x%lx\n", load_addr);

	/*mean httpd start to monitor HTTPD port*/
	g_httpd_started = 1;
#if defined(Ralink_platform) || defined(Atheros_platform)//my_config.mk
	net_set_timeout_handler(SN_TIMEOUT * CFG_HZ, tcp_timeout_hdlr);
	NetSetHandler(tcp_handler);
#elif defined(CONFIG_UBOOT_AM335X_PLATFORM)/* idleman-20140429 */
	net_set_timeout_handler(SN_TIMEOUT * 30000000, tcp_timeout_hdlr);
	net_set_udp_handler(tcp_handler);
#endif  /* defined(CONFIG_UBOOT_AM335X_PLATFORM) */
	/*Indicate the kernel file temporary store address*/
	g_httpd_upload_mem = load_addr;
#if defined(Ralink_platform) || defined(Atheros_platform) || defined(CONFIG_UBOOT_AM335X_PLATFORM)/* idleman-20140429 */
	/* zero out server ether in case the server ip has changed */
	memset(net_server_ethaddr, 0, 6);
#elif defined(Realtek_platform)
	httpd_entry();
#endif  /* defined(CONFIG_UBOOT_AM335X_PLATFORM) */
}

/*-----------------------------------------------------------------------------
 * Name: resend_tcp_sync_ack
 *
 * Description: After we got ARP response, we resend tcp sync_ack packet
 *
 * Inputs:  
 *
 * Outputs: -
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
void resend_tcp_sync_ack(void)
{
    dbg_tcp("File: %s, Func: %s, Line: %d\n", __FILE__, __FUNCTION__, __LINE__);
    tcp_send_syn_ack();
}


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
;    File    : httpd.c
;    Abstract: Simple httpd daemon
;
;       Modification History:
;       By              Date     Ver.   Modification Description
;       --------------- -------- -----  --------------------------------------
;       David           20101014        Newly Create
;*****************************************************************************/

/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/
#include <common.h>
#include <command.h>
#include <malloc.h>
#include <net.h>
#include "bootp.h"
#include <linux/ctype.h>
#include "protoHandler.h"
#include "tcp.h"
#include "httpd.h"

/*-----------------------------------------------------------------------------
                    macros, defines, typedefs, enums
 ----------------------------------------------------------------------------*/
/* for strtol */
#define ABS_LONG_MIN    2147483648UL
#define FIRMWARE_HEADER_LEN 96

#if defined(Realtek_platform)
#ifndef	NULL
#define NULL	((void *)0)
#endif
#endif /* #if defined(Realtek_platform) */

//#define DBG_HTTP
/*debug utility*/
#ifdef	DBG_HTTP
    #define dbg_http(fmt,args...)	printf (fmt ,##args)
    #define dbg_httpx(level,fmt,args...) if (DBG_HTTP>=level) printf(fmt,##args);
#else
    #define dbg_http(fmt,args...)
    #define dbg_httpx(level,fmt,args...)
#endif

#if defined(Realtek_platform)
unsigned long NetBootFileXferSize=0;
#endif  /* #if defined(Realtek_platform) */
/*-----------------------------------------------------------------------------
                    data declarations, extern, static, const
 ----------------------------------------------------------------------------*/

#ifdef VENDOR_NAME
static unsigned char http_indexdata[]="HTTP/1.0 200 OK\r\n\
Content-type: text/html\r\n\
\r\n\
<html><head>\
<META content='text/html; charset=iso-8859-1' http-equiv=Content-Type>\
</head><body><form enctype=multipart/form-data method=post><center>\
<center>\
<p><font color=blue face=Courier New, Courier, mono size=3><b> <font color='#FFFFFF' face='Courier New, Courier, mono'>--<font color='#0000FF'>"VENDOR_NAME"  </font></font><font face='Courier New, Courier, mono'>Firmware Upgrade System </font></b></font></p>\
</b></font></font>  </p><p>\
<table cellPadding=0 cellSpacing=10>\
<tr><td align=right>Firmware Image:</td><td>&nbsp;&nbsp;<input type=file name=userfile value=></td></tr>\
<tr><td>&nbsp;</td><td><input type=submit value='Upload'></td></tr>\
</table>\
<hr><ul><font face=Verdana color=red size=4>NOTICE !!</font><br></center>\
<font face=Verdana size=1><li>If you upload the binary file to the wrong TARGET, the device may not work properly\
or even could not boot-up again.\
  <hr>\
</form>\
</body>\
</html>";
#else   /* #ifdef VENDOR_NAME */
static unsigned char http_indexdata[]="HTTP/1.0 200 OK\r\n\
Content-type: text/html\r\n\
\r\n\
<html><head>\
<META content='text/html; charset=iso-8859-1' http-equiv=Content-Type>\
</head><body><form enctype=multipart/form-data method=post><center>\
<center>\
<p><font color=blue face=Courier New, Courier, mono size=3><b> <font face='Courier New, Courier, mono'>Firmware Upgrade System </font></b></font></p>\
</b></font></font>  </p><p>\
<table cellPadding=0 cellSpacing=10>\
<tr><td align=right>Firmware Image:</td><td>&nbsp;&nbsp;<input type=file name=userfile value=></td></tr>\
<tr><td>&nbsp;</td><td><input type=submit value='Upload'></td></tr>\
</table>\
<hr><ul><font face=Verdana color=red size=4>NOTICE !!</font><br></center>\
<font face=Verdana size=1><li>If you upload the binary file to the wrong TARGET, the device may not work properly\
or even could not boot-up again.\
  <hr>\
</form>\
</body>\
</html>";
#endif  /* #ifdef VENDOR_NAME */

static	unsigned char post_success[]="Update successfully!<br><br>Update in progress.<br> Do not turn off or reboot the Device during this time.\
";

static	unsigned char post_invalid_file[]="Invalid image file.\
";

unsigned char post_response[1024];

static	unsigned char post_response_hdr[]="HTTP/1.0 200 OK\r\n\
Content-type: text/html\r\n\
\r\n\
<html>\
<head><title>Upload Complete</title></head>\
<body>";
//static unsigned char post_response_footer[]="seconds</body></html>";

#ifdef CONFIG_UBOOT_RECOVERY_TO_MMC
static unsigned char post_cnt[]="HTTP/1.0 200 OK\r\n\
Content-type: text/html\r\n\r\n\
<html><head> \
<title>backup loader</title> \
<script language=\"javascript\">\
var count = 20;	\
function count_down(){\
if (count == 120) {return;}\
get_by_id(\"show_sec\").innerHTML = count;\
if (count < 120) {count+=20;setTimeout('count_down()', 1200);}}\
function get_by_id(id){with(document){return getElementById(id);}}\
</script>\
</head>\
<center>\
<font color=blue face=verdana size=3><b>Device is Upgrading the Firmware</b></font>\
<p>\
<table>\
<tr><td align=center><font face=Arial size=2 color=red>\
<b><span id=\"show_sec\"></span>&nbsp;&#37;</b>\
</font></td></tr>\
</table>\
<hr><ul><font face=Verdana color=red size=4>NOTICE !!</font><br>\
<font face=Verdana size=1><li>Don't turn the device off before the Upgrade jobs done !\
</center>\
<script>\
count_down();\
</script>\
</html>";
#endif  /* #ifdef CONFIG_UBOOT_RECOVERY_TO_MMC */
#ifdef CONFIG_UBOOT_RECOVERY_TO_SPI
static unsigned char post_cnt[]="HTTP/1.0 200 OK\r\n\
Content-type: text/html\r\n\r\n\
<html><head> \
<title>backup loader</title> \
<script language=\"javascript\">\
var count = 1;	\
function count_down(){\
if (count == 101) {return;}\
get_by_id(\"show_sec\").innerHTML = count;\
if (count < 101) {count++;setTimeout('count_down()', 1200);}}\
function get_by_id(id){with(document){return getElementById(id);}}\
</script>\
</head>\
<center>\
<font color=blue face=verdana size=3><b>Device is Upgrading the Firmware</b></font>\
<p>\
<table>\
<tr><td align=center><font face=Arial size=2 color=red>\
<b><span id=\"show_sec\"></span>&nbsp;&#37;</b>\
</font></td></tr>\
</table>\
<hr><ul><font face=Verdana color=red size=4>NOTICE !!</font><br>\
<font face=Verdana size=1><li>Don't turn the device off before the Upgrade jobs done !\
</center>\
<script>\
count_down();\
</script>\
</html>";
#endif  /* #ifdef CONFIG_UBOOT_RECOVERY_TO_SPI */

static int content_offset=0;
static int content_length=0;
static int httpd_upload_mem_len=0;
static int upload_len=0;
static int foundboundary=0;

/*default HTTPD IP address*/
static unsigned char httpd_def_Ip[] = {192,168,1,254};

/*write upload image to load_addr for senao*/
unsigned char readyToUpgrade = 0;
unsigned long g_httpd_upload_mem;

/* To know ARP reply requested by HTTPD  */
int g_httpd_started=0;

/*-----------------------------------------------------------------------------
                    functions declarations
 ----------------------------------------------------------------------------*/
static int find_content_length(char *payload,int length);
static int find_content_offset(char *payload,int length);
static int find_image_head(char *payload, int length, int *headlen);
static int get_ip_str(unsigned char *httpIp);
static int create_response_data(unsigned char *postData, unsigned char *postInfo, int time, int *packetlen);
static int valid_image(int len);

/*-----------------------------------------------------------------------------
                    Static functions implementation
 ----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
 * Name: my_strtoul
 *
 * Description: 
 *
 * Inputs:  
 *
 * Outputs: -
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
unsigned long int my_strtoul(const char *nptr, char **endptr, int base)
{
    unsigned long int v=0;
    
    while(isspace(*nptr)) ++nptr;
    if (*nptr == '+') ++nptr;
    if (base==16 && nptr[0]=='0') goto skip0x;

    if (!base) 
    {
        if (*nptr=='0') 
        {
            base=8;
            skip0x:
            if (nptr[1]=='x' || nptr[1]=='X') 
            {
                nptr+=2;
                base=16;
           }
        } 
        else
        {
            base=10;
        }
    }

    while(*nptr) 
    {
        register unsigned char c=*nptr;
        c=(c>='a'?c-'a'+10:c>='A'?c-'A'+10:c<='9'?c-'0':0xff);

        if (c>=base) 
        {
            break;
        }

        /*compare base*/
        {
            register unsigned long int w=v*base;

            if (w<v) 
            {
                return ULONG_MAX;
            }
            v=w+c;
        }
        ++nptr;
    }

    if (endptr) *endptr=(char *)nptr;

    return v;
}

/*-----------------------------------------------------------------------------
 * Name: my_strtol
 *
 * Description: 
 *
 * Inputs:  
 *
 * Outputs: -
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
long int my_strtol(const char *nptr, char **endptr, int base)
{
    int neg=0;
    unsigned long int v;
    
    while(isspace(*nptr)) 
    {
        nptr++;
    }
    
    if (*nptr == '-') 
    {
        neg=-1; 
        ++nptr; 
    }

    v=my_strtoul(nptr,endptr,base);

    if (v>=ABS_LONG_MIN) 
    {
        if (v==ABS_LONG_MIN && neg) 
        {
            return v;
        }
        return (neg?LONG_MIN:LONG_MAX);
    }

    return (neg?-v:v);
}
 
/*-----------------------------------------------------------------------------
 * Name: find_content_length
 *
 * Description: 
 *
 * Inputs:  
 *
 * Outputs: -
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
static int find_content_length(char *payload, int length)
{
	int i, content_length=0;

	// find out the content-length
	for(i=0;i<length;i++) 
	{
		while(1) 
		{
			if (payload[i]=='\r'&&payload[i+1]=='\n') 
			{
				i+=2;
				break;
			}
			i++;
		}
		
		if (payload[i]=='C'&&payload[i+1]=='o'&&payload[i+2]=='n'&&payload[i+3]=='t'&&
			payload[i+4]=='e'&&payload[i+5]=='n'&&payload[i+6]=='t'&&payload[i+7]=='-'&&
			payload[i+8]=='L'&&payload[i+9]=='e'&&payload[i+10]=='n'&&payload[i+11]=='g') 
		{
			i += 15;

			content_length = my_strtol(&payload[i],NULL,10);
			break;
		}
	}

	return content_length;
}

/*-----------------------------------------------------------------------------
 * Name: find_content_offset
 *
 * Description: 
 *
 * Inputs:  
 *
 * Outputs: -
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
static int find_content_offset(char *payload, int length)
{
    int i = 0;

    for(i = 0; i < length; i++) 
    {
        while(1) 
        {
            if (payload[i]=='\r' && payload[i+1]=='\n') 
            {
                i += 2;
                break;
            }
            i++;
        }

        while(i<length)
        {
            if (payload[i]=='\r' && payload[i+1]=='\n' && payload[i+2]=='\r' && payload[i+3]=='\n') 
            {
    	        i += 4;
    	        return i;
            }
            i++;
        }
    }
    return 0;/* idleman, fixed warning message */
}

/*-----------------------------------------------------------------------------
 * Name: find_image_head
 *
 * Description: 
 *
 * Inputs:  
 *
 * Outputs: -
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
static int find_image_head(char *payload, int length, int *headlen)
{
	int i;

	/* find out the head. two 0D0A0DA encounted .means head of image.
	   but they maybe in in Two packet */
	if(2 == foundboundary)
	{
		/*image header already found. just return 1. copy whole packet*/
		*headlen=0;
		return 1;
	}
	
	for(i=0;i<length;i++) 
	{
		while(1) 
		{
			if (payload[i]=='\r'&&payload[i+1]=='\n') 
			{
				i+=2;
				break;
			}
			i++;
		}
		
		while(i<length) 
		{
			if (payload[i]=='\r'&&payload[i+1]=='\n'&&payload[i+2]=='\r'&&payload[i+3]=='\n') 
			{
				foundboundary++;
				i+=4;

				if(2==foundboundary)
				{
					*headlen=i;
					 return 1;
				}
			}
			i++;
		}
	}

	return 0;
}

/*-----------------------------------------------------------------------------
 * Name: get_ip_str
 *
 * Description: 
 *
 * Inputs:  
 *
 * Outputs: -
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
static int get_ip_str(unsigned char *httpIp)
{
	int i, j, shift = 0;
	struct in_addr ip_addr;
	unsigned char ip_httpd[4];

	ip_addr = getenv_ip("ipaddr");

	if(ip_addr.s_addr == 0)
	{
		memcpy(ip_httpd, httpd_def_Ip, 4);
	}
	else
	{
		memcpy(ip_httpd, &ip_addr.s_addr, 4);
	}

	for(i=0;i<4;i++)
	{
		int cutNum=0;
		int firstNumFlag=0;
		
		for(j=100;j!=0;j/=10)
		{
			int num;
			num = ((ip_httpd[i] & 0x000000FF)-cutNum)/j;

			switch(num)
			{
				case 0:
					if(firstNumFlag || j==1)
					{
						httpIp[shift] = '0';
						shift++;
						cutNum+=num*j;
					}
					break;
				case 1:
					firstNumFlag = 1;
					httpIp[shift] = '1';
					shift++;
					cutNum+=num*j;
					break;
				case 2:
					firstNumFlag = 1;
					httpIp[shift] = '2';
					shift++;
					cutNum+=num*j;
					break;
				case 3:
					firstNumFlag = 1;
					httpIp[shift] = '3';
					shift++;
					cutNum+=num*j;
					break;
				case 4:
					firstNumFlag = 1;
					httpIp[shift] = '4';
					shift++;
					cutNum+=num*j;
					break;
				case 5:
					firstNumFlag = 1;
					httpIp[shift] = '5';
					shift++;
					cutNum+=num*j;
					break;
				case 6:
					firstNumFlag = 1;
					httpIp[shift] = '6';
					shift++;
					cutNum+=num*j;
					break;
				case 7:
					firstNumFlag = 1;
					httpIp[shift] = '7';
					shift++;
					cutNum+=num*j;
					break;
				case 8:
					firstNumFlag = 1;
					httpIp[shift] = '8';
					shift++;
					cutNum+=num*j;
					break;
				case 9:
					firstNumFlag = 1;
					httpIp[shift] = '9';
					shift++;
					cutNum+=num*j;
					break;
			}
		}

		if(i!=3)
		{
			httpIp[shift] = '.';
			shift++;
		}
	}
	return 0;
}

/*-----------------------------------------------------------------------------
 * Name: create_response_data
 *
 * Description: 
 *
 * Inputs:  
 *
 * Outputs: -
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
static int create_response_data(unsigned char *postData, unsigned char *postInfo, int time, int *packetlen)
{
	int len=0;
	unsigned char httpIp[16];

	dbg_http("File: %s, Func: %s, Line: %d\n", __FILE__, __FUNCTION__, __LINE__);

	memset(httpIp, 0x00, sizeof(httpIp));
	get_ip_str(httpIp);
	memcpy(post_response+len, post_cnt, sizeof(post_cnt));
	len += sizeof(post_cnt);

	*packetlen = len;
	return 0;
}
/*-----------------------------------------------------------------------------
 * Name: decodeImage
 *
 * Description:
 *  1.shift header (96bytes)
 *  2. firmware.bin file -> XOR decode ->  uImage
 *
 * Inputs:  
 *
 * Outputs: -
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
static int decodeImage(int fw_len)
{
    int i=0, ret=0;
    char ch, *p=NULL;
#if 0/* idleman-20150806, enable it in the future */
/*
typedef struct __IMAGE_HEADER
{   
    T_UINT32   start          __ATTRIBUTE_PACKED__;
    T_UINT32   vendor_id      __ATTRIBUTE_PACKED__;
    T_UINT32   product_id     __ATTRIBUTE_PACKED__;
    T_CHAR     version[16]    __ATTRIBUTE_PACKED__;  
    T_UINT32   type           __ATTRIBUTE_PACKED__;
    T_UINT32   comp_file_len  __ATTRIBUTE_PACKED__;
    T_UINT32   comp_file_sum  __ATTRIBUTE_PACKED__;
    T_CHAR     md5[16]        __ATTRIBUTE_PACKED__;
    T_CHAR     pad[32]        __ATTRIBUTE_PACKED__;
    T_UINT32   header_sum     __ATTRIBUTE_PACKED__;
    T_UINT32   magic_key      __ATTRIBUTE_PACKED__;
} __S_ATTRIBUTE_PACKED__ imageHeader_t;
*/
    /* check header correct or not */
    int vid, pid;
    const char *s = getenv("hw_id");
    unsigned char *header_hex;
    char header[FIRMWARE_HEADER_LEN] = {0};
    char cid[8] = {0};
    header_hex = (unsigned char *)g_httpd_upload_mem;

    vid = ((header_hex[4] << 24) 
         | (header_hex[5] << 16) 
         | (header_hex[6] << 8)
         | (header_hex[7] << 0)
          );
    pid = ((header_hex[8] << 24) 
         | (header_hex[9] << 16) 
         | (header_hex[10] << 8)
         | (header_hex[11] << 0)
          );

    sprintf(cid, "%08X", ((vid << 16) | pid));

    if(strcmp(cid, s)==0)
    {
        printf("cid mathed!!!!!\n");
    }
    else
    {
        printf("cid unmathed(%s)!!!!!\n", cid);
        goto decodeImage_end;
    }
#endif
    if(fw_len>FIRMWARE_HEADER_LEN)
    {
        /* 1. shift to uImage start address */
        g_httpd_upload_mem += FIRMWARE_HEADER_LEN;
        net_boot_file_size = fw_len-FIRMWARE_HEADER_LEN;
    
        /* 2. XOR decode (use MAGIC_KEY as seed) */
        while(i<net_boot_file_size)
        {
            p = ((char *)g_httpd_upload_mem + i);
            ch = *p;
            ch = ch ^ (char)(((MAGIC_KEY >> (i%8)) & 0xff));
            *p = ch;
            
            i++;
        };
    
        /* 3. adjust load_addr address */
#if defined(Realtek_platform)
        sn_memmove(load_addr, g_httpd_upload_mem, net_boot_file_size);
#elif defined(Ralink_platform) || defined(Atheros_platform) || defined(CONFIG_UBOOT_AM335X_PLATFORM)/* idleman-20140429 */
        memmove((void *)load_addr, (const void *)g_httpd_upload_mem, net_boot_file_size);
#endif  /* defined(CONFIG_UBOOT_AM335X_PLATFORM) */
        ret = 1;
    }
    else
    {
        goto decodeImage_end;
    }

decodeImage_end:

    return ret;
}
/*-----------------------------------------------------------------------------
 * Name: valid_image
 *
 * Description: If the image is valided, we must set NetState to NETLOOP_SUCCESS,
 *              and set net_boot_file_size for flash write operation
 *
 * Inputs: len  
 *
 * Outputs: -
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
static int valid_image(int len)
{
    int ret=1;    

    if(decodeImage(len))
    {
#if defined(Ralink_platform) || defined(Atheros_platform)
        NetState = NETLOOP_SUCCESS;
#elif defined(CONFIG_UBOOT_AM335X_PLATFORM)/* idleman-20140429 */
        net_set_state(NETLOOP_SUCCESS);
#endif  /* defined(CONFIG_UBOOT_AM335X_PLATFORM) */
        printf("Decode Done: valid image!!\n");
        ret = 1;
    }
    else
    {
    	/* Note : do not change NetState*/
        //NetState = NETLOOP_FAIL;
        printf("Decode Fail: invalid image!!\n");
        ret = 0;
    }

    return ret;
}

/*-----------------------------------------------------------------------------
                    Extern functions implementation
 ----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
 * Name: http_send_main_page
 *
 * Description: 
 *
 * Inputs:  
 *
 * Outputs: -
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
int http_send_main_page(void)
{    
    tcp_write(http_indexdata,sizeof(http_indexdata));
    return 1;
}

/*-----------------------------------------------------------------------------
 * Name: http_upload_file
 *
 * Description: 
 *
 * Inputs:  
 *
 * Outputs: -
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
int http_upload_file(unsigned char *payload , int length, unsigned char http_state)
{
	int headlen=0, ret_val=0, len=0;

	dbg_http(".");
	/*since IE and firefox fragment the packet in different way.
	*IE  			|len-n*mss |+| mss| + |mss | + ... + |mss |
	*firefox		|mss|     +     |mss| +... +|mss|   +   |len-n*mss |
	*	
	*/
#ifdef REVERSE_SEQ_OVERWRITE
        /* If retransmission flag was set, adjust the httpd_upload_mem_len to correct length. */
	if(g_tcp_info.retransmission)
	{
	    httpd_upload_mem_len-=length;
	    g_tcp_info.retransmission=0;
	}
#endif	
	if (0 == content_length)
	{
		content_length = find_content_length((char *)payload, length);
		dbg_http("\n#content_length %d\n", content_length);
	}

	if (0 == content_offset)
	{
		content_offset = find_content_offset((char *)payload,length);
	}

	if (find_image_head((char *)payload, length, &headlen))
	{
		/*in order to copy image aligned. we need find the image header. */
		memcpy((void *)(g_httpd_upload_mem+httpd_upload_mem_len), payload+headlen, length-headlen);
		httpd_upload_mem_len+=(length-headlen);
	}

	upload_len+=length;

	switch (http_state)
	{
		case HTTP_POST_START:
			break;
		default:
			break;
	}

	dbg_http("\n#upload_len=%d, headlen=%d\n", upload_len, headlen);
	if(upload_len >= (content_length+content_offset))
	{
		/*all image should be uploaded*/
		printf("\n#web upload success!\n");
		dbg_http("upload_len %x content_length %x content_offset %x g_httpd_upload_mem %x httpd_upload_mem_len %x\n",
		          upload_len, content_length, content_offset, g_httpd_upload_mem, httpd_upload_mem_len);

		/*david, we always assume image is valid? */
		if( 1==(valid_image(httpd_upload_mem_len)) )
		{
			readyToUpgrade = 1;
			tcp_ack();
			printf("Send ACK before RESPONSE!!!\n");
			create_response_data(post_response, post_success, 60, &len);
			tcp_write(post_response,len);
			printf("Send RESPONSE to show UPGRADE time!!!\n");
			tcp_send_fin_ack();
			g_tcp_info.tcpi_state=TCP_FIN_WAIT1;
		}
		else
		{
			memset(post_response, 0, sizeof(post_response));
			memcpy(post_response, post_response_hdr, sizeof(post_response_hdr));
			len+=sizeof(post_response_hdr);

			/*david, to display error massage */
			memcpy(post_response+len, post_invalid_file, sizeof(post_invalid_file));
			len += sizeof(post_invalid_file);

			tcp_write(post_response,len);
			tcp_send_fin_ack();
                        /* reset global flag */
                        g_tcp_info.uploadfailed=1;
                        httpd_upload_mem_len=0;
                        upload_len=0;
                        content_length=0;
                        content_offset=0;
                        foundboundary=0;
		}
		return 1;
	}

	return ret_val;
}


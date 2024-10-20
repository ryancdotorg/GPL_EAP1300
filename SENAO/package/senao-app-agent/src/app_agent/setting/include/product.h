/*****************************************************************************
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
;------------------------------------------------------------------------------
;
;    Project : SI-688H
;    Creator :
;    File    :
;    Abstract: This file should accord to project! So not synchonize
;
;       Modification History:
;       By              Date     Ver.   Modification Description
;       --------------- -------- -----  --------------------------------------
;
;*****************************************************************************/
#ifndef _PRODUCT_H
#define _PRODUCT_H

/*-------------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                             */
/*-------------------------------------------------------------------------*/

#include <gconfig.h>
#include <opmodes.h>
/*-------------------------------------------------------------------------*/
/*                           DEFINITIONS                                   */
/*-------------------------------------------------------------------------*/

//mhchen:2007-0202 VOIP Information,web use
#define CALL_MONITOR_FILE 		"/var/log/asterisk/monitor/channels"
#define SIP_REGISTRY_FILE		"/var/log/asterisk/monitor/registry"
#define SIP_PEERS_FILE			"/var/log/asterisk/monitor/peers"
#define SIP_STATUS_FILE			"/var/log/asterisk/monitor/status"
#define SIP_CDR_FILELIST_NAME 	"cdrlist"
//#define SIP_CDR_FILE_PATH     "/storage/"
#define SIP_CDR_FILE_PATH     	"/var/log/asterisk/cdr-custom/"
#define CSVCUT_FILE_PATH      	"/var/cvscut/"


#define DHCPDItemListMax 	1 //only one dhcp pool is supported in Rt28xx
#define NUM_WAN_DNS		 	2

#define MACFILTER_PATTERN 			"%d,%d,%02X%02X%02X%02X%02X%02X,%d,%s"
#define MACFILTER_PATTERN_ITEMS 	10

#define L7FILTER_PATTERN 			"%d,%d"
#define L7FILTER_PATTERN_ITEMS 		2

/*jaykung 20060823 blacklist for grpinfo*/
// #define BLACKLIST_PATTERN "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"
// #define BLACKLIST_PATTERN "%d,%s,%d,%s,%d,%s,%d,%s,%d,%s,%d,%s,%d,%s,%d,%s,%d,%s,%d,%ss"
#define BLACKLIST_PATTERN_ITEM 		10

/* day time mask */
#define IS_DT_MON   0x1000000
#define IS_DT_TUE   0x0100000
#define IS_DT_WED   0x0010000
#define IS_DT_THR   0x0001000
#define IS_DT_FRI   0x0000100
#define IS_DT_SAT   0x0000010
#define IS_DT_SUN   0x0000001

/*WLAN11b g channel list num*/
#define WLANCHMAXUMN 				14

//#define WLAN_CLIENTINFO_PATH    		"/proc/net/madwifi/ath0/associated_sta"

#define NAT_VSERVER_PATTERN 			"%d,%d,%d,%d,%d,%d,"
#define NAT_VSERVER_PATTERN_ITEMS 		7
#define NUM_NAT_SERVER 				NUM_NAT_VSERVER
#define NUM_NAT_PORT_FORWARD			NUM_PORT_FORWARD
#define URL_CONTENT_MAX             		40

#define MAX_SCHEDULE_LIST 			NUM_SCHEDULE_LIST
#define MAX_SCHED_DES   			25

//#define MAX_TIME_INFO_LIST 			8
#define MAX_TIME_INFO_DES   			25

#define MAX_DATABASE_LIST 			32
#define MAX_DATABASE_DES   			20

#define ISENABLE_PATTERN    			"%d,"

#define NUM_APPLICATION_FILTER  		2

#define MAX_DHCP_MACTOIP                        NUM_DHCPD_MACTOIP

#define IPFILTER_PATTERN 			"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%s"
#define IPFILTER_PATTERN_ITEMS  		11
#define DAYTIME_PATTERN 			"%d,%02d,%02d,%02d,%02d,0x%07x"
#define DAYTIME_PATTERN_ITEMS  			6

#define MAX_WEPKEY_LEN 				33
#define MAX_MAC_ADDR_LEN_IN_BYTE 		6
#define MAX_MAC_ADDR_LEN_IN_STR			20
#define NUM_SNMP        			5
#define NUM_COUNTRY    				97
#define NUM_STATIC_ASSIGN_IPS     		10
/*smtp*/
#define STMP_MAX_ID     			128
#define STMP_MIN_ID     			1
#define STMP_MAX_PWD				64
#define STMP_MIN_PWD    			1

#define MAX_LENGTH_TELNUM 			32

//static routing
#define MAX_STATIC_ROUTING      		NUM_STATICROUTING
#define MAX_STATICROUTING_NAME  		24

//max num of sntp server
#define NTP_SERVER_MAX 				3

//max num of dmz host
#define NUM_DMZ_HOSTS				5

/*jaykung 20061027 getFileInfo will produce this file to web_cgi*/
#define PARSE_FILEINFO_PATH 		"/var/tmp/info"

/************************************************/
/*jaykung 20070503                              */
/*define output file path for different files   */
#define PARSE_DHCP_LEASE            "/var/tmp/p_dhcpInfo"
#define PARSE_SIP_CDR               "/var/tmp/p_sipCdr"
#define PARSE_SIP_CALLMONITOR       "/var/tmp/p_sipCallMonitor"
#define PARSE_SIP_PEER              "/var/tmp/p_sipPeer"
#define PARSE_SIP_REG              	"/var/tmp/p_sipReg"
/**************************************************/
/**Victor 20070507 path to store transcoding_list
The path will affect processes
(1) "vm_file_check_gen" and
(2) "trans_coding_check"*/
//Record if there were any vm_file needs to transcoding
#define TRANSCODING_LIST 			"/storage/transcoding_list"
//copy from transcoding_list in /storage, under transcoding proccess
#define TEMP_TRANSCODING_LIST 		"/tmp/transcoding_list"
/**backup from transcoding_list in /storage, it can be worked(recover) whether the transcoding proccess
was shut down un-expectly*/
#define BACKUP_TRANSCODING_LIST 	"/storage/backup_transcoding_list"
#define TRANSCODING_PROCESS_NAME 	"transcoder"
#define TRANS_CODING_CHECK_NAME 	"trans_coding_check"
/**************************************************/

/*jaykung 20070515 max number for followme function*/
#define MAX_FOLLOWME_NUM    		3

/*mhchen,2007-0303 resverd number for pbx*/
#define PBX_RESERVED_NUMBER 		"777,770,771,700,900,9,888"
#define PBX_RESERVED_NUMBER_RANGE 	"701,720,900,999"

#define CHECK_VERSION_LANG_FILE     "/var/%s_xml.txt"


/*-------------------------------------------------------------------------*/
/*                           GROUP DEFINITIONS                             */
/*-------------------------------------------------------------------------*/
// default cfg table group set macro
// single group set
#define G_CFG_SET_1_(x)        {(x),  G_NO, G_NO, G_NO}
#define G_CFG_SET_2_(x)        {G_NO, (x),  G_NO, G_NO}
#define G_CFG_SET_3_(x)        {G_NO, G_NO, (x),  G_NO}
#define G_CFG_SET_4_(x)        {G_NO, G_NO, G_NO, (x) }
// multi group set
#define G_CFG_SET_1_2_(x,y)    {(x),  (y),  G_NO, G_NO}

/* APCFG Group OPT */
#define OPT_G_DUMMY                  0
#define OPT_G_LAN                    1
#define OPT_G_WAN                    2
#define OPT_G_WLAN                   3
#define OPT_G_AST                    4
#define OPT_G_FW                     5
#define OPT_G_QOS                    6
#define OPT_G_TIMEZONE               7
#define OPT_G_WANDNS                 8
#define OPT_G_HOST_DOMAIN_NAME       9
#define OPT_G_SCH                    10
#define OPT_G_DHCPD                  11
#define OPT_G_WLANFW                 12
#define OPT_G_AUTO_PROV              13
#define OPT_G_NTPD                   14
#define OPT_G_URL                    15
#define OPT_G_FILTER                 16
#define OPT_G_DDNS                   17
#define OPT_G_UPNPD                  18
#define OPT_G_HTTPD                  19
#define OPT_G_FWMD                   20
#define OPT_G_DNSPROXY               21
#define OPT_G_TELENTD                22
#define OPT_G_TFTPD                  23
#define OPT_G_LOG                    24
#define OPT_G_PPTPD                  25
#define OPT_G_SMTP                   26
#define OPT_G_ROUTING                27
#define OPT_G_ALG                    28
#define OPT_G_KCODES                 29
#define OPT_G_SNMPD                  30
#define OPT_G_USB                    31
#define OPT_G_ATH1                   32
#define OPT_G_ATH2                   33
#define OPT_G_LLTD                   34
#define OPT_G_EBTABLES               35
#define OPT_G_RA1                    36
#define OPT_G_RA2                    37
#define OPT_G_IGMPPROXY              38
#define OPT_G_RA2WLANFW              39
#define OPT_G_NETBIOSD               40
#define OPT_G_TR                     41
#define OPT_G_HTTPSD                 42
#define OPT_G_MACBRIDGE              43
#define OPT_G_ACCOUNT                44
#define OPT_G_MACBRIDGING            45
#define OPT_G_FTPD                   46
#define OPT_G_HWNAT                  47
#define OPT_G_VPN                    48
#define OPT_G_AIRPORT                49
#define OPT_G_JP_STATICROUTING       50
#define OPT_G_WLAN_RADIO1            51
#define OPT_G_WLAN_RADIO2            52
#define OPT_G_HOSTAPD1               53
#define OPT_G_HOSTAPD2               54
#define OPT_G_IPV6                   55
#define OPT_G_FIREWALLMD             56
#define OPT_G_NPH                    57
#define OPT_G_SCUTM                  58
#define OPT_G_DLNA                   59
#define OPT_G_DHCPRELAY              60
#define OPT_G_FIREWALL_V6            61
#define OPT_G_IPTV		             62
#define OPT_G_ROUTING_V6	         63
//============================ G_CFG_SET_2_ ================================
#define OPT_G_SAMPLE_64              64
#define OPT_G_SAMPLE_65              65
#define OPT_G_IPCAM                  66
#define OPT_G_PLC                    67
#define OPT_G_SAMBA                  68
#define OPT_G_SAMBA_CLIENT           69
#define OPT_G_SAMBA_MOUNT            70
#define OPT_G_RPCBIND                71
#define OPT_G_NFS                    72
#define OPT_G_IPCAMMGR               73
#define OPT_G_PPPOESERVER            74
#define OPT_G_ENCLOUD                75
#define OPT_G_MDNS_RESPONDER         76
#define OPT_G_RTSP 			         77
#define OPT_G_NATTRAVERSAL			 78
#define OPT_G_PACKET_TRANSCEIVER     79
#define OPT_G_TCRULE                 80


/* Group for restarting setting */
#define G_BITMAP                     T_UINT64
#define G_BITS                       (sizeof(G_BITMAP)*8)
#define G_MASK(x)                    ((G_BITMAP)1<< (x & (G_BITS - 1)))
#define G_FOUND_OPTS(x,y)            (x[(unsigned)y/G_BITS])
#define G_MATCH_MACRO(x,y)           (G_FOUND_OPTS(x, OPT_ ## y) & (y))
#define G_PRINT_MATCH_MACRO(x,y)     if(G_FOUND_OPTS(x, OPT_ ## y) & (y)) T_PRINTF("[%s] ", #y);
#define G_MACRO_TO_STR(x)            #x
//G_BITMAP found_opts[256/G_BITS];

#define G_NO                         0   /* no group, or group is not assigned yet */
#define G_DUMMY                      G_MASK(OPT_G_DUMMY)            /* Dummy group. Don't store in ap_cfg_new and ap_cfg. Don't handle change */
#define G_LAN                        G_MASK(OPT_G_LAN)
#define G_WAN                        G_MASK(OPT_G_WAN)
#define G_WLAN                       G_MASK(OPT_G_WLAN)
#define G_AST                        G_MASK(OPT_G_AST)
#define G_FW                         G_MASK(OPT_G_FW)
#define G_QOS                        G_MASK(OPT_G_QOS)
#define G_TIMEZONE                   G_MASK(OPT_G_TIMEZONE)
#define G_WANDNS                     G_MASK(OPT_G_WANDNS)
#define G_HOST_DOMAIN_NAME           G_MASK(OPT_G_HOST_DOMAIN_NAME)
#define G_SCH                        G_MASK(OPT_G_SCH)
#define G_DHCPD                      G_MASK(OPT_G_DHCPD)
#define G_WLANFW                     G_MASK(OPT_G_WLANFW)
#define G_AUTO_PROV                  G_MASK(OPT_G_AUTO_PROV)
#define G_NTPD                       G_MASK(OPT_G_NTPD)
#define G_URL                        G_MASK(OPT_G_URL)
#define G_FILTER                     G_MASK(OPT_G_FILTER)
#define G_DDNS                       G_MASK(OPT_G_DDNS)
#define G_UPNPD                      G_MASK(OPT_G_UPNPD)
#define G_HTTPD                      G_MASK(OPT_G_HTTPD)
#define G_FWMD                       G_MASK(OPT_G_FWMD)
#define G_DNSPROXY                   G_MASK(OPT_G_DNSPROXY)
#define G_TELENTD                    G_MASK(OPT_G_TELENTD)
#define G_TFTPD                      G_MASK(OPT_G_TFTPD)
#define G_LOG                        G_MASK(OPT_G_LOG)
#define G_PPTPD                      G_MASK(OPT_G_PPTPD)
#define G_SMTP                       G_MASK(OPT_G_SMTP)
#define G_ROUTING                    G_MASK(OPT_G_ROUTING)
#define G_ALG                        G_MASK(OPT_G_ALG)
#define G_KCODES                     G_MASK(OPT_G_KCODES)
#define G_SNMPD                      G_MASK(OPT_G_SNMPD)
#define G_USB                        G_MASK(OPT_G_USB)
#define G_ATH1                       G_MASK(OPT_G_ATH1)   // <---- 32
#define G_ATH2                       G_MASK(OPT_G_ATH2)
#define G_LLTD                       G_MASK(OPT_G_LLTD)
#define G_EBTABLES                   G_MASK(OPT_G_EBTABLES)
#define G_RA1                        G_MASK(OPT_G_RA1)
#define G_RA2                        G_MASK(OPT_G_RA2)
#define G_IGMPPROXY                  G_MASK(OPT_G_IGMPPROXY)
#define G_RA2WLANFW                  G_MASK(OPT_G_RA2WLANFW)
#define G_NETBIOSD                   G_MASK(OPT_G_NETBIOSD)
#define G_TR                         G_MASK(OPT_G_TR)
#define G_HTTPSD                     G_MASK(OPT_G_HTTPSD)
#define G_MACBRIDGE                  G_MASK(OPT_G_MACBRIDGE)
#define G_ACCOUNT                    G_MASK(OPT_G_ACCOUNT)
#define G_MACBRIDGING                G_MASK(OPT_G_MACBRIDGING)
#define G_FTPD                       G_MASK(OPT_G_FTPD)
#define G_HWNAT                      G_MASK(OPT_G_HWNAT)
#define G_VPN                        G_MASK(OPT_G_VPN)
#define G_AIRPORT                    G_MASK(OPT_G_AIRPORT)
#define G_JP_STATICROUTING           G_MASK(OPT_G_JP_STATICROUTING)
#define G_WLAN_RADIO1                G_MASK(OPT_G_WLAN_RADIO1)
#define G_WLAN_RADIO2                G_MASK(OPT_G_WLAN_RADIO2)
#define G_HOSTAPD1                   G_MASK(OPT_G_HOSTAPD1)
#define G_HOSTAPD2                   G_MASK(OPT_G_HOSTAPD2)
#define G_IPV6                       G_MASK(OPT_G_IPV6)
#define G_FIREWALLMD                 G_MASK(OPT_G_FIREWALLMD)
#define G_NPH                        G_MASK(OPT_G_NPH)
#define G_SCUTM                      G_MASK(OPT_G_SCUTM)
#define G_DLNA                       G_MASK(OPT_G_DLNA)
#define G_DHCPRELAY                  G_MASK(OPT_G_DHCPRELAY)
#define G_FIREWALL_V6                G_MASK(OPT_G_FIREWALL_V6)
#define G_IPTV                       G_MASK(OPT_G_IPTV)
#define G_ROUTING_V6                 G_MASK(OPT_G_ROUTING_V6)
//============================ G_CFG_SET_2_ ================================
#define G_SAMPLE_64                  G_MASK(OPT_G_SAMPLE_64)   // wait to use...
#define G_SAMPLE_65                  G_MASK(OPT_G_SAMPLE_65)   // wait to use...
#define G_IPCAM                      G_MASK(OPT_G_IPCAM)
#define G_PLC                        G_MASK(OPT_G_PLC)
#define G_SAMBA                      G_MASK(OPT_G_SAMBA)
#define G_SAMBA_CLIENT               G_MASK(OPT_G_SAMBA_CLIENT)
#define G_SAMBA_MOUNT                G_MASK(OPT_G_SAMBA_MOUNT)
#define G_RPCBIND                    G_MASK(OPT_G_RPCBIND)
#define G_NFS                        G_MASK(OPT_G_NFS)
#define G_IPCAMMGR                   G_MASK(OPT_G_IPCAMMGR)
#define G_PPPOESERVER                G_MASK(OPT_G_PPPOESERVER)
#define G_ENCLOUD                    G_MASK(OPT_G_ENCLOUD)
#define G_MDNS_RESPONDER             G_MASK(OPT_G_MDNS_RESPONDER)
#define G_RTSP                       G_MASK(OPT_G_RTSP)
#define G_NATTRAVERSAL               G_MASK(OPT_G_NATTRAVERSAL)
#define G_PACKET_TRANSCEIVER         G_MASK(OPT_G_PACKET_TRANSCEIVER)
#define G_TCRULE                     G_MASK(OPT_G_TCRULE)

#if HAS_IPCAM
#define G_SCHEDULE   (G_SCH)
#define G_HOSTNAME   (G_HOST_DOMAIN_NAME)
#else
#if HASNT_NORMAL_SCHEDULE
#define G_SCHEDULE   (G_SCH|G_WLAN)
#else
#define G_SCHEDULE   (G_SCH|G_FW|G_WLAN|G_LAN)
#endif
#define G_HOSTNAME   (G_WAN|G_HOST_DOMAIN_NAME)
#endif

/* 20120521 Jason: schedule2_1 ~ schedule2_20 use this group to decrease flash write times (first use in WN-AG300DGR Child filter function)*/
#if HAS_DUMMY_SCHEDULE2
#define G_SCHEDULE2   G_DUMMY
#else
#define G_SCHEDULE2   (G_SCH|G_WLAN)
#endif

#if HAS_SCHEDULED_FIREWALL
#define G_FIREWALL (G_FW|G_SCH)
#else
#define G_FIREWALL (G_FW)
#endif

#if DUAL_BAND_CONCURRENT_ONLY_SUPPORT_ONE_WLAN_LED
/* 2011-07-29 Norkay, Only support one WLAN LED, so reload two Radio when 2.4G/5G disable or enable. */
#define G_WLAN11G_RADIO (G_WLAN|G_RA2|G_SCHEDULE)
#define G_RA2_RADIO     (G_WLAN|G_RA2|G_SCHEDULE)
#else
#define G_WLAN11G_RADIO (G_WLAN|G_SCHEDULE)
#define G_RA2_RADIO     (G_RA2)
#endif

#if HAS_PBX_SETTING
#define G_MOBILE_DEVICE_APP (G_AST)
#else
#define G_MOBILE_DEVICE_APP (G_NO)
#endif

/*for AP profile*/
#define MAX_AP_PROFILE_ITEMS    3
#define MAX_MAC_ADDR_LENGTH     18
#define MAX_SSID_LEN            32
#define MAX_KEY_LEN             64
#define MAX_PASSPHRASE_LEN      64 //jaykung

#define MAX_SIMPLE_NVR_SAMBA_PROFILE_ITEMS    4

/*jaykung 20080826 for WEB init page*/
typedef enum
{
	WEB_PAGE_NONE=0,
	WEB_PAGE_INDEX=1,
	WEB_PAGE_DUPLICATED,
	WEB_PAGE_REMOTE,
	WEB_PAGE_MANUFACTURER,
	WEB_PAGE_LOGOUT,
	WEB_PAGE_REPEATER_EASYSETUP
} WEB_INIT_PAGE_NUM;
/*-------------------------------------------------------------------------*/
/*                           enum                                          */
/*-------------------------------------------------------------------------*/

/* yolin for IODATA firmware check */
enum
{
    SYS_VERSION_CHECK_DISABLE=0,
    SYS_VERSION_NOTIFY_ENABLE,
    SYS_VERSION_AUTO_UPDATE
};


/*jaykung 20061017 blacklist status*/
enum
{
	BLKLIST_FIXED_VALUE=1,
	BLKLIST_TMP_VALUE
};

enum
{
	PROTOCOL_TCP=0, PROTOCOL_UDP,PROTOCOL_BOTH,PROTOCOL_ICMP=3,PROTOCOL_ANY=4
};

enum
{
	PROTOCOL_TCP_BIT  = 1<<0,  /* Bit 0 */
	PROTOCOL_UDP_BIT  = 1<<1,  /* Bit 1 */
	PROTOCOL_BOTH_BIT = 0x3,   /* TCP+UDP, Bit 0,1 */
    PROTOCOL_ICMP_BIT = 1<<2,
	PROTOCOL_ANY_BIT  = 1<<3,
};

/*first byte 0:nouse, 1:enable:Aceept, 2: edit, 3:disable, 4:enable:reject*/

#if 0
/*macfilter disable allow reject*/
/*jaykung 20060725*/
enum
{
	FILTER_NONUSED=0, FILTER_ACCEPT, FILTER_EDIT, FILTER_DISABLE_ACCEPT, FILTER_DENY, FILTER_DISABLE_DENY
};
#else
/*jaykung 20070502 TYPE FOR FILTER*/
typedef enum
{
	FILTER_NONUSED=0,
	FILTER_ENABLE,
	FILTER_EDIT,
	FILTER_DISABLE
} FILTER_TYPE;
#endif
/*jaykung 20060808 firstbyte status*/
enum
{
	STATUS_NONUSED=0,STATUS_ENABLE,STATUS_EDIT,STATUS_DISABLE
};

#if 0
/*Joeyli 20080820 EAP Method
	0:PEAP, 1:TLS, 2:TTLS, 3:FAST, 4:MD5*/
enum
{
	EAP_PEAP=0,
	EAP_TLS,
	EAP_TTLS,
	EAP_FAST,
	EAP_MD5
};
#endif

/*0:half hour 1:one hour 2:two hour 3:half day 4:one day
  5:two day 6:one week 7:two weeks 8:forever     */
/*dhcp lease time*/
/*Wlan AutoChannel selection */
enum
{
	HALF_HOUR=0, ONE_HOUR, TWO_HOUR, HALF_DAY, ONE_DAY, TWO_DAY,
	ONE_WEEK, TWO_WEEK, FOREVER_TEN_YEAR, EIGHT_HOUR
};

enum
{
    BAND_W52=0,
    BAND_W53,
    BAND_W56,
    BAND_ALL
};

enum{
    ECO_TYPE_DEFAULT = 0, ECO_TYPE_MANUAL, ECO_TYPE_DISABLE
};

/*jaykung 20070509 QoS for port(priority)*/
typedef enum
{
	PORT_PRIORITY_LOW=0,
	PORT_PRIORITY_HIGH
} PORT_PRIORITY_TYPE;

/*joey 20081212 qos type based*/
typedef enum {
APPS_BASED_QOS=0,
PORT_BASED_QOS,
IP_BASED_QOS
} QOS_BASED;

#if HAS_WAN_3G
/*joey 20081223 3g isp list*/
typedef enum {
CHINA_MOBILE_CHINA=0,
CHT_TAIWAN,
TAIWANMOBILE_TAIWAN,
FETNET_TAIWAN,
TWN_GSM,
ISP_OTHER=100
} WAN_3G_ISP_LIST;

/* Mook 2009-0930 3G band list */
#if HAS_WAN_3G_BAND_SELECTION
typedef enum {
    BAND_AUTO = 0,
    BAND_2G_900_1800,
    BAND_3G_2100,
    BAND_3G_2100_2G_900_1800
} WAN_3G_BAND_LIST;
#endif
#endif /* HAS_WAN_3G */

#if HAS_SC_AUTO_FW_CHECK
typedef enum {
    ACTIOIN_INITIAL = 0,
    ACTIOIN_INSTALL,
    ACTIOIN_REMIND_LATER,
    ACTIOIN_NOT_REMIND_THIS_VERSION,
    ACTIOIN_NOT_REMIND_EVER
} FW_CHECK_ACTION;
#endif

/*interface for static routing*/
typedef enum
{
	SROUTING_LAN=0,
	SROUTING_WAN,
	SROUTING_MAX
} STATIC_ROUTING_INTERFACE;


/* WAN connection type */
/* cfho 2006-0712 */
enum {  WAN_CONNECTION_TYPE_STATICIP=0,
        WAN_CONNECTION_TYPE_PPPOE=1,
        WAN_CONNECTION_TYPE_DHCP=2,
        WAN_CONNECTION_TYPE_PPTP=3,
        WAN_CONNECTION_TYPE_PPPOE_RU=4,
        WAN_CONNECTION_TYPE_PPTP_RU=5,
        WAN_CONNECTION_TYPE_PPPOE_JP=6,
        WAN_CONNECTION_TYPE_3G=7,
        WAN_CONNECTION_TYPE_WIMAX=8,
        WAN_CONNECTION_TYPE_L2TP=9,
        WAN_CONNECTION_TYPE_XGP=10,
		WAN_CONNECTION_TYPE_DSLITE=11,
        WAN_CONNECTION_TYPE_L2TP_IPSEC=12,
        WAN_CONNECTION_TYPE_NULL=13,
};

#if SUPPORT_IPV6_SETTING
enum {
    IPV6_TYPE_LINK_LOCAL=0,
    IPV6_TYPE_STATIC,
	IPV6_TYPE_DHCP,
	IPV6_TYPE_PPPOE,
	IPV6_TYPE_6IN4_TUNNEL,
	IPV6_TYPE_6TO4,
	IPV6_TYPE_STATELESS_AUTO,
	IPV6_TYPE_AUTO_CONFIG,
    IPV6_TYPE_6RD,
    IPV6_TYPE_MAX
};

#if SUPPORT_IPV6_ROUTE
/*interface for ipv6 routing*/
typedef enum
{
	IP6ROUTING_LO=0,
	IP6ROUTING_LAN,
	IP6ROUTING_WAN,
	IP6ROUTING_MAX
} IPV6_ROUTING_INTERFACE;
#endif
#endif

#if HAS_VPN
enum VPN_CONNECTION_TYPE {
    VPN_CONNECTION_TYPE_DISABLED = 0,
    VPN_CONNECTION_TYPE_PPTP = 1,
    VPN_CONNECTION_TYPE_L2TP = 2,
    VPN_CONNECTION_TYPE_IPSEC = 3,
    VPN_CONNECTION_TYPE_L2TP_IPSEC = 4,
    VPN_CONNECTION_TYPE_MAX
};
#endif

#if HAS_PPPOE_JAPAN
enum WAN_PPPOE_TYPE
{
    TYPE_NORMAL = 0,
    TYPE_UNNUMBER_IP,
    TYPE_UNNUMBER_PRIVATE_IP,
};
#endif

/* DDNS connection type */
enum
{
	DDNS_TYPE_3322=0,
	DDNS_TYPE_DHS,
	DDNS_TYPE_DYNDNS,
	DDNS_TYPE_ODS,
	DDNS_TYPE_TZO,
	DDNS_TYPE_GNUDIP,
	DDNS_TYPE_DYNS,
	DDNS_TYPE_ZONEEDIT,
	DDNS_TYPE_DHIS,
	DDNS_TYPE_CYBERGATE,
	DDNS_TYPE_DYNDNS_C,
	DDNS_TYPE_NO_IP,
	DDNS_TYPE_EURODNS,
	DDNS_TYPE_REGFISH,
	DDNS_TYPE_IOBB,
	DDNS_TYPE_DLINK,
	DDNS_TYPE_ENGENIUS
};

typedef enum{
    ACL_TYPE_ALLOW= 0,
    ACL_TYPE_DENY
}ACL_TYPE;

/*joey 20071107 type for web submit */
typedef enum{
    SUBMIT_DEL= 0,
    SUBMIT_ADD,
    SUBMIT_APPLY,
	SUBMIT_EDIT,
	SUBMIT_ACTIVE
}WEB_SUBMIT;

/*jaykung 20060809 sipGetAuthTyep*/
typedef enum
{
	SIP_GETAUTH_TYPE_INPUTBYUSER=1,
	SIP_GETAUTH_TYPE_SAMEASID,
	SIP_GETAUTH_TYPE_AUTOPROVISION
} SIP_GETAUTH_TYPE;

/*jaykung 20060823 Blacklist type for grpinfo*/
typedef enum
{
	GRP_BLACKLIST_NONUSED=0,GRP_BLACKLIST_FULLMATCH,GRP_BLACKLIST_PARTIALMATCH
} BLACKLIST_TYPE;

/*jaykung 20070515 fowarding type*/
typedef enum
{
	FW_DISABLE=0,
	FW_UNCONDITIONAL,
	FW_SPECIAL_CASE,//no answer, on busy
	FW_FOLLOWME
} FORWARDING_TYPE;

/* wlan and lan type*/
typedef enum
{
	LAN_TYPE= 0,
	WAN_TYPE,
	ATHEROS_11A_TYPE,
	ATHEROS_11G_TYPE,
	INTERSIL_11B_TYPE
}INTERFACE_TYPE;

#if 0
typedef enum
{
	WLAN_ENC_NONE, WLAN_ENC_WEP,
	WLAN_ENC_WPA_TKIP, WLAN_ENC_WPA_AES,
	WLAN_ENC_WPA2_TKIP, WLAN_ENC_WPA2_AES,
	WLAN_ENC_WPA_EAP
}WLAN_ENC_TYPE;
#endif
typedef enum
{
	S_AP_GATEWAY=0,
	S_AP_ROUTER
} SCD_APMODE;

/*jaykung 20071012 WscMode*/
typedef enum {
    WSC_MODE_PIN=1,
    WSC_MODE_PBC
    } WLAN_WSCMODE;

/* 20090817 jerry: WdsPhyMode */
typedef enum {
	WDS_PHYMODE_CCK=0,
	WDS_PHYMODE_OFDM,
	WDS_PHYMODE_HTMIX,
	WDS_PHYMODE_GREENFIELD
}WDS_PHYMODE;


typedef enum {
REGDOMAIN_FCC=0,
REGDOMAIN_IC,
REGDOMAIN_ETSI,
REGDOMAIN_SPAIN,
REGDOMAIN_FR,
REGDOMAIN_MKK,
REGDOMAIN_MKK1,
REGDOMAIN_ISRAEL
} REGULAR_DOMAIN;

typedef enum {
WIRELESSMODE_24G_BG=0,
WIRELESSMODE_24G_B,
WIRELESSMODE_5G_A,
// WIRELESSMODE_ABG,
WIRELESSMODE_24G_G=4,
//WIRELESSMODE_5G_ABGN // both band   5
WIRELESSMODE_24G_N=6, //2.4G n
WIRELESSMODE_24G_GN,
WIRELESSMODE_5G_AN, //5G
WIRELESSMODE_24G_BGN, // if check 802.11b.      9
// WIRELESSMODE_24G_AGN,// if check 802.11b.      10
WIRELESSMODE_5G_N=11, // 11n-only with 5G band		11
WIRELESSMODE_5G_AC=13 // 11ac with 5G band		13
} WIRELESS_MODE;

#if HAS_SUPPORT_ATHEROS_WLAN
typedef enum {
ATHEROS_WDS_ROOTAP=0,
ATHEROS_WDS_CLIENT,
ATHEROS_WDS_REPEATER
} ATHEROS_WDS_TYPE;
#endif

#if 0
typedef enum _RT_802_11_PHY_MODE {
	PHY_11BG_MIXED = 0,
	PHY_11B,
	PHY_11A,
	PHY_11ABG_MIXED,
	PHY_11G,
	PHY_11ABGN_MIXED,	// both band   5
	PHY_11N_2_4G,		// 11n-only with 2.4G band   	6
	PHY_11GN_MIXED,	// 2.4G band      7
	PHY_11AN_MIXED,	// 5G  band       8
	PHY_11BGN_MIXED,	// if check 802.11b.      9
	PHY_11AGN_MIXED,	// if check 802.11b.      10
	PHY_11N_5G,			// 11n-only with 5G band		11
} RT_802_11_PHY_MODE;
#endif

typedef enum{
PWR_SAVE_DISABLE=0,
PWR_SAVE_WLAN_ONLY,
PWR_SAVE_ETH_ONLY,
PWR_SAVE_ALL
}POWER_SAVE_MODE;

#if FOR_ZYXEL
typedef enum {
    LANGUAGE_ENG=0,
	LANGUAGE_GER,
	LANGUAGE_FRE,
	LANGUAGE_SPA,
	LANGUAGE_BIG5,
	LANGUAGE_ITA,
	LANGUAGE_CN,
	LANGUAGE_TURKEY,
	LANGUAGE_THAI,
	LANGUAGE_POLISH,
	LANGUAGE_CZECH,
	LANGUAGE_SLOVENE
} LANGUAGE_SET;
#else
typedef enum {
    LANGUAGE_ENG=0,
	LANGUAGE_BIG5
} LANGUAGE_SET;
#endif

typedef enum {
AP_MODE_AP=0,
// AP_MODE_AP_CLIENT=1,
AP_MODE_REPEATER=2,
AP_MODE_WDS=5,
AP_MODE_AP_CLIENT=6
} AP_MODE;

typedef enum {
SNMP_VER_ALL=0,
SNMP_VER1,
SNMP_VER2C,
SNMP_VER3
} SNMP_VER;

typedef enum {
SNMP_LAN=0,
SNMP_WAN,
SNMP_LANWAN,
} SNMP_CONN_MODE;

#if HAS_RADIO_SETTING
typedef enum {
ETH_PURPOSE_LAN,
ETH_PURPOSE_WAN,
} _ETH_PURPOSE_TYPE;
#endif

/* LAN connection type */
typedef enum {
/* static IP */
LAN_CONNECTION_STATIC=0,
/* udhcpc */
LAN_CONNECTION_DHCP,
/* static IP + udhcpd */
LAN_CONNECTION_STATIC_DHCPD,
/* Auto mode
 * 1.Get Response : udhcpc
 * 2. No Response : static IP + udhcpd */
LAN_CONNECTION_AUTO
} LAN_CONNECTION_TYPE;

//enum {WPS_OFF=0,WPS_ON=1};

/* bad site filter */
enum {
	BSFLT_STATUS_OK = 0,
	BSFLT_STATUS_STOP,
	BSFLT_STATUS_END,
	BSFLT_STATUS_NG
};

#if SUPPORT_SX_USB_MODE
enum {
	SX_USB_DISABLE_MODE = 0,
	SX_USB_NAS_MODE,
	SX_USB_NET_USB_MODE,
	SX_USB_ONLY_PS_MODE, /* printer server */
	SX_USB_WEBDAV_MODE
};
#endif
#if HAS_KCODES_FUNCTION
enum {
	KC_USB_DISABLE_MODE = 0,
	KC_USB_SMB_MODE,	/*samba server*/
	KC_USB_NET_USB_MODE,	 /*support printer server and storage*/
	KC_USB_BOTH_MODE	/* Support both smb and print server*/
};
#endif


#if HAS_SUPPORT_DUPLICATE_AP_SETTINGS
enum {
    DUP_AP_NONE = 0,
    DUP_AP_PROCESS,
    DUP_AP_SUCCESS,
    DUP_AP_FAILURE
};
#endif

#if HAS_IODATA_ADVANCED_ECO_SETTINGS
//20110419 Jason: [echo status bitmap table]
//---------------------------------------------------------------------------------------------------------------
// eco_status = 0b  | H       | G 		      | F 	      | E 	   	    | D 	 | C   	 | B 		 | A      |
//  		 status	| WAN     | 5G   WLAN LOW | 5G   WLAN | WLAN LOW    | WLAN   | LAN 	 | LAN Giga  | LED	  |
//  			1/0	| off/on  | [3x3]/[1x1]   | on/off    | [2x2]/[1x1] | on/off | on/off| on/off 	 | on/off |
//---------------------------------------------------------------------------------------------------------------

/* ECO default status
 * bitmap: 0bGFEDCBA
 * A: LED - on/off
 * B: LAN Giga - on/off
 * C: LAN - on/off
 * D: WLAN - on/off
 * E: WLAN LOW - [1x1]/[2x2]
 * F: 5G WLAN - on/off
 * G: 5G WLAN LOW - [1x1]/[2x2]
 * H: WAN - off/on
 */
#define ECO_SETTINGS_LED_MASK 			(1 << 0)
#define ECO_SETTINGS_LAN_RATE_MASK 		(1 << 1)
#define ECO_SETTINGS_LAN_STATUS_MASK 	(1 << 2)
#define ECO_SETTINGS_WLAN_MASK 			(1 << 3)
#define ECO_SETTINGS_WLAN_RATE_MASK		(1 << 4)
#define ECO_SETTINGS_5G_WLAN_MASK       (1 << 5)
#define ECO_SETTINGS_5G_WLAN_RATE_MASK  (1 << 6)
#define ECO_SETTINGS_WAN_STATUS_MASK    (1 << 7)
#endif
/*-------------------------------------------------------------------------*/
/*                           struct                                        */
/*-------------------------------------------------------------------------*/

/* ------------------------------------------------------------------------------------------------------*/
/*     DATA TYPE                         TYPE ID                                                         */
/*     T_INT32                     TYPE_IP, TYPE_NETMASK, TYPE_GATEWAY, TYPE_INT, TYPE_COUNTRYCODE       */
/*     T_UCHAR[6]                  TYPE_MACADDR                                                          */
/*     T_UCHAR[MAX_AP_VALUE_LEN]   TYPE_WEPKEY, TYPE_STR                                                 */
/* ------------------------------------------------------------------------------------------------------*/

typedef enum
{
	TYPE_BOOL=0,	  /* BOOL, 0-1 */
	TYPE_INT,		  /* INT */
	TYPE_UINT,		  /* unsigned INT */
	TYPE_LONG,		  /* Long */
	TYPE_STR,		  /* STR */
    TYPE_STR_32,      /*STR length is 32*/
    TYPE_DOMAINSTR,   /*Domain Str*/
	TYPE_IP,		  /* IP */
#if SUPPORT_IPV6_SETTING
	TYPE_IPV6,
#endif
	TYPE_BLANK_IP,	  /* Blank Ip */
	TYPE_MASK,		  /* MASK*/
    TYPE_BLANK_MASK,  /*BLANK MASK*/
	TYPE_PORT,		  /* Port Number */
	TYPE_SIP_PORT,	  /*Sip Port*/
	TYPE_MACADDR,	  /* MACADDR */
	TYPE_SSID,		  /* SSID, 0-32 length */
#if HAS_SUPPORT_ATHEROS_WLAN
	TYPE_ATH_SSID,		  /* SSID, 0-32 length */
	TYPE_ATH_WDS_TYPE,   /*0: ROOTAP, 1:WDS CLIENT*/
#endif
    TYPE_VLANTAG,      /*vlanid 1~4096*/
	TYPE_WEPKEY,	  /* WEPKEY, Hex code */
	TYPE_KEYID,		  /* Default Key Id, 1-4 */
    TYPE_BEACON,      /* Beacon Interval, 20-1024 */
	TYPE_ATHEROS_BEACON, /*Beacon Interval, 20-1024*/
	TYPE_DTIM,		  /* DTIM Period, 1-255 */
	TYPE_FRAGMENT,	  /* Wireless Fragment threshold, 256-2346 */
	TYPE_RTS_THRESHOLD,	/* 0-2346*/
	TYPE_24G_CHANNEL, /* 11b Channel */
	TYPE_11M_DATARATE,	   /* 11Mbits Data Rate, */
	TYPE_5G_CHANNEL,	   /* 5G Channel */
	TYPE_11A_DATARATE,	   /* 0,6,9,12,18,24,36,48,54 */
	TYPE_11G_DATARATE,	   /*0,1,2,5,11,6,9,12,18,24,36,48,54*/
	TYPE_INTERSIL_AUTH,	   /* 11b auth mode, 0:open 1:share 3:dot1x ?? */
	TYPE_INTERSIL_WEP_TYPE,/* 11b wep type, 0, 64, 128bits*/
	TYPE_INTERSIL_POWER_LV,/* 11b power level, 0:Normal 1:20dBm 2:17dBm 3:13dBm*/
	TYPE_PREAMBLE_TYPE,		/*1:short 2:long*/
	TYPE_RT2860_AUTH,	   /**/
	TYPE_ATHEROS_WEP_TYPE, /* 0 64 128 152bits */
	TYPE_ATHEROS_TXPOWER,  /*unit:percentage*/
#if SUPPORT_GREENMODE_SETTING
    TYPE_ATHEROS_GRNMODE,   /*enable:greenmode*/
#endif
#if HAS_TX_POWER_DBM_SETTING
	TYPE_TXPOWER_DBM,   /*unit:dbm*/
#endif
	TYPE_COUNTRYCODE, /* COUNTRYCODE */
	TYPE_APMODE,			/*AP or simple WDS*/
    TYPE_SYSOPMODE,         /*AP, AP Bridge-Point to Point, ... , AP Bridge_WDS*/
#if HAS_RADIO_SETTING
	TYPE_RADIOOPMODE,		/*AP, CB, CR, WPS Bridge, Universal Repeater, disable*/
	TYPE_ETH_PURPOSE,		/*what purpose the Eth port is*/
#endif
    TYPE_PWRSAVEMODE,       /*0: disable, 1: WLAN only, 2:Ethernet Only, 3: WLAN+Ethernet */
#if FOR_COREGA || FOR_ZYXEL
	TYPE_LANGUAGE,			/*Language for WEB*/
#endif
    TYPE_WDS_SEC_TYPE,  /*wds sec mode*/
	TYPE_PAE_AUTHTYPE,	   /* 0:PEAP, 1:TLS, 2:TTLS, 3:EAP-FAST, 4:MD5*/
	TYPE_PAE_REKEY_SEC,	   /* 100-3600 */
	TYPE_DHCP_LEASE_TIME,  /*0-8  */
	TYPE_FILTER_TYPE,  /*0:disable, 1:Aceept, 2: Reject */
	TYPE_FILTER_MODE, /*0:whitelist 1:blacklist*/
#if HAS_HTTP_PROXY
	TYPE_URLFILTER_MODE,/*0:whitelist 1:blacklist*/
#endif
	TYPE_SNMP_ACCESS,	   /* SNMP Access, 0:read, 1:write */
	TYPE_SNMP_VER,		   /* SNMP Version 0: all, 1:v1, 2:v2 3:usm*/
	TYPE_WAN_CONN,		   /*WAN connection type, 0: Static, 1:PPPoE, 2: DHCPc*/
	TYPE_WANINTERFACE,
	TYPE_DDNS,				/*DDNS operator, e.g. dyndns.org, toz.com */
	TYPE_IP_FILTER,		   /* ip pairs for IP filtering rule*/
	TYPE_MAC_FILTER,		/* MAC address for IP filtering rule*/
	TYPE_NAT_VSERVER,		/*NAT Virtual server*/
	TYPE_URL_FILTER,		/* URL filter */
#if HAS_URLFILTER_SUPPORT_IP_RANGE
    TYPE_URL_FILTER_IP,     /*URL filter support to limit ip range*/
#endif
#if HAS_MAC_PASSTHROUGH
	TYPE_MAC_BRIDGING,		/* MAC bridging */
#endif
#if HAS_PRIORITY_WEB_ACCOUNT
	TYPE_WEB_ACCOUNT,       /* WEB Account */
#endif
	TYPE_L7_FILTER,			/*L7 filter*/
#if HAS_WLAN_WMM_QOS
	TYPE_WMMQOS_RULE,		/* WLAN QoS (using WMM) */
#endif
	TYPE_PORT_QOS,
	TYPE_PORT_QOS_B,
#if HAS_QOS_HWPORT
	TYPE_QOS_HWPORT,       /* HW Port Based Qos*/
#endif
	TYPE_RT2860_ENC_TYPE,  /*0: None, 1: WEP 2: WPA-PSK TKIP 3: WPA-EAP... */
	TYPE_WPA_PASSPHRASE,	   /* WPA Pre-Shared Key, between 8 and 63 char of ASCII text,or a 64 character Hex string, start with "0x" (total size is 2+64=66)*/
	TYPE_RT2860_MODE,	/*bg=0,b,a,abg,g,abgn,n,gn,an,bgn,agn*/
	TYPE_ATH11ABG_MODE,
	TYPE_DOMAIN,
	TYPE_MTU,
	TYPE_IDLETIME,
	TYPE_AST_MAXCALL,	   /* Asterisk Max calls 4-20 */
#if HAS_PBX_SETTING
	TYPE_USER,
	TYPE_PBX,
    TYPE_PBX_REG,
#if 0
	TYPE_CALL_MONITOR,
	TYPE_SIP_REGISTRY,
	TYPE_SIP_PEERS_INFO,
	TYPE_SIP_CALL_RECORD,
#endif
#endif
	/** VOIP TYPE
    TYPE_USERCFNUM,
    TYPE_GROUP,
    TYPE_GROUPBLACKLIST,
    TYPE_OUTBOUNDSIPCONF,
    TYPE_DIALPLANCONF,
    **/
#if HAS_SYSTEM_SCHEDULE_FUNCTION || HAS_PARENTAL_CONTROL
	TYPE_SCH,
#if HAS_SCHEDULE_EXTEND
	TYPE_SCH_B,
#endif
#endif
#if HAS_SYSTEM_SCHEDULE2_FUNCTION
	TYPE_SCH2,
#endif
	TYPE_BANDWIDTH,
	TYPE_DHCP_MACTOIP,
	TYPE_STATICROUTING,
	TYPE_TIMEZONE,
    TYPE_DAYLIGHTMONTH,
    TYPE_DAYLIGHTDAY,
#if HAS_MANUAL_TIME_SETTING || HAS_WAN_CONNECT_TIME
	TYPE_TIME_INFO,
#endif
	TYPE_SYSINFO,
	TYPE_POLLING,/*SNTP 1 - 4*/
	TYPE_KEY_LIFETIME,/*Range from 0 to 86400 */
#if HAS_TOS_DIFFSERV
	TYPE_TOS_DIFFSERV,
#endif
#if HAS_SIP_CDR
	TYPE_CDR_RESERVE_DAYS,
#endif
	TYPE_ISP_CONN,
	TYPE_WLAN_CHANNEL,
#if HAS_SUPPORT_CHANNEL_11ABG
	TYPE_WLAN11ABG_CHANNEL,
#endif
	//TYPE_SCH_FOR_PBX,
	//TYPE_LOGEVENT,
    TYPE_REGDOMAIN,
#if HAS_SMTP
	TYPE_SMTP_AUTE,
#endif
	TYPE_RIP_MODE,
	TYPE_ALG_SUPPORT,
	TYPE_DOS_FEATURE,
	TYPE_DOS_PORT_SCAN,
	TYPE_DMZ,
	TYPE_SPEC_APP,
    TYPE_AUTOCH_CHKTIME,
#if HAS_AP_PROFILE
//    TYPE_AP_PROFILE,
#endif
	TYPE_WMM,
#ifdef HAS_PPPOE_JAPAN
	TYPE_DUALWANROUTING,
#endif
#if HAS_IMQ_SUPPORT
    TYPE_QOS_BASED,
#endif
#if HAS_WAN_3G
    TYPE_3G_ISP,
#if HAS_WAN_3G_AUTOMOUNT
    TYPE_3G_AUTO_MOUNT,
#endif
#if HAS_WAN_3G_BAND_SELECTION
    TYPE_3G_BAND, /* 2G 900/1800, 3G 2100, 3G 2100 2G 900/1800 */
#endif
#endif
#if HAS_VPN
    TYPE_VPN,
#if HAS_VPN_IPSEC
    TYPE_IPSEC,
    TYPE_IPSEC_CONN_BAS,
    TYPE_IPSEC_CONN_ADV,
#endif
#if HAS_VPN_L2TP
    TYPE_L2TP,
#endif
#if HAS_VPN_PPTP /* [20101222 Andy Yu] */
	TYPE_PPTP,
#endif
#if HAS_VPN_L2TP || HAS_VPN_PPTP
	TYPE_VPN_USER,
#endif
#endif /* HAS_VPN */
#if GEN_ADMIN_PASSWORD_BY_MACADDR || GEN_ADMIN_PASSWORD_BY_SN
    TYPE_ADMIN_PWD,
#endif
#if HAS_BAD_SITE_FILTER
    TYPE_BSFLT_USR,
	TYPE_BSFLT_GRP,
#endif
#if HAS_WAN_LINKCHECK
    TYPE_WAN_CHECK,
#endif
#if HAS_SC_AUTO_FW_CHECK
    TYPE_AUTO_FW_ACTION,
#endif
#if HAS_DOMAIN_ROUTING
	TYPE_DOMAINROUTING,
#endif
#if HAS_HTTP_SYSTEM_NAME
	TYPE_HTTP_SYS_NAME,
#endif
#if HAS_NAT_SESSIONS_CTRL
    TYPE_NAT_SESSION,		/* NAT Sessions */
#endif
#if HAS_WAN_TO_WAN_ACL
    TYPE_WAN_TO_WAN_ACL,		/* WAN to WAN ACL */
#endif
#if HAS_TR_AGENT
	TYPE_TR069_OUI,
#endif
#if SUPPORT_IPV6_SETTING
	TYPE_IPV6_CONN,
#if SUPPORT_IPV6_IPFILTER || KNL2621_SUPPORT_IPV6_PREFIX_FILTER
	TYPE_IPV6_FILTER_MODE, /*0:whitelist 1:blacklist*/
	TYPE_IPV6_IP_FILTER,
#endif
#if SUPPORT_IPV6_ROUTE
	TYPE_IPV6_ROUTING,
#endif
#endif
#if HAS_ADV_NAT
	TYPE_ADV_PORT_FORWARDING,
	TYPE_ADV_PORT_FORWARDING_EXT,
#endif
#if HAS_ACCESS_CONTROL
	TYPE_ACCESS_CONTROL,		/* access control */
	TYPE_ACCESS_CONTROL_IP,		/* access control IP */
	TYPE_ACCESS_CONTROL_MAC,		/* access control MAC */
#endif
#if HAS_INBOUND_FILTER
	TYPE_INBOUND_FILTER_SETTING,
	TYPE_INBOUND_FILTER_IP_ADDRESS,
#endif
#if HAS_NPH_MODULE && HAS_NPH_VLAN_SWITCH
	TYPE_NPH,
#endif
#if HAS_ARP_PROXY
	TYPE_ARP_PROXY,
#endif
#if HAS_ACCESS_CONTROL_BELKIN
	TYPE_ACCESS_CONTROL_BELKIN,		/* access control for BELKIN*/
	TYPE_ACCESS_CONTROL_SERVICE,	/* access control Service */
	TYPE_ACCESS_CONTROL_URL,		/* access control URL Filter */
#endif
#if HAS_PARENTAL_CONTROL
	TYPE_PARENTAL_CONTROL,
	TYPE_PARENTAL_CONTROL_IP,
	TYPE_PARENTAL_CONTROL_MAC,
	TYPE_PARENTAL_CONTROL_SERVICE,
	TYPE_PARENTAL_CONTROL_URL,
#endif
#if HAS_SUPPORT_UBICOM_STREAMENGINE
	TYPE_STREAMENGINE_QOS,
#endif
#if SUPPORT_WPS_GENERATE_NEW_PIN
	TYPE_WPS_PIN,
#endif
#if SUPPORT_DEF_CAFE_PSK
	TYPE_CAFE_PSK,
#endif
#if HAS_RTL_VLAN_SUPPORT
	TYPE_RTL_VLAN,
#endif
#if HAS_PPPOE_ACCOUNT_PROFILE
	TYPE_PPPOE_PRFILE,
#endif
#if SUPPORT_BLUETOOTH_MODULE
	TYPE_BT_PIN,
#endif
#if HAS_IPCAM
	TYPE_IPCAM_MD_WINDOW,
	TYPE_IPCAM_SCHEDULE,
#endif
#if SUPPORT_PPPOE_SERVER_FOR_LOCAL
	TYPE_PPPOE_SERVER_ACCOUNT,
#endif
	TYPE_MAX_TYPE		  /* !!! DO NOT ADD ANY ITEM BELOW THIS ID !!! */
} AP_CFG_TYPE;
#define TYPE_NONE TYPE_MAX_TYPE

//TYPE_WLAN_MODE,	  /* Wlan IF running mode, AP, STA, WDS (need two cards)*/
#endif


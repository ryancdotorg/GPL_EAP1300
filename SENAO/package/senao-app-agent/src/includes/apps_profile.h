#if __cplusplus
extern "C" {
#endif

#ifndef  _APPS_PROFILE_H_
#define  _APPS_PROFILE_H_

/********************************/
/*   INCLUDED COMPONENTS        */
/********************************/
#define APPS_INCLUDE_DOWNLOAD
//<< APC_APPSH_INCLUDE
/********** Access List **********/
//<< APC_APPSH_ACCESSHOST
#define APPS_ACCESS_LIST_ENTRIES    2
//>> APC_APPSH_ACCESSHOST

/********** DHCP Client Config **********/
//<< APC_APPSH_DHCPC
#define APPS_DHCPC_MAX_DNS_NUM          4
#define APPS_DHCPC_MAX_GATEWAY_NUM      2
//>> APC_APPSH_DHCPC

/********** DHCP Server Config **********/
//<< APC_APPSH_DHCPS
#define APPS_NUM_OF_DHCP_POOL	        1
#define APPS_NUM_OF_STATIC_IP	        0
//>> APC_APPSH_DHCPS

/********** DNS Relay Config **********/
//<< APC_APPSH_DNSRELAY
#define APPS_NUM_OF_DNS                  3
#define APPS_NUM_OF_DNS_DYNAMIC          4
//>> APC_APPSH_DNSRELAY

/********** Firewall Config **********/
//<< APC_APPSH_FIREWALL
#define APPS_NUM_OF_FILTER_ENTRY          10
#define APPS_NUM_OF_HACKER_LOG            64
//>> APC_APPSH_FIREWALL

/********** IP Config **********/
//<< APC_APPSH_IP
#define APPS_IF_ADM0_LAN_IP            0xc0a80101  // 192.168.1.1
#define APPS_IF_ADM0_LAN_MASK          0xffffff00  // 255.255.255.0
#define APPS_IF_ADM0_LAN_NET           (APPS_IF_ADM0_LAN_IP & APPS_IF_ADM0_LAN_MASK)
#define APPS_IF_ADM1_WAN_IP            0x00000000  // 0.0.0.0
#define APPS_IF_ADM1_WAN_MASK          0x00000000  // 0.0.0.0
#define APPS_IF_ADM1_WAN_NET           (APPS_IF_ADM1_WAN_IP & APPS_IF_ADM1_WAN_MASK)
#define APPS_IF_ALIAS_NUM              5
//>> APC_APPSH_IP

/********** Ethernet Filter Config **********/
//<< APC_APPSH_ETHFTR
#define APPS_NUM_OF_ETHFTR_ENTRY          32
//>> APC_APPSH_ETHFTR

/********** NAT Config **********/
//<< APC_APPSH_NAT
#define APPS_NAT_RD_LINK_NUM            0
#define APPS_NAT_VS_LINK_NUM            10
#define APPS_NAT_VDMZ_LINK_NUM          6
#define APPS_NAT_TRIGGER_LINK_NUM       10
//>> APC_APPSH_NAT

/********** PPPoE Config **********/
//<< APC_APPSH_PPPOE
//>> APC_APPSH_PPPOE

/********** PPTP Config **********/
//<< APC_APPSH_PPTP
//>> APC_APPSH_PPTP

/********** RIP Config **********/
//<< APC_APPSH_RIP
//>> APC_APPSH_RIP

/********** Route Config **********/
//<< APC_APPSH_ROUTE
#define APPS_NUM_OF_STATIC_ROUTE_ENTRY  10
//>> APC_APPSH_ROUTE

/********** SNTP Client Config **********/
//<< APC_APPSH_SNTPC
extern  int Tz_DefaultValue;
#define APPS_SNTPC_POLLING_TIME         86400
#define APPS_SNTPC_USERCFG_SNTPSVR_NUM  4
//>> APC_APPSH_SNTPC

/********** TFTP Client Config **********/
//<< APC_APPSH_TFTPC
#define APPS_TIMEOUT_OF_TFTPC       3
#define APPS_PORT_OF_TFTPC          69
//>> APC_APPSH_TFTPC

/********** TFTP Server Config **********/
//<< APC_APPSH_TFTPS
#define APPS_TIMEOUT_OF_TFTPS       15
#define APPS_PORT_OF_TFTPS          69
//>> APC_APPSH_TFTPS

/********** USER Config **********/
//<< APC_APPSH_USER
#define APPS_NUM_OF_USERS               10
#define APPS_USERNAME_MIN_SIZE          3
#define APPS_USERNAME_MAX_SIZE          12
#define APPS_PASSWORD_MIN_SIZE          3
#define APPS_PASSWORD_MAX_SIZE          12

#define APPS_DEFAULT_ADMIN_NAME         "admin"
#define APPS_DEFAULT_ADMIN_PASSWORD     "admin"

#define APPS_DEFAULT_GUEST_NAME         ""
#define APPS_DEFAULT_GUEST_PASSWORD     ""

#define APPS_HIGHEST_USER_PERM          3
#define APPS_LOWEST_USER_PERM           1
//>> APC_APPSH_USER

/********** Web Config **********/
//<< APC_APPSH_WEB
#define APPS_HTTP_LISTEN_PORTS    2
//>> APC_APPSH_WEB

//>> APC_APPSH_INCLUDE
#define APPS_INCLUDE_CLI

/*******************************
  Interface table of this system
 *******************************/
//<< APC_APPSH_IF
#define APPS_IF_ADM0_LAN         "adm0"
#define APPS_IF_ADM1_WAN         "adm1"
#define APPS_IF_PPPOE0         "pppoe0"
#define APPS_IF_PPTP0         "pptp0"
//>> APC_APPSH_IF

/*******************************
  Profile constant of this system
 *******************************/
/* User Config */
#ifdef  APPS_INCLUDE_USER
//<< APC_APPSH_USER
#define APPS_NUM_OF_USERS               10
#define APPS_USERNAME_MIN_SIZE          3
#define APPS_USERNAME_MAX_SIZE          12
#define APPS_PASSWORD_MIN_SIZE          3
#define APPS_PASSWORD_MAX_SIZE          12

#define APPS_DEFAULT_ADMIN_NAME         "admin"
#define APPS_DEFAULT_ADMIN_PASSWORD     "admin"

#define APPS_DEFAULT_GUEST_NAME         ""
#define APPS_DEFAULT_GUEST_PASSWORD     ""

#define APPS_HIGHEST_USER_PERM          3
#define APPS_LOWEST_USER_PERM           1
//>> APC_APPSH_USER
#endif

/* Download Config */
#ifdef  APPS_INCLUDE_DOWNLOAD
#endif

/* PPPoE Config */
#ifdef  APPS_INCLUDE_PPPOE
//<< APC_APPSH_PPPOE
//>> APC_APPSH_PPPOE
#endif

/* PPTP Config */
#ifdef  APPS_INCLUDE_PPTP
//<< APC_APPSH_PPTP
//>> APC_APPSH_PPTP
#endif

/* DHCP Client Config */
#ifdef  APPS_INCLUDE_DHCP_CLIENT
//<< APC_APPSH_DHCPC
#define APPS_DHCPC_MAX_DNS_NUM          4
#define APPS_DHCPC_MAX_GATEWAY_NUM      2
//>> APC_APPSH_DHCPC
#endif

/* IP Config */
#ifdef  APPS_INCLUDE_IP
//<< APC_APPSH_IP
#define APPS_IF_ADM0_LAN_IP            0xc0a80101  // 192.168.1.1
#define APPS_IF_ADM0_LAN_MASK          0xffffff00  // 255.255.255.0
#define APPS_IF_ADM0_LAN_NET           (APPS_IF_ADM0_LAN_IP & APPS_IF_ADM0_LAN_MASK)
#define APPS_IF_ADM1_WAN_IP            0x00000000  // 0.0.0.0
#define APPS_IF_ADM1_WAN_MASK          0x00000000  // 0.0.0.0
#define APPS_IF_ADM1_WAN_NET           (APPS_IF_ADM1_WAN_IP & APPS_IF_ADM1_WAN_MASK)
#define APPS_IF_ALIAS_NUM              5
//>> APC_APPSH_IP
#endif

/* NAT Config */
#ifdef APPS_INCLUDE_NAT
//<< APC_APPSH_NAT
#define APPS_NAT_RD_LINK_NUM            0
#define APPS_NAT_VS_LINK_NUM            10
#define APPS_NAT_VDMZ_LINK_NUM          6
#define APPS_NAT_TRIGGER_LINK_NUM       10
//>> APC_APPSH_NAT
#endif

/* Route Config */
#ifdef APPS_INCLUDE_ROUTE
//<< APC_APPSH_ROUTE
#define APPS_NUM_OF_STATIC_ROUTE_ENTRY  10
//>> APC_APPSH_ROUTE
#endif

/* DNS Relay Config */
#ifdef APPS_INCLUDE_DNS_RELAY
//<< APC_APPSH_DNSRELAY
#define APPS_NUM_OF_DNS                  3
#define APPS_NUM_OF_DNS_DYNAMIC          4
//>> APC_APPSH_DNSRELAY
#endif

/* DHCP Server Config */
#ifdef  APPS_INCLUDE_DHCP_SERVER
//<< APC_APPSH_DHCPS
#define APPS_NUM_OF_DHCP_POOL	        1
#define APPS_NUM_OF_STATIC_IP	        0
//>> APC_APPSH_DHCPS
#endif

/* SNTP Client Config */
#ifdef APPS_INCLUDE_SNTP_CLIENT
//<< APC_APPSH_SNTPC
extern  int Tz_DefaultValue;
#define APPS_SNTPC_POLLING_TIME         86400
#define APPS_SNTPC_USERCFG_SNTPSVR_NUM  4
//>> APC_APPSH_SNTPC
#endif

/* Firewall Config */
#ifdef APPS_INCLUDE_FIREWALL
//<< APC_APPSH_FIREWALL
#define APPS_NUM_OF_FILTER_ENTRY          10
#define APPS_NUM_OF_HACKER_LOG            64
//>> APC_APPSH_FIREWALL
#endif

/* RIP Config */
#ifdef APPS_INCLUDE_RIP
//<< APC_APPSH_RIP
//>> APC_APPSH_RIP
#endif

/* HTTP and Web Config */
#ifdef APPS_INCLUDE_WEB
//<< APC_APPSH_WEB
#define APPS_HTTP_LISTEN_PORTS    2
//>> APC_APPSH_WEB
#endif

/* Access List */
#ifdef  APPS_INCLUDE_ACCESS_HOST
//<< APC_APPSH_ACCESSHOST
#define APPS_ACCESS_LIST_ENTRIES    2
//>> APC_APPSH_ACCESSHOST
#endif

/* Ethernet Filter Config */
#ifdef APPS_INCLUDE_ETHER_FILTER
//<< APC_APPSH_ETHFTR
#define APPS_NUM_OF_ETHFTR_ENTRY          32
//>> APC_APPSH_ETHFTR
#endif

/* TFTP Client Config */
#ifdef APPS_INCLUDE_TFTP_CLIENT
//<< APC_APPSH_TFTPC
#define APPS_TIMEOUT_OF_TFTPC       3
#define APPS_PORT_OF_TFTPC          69
//>> APC_APPSH_TFTPC
#endif

/* TFTP server Config */
#ifdef APPS_INCLUDE_TFTP_SERVER
//<< APC_APPSH_TFTPS
#define APPS_TIMEOUT_OF_TFTPS       15
#define APPS_PORT_OF_TFTPS          69
//>> APC_APPSH_TFTPS
#endif

#endif

#if __cplusplus
}
#endif

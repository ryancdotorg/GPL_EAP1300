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
typedef enum linkstatus_t {
	LINK_INIT = 0,
	STATUS_OK,
	STATUS_FAIL_DEV_DNS = 10,
	STATUS_FAIL_DEV_IF_GW,
	ICMP_CREATE_ERR = 100,
	ICMP_SEND_ERR,
	ICMP_RCV_ERR,
	ICMP_DATA_ERR,
	ICMP_NO_RESPONSE,
	ARP_CREATE_ERR = 200,
	ARP_SEND_ERR,
	ARP_DATA_ERR,
	ARP_RCV_ERR,
	ARP_DUPLICATE,
	DNS_CREATE_ERR = 300,
	DNS_SEND_ERR,
	DNS_RCV_ERR,
	DNS_DATA_ERR,
	DNS_NO_RESPONSE,
	HTTP_DATA_ERR = 400,
	HTTP_DNS_ERR,
	HTTP_CREATE_ERR,
	HTTP_SEND_ERR,
	HTTP_RCV_ERR,
	HTTP_CONN_ERR,
	HTTP_PAGE_ERR,
	HTTP_NO_RESPONSE,
	HTTPS_DATA_ERR = 500,
	HTTPS_DNS_ERR,
	HTTPS_CREATE_ERR,
	HTTPS_SEND_ERR,
	HTTPS_HANDSHAKE_ERR,
	HTTPS_RCV_ERR,
	HTTPS_CONN_ERR,
	HTTPS_PAGE_ERR,
	HTTPS_NO_RESPONSE,
	PHYLINK_DATA_ERR = 600,
	PHYLINK_CREATE_ERR,
	PHYLINK_DOWN,
	PHYLINK_UNKNOWN,
	MESHAP_FINISH = 700, /* listen_finish file exist */
	MESHAP_CONNECTED = 701, /* listen connected file exist */
}linkstatus;

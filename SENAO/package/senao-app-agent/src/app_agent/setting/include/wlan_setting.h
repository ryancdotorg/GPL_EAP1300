/****************************************************************************
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
;----------------------------------------------------------------------------
;
;    Project : app_agent
;    Creator : Jerry Chen
;    File    : wlan_setting.h
;    Abstract:
;
;       Modification History:
;       By              Date     	Ver.   	Modification Description
;       --------------- -------- 	-----  	-------------------------------------
;		Jerry			2012/09/10			First commit
;****************************************************************************/

#ifndef _WLAN_SETTING_H_
#define _WLAN_SETTING_H_

/*--------------------------------------------------------------------------*/
/*                           INCLUDE HEADER FILES                           */
/*--------------------------------------------------------------------------*/
#include "deviceinfo.h"
#include "gconfig.h"
#include "../../appagents.h"

/*--------------------------------------------------------------------------*/
/*                                PARAMETER                                 */
/*--------------------------------------------------------------------------*/
#define MAX_SITE_SURVEY_INFO_NUM	32

/*--------------------------------------------------------------------------*/
/*                                STRUCTURE                                 */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*                            FUNCTION PROTOTYPE                            */
/*--------------------------------------------------------------------------*/
int get_wlan_radios_cb(HTTPS_CB *pkt);
int get_radio_id(HTTP_PACKET_CONTENT_FORMAT format, char *query_str);
int get_ssid_id(HTTP_PACKET_CONTENT_FORMAT format, char *query_str);
int get_wlan_authentication(char *auth_type_str, int *authentication);
int get_wlan_encryption(char *encryption_type_str, int *encryption);
int is_valid_wlan_security(int encryption, int authentication);
int get_wlan_settings(HTTPS_CB *pkt, WLAN_RADIO_SETTINGS_T *settings, char **result);
int get_wlan_radio_settings_cb(HTTPS_CB *pkt);
int set_wlan_radios_cb(HTTPS_CB *pkt);
int get_wlan_radio_security_cb(HTTPS_CB *pkt);
int set_wlan_radio_security_cb(HTTPS_CB *pkt);
int get_wlan_radio_settings_cb(HTTPS_CB *pkt);
int set_wlan_radio_settings_cb(HTTPS_CB *pkt);
int get_access_control_list_cb(HTTPS_CB *pkt);
int set_access_control_list_cb(HTTPS_CB *pkt);
int add_access_control_list_cb(HTTPS_CB *pkt);
int delete_access_control_list_cb(HTTPS_CB *pkt);
int get_wlan_station_status_cb(HTTPS_CB *pkt);
int get_wlan_sitesurvey_cb(HTTPS_CB *pkt);
int set_wlan_connection_cb(HTTPS_CB *pkt);
#if 0 // not support
int set_wlan_radio_settings_cb(HTTPS_CB *pkt);
int set_wlan_radio_security_cb(HTTPS_CB *pkt);
int get_wlan_radio_information_cb(HTTPS_CB *pkt);
#endif
int get_wifiinfo_cb(HTTPS_CB *pkt);
int set_wifiinfo_cb(HTTPS_CB *pkt);
int kick_wireless_client_by_mac_cb(HTTPS_CB *pkt);
int get_wifioptlist_cb(HTTPS_CB *pkt);
int set_countrycode_cb(HTTPS_CB *pkt);
#endif

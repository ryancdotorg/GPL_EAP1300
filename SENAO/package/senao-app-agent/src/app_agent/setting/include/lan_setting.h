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
;    File    : lan_setting.h
;    Abstract:
;
;       Modification History:
;       By              Date     	Ver.   	Modification Description
;       --------------- -------- 	-----  	-------------------------------------
;		Jerry			2012/09/10			First commit
;****************************************************************************/
#ifndef _LAN_SETTING_H_
#define _LAN_SETTING_H_

/*--------------------------------------------------------------------------*/
/*                           INCLUDE HEADER FILES                           */
/*--------------------------------------------------------------------------*/
#include "deviceinfo.h"
#include "../../appagents.h"
#include "gconfig.h"

/*--------------------------------------------------------------------------*/
/*                                STRUCTURE                                 */
/*--------------------------------------------------------------------------*/
typedef struct _dhcpLeaseTimeData_{
    char title[15+1];
    char config[15+1];
} dhcpLeaseTimeData_t;

/*--------------------------------------------------------------------------*/
/*                            FUNCTION PROTOTYPE                            */
/*--------------------------------------------------------------------------*/
int get_lan_settings(HTTPS_CB *pkt);
int get_client_status(HTTPS_CB *pkt);
int set_lan_settings(HTTPS_CB *pkt);

int get_lan_access_control_list_cb(HTTPS_CB *pkt);
int set_lan_access_control_list_cb(HTTPS_CB *pkt);
int add_lan_access_control_list_cb(HTTPS_CB *pkt);
int delete_lan_access_control_list_cb(HTTPS_CB *pkt);
#if HAS_AP
int get_ap_wan_settings_cb(HTTPS_CB *pkt);
int set_ap_wan_settings_cb(HTTPS_CB *pkt);
#endif
int get_upnp_settings_cb(HTTPS_CB *pkt);
int set_upnp_settings_cb(HTTPS_CB *pkt);
#if 0 // not support HAS_PARENTAL_CONTROL
int get_blocked_client_list_cb(HTTPS_CB *pkt);
int edit_blocked_client_list_cb(HTTPS_CB *pkt);
int delete_blocked_client_list_cb(HTTPS_CB *pkt);
#endif
int get_blocked_client_list_cb(HTTPS_CB *pkt);
int edit_blocked_client_list_cb(HTTPS_CB *pkt);
int delete_blocked_client_list_cb(HTTPS_CB *pkt);
bool isDuplicateMac(BLOCKED_CLIENT_LIST_T *setting);
int get_ap_wan_all_settings_cb(HTTPS_CB *pkt);
int set_ap_wan_all_settings_cb(HTTPS_CB *pkt);
#endif

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
;    File    : wan_setting.h
;    Abstract:
;
;       Modification History:
;       By              Date     	Ver.   	Modification Description
;       --------------- -------- 	-----  	-------------------------------------
;		Jerry			2012/09/10			First commit
;****************************************************************************/

#ifndef _WAN_SETTING_H_
#define _WAN_SETTING_H_

/*--------------------------------------------------------------------------*/
/*                           INCLUDE HEADER FILES                           */
/*--------------------------------------------------------------------------*/
//#include "hnap.h"
#include "deviceinfo.h"
#include "gconfig.h"
#include "../../appagents.h"

/*--------------------------------------------------------------------------*/
/*                                DEFINITION                                */
/*--------------------------------------------------------------------------*/
#define NAT_TRAVERSAL_CONF "/tmp/natt.conf"
#define NAT_TRAVERSAL_TOP_CONF "/tmp/natt_top.conf"

/*--------------------------------------------------------------------------*/
/*                                STRUCTURE                                 */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*                            FUNCTION PROTOTYPE                            */
/*--------------------------------------------------------------------------*/
int get_ipv4_wan_status_cb(HTTPS_CB *pkt);
int get_ipv4_wan_settings_cb(HTTPS_CB *pkt);
int set_ipv4_wan_settings_cb(HTTPS_CB *pkt);
int get_wan_status_cb(HTTPS_CB *pkt);
int get_wan_settings_cb(HTTPS_CB *pkt);
int set_wan_settings_cb(HTTPS_CB *pkt);
#if SUPPORT_IPV6_SETTING
int get_ipv6_wan_status_cb(HTTPS_CB *pkt);
int get_ipv6_wan_settings_cb(HTTPS_CB *pkt);
int set_ipv6_wan_settings_cb(HTTPS_CB *pkt);
#endif
#endif

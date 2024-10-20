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

/*--------------------------------------------------------------------------*/
/*                           INCLUDE HEADER FILES                           */
/*--------------------------------------------------------------------------*/
#include "deviceinfo.h"
#include "../../appagents.h"
#include "gconfig.h"

/*--------------------------------------------------------------------------*/
/*                               DEFINITION                                 */
/*--------------------------------------------------------------------------*/
#define STR_DDNS_TYPE_3322             "3322"
#define STR_DDNS_TYPE_DYNDNS    "DynDNS"
#define STR_DDNS_TYPE_ZONEEDIT  "ZoneEdit"
#define STR_DDNS_TYPE_NO_IP             "NoIP"
#define STR_DDNS_TYPE_ENGENIUS  "EnGeniusDDNS"

static char *all_ddns_type[]={STR_DDNS_TYPE_3322, STR_DDNS_TYPE_DYNDNS,STR_DDNS_TYPE_ZONEEDIT,STR_DDNS_TYPE_NO_IP STR_DDNS_TYPE_ENGENIUS};
#define DDNS_TYPE_MAX sizeof(all_ddns_type)/sizeof(all_ddns_type[0])
static int en_ddns_refresh_time[]={3,6,9,12,24};
#define EN_DDNS_REFRESH_TIME_NUM sizeof(en_ddns_refresh_time)/sizeof(en_ddns_refresh_time[0])

enum
{
        EN_DDNS_TYPE_3322=0,
        EN_DDNS_TYPE_DYNDNS,
        EN_DDNS_TYPE_ZONEEDIT,
        EN_DDNS_TYPE_NO_IP,
        EN_DDNS_TYPE_ENGENIUS
};

#define BIT_DDNS_TYPE_3322      (APP_AGENT_SUPPORT_DDNS_TYPE_3322 << EN_DDNS_TYPE_3322)
#define BIT_DDNS_TYPE_DYNDNS    (APP_AGENT_SUPPORT_DDNS_TYPE_DYNDNS << EN_DDNS_TYPE_DYNDNS)
#define BIT_DDNS_TYPE_ZONEEDIT  (APP_AGENT_SUPPORT_DDNS_TYPE_ZONEEDIT << EN_DDNS_TYPE_ZONEEDIT)
#define BIT_DDNS_TYPE_NO_IP     (APP_AGENT_SUPPORT_DDNS_TYPE_NO_IP << EN_DDNS_TYPE_NO_IP)
#define BIT_DDNS_TYPE_ENGENIUS  (APP_AGENT_SUPPORT_DDNS_TYPE_ENGENIUS << EN_DDNS_TYPE_ENGENIUS)

#define SUPPORT_DDNS_TYPE_BITMAP (BIT_DDNS_TYPE_3322|BIT_DDNS_TYPE_DYNDNS|BIT_DDNS_TYPE_ZONEEDIT|BIT_DDNS_TYPE_NO_IP|BIT_DDNS_TYPE_ENGENIUS)

/*--------------------------------------------------------------------------*/
int get_ddns_settings_cb(HTTPS_CB *pkt);
int set_ddns_settings_cb(HTTPS_CB *pkt);
#if HAS_ENGENIUS_DDNS
int get_en_ddns_alias_name_available_cb(HTTPS_CB *pkt);
#endif
int get_ddns_provider_cb(HTTPS_CB *pkt);

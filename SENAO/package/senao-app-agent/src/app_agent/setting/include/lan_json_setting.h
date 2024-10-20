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
;    Creator :
;    File    : lan_json_setting.h
;    Abstract:
;
;       Modification History:
;       By              Date            Ver.   	Modification Description
;       --------------- --------        -----   -------------------------------------
;                       2015/12/18              First commit
;****************************************************************************/
#ifndef _LAN_JSON_SETTING_H_
#define _LAN_JSON_SETTING_H_

/*--------------------------------------------------------------------------*/
/*                           INCLUDE HEADER FILES                           */
/*--------------------------------------------------------------------------*/
#include "json.h"
#include "deviceinfo.h"
#include "../../appagents.h"
/*--------------------------------------------------------------------------*/
/*                                STRUCTURE                                 */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*                            FUNCTION PROTOTYPE                            */
/*--------------------------------------------------------------------------*/
bool get_device_lan_settings_json_cb(struct json_object *jobj, ROUTER_LAN_SETTINGS_T *setting);
bool parse_blocked_client_list_json_cb(char *query_str, BLOCKED_CLIENT_LIST_T *setting, int *total_num);
#endif

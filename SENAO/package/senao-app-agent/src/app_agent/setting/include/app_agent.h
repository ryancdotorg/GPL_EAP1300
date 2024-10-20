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
;    Project :
;    Creator :
;    File    :
;    Abstract:
;
;       Modification History:
;       By              Date     Ver.   Modification Description
;       --------------- -------- -----  -------------------------------------
;
;****************************************************************************/
#ifndef _APP_AGENT_SERVER_H_
#define _APP_AGENT_SERVER_H_

/*--------------------------------------------------------------------------*/
/*                           INCLUDE HEADER FILES                           */
/*--------------------------------------------------------------------------*/

#include "../../appagents.h"
#include "api_tokens.h"
#include "variable/variable.h"
#include "variable/api_sys.h"
#if HAS_COMMAND_LIST
#include "../../command_list.h"
#endif

/*--------------------------------------------------------------------------*/
/*                                DEFINITION                                */
/*--------------------------------------------------------------------------*/
#if HAS_COMMAND_LIST
#define APPAGENT_SYSTEM(x...)          command_queue(x)
#else
#define APPAGENT_SYSTEM(x...)          SYSTEM(x)
#endif

// todo
#define LOGIN_USERNAME				   "admin"
#define LOGIN_PASSWORD				   "admin"

/*--------------------------------------------------------------------------*/
/*                                PARAMETER                                 */
/*--------------------------------------------------------------------------*/
#if HAS_REDIRECT_DEVICE_SETTING
#ifndef DEVICE_IPADDR
#define DEVICE_IPADDR   "192.168.99.99"
#endif
#endif
#define MAX_WEB_ACCOUNTS_NUM 6
/*--------------------------------------------------------------------------*/
/*                                STRUCTURE                                 */
/*--------------------------------------------------------------------------*/
typedef struct _APP_AGENT_SERVER_SUPPORT_TABLE_
{
	int http_method;
	const char *method;
	int (*handler_cb)(HTTPS_CB *pkt);
	int needAuth;
} APP_AGENT_SERVER_SUPPORT_TABLE_T;

/*--------------------------------------------------------------------------*/
/*                            FUNCTION PROTOTYPE                            */
/*--------------------------------------------------------------------------*/
void app_agent_process(int s, HTTPS_CB *pkt);
char* redirect_to_target_device(HTTPS_CB *pkt, char *target_ip);
void redirect_to_target(HTTPS_CB *pkt, char *target_ip);
int get_ap_wan_settings_json_cb(HTTPS_CB *pkt,AP_WAN_SETTINGS_T *setting,char *result);
int set_ap_wan_settings_json_cb(char *query_str,AP_WAN_SETTINGS_T *setting, char *result_str);
#endif

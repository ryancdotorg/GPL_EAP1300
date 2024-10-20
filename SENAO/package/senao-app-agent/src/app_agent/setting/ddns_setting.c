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
;    File    : wlan_setting.c
;    Abstract:
;
;       Modification History:
;       By              Date        Ver.    Modification Description
;       --------------- --------    -----   -------------------------------------
;
;****************************************************************************/

/*--------------------------------------------------------------------------*/
/*                           INCLUDE HEADER FILES                           */
/*--------------------------------------------------------------------------*/
#include "ddns_setting.h"
#include "app_agent.h"
#include "api_tokens.h"
#include "sysCore.h"
#include "sysAddr.h"

/*--------------------------------------------------------------------------*/
/*                                PARAMETER                                 */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*                             GLOBAL VARIABLE                              */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*                            FUNCTION PROTOTYPE                            */
/*--------------------------------------------------------------------------*/



/*****************************************************************
 * NAME:    get_ddns_settings_cb
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
int get_ddns_settings_cb(HTTPS_CB *pkt)
{
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
    SYSTEM_DDNS_INFO_T setting;
        char *result_str=OK_STR,uid[32],other_ddns[32],*ptr=NULL;
        int eg_ddns_enable=1,my_ddns_enable=0;

    if(NULL == pkt)
        return -1;

    memset(&setting, 0, sizeof(setting));

        api_get_bool_option("eg-ddns.ddns.enable",&eg_ddns_enable);
        api_get_bool_option("ddns.myddns.enabled",&my_ddns_enable);
        if(eg_ddns_enable || my_ddns_enable)
                setting.enable=1;
        
        if(eg_ddns_enable)
                strcpy(setting.provider,STR_DDNS_TYPE_ENGENIUS);
        else if(my_ddns_enable){
                memset(other_ddns, 0x00, sizeof(other_ddns));
                api_get_string_option("ddns.myddns.service_name",other_ddns,sizeof(other_ddns));

                if (!strcmp("3322.org",other_ddns))
                        strcpy(setting.provider,STR_DDNS_TYPE_3322);
                else if (!strcmp("dyndns.org",other_ddns))
                        strcpy(setting.provider,STR_DDNS_TYPE_DYNDNS);
                else if(!strcmp("no-ip.com",other_ddns))
                        strcpy(setting.provider,STR_DDNS_TYPE_NO_IP); 
                else if (!strcmp("zoneedit.com",other_ddns))
                        strcpy(setting.provider,STR_DDNS_TYPE_ZONEEDIT);
                else
                        api_get_string_option("ddns.myddns.update_url",setting.provider,sizeof(setting.provider));
        }

        api_get_string_option("ddns.myddns.domain",setting.host,sizeof(setting.host));
        api_get_string_option("ddns.myddns.username",setting.userName,sizeof(setting.userName));
        api_get_string_option("ddns.myddns.password",setting.password,sizeof(setting.password));

        memset(uid, 0x00, sizeof(uid));
	sysutil_interact(uid, sizeof(uid), "cat /etc/UID.conf");
	if(strlen(uid)==1){
		strcpy(uid,"0000000");
		uid[strlen(uid)]='\0';
	}
        if(NULL != (ptr = strchr(uid, '\n')))
	*ptr = '\0';
        strcpy(setting.defaultName,uid);

        api_get_integer_option("eg-ddns.ddns.update_time",&(setting.refreshTime));
        api_get_string_option("eg-ddns.ddns.domain",setting.aliasName,sizeof(setting.aliasName));

    get_ddns_settings_json_cb(pkt, &setting, result_str);
}

/*****************************************************************
 * NAME:    set_ddns_settings_cb
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
int set_ddns_settings_cb(HTTPS_CB *pkt)
{
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
    SYSTEM_DDNS_INFO_T settings;
    bool result = TRUE;
    int i=0,others=0;
    char ddns_services[20]={0};

    if(NULL == pkt)
        return -1;

    memset(&settings, 0, sizeof(settings));
    result &=parse_json_ddns_settings(query_str, &settings);

    if(result)
    {
        if (!strcmp(STR_DDNS_TYPE_ENGENIUS,settings.provider)){
                api_set_bool_option("eg-ddns.ddns.enable",settings.enable);
                if(settings.enable)
                	api_set_bool_option("ddns.myddns.enabled",0);
        }
        else{
                api_set_bool_option("ddns.myddns.enabled",settings.enable);
                if(settings.enable)
                	api_set_bool_option("eg-ddns.ddns.enable",0);
 
                if (!strcmp(STR_DDNS_TYPE_3322,settings.provider))
                        strcpy(ddns_services,"3322.org");
                else if (!strcmp(STR_DDNS_TYPE_DYNDNS,settings.provider))
                        strcpy(ddns_services,"dyndns.org");
                else if(!strcmp(STR_DDNS_TYPE_NO_IP,settings.provider))
                        strcpy(ddns_services,"no-ip.com"); 
                else if (!strcmp(STR_DDNS_TYPE_ZONEEDIT,settings.provider))
                        strcpy(ddns_services,"zoneedit.com");
                else{
                        others=1;
                        api_set_string_option("ddns.myddns.update_url",settings.provider,sizeof(settings.provider));
                        api_set_string_option("ddns.myddns.service_name","-",sizeof("-"));
                }

                if(!others)
                        api_set_string_option("ddns.myddns.service_name",ddns_services,sizeof(ddns_services));
        }   
        //DefaultName can not be changed !!
        api_set_integer_option("eg-ddns.ddns.update_time",settings.refreshTime);
        api_set_string_option("eg-ddns.ddns.domain",settings.aliasName,sizeof(settings.aliasName));       
        api_set_string_option("ddns.myddns.domain",settings.host,sizeof(settings.host));
        api_set_string_option("ddns.myddns.username",settings.userName,sizeof(settings.userName));
        api_set_string_option("ddns.myddns.password",settings.password,sizeof(settings.password));

        system("uci commit");
        system("luci-reload ddns");
	system("luci-reload eg-ddns");
    }

send_pkt:
    send_simple_response(pkt, (TRUE == result)?OK_STR:ERROR_STR);

    return 0;
}

#if HAS_ENGENIUS_DDNS
/*****************************************************************
* NAME:    parse_alias_name
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
bool parse_alias_name(HTTPS_CB *pkt, char *query_str, char *aliasName)
{
    bool result= FALSE;

    if((NULL != pkt) && (NULL != query_str))
    {
        result = parse_alias_name_json_cb(query_str, aliasName);
    }

    return result;
}

/*****************************************************************
* NAME:    get_en_ddns_alias_name_available_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int get_en_ddns_alias_name_available_cb(HTTPS_CB *pkt)
{
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
    bool result=FALSE;
    char aliasName[64+1], buf[32], check_result[16]={0};
    int rval;

    if(NULL == pkt)
        return -1;

    if(NULL == query_str)
        send_simple_response(pkt, ERROR_STR);
    else
    {
        result = parse_alias_name(pkt, query_str, aliasName);

        if(TRUE == result)
        {
            rval = sysutil_interact(buf, sizeof(buf), "curl --connect-timeout 3 -m 3 --digest --user %s:%s \"http://ns1.%s/ddns/check?account=%s&name=%s\"",
                            sysutil_GetEngeniusDdnsUsername(), sysutil_GetEngeniusDdnspassword(), sysutil_GetEngeniusDdnsDomain(),
                            sysutil_GetEngeniusDdnsHost(), aliasName);
            if(rval>=0)
                strcpy(check_result, buf);
        }
        get_en_ddns_alias_name_available_json_cb(pkt, result, check_result);
    }
}
#endif

/*****************************************************************
 * NAME:    get_ddns_provider_cb
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
int get_ddns_provider_cb(HTTPS_CB *pkt)
{
    char *result_str=OK_STR;

    if(NULL == pkt)
        return -1;

    get_ddns_provider_json_cb(pkt, result_str);
}


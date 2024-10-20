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
;    Creator : ynyang
;    File    : sitecom_setting.c
;    Abstract:
;
;       Modification History:
;       By              Date        Ver.    Modification Description
;       --------------- --------    -----   -------------------------------------
;       ynyang          2013/07/08          First commit
;****************************************************************************/

/*--------------------------------------------------------------------------*/
/*                           INCLUDE HEADER FILES                           */
/*--------------------------------------------------------------------------*/
#include "sitecom_setting.h"
#include "app_agent.h"
#define FW_FILE "/tmp/download"
/*--------------------------------------------------------------------------*/
/*                                PARAMETER                                 */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*                            FUNCTION PROTOTYPE                            */
/*--------------------------------------------------------------------------*/

#if 1//HAS_SC_AUTO_FW_CHECK
/*****************************************************************
* NAME:    get_auto_fw_upgrade_status_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_auto_fw_upgrade_status_cb(HTTPS_CB *pkt)
{
    if(NULL == pkt){
        return -1;
    }

    if(pkt->json)
    {
        get_auto_fw_upgrade_status_json_cb(pkt, true, OK_STR);
    }

    return 0;
}
#if 0
/*****************************************************************
* NAME:    set_auto_fw_upgrade_status_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int set_auto_fw_upgrade_status_cb(HTTPS_CB *pkt)
{
    bool enabled;
    bool result = TRUE;
    char *return_str;
#if 0
    char *query_str="{\n" \
                    "  \"Enabled\" : \"boolean\"\n" \
                    "}";
#else
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
#endif
    HTTP_PACKET_CONTENT_FORMAT pkt_format;

    // The default value in return packet is ERROR.
    return_str = ERROR_STR;
    
    if((NULL == pkt) || (NULL == query_str)){
        goto send_pkt;
    }

    if(pkt->json)
    {
        pkt_format = JSON_FORMAT;
        result &= parse_auto_fw_upgrade_status_json_cb(query_str, &enabled, return_str);
    }

    if(TRUE != result) goto send_pkt;
    
    /* Set the setting into apcfg */
    if(DISABLE == enabled){
        apCfgSetIntValue(AUTO_FW_NEVERCHECK_ENABLE_TOK, 1);
        apCfgSetIntValue(AUTO_FW_CHECK_ACTION_TOK, ACTIOIN_NOT_REMIND_EVER);
    }
    else{
        apCfgSetIntValue(AUTO_FW_NEVERCHECK_ENABLE_TOK, 0);
        apCfgSetIntValue(AUTO_FW_CHECK_ACTION_TOK, ACTIOIN_INITIAL);
    }

    /* 2. save new token value */
    if(result && apCfgIsModified())
    {
        apCfgUpdateModifiedData();
        reolad_module();
    }

    /* 3. send response packet */
    return_str = OK_STR;

send_pkt:
    send_simple_response(pkt, return_str);

    return 0;
}
#endif
#endif //HAS_SC_AUTO_FW_CHECK

#if HAS_SC_UTMPROXY_FUNCTION
/*****************************************************************
* NAME:    get_scs_status_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_scs_status_cb(HTTPS_CB *pkt)
{
    bool enabled;
    if(NULL == pkt){
        return -1;
    }

    enabled = (apCfgGetIntValue(SC_UTMPROXY_ENABLE_TOK) == 0)?DISABLE:ENABLE;

    if(pkt->json)
    {
        get_scs_status_json_cb(pkt, enabled, OK_STR);
    }

    return 0;
}

/*****************************************************************
* NAME:    set_scs_status_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int set_scs_status_cb(HTTPS_CB *pkt)
{
    bool enabled;
    bool result = TRUE;
    char *return_str;
#if 0
    char *query_str="{\n" \
                    "  \"Enabled\" : \"boolean\"\n" \
                    "}";
#else
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
#endif
    HTTP_PACKET_CONTENT_FORMAT pkt_format;

    // The default value in return packet is ERROR.
    return_str = ERROR_STR;
    
    if((NULL == pkt) || (NULL == query_str))
    {
        goto send_pkt;
    }

    if(pkt->json)
    {
        pkt_format = JSON_FORMAT;
        result &= parse_scs_status_json_cb(query_str, &enabled, return_str);
    }

    if(TRUE != result) goto send_pkt;
    
    /* Set the setting into apcfg */
    if(DISABLE == enabled){
        apCfgSetIntValue(SC_UTMPROXY_ENABLE_TOK, 0);
    }
    else{
        apCfgSetIntValue(SC_UTMPROXY_ENABLE_TOK, 1);
    }

    /* 2. save new token value */
    if(result && apCfgIsModified())
    {
        apCfgUpdateModifiedData();
        reolad_module();
    }

    /* 3. send response packet */
    return_str = OK_STR;

send_pkt:
    send_simple_response(pkt, return_str);

    return 0;
}
#endif
#if HAS_STREAM_BOOST_SETTING
/*****************************************************************
* NAME:    get_stream_boost_settings_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_stream_boost_settings_cb(HTTPS_CB *pkt)
{
    int enabled = 0;
    STREAM_BOOST_DATA setting;

    if(NULL == pkt){
        return -1;
    }

    memset(&setting, 0, sizeof(setting));

    api_get_bool_option(APPFLOW_TCCONTROLLER_ENABLE_STREAMBOOST_OPTION, &enabled);
    setting.enabled = (enabled)?true:false;

    if(pkt->json)
    {
        get_stream_boost_settings_json_cb(pkt, &setting, OK_STR);
    }

    return 0;
}

/*****************************************************************
* NAME:    set_stream_boost_settings_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int set_stream_boost_settings_cb(HTTPS_CB *pkt)
{
    bool enabled;
    int upLimit = 0, downLimit = 0;
    bool result = TRUE;
    char *return_str;
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
    STREAM_BOOST_DATA setting;
    HTTP_PACKET_CONTENT_FORMAT pkt_format;

    // The default value in return packet is ERROR.
    return_str = ERROR_STR;
    
    if((NULL == pkt) || (NULL == query_str))
    {
        goto send_pkt;
    }

    memset(&setting, 0, sizeof(setting));

    if(pkt->json)
    {
        pkt_format = JSON_FORMAT;
        result &= parse_stream_boost_settings_json_cb(query_str, &setting, return_str);
    }

    if(TRUE != result)
        goto send_pkt;
    
    /* Set the setting into /etc/config */
    api_set_bool_option(APPFLOW_TCCONTROLLER_ENABLE_STREAMBOOST_OPTION, setting.enabled);
    upLimit = (setting.enabled)?setting.upLimit:STREAM_BOOST_MAX_UPLOAD_LIMIT;
    upLimit = (upLimit*0.125)*1000*1000; //Mbps to Bytes/second
    api_set_integer_option(APPFLOW_TCCONTROLLER_UPLIMIT_OPTION, upLimit);
    downLimit = (setting.enabled)?setting.downLimit:STREAM_BOOST_MAX_DOWNLOAD_LIMIT;
    downLimit = (downLimit*0.125)*1000*1000; //Mbps to Bytes/second
    api_set_integer_option(APPFLOW_TCCONTROLLER_DOWNLIMIT_OPTION, downLimit);

    /* 2. save new token value */
    if(result)
    {
        SYSTEM("uci commit");
        SYSTEM("streamboost restart");
    }

    /* 3. send response packet */
    return_str = OK_STR;

send_pkt:
    send_simple_response(pkt, return_str);

    return 0;
}
#endif

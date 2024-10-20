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
;    File    : ipcam_samba_setting.c
;    Abstract:
;
;       Modification History:
;       By              Date        Ver.    Modification Description
;       --------------- --------    -----   -------------------------------------
;       Jerry           2012/09/10          First commit
;****************************************************************************/

/*--------------------------------------------------------------------------*/
/*                           INCLUDE HEADER FILES                           */
/*--------------------------------------------------------------------------*/
#include "ipcam_samba_setting.h"
#include "app_agent.h"
#include "admin_cfg.h"
/*--------------------------------------------------------------------------*/
/*                                PARAMETER                                 */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*                            FUNCTION PROTOTYPE                            */
/*--------------------------------------------------------------------------*/
/*****************************************************************
* NAME:    set_ipcam_samba_client_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int set_ipcam_samba_client_cb(HTTPS_CB *pkt)
{
    /*******************************************************************
     * SetIpcamSambaClientResult type:                                *
     * OK, ERROR, LOGIN_ERROR.										  *
     *******************************************************************/
#if 0
    /*char *query_str="{ \"AdminUsername\" : \"admin\", \"AdminPassword\" : \"admin\", 
					   \"EnableRecord\" : \"1\", \"EnableOverwrite\" : \"1\",
					   \"SambaServer\" : \"192.168.0.1\", \"SambaFolderPath\" : \"folder\",
					   \"SambaServerUsername\" : \"admin\", \"SambaServerPassword\" : \"admin\"}";*/
#else
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
#endif
    bool result = TRUE;
    int i = 0;

    IPCAM_SAMBA_SETTINGS_T settings;
    char *return_str;
	char username[32] = {0}, password[32] = {0};
	int record=0, overwrite=0;
    char samba_server[32] = {0}, samba_folder_path[32] = {0};
    char samba_username[32] = {0}, samba_password[32] = {0};
    HTTP_PACKET_CONTENT_FORMAT pkt_format;

    // The default value in return packet is ERROR.
    return_str = ERROR_STR;

    if((NULL == pkt) || (NULL == query_str))
    {
        goto send_pkt;
    }

    memset(&settings, 0, sizeof(settings));

    if(pkt->json)
    {
        pkt_format = JSON_FORMAT;
        result &= parse_json_ipcam_samba_client(query_str, &settings, return_str);
    }
#if 1
	printf("%s[%d] settings.username: [%s]\n", __FUNCTION__, __LINE__, settings.username);
	printf("%s[%d] settings.password: [%s]\n", __FUNCTION__, __LINE__, settings.password);
	printf("%s[%d] settings.record: [%d]\n", __FUNCTION__, __LINE__, settings.record);
	printf("%s[%d] settings.overwrite: [%d]\n", __FUNCTION__, __LINE__, settings.overwrite);
	printf("%s[%d] settings.samba_server: [%s]\n", __FUNCTION__, __LINE__, settings.samba_server);
	printf("%s[%d] settings.samba_folder_path: [%s]\n", __FUNCTION__, __LINE__, settings.samba_folder_path);
	printf("%s[%d] settings.samba_username: [%s]\n", __FUNCTION__, __LINE__, settings.samba_username);
	printf("%s[%d] settings.samba_password: [%s]\n", __FUNCTION__, __LINE__, settings.samba_password);
#endif

	if(TRUE != result) goto send_pkt;

	if(strcmp(settings.username, LOGIN_USERNAME)!=0 || strcmp(settings.username, LOGIN_USERNAME)!=0)
	{
		return_str = ERROR_LOGIN_STR;
        goto send_pkt;
	}

	if(TRUE == result)
    {
        return_str = OK_STR;
    }
send_pkt:
    /* 3. send response packet */
    send_simple_response(pkt, return_str);

    return 0;
}

/*****************************************************************
* NAME:    set_ipcam_motion_detect_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int set_ipcam_motion_detect_cb(HTTPS_CB *pkt)
{
    /*******************************************************************
     * SetIpcamMotionDetectResult type:                                *
     * OK, ERROR, LOGIN_ERROR.										  *
     *******************************************************************/
#if 0
    /*char *query_str="{ \"AdminUsername\" : \"admin\", \"AdminPassword\" : \"admin\", 
						 \"EnableMotionDetect\" : \"1\"}";*/
#else
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
#endif
    bool result = TRUE;
    int i = 0;

    IPCAM_SAMBA_SETTINGS_T settings;
    char *return_str;
	char username[32] = {0}, password[32] = {0};
	int motion_detect=0;
    HTTP_PACKET_CONTENT_FORMAT pkt_format;

    // The default value in return packet is ERROR.
    return_str = ERROR_STR;

    if((NULL == pkt) || (NULL == query_str))
    {
        goto send_pkt;
    }

    memset(&settings, 0, sizeof(settings));

    if(pkt->json)
    {
        pkt_format = JSON_FORMAT;
        result &= parse_json_ipcam_motion_detect(query_str, &settings, return_str);
    }
#if 1
	printf("%s[%d] settings.username: [%s]\n", __FUNCTION__, __LINE__, settings.username);
	printf("%s[%d] settings.password: [%s]\n", __FUNCTION__, __LINE__, settings.password);
	printf("%s[%d] settings.motion_detect: [%d]\n", __FUNCTION__, __LINE__, settings.motion_detect);
#endif

	if(TRUE != result) goto send_pkt;

	if(strcmp(settings.username, LOGIN_USERNAME)!=0 || strcmp(settings.username, LOGIN_USERNAME)!=0)
	{
		return_str = ERROR_LOGIN_STR;
        goto send_pkt;
	}

	if(TRUE == result)
    {
        return_str = OK_STR;
    }
send_pkt:
    /* 3. send response packet */
    send_simple_response(pkt, return_str);

    return 0;
}

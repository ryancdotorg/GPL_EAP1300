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
;    File    : device_setting.c
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
#include "appagent_cfg_data.h"
#include "sysgpio.h"
#include "device_setting.h"
#include "api_tokens.h"
#include "sysCore.h"
#include "sysFile.h"
#include "sysCommon.h"
#include "app_agent.h"
#include "mesh_setting.h"

/*--------------------------------------------------------------------------*/
/*                                DEFINITION                                */
/*--------------------------------------------------------------------------*/
#define FW_FILE "/tmp/download"
//http://192.168.1.200/cgi-bin/luci/easyflashops
#define FW_UPGRADE_HTM_STR        "cgi-bin/luci/easyflashops"
#define CONFIG_UPGRADE_HTM_STR    "cgi-bin/luci/easyrestore"

/*--------------------------------------------------------------------------*/
/*                                PARAMETER                                 */
/*--------------------------------------------------------------------------*/
int ipcam_timezone[] = {0, 1, 2, 3, 4, 5, 7, 10, 9, 14, 13, 16, 15, 17, 18, 21, 19, 20, 22, 23, 25, 26, 27, 29, 35, 36, 39, 40, 43, 47, 48, 50, 54, 60, 61, 64, 68, 67, 71, 73, 72};

/*--------------------------------------------------------------------------*/
/*                            FUNCTION PROTOTYPE                            */
/*--------------------------------------------------------------------------*/

void get_IP_Port(int sock, int ipcam_port, char *ip_addr, char *port)
{
	char mask[32]={0}, IPaddr[API_IPADDR_SIZE+1]={0}, externalIP[API_IPADDR_SIZE+1]={0};
	bool isFromLocal=1;
	UINT8 client_ipstr[64]={0};
	int len=0, count=0;
#if HAS_SYSUTIL_NAT_TRAVERSAL_DEAMON && HAS_SYSUTIL_SENAO_UDP_TUNNEL_DEAMON
	int layer=0;
#endif
	struct sockaddr_in peer;

	len = sizeof(struct sockaddr_in);
	getpeername(sock, (struct sockaddr *)&peer, &len);
	sprintf(client_ipstr, "%s", inet_ntoa(peer.sin_addr));

#if HAS_AP
	sysutil_get_interface_ipaddr("br-lan", IPaddr, sizeof(IPaddr));
	if(strlen(IPaddr) == 0)
		sysutil_get_interface_ipaddr("br-lan:manager", IPaddr, sizeof(IPaddr));
#else
	sysutil_get_interface_ipaddr("br-wan", IPaddr, sizeof(IPaddr));
#endif
	if(strlen(IPaddr) > 0)
	{
		while(sysutil_check_file_existed("/tmp/natt.externalIP") == FALSE)
		{
			sleep(1);
			if(count > 5)
				break;
			else
				count++;
		}
		sysutil_interact(externalIP, sizeof(externalIP), "cat /tmp/natt.externalIP");
		externalIP[strlen(externalIP)] = '\0';
		sysutil_interact(mask,sizeof(mask),"ifconfig br-lan | grep Mask | awk -F \" \" \'{print $4}\' | awk -F \":\" \'{print $2}\'");
		mask[strlen(mask)] = '\0';

		if(strlen(mask) == 0)
			strcpy(mask, "255.255.255.0");

		if(strlen(externalIP) == 0 )
			strcpy(externalIP, IPaddr);

		isFromLocal = api_check_is_same_subnet(IPaddr, client_ipstr, mask);
	}
#if HAS_SYSUTIL_NAT_TRAVERSAL_DEAMON && HAS_SYSUTIL_SENAO_UDP_TUNNEL_DEAMON
	layer = sysutil_GetNetworkLayer();
	if(isFromLocal==1 || layer==1)
#else
	if(isFromLocal==1)
#endif
	{
		sprintf(ip_addr, "%s", IPaddr);
		sprintf(port, "%d", ipcam_port);
	}
	else
	{
		char final_port[10]={0};

		sprintf(ip_addr, "%s", externalIP);

		if(ipcam_port==9000)
		{
			api_get_string_option("nat-traversal.nattraversal.ex_upload_port", final_port, sizeof(final_port));
		}
		else if(ipcam_port==5540)
		{
			api_get_string_option("nat-traversal.nattraversal.ex_onboard_cam_rtsp_port", final_port, sizeof(final_port));
		}
		sprintf(port, "%s", final_port);
	}
}

/*****************************************************************
* NAME:    login_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int login_cb(HTTPS_CB *pkt)
{
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
    int result = AGENT_LOGIN_FAIL;
#if HAS_UPDATE_NODE_INFO_AT_LOGIN
    char mesh_key[64]={0};
    char mesh_id[64]={0};
#endif
    if(NULL == pkt)
        return -1;
#if HAS_UPDATE_NODE_INFO_AT_LOGIN
    api_get_string_option("wireless.wifi1_mesh.aeskey", mesh_key, sizeof(mesh_key));
    api_get_string_option("wireless.wifi1_mesh.Mesh_id", mesh_id, sizeof(mesh_id));
    APPAGENT_SYSTEM("/sbin/mesh.sh get_mesh_global_node_info %s %s &", mesh_id, mesh_key);
#endif
	if(sysutil_check_file_existed("/tmp/clientRefresh.sh"))
	{
		APPAGENT_SYSTEM("/tmp/clientRefresh.sh &");
	}

    if(pkt->json)
        result = login_json_cb(query_str, NULL);

    return result;
}

#if HAS_LIMIT_APP_ACCOUNT_LOGIN
/*****************************************************************
* NAME:    logout_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int logout_cb(HTTPS_CB *pkt)
{
    char *query_str;
    int app_type;
    int result = AGENT_LOGIN_OK;

    if(NULL == pkt)
        return -1;

    query_str = get_env(&pkt->envcfg, "QUERY_STRING");

    get_json_integer_from_query(query_str, &app_type, "APPType");

    switch(app_type)
    {
        case AGENT_APP_ENMESH:
            result = AGENT_LOGIN_OK_APP_ENMESH;
            break;
        default:
            break;
    }

    return result;
}
#endif

/*****************************************************************
* NAME:    change_login_pw_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int change_login_pw_cb(HTTPS_CB *pkt)
{
    bool result = TRUE;
    char return_str[63+1];

    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");

    HTTP_PACKET_CONTENT_FORMAT pkt_format;

    // The default value in return packet is ERROR.
    strcpy(return_str, ERROR_STR);

    if((NULL == pkt) || (NULL == query_str))
        goto send_pkt;

    if(pkt->json)
    {
        pkt_format = JSON_FORMAT;
        result = change_login_pw_json_cb(query_str, (char *)return_str);
    }

    if(TRUE != result) 
        goto send_pkt;

    strcpy(return_str, OK_STR);

send_pkt:
    //send the success response
    send_simple_response(pkt, return_str);

    return 0;
}

/*****************************************************************
* NAME:    check_alive_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int check_alive_cb(HTTPS_CB *pkt){

    if(NULL == pkt)
    {
        return -1;
    }

    //send the success response
    send_simple_response(pkt, OK_STR);

    return 0;
}

/*****************************************************************
* NAME:    reboot_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int reboot_cb(HTTPS_CB *pkt)
{

    if(NULL == pkt)
        return -1;

    //send the success response
    send_simple_response(pkt, OK_STR);

#if HAS_IPCAM
    char login_response[1024]={0}, response[64]={0};
    char xrelayd_ip[128]={0};

    api_get_string_option("xrelayd.xrelayd.conn_sec_ip", xrelayd_ip, sizeof(xrelayd_ip));

    sysutil_interact(login_response, sizeof(login_response),
                     "app_client -i %s -m POST -a Login -e 1 -p \'{\"AdminUsername\":\"senao_mesh_ipcam\",\"AdminPassword\":\"ipcam\"}\'", xrelayd_ip);

    sysutil_interact(response, sizeof(response), "app_client -i %s -m GET -a Reboot -e 1", xrelayd_ip);
    sleep(3);
#endif

    SYSTEM("reboot");

    return 0;
}

/*****************************************************************
* NAME:    reset_to_default_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int reset_to_default_cb(HTTPS_CB *pkt)
{
    char login_response[1024]={0}, response[64]={0};
    char xrelayd_ip[128]={0};

    if(NULL == pkt)
    {
        return -1;
    }

    send_simple_response(pkt, OK_STR);

    api_get_string_option("xrelayd.xrelayd.conn_sec_ip", xrelayd_ip, sizeof(xrelayd_ip));
    sysutil_interact(login_response, sizeof(login_response),
                     "app_client -i %s -m POST -a Login -e 1 -p \'{\"AdminUsername\":\"senao_mesh_ipcam\",\"AdminPassword\":\"ipcam\"}\'", xrelayd_ip);

    sysutil_interact(response, sizeof(response), "app_client -i %s -m POST -a ResetToDefault -e 1", xrelayd_ip);

	SYSTEM("rm -rf /overlay/* ; reboot");

    return 0;
}

/*****************************************************************
* NAME:    reboot_factory_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int reboot_factory_cb(HTTPS_CB *pkt)
{

    if(NULL == pkt)
        return -1;

    //send the success response
    send_simple_response(pkt, OK_STR);

    SYSTEM("rm -rf /overlay/*");

    return 0;
}

/*****************************************************************
* NAME:    get_device_settings_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_device_settings_cb(HTTPS_CB *pkt)
{
    int  rval;
    char buf[64];
    char *result;
    char wan_interface[5]={0};
    char opmode[3]={0};
    DEVICE_SETTING_T setting;

    if(NULL == pkt)
        return -1;

    memset(&setting, 0, sizeof(setting));

    api_get_string_option(SYSPRODUCTINFO_MODEL_VENDERNAME_OPTION, setting.vendor_name, sizeof(setting.vendor_name));
    api_get_string_option(SYSPRODUCTINFO_MODEL_MODELNAME_OPTION, setting.model_name, sizeof(setting.model_name));
    api_get_string_option(SYSPRODUCTINFO_MODEL_DESCRIPTION_OPTION, setting.model_description, sizeof(setting.model_description));

    sysutil_get_serial_number_info(setting.product_sn, sizeof(setting.product_sn));

    sysutil_get_hardware_version_info(setting.hardware_version, sizeof(setting.hardware_version));
#if APP_AGENT_SUPPORT_ENSHARE
    api_get_string_option("system.@system[0].opmode", opmode, sizeof(opmode));
    if ( strcmp(opmode,"ar") == 0) 
    {
        api_get_string_option("network.wan.ifname", wan_interface, sizeof(wan_interface));
    }
    else
    {
        sysutil_interact(wan_interface,sizeof(wan_interface), "cat /tmp/wandev");
    }

    /*PCI would like to take 17 letters for Wan Mac Address*/
    memset(buf, 0, sizeof(buf));
    rval =  sysutil_get_interface_mac(wan_interface, buf, sizeof(buf));
    buf[strcspn(buf, "\n")] = '\0';
    sprintf(setting.wan_mac, "%s", buf);
#else
    memset(buf, 0, sizeof(buf));
    rval = sysutil_interact(buf, sizeof(buf), "setconfig -g 7");
    buf[strcspn(buf, "\n")] = '\0';
    sprintf(setting.wan_mac, "%s", buf);

#endif
    /*PCI would like to take 17 letters for Lan Mac Address*/
    memset(buf, 0, sizeof(buf));
    rval = sysutil_interact(buf, sizeof(buf), "setconfig -g 6");
    buf[strcspn(buf, "\n")] = '\0';
    buf[strcspn(buf, " ")] = '\0';
    sprintf(setting.lan_mac, "%s", buf);

    /*PCI would like to take 17 letters for Wlan Mac Address*/
    memset(buf, 0, sizeof(buf));
    rval = sysutil_interact(buf, sizeof(buf), "setconfig -g 6");
    buf[strcspn(buf, "\n")] = '\0';
    buf[strcspn(buf, " ")] = '\0';
    sprintf(setting.wlan_mac, "%s", buf);

    result = OK_STR;

    if(pkt->json)
        get_device_settings_json_cb(pkt, &setting, result);

    return 0;
}

/*****************************************************************
* NAME:    get_system_information_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_system_information_cb(HTTPS_CB *pkt)
{

    char *result;
    SYSTEM_INFO_T setting;
    T_CHAR kversion[15+1], fversion[15+1], firstByte[3+1], secondByte[3+1], buildDate[15+1], upTime[63+1];
    int buildYears=0, buildMonths=0, buildDays=0, days=0, hours=0, minutes=0, seconds=0;

    if(NULL == pkt)
        return -1;

    memset(&setting, 0, sizeof(setting));
    memset(kversion, 0, sizeof(kversion));
    memset(fversion, 0, sizeof(fversion));
    memset(upTime, 0, sizeof(upTime));

    memset(firstByte, 0, sizeof(firstByte));
    memset(secondByte, 0, sizeof(secondByte));
    sysutil_get_kernel_version_info(kversion, sizeof(kversion));
    if(strcmp(kversion, "---" ))
        sscanf(kversion, "%[^.].%[^.].%*", firstByte, secondByte);
    sprintf(setting.kernel_version, "%s.%s", firstByte, secondByte);
    memset(firstByte, 0, sizeof(firstByte));
    memset(secondByte, 0, sizeof(secondByte));
    sysutil_get_firmware_version_info(fversion, sizeof(fversion));
    if(strcmp(fversion, "---" ))
        sscanf(fversion, "%[^.].%[^.].%*", firstByte, secondByte);
    sprintf(setting.firmware_version, "%s", fversion);
    sysutil_get_build_date_info(buildDate, sizeof(buildDate));
    if(strcmp(buildDate, "---" ))
    {
        sscanf(buildDate, "%d-%d-%d", &buildYears, &buildMonths, &buildDays);
        sprintf(setting.build_date, "%d/%d/%d", buildYears, buildMonths, buildDays);
    }
    sysutil_get_local_time("%Y/%m/%d %H:%M:%S", setting.local_time, sizeof(setting.local_time));
    if (sysutil_get_uptime(upTime, sizeof(upTime)))
    {
        sscanf(upTime, "%d days ,%d hours ,%d minutes ,%d seconds", &days, &hours, &minutes, &seconds);
        if(days > 0)
                sprintf(setting.uptime, "%d days %d hours %d min %ld sec", days, hours, minutes, seconds);
        else if(hours > 0)
                sprintf(setting.uptime, "%d hours %d min %ld sec", hours, minutes, seconds);
        else if(minutes > 0)
                sprintf(setting.uptime, "%d min %ld sec", minutes, seconds);
        else
                sprintf(setting.uptime, "%ld sec", seconds);
    }

    result = OK_STR;

    if(pkt->json)
        get_system_information_json_cb(pkt, &setting, result);

    return 0;
}
#if HAS_IPCAM
/*****************************************************************
* NAME:    download_device_config_file_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int download_device_config_file_cb(HTTPS_CB *pkt)
{
        char *result= ERROR_STR;
        char hostName[31+1], localTime[15+1], filePath[127+1],lanIP[32],type[10],http_filePath[255+1];
        char download_path[255+1];
        char xrelayd_ip[128]={0};
        
        if(NULL == pkt)
                return -1;
        if (sysutil_check_file_existed("/tmp/config.dlf"))
                SYSTEM("rm %s", filePath);
        
        api_get_string_option("xrelayd.xrelayd.conn_sec_ip", xrelayd_ip, sizeof(xrelayd_ip));
        SYSTEM("app_client -i %s -m POST -a Login -e 1 -p \'{\"AdminUsername\":\"senao_mesh_ipcam\",\"AdminPassword\":\"ipcam\"}\'", xrelayd_ip);
        SYSTEM("app_client -m GET -i %s -a DownloadDeviceConfigFile -e 1 -d 1", xrelayd_ip);
        SYSTEM("curl \"http://%s/downloads/config.dlf\" --output /tmp/config.dlf", xrelayd_ip);

        memset(hostName,0x0, sizeof(hostName));
        memset(localTime,0x0, sizeof(localTime));
        memset(filePath,0x0, sizeof(filePath));
        memset(lanIP,0x0,sizeof(lanIP));
        memset(type,0x0,sizeof(type));

        api_get_string_option(NETWORK_LAN_PROTO_OPTION,type,sizeof(type));
        if(!strcmp(type,"dhcp"))
                sysutil_get_lan_ipaddr(lanIP, sizeof(lanIP));
        else
                api_get_lan_ipaddr_option(NETWORK_LAN_IPADDR_OPTION,lanIP,sizeof(lanIP));

        api_get_string_option(SYSTEM_SYSTEM_HOSTNAME_OPTION, hostName, sizeof(hostName));
        sysutil_get_local_time("%Y-%m-%d", localTime, sizeof(localTime));
        snprintf(filePath, sizeof(filePath), "%s-%s-%s.tar.gz", "/tmp/backup", hostName, localTime);
        snprintf(download_path, sizeof(download_path),"%s-%s-%s.tar.gz","/backup",hostName,localTime);

        if (sysutil_check_file_existed(filePath))
                SYSTEM("rm %s", filePath);

        if (!SYSTEM("sysupgrade --create-backup %s", filePath))
                result = OK_STR;

        SYSTEM("cp %s /www%s",filePath,download_path);
        SYSTEM("rm %s",filePath);

        snprintf(http_filePath, sizeof(http_filePath), "http://%s%s",lanIP,download_path);

        if(pkt->json)
                download_device_config_file_json_cb(pkt,http_filePath, result);

        return 0;
}

#else
/*****************************************************************
* NAME:    download_device_config_file_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int download_device_config_file_cb(HTTPS_CB *pkt)
{
    char *result;
    char hostName[31+1], localTime[15+1], filePath[127+1];

    result = ERROR_STR;

    if(NULL == pkt)
        return -1;

    memset(hostName, 0, sizeof(hostName));
    memset(localTime, 0, sizeof(localTime));
    memset(filePath, 0, sizeof(filePath));

    api_get_string_option(SYSTEM_SYSTEM_HOSTNAME_OPTION, hostName, sizeof(hostName));
    sysutil_get_local_time("%Y-%m-%d", localTime, sizeof(localTime));
    snprintf(filePath, sizeof(filePath), "%s-%s-%s.tar.gz", "/tmp/backup", hostName, localTime);

    if (sysutil_check_file_existed(filePath))
        SYSTEM("rm %s", filePath);

    if (!SYSTEM("sysupgrade --create-backup %s", filePath))
        result = OK_STR;

    //20170116 Jason: If User use this function from APP, we should cp file to usb.
#if APP_AGENT_SUPPORT_ENSHARE
    SYSTEM("cp %s %s", filePath, "/www/usb_admin/sda1/.tmp/");
    memset(filePath, 0, sizeof(filePath));
    snprintf(filePath, sizeof(filePath), "%s-%s-%s.tar.gz", "/www/usb_admin/sda1/.tmp/backup", hostName, localTime);
#endif

    if(pkt->json)
        download_device_config_file_json_cb(pkt, filePath, result);

    return 0;
}
#endif
/*****************************************************************
* NAME:    upgrade_fw_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int upgrade_fw_cb(HTTPS_CB *pkt)
{
    bool result = TRUE;
    bool wget_status = FALSE;
    int  times;
    unsigned int  file_size;
    char wan_ip[32];
    char *return_str;
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
    UPGRADE_FW settings;

    // The default value in return packet is ERROR.
    return_str = ERROR_STR;

    if((NULL == pkt) || (NULL == query_str))
    {
        goto send_pkt;
    }

    memset(&settings, 0, sizeof(settings));

    if(pkt->json)
    {
        result = parse_upgrade_fw_json_cb(query_str, &settings, return_str);
    }

    if(TRUE != result) goto send_pkt;

    wget_status = sysutil_check_proc_existed("wget", &times);

    /* Download DownloadStatus Install FW */
    if((strcmp("Download",settings.action) == 0) &&
#if SUPPORT_WAN_SETTING
       sysutil_get_wan_ipaddr(wan_ip, sizeof(wan_ip)) &&
#endif
       (strlen(settings.url) != 0 && strcmp(settings.url,"NULL") != 0) &&
       (wget_status <= 0))
    {
        APPAGENT_SYSTEM("wget -O%s --connect-timeout=6 --tries=2 %s &", FW_FILE, settings.url);
    }
    else if((strcmp("DownloadStatus",settings.action) == 0) && (strcmp("NULL",settings.url) == 0))
    {
        if(wget_status)
        {
            return_str = DOWNLOADING_STR;
        }
        else
        {
            file_size = 0;

            if(sysutil_check_file_existed(FW_FILE))
            {
                sysutil_get_file_size(FW_FILE, &file_size);
            }

            return_str = (file_size != 0)?FINISH_STR:DOWNLOAD_FAIL_STR;
        }

        goto send_pkt;
    }
    else if((strcmp("Install",settings.action) == 0) &&
            sysutil_check_file_existed(FW_FILE) &&
            (strcmp("NULL",settings.url) == 0))
    {
        APPAGENT_SYSTEM("sysupgrade -v %s &", FW_FILE);
    }
    else
    {
        goto send_pkt;
    }

    /* send response packet */
    return_str = OK_STR;

send_pkt:
    send_simple_response(pkt, return_str);

    return 0;
}

/*****************************************************************
* NAME:    get_device_settings
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
bool get_device_settings(DEVICE_SETTING_T *setting)
{
    int  rval;
    char buf[64];

    if(NULL == setting)
    {
        return FALSE;
    }
	
	api_get_string_option(SYSPRODUCTINFO_MODEL_VENDERNAME_OPTION,setting->vendor_name, sizeof(setting->vendor_name));
	api_get_string_option(SYSPRODUCTINFO_MODEL_MODELNAME_OPTION, setting->model_name, sizeof(setting->model_name));
	api_get_string_option(SYSPRODUCTINFO_MODEL_DESCRIPTION_OPTION, setting->model_description, sizeof(setting->model_description));

    /*PCI would like to take 11 letters for SN*/
    memset(buf, 0x00, sizeof(buf));
    rval = sysutil_interact(buf, sizeof(buf), "setconfig -g 0");
    buf[strcspn(buf, "\n") - 1] = '\0';
    sprintf(setting->product_sn, "%s", buf);

    /*PCI would like to take 5 letters for HW version*/
    memset(buf, 0x00, sizeof(buf));
    rval = sysutil_interact(buf, sizeof(buf), "setconfig -g 1");
    buf[strcspn(buf, "\n") - 1] = '\0';
    sprintf(setting->hardware_version, "%s", buf);

    /*PCI would like to take 17 letters for Lan Mac Address*/
    memset(buf, 0x00, sizeof(buf));
    rval = sysutil_interact(buf, sizeof(buf), "setconfig -g 6");
    buf[strcspn(buf, "\n") - 1] = '\0';
    sprintf(setting->lan_mac, "%s", buf);

    /*PCI would like to take 17 letters for Wlan Mac Address*/
    memset(buf, 0x00, sizeof(buf));
    rval = sysutil_interact(buf, sizeof(buf), "setconfig -g 8");
    buf[strcspn(buf, "\n") - 1] = '\0';
    sprintf(setting->wlan_mac, "%s", buf);

    return TRUE;
}

/*****************************************************************
* NAME:    get_timezone_capability_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_timezone_capability_cb(HTTPS_CB *pkt)
{
	if(pkt->json)
	{
		get_timezone_capability_json_cb(pkt);
	}

	return TRUE;
}

/*****************************************************************
* NAME:    get_systimes_etting_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_systime_setting_cb(HTTPS_CB *pkt)
{
    char *result;
    TIME_SETTINGS_T time_settings;
    int ntp_enable=0, zonenumber=0, day_light_enable=0;
    char year[5]={0}, month[3]={0}, day[3]={0}, hour[3]={0}, minute[3]={0}, sec[3]={0};

    if(NULL == pkt)
        return -1;

    memset(&time_settings, 0, sizeof(time_settings));

    api_get_system_ntp_enable_server_option(SYSTEM_NTP_ENABLE_SERVER_OPTION, &ntp_enable);
	if (ntp_enable==0)    // Manually
	{
        time_settings.ntp_used_index = 1;
	}
	else if (ntp_enable==1)    // sync NTP
	{
        time_settings.ntp_used_index = 0;
	}
    
    api_get_integer_option(SYSTEM_SYSTEM_ZONENUMBER_OPTION, &zonenumber);
    time_settings.timezone_id = zonenumber;

    api_get_string_option(SYSTEM_NTP_SERVER_LIST, time_settings.ntp_server_addr, sizeof(time_settings.ntp_server_addr));

    sysutil_get_local_time("%Y", year, sizeof(year));
    time_settings.year = atoi(year);
    sysutil_get_local_time("%m", month, sizeof(month));
    time_settings.month = atoi(month);
    sysutil_get_local_time("%d", day, sizeof(day));
    time_settings.day = atoi(day);
    sysutil_get_local_time("%H", hour, sizeof(hour));
    time_settings.hour = atoi(hour);
    sysutil_get_local_time("%M:", minute, sizeof(minute));
    time_settings.minute = atoi(minute);
    sysutil_get_local_time("%S", sec, sizeof(sec));
    time_settings.sec = atoi(sec);
    sysutil_get_local_time("%Y/%m/%d %H:%M:%S", time_settings.current_sys_time, sizeof(time_settings.current_sys_time));

    api_get_system_ntp_day_light_saving_enable_option(NTPCLIENT_DAYLIGHTSAVING_DAYLIGHTENABLE_OPTION, &day_light_enable);
    time_settings.daylight_saving_en = day_light_enable;

    result = OK_STR;

    if(pkt->json)
	{
		get_systime_setting_json_cb(pkt, &time_settings, result);
	}

	return TRUE;
}

/*****************************************************************
* NAME:    set_systimes_etting_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int set_systime_setting_cb(HTTPS_CB *pkt){
	char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
	char *return_str=ERROR_STR;
	char *ptr=NULL;
	char ntpserver[30]={0},command_1[64]={0},command_2[1024]={0},response[64]={0},login_response[1024]={0};
#if HAS_IPCAM
	char xrelayd_ip[128]={0};
#endif
	struct json_object;
	int ntpindex=0,timezoneID=0,year=0,month=0,day=0,hour=0,minute=0,second=0,daylightsaving=0;
	int auto_detection=0, support_auto_detection=0;
	
	if(NULL == query_str)
		goto send_pkt;	

	if((get_json_integer_from_query(query_str, &ntpindex, "NtpUsedIndex") == -1)
		|| (get_json_integer_from_query(query_str, &timezoneID, "TimeZoneID") == -1)
		|| (get_json_string_from_query(query_str, ntpserver, "NtpServer") == -1)
		|| (get_json_integer_from_query(query_str, &year, "Year") == -1)
		|| (get_json_integer_from_query(query_str, &month, "Month") == -1)
		|| (get_json_integer_from_query(query_str, &day, "Day") == -1)
		|| (get_json_integer_from_query(query_str, &hour, "Hour") == -1)
		|| (get_json_integer_from_query(query_str, &minute, "Minute") == -1)
		|| (get_json_integer_from_query(query_str, &second, "Second") == -1)
		|| (get_json_integer_from_query(query_str, &daylightsaving, "DayLightSavingEn")) == -1)
		{
			goto send_pkt;
		}
		//0: Sync with NTP
		//1: Sync with PC/Phone
		api_set_integer_option(SYSTEM_NTP_ENABLE_SERVER_OPTION,!(ntpindex));

		support_auto_detection = (get_json_integer_from_query(query_str, &auto_detection, "AutoDetection") == 1) ? 1 : 0;
		if( support_auto_detection == 1)
		{
			api_set_integer_option("system.ntp.auto_detect", auto_detection);
			if(auto_detection == 1)
			{
				api_set_string_option("system.@system[0].zonename", "auto", sizeof("auto"));
			}
			else
			{
				if(api_set_system_timezone_option(SYSTEM_SYSTEM_TIMEZONE_OPTION,api_timezone_table[timezoneID].timezone,sizeof(api_timezone_table[timezoneID].timezone)))
				{
					goto send_pkt;
				}
			}
		}
		else
		{
			// those 3 options should be synced
			//system.@system[0].timezone=UTC-8
			//system.@system[0].zonename=China, Hong Kong, Western Australia, Singapore, Taiwan, Russia
			//system.@system[0].zonenumber=18
			if(api_set_system_timezone_option(SYSTEM_SYSTEM_TIMEZONE_OPTION,api_timezone_table[timezoneID].timezone,sizeof(api_timezone_table[timezoneID].timezone)))
			{
				goto send_pkt;
			}
		}
		if(api_set_string_option(SYSTEM_NTP_SERVER_LIST,ntpserver,sizeof(ntpserver)))
		{
			goto send_pkt;
		}
		api_set_integer_option(NTPCLIENT_DAYLIGHTSAVING_DAYLIGHTENABLE_OPTION,daylightsaving);
#if HAS_SYSTIME
                api_set_integer_option("systime.systime.year",year);
                api_set_integer_option("systime.systime.month",month);
                api_set_integer_option("systime.systime.day",day);
                api_set_integer_option("systime.systime.hour",hour);
                api_set_integer_option("systime.systime.day",minute);
#else
		//YYYY-MM-DD hh:mm[:ss]
		//e.g. date -s '%04d-%02d-%02d %02d:%02d:%02d'
		if(ntpindex){
		        sprintf(command_1,"date -s \'%04d-%02d-%02d %02d:%02d:%02d\'",
			        year,month,day,hour,minute,second);
		        system(command_1);
                }
#if HAS_IPCAM
		//sync with IP Camera board
		api_get_string_option("xrelayd.xrelayd.conn_sec_ip", xrelayd_ip, sizeof(xrelayd_ip));
		sprintf(command_2,"app_client -i %s -m POST -a SetSysTimeSetting -e 1 -p \'{\"NtpUsedIndex\":%d,\"TimeZoneID\":%d,\"NtpServer\":\"%s\",\"Year\":%04d,\"Month\":%02d,\"Day\":%02d,\"Hour\":%02d,\"Minute\":%02d,\"Second\":%02d,\"DayLightSavingEn\":%d}\'",xrelayd_ip,ntpindex,ipcam_timezone[timezoneID],ntpserver,year,month,day,hour,minute,second,daylightsaving);
		sysutil_interact(response, sizeof(response),command_2);
		if(ptr = strstr(response,"ERROR"))
			goto send_pkt;
#endif	
#endif
		return_str=OK_STR;
		APPAGENT_SYSTEM("luci-reload auto system &");

send_pkt:
		/* Send response packet */
		send_simple_response(pkt,return_str);

	return;
}

int get_http_port_cb(HTTPS_CB *pkt)
{
        char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
        char *result=OK_STR;
        int http_port=80,second_http_port=81;
        
        if(NULL == pkt)
                return -1;
        
        api_get_integer_option("lighttpd.http.port",&http_port);
        api_get_integer_option("lighttpd.http.second_port",&second_http_port);

        if(pkt->json)
                get_http_port_json_cb(pkt,http_port,second_http_port,result);
}


int set_http_port_cb(HTTPS_CB *pkt)
{
        char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
        bool result = TRUE;
        int http_port=80,second_http_port=81;

        if(NULL == pkt)
                return -1;
        
        if(pkt->json)
                result &=set_http_port_json_cb(query_str,&http_port,&second_http_port);
        
        if(result == FALSE)
                goto send_pkt;

        api_set_integer_option("lighttpd.http.port",http_port);
        api_set_integer_option("lighttpd.http.second_port",second_http_port);
        system("uci commit");
	system("luci-reload lighttpd");
        
send_pkt:
        send_simple_response(pkt, (TRUE == result)?OK_STR:ERROR_STR);
        
        return 0;
}

/*****************************************************************
* NAME:    get_fw_upgrade_url_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_fw_upgrade_url_cb(HTTPS_CB *pkt)
{
        char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
        char fwUpgradeStr[128]={0},lanIP[32]={0},type[10]={0},port[8]={0};

        if(NULL == pkt)
                return -1;
        
        api_get_string_option(NETWORK_LAN_PROTO_OPTION,type,sizeof(type));
        if(!strcmp(type,"dhcp"))
                sysutil_get_lan_ipaddr(lanIP, sizeof(lanIP));
        else
                api_get_lan_ipaddr_option(NETWORK_LAN_IPADDR_OPTION,lanIP,sizeof(lanIP));

        if(api_get_string_option("lighttpd.http.port",port,sizeof(port)))
                strcpy(port,"80");

        sprintf(fwUpgradeStr, "http://%s:%s/%s",lanIP, port, FW_UPGRADE_HTM_STR);
    
        if(pkt->json)
                get_fw_upgrade_url_json_cb(pkt, fwUpgradeStr);
}

/*****************************************************************
* NAME:    get_config_upgrade_url_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_config_upgrade_url_cb(HTTPS_CB *pkt)
{
        char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
        char configUpgradeStr[128]={0},lanIP[32]={0},type[10]={0},port[8]={0};

        if(NULL == pkt)
                return -1;

        api_get_string_option(NETWORK_LAN_PROTO_OPTION,type,sizeof(type));
        if(!strcmp(type,"dhcp"))
                sysutil_get_lan_ipaddr(lanIP, sizeof(lanIP));
        else
                api_get_lan_ipaddr_option(NETWORK_LAN_IPADDR_OPTION,lanIP,sizeof(lanIP));

        if(api_get_string_option("lighttpd.http.port",port,sizeof(port)))
                strcpy(port,"80");

        sprintf(configUpgradeStr, "http://%s:%s/%s",lanIP, port, CONFIG_UPGRADE_HTM_STR);

        if(pkt->json)
                get_config_upgrade_url_json_cb(pkt, configUpgradeStr);
}

#if HAS_EG_AUTO_FW_CHECK
/*****************************************************************
* NAME:    get_fw_release_info_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_fw_release_info_cb(HTTPS_CB *pkt)
{
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
	char *result = ERROR_STR;
	char FWUpgradeInfo[1024]={0};
    int rval;
	int verCheck = 0;

	if(NULL == pkt)
    {
        return -1;
    }

	api_get_integer_option(SYSTEM_FIRMWARE_VERSION_CHECK_OPTION, &verCheck);
    if(verCheck==1 && sysutil_check_file_existed("/tmp/new_fw_info"))
    {
        rval=sysutil_interact(FWUpgradeInfo, sizeof(FWUpgradeInfo), "cat /tmp/new_fw_info");
        if(rval>=0)
        {
			//{"model":"ESR900","version":"v9.0.4","id":"34","file_size":"13267072","release_date":"2014-06-26","change_log":"1. Fix ESR900 issue.\r\n2. Fix GUI issue."};
			if(strstr(FWUpgradeInfo, "model") && strstr(FWUpgradeInfo, "version") && strstr(FWUpgradeInfo, "file_size"))
			{
				result = OK_STR;
			}
        }
    }

    if(pkt->json)
    {
        get_fw_release_info_json_cb(pkt, &FWUpgradeInfo, result);
    }

	return 0;
}

/*****************************************************************
* NAME:    set_auto_fw_upgrade_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int set_auto_fw_upgrade_cb(HTTPS_CB *pkt)
{
	char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
    bool result = TRUE;
    int fw_enable;

    if(NULL == pkt)
    {
        return -1;
    }

	if (get_json_integer_from_query(query_str, &fw_enable, "enable"))
	{
		result = FALSE;
		goto send_pkt;
	}

	api_set_integer_option(SYSTEM_FIRMWARE_VERSION_CHECK_OPTION, fw_enable);
	APPAGENT_SYSTEM("uci commit");

send_pkt:
    send_simple_response(pkt, (TRUE == result)?OK_STR:ERROR_STR);

    return 0;
}

/*****************************************************************
* NAME:    do_auto_fw_upgrade_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int do_auto_fw_upgrade_cb(HTTPS_CB *pkt)
{
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");

	char *return_str;
	char index_id[8]={0};

    // The default value in return packet is ERROR.
    return_str = ERROR_STR;

    if((NULL == pkt) || (NULL == query_str))
    {
        goto send_pkt;
    }

	if (get_json_string_from_query(query_str, index_id, "id") != 1)
	{
		goto send_pkt;
	}

	APPAGENT_SYSTEM("doAutoFWupgrade.sh %s &", index_id);

    /* send response packet */
    return_str = OK_STR;

send_pkt:
    send_simple_response(pkt, return_str);

    return 0;
}
#endif

/*****************************************************************
* NAME:    set_device_led_action_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int set_device_led_action_cb(HTTPS_CB *pkt)
{
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
    bool result = TRUE;
    int type, action;
    char *type_str, *action_str;
    char buf[80];

    if(NULL == pkt)
    {
        return -1;
    }

    memset(buf, 0x00, sizeof(buf));

    result = parse_device_led_action_cb(query_str, &type, &action);

    if(FINDPROC("led.sh") > 0)
    {
        KILLALL("led.sh");
        sleep(1);
    }

    if(result)
    {
        switch(action)
        {
            case 0:
                action_str = "set";
                break;
            case 1:
                action_str = "clear";
                break;
            case 2:
                action_str = "blink";
                break;
            default:
                result = FALSE;
                goto send_pkt;
                break;
        }

        switch(type)
        {
            case 0:
#ifdef POWER_LED_LED_NAME
                type_str = POWER_LED_LED_NAME;
#else
                result = FALSE;
                goto send_pkt;
#endif
                break;
            case 1:
#ifdef WLAN_2G_LED_NAME
                type_str = WLAN_2G_LED_NAME;
#else
                result = FALSE;
                goto send_pkt;
#endif
                break;
            case 2:
#ifdef WLAN_5G_LED_NAME
                type_str = WLAN_5G_LED_NAME;
#else
                result = FALSE;
                goto send_pkt;
#endif
                break;
            case 3:
#ifdef WLAN_WPS_LED_NAME
                type_str = WLAN_WPS_LED_NAME;
#else
                result = FALSE;
                goto send_pkt;
#endif
                break;
            case 4:
#ifdef POWER_LED_LED_NAME
                APPAGENT_SYSTEM("led.sh %s %s & ", action_str, POWER_LED_LED_NAME);
#endif
#ifdef WLAN_2G_LED_NAME
                APPAGENT_SYSTEM("led.sh %s %s & ", action_str, WLAN_2G_LED_NAME);
#endif
#ifdef WLAN_5G_LED_NAME
                APPAGENT_SYSTEM("led.sh %s %s & ", action_str, WLAN_5G_LED_NAME);
#endif
#ifdef WLAN_WPS_LED_NAME
                APPAGENT_SYSTEM("led.sh %s %s & ", action_str, WLAN_WPS_LED_NAME);
#endif
                goto send_pkt;
                break;
            default:
                result = FALSE;
                goto send_pkt;
                break;
        }

        APPAGENT_SYSTEM("led.sh %s %s & ", action_str, type_str);
    }

send_pkt:
    send_simple_response(pkt, (TRUE == result)?OK_STR:ERROR_STR);

    return 0;
}

#if SUPPORT_IPERF_THROUGHPUT_TEST
/*****************************************************************
* NAME:    is_throughput_test_in_process
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
bool is_throughput_test_in_process()
{
    bool result;
    char *ptr;
    char buf[256];

    result = FALSE;
    memset(buf, 0x00, sizeof(buf));

#if 0
    /* 1348 root       1556 R   iperf3 -6 -c fe80::8adc:96ff:fe17:4965%br-lan -i 2 -t 10 -w 256k -P 4 -p 60001 */
    sysutil_interact(buf, sizeof(buf), "ps -w | awk '/iperf3/{print $14}'");

    ptr = buf;

    while(NULL != (ptr = strchr(ptr, '\n')))
    {
        ptr++;

        if(strstr(ptr, "256k"))
        {
            result = TRUE;
            break;
        }
    }
#else
    sysutil_interact(buf, sizeof(buf), "ps -w | grep \"iperf3\" |  grep \"256k\" | grep -v \"grep\"");

    if(strlen(buf)) {
        result = TRUE;
    }
#endif

    return result;
}

/*****************************************************************
* NAME:    run_throughput_test_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int run_throughput_test_cb(HTTPS_CB *pkt)
{
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
    char *return_str;
    char destination[32];
    char buf[64]={0};
    char buf2[4]={0};

    int port;

    return_str = ERROR_STR;
    memset(destination, 0x00, sizeof(destination));

    if(is_throughput_test_in_process())
    {
        return_str = ERROR_PROCESS_IS_RUNNING_STR;
    }
    else
    {
        if(sysutil_check_file_existed(THROUGHPUT_TEST_RESULT_FILE))
        {
            SYSTEM("rm -f %s", THROUGHPUT_TEST_RESULT_FILE);
        }

        if(get_json_string_from_query(query_str, destination, "Destination"))
        {
            sysutil_interact(buf2, sizeof(buf2), "[ $(echo %s | grep -E \"^[a-fA-F0-9:]{17}$\") ] && echo -n 1 || echo -n 0", destination);
            if(atoi(buf2))
            {
                sys_interact(buf, sizeof(buf), "/sbin/mesh.sh mac_to_ipv6_unique_local %s | tr -d '\\n'", destination);
            }
            else
            {
                sprintf(buf, "%s", destination);
            }
            api_get_integer_option("iperf3.iperf3.server_port", &port);

            APPAGENT_SYSTEM("iperf3 -6 -c %s -i 2 -t 10 -w 256k -P 4 -p %d > %s &",
                    buf, port, THROUGHPUT_TEST_RESULT_FILE);

            return_str = OK_STR;
        }
    }

    send_simple_response(pkt, return_str);

    return 0;
}

/*****************************************************************
* NAME:    get_throughput_test_result_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_throughput_test_result_cb(HTTPS_CB *pkt)
{
    char *ptr;
    char *result = ERROR_STR;
    char upload[16], download[16];
    int rval;

    if(NULL == pkt)
    {
        return -1;
    }

    ptr = NULL;
    memset(upload, 0x00, sizeof(upload));
    memset(download, 0x00, sizeof(download));

    if(is_throughput_test_in_process())
    {
        result = ERROR_PROCESS_IS_RUNNING_STR;
    }
    else if(sysutil_check_file_existed(THROUGHPUT_TEST_RESULT_FILE))
    {
        sysutil_interact(upload, sizeof(upload),
                "cat %s | grep SUM | awk \'/sender/{print $6, $7}\'", THROUGHPUT_TEST_RESULT_FILE);
        sysutil_interact(download, sizeof(download),
                "cat %s | grep SUM | awk \'/receiver/{print $6, $7}\'", THROUGHPUT_TEST_RESULT_FILE);

        if((strcmp("---", upload)) && strcmp("---", download))
        {
            if(ptr = strchr(upload, '\n'))
            {
                *ptr = '\0';
            }
            if(ptr = strchr(download, '\n'))
            {
                *ptr = '\0';
            }

            result = OK_STR;
        }
    }

    get_throughput_test_result_json_cb(pkt, upload, download, result);

    return 0;
}
#endif

#if HAS_SPEEDTEST_THROUGHPUT_TEST
/*****************************************************************
* NAME:    run_speedtest_test_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int run_speedtest_test_cb(HTTPS_CB *pkt)
{
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
    char buf[16]={0};
    char server[256]={0};
    char *return_str;

    return_str = ERROR_STR;

    if(FINDPROC("speedtest-maste") > 0)
    {
        return_str = ERROR_PROCESS_IS_RUNNING_STR;
    }
    else
    {
        memset(buf, 0x00, sizeof(buf));
        sysutil_interact(buf, sizeof(buf), "checkInternet.sh");

        get_json_string_from_query(query_str, server, "Server");

        if(strcmp(buf, "Online"))
        {
            return_str = ERROR_NO_INTERNET_STR;
        }
        else
        {
            if (strlen(server))
            {
                APPAGENT_SYSTEM("speedtest-master -s %s -f 1 -d 10 -r > /tmp/speedtest_result &", server);
            }
            else
            {
                APPAGENT_SYSTEM("speedtest-master -f 1 -d 10 -r > /tmp/speedtest_result &");
            }

            return_str = OK_STR;
        }
    }

    send_simple_response(pkt, return_str);

    return 0;
}

/*****************************************************************
* NAME:    get_speedtest_test_result_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int get_speedtest_test_result_cb(HTTPS_CB *pkt)
{
    char *ptr;
    char *result = ERROR_STR;
    char buf[32]={0};
    int rval;
    SPEEDTEST_RESULT_T test_result;

    if(NULL == pkt)
    {
        return -1;
    }

    ptr = NULL;
    memset(buf, 0x00, sizeof(buf));
    memset(&test_result, 0x00, sizeof(test_result));

    sysutil_interact(buf, sizeof(buf), "checkInternet.sh");

    if(strcmp(buf, "Online"))
    {
        result = ERROR_NO_INTERNET_STR;
    }
    else if(FINDPROC("speedtest-maste") > 0)
    {
        result = ERROR_PROCESS_IS_RUNNING_STR;
    }
    else if(sysutil_check_file_existed("/tmp/speedtest_result"))
    {
		sysutil_interact(buf, sizeof(buf), "cat /tmp/speedtest_result | grep \"date\" | wc -l");
		if(atoi(buf))
		{
			result = OK_STR;
			sysutil_interact(buf, sizeof(buf), "cat /tmp/speedtest_result | grep Download | awk -F : '{print $2}' | tail -n 1");
			sscanf(buf, " %s %s", test_result.download, test_result.download_unit);
			sysutil_interact(buf, sizeof(buf), "cat /tmp/speedtest_result | grep Upload | awk -F : '{print $2}' | tail -n 1");
			sscanf(buf, " %s %s", test_result.upload, test_result.upload_unit);
			sysutil_interact(buf, sizeof(buf), "cat /tmp/speedtest_result | grep date | awk '{print $2 \" \" $3}' | tr -d \"\n\"");
			sprintf(test_result.date, "%s", buf);
			sysutil_interact(buf, sizeof(buf), "cat /tmp/speedtest_result | grep Country | awk '{print $2 \" \" $3}' | tr -d \"\n\"");
			sprintf(test_result.country, "%s", buf);
			sysutil_interact(buf, sizeof(buf), "cat /tmp/speedtest_result | grep Sponsor | awk '{print $2 \" \" $3}' | tr -d \"\n\"");
			sprintf(test_result.sponsor, "%s", buf);
		}
    }

    get_speedtest_test_result_json_cb(pkt, result, &test_result);

    return 0;
}

/*****************************************************************
* NAME:    find_speedtest_best_server
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int find_speedtest_best_server(HTTPS_CB *pkt)
{
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
    //char *query_str = (char *)pkt->body;
    char buf[16]={0};
    char *return_str;

    return_str = ERROR_STR;

    if(FINDPROC("speedtest-maste") > 0)
    {
        return_str = ERROR_PROCESS_IS_RUNNING_STR;
    }
    else
    {
        memset(buf, 0x00, sizeof(buf));
        sysutil_interact(buf, sizeof(buf), "checkInternet.sh");

        if(strcmp(buf, "Online"))
        {
            return_str = ERROR_NO_INTERNET_STR;
        }
        else
        {
            APPAGENT_SYSTEM("speedtest-master -n 1 &");
            return_str = OK_STR;
        }
    }

    send_simple_response(pkt, return_str);

    return 0;
}

/*****************************************************************
* NAME:    get_speedtest_best_server_result_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int get_speedtest_best_server_result_cb(HTTPS_CB *pkt)
{
    char *ptr;
    char *result = ERROR_STR;
    char buf[32]={0}, best_server_result[2048]={0};

    if(NULL == pkt)
    {
        return -1;
    }

    ptr = NULL;

    sysutil_interact(buf, sizeof(buf), "checkInternet.sh");

    if(strcmp(buf, "Online"))
    {
        result = ERROR_NO_INTERNET_STR;
    }
    else if(FINDPROC("speedtest-maste") > 0)
    {
        result = ERROR_PROCESS_IS_RUNNING_STR;
    }
    else if(sysutil_check_file_existed("/tmp/speedtest_nearest_servers_list"))
    {
		sysutil_interact(best_server_result, sizeof(best_server_result), "cat /tmp/speedtest_nearest_servers_list");
        result = OK_STR;
    }

    get_speedtest_best_server_result_json_cb(pkt, result, best_server_result);

    return 0;
}
#endif

/*****************************************************************
* NAME:    run_ping_test_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int run_ping_test_cb(HTTPS_CB *pkt)
{
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
    char *return_str;
    char destination[32];
    int count;

    count = 0;
    return_str = ERROR_STR;
    memset(destination, 0x00, sizeof(destination));

    if(FINDPROC("ping") > 0)
    {
        return_str = ERROR_PROCESS_IS_RUNNING_STR;
    }
    else
    {
        if(sysutil_check_file_existed(PING_TEST_RESULT_FILE))
        {
            SYSTEM("rm -f %s", PING_TEST_RESULT_FILE);
        }

        if((get_json_string_from_query(query_str, destination, "Destination")) &&
           (get_json_integer_from_query(query_str, &count, "NumberOfPing")))
        {
            APPAGENT_SYSTEM("ping -c %d %s > %s 2>&1 &", count, destination, PING_TEST_RESULT_FILE);

            return_str = OK_STR;
        }
    }

    send_simple_response(pkt, return_str);

    return 0;
}

/*****************************************************************
* NAME:    get_ping_test_result_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_ping_test_result_cb(HTTPS_CB *pkt)
{
    char *ptr;
    char *result;

    if(NULL == pkt)
    {
        return -1;
    }

    ptr = NULL;
    result = ERROR_STR;

    if(FINDPROC("ping") > 0)
    {
        result = ERROR_PROCESS_IS_RUNNING_STR;
    }
    else if(sysutil_check_file_existed(PING_TEST_RESULT_FILE))
    {
        result = OK_STR;
    }

    get_specific_test_result_json_cb(pkt, PING_TEST_RESULT_FILE, result);

    return 0;
}

/*****************************************************************
* NAME:    run_trace_route_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int run_trace_route_cb(HTTPS_CB *pkt)
{
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
    char *return_str;
    char destination[32];

    return_str = ERROR_STR;
    memset(destination, 0x00, sizeof(destination));

    if(FINDPROC("traceroute") > 0)
    {
        return_str = ERROR_PROCESS_IS_RUNNING_STR;
    }
    else
    {
        if(sysutil_check_file_existed(TRACE_ROUTE_RESULT_FILE))
        {
            SYSTEM("rm -f %s", TRACE_ROUTE_RESULT_FILE);
        }

        if(get_json_string_from_query(query_str, destination, "Destination"))
        {
            APPAGENT_SYSTEM("traceroute -I -q 3 -w 5 -m 30 -n  %s > %s 2>&1 &", destination, TRACE_ROUTE_RESULT_FILE);

            return_str = OK_STR;
        }
    }

    send_simple_response(pkt, return_str);

    return 0;
}

/*****************************************************************
* NAME:    get_trace_route_result_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_trace_route_result_cb(HTTPS_CB *pkt)
{
    char *ptr;
    char *result;

    if(NULL == pkt)
    {
        return -1;
    }

    ptr = NULL;
    result = ERROR_STR;

    if(FINDPROC("traceroute") > 0)
    {
        result = ERROR_PROCESS_IS_RUNNING_STR;
    }
    else if(sysutil_check_file_existed(TRACE_ROUTE_RESULT_FILE))
    {
        result = OK_STR;
    }

    get_specific_test_result_json_cb(pkt, TRACE_ROUTE_RESULT_FILE, result);

    return 0;
}

/*****************************************************************
* NAME:    set_sdcard_format_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int set_sdcard_format_cb(HTTPS_CB *pkt)
{
    if(NULL == pkt)
        return -1;

    //send the success response
    send_simple_response(pkt, OK_STR);

    char buf[256]={0}, sd_path[32]={0}, sd_name[32]={0};
	int rval;

    rval = sysutil_interact(buf, sizeof(buf), "mount | grep /mnt/sdcard");
    if(rval >= 0)
    {
//buf: /dev/sda1 on /mnt/sdcard type vfat (rw,relatime,fmask=0022,dmask=0022,codepage=cp437,iocharset=iso8859-1,shortname=mixed,errors=remount-ro)
        sscanf(buf,"%s on %*s type %*s %*s", sd_path);
        sscanf(sd_path,"/dev/%s", sd_name);

        SYSTEM("echo 'echo start > /tmp/sdcard_format_status; sdcard_format.sh %s; app_client.sh SetIPCamExSDcardMount; echo end > /tmp/sdcard_format_status;' > /tmp/sdcard_format.sh", sd_name);

        SYSTEM("chmod 755 /tmp/sdcard_format.sh");
        APPAGENT_SYSTEM("sh /tmp/sdcard_format.sh &");
    }

    return 0;
}

/*****************************************************************
* NAME:    get_sdcard_format_status_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_sdcard_format_status_cb(HTTPS_CB *pkt)
{
    if(NULL == pkt)
        return -1;

    char format_status[12]={0};

    if(sysutil_check_file_existed("/tmp/sdcard_format_status"))
	{
		sysutil_interact(format_status, sizeof(format_status), "cat /tmp/sdcard_format_status");

		if(strrchr(format_status,'\n'))
            format_status[strlen(format_status)-1]='\0';

		if(strcmp(format_status, "end")==0)
		{
			SYSTEM("rm -rf /tmp/sdcard_format_status");
		}
	}
	else
		strcpy(format_status, "none");

    //send the success response
    send_simple_response(pkt, format_status);

    return 0;
}

#if HAS_IPCAM
/*****************************************************************
* NAME:    set_ipcam_simple_nvr_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int set_ipcam_simple_nvr_cb(HTTPS_CB *pkt)
{
	char targeIP[16]={0};
#if HAS_REDIRECT_DEVICE_SETTING
    char *ret = NULL;
#endif

    if(NULL == pkt)
        return -1;

#if HAS_IPCAM
	api_get_string_option("xrelayd.xrelayd.conn_sec_ip", targeIP, sizeof(targeIP));
#else
	sprintf(targeIP,"%s",DEVICE_IPADDR);
#endif

#if HAS_REDIRECT_DEVICE_SETTING
    ret = redirect_to_target_device(pkt, targeIP);

    basic_json_response(pkt, ret);
    free(ret);
#else
    send_simple_response(pkt, ERROR_STR);
#endif

    return 0;
}

/*****************************************************************
* NAME:    get_rtsp_port_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_rtsp_port_cb(HTTPS_CB *pkt)
{
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
    char *result=OK_STR;
    int rtsp_port = 554;

    if(NULL == pkt)
        return -1;

    api_get_integer_option("firewall.rtsp.src_dport", &rtsp_port);

    if(pkt->json)
        get_rtsp_port_json_cb(pkt, rtsp_port, result);

    return 0;
}

/*****************************************************************
* NAME:    set_rtsp_port_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int set_rtsp_port_cb(HTTPS_CB *pkt)
{
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
    bool result = TRUE;
    int rtsp_port = 554;

    if(NULL == pkt)
        return -1;

    if(pkt->json)
        result &=parse_json_rtsp_port_json_cb(query_str, &rtsp_port);

    if(result == FALSE)
        goto send_pkt;

    api_set_integer_option("firewall.rtsp.src_dport", rtsp_port);
    system("uci commit");
    system("luci-reload firewall");

send_pkt:
    send_simple_response(pkt, (TRUE == result)?OK_STR:ERROR_STR);

    return 0;
}

/*****************************************************************
* NAME:    get_sdcard_sync_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_sdcard_sync_cb(HTTPS_CB *pkt)
{
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
    char buf[64];

    if(NULL == pkt)
        return -1;

    if (sysutil_interact(buf, sizeof(buf), "mount | grep /mnt/sdcard") >= 0)
    {
        APPAGENT_SYSTEM("app_client.sh SetIPCamExSDcardMount");
    }

    send_simple_response(pkt, OK_STR);

    return 0;
}

#endif

/*****************************************************************
* NAME:    get_system_information_with_ip_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_system_information_with_ip_cb(HTTPS_CB *pkt)
{
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
    char buf[64];

    if(NULL == pkt)
        return -1;

    get_json_system_information_with_ip_cb(pkt, OK_STR);

    return 0;
}
/*****************************************************************
* NAME:    get_has_been_connect_info_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:
******************************************************************/
int get_mesh_connected_history_cb(HTTPS_CB *pkt)
{
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");

    if(NULL == pkt)
        return -1;

    get_json_mesh_connected_history_cb(pkt, OK_STR);
}
/*****************************************************************
* NAME:    reboot_all_devices_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   RebootAllDevices
******************************************************************/
int reboot_all_devices_cb(HTTPS_CB *pkt)
{
    char mesh_key[64]={0};
    char mesh_id[64]={0};

    if(NULL == pkt)
        return -1;

    api_get_string_option("wireless.wifi1_mesh.aeskey", mesh_key, sizeof(mesh_key));
    api_get_string_option("wireless.wifi1_mesh.Mesh_id", mesh_id, sizeof(mesh_id));

    APPAGENT_SYSTEM("/sbin/mesh.sh mesh_reboot_all_device %s %s", mesh_id, mesh_key);

    APPAGENT_SYSTEM("echo reboot | at now + 2 minutes");

    send_simple_response(pkt, OK_STR);

    return 0;
}
/*****************************************************************
* NAME:    reset_all_devices_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   ResetAllDevices
******************************************************************/
int reset_all_devices_cb(HTTPS_CB *pkt)
{
    char mesh_key[64]={0};
    char mesh_id[64]={0};

    if(NULL == pkt)
        return -1;

    api_get_string_option("wireless.wifi1_mesh.aeskey", mesh_key, sizeof(mesh_key));
    api_get_string_option("wireless.wifi1_mesh.Mesh_id", mesh_id, sizeof(mesh_id));

    APPAGENT_SYSTEM("/sbin/mesh.sh mesh_reset_all_device %s %s", mesh_id, mesh_key);

    send_simple_response(pkt, OK_STR);

    return 0;
}
/*****************************************************************
* NAME:    update_mesh_node_info_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int update_mesh_node_info_cb(HTTPS_CB *pkt)
{
    char mesh_key[64]={0};
    char mesh_id[64]={0};

    if(NULL == pkt)
        return -1;

    api_get_string_option("wireless.wifi1_mesh.aeskey", mesh_key, sizeof(mesh_key));
    api_get_string_option("wireless.wifi1_mesh.Mesh_id", mesh_id, sizeof(mesh_id));

    APPAGENT_SYSTEM("/sbin/mesh.sh get_mesh_global_node_info %s %s &", mesh_id, mesh_key);

    send_simple_response(pkt, OK_STR);

    return 0;
}

#if FOR_SC
/*****************************************************************
* NAME:    set_sc_fw_auto_upgrade_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int set_sc_fw_auto_upgrade_cb(HTTPS_CB *pkt)
{
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
    char fw_auto_upgrade[8]={0};

    if(NULL == pkt)
        return -1;

    get_json_string_from_query(query_str, fw_auto_upgrade, "FwAutoUpgrade");
    api_set_integer_option("autofw.config.FW_action", (atoi(fw_auto_upgrade)==1)?0:4);
    APPAGENT_SYSTEM("uci commit autofw");
    APPAGENT_SYSTEM("/usr/shc/autofw start");

    send_simple_response(pkt, OK_STR);

    return 0;
}

/*****************************************************************
* NAME:    execute_sc_firmware_upgrade_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int execute_sc_firmware_upgrade_cb(HTTPS_CB *pkt)
{
    char *query_str;
    SC_AUTO_FIRMWARE_UPGRADE action;
    bool boolean;
    bool new_firmware;
    char buf[128];
    char file_size[32];
    char ori_ver[32];
    char new_ver[32];
    char *result;
    char *ptr;
    int ori, new;
    int percentage;
    int fw_action_ori;

    if(NULL == pkt)
        return -1;

    boolean = FALSE;
    new_firmware = FALSE;
    result = ERROR_STR;
    percentage = 0;
    fw_action_ori = 0;

    query_str = get_env(&pkt->envcfg, "QUERY_STRING");

    get_json_integer_from_query(query_str, (int *)&action, "Action");

    switch(action)
    {
        case SC_ENABLE_AUTO_FIRMWARE_UPGRADE:
            get_json_boolean_from_query(query_str, &boolean, "EnableAutoFirmwareUpgrade");

            if(boolean)
            {
                api_set_integer_option("autofw.config.FW_action",0);
            }
            else
            {
                api_set_integer_option("autofw.config.FW_action",4);
            }

            APPAGENT_SYSTEM(AUTO_FIRMWARE_INIT_SCRIPT" start");

            result = OK_STR;
            break;
        case SC_CHECK_NEW_FIRMWARE:
            api_get_integer_option("autofw.config.FW_action", &fw_action_ori);
            APPAGENT_SYSTEM("rm -f "FIRMWARE_INFO_LOG_FILE);

            api_set_string_option("autofw.config.GUI_action", "check", sizeof("check"));
            api_set_integer_option("autofw.config.FW_action", 0);
            api_set_integer_option("autofw.config.FW_action_ori",fw_action_ori);

            APPAGENT_SYSTEM(AUTO_FIRMWARE_INIT_SCRIPT" start");

            result = OK_STR;
            break;
        case SC_CHECK_NEW_FIRMWARE_RESULT:
            if(sysutil_check_file_existed(FIRMWARE_INFO_LOG_FILE))
            {
                memset(ori_ver, 0x00, sizeof(ori_ver));
                memset(new_ver, 0x00, sizeof(new_ver));

                sys_interact(ori_ver, sizeof(ori_ver), "cat %s | awk -F\":\" '/FW_running/{print $2}'", FIRMWARE_INFO_LOG_FILE);
                sys_interact(new_ver, sizeof(new_ver), "cat %s | awk -F\":\" '/FW_onServer/{print $2}'", FIRMWARE_INFO_LOG_FILE);

                /* Remove the character '\n' */
                ptr = strchr(ori_ver, '\n');
                *ptr = '\0';
                ptr = strchr(new_ver, '\n');
                *ptr = '\0';

                /* Remove the character '.' */
                ptr = strchr(ori_ver, '.');
                *ptr = '\0';
                ori = (atoi(&ori_ver[0]) * 1000) + atoi(ptr + 1);

                ptr = strchr(new_ver, '.');
                *ptr = '\0';
                new = (atoi(&new_ver[0]) * 1000) + atoi(ptr + 1);

                new_firmware = (new > ori)?TRUE:FALSE;

                result = OK_STR;
            }
            break;
        case SC_DOWNLOAD_NEW_FIRMWARE:
            if(sysutil_check_file_existed(FIRMWARE_DOWNLOAD_STATUS_FILE))
            {
                memset(buf, 0x00, sizeof(buf));
                sys_interact(buf, sizeof(buf), "cat "FIRMWARE_DOWNLOAD_STATUS_FILE);

                ptr = strchr(buf, '\n');
                *ptr = '\0';

                percentage = atoi(buf);
            }

            if(0 == percentage || 100 == percentage)
            {
                api_get_integer_option("autofw.config.FW_action", &fw_action_ori);
                api_set_string_option("autofw.config.GUI_action", "download", sizeof("download"));
                api_set_integer_option("autofw.config.FW_action",1);
                api_set_integer_option("autofw.config.FW_action_ori",fw_action_ori);

                APPAGENT_SYSTEM(AUTO_FIRMWARE_INIT_SCRIPT" start");

                result = OK_STR;
            }
            else if(0 < percentage && 100 > percentage)
            {
                result = ERROR_PROCESS_IS_RUNNING_STR;
            }

            break;
        case SC_DOWNLOAD_FIRMWARE_STATUS:
            if(sysutil_check_file_existed(FIRMWARE_DOWNLOAD_STATUS_FILE))
            {
                memset(buf, 0x00, sizeof(buf));
                sys_interact(buf, sizeof(buf), "cat "FIRMWARE_DOWNLOAD_STATUS_FILE);

                ptr = strchr(buf, '\n');
                *ptr = '\0';

                percentage = atoi(buf);

                /* Change the firmware's name to firmware.img when the firmware is downloaded. */
                if(100 == percentage)
                {
                    APPAGENT_SYSTEM(AUTO_FIRMWARE_INIT_SCRIPT" prepare_to_update");

                    memset(file_size, 0x00, sizeof(file_size));
                    sys_interact(file_size, sizeof(file_size), "ls -l %s | awk '{print $5}'", DOWNLOADED_FIRMWARE_FILE);

                    ptr = strchr(file_size, '\n');
                    *ptr = '\0';
                }

                result = OK_STR;

            }
            break;
        case SC_EXECUTE_FIRMWARE_UPGRADE:
            if(sysutil_check_file_existed(DOWNLOADED_FIRMWARE_FILE))
            {
                APPAGENT_SYSTEM("sysupgrade -v "DOWNLOADED_FIRMWARE_FILE);

                result = OK_STR;
            }
            break;
        default:
            break;
    }

    execute_sc_firmware_upgrade_json_cb(pkt, new_firmware, percentage, file_size, result);

    return 0;
}
#endif

/*****************************************************************
* NAME:    get_system_throughput_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_system_throughput_cb(HTTPS_CB *pkt){

    char *result, *ptr_tx, *ptr_rx, *ptr_total;
    char buf[128], download_rate[32], upload_rate[32];
    char opmode[3]={0};
    int rval;
    char wanIp[15+1]={0};
    char wan_interface[5]={0};
    int get_ip = 1;

    if(NULL == pkt)
    {
        return -1;
    }


    memset(wanIp,0,sizeof(wanIp));
#if SUPPORT_WAN_SETTING
      api_get_string_option("system.@system[0].opmode", opmode, sizeof(opmode));

      if ( strcmp(opmode,"ar") == 0) {
          api_get_string_option("network.wan.ifname", wan_interface, sizeof(wan_interface));
          get_ip = sysutil_get_wan_ipaddr(wanIp, sizeof(wanIp)); // lan wan same ip
      }
      else
      {
          sysutil_interact(wan_interface,sizeof(wan_interface), "cat /tmp/wandev");
          get_ip = sysutil_get_lan_ipaddr(wanIp, sizeof(wanIp)); // lan wan same ip
      }
#else
      get_ip = 0;
#endif
    if (get_ip == 0 )
    {
        strcpy(wanIp,"---");
    }

    if (0 != strcmp(wanIp, "---")) //connect
    {
        rval = sysutil_interact(buf, sizeof(buf), " bwm-ng -I %s -d 1 -c 1 |grep %s",wan_interface ,wan_interface);

        if(rval >= 0)
        {
            sscanf(buf, "%*s %[^s]s %[^s]s ", download_rate, upload_rate);
            strcat(download_rate, "s");
            strcat(upload_rate, "s");
            result = OK_STR;
        }
        else
        {
            result = ERROR_STR;
        }
    }
    else //disconnect
    {
        sprintf(download_rate,"%s","0.00  B/s");
        sprintf(upload_rate,"%s","0.00  B/s");
        result = OK_STR;
    }

    if(pkt->json)
    {
        get_system_throughput_json_cb(pkt, download_rate, upload_rate, result);
    }

    return 0;
}

/*****************************************************************
* NAME:    execute_firmware_upgrade_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int execute_firmware_upgrade_cb(HTTPS_CB *pkt)
{
    char *query_str;
    FIRMWARE_UPGRADE action;
    bool boolean;
    bool new_firmware;
    char buf[128]={0};
    char file_size[32]={0};
    char ori_ver[32]={0};
    char new_ver[32]={0};
    char now_fw_size[64]={0};
    char total_fw_size[64]={0};
    char fwID[32]={0};
    char fw_log[1024]={0};
    char *result;
    char *ptr;
    char *saveptr1, *saveptr2, *ptr_ori, *ptr_new;
    int ori, new, now_size, total_size;
    int percentage;
    int fw_action_ori;
    char release_date[32]={0};
    char change_log[1024]={0};
    char version[32]={0};

    if(NULL == pkt)
        return -1;

    boolean = FALSE;
    new_firmware = FALSE;
    result = ERROR_STR;
    percentage = 0;
    fw_action_ori = 0;

    query_str = get_env(&pkt->envcfg, "QUERY_STRING");

    get_json_integer_from_query(query_str, (int *)&action, "Action");
    
    switch(action)
    {
        case ENABLE_AUTO_FIRMWARE_UPGRADE:
            get_json_boolean_from_query(query_str, &boolean, "EnableAutoFirmwareUpgrade");

            if(boolean)
            {
                api_set_integer_option("autofw.config.FW_action",0);
            }
            else
            {
                api_set_integer_option("autofw.config.FW_action",4);
            }

            //APPAGENT_SYSTEM(AUTO_FIRMWARE_INIT_SCRIPT" start");

            result = OK_STR;
            break;
        case CHECK_NEW_FIRMWARE:
            if(sysutil_check_file_existed(CHKNEWFW_SCRIPT))
            {
                APPAGENT_SYSTEM(CHKNEWFW_SCRIPT);
                result = OK_STR; 
            }           
            break;
        case CHECK_NEW_FIRMWARE_RESULT:
        
            if(sysutil_check_file_existed(FIRMWARE_INFO_LOG_FILE))
            {
                memset(fw_log, 0x00, sizeof(fw_log));
                sys_interact(fw_log, sizeof(fw_log), "cat %s ", FIRMWARE_INFO_LOG_FILE);
        
                if( strcmp(fw_log, "0") != 0)
                {
                    memset(ori_ver, 0x00, sizeof(ori_ver));
                    memset(new_ver, 0x00, sizeof(new_ver));
                    memset(release_date, 0x00, sizeof(release_date));
                    memset(change_log, 0x00, sizeof(change_log));
                    memset(version, 0x00, sizeof(version));

                    sys_interact(new_ver, sizeof(new_ver), "cat %s |awk -F\":\" '{print $4}' |awk -F'\"' '{print $2}' |awk -F \"-\" '{print $1}' |cut -c '2-'", FIRMWARE_INFO_LOG_FILE);
                    //sys_interact(ori_ver, sizeof(ori_ver), "cat /etc/version | grep Firmware | awk 'BEGIN{FS= \" \"} {print $4}'");
                    sys_interact(ori_ver, sizeof(ori_ver), "sh /sbin/minversion.sh");
                    //printf("=================%s\n", ori_ver);
                    sys_interact(release_date, sizeof(release_date), "cat %s |awk -F\":\" '{print $10}' |awk -F'\"' '{print $2}'", FIRMWARE_INFO_LOG_FILE);
                    sys_interact(change_log, sizeof(change_log), "cat %s |awk -F\":\" '{print $12}' |awk -F'\"' '{print $2}'", FIRMWARE_INFO_LOG_FILE);
                    sys_interact(version, sizeof(version), "cat %s |awk -F\":\" '{print $4}' |awk -F'\"' '{print $2}' |awk -F \"-\" '{print $1}' |cut -c '2-'", FIRMWARE_INFO_LOG_FILE);

                    // Remove the character '\n'
                    new_ver[strlen(new_ver)-1]=0;
                    ori_ver[strlen(ori_ver)-1]=0;
                    release_date[strlen(release_date)-1]=0;
                    change_log[strlen(change_log)-1]=0;
                    version[strlen(version)-1]=0;

                    ptr_new = strtok_r(new_ver, ".", &saveptr1);   
                    ptr_ori = strtok_r(ori_ver, ".", &saveptr2);                 
                    
                    while (ptr_ori != NULL && ptr_new != NULL)
                    {
                        if (atoi(ptr_ori) > atoi(ptr_new))
                        {
                            new_firmware = FALSE;
                            break;
                        }
                        else if (atoi(ptr_ori) < atoi(ptr_new))
                        {
                            new_firmware = TRUE;
                            break;
                        }

                        ptr_new = strtok_r(saveptr1, ".", &saveptr1);
                        ptr_ori = strtok_r(saveptr2, ".", &saveptr2);
                    }               
                }
                result = OK_STR;
            }
            break;
        case DOWNLOAD_NEW_FIRMWARE:
            if( sysutil_check_file_existed(FIRMWARE_INFO_LOG_FILE) && sysutil_check_file_existed(DOWNLOAD_FW_SCRIPT) )
            {
                sys_interact(fwID, sizeof(fwID), "cat %s | awk -F\":\" '{print $5}' |awk -F\",\" '{print $1}' ", FIRMWARE_INFO_LOG_FILE);
                APPAGENT_SYSTEM(DOWNLOAD_FW_SCRIPT" %s", fwID);
                result = OK_STR;
            }

            break;
        case DOWNLOAD_FIRMWARE_STATUS:
            memset(file_size, 0x00, sizeof(file_size));
            if(sysutil_check_file_existed(FIRMWARE_INFO_LOG_FILE))
            {
                memset(now_fw_size, 0x00, sizeof(now_fw_size));
                memset(total_fw_size, 0x00, sizeof(total_fw_size));

                sys_interact(now_fw_size, sizeof(now_fw_size), "ls -al /tmp |grep firmware.img |awk -F\" \" '{print $5}'");
                sys_interact(total_fw_size, sizeof(total_fw_size), "cat %s | awk -F\":\" '{print $9}' |awk -F\",\" '{print $1}'", FIRMWARE_INFO_LOG_FILE);
                now_size = atoi(now_fw_size);
                total_size = atoi(total_fw_size);
                percentage = (now_size / total_size )*100;

                if(100 == percentage)
                {   
                    //strcpy(file_size, total_fw_size);
                    strncpy(file_size,total_fw_size,strlen(total_fw_size)-1);
                }
                result = OK_STR;
            }
            break;
        case EXECUTE_FIRMWARE_UPGRADE:
            // if(sysutil_check_file_existed(DOWNLOADED_FIRMWARE_FILE))
            // {
            //     APPAGENT_SYSTEM("sysupgrade -v "DOWNLOADED_FIRMWARE_FILE);

            //     result = OK_STR;
            // }
            break;
        default:
            break;
    }

    execute_firmware_upgrade_json_cb(pkt, new_firmware, percentage, file_size, result, release_date, change_log, version);

    return 0;
}



/*****************************************************************
* NAME:    do_manual_firmware_upgrade_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int do_manual_firmware_upgrade_cb(HTTPS_CB *pkt)
{
    char *query_str;
    FIRMWARE_UPGRADE action;
    char buf[128]={0};
    char ipaddr[32]={0};
    char gatewayMac[32]={0};
    char meshMac[32]={0};
    char deviceType[32]={0};
    char *result;
    char target_ip[1024];
    long download_size, total_size;
    int doAction=1;
    char *buf_ip;
    char redirectIP[1024]={0};
    char InternetSt[64]={0};
    
    int i=0,j=0;

    if(NULL == pkt)
        return -1;

    result = ERROR_STR;

    query_str = get_env(&pkt->envcfg, "QUERY_STRING");
    memset(InternetSt, 0x00, sizeof(InternetSt));
    sysutil_interact(InternetSt, sizeof(InternetSt), "checkInternet.sh");

    if( strcmp(InternetSt, "Online") != 0)
    {
        send_simple_response(pkt, ERROR_NO_INTERNET_STR);
        return -1;
    }

    memset(deviceType, 0x00, sizeof(deviceType));
    api_get_string_option("mesh.wifi.controller", deviceType, sizeof(deviceType));

//    memset(target_ip, 0x00, sizeof(target_ip));
  //  get_json_string_from_query(query_str, target_ip, "TargetMeshIP");

    get_json_integer_from_query(query_str, (int *)&action, "Action");

    if (1 != action)
    {
        if (0 == strcmp(deviceType, "master"))
        {
            doAction = 0;
            memset(target_ip, 0x00, sizeof(target_ip));
            get_json_string_from_query(query_str, target_ip, "TargetMeshIP");
            sysutil_interact(ipaddr, sizeof(ipaddr), "ifconfig br-lan |grep 'inet addr'| awk '{print $2}' | awk -F: '{print $2}' | tr -d '\n'");
            buf_ip = strtok(target_ip, ",");
            while (buf_ip != NULL)
            {
                //cprintf("-------------------FUNCTION:%s-------LINE:%d-------buf_ip:%s-----------\n", __FUNCTION__, __LINE__, buf_ip);
                if (strcmp(buf_ip, ipaddr) != 0)  //filter master's ip in TargerMeshIP
                {
                    if (i==0)
                    {
                        j += sprintf(redirectIP + j,"%s", buf_ip);
                    }
                    else
                    {
                        j += sprintf(redirectIP + j, ",%s", buf_ip);
                    }
                    i++;
                }
                else
                {
                    //TargerMeshIP has master, do the action
                    doAction = 1;
                }
                buf_ip = strtok(NULL, ",");
            }
            //cprintf("-------------------FUNCTION:%s-------LINE:%d-------redirectIP:%s-----------\n", __FUNCTION__, __LINE__, redirectIP);
            /* Send to target MESH device */
    #if HAS_REDIRECT_DEVICE_SETTING
            redirect_to_target(pkt, redirectIP);
    #endif
        }
    }
    /* If the target MESH IP does not exist, then the packet is for the DUT itself. */
    //if(0 == strlen(target_ip) || check_lan_ip(target_ip))
    {
        switch(action)
        {
            case ENABLE_AUTO_FIRMWARE_UPGRADE:
                break;
            case CHECK_NEW_FIRMWARE:
                //cprintf("=========================action1=========================\n");
                if(sysutil_check_file_existed(UPGRADE_LIST))
                {
                    APPAGENT_SYSTEM("rm %s", UPGRADE_LIST);
                }
                if(sysutil_check_file_existed(GEN_UPGRADELIST_LUA))
                {
                    APPAGENT_SYSTEM("lua "GEN_UPGRADELIST_LUA);
                }
                result = OK_STR;
                get_download_firmware_result_json_cb(pkt, result, action);
                break;
            case CHECK_NEW_FIRMWARE_RESULT:
                //cprintf("=========================action2=========================\n");
                if(sysutil_check_file_existed(UPGRADE_LIST))
                {
                    result = OK_STR;
                }
                get_new_firmware_result_json_cb(pkt, result, action);

                break;
            case DOWNLOAD_NEW_FIRMWARE:
                //cprintf("=========================action3=========================\n");
                if(sysutil_check_file_existed(DOWNLOAD_FW_SCRIPT))
                {
                    APPAGENT_SYSTEM("rm %s", FIRMWARE_DOWNLOAD_STATUS);
                    if (doAction)
                    {
                        APPAGENT_SYSTEM(DOWNLOAD_FW_SCRIPT" %s &", "manualDownload");
                    }
                }
                result = OK_STR;
                get_download_firmware_result_json_cb(pkt, result, action);

                break;
            case DOWNLOAD_FIRMWARE_STATUS:
                //cprintf("=========================action4=========================\n");
                /*
                APPAGENT_SYSTEM("rm %s", FIRMWARE_DOWNLOAD_STATUS);

                
                memset(buf, 0x00, sizeof(buf));

    			if(sysutil_check_file_existed(FIRMWARE_IMAGE))
    			{
                    memset(ipaddr, 0x00, sizeof(ipaddr));
                    memset(gatewayMac, 0x00, sizeof(gatewayMac));
                    memset(meshMac, 0x00, sizeof(meshMac));                
                    memset(buf, 0x00, sizeof(buf));

                    sysutil_interact(ipaddr, sizeof(ipaddr), "ifconfig br-lan |grep 'inet addr'| awk '{print $2}' | awk -F: '{print $2}' | tr -d '\n'");
                    sysutil_interact(gatewayMac, sizeof(gatewayMac), "batctl gwl | tail -n +2 | awk -F' ' '{ print $2 }' | tr -d '\n'");
                    sysutil_interact(meshMac, sizeof(meshMac), "mesh.sh get_myselfmac");
                    meshMac[strlen(meshMac)-1]=0;
                    sysutil_interact(buf, sizeof(buf), "cat %s  | awk -F':' '{print $5}' |awk -F',' '{print $1}' |awk -F'\"' '{print $2}'", FIRMWARE_INFO_LOG_FILE);
                    total_size = atoi(buf);

                    memset(buf, 0x00, sizeof(buf));
                    sysutil_interact(buf, sizeof(buf), "ls -l %s | awk '{print $5}'", FIRMWARE_IMAGE);
                    download_size = atoi(buf);

                    if(download_size == total_size)
                    { 
                        if (strcmp(deviceType, "client") == 0)
                        {
                            APPAGENT_SYSTEM("mesh_control 'echo \\\"1 %s %s\\\" >>%s' '%s'", meshMac, ipaddr, FIRMWARE_DOWNLOAD_STATUS, gatewayMac);
                        }
                        else
                        { 
                            APPAGENT_SYSTEM("echo \"1 %s %s\" >>%s", meshMac, ipaddr, FIRMWARE_DOWNLOAD_STATUS);
                        }
                    }
                    else
                    {
                        if (strcmp(deviceType, "client") == 0)
                        { 
                            APPAGENT_SYSTEM("mesh_control 'echo \\\"0 %s %s\\\" >>%s' '%s'", meshMac, ipaddr, FIRMWARE_DOWNLOAD_STATUS, gatewayMac);
                        }
                        else
                        { 
                            APPAGENT_SYSTEM("echo \"0 %s %s\" >>%s", meshMac, ipaddr, FIRMWARE_DOWNLOAD_STATUS);
                        }
                    }
    			}
                */
                result = OK_STR;

                get_download_firmware_result_json_cb(pkt, result, action);
                break;
            case EXECUTE_FIRMWARE_UPGRADE:
                //cprintf("=========================action5=========================\n");
                if(sysutil_check_file_existed(FIRMWARE_IMAGE) && doAction==1)
                 {
                     APPAGENT_SYSTEM(DOWNLOAD_FW_SCRIPT" %s &", "upgrade");
                 }
                 result = OK_STR;
                 get_download_firmware_result_json_cb(pkt, result, action);

                 break;
            default:
                 break;
        }
    }


    return 0;
}

/*****************************************************************
* NAME:    set_fw_auto_upgrade_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int set_fw_auto_upgrade_cb(HTTPS_CB *pkt)
{
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
    char fw_auto_upgrade[8]={0};

    if(NULL == pkt)
        return -1;    

    get_json_string_from_query(query_str, fw_auto_upgrade, "FwAutoUpgrade");
    api_set_integer_option("autofw.config.enable", (atoi(fw_auto_upgrade)==1)?1:0);
//    APPAGENT_SYSTEM("uci commit autofw");
	APPAGENT_SYSTEM("luci-reload auto autofw &");
    // APPAGENT_SYSTEM("/sbin/autoFwUpgrade.sh");
//    SYSTEM("/sbin/autoFwUpgrade.sh &");
    send_simple_response(pkt, OK_STR);

    return 0;
}
/*****************************************************************
* NAME:    get_base_status_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_base_status_cb(HTTPS_CB *pkt)
{
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
    char buf[64];

    char *result;
    int AutoFwEnable;
    char ExternalIP[32];
    char FwVersion[32];
    char MainIP[32];
    char ProductCode[32];
    char upTime[32];
    char InternetSt[64]={0};
    int days=0, hours=0, minutes=0, seconds=0;

    result = OK_STR;
    AutoFwEnable = FALSE;

//    memset(AutoFwEnable, 0x00, sizeof(AutoFwEnable));
    memset(ExternalIP, 0x00, sizeof(ExternalIP));
    memset(InternetSt, 0x00, sizeof(InternetSt));
    memset(FwVersion, 0x00, sizeof(FwVersion));
    memset(MainIP, 0x00, sizeof(MainIP));
    memset(ProductCode, 0x00, sizeof(ProductCode));
    memset(upTime, 0x00, sizeof(upTime));

    api_get_integer_option("autofw.config.enable",&AutoFwEnable);

    sysutil_interact(InternetSt, sizeof(InternetSt), "checkInternet.sh");
    if( strcmp(InternetSt, "Offline") != 0)
    {
        sysutil_interact(ExternalIP, sizeof(ExternalIP), "curl --connect-timeout 2 -s icanhazip.com");
        ExternalIP[strlen(ExternalIP)-1]=0;
    }

    sysutil_interact(FwVersion, sizeof(FwVersion), "cat /etc/version | grep Firmware | awk '{print $4}'");
    FwVersion[strlen(FwVersion)-1]=0;

    sysutil_interact(MainIP, sizeof(MainIP), "ifconfig br-lan | awk '/inet addr/{print substr($2,6)}'");
    MainIP[strlen(MainIP)-1]=0;

    api_get_string_option("sysProductInfo.model.modelName", ProductCode, sizeof(ProductCode));

    if (sysutil_get_uptime(upTime, sizeof(upTime)))
    {
        sscanf(upTime, "%d days ,%d hours ,%d minutes ,%d seconds", &days, &hours, &minutes, &seconds);
        if(days > 0)
                sprintf(upTime, "%d days %d hours %d min %ld sec", days, hours, minutes, seconds);
        else if(hours > 0)
                sprintf(upTime, "%d hours %d min %ld sec", hours, minutes, seconds);
        else if(minutes > 0)
                sprintf(upTime, "%d min %ld sec", minutes, seconds);
        else
                sprintf(upTime, "%ld sec", seconds);
    }

    if(NULL == pkt)
        return -1;

    get_json_base_status_cb(pkt, result, AutoFwEnable, ExternalIP, FwVersion, MainIP, ProductCode, upTime);

    return 0;
}

/*****************************************************************
* NAME:    get_mesh_node_simplify_info_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_mesh_node_simplify_info_cb(HTTPS_CB *pkt)
{
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");    

    if(NULL == pkt)
        return -1;

    get_json_mesh_node_simplify_info_cb(pkt, OK_STR);

    return 0;
}

/*****************************************************************
* NAME:    get_SN_number_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_SN_number_cb(HTTPS_CB *pkt)
{
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");    
    char SN_number[64]={0};
    char r_domain[16]={0};
    char VenderName[32]={0};
    char MeshDeviceLocation[32]={0};
    char *result;
    char eth_status_str[2];
    int eth_status=0;

    result = OK_STR;

    if(NULL == pkt)
        return -1;

    memset(SN_number, 0x00, sizeof(SN_number));
    memset(VenderName, 0x00, sizeof(VenderName));
    memset(r_domain, 0x00, sizeof(r_domain));
    memset(MeshDeviceLocation, 0x00, sizeof(MeshDeviceLocation));

    sysutil_interact(SN_number, sizeof(SN_number), "setconfig -g 0");
    SN_number[strlen(SN_number)-1]=0;

    sysutil_interact(r_domain, sizeof(r_domain), "setconfig -g 4");
    r_domain[strlen(r_domain)-1]=0;

    api_get_string_option("sysProductInfo.model.venderName", VenderName, sizeof(VenderName));

    api_get_string_option("wireless.wifi0_mesh.MeshDeviceLocation", MeshDeviceLocation, sizeof(MeshDeviceLocation));

    sysutil_interact(eth_status_str, sizeof(eth_status_str), "cat /sys/class/net/eth0/carrier | tr -d \"\n\"");
    eth_status+=atoi(eth_status_str);
//    printf("eth0 status : %d, final %d\n", atoi(eth_status_str), eth_status);
    sysutil_interact(eth_status_str, sizeof(eth_status_str), "cat /sys/class/net/eth1/carrier | tr -d \"\n\"");
    eth_status+=atoi(eth_status_str);
//    printf("eth1 status : %d, final %d\n", atoi(eth_status_str), eth_status);

    get_json_SN_number_cb(pkt, result, SN_number, r_domain, VenderName, MeshDeviceLocation, eth_status);

    return 0;
}

/*****************************************************************
* NAME:    get_internet_status_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int get_internet_status_cb(HTTPS_CB *pkt)
{
	int status=0;
	char buf[4]={0};

	if(NULL == pkt)
		return -1;

	if(sysutil_check_file_existed("/tmp/cur_master_internet_status"))
	{
		sysutil_interact(buf, sizeof(buf), "cat /tmp/cur_master_internet_status | tr -d \"\n\"");
		status=atoi(buf);
	}

	get_json_internet_status_cb(pkt, status);
	return 0;
}

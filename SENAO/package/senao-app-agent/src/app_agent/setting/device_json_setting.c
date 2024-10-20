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
;    File    : json_wan_setting.c
;    Abstract:
;
;       Modification History:
;       By              Date        Ver.    Modification Description
;       --------------- --------    -----   -------------------------------------
;                       2015/12/18          First commit
;****************************************************************************/

/*--------------------------------------------------------------------------*/
/*                           INCLUDE HEADER FILES                           */
/*--------------------------------------------------------------------------*/
#include "device_json_setting.h"
#include "json_setting.h"

/*--------------------------------------------------------------------------*/
/*                                PARAMETER                                 */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*                                DEFINITION                                */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*                            FUNCTION PROTOTYPE                            */
/*--------------------------------------------------------------------------*/
/*****************************************************************
* NAME:    get_device_device_settings_json_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
bool get_device_device_settings_json_cb(struct json_object *jobj, DEVICE_SETTING_T *setting)
{
    char fversion[15+1];

    if(NULL == jobj)
        return FALSE;

    json_object_object_add(jobj, "VendorName", json_object_new_string(setting->vendor_name));
    json_object_object_add(jobj, "ModelName", json_object_new_string(setting->model_name));
    json_object_object_add(jobj, "ModelDescription", json_object_new_string(setting->model_description));
    json_object_object_add(jobj, "ProductSN", json_object_new_string(setting->product_sn));
    json_object_object_add(jobj, "HardwareVersion", json_object_new_string(setting->hardware_version));
    memset(fversion,0x0,sizeof(fversion));
    sysutil_get_firmware_version_info(fversion, sizeof(fversion));
    json_object_object_add(jobj, "FirmwareVersion", json_object_new_string(fversion));
    json_object_object_add(jobj, "WanMacAddress", json_object_new_string(setting->wan_mac));
    json_object_object_add(jobj, "LanMacAddress", json_object_new_string(setting->lan_mac));
    json_object_object_add(jobj, "WlanMacAddress", json_object_new_string(setting->wlan_mac));

    return TRUE;
}

/*****************************************************************
* NAME:    parse_device_led_action_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
bool parse_device_led_action_cb(char *query_str, int *type, int *action)
{
    bool result;
    struct json_object *jobj, *jobj_type, *jobj_action;

    result = FALSE;

    if(0 != strlen(query_str))
    {
        if((jobj = json_tokener_parse(query_str)))
        {
            result = senao_json_object_get_integer(jobj, "LEDType", type);
            result &= senao_json_object_get_integer(jobj, "Action", action);

            json_object_put(jobj);
        }
    }

    return result;
}

#if SUPPORT_IPERF_THROUGHPUT_TEST
/*****************************************************************
 * NAME:    get_throughput_test_result_json_cb
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
int get_throughput_test_result_json_cb(HTTPS_CB *pkt, char *upload, char *download, char *result)
{
    struct json_object *jobj;
    char api_result[64];

    if(NULL == pkt)
    {
        return FALSE;
    }

    /* Construct the packet content in json format. */
    jobj = json_object_new_object();

    memset(api_result, 0x00, sizeof(api_result));
    sprintf(api_result, "%sResult", pkt->json_action);
    json_object_object_add(jobj, api_result, json_object_new_string((strlen(upload) && strlen(download))? OK_STR : ERROR_STR));
    json_object_object_add(jobj, "UploadThroughput", json_object_new_string(upload));
    json_object_object_add(jobj, "DownloadThroughput", json_object_new_string(download));

    /* Store packet content into buffer and send it out */
    http_store_data_to_buffer(HTTP_RESPONSE_CODE_200_TEMPLATE);
    http_store_data_to_buffer(HTTP_JSON_RESPONSE_HEADER_TEMPLATE);
    http_store_data_to_buffer("%s\n", json_object_to_json_string(jobj));

    http_send_stored_data(pkt->fd);

    /* Free obj */
    json_object_put(jobj);

    return TRUE;
}
#endif

#if HAS_SPEEDTEST_THROUGHPUT_TEST
/*****************************************************************
 * NAME:    get_throughput_test_result_json_cb
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
int get_speedtest_test_result_json_cb(HTTPS_CB *pkt, char *result, SPEEDTEST_RESULT_T *test_result)
{
    struct json_object *jobj;
    char api_result[64];

    if(NULL == pkt)
    {
        return FALSE;
    }

    /* Construct the packet content in json format. */
    jobj = json_object_new_object();

    memset(api_result, 0x00, sizeof(api_result));
    sprintf(api_result, "%sResult", pkt->json_action);
    json_object_object_add(jobj, api_result, json_object_new_string(result));
    json_object_object_add(jobj, "UploadThroughput", json_object_new_string(test_result->upload));
    json_object_object_add(jobj, "DownloadThroughput", json_object_new_string(test_result->download));
    json_object_object_add(jobj, "UploadUnit", json_object_new_string(test_result->upload_unit));
    json_object_object_add(jobj, "DownloadUnit", json_object_new_string(test_result->download_unit));
    json_object_object_add(jobj, "Date", json_object_new_string(test_result->date));
    json_object_object_add(jobj, "Country", json_object_new_string(test_result->country));
    json_object_object_add(jobj, "Sponsor", json_object_new_string(test_result->sponsor));
    json_object_object_add(jobj, "isServerSelectable", json_object_new_string("True"));

    /* Store packet content into buffer and send it out */
    http_store_data_to_buffer(HTTP_RESPONSE_CODE_200_TEMPLATE);
    http_store_data_to_buffer(HTTP_JSON_RESPONSE_HEADER_TEMPLATE);
    http_store_data_to_buffer("%s\n", json_object_to_json_string(jobj));

    http_send_stored_data(pkt->fd);

    /* Free obj */
    json_object_put(jobj);

    return TRUE;
}

/*****************************************************************
 * NAME:    get_speedtest_best_server_result_json_cb
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
int get_speedtest_best_server_result_json_cb(HTTPS_CB *pkt, char *result, char *best_server_result)
{
    struct json_object *jobj;
    char api_result[64];

    if(NULL == pkt)
    {
        return FALSE;
    }

    /* Construct the packet content in json format. */

    if (strlen(best_server_result)>1)
    {
        jobj = json_tokener_parse(best_server_result);
    }
    else
    {
        jobj = json_object_new_object();
    }

    memset(api_result, 0x00, sizeof(api_result));
    sprintf(api_result, "%sResult", pkt->json_action);
    json_object_object_add(jobj, api_result, json_object_new_string(result));

    /* Store packet content into buffer and send it out */
    http_store_data_to_buffer(HTTP_RESPONSE_CODE_200_TEMPLATE);
    http_store_data_to_buffer(HTTP_JSON_RESPONSE_HEADER_TEMPLATE);
    http_store_data_to_buffer("%s\n", json_object_to_json_string(jobj));

    http_send_stored_data(pkt->fd);

    /* Free obj */
    json_object_put(jobj);

    return TRUE;
}
#endif

/*****************************************************************
 * NAME:    get_specific_test_result_json_cb
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
int get_specific_test_result_json_cb(HTTPS_CB *pkt, char *result_file_path, char *result)
{
    struct json_object *jobj;
    struct json_object *jarr;
    char buf[256];
    char api_result[64];
    char *ptr;
    FILE *fp;

    if(NULL == pkt)
    {
        return FALSE;
    }

    fp = NULL;

    /* Construct the packet content in json format. */
    jobj = json_object_new_object();

    if(NULL != result_file_path)
    {
        if(NULL != (fp = fopen(result_file_path, "r")))
        {
            jarr = json_object_new_array();

            while(NULL != fgets(buf, sizeof(buf), fp))
            {
                /* Remove the character '\n' */
                if(NULL != (ptr = strrchr(buf, '\n')))
                {
                    *ptr = '\0';
                }

                json_object_array_add(jarr, json_object_new_string(buf));
            }

            fclose(fp);
            json_object_object_add(jobj, "TestResult", jarr);
        }
    }
    else
    {
        result = ERROR_STR;
    }

    memset(api_result, 0x00, sizeof(api_result));
    sprintf(api_result, "%sResult", pkt->json_action);
    json_object_object_add(jobj, api_result, json_object_new_string(result));

    /* Store packet content into buffer and send it out */
    http_store_data_to_buffer(HTTP_RESPONSE_CODE_200_TEMPLATE);
    http_store_data_to_buffer(HTTP_JSON_RESPONSE_HEADER_TEMPLATE);
    http_store_data_to_buffer("%s\n", json_object_to_json_string(jobj));

    http_send_stored_data(pkt->fd);

    /* Free obj */
    json_object_put(jobj);

    return TRUE;
}
/*****************************************************************
 * NAME:    get_json_system_information_with_ip_cb
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
int get_json_system_information_with_ip_cb(HTTPS_CB *pkt, char *result)
{
    int i;
    struct json_object *jobj;
    //int wifi0_guest=0;
    //int wifi1_guest=0;
    char api_result[128]={0};
    char buf[128]={0};
    char upTime[64]={0};
    char autoFWCheck[64]={0};
    char fw_ver1[8]={0};
    char fw_ver2[8]={0};
    int days=0, hours=0, minutes=0, seconds=0;
    int auto_upgrade=0;

    if(NULL == pkt)
    {
        return -1;
    }

    jobj = json_object_new_object();

    memset(api_result, 0x00, sizeof(api_result));
    sprintf(api_result, "%sResult", pkt->json_action);

    json_object_object_add(jobj, api_result, json_object_new_string(result));

    sysutil_interact(buf, sizeof(buf), "curl --connect-timeout 2 -s ipecho.net/plain");
    json_object_object_add(jobj, "ExternalIP", json_object_new_string(buf));

    sysutil_interact(buf, sizeof(buf), "ifconfig br-lan | awk '/inet addr/{print substr($2,6)}' | tr -d '\n'");
    json_object_object_add(jobj, "MainIP", json_object_new_string(buf));

    sysutil_interact(buf, sizeof(buf), "cat /etc/version | grep Firmware | awk '{print $4}'");
    sscanf(buf,"%[^.].%[^.].%*s\n",fw_ver1,fw_ver2);
    sprintf(buf,"%s.%s", fw_ver1, fw_ver2);
    json_object_object_add(jobj, "FwVersion", json_object_new_string(buf));

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

    json_object_object_add(jobj, "UpTime", json_object_new_string(upTime));
    api_get_string_option("sysProductInfo.model.modelName", buf, sizeof(buf));
    json_object_object_add(jobj, "ProductCode", json_object_new_string(buf));

    if (api_get_integer_option("autofw.config.FW_action", &auto_upgrade))
    {
        auto_upgrade = 0;
    }
    else
    {
        if (auto_upgrade == 0)
        {
            auto_upgrade=1;
        }
        else
        {
            auto_upgrade=0;
        }
    }

    json_object_object_add(jobj, "AutoFWUpgrade", json_object_new_int(auto_upgrade));

    basic_json_response(pkt, (char *)json_object_to_json_string(jobj));

    json_object_put(jobj);

    return 0;
}

int get_json_mesh_connected_history_cb(HTTPS_CB *pkt, char *result)
{
    struct json_object *jobj;
    struct json_object *jarr;
    struct json_object *jarr_obj;
    char api_result[64];
    char buf[64]={0};
    int flags=0;
    int i=0,meshHistoryCounter=0;

    if(NULL == pkt)
    {
        return -1;
    }

    jobj = json_object_new_object();

    memset(api_result, 0x00, sizeof(api_result));
    sprintf(api_result, "%sResult", pkt->json_action);
    json_object_object_add(jobj, api_result, json_object_new_string(result));

    jarr = json_object_new_array();

	sysutil_interact(buf, sizeof(buf), "uci show mesh | grep rooms | grep MacAddress | wc -l");

	meshHistoryCounter = atoi(buf);

	for(i=0;i<meshHistoryCounter;i++)
	{
		jarr_obj = json_object_new_object();

		api_get_string_option2(buf, sizeof(buf), "mesh.@rooms[%d].MacAddress", i);
		json_object_object_add(jarr_obj, "Mac", json_object_new_string((strlen(buf) == 0)?"":buf));

		api_get_string_option2(buf, sizeof(buf), "mesh.@rooms[%d].LocationName", i);
		json_object_object_add(jarr_obj, "Location", json_object_new_string((strlen(buf) == 0)?"":buf));

		api_get_string_option2(buf, sizeof(buf), "mesh.@rooms[%d].DeviceType", i);
		json_object_object_add(jarr_obj, "DeviceType", json_object_new_int((strlen(buf) == 0)?0:atoi(buf)));

		if(api_get_string_option2(buf, sizeof(buf), "mesh.@rooms[%d].ModelName", i))
		{
			//if no model name, default return emr3000.
			strcpy(buf, "EMR3000");
		}

		json_object_object_add(jarr_obj, "ModelName", json_object_new_string(buf));

		json_object_array_add(jarr, jarr_obj);
	}

	json_object_object_add(jobj, "HasBeenConnect", jarr);

	basic_json_response(pkt, (char *)json_object_to_json_string(jobj));

	json_object_put(jobj);

    return 0;
}

#if FOR_SC
/*****************************************************************
 * NAME:    execute_sc_firmware_upgrade_json_cb
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
int execute_sc_firmware_upgrade_json_cb(HTTPS_CB *pkt, bool boolean, int percentage, char *file_size, char *result)
{
    struct json_object *jobj;
    char api_result[64];

    if(NULL == pkt)
    {
        return FALSE;
    }

    /* Construct the packet content in json format. */
    jobj = json_object_new_object();

    memset(api_result, 0x00, sizeof(api_result));
    sprintf(api_result, "%sResult", pkt->json_action);
    json_object_object_add(jobj, api_result, json_object_new_string(result));
    json_object_object_add(jobj, "NewFirmware", json_object_new_boolean(boolean));
    json_object_object_add(jobj, "DownloadStatus", json_object_new_int(percentage));
    json_object_object_add(jobj, "FileSize", json_object_new_string(file_size));

    /* Store packet content into buffer and send it out */
    http_store_data_to_buffer(HTTP_RESPONSE_CODE_200_TEMPLATE);
    http_store_data_to_buffer(HTTP_JSON_RESPONSE_HEADER_TEMPLATE);
    http_store_data_to_buffer("%s\n", json_object_to_json_string(jobj));

    http_send_stored_data(pkt->fd);

    /* Free obj */
    json_object_put(jobj);

    return TRUE;
}
#endif
/*****************************************************************
 * NAME:    execute_firmware_upgrade_json_cb
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
int execute_firmware_upgrade_json_cb(HTTPS_CB *pkt, bool boolean, int percentage, char *file_size, char *result, char *release_date, char *change_log, char *version)
{
    struct json_object *jobj;
    char api_result[64];

    if(NULL == pkt)
    {
        return FALSE;
    }

    /* Construct the packet content in json format. */
    jobj = json_object_new_object();

    memset(api_result, 0x00, sizeof(api_result));
    sprintf(api_result, "%sResult", pkt->json_action);
    json_object_object_add(jobj, api_result, json_object_new_string(result));
    json_object_object_add(jobj, "NewFirmware", json_object_new_boolean(boolean));
    json_object_object_add(jobj, "DownloadStatus", json_object_new_int(percentage));
    json_object_object_add(jobj, "FileSize", json_object_new_string(file_size));

    json_object_object_add(jobj, "release_date", json_object_new_string(release_date));
    json_object_object_add(jobj, "change_log", json_object_new_string(change_log));
    json_object_object_add(jobj, "version", json_object_new_string(version));

    /* Store packet content into buffer and send it out */
    http_store_data_to_buffer(HTTP_RESPONSE_CODE_200_TEMPLATE);
    http_store_data_to_buffer(HTTP_JSON_RESPONSE_HEADER_TEMPLATE);
    http_store_data_to_buffer("%s\n", json_object_to_json_string(jobj));

    http_send_stored_data(pkt->fd);

    /* Free obj */
    json_object_put(jobj);

    return TRUE;
}

/*****************************************************************
 * NAME:    get_new_firmware_result_json_cb
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
int get_new_firmware_result_json_cb(HTTPS_CB *pkt, char *result, int action)
{
    struct json_object *jobj, *jarr_obj, *jarr;
    char buf[256];
    char mac[64], ip[64], checkResult[64];
    char api_result[64];
    char *ptr;
    FILE *fp;
    int opmode;

    if(NULL == pkt)
    {
        return FALSE;
    }

    fp = NULL;

    opmode = 0;
    memset(api_result, 0x00, sizeof(api_result));
    sprintf(api_result, "%sResult", pkt->json_action);
    
    /* Construct the packet content in json format. */
    jobj = json_object_new_object();

    if(NULL != UPGRADE_LIST)
    {
        if(NULL != (fp = fopen(UPGRADE_LIST, "r")))
        {
            jarr = json_object_new_array();

            while(NULL != fgets(buf, sizeof(buf), fp))
            {
                jarr_obj = json_object_new_object();
                /* Remove the character '\n' */
                if(NULL != (ptr = strrchr(buf, '\n')))
                {
                    *ptr = '\0';
                }

                sscanf(buf,"%s %s %s %d\n",checkResult, mac, ip, &opmode);
                json_object_object_add(jarr_obj, "CheckResult", json_object_new_string(checkResult));
                json_object_object_add(jarr_obj, "Mac", json_object_new_string(mac));
                json_object_object_add(jarr_obj, "IP", json_object_new_string(ip));
                json_object_object_add(jarr_obj, "OpMode", json_object_new_int(opmode));
                json_object_array_add(jarr, jarr_obj);
            }
            fclose(fp);
            json_object_object_add(jobj, "DeviceList", jarr);
        }
        
        
    }
    else
    {
        result = ERROR_STR;
    }

    json_object_object_add(jobj, api_result, json_object_new_string(result));
    json_object_object_add(jobj, "ResponseAction", json_object_new_int(action));
    
    /* Store packet content into buffer and send it out */
    http_store_data_to_buffer(HTTP_RESPONSE_CODE_200_TEMPLATE);
    http_store_data_to_buffer(HTTP_JSON_RESPONSE_HEADER_TEMPLATE);
    http_store_data_to_buffer("%s\n", json_object_to_json_string(jobj));

    http_send_stored_data(pkt->fd);

    /* Free obj */
    json_object_put(jobj);

    return TRUE;
}

/*****************************************************************
 * NAME:    get_download_firmware_result_json_cb
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
int get_download_firmware_result_json_cb(HTTPS_CB *pkt, char *result, int action)
{
    struct json_object *jobj, *jarr_obj, *jarr;
    char api_result[64];
    char buf[256];
    char download[64], mac[64], ip[64];
    char *ptr;
    FILE *fp;
    int opmode;

    if(NULL == pkt)
    {
        return FALSE;
    }

    opmode = 0;
    memset(api_result, 0x00, sizeof(api_result));
    sprintf(api_result, "%sResult", pkt->json_action);
    /* Construct the packet content in json format. */
    jobj = json_object_new_object();

    if (action == DOWNLOAD_FIRMWARE_STATUS)
    {

        if (NULL != FIRMWARE_DOWNLOAD_STATUS)
        {
            system("sleep 3");
            system("cat "FIRMWARE_DOWNLOAD_STATUS);
            if(NULL != (fp = fopen(FIRMWARE_DOWNLOAD_STATUS, "r")))
            {
                jarr = json_object_new_array();

                while(NULL != fgets(buf, sizeof(buf), fp))
                {
                    jarr_obj = json_object_new_object();
                    /* Remove the character '\n' */
                    if(NULL != (ptr = strrchr(buf, '\n')))
                    {
                        *ptr = '\0';
                    }

                    sscanf(buf,"%s %s %s %d\n",download, mac, ip, &opmode);
                    json_object_object_add(jarr_obj, "DownloadResult", json_object_new_string(download));
                    json_object_object_add(jarr_obj, "Mac", json_object_new_string(mac));
                    json_object_object_add(jarr_obj, "IP", json_object_new_string(ip));

                    json_object_array_add(jarr, jarr_obj);
                }

                fclose(fp);
                json_object_object_add(jobj, "DeviceList", jarr);
            }
            
        }
        else
        {
            result = ERROR_STR;
        }
    }

    json_object_object_add(jobj, api_result, json_object_new_string(result));
    json_object_object_add(jobj, "ResponseAction", json_object_new_int(action));
    /* Store packet content into buffer and send it out */
    http_store_data_to_buffer(HTTP_RESPONSE_CODE_200_TEMPLATE);
    http_store_data_to_buffer(HTTP_JSON_RESPONSE_HEADER_TEMPLATE);
    http_store_data_to_buffer("%s\n", json_object_to_json_string(jobj));

    http_send_stored_data(pkt->fd);

    /* Free obj */
    json_object_put(jobj);

    return TRUE;
}

/*****************************************************************
 * NAME:    get_json_base_status_cb
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
int get_json_base_status_cb(HTTPS_CB *pkt, char *result, int AutoFwEnable, char *ExternalIP, char *FwVersion, char *MainIP, char *ProductCode, char *upTime)
{
    struct json_object *jobj;
    char api_result[64];

    if(NULL == pkt)
    {
        return FALSE;
    }

    /* Construct the packet content in json format. */
    jobj = json_object_new_object();

    memset(api_result, 0x00, sizeof(api_result));
    sprintf(api_result, "%sResult", pkt->json_action);
    json_object_object_add(jobj, api_result, json_object_new_string(result));
    json_object_object_add(jobj, "AutoFWUpgrade", json_object_new_int(AutoFwEnable));
    json_object_object_add(jobj, "ExternalIP", json_object_new_string(ExternalIP));
    json_object_object_add(jobj, "FwVersion", json_object_new_string(FwVersion));
    json_object_object_add(jobj, "MainIP", json_object_new_string(MainIP));
    json_object_object_add(jobj, "ProductCode", json_object_new_string(ProductCode));
    json_object_object_add(jobj, "UpTime", json_object_new_string(upTime));

    /* Store packet content into buffer and send it out */
    http_store_data_to_buffer(HTTP_RESPONSE_CODE_200_TEMPLATE);
    http_store_data_to_buffer(HTTP_JSON_RESPONSE_HEADER_TEMPLATE);
    http_store_data_to_buffer("%s\n", json_object_to_json_string(jobj));

    http_send_stored_data(pkt->fd);

    /* Free obj */
    json_object_put(jobj);

    return TRUE;
}

int get_json_mesh_node_simplify_info_cb(HTTPS_CB *pkt, char *result)
{
    struct json_object *jobj;
    struct json_object *jobj2;
    struct json_object *jobj3;
    struct json_object *jarr;
    struct json_object *jarr1;
    struct json_object *jarr_obj;
    struct json_object *jarr_obj1;
    struct json_object *medi_array_obj;
    struct json_object *medi_array_obj2;
    struct json_object *medi_array_obj_name;
    struct json_object *medi_array_obj_name_result;
    struct json_object *jobj_guestClient;
    struct json_object *DeviceType;
    struct json_object *DeviceName;
    struct json_object *LocationName;
    struct json_object *MeshRole;
    struct json_object *MeshController;
    struct json_object *TQ;
    struct json_object *LANIPAddress;
    struct json_object *LANMacAddress;
    struct json_object *WANMacAddress;
    struct json_object *MacAddress;
    struct json_object *Neighbors;
    struct json_object *GuestClientList;
    struct json_object *GuestClient24GList;
    struct json_object *GuestClient5GList;
    struct json_object *WiFiStatus;
    struct json_object *DevicesConnectedNumber;
    struct json_object *MAC;
    struct json_object *RSSI;
    struct json_object *LedStatus;
    struct json_object *FullFwVersion;
    struct json_object *MeshResult;
    struct device_info all_device[128]={0};
    struct json_object *NextHopRssi;
    struct json_object *NextHopMac;
    struct json_object *TrueWANMAC;
    int MaxRSSI=0;
    int neighbors_length=0;
    int GuestClient24GList_length=0;
    int GuestClient5GList_length=0;
    char api_result[64];
    char buf[20480]={0};
    char master_mesh_mac[64];
    int i=0,j=0,flags=0;
    int array_length;
    int wifi0=0;
    int wifi1=0;
    int array_length_ResultOK=0;
    MESH_GUEST_CLIENT_INFO_T gClientInfo;
    int DevicesConnectedNumber_g=0;

    if(NULL == pkt)
    {
        return -1;
    }

    jobj = json_object_new_object();

    memset(api_result, 0x00, sizeof(api_result));
    memset(all_device, 0x00, sizeof(all_device));
    sprintf(api_result, "%sResult", pkt->json_action);
    json_object_object_add(jobj, api_result, json_object_new_string(result));

    if(sysutil_check_file_existed("/tmp/mesh_global_node_info"))
    {
		sysutil_interact(buf, sizeof(buf), "cat /tmp/mesh_global_node_info");

		jobj2 = json_tokener_parse(buf);

		array_length = json_object_array_length(jobj2);

		for (i = 0; i < array_length; i++)
		{
			medi_array_obj = json_object_array_get_idx(jobj2, i);
			MeshResult = json_object_object_get(medi_array_obj, "GetMeshNodeInfoResult");

			if (strcmp(json_object_get_string(MeshResult), "OK") == 0)
			{
				medi_array_obj_name = json_object_object_get(medi_array_obj, "MeshDevice");
				DeviceType = json_object_object_get(medi_array_obj_name, "DeviceType");
				DeviceName = json_object_object_get(medi_array_obj_name, "PN");
				if(json_object_get_string(DeviceName) == NULL)
				{
					DeviceName = json_object_object_get(medi_array_obj_name, "DeviceName");
				}
				LocationName = json_object_object_get(medi_array_obj_name, "LocationName");
				MeshRole = json_object_object_get(medi_array_obj_name, "MeshRole");
				LANIPAddress = json_object_object_get(medi_array_obj_name, "LANIPAddress");
				LANMacAddress = json_object_object_get(medi_array_obj_name, "LANMacAddress");
				MacAddress = json_object_object_get(medi_array_obj_name, "MacAddress");
				WiFiStatus = json_object_object_get(medi_array_obj_name, "WiFiStatus");
				DevicesConnectedNumber = json_object_object_get(medi_array_obj_name, "DevicesConnectedNumber");
				LedStatus = json_object_object_get(medi_array_obj_name, "LedStatus");
				FullFwVersion = json_object_object_get(medi_array_obj_name, "FullFwVersion");
                WANMacAddress = json_object_object_get(medi_array_obj_name, "WANMacAddress");
                MeshController = json_object_object_get(medi_array_obj_name, "MeshController");
                TQ = json_object_object_get(medi_array_obj_name, "TQ");
                NextHopRssi = json_object_object_get(medi_array_obj_name, "NextHopRssi");
                NextHopMac = json_object_object_get(medi_array_obj_name, "NextHopMac");
                TrueWANMAC = json_object_object_get(medi_array_obj_name, "TrueWANMAC");
				strcpy(all_device[array_length_ResultOK].deviceType, json_object_get_string(DeviceType));
				strcpy(all_device[array_length_ResultOK].deviceName, json_object_get_string(DeviceName));
				strcpy(all_device[array_length_ResultOK].locationName, json_object_get_string(LocationName));
				strcpy(all_device[array_length_ResultOK].meshRole, json_object_get_string(MeshRole));
				strcpy(all_device[array_length_ResultOK].lANIPAddress, json_object_get_string(LANIPAddress));
				strcpy(all_device[array_length_ResultOK].LANMacAddress, json_object_get_string(LANMacAddress));
				strcpy(all_device[array_length_ResultOK].mac, json_object_get_string(MacAddress));
				strcpy(all_device[array_length_ResultOK].FullFwVersion, json_object_get_string(FullFwVersion));
                strcpy(all_device[array_length_ResultOK].WANMacAddress, (json_object_get_string(WANMacAddress) != NULL) ? json_object_get_string(WANMacAddress) : "");
                strcpy(all_device[array_length_ResultOK].TrueWANMAC, (json_object_get_string(TrueWANMAC) != NULL) ? json_object_get_string(TrueWANMAC) : "");
				strcpy(all_device[array_length_ResultOK].meshController, (json_object_get_string(MeshController) != NULL) ? json_object_get_string(MeshController) : "slave");
                all_device[array_length_ResultOK].wifi_status = json_object_get_int(WiFiStatus);
				all_device[array_length_ResultOK].dev_connect_number = json_object_get_int(DevicesConnectedNumber);
				all_device[array_length_ResultOK].led_status = json_object_get_int(LedStatus);
                all_device[array_length_ResultOK].tq = (TQ != NULL) ? json_object_get_int(TQ) : 0;

				if(0 == strcmp("server", all_device[array_length_ResultOK].meshRole) &&
				   0 == strcmp("slave", all_device[array_length_ResultOK].meshController))
				{
					sysutil_interact(all_device[array_length_ResultOK].NextHopMac,
							sizeof(all_device[array_length_ResultOK].NextHopMac),
							"/usr/shc/getMeshNextHop %s %s %s",
							all_device[array_length_ResultOK].LANMacAddress,
							all_device[array_length_ResultOK].mac,
							all_device[array_length_ResultOK].TrueWANMAC);
					all_device[array_length_ResultOK].NextHopRssi = 100;
				}
				else
				{
					if(0 == strcmp("server", all_device[array_length_ResultOK].meshRole) &&
					   0 == strcmp("master", all_device[array_length_ResultOK].meshController))
					{
						memset(master_mesh_mac, 0x00, sizeof(master_mesh_mac));
						sprintf(master_mesh_mac, "%s", all_device[array_length_ResultOK].mac);
					}

					strcpy(all_device[array_length_ResultOK].NextHopMac, (json_object_get_string(NextHopMac) != NULL) ? json_object_get_string(NextHopMac) : "");
					all_device[array_length_ResultOK].NextHopRssi = (NextHopRssi != NULL) ? json_object_get_int(NextHopRssi) : 0;
				}

				senao_json_object_get_string(medi_array_obj_name, "UID", all_device[array_length_ResultOK].uid);

				Neighbors = json_object_object_get(medi_array_obj, "Neighbors");
				neighbors_length = json_object_array_length(Neighbors);

				MaxRSSI = 0;
				jobj3=json_tokener_parse(json_object_get_string(Neighbors));
				for (j=0;j<neighbors_length;j++)
				{
				  medi_array_obj2 = json_object_array_get_idx(jobj3, j);
				  RSSI = json_object_object_get(medi_array_obj2, "RSSI");
				  MAC = json_object_object_get(medi_array_obj2, "MAC");
				  if(MaxRSSI < json_object_get_int(RSSI))
				  {
					  MaxRSSI = json_object_get_int(RSSI);
				  }
				}

				all_device[array_length_ResultOK].rssi = MaxRSSI;

                GuestClientList = json_object_object_get(medi_array_obj, "GuestClientList");
                if (GuestClientList != NULL)
                {
                    GuestClient24GList = json_object_object_get(GuestClientList, "twoG");
                    GuestClient24GList_length = json_object_array_length(GuestClient24GList);

                    for (j=0;j<GuestClient24GList_length;j++)
                    {
                        medi_array_obj2 = json_object_array_get_idx(GuestClient24GList, j);

                        senao_json_object_get_string(medi_array_obj2, "MAC", all_device[array_length_ResultOK].gClientInfo.guest24_mac[j]);
                        senao_json_object_get_string(medi_array_obj2, "IP", all_device[array_length_ResultOK].gClientInfo.guest24_ip[j]);
                        senao_json_object_get_string(medi_array_obj2, "HostName", all_device[array_length_ResultOK].gClientInfo.guest24_hostname[j]);
                    }

                    GuestClient5GList = json_object_object_get(GuestClientList, "fiveG");
                    GuestClient5GList_length = json_object_array_length(GuestClient5GList);

                    for (j=0;j<GuestClient5GList_length;j++)
                    {
                        medi_array_obj2 = json_object_array_get_idx(GuestClient5GList, j);

                        senao_json_object_get_string(medi_array_obj2, "MAC", all_device[array_length_ResultOK].gClientInfo.guest5_mac[j]);
                        senao_json_object_get_string(medi_array_obj2, "IP", all_device[array_length_ResultOK].gClientInfo.guest5_ip[j]);
                        senao_json_object_get_string(medi_array_obj2, "HostName", all_device[array_length_ResultOK].gClientInfo.guest5_hostname[j]);
                    }
                }

				array_length_ResultOK++;
			}
		}

		// Search for the MASTER MESH MAC.
		for (i = 0; i < array_length; i++)
		{
			if(0 == strcmp(all_device[i].NextHopMac, master_mesh_mac))
			{
				flags = 1;
				break;
			}
		}

		// If there is no MASTER MESH MAC,
		// and then search the slaves whose next hop are assigned to each other.
		// Modify one of the salves's next hop to the MASTER MESH MAC.
		if(0 == flags)
		{
			for (i = 0; i < array_length - 1; i++)
			{
				for (j = 0; j < array_length; j++)
				{
					if(0 == strcasecmp(all_device[i].NextHopMac, all_device[j].mac))
					{
						if(0 == strcasecmp(all_device[i].mac, all_device[j].NextHopMac))
						{
							memset(all_device[i].NextHopMac, 0x00, sizeof(all_device[i].NextHopMac));
							sprintf(all_device[i].NextHopMac, "%s", master_mesh_mac);
							all_device[i].NextHopRssi = 99;
						}

						break;
					}
				}
			}
		}

		jarr = json_object_new_array();

		for (i=0;i<array_length_ResultOK;i++)
		{
		   jarr_obj = json_object_new_object();
		   json_object_object_add(jarr_obj, "DeviceType", json_object_new_string(all_device[i].deviceType));
		   json_object_object_add(jarr_obj, "DeviceName", json_object_new_string(all_device[i].deviceName));
		   json_object_object_add(jarr_obj, "LocationName", json_object_new_string(all_device[i].locationName));
		   json_object_object_add(jarr_obj, "MeshRole", json_object_new_string(all_device[i].meshRole));
		   json_object_object_add(jarr_obj, "LANIPAddress", json_object_new_string(all_device[i].lANIPAddress));
		   json_object_object_add(jarr_obj, "LANMacAddress", json_object_new_string(all_device[i].LANMacAddress));
		   json_object_object_add(jarr_obj, "MacAddress", json_object_new_string(all_device[i].mac));
		   json_object_object_add(jarr_obj, "UID", json_object_new_string(all_device[i].uid));
           json_object_object_add(jarr_obj, "MeshController", json_object_new_string(all_device[i].meshController));

		   //if(strcmp(all_device[i].meshRole, "server") == 0)
           if(strcmp(all_device[i].deviceType, "1") == 0 || strcmp(all_device[i].deviceType, "2") == 0)
		   {
				json_object_object_add(jarr_obj, "MaxRSSI", json_object_new_int(100));
		   }
		   else
		   {
				json_object_object_add(jarr_obj, "MaxRSSI", json_object_new_int(all_device[i].rssi));
		   }
		   json_object_object_add(jarr_obj, "WiFiStatus", json_object_new_int(all_device[i].wifi_status));
		   //json_object_object_add(jarr_obj, "DevicesConnectedNumber", json_object_new_int(all_device[i].dev_connect_number));
		   json_object_object_add(jarr_obj, "LedStatus", json_object_new_int(all_device[i].led_status));
           json_object_object_add(jarr_obj, "TQ", json_object_new_int(all_device[i].tq));
           json_object_object_add(jarr_obj, "NextHopRssi", json_object_new_int(all_device[i].NextHopRssi));
           json_object_object_add(jarr_obj, "NextHopMac", json_object_new_string(all_device[i].NextHopMac));
		   json_object_object_add(jarr_obj, "FullFwVersion", json_object_new_string(all_device[i].FullFwVersion));
           if(strlen(all_device[i].WANMacAddress))
           {
                json_object_object_add(jarr_obj, "WANMacAddress", json_object_new_string(all_device[i].WANMacAddress)); 
           }
           if(strlen(all_device[i].TrueWANMAC))
           {
               json_object_object_add(jarr_obj, "TrueWANMAC", json_object_new_string(all_device[i].TrueWANMAC));
           }
//////////////////////////////////////////////////////////
           jobj_guestClient = json_object_new_object();

           DevicesConnectedNumber_g=0;
           j = 0;
           jarr1 = json_object_new_array();

           while(0 != strlen(all_device[i].gClientInfo.guest24_mac[j]))
           {
               jarr_obj1 = json_object_new_object();

               json_object_object_add(jarr_obj1, "MAC", json_object_new_string(all_device[i].gClientInfo.guest24_mac[j]));
               json_object_object_add(jarr_obj1, "IP", json_object_new_string(all_device[i].gClientInfo.guest24_ip[j]));
               json_object_object_add(jarr_obj1, "HostName", json_object_new_string(all_device[i].gClientInfo.guest24_hostname[j]));

               json_object_array_add(jarr1, jarr_obj1);
               j++;
           }

           DevicesConnectedNumber_g=DevicesConnectedNumber_g+j;
           json_object_object_add(jobj_guestClient, "2.4G", jarr1);
           j = 0;
           jarr1 = json_object_new_array();

           while(0 != strlen(all_device[i].gClientInfo.guest5_mac[j]))
           {
               jarr_obj1 = json_object_new_object();

               json_object_object_add(jarr_obj1, "MAC", json_object_new_string(all_device[i].gClientInfo.guest5_mac[j]));
               json_object_object_add(jarr_obj1, "IP", json_object_new_string(all_device[i].gClientInfo.guest5_ip[j]));
               json_object_object_add(jarr_obj1, "HostName", json_object_new_string(all_device[i].gClientInfo.guest5_hostname[j]));

               json_object_array_add(jarr1, jarr_obj1);
               j++;
           }
           DevicesConnectedNumber_g=DevicesConnectedNumber_g+j;
           json_object_object_add(jobj_guestClient, "5G", jarr1);

           json_object_object_add(jarr_obj, "GuestClientList", jobj_guestClient);
///////////////////////////////////////
           all_device[i].dev_connect_number = all_device[i].dev_connect_number + DevicesConnectedNumber_g;
           json_object_object_add(jarr_obj, "DevicesConnectedNumber", json_object_new_int(all_device[i].dev_connect_number));

		   json_object_array_add(jarr, jarr_obj);
		}

		json_object_object_add(jobj, "MeshNodesList", jarr);
	}

    basic_json_response(pkt,  (char *)json_object_to_json_string(jobj));

    json_object_put(jobj);

    return 0;
}

/*****************************************************************
 * NAME:    get_json_SN_number_cb
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
int get_json_SN_number_cb(HTTPS_CB *pkt, char *result, char *SN_number, char *r_domain, char *VenderName, char *MeshDeviceLocation, int eth_status)
{
    struct json_object *jobj;
    char api_result[64];

    if(NULL == pkt)
    {
        return FALSE;
    }

    /* Construct the packet content in json format. */
    jobj = json_object_new_object();

    memset(api_result, 0x00, sizeof(api_result));
    sprintf(api_result, "%sResult", pkt->json_action);
    json_object_object_add(jobj, api_result, json_object_new_string(result));
    json_object_object_add(jobj, "VenderName", json_object_new_string(VenderName));
    json_object_object_add(jobj, "SerialNumber", json_object_new_string(SN_number));
    json_object_object_add(jobj, "MeshDeviceLocation", json_object_new_string(MeshDeviceLocation));
    json_object_object_add(jobj, "EthStatus", json_object_new_int(eth_status));
    json_object_object_add(jobj, "RegularDomain", json_object_new_string(r_domain));
    if(strcmp("EMR5000", VenderName)==0)
    {
        json_object_object_add(jobj, "DeviceType", json_object_new_int(2));
    }
    else if(strcmp("EMD1", VenderName)==0 || strcmp("EMD11", VenderName)==0 )
    {
        json_object_object_add(jobj, "DeviceType", json_object_new_int(3));
    }
    else if(strcmp("EWS1025", VenderName)==0)
    {
        json_object_object_add(jobj, "DeviceType", json_object_new_int(4));
    }
    else if(strcmp("EMR3500", VenderName)==0)
    {
        json_object_object_add(jobj, "DeviceType", json_object_new_int(5));
    }
    else if(strcmp("EMD2", VenderName)==0)
    {
        json_object_object_add(jobj, "DeviceType", json_object_new_int(6));
    }
    else //EMR3000
    {
        json_object_object_add(jobj, "DeviceType", json_object_new_int(1));
	}

    /* Store packet content into buffer and send it out */
    http_store_data_to_buffer(HTTP_RESPONSE_CODE_200_TEMPLATE);
    http_store_data_to_buffer(HTTP_JSON_RESPONSE_HEADER_TEMPLATE);
    http_store_data_to_buffer("%s\n", json_object_to_json_string(jobj));

    http_send_stored_data(pkt->fd);

    /* Free obj */
    json_object_put(jobj);

    return TRUE;
}

/*****************************************************************
 * NAME:    get_json_internet_status_cb
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
int get_json_internet_status_cb(HTTPS_CB *pkt, int status)
{
	struct json_object *jobj;
	char api_result[64];

	if(NULL == pkt)
	{
		return FALSE;
	}

	jobj = json_object_new_object();

	memset(api_result, 0x00, sizeof(api_result));
	sprintf(api_result, "%sResult", pkt->json_action);

	json_object_object_add(jobj, api_result, json_object_new_string(OK_STR));
	json_object_object_add(jobj, "InternetStatus", json_object_new_int(status));

	basic_json_response(pkt, (char *)json_object_to_json_string(jobj));
	json_object_put(jobj);

	return TRUE;
}

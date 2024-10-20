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
;    File    : ipcam_json_setting.c
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
#include "json_setting.h"
#include "ipcam_setting.h"
#include "ipcam_json_setting.h"

/*--------------------------------------------------------------------------*/
/*                                PARAMETER                                 */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*                                DEFINITION                                */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*                            FUNCTION PROTOTYPE                            */
/*--------------------------------------------------------------------------*/
#if ROUTER_SUPPORT_IPCAM
/*****************************************************************
* NAME:    get_device_lan_settings_json_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
bool get_ip_camera_list_json_cb(HTTPS_CB *pkt, IPCAM_DEVICE_INFO_T setting[], char *result_str)
{
    struct json_object *jobj;
    struct json_object *jarr;
    struct json_object *jarr_obj;
    char api_result[64];
    int i;

    if(NULL == pkt)
    {
        return FALSE;
    }

    /* Construct the packet content in json format. */
    jobj = json_object_new_object();

    memset(api_result, 0x00, sizeof(api_result));
    sprintf(api_result, "%sResult", pkt->json_action);
    json_object_object_add(jobj, api_result, json_object_new_string(result_str));

    jarr = json_object_new_array();

    for(i = 0; 0 != strlen(setting[i].ip); i++)
    {
        jarr_obj = json_object_new_object();

        json_object_object_add(jarr_obj, "DeviceName", json_object_new_string(setting[i].device_name));
        json_object_object_add(jarr_obj, "MacAddress", json_object_new_string(setting[i].mac));
        json_object_object_add(jarr_obj, "IpAddress", json_object_new_string(setting[i].ip));
        json_object_object_add(jarr_obj, "PublicPort", json_object_new_int(setting[i].httpd_port));
        json_object_object_add(jarr_obj, "CamAgentHttpPulicPort", json_object_new_int(setting[i].app_http_port));
        json_object_object_add(jarr_obj, "CamAgentPulicPort", json_object_new_int(setting[i].app_https_port));
        json_object_object_add(jarr_obj, "DeviceUID", json_object_new_string(setting[i].uid));

        json_object_array_add(jarr, jarr_obj);
    }

    json_object_object_add(jobj, "CameraList", jarr);

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
 * NAME:    get_ipcam_samba_folder_list_json_cb
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
int get_ipcam_samba_folder_list_json_cb(HTTPS_CB *pkt, char folder_list[][64], char *result_str)
{
    struct json_object *jobj;
    struct json_object *jarr_samba_folder_list;
    int i;

    if(NULL == pkt)
    {
        return -1;
    }

    i = 0;
    jobj = NULL;
    jarr_samba_folder_list = NULL;

    /* Construct the packet content in json format. */
    jobj = json_object_new_object();

    json_object_object_add(jobj, "GetIPCameSambaFolderListResult", json_object_new_string(result_str));

    jarr_samba_folder_list = json_object_new_array();

    while(0 != strlen(folder_list[i]))
    {
        json_object_array_add(jarr_samba_folder_list, json_object_new_string(folder_list[i]));
        i++;
    }

    json_object_object_add(jobj, "SambaFolderList", jarr_samba_folder_list);

    /* Store packet content into buffer and send it out */
    http_store_data_to_buffer(HTTP_RESPONSE_CODE_200_TEMPLATE);
    http_store_data_to_buffer(HTTP_JSON_RESPONSE_HEADER_TEMPLATE);
    http_store_data_to_buffer("%s\n", json_object_to_json_string(jobj));

    http_send_stored_data(pkt->fd);

    /* Free obj */
    json_object_put(jobj);

    return 0;
}

/*****************************************************************
 * NAME:    parse_json_get_play_back_info_cb
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author: Eason 2015/3/23
 * Modify:
 ******************************************************************/
int get_play_back_info_json_cb(HTTPS_CB *pkt, char *path, char *result)
{
    struct json_object *jobj, *jobj_video,*jobj_snapshot,*jarr_videolist,*jarr_snapshotlist;

    if(NULL == pkt)
    {
        return -1;
    }

    FILE *fp1;
    char buf[512]={0};
    char date[32]={0};
    char videoPath[128]={0};
    char snapshotPath[128]={0};
    char alarmListPath[256]={0};
    char scheduleListPath[256]={0};
    char alarmVideoFile[]="/.alarm_%s_list";
    char alarmSnapshotFile[]="/.alarm_%s_photo_list";
    char schedVideoFile[]="/.sched_%s_list";
    char getAlarmVideoFile[64]={0};
    char getAlarmSnapshotFile[64]={0};
    char getSchedVideoFile[64]={0};
    int  numOfsched=0, numOfAlarm=0;

    jobj = json_object_new_object();

    if(strcmp(result, ERROR_STR) == 0 || strlen(path) == 0)
    {
        json_object_object_add(jobj, "GetPlayBackInfoResult", json_object_new_string(result));
        goto end;
    }

    if(sysutil_check_file_existed(path))  //path: /mnt/sda1/IP_Camera/Folder_Name
    {
        json_object_object_add(jobj, "GetPlayBackInfoResult", json_object_new_string(result));

        jarr_videolist = json_object_new_array();
        jarr_snapshotlist = json_object_new_array();

        // /mnt/sda1/IP_Camera/Folder_Name/video
        sprintf(videoPath, "%s/%s", path, VIDEO_FOLDER_NAME);

        if(sysutil_check_file_existed(videoPath))
        {
            SYSTEM("scan_folder -s %s", videoPath);

            fp1 = fopen(FOLDER_LIST, "r");
            while(fgets(buf, sizeof(buf), fp1) != NULL)
            {
                numOfsched = 0;
                numOfAlarm = 0;

                sscanf(buf, "%[^,]", date); //date:20150305

                sprintf(getAlarmVideoFile, alarmVideoFile, date); //.alarm_20150305_list
                // /mnt/sda1/IP_Camera/Folder_Name/video/20150305/.alarm_20150305_list
                sprintf(alarmListPath, "%s/%s/%s", videoPath, date, getAlarmVideoFile);
                if(sysutil_check_file_existed(alarmListPath))
                {
                    //count /mnt/sda1/IP_Camera/Folder_Name/video/20150305/.alarm_20150305_list
                    sysutil_get_file_line_num(alarmListPath, &numOfAlarm);
                }

                sprintf(getSchedVideoFile, schedVideoFile, date); //.sched_20150305_list
                // /mnt/sda1/IP_Camera/Folder_Name/video/20150305/.sched_20150305_list
                sprintf(scheduleListPath, "%s/%s/%s", videoPath, date, getSchedVideoFile);
                if(sysutil_check_file_existed(scheduleListPath))
                {
                    //count /mnt/sda1/IP_Camera/Folder_Name/video/20150305/.sched_20150305_list
                    sysutil_get_file_line_num(scheduleListPath, &numOfsched);
                }

                if (numOfAlarm!=0 || numOfsched!=0)
                {
                    jobj_video = json_object_new_object();

                    json_object_object_add(jobj_video, "Date", json_object_new_string(date));
                    json_object_object_add(jobj_video, "sched_video", json_object_new_int(numOfsched));
                    json_object_object_add(jobj_video, "alarm_video", json_object_new_int(numOfAlarm));

                    json_object_array_add(jarr_videolist, jobj_video);
                }
            }
            fclose(fp1);
        }
        json_object_object_add(jobj, "Video", jarr_videolist);

        /*========================================================================*/

        // /mnt/sda1/IP_Camera/Folder_Name/snapshot
        sprintf(snapshotPath, "%s/%s", path, SNAPSHOT_FOLDER_NAME);

        if(sysutil_check_file_existed(snapshotPath))
        {
            SYSTEM("scan_folder -s %s", snapshotPath);

            fp1 = fopen(FOLDER_LIST, "r");
            while(fgets(buf, sizeof(buf), fp1) != NULL)
            {
                numOfsched = 0;
                numOfAlarm = 0;

                sscanf(buf, "%[^,]", date); //date:20150305

                sprintf(getAlarmSnapshotFile, alarmSnapshotFile, date); //.alarm_20150305_photo_list
                // /mnt/sda1/IP_Camera/Folder_Name/snapshot/20150305/.alarm_20150305_photo_list
                sprintf(alarmListPath, "%s/%s/%s", snapshotPath, date, getAlarmSnapshotFile);
                if(sysutil_check_file_existed(alarmListPath))
                {
                    //count /mnt/sda1/IP_Camera/Folder_Name/snapshot/20150305/.alarm_20150305_photo_list
                    sysutil_get_file_line_num(alarmListPath, &numOfAlarm);
                }

                if (numOfAlarm!=0 || numOfsched!=0)
                {
                    jobj_snapshot = json_object_new_object();

                    json_object_object_add(jobj_snapshot, "Date", json_object_new_string(date));
                    json_object_object_add(jobj_snapshot, "sched_snapshot", json_object_new_int(numOfsched));
                    json_object_object_add(jobj_snapshot, "alarm_snapshot", json_object_new_int(numOfAlarm));

                    json_object_array_add(jarr_snapshotlist, jobj_snapshot);
                }
            }
            fclose(fp1);
        }
        json_object_object_add(jobj, "Snapshot", jarr_snapshotlist);
    }
    else
    {
        json_object_object_add(jobj, "GetPlayBackInfoResult", json_object_new_string(ERROR_NO_STORAGE_DEVICE_STR));
    }

end:
    http_store_data_to_buffer(HTTP_RESPONSE_CODE_200_TEMPLATE);
    http_store_data_to_buffer(HTTP_JSON_RESPONSE_HEADER_TEMPLATE);
    http_store_data_to_buffer("%s\n", json_object_to_json_string(jobj));

    http_send_stored_data(pkt->fd);

    json_object_put(jobj);
    return 0;
}
#endif

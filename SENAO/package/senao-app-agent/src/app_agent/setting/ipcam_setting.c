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
;    File    : ipcam_setting.c
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
#include "app_agent.h"
#include "deviceinfo.h"
#include "sysCore.h"
#include "wan_setting.h"
#include "ipcam_setting.h"
#include "ipcam_json_setting.h"
#include "usb_json_setting.h"
#include "check/ipaddr_check.h"
#include "variable/api_wan.h"

/*--------------------------------------------------------------------------*/
/*                                PARAMETER                                 */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*                             GLOBAL VARIABLE                              */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*                            FUNCTION PROTOTYPE                            */
/*--------------------------------------------------------------------------*/

#if ROUTER_SUPPORT_IPCAM
/*****************************************************************
 * NAME:    get_ip_camera_list_cb
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
int get_ip_camera_list_cb(HTTPS_CB *pkt)
{
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
    IPCAM_DEVICE_INFO_T setting[16];
    char *result_str = ERROR_STR;
    char buf[256];
    char buf_upnpd_lease[256];
    FILE *fp;
    int i;

    if(NULL == pkt)
        return -1;

    memset(setting, 0, sizeof(setting));
    memset(buf, 0, sizeof(buf));
    i = 0;

    if(NULL != (fp = fopen(LOG_IPCAM_FILE, "r")))
    {
        while(NULL != fgets(buf, sizeof(buf), fp))
        {
            // 20131315 stevenlin: parse IPCAM info
            // TCP:8090:192.168.0.101:8090:UID1234567-SNCAM-000D0DA0073E-Room

            sscanf(buf,"TCP:%d:%[^:]:%*[^:]:UID%[^-]-SNCAM-%[^-]-%s",
                    &setting[i].httpd_port, setting[i].ip, setting[i].uid, setting[i].mac, setting[i].device_name);

            // get public app port of http from upnpd lease file.
            sysutil_interact(buf_upnpd_lease, sizeof(buf_upnpd_lease),
                    "cat /var/log/upnp.leases | grep %s | grep 'APP_AGENT_HTTP$'", setting[i].mac);
            if(0 != strlen(buf_upnpd_lease))
            {
                // TCP:50043:192.168.0.101:9091:00037FAAFF00-APP_AGENT_HTTP
                sscanf(buf_upnpd_lease,"TCP:%d:%*s",&setting[i].app_http_port);
            }

            // get public app port of https from upnpd lease file.
            sysutil_interact(buf_upnpd_lease, sizeof(buf_upnpd_lease),
                    "cat /var/log/upnp.leases | grep %s | grep 'APP_AGENT$'", setting[i].mac);
            if(0 != strlen(buf_upnpd_lease))
            {
                // TCP:50043:192.168.0.101:9091:00037FAAFF00-APP_AGENT
                sscanf(buf_upnpd_lease,"TCP:%d:%*s",&setting[i].app_https_port);
            }

            i++;
        }

        result_str = OK_STR;
        fclose(fp);
    }

    get_ip_camera_list_json_cb(pkt, setting, result_str);

    return 0;
}

/*****************************************************************
* NAME:    get_ipcam_samba_folder_list_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int get_ipcam_samba_folder_list_cb(HTTPS_CB *pkt)
{
    char *return_str = ERROR_STR;
    char *ptr = NULL;
    char buf[2048];
    char folder_list[32][64];
    int i;

    if(NULL == pkt)
    {
        return -1;
    }

    i = 0;
    memset(buf, 0x00, sizeof(buf));
    memset(folder_list, 0x00, sizeof(folder_list));

    if(sysutil_check_file_existed(LIGHTTPD_DEFAULT_IPCAM_FOLDER_PATH))
    {
        sysutil_interact(buf, sizeof(buf), "ls -l %s ", LIGHTTPD_DEFAULT_IPCAM_FOLDER_PATH);

        if(0 != strlen(buf))
        {
            ptr = strtok(buf, "\n");

            do
            {
                if('d' == ptr[0])
                {
                    /* drwxrwxrwx    3 root     root          8192 Nov 18  2016 EDS1130-6112 */
                    sscanf(ptr, "%*s %*s %*s %*s %*s %*s %*s %*s %s\n", folder_list[i]);
                    i++;
                }
            } while (NULL != (ptr = strtok(NULL, "\n")));
        }

        return_str = OK_STR;
    }
    else
    {
        return_str = ERROR_NO_USB_DEVICE_STR;
    }

    get_ipcam_samba_folder_list_json_cb(pkt, folder_list, return_str);

    return 0;
}

/*****************************************************************
* NAME:    get_ipcam_file_list_by_date_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int get_ipcam_file_list_by_date_cb(HTTPS_CB *pkt)
{
    bool result = TRUE;
    char *query_str = get_env(&pkt->envcfg, "QUERY_STRING");
    char filelist_link[256], filelist_path[256];
    char date[16], event_type[16], file_type[16], ipcam_folder[64];
    char ip_address[16];
    char client_ip[32]={0};
    int port, local_storage_port;


    if(NULL == pkt)
    {
        return -1;
    }

#if HAS_LIGHTTPD_SERVER
    sprintf(client_ip, "%s", sysutil_get_peername(pkt->fd));

    T_NAT_TRAVERSAL_INFO obj;
    memset(&obj, 0x00, sizeof(obj));
    api_get_nat_traversal_info(&obj);

    if(obj.is_behind_nat)
    {
        if(api_check_is_same_subnet(obj.wan_ip, client_ip, (char *)api_get_wan_mask()))
        {
            sprintf(ip_address, "%s", obj.wan_ip);
            port = obj.local_storage_port;
        }
        else
        {
            sprintf(ip_address, "%s", obj.external_wan_ip);
            port = obj.storage_port;
        }
    }
    else
    {
        sprintf(ip_address, "%s", obj.wan_ip);
        port = obj.local_storage_port;
    }

    local_storage_port = obj.local_storage_port;

    result = get_json_string_from_query(query_str, date, "Date");
    result &= get_json_string_from_query(query_str, event_type, "EventType");
    result &= get_json_string_from_query(query_str, file_type, "FileType");
    result &= get_json_string_from_query(query_str, ipcam_folder, "IPCameraFolder");

    if(TRUE == result)
    {
        /************************ filelist_link format *************************/
        /* usb_admin/sda1/IP_Camera/xxxxxx/video/20110101/.alarm_20110101_list */
        /* usb_admin/sda1/IP_Camera/xxxxxx/video/20110101/.sched_20110101_list */
        /***********************************************************************/
        if (!strcmp(file_type, "VIDEO"))
        {
            if (!strcmp(event_type, "ALARM"))
            {
                sprintf(filelist_link, "%s%s/%s/%s/%s/.alarm_%s_list",
                        LIGHTTPD_SAMBA_PATH,
                        IPCAM_FOLDER, ipcam_folder,VIDEO_FOLDER_NAME,
                        date, date);
                sprintf(filelist_path, "%s/%s/%s/%s/.alarm_%s_list",
                        LIGHTTPD_DEFAULT_IPCAM_FOLDER_PATH,
                        ipcam_folder,VIDEO_FOLDER_NAME,
                        date, date);
            }
            else if(!strcmp(event_type, "SCHEDULE"))
            {
                sprintf(filelist_link, "%s%s/%s/%s/%s/.sched_%s_list",
                        LIGHTTPD_SAMBA_PATH,
                        IPCAM_FOLDER, ipcam_folder,VIDEO_FOLDER_NAME,
                        date, date);
                sprintf(filelist_path, "%s/%s/%s/%s/.sched_%s_list",
                        LIGHTTPD_DEFAULT_IPCAM_FOLDER_PATH,
                        ipcam_folder,VIDEO_FOLDER_NAME,
                        date, date);
            }
            else
            {
                result = FALSE;
            }
        }
        else if (!strcmp(file_type, "IMAGE"))
        {
            if (!strcmp(event_type, "ALARM"))
            {
                sprintf(filelist_link, "%s%s/%s/%s/%s/.alarm_%s_photo_list",
                        LIGHTTPD_SAMBA_PATH,
                        IPCAM_FOLDER, ipcam_folder,SNAPSHOT_FOLDER_NAME,
                        date, date);
                sprintf(filelist_path, "%s/%s/%s/%s/.alarm_%s_photo_list",
                        LIGHTTPD_DEFAULT_IPCAM_FOLDER_PATH,
                        ipcam_folder,SNAPSHOT_FOLDER_NAME,
                        date, date);
            }
            else if(!strcmp(event_type, "SCHEDULE"))
            {
                sprintf(filelist_link, "%s%s/%s/%s/%s/.sched_%s_photo_list",
                        LIGHTTPD_SAMBA_PATH,
                        IPCAM_FOLDER, ipcam_folder,SNAPSHOT_FOLDER_NAME,
                        date, date);
                sprintf(filelist_path, "%s/%s/%s/%s/.sched_%s_photo_list",
                        LIGHTTPD_DEFAULT_IPCAM_FOLDER_PATH,
                        ipcam_folder,SNAPSHOT_FOLDER_NAME,
                        date, date);
            }
            else
            {
                result = FALSE;
            }
        }
        else
        {
            result = FALSE;
        }

        if((TRUE == result) && (access(filelist_path, 0) != 0))
        {
            result = FALSE;
        }
    }

    if(FALSE == result)
    {
        port = 0;
        memset(ip_address, 0x00, sizeof(ip_address));
        memset(filelist_link, 0x00, sizeof(filelist_link));
    }
#endif /* End of HAS_LIGHTTPD_SERVER*/

    get_file_list_by_date_json_cb(pkt, result, ip_address, port, local_storage_port, filelist_link);

    return 0;
}

/*****************************************************************
 * NAME:    get_play_back_info_cb
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:   Eason
 * Modify:
 ******************************************************************/
int get_play_back_info_cb(HTTPS_CB *pkt)
{
    char *query_str = get_env(&pkt->envcfg, "QUERY_STRING");
    char path[64], root[32], *return_str;

    if(NULL == pkt)
    {
        return -1;
    }

    memset(path, 0x00, sizeof(path));
    memset(root, 0x00, sizeof(root));

    if(get_json_string_from_query(query_str, root, "IPCameraFolder") == TRUE)
    {
#if SUPPORT_SIMPLE_NVR
        return_str = OK_STR;
        sprintf(path, "%s/%s", LIGHTTPD_DEFAULT_IPCAM_FOLDER_PATH, root);
#else
        return_str = ERROR_STR;
#endif
    }

    get_play_back_info_json_cb(pkt, path, return_str);

    return 0;
}
#endif

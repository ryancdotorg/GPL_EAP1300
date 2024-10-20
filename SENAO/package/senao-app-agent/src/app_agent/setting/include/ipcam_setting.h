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
;    File    : ipcam_setting.h
;    Abstract:
;
;       Modification History:
;       By              Date            Ver.   	Modification Description
;       --------------- --------        -----   -------------------------------------
;
;****************************************************************************/
#ifndef _IPCAM_SETTING_H_
#define _IPCAM_SETTING_H_

/*--------------------------------------------------------------------------*/
/*                           INCLUDE HEADER FILES                           */
/*--------------------------------------------------------------------------*/
#include "deviceinfo.h"
#include "../../appagents.h"
#if HAS_USB_SETTING
#include "usb_setting.h"
#endif

/*--------------------------------------------------------------------------*/
/*                                DEFINITION                                */
/*--------------------------------------------------------------------------*/
#define LOG_IPCAM_FILE "/var/log/upnp.httpdports"
#define IPCAM_FOLDER "/IP_Camera"
#define VIDEO_FOLDER_NAME "video"
#define SNAPSHOT_FOLDER_NAME "snapshot"
#define FILE_LIST "/tmp/file_list"
#define FOLDER_LIST "/tmp/folder_list"

#define LIGHTTPD_DEFAULT_IPCAM_FOLDER_PATH LIGHTTPD_DEFAULT_FOLDER_PATH IPCAM_FOLDER

/*--------------------------------------------------------------------------*/
/*                            FUNCTION PROTOTYPE                            */
/*--------------------------------------------------------------------------*/
#if ROUTER_SUPPORT_IPCAM
int get_ip_camera_list_cb(HTTPS_CB *pkt);
int get_ipcam_samba_folder_list_cb(HTTPS_CB *pkt);
int get_ipcam_file_list_by_date_cb(HTTPS_CB *pkt);
int get_play_back_info_cb(HTTPS_CB *pkt);
#endif
#endif

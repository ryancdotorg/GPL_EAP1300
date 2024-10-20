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
;    File    : usb_setting.h
;    Abstract:
;
;       Modification History:
;       By              Date     	Ver.   	Modification Description
;       --------------- -------- 	-----  	-------------------------------------
;
;****************************************************************************/
#ifndef _USB_SETTING_H_
#define _USB_SETTING_H_

/*--------------------------------------------------------------------------*/
/*                           INCLUDE HEADER FILES                           */
/*--------------------------------------------------------------------------*/
#include "../../appagents.h"
#include "deviceinfo.h"
#include "app_agent.h"

/*--------------------------------------------------------------------------*/
/*                                DEFINITION                                */
/*--------------------------------------------------------------------------*/
#define WEB_HOME_PATH			"/www"
#define VENDOR_CONFIG_FOLDER    ".EnGenius_config"
//20130923 liwei: copy from sysconfig/setting/include/lighttpdsetting.h
#define FOLDER_FILE_LIST_PATH   WEB_HOME_PATH "/" CURRENT_FOLDER_FILE_LIST_NAME
//20130923 liwei: copy from sysconfig/setting/include/usbmgrsetting.h
#define USB_ADMIN_FOLDER        "/usb_admin"
#define USB_FAVORITE_LIST       "favorite_list"
#define USB_PUBLIC_LIST         "public_list"

#define LIGHTTPD_DEFAULT_FOLDER_NAME "/sda1"
#define LIGHTTPD_SAMBA_PATH          USB_ADMIN_FOLDER LIGHTTPD_DEFAULT_FOLDER_NAME
#define LIGHTTPD_DEFAULT_FOLDER_PATH WEB_HOME_PATH USB_ADMIN_FOLDER LIGHTTPD_DEFAULT_FOLDER_NAME

#define MAX_USB_FILE_NAME_LEN   255

/*--------------------------------------------------------------------------*/
/*                                STRUCTURE                                 */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*                            FUNCTION PROTOTYPE                            */
/*--------------------------------------------------------------------------*/
int get_usb_port_settings_cb(HTTPS_CB *pkt);
int check_ip_login_cb(HTTPS_CB *pkt);
int set_usb_port_settings_cb(HTTPS_CB *pkt);
int get_storage_info_cb(HTTPS_CB *pkt);
int generate_file_list_by_type_cb(HTTPS_CB *pkt);
int check_generate_process_by_type_cb(HTTPS_CB *pkt);
int get_file_list_by_type_cb(HTTPS_CB *pkt);
int add_file_into_file_list_cb(HTTPS_CB *pkt);
int rename_file_in_file_list_cb(HTTPS_CB *pkt);
int delete_file_from_file_list_cb(HTTPS_CB *pkt);
int get_file_list_under_folder_cb(HTTPS_CB *pkt);
int get_file_list_under_folder_in_file_cb(HTTPS_CB *pkt);
int get_folder_path_by_file_name_cb(HTTPS_CB *pkt);
int delete_file_by_name_cb(HTTPS_CB *pkt);
int delete_file_by_file_name_cb(HTTPS_CB *pkt);
int edit_filename_by_name_cb(HTTPS_CB *pkt);
int add_file_into_favorite_list_cb(HTTPS_CB *pkt);
int delete_file_from_favorite_list_cb(HTTPS_CB *pkt);
int add_file_into_public_list_cb(HTTPS_CB *pkt);
int delete_file_from_public_list_cb(HTTPS_CB *pkt);
int reload_downsized_picture_cb(HTTPS_CB *pkt);
int get_music_information_cb(HTTPS_CB *pkt);
int search_file_by_name_cb(HTTPS_CB *pkt);
int create_folder_cb(HTTPS_CB *pkt);
#endif

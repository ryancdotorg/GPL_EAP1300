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
;    File    : usb_json_setting.c
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
#include "json.h"
#include "json_setting.h"
#include "usb_json_setting.h"
#include <sysUsbStorage.h>
/*--------------------------------------------------------------------------*/
/*                                DEFINITION                                */
/*--------------------------------------------------------------------------*/
#define VENDOR_CONFIG_FOLDER                    ".EnGenius_config"
/*--------------------------------------------------------------------------*/
/*                            FUNCTION PROTOTYPE                            */
/*--------------------------------------------------------------------------*/
/*****************************************************************
* NAME:    get_usb_port_settings_json_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
bool get_usb_port_settings_json_cb(HTTPS_CB *pkt, SAMBA_SETTING *settings, char *result)
{
    struct json_object *jobj;
    struct json_object *jstr_result;
    struct json_object *jstr_null;
    struct json_object *jint_mode;
    struct json_object *jstr_server_name;
    struct json_object *jstr_work_group;
    struct json_object *jstr_description;
    struct json_object *jstr_username;
    struct json_object *jstr_password;

    if(NULL == pkt)
    {
        return -1;
    }

    jobj = json_object_new_object();

    jstr_result = json_object_new_string(result);
    json_object_object_add(jobj, "GetUSBportSettingsResult", jstr_result);

    jint_mode = json_object_new_int(settings->mode);
    json_object_object_add(jobj, "Mode", jint_mode);

    if(1 == settings->mode)
    {
        jstr_server_name = json_object_new_string(settings->server_name);
        jstr_work_group = json_object_new_string(settings->work_group);
        jstr_description = json_object_new_string(settings->description);
        jstr_username = json_object_new_string(settings->username);
        jstr_password = json_object_new_string(settings->password);

        json_object_object_add(jobj, "ServerName", jstr_server_name);
        json_object_object_add(jobj, "WorkGroup", jstr_work_group);
        json_object_object_add(jobj, "Description", jstr_description);
        json_object_object_add(jobj, "UserName", jstr_username);
        json_object_object_add(jobj, "Password", jstr_password);
    }
    else
    {
        jstr_null = json_object_new_string(NULL_STR);

        json_object_object_add(jobj, "ServerName", jstr_null);
        json_object_object_add(jobj, "WorkGroup", jstr_null);
        json_object_object_add(jobj, "Description", jstr_null);
        json_object_object_add(jobj, "UserName", jstr_null);
        json_object_object_add(jobj, "Password", jstr_null);
    }

    http_store_data_to_buffer(HTTP_RESPONSE_CODE_200_TEMPLATE);
    http_store_data_to_buffer(HTTP_JSON_RESPONSE_HEADER_TEMPLATE);
    http_store_data_to_buffer("%s\n", json_object_to_json_string(jobj));

    http_send_stored_data(pkt->fd);

    /* Free obj */
    json_object_put(jstr_result);
    json_object_put(jint_mode);
    if(1 == settings->mode)
    {
        json_object_put(jstr_server_name);
        json_object_put(jstr_work_group);
        json_object_put(jstr_description);
        json_object_put(jstr_username);
        json_object_put(jstr_password);
    }
    else
    {
        json_object_put(jstr_null);
    }
    json_object_put(jobj);

    return 0;
}

/*****************************************************************
* NAME:    parse_usb_port_settings_json_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
bool parse_usb_port_settings_json_cb(char *query_str, SAMBA_SETTING *settings, char **return_str)
{
    bool result, is_jobj;
    struct json_object *jobj;
    struct json_object *jint_mode;
    struct json_object *jstr_server_name;
    struct json_object *jstr_work_group;
    struct json_object *jstr_description;
    struct json_object *jstr_username;
    struct json_object *jstr_password;

    result = TRUE;
    is_jobj = FALSE;

    if(0 != strlen(query_str))
    {
        if((jobj = json_tokener_parse(query_str)))
        {
            is_jobj = TRUE;

            if((jint_mode = json_object_object_get(jobj, "Mode")))
            {
                settings->mode = json_object_get_int(jint_mode);

                /* Free obj */
                json_object_put(jint_mode);
            }

            if((NULL == jint_mode) || (!api_check_digit_range(settings->mode, 1, 2)))
            {
                *return_str = ERROR_BAD_MODE_STR;
                result = FALSE;
                goto out;
            }

            if((jstr_server_name = json_object_object_get(jobj, "ServerName")))
            {
                sprintf(settings->server_name, "%s", json_object_get_string(jstr_server_name));

                /* Free obj */
                json_object_put(jstr_server_name);
            }

            if((NULL == jstr_server_name) || (!api_check_digit_range(strlen(settings->server_name), 1, 15)))
            {
                *return_str = ERROR_SERVER_NAME_STR;
                result = FALSE;
                goto out;
            }

            if((jstr_work_group = json_object_object_get(jobj, "WorkGroup")))
            {
                sprintf(settings->work_group, "%s", json_object_get_string(jstr_work_group));

                /* Free obj */
                json_object_put(jstr_work_group);
            }

            if((NULL == jstr_work_group) || (!api_check_digit_range(strlen(settings->work_group), 1, 15)))
            {
                *return_str = ERROR_GROUP_NAME_STR;
                result = FALSE;
                goto out;
            }

            if((jstr_description = json_object_object_get(jobj, "Description")))
            {
                sprintf(settings->description, "%s", json_object_get_string(jstr_description));

                /* Free obj */
                json_object_put(jstr_description);
            }

            if((NULL == jstr_description) || (!api_check_digit_range(strlen(settings->description), 0, 47)))
            {
                *return_str = ERROR_COMMENT_STR;
                result = FALSE;
                goto out;
            }

            if((jstr_username = json_object_object_get(jobj, "UserName")))
            {
                sprintf(settings->username, "%s", json_object_get_string(jstr_username));


                /* Free obj */
                json_object_put(jstr_username);
            }

            if((NULL == jstr_username) || (!api_check_digit_range(strlen(settings->username), 1, 15)))
            {
                *return_str = ERROR_USER_NAME_STR;
                result = FALSE;
                goto out;
            }

            if((jstr_password = json_object_object_get(jobj, "Password")))
            {
                sprintf(settings->password, "%s", json_object_get_string(jstr_password));

                /* Free obj */
                json_object_put(jstr_password);
            }

            if((NULL == jstr_password) || (!api_check_digit_range(strlen(settings->password), 1, 15)))
            {
                *return_str = ERROR_NEW_PW_STR;
                result = FALSE;
                goto out;
            }
        }
    }

out:
    if(is_jobj)
    {
        /* Free obj */
        json_object_put(jobj);
    }

    return result;
}

/*****************************************************************
 * NAME:    get_storage_info_json_cb
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
void get_storage_info_json_cb(HTTPS_CB *pkt, STORAGE_INFORMATION *setting, int remote_access)
{
    int i;
    struct json_object *jobj;
    struct json_object *jstr_result;
    struct json_object *jstr_remote_access;
    struct json_object *jarr_usb_info, *jobj_usb_info;
    struct json_object *jstr_usb_name, *jstr_usb_path, *jstr_usb_guest_path;
    struct json_object *jstr_total_size, *jstr_left_size, *jstr_used_precentage;

    i = 0;

    jstr_remote_access = NULL;
    jobj_usb_info = NULL;
    jstr_usb_name = NULL;
    jstr_usb_path = NULL;
    jstr_usb_guest_path = NULL;
    jstr_total_size = NULL;
    jstr_left_size = NULL;
    jstr_used_precentage = NULL;

    /* Construct the packet content in json format. */
    jobj = json_object_new_object();

    jstr_result = json_object_new_string(OK_STR);
    json_object_object_add(jobj, "GetStorageInfoResult", jstr_result);

    jstr_remote_access = json_object_new_string((remote_access)?ENABLE_STR:DISABLE_STR);
    json_object_object_add(jobj, "RemoteAccess", jstr_remote_access);

    jarr_usb_info = json_object_new_array();

    while(strlen(setting[i].usb_name))
    {
        jobj_usb_info = json_object_new_object();

        jstr_usb_name = json_object_new_string(setting[i].usb_name);
        json_object_object_add(jobj_usb_info, "UsbName", jstr_usb_name);

        jstr_usb_path = json_object_new_string(setting[i].usb_path);
        json_object_object_add(jobj_usb_info, "UsbPath", jstr_usb_path);

        jstr_usb_guest_path = json_object_new_string(setting[i].usb_guest_path);
        json_object_object_add(jobj_usb_info, "UsbGuestPath", jstr_usb_guest_path);

        jstr_total_size = json_object_new_string(setting[i].total_size);
        json_object_object_add(jobj_usb_info, "TotalSize", jstr_total_size);

        jstr_left_size = json_object_new_string(setting[i].left_size);
        json_object_object_add(jobj_usb_info, "LeftSize", jstr_left_size);

        jstr_used_precentage = json_object_new_string(setting[i].used_precentage);
        json_object_object_add(jobj_usb_info, "UsedPrecentage", jstr_used_precentage);

        json_object_array_add(jarr_usb_info, jobj_usb_info);
        i++;
    }

    json_object_object_add(jobj, "StorageInformation", jarr_usb_info);

    http_store_data_to_buffer(HTTP_RESPONSE_CODE_200_TEMPLATE);
    http_store_data_to_buffer(HTTP_JSON_RESPONSE_HEADER_TEMPLATE);
    http_store_data_to_buffer("%s\n", json_object_to_json_string(jobj));

    http_send_stored_data(pkt->fd);

    /* Free obj */
    json_object_put(jobj);
}


/*****************************************************************
 * NAME:    get_file_list_by_type_json_cb
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
void get_file_list_by_type_json_cb(HTTPS_CB *pkt, bool result, char *file_list_path)
{
    struct json_object *jobj;
    struct json_object *jstr_result;
    struct json_object *jstr_file_list_path;

    if(NULL == pkt)
    {
        return;
    }

    /* Construct the packet content in json format. */
    jobj = json_object_new_object();

    jstr_result = json_object_new_string((result)?OK_STR:ERROR_STR);
    json_object_object_add(jobj, "GetFileListByTypeResult", jstr_result);

    jstr_file_list_path = json_object_new_string((result)?file_list_path:"");
    json_object_object_add(jobj, "FileListPath", jstr_file_list_path);

    /* Store packet content into buffer and send it out */
    http_store_data_to_buffer(HTTP_RESPONSE_CODE_200_TEMPLATE);
    http_store_data_to_buffer(HTTP_JSON_RESPONSE_HEADER_TEMPLATE);
    http_store_data_to_buffer("%s\n", json_object_to_json_string(jobj));

    http_send_stored_data(pkt->fd);

    /* Free obj */
    json_object_put(jobj);

    return;
}

/*****************************************************************
 * NAME:    get_file_list_under_folder_json_cb
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
void get_file_list_under_folder_json_cb(HTTPS_CB *pkt)
{
    FILE *fp;
    char buf[1024], file_size[16], file_name[256];
    struct json_object *jobj;
    struct json_object *jstr_result;
    struct json_object *jarr_folder_list, *jstr_folder_list;
    struct json_object *jarr_file_list, *jobj_file_list;
    struct json_object *jstr_file_size, *jstr_file_name;

    if(NULL == pkt)
    {
        return;
    }

    jarr_folder_list = NULL;
    jarr_file_list = NULL;
    jobj_file_list = NULL;
    jstr_folder_list = NULL;
    jstr_file_name = NULL;
    jstr_file_size = NULL;

    /* Construct the packet content in json format. */
    jobj = json_object_new_object();

    jstr_result = json_object_new_string(OK_STR);
    json_object_object_add(jobj, "GetFileListUnderFolder", jstr_result);

    if(sysutil_check_file_existed(CURRENT_FOLDER_FILE_LIST_PATH))
    {
        if(NULL != (fp = fopen(CURRENT_FOLDER_FILE_LIST_PATH, "r")))
        {
            jarr_folder_list = json_object_new_array();
            jarr_file_list = json_object_new_array();
            while(NULL != fgets(buf, sizeof(buf), fp))
            {
                memset(file_size, 0x00, sizeof(file_size));
                memset(file_name, 0x00, sizeof(file_name));

                /* ======================= Format of ls -alh ======================= */
                /* -rwxr-xr-x    1 0        0          179.1k Jun 23  2012 cover.jpg */
                /* ================================================================= */
                sscanf(buf, "%*s %*s %*s %*s %s %*s %*s %*s %[^\n]s", file_size, file_name);

#if 0
                /* Skip the folder . and the folder .. */
                if(1 == strlen(file_name) && ('.' == file_name[0]))
                {
                    continue;
                }
                if(2 == strlen(file_name) && (0 == strcmp(file_name, "..")))
                {
                    continue;
                }
#endif
                /* Skip the folder whose name start with . */
                if('.' == file_name[0])
                {
                    continue;
                }
                /* Skip the link file or folder */
                if(strstr(file_name, " -> "))
                {
                    continue;
                }

                if('d' == buf[0])
                {
                    jstr_folder_list = json_object_new_string(file_name);
                    json_object_array_add(jarr_folder_list, jstr_folder_list);
                }
                else
                {
                    jobj_file_list = json_object_new_object();

                    jstr_file_name = json_object_new_string(file_name);
                    jstr_file_size = json_object_new_string(file_size);
                    json_object_object_add(jobj_file_list, "FileName", jstr_file_name);
                    json_object_object_add(jobj_file_list, "FileSize", jstr_file_size);

                    json_object_array_add(jarr_file_list, jobj_file_list);
                }
            }
            json_object_object_add(jobj, "FolderList", jarr_folder_list);
            json_object_object_add(jobj, "FileList", jarr_file_list);
            fclose(fp);
        }
    }

    /* Store packet content into buffer and send it out */
    http_store_data_to_buffer(HTTP_RESPONSE_CODE_200_TEMPLATE);
    http_store_data_to_buffer(HTTP_JSON_RESPONSE_HEADER_TEMPLATE);
    http_store_data_to_buffer("%s\n", json_object_to_json_string(jobj));

    http_send_stored_data(pkt->fd);

    /* Free obj */
    json_object_put(jobj);

    return;
}

/*****************************************************************
 * NAME:    get_file_list_under_folder_in_file_json_cb
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
void get_file_list_under_folder_in_file_json_cb(HTTPS_CB *pkt)
{
    struct json_object *jobj;
    struct json_object *jstr_result;
    struct json_object *jstr_file_list_path;

    if(NULL == pkt)
    {
        return;
    }

    jobj = NULL;
    jstr_result = NULL;
    jstr_file_list_path = NULL;

    /* Construct the packet content in json format. */
    jobj = json_object_new_object();

    jstr_result = json_object_new_string(OK_STR);
    json_object_object_add(jobj, "GetFileListUnderFolderInFile", jstr_result);

    jstr_file_list_path = json_object_new_string(CURRENT_FOLDER_FILE_LIST_NAME);
    json_object_object_add(jobj, "FolderFileListPath", jstr_file_list_path);

    /* Store packet content into buffer and send it out */
    http_store_data_to_buffer(HTTP_RESPONSE_CODE_200_TEMPLATE);
    http_store_data_to_buffer(HTTP_JSON_RESPONSE_HEADER_TEMPLATE);
    http_store_data_to_buffer("%s\n", json_object_to_json_string(jobj));

    http_send_stored_data(pkt->fd);

    /* Free obj */
    json_object_put(jobj);

    return;
}

/*****************************************************************
 * NAME:    get_folder_path_by_file_name_json_cb
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
void get_folder_path_by_file_name_json_cb(HTTPS_CB *pkt, bool result, char *folder_path)
{
    char *ptr;
    struct json_object *jobj;
    struct json_object *jstr_result;
    struct json_object *jarr_folder_path, *jstr_folder_path;

    if(NULL == pkt)
    {
        return;
    }

    jstr_folder_path = NULL;

    /* Construct the packet content in json format. */
    jobj = json_object_new_object();

    jstr_result = json_object_new_string((result)?OK_STR:ERROR_STR);
    json_object_object_add(jobj, "GetFileListUnderFolder", jstr_result);

    jarr_folder_path = json_object_new_array();

    if(strlen(folder_path))
    {
        ptr = strtok(folder_path, "\n");
        do
        {
            jstr_folder_path = json_object_new_string(ptr);
            json_object_array_add(jarr_folder_path, jstr_folder_path);
        } while((ptr = strtok(NULL, "\n")));
    }

    json_object_object_add(jobj, "FolderPath", jarr_folder_path);

    /* Store packet content into buffer and send it out */
    http_store_data_to_buffer(HTTP_RESPONSE_CODE_200_TEMPLATE);
    http_store_data_to_buffer(HTTP_JSON_RESPONSE_HEADER_TEMPLATE);
    http_store_data_to_buffer("%s\n", json_object_to_json_string(jobj));

    http_send_stored_data(pkt->fd);

    /* Free obj */
    json_object_put(jobj);

    return;
}

/*****************************************************************
 * NAME:    parse_delete_file_name_json_cb
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
bool parse_delete_file_name_json_cb(char *query_str, char filename[][128], char *folder)
{
    int i;
    bool result, is_jobj;
    struct json_object *jobj;
    struct json_object *jarr_filename, *jstr_filename, *jobj_folder;
    struct array_list *array_filename;

    result = TRUE;
    is_jobj = FALSE;

    if(0 != strlen(query_str))
    {
        if((jobj = json_tokener_parse(query_str)))
        {
            is_jobj = TRUE;

            jarr_filename = json_object_object_get(jobj, "Filename");
            array_filename = json_object_get_array(jarr_filename);

            for(i = 0; i < array_filename->length; i++)
            {
                jstr_filename = json_object_array_get_idx(jarr_filename, i);

                sprintf(filename[i], "%s", json_object_get_string(jstr_filename));
            }

            if((jobj_folder = json_object_object_get(jobj, "Folder")))
            {
                sprintf(folder, "%s", json_object_get_string(jobj_folder));
            }
            else
            {
                result = FALSE;
                goto out;
            }
        }
    }

out:
    if(is_jobj)
    {
        /* Free obj */
        json_object_put(jobj);
    }

    return result;
}

/*****************************************************************
 * NAME:    delete_file_response_json_cb
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
void delete_file_response_json_cb(HTTPS_CB *pkt, char *result_str, char broken_file_name[][256])
{
    struct json_object *jobj;
    struct json_object *jstr_result;
    struct json_object *jarr_file_list, *jstr_file_name;
    int i;
    char action_name[32];

    if(NULL == pkt)
    {
        return;
    }

    i = 0;
    jstr_result = NULL;
    jarr_file_list = NULL;
    jstr_file_name = NULL;
    memset(action_name, 0x00, sizeof(action_name));

    sprintf(action_name, "%sResult", pkt->json_action);

    /* Construct the packet content in json format. */
    jobj = json_object_new_object();

    jstr_result = json_object_new_string(result_str);
    json_object_object_add(jobj, action_name, jstr_result);

    jarr_file_list = json_object_new_array();

    while(0 != strlen(broken_file_name[i]))
    {
        jstr_file_name = json_object_new_string(broken_file_name[i]);
        json_object_array_add(jarr_file_list, jstr_file_name);

        i++;
    }

    json_object_object_add(jobj, "BrokenFileList", jarr_file_list);

    /* Store packet content into buffer and send it out */
    http_store_data_to_buffer(HTTP_RESPONSE_CODE_200_TEMPLATE);
    http_store_data_to_buffer(HTTP_JSON_RESPONSE_HEADER_TEMPLATE);
    http_store_data_to_buffer("%s\n", json_object_to_json_string(jobj));

    http_send_stored_data(pkt->fd);

    /* Free obj */
    json_object_put(jobj);

    return;
}

/*****************************************************************
 * NAME:    parse_delete_files_name_json_cb
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
bool parse_delete_files_name_json_cb(char *query_str, char filename[][256])
{
    int i;
    bool result;
    struct json_object *jobj;
    struct json_object *jarr_filename, *jstr_filename;
    struct array_list *array_filename;

    result = TRUE;
    jobj = NULL;
    jarr_filename = NULL;

    if(0 != strlen(query_str))
    {
        if((jobj = json_tokener_parse(query_str)))
        {
            jarr_filename = json_object_object_get(jobj, "Filename");

            if(jarr_filename)
            {
                array_filename = json_object_get_array(jarr_filename);

                for(i = 0; i < array_filename->length; i++)
                {
                    jstr_filename = json_object_array_get_idx(jarr_filename, i);
                    sprintf(filename[i], "%s", json_object_get_string(jstr_filename));
                }
            }
        }
    }

out:
    if(NULL != jobj)
    {
        /* Free obj */
        json_object_put(jobj);
    }

    return result;
}

/*****************************************************************
 * NAME:    parse_edit_file_name_json_cb
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
bool parse_edit_file_name_json_cb(char *query_str, char *old_name, char *new_name, char *folder)
{
    bool result, is_jobj;
    struct json_object *jobj;
    struct json_object *jobj_oldname, *jobj_newname, *jobj_folder;

    result = TRUE;
    is_jobj = FALSE;

    if(0 != strlen(query_str))
    {
        if((jobj = json_tokener_parse(query_str)))
        {
            is_jobj = TRUE;

            if((jobj_oldname = json_object_object_get(jobj, "OldName")))
            {
                sprintf(old_name, "%s", json_object_get_string(jobj_oldname));
            }
            else
            {
                result = FALSE;
                goto out;
            }

            if((jobj_newname = json_object_object_get(jobj, "NewName")))
            {
                sprintf(new_name, "%s", json_object_get_string(jobj_newname));
            }
            else
            {
                result = FALSE;
                goto out;
            }

            if((jobj_folder = json_object_object_get(jobj, "Folder")))
            {
                sprintf(folder, "%s", json_object_get_string(jobj_folder));
            }
            else
            {
                result = FALSE;
                goto out;
            }
        }
    }

out:
    if(is_jobj)
    {
        /* Free obj */
        json_object_put(jobj);
    }

    return result;
}

/*****************************************************************
 * NAME:    get_music_information_json_cb
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
void get_music_information_json_cb(HTTPS_CB *pkt)
{
    bool result;
    int i, start_index, desired_amount, buf_len;
#if 0
    char *query_str="{\n" \
                     "  \"UsbPath\" : \"\/sda1\",\n" \
                     "  \"StartIndex\" : \"80\",\n" \
                     "  \"DesiredAmount\" : \"2\",\n" \
                     "}";
#else
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
#endif
    char buf[1024];
    char *ptr;
    char mount_path[128], usb_path[16];
    char music_list_path[128];
    struct json_object *jobj;
    struct json_object *jint_start_index, *jint_desired_amount;
    struct json_object *jobj_out, *jstr_result;
    struct json_object *jarr_music_infos, *jstr_music_info;

    result = TRUE;
    jobj = NULL;
    jobj_out = NULL;
    jstr_result = NULL;
    jarr_music_infos = NULL;
    jstr_music_info = NULL;

    if(0 != strlen(query_str))
    {
        if((jobj = json_tokener_parse(query_str)))
        {
            if((jint_start_index = json_object_object_get(jobj, "StartIndex")))
            {
                start_index = json_object_get_int(jint_start_index);
            }
            else
            {
                result = FALSE;
                goto out;
            }

            if((jint_desired_amount = json_object_object_get(jobj, "DesiredAmount")))
            {
                desired_amount = json_object_get_int(jint_desired_amount);
            }
            else
            {
                result = FALSE;
                goto out;
            }

            if(FALSE == senao_json_object_get_string(jobj, "UsbPath", usb_path))
            {
                result = FALSE;
                goto out;
            }
        }
    }

    get_mount_path(mount_path); //todo... /www/usb_admin

    sprintf(music_list_path, "%s%s%s/%s/"MUSIC_FILE_LIST_NAME, mount_path, ('/' == usb_path[0])?"":"/", usb_path, VENDOR_CONFIG_FOLDER);

    if(sysutil_check_file_existed(music_list_path))
    {
        jobj_out = json_object_new_object();

        jstr_result = json_object_new_string((result)?OK_STR:ERROR_STR);
        json_object_object_add(jobj_out, "GetMusicInformationResult", jstr_result);

        jarr_music_infos = json_object_new_array();

        for(i = start_index; i < (start_index + desired_amount); i++)
        {
            //sysutil_interact(buf, sizeof(buf), "getinfo.sh --mount /tmp/usb -m /tmp/usb/sda1/music_list --index %d", i);
            //sysutil_interact(buf, sizeof(buf), "getinfo.sh --mount %s --path %s/%s -m %s --index %d", mount_path, usb_path, VENDOR_CONFIG_FOLDER, music_list_path, i);
            sysutil_interact(buf, sizeof(buf), "getinfo.sh --mount %s --path %s/%s -m %s --index %d", "/www", "usb_admin/sda1", VENDOR_CONFIG_FOLDER, music_list_path, i);

            /* Send error message if the assigned file is not existed */
            if(strstr(buf, "error: file not exist"))
            {
                result = FALSE;
            }
            else
            {
                buf_len = strlen(buf);
                buf[buf_len-1] = '\0';

                /* '\"' will cause the json crash, so rename it into '\ ' to avoid the crash. */
                ptr = buf;
                while('\0' != *ptr)
                {
                    if('\\' == *(ptr))
                    {
                        if('"' == *(ptr+1))
                        {
                            *ptr = ' ';
                            ptr++;
                        }
                    }

                    ptr++;
                }

                jstr_music_info = json_tokener_parse(buf);

                json_object_array_add(jarr_music_infos, jstr_music_info);
            }
        }

        json_object_object_add(jobj_out, "MusicInfos", jarr_music_infos);
    }
    else
    {
        result = FALSE;
    }

    if(TRUE == result)
    {
        /* Store packet content into buffer and send it out */
        http_store_data_to_buffer(HTTP_RESPONSE_CODE_200_TEMPLATE);
        http_store_data_to_buffer(HTTP_JSON_RESPONSE_HEADER_TEMPLATE);
        http_store_data_to_buffer("%s\n", json_object_to_json_string(jobj_out));

        http_send_stored_data(pkt->fd);
    }
    else
    {
        simple_json_response(pkt, ERROR_FILE_NOT_EXISTED_STR);
    }

out:
    /* Free obj */
    if(jobj)
    {
        json_object_put(jobj);
    }
}

/*****************************************************************
 * NAME:    search_file_by_name_json_cb
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
void search_file_by_name_json_cb(HTTPS_CB *pkt, bool result, char *file_names)
{
    char *ptr;
    struct json_object *jobj;
    struct json_object *jstr_result;
    struct json_object *jarr_file_names, *jstr_file_names;

    if(NULL == pkt)
    {
        return;
    }

    jstr_file_names = NULL;

    /* Construct the packet content in json format. */
    jobj = json_object_new_object();

    jstr_result = json_object_new_string((result)?OK_STR:ERROR_STR);
    json_object_object_add(jobj, "SearchFileByNameResult", jstr_result);

    jarr_file_names = json_object_new_array();

    if(strlen(file_names))
    {
        ptr = strtok(file_names, "\n");
        do
        {
            jstr_file_names = json_object_new_string(ptr);
            json_object_array_add(jarr_file_names, jstr_file_names);
        } while((ptr = strtok(NULL, "\n")));
    }

    json_object_object_add(jobj, "FileNames", jarr_file_names);

    /* Store packet content into buffer and send it out */
    http_store_data_to_buffer(HTTP_RESPONSE_CODE_200_TEMPLATE);
    http_store_data_to_buffer(HTTP_JSON_RESPONSE_HEADER_TEMPLATE);
    http_store_data_to_buffer("%s\n", json_object_to_json_string(jobj));

    http_send_stored_data(pkt->fd);

    /* Free obj */
    json_object_put(jobj);
}

/*****************************************************************
 * NAME:    add_file_into_public_list_json_cb
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
void add_file_into_public_list_json_cb(HTTPS_CB *pkt, char *result_str, char *public_link)
{
    struct json_object *jobj;
    struct json_object *jstr_result;
    struct json_object *jstr_public_link;

    if(NULL == pkt)
    {
        return;
    }

    /* Construct the packet content in json format. */
    jobj = json_object_new_object();

    jstr_result = json_object_new_string(result_str);
    json_object_object_add(jobj, "AddFileIntoPublicListResult", jstr_result);

    jstr_public_link = json_object_new_string(public_link);
    json_object_object_add(jobj, "PublicLink", jstr_public_link);

    /* Store packet content into buffer and send it out */
    http_store_data_to_buffer(HTTP_RESPONSE_CODE_200_TEMPLATE);
    http_store_data_to_buffer(HTTP_JSON_RESPONSE_HEADER_TEMPLATE);
    http_store_data_to_buffer("%s\n", json_object_to_json_string(jobj));

    http_send_stored_data(pkt->fd);

    /* Free obj */
    json_object_put(jobj);

    return;
}

/*****************************************************************
 * NAME:    get_file_list_by_date_json_cb
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
void get_file_list_by_date_json_cb(HTTPS_CB *pkt, bool result, char *ip_address, int port, int local_port, char *filelist_link)
{
    char action_str[64];
    struct json_object *jobj;
    struct json_object *jstr_result;
    struct json_object *jstr_ip, *jint_port, *jint_local_port, *jstr_filelist_link;

    /* Construct the packet content in json format. */
    jobj = json_object_new_object();

    sprintf(action_str, "%sResult", pkt->json_action);
    jstr_result = json_object_new_string((TRUE == result)?OK_STR:ERROR_STR);
    json_object_object_add(jobj, action_str, jstr_result);

    jstr_ip = json_object_new_string(ip_address);
    jint_port = json_object_new_int(port);
    jint_local_port = json_object_new_int(local_port);
    jstr_filelist_link = json_object_new_string(filelist_link);

    json_object_object_add(jobj, "FileListDownIp", jstr_ip);
    json_object_object_add(jobj, "FileListDownPort", jint_port);
    json_object_object_add(jobj, "FileListDownLocalPort", jint_local_port);
    json_object_object_add(jobj, "FileListDownlink", jstr_filelist_link);

    http_store_data_to_buffer(HTTP_RESPONSE_CODE_200_TEMPLATE);
    http_store_data_to_buffer(HTTP_JSON_RESPONSE_HEADER_TEMPLATE);
    http_store_data_to_buffer("%s\r\n", json_object_to_json_string(jobj));

    http_send_stored_data(pkt->fd);

    /* Free obj */
    json_object_put(jobj);
}


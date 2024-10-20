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
;    File    : usb_setting.c
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
#include "admin_cfg.h"
#include "usb_setting.h"
#include "usb_json_setting.h"
#include <sysUsbStorage.h>
#include <sysCommon.h>

/*--------------------------------------------------------------------------*/
/*                                PARAMETER                                 */
/*--------------------------------------------------------------------------*/
typedef struct  _FILE_TYPE_INFO_ {
    char *file_type_name;
    FILE_TYPE file_type;
    char *file_list_name;
} FILE_TYPE_INFO;

FILE_TYPE_INFO file_type_info[] = {
    {"DOCUMENT",     DOCUMENT_FILE_TYPE,     DOCUMENT_FILE_LIST_NAME},
    {"MUSIC",        MUSIC_FILE_TYPE,        MUSIC_FILE_LIST_NAME},
    {"PICTURE",      PICTURE_FILE_TYPE,      PICTURE_FILE_LIST_NAME},
    {"VIDEO",        VIDEO_FILE_TYPE,        VIDEO_FILE_LIST_NAME}
};

/*--------------------------------------------------------------------------*/
/*                                DEFINITION                                */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*                            FUNCTION PROTOTYPE                            */
/*--------------------------------------------------------------------------*/
//Common Function
/*****************************************************************
* NAME:    add_file_into_specific_list_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int add_file_into_specific_list_cb(HTTPS_CB *pkt, char *specific_list, char *file_name)
{
#if 0
    char *query_str="{\n" \
                    "  \"FileName\" : \"/sda1/Music/2011-01-14 黃小琥 - 如果能… 重來/01 愛情開箱文.mp3\"\n" \
                    "}";
#else
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
#endif
    char usb_path[32];
    char specific_list_path[64];
    char tmp_file_name[512];
    char *ptr;
    bool result;

    if(NULL == pkt)
    {
        return -1;
    }

    result = FALSE;
    ptr = NULL;

    if(NULL != query_str)
    {
        get_json_string_from_query(query_str, tmp_file_name, "FileName");
        sysCheckStringOnWeb(tmp_file_name, file_name);

        if('/' == file_name[0])
        {
            ptr = strchr(&file_name[1], '/'); /* Skip the first '/' character */

			// 20130923 liwei: The new format is /usb_admin/sda1/Music/XXX,
			//				   need skip the '/' more once.
			if(strncmp(&file_name[1],"usb_admin",9)==0)
			{
				ptr = strchr(&ptr[1], '/');
			}
        }
        else
        {
            ptr = strchr(file_name, '/');
			// 20130923 liwei: The new format is /usb_admin/sda1/Music/XXX,
			//				   need skip the '/' more once.
			if(strncmp(file_name,"usb_admin",9)==0)
			{
				ptr = strchr(&ptr[1], '/');
			}
        }

        if(NULL != ptr)
        {
            snprintf(usb_path, ptr - file_name + 1, "%s", file_name);

            sprintf(specific_list_path, "%s%s%s/%s/%s",
                    WEB_HOME_PATH, ('/' == usb_path[0])?"":"/", usb_path,
                    VENDOR_CONFIG_FOLDER, specific_list);

            SYSTEM("add_file_name_into_file_list.sh \"%s\"  \"%s%s\";",
                    specific_list_path, ('/' == file_name[0])?"":"/", file_name);

            result = TRUE;
        }
    }

    return result;
}

/*****************************************************************
* NAME:    delete_file_from_specific_list_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int delete_file_from_specific_list_cb(HTTPS_CB *pkt, char *specific_list)
{
#if 0
    char *query_str="{\n" \
                    "  \"FileName\" : \"/sda1/Music/2011-01-14 黃小琥 - 如果能… 重來/01 愛情開箱文.mp3\"\n" \
                    "}";
#else
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
#endif
    char usb_path[32];
    char favorite_list_path[64];
    char file_name[256];
    char *ptr;
    bool result;

    if(NULL == pkt)
    {
        return -1;
    }

    result = FALSE;
    ptr = NULL;

    if(NULL != query_str)
    {
        get_json_string_from_query(query_str, file_name, "FileName");

        if('/' == file_name[0])
        {
            ptr = strchr(&file_name[1], '/') + 1; /* Skip the first '/' character */

			// 20130923 liwei: The new format is /usb_admin/sda1/Music/XXX,
			//				   need skip the '/' more once.
			if(strncmp(&file_name[1],"usb_admin",9)==0)
			{
				ptr = strchr(ptr, '/') + 1;
			}
        }
        else
        {
            ptr = strchr(file_name, '/') + 1; /* Skip the first '/' character */

			// 20130923 liwei: The new format is usb_admin/sda1/Music/XXX,
			//				   need skip the '/' more once.
			if(strncmp(file_name,"usb_admin",9)==0)
			{
				ptr = strchr(ptr, '/') + 1;
			}
        }

        if(NULL != ptr)
        {
            snprintf(usb_path, ptr - file_name + 1, "%s", file_name);

            sprintf(favorite_list_path, "%s%s/%s/%s",
                    WEB_HOME_PATH, usb_path, VENDOR_CONFIG_FOLDER, specific_list);
            SYSTEM("sed '\\#^%s$#d' -i %s", file_name, favorite_list_path);

            result = TRUE;
        }
    }

    send_simple_response(pkt, (TRUE == result)?OK_STR:ERROR_STR);
    return 0;
}

/*****************************************************************
* NAME:    get_mount_path
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
void get_mount_path(char *mount_path)
{
    char mount_point[128];
    char *ptr = NULL;

    memset(mount_point, 0x00, sizeof(mount_point));
#if 0
    sysconf_util_cmd("inform usb-mount-point ,", mount_point);

    /* =================== Mount point format =================== */
    /* /tmp/usb/sda1,/tmp/usb/sdb1,/tmp/usb/sdc1,...              */
    /* /mnt/sda1,/mnt/sdb1,/mnt/sdc1,...                          */
    /* ========================================================== */
    /* Get the path of mounted USB device, e.g. /mnt or /tmp/usb. */
    ptr = strstr(mount_point, "/sd");

    /* Format of mount path : /tmp/usb or /mnt */
    snprintf(mount_path, (ptr - mount_point) + 1, "%s", mount_point);
#else
    sprintf(mount_path, "%s", "/www/usb_admin");
#endif
}

/*****************************************************************
* NAME:    get_file_type
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_file_type(char *file_type)
{
    int i;
    for(i = 0; i < T_NUM_OF_ELEMENTS(file_type_info); i++)
    {
        if(0 == strncmp(file_type_info[i].file_type_name, file_type, strlen(file_type)))
        {
            break;
        }
    }
    return i;
}

/*****************************************************************
* NAME:    check_escape_character
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
void check_escape_character(char *srcStr, char *destStr)
{
    int len;
    int i, j;
    int cnt = 0;

    // protect
    if(!destStr)
    {
        return;
    }

    // protect
    if(srcStr == NULL || (len = strlen(srcStr))==0)
    {
        destStr[0] = '\0';
        return;
    }

    for(i=0; i<=len; i++)
    {
        if(srcStr[i] == '\"' || srcStr[i] == '`' || srcStr[i] == '$' || srcStr[i] == '[')
        {
            destStr[cnt++] = '\\';
        }
        else if(srcStr[i] == '\\')
        {
            for(j = 0; j < 4; j++)
            {
                destStr[cnt++] =  '\\';
            }
        }
        destStr[cnt++] = srcStr[i];
    }
}

/*****************************************************************
* NAME:    is_generating_file_list_running
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
bool is_generating_file_list_running(char *file_type)
{
    int index;
    bool result;

	// 20130927 liwei: Use genfile.sh to generate all file list.
	// If genfile.sh is existed, return TRUE.
	if(FINDPROC("genfile.sh"))
	{
		return TRUE;
	}

    result = FALSE;
    index = get_file_type(file_type);

    switch(file_type_info[index].file_type)
    {
        case DOCUMENT_FILE_TYPE:
            if(FINDPROC("gendoclist.sh"))
            {
                result = TRUE;
            }
            break;
        case MUSIC_FILE_TYPE:
            if(FINDPROC("genmuslist.sh"))
            {
                result = TRUE;
            }
            break;
        case PICTURE_FILE_TYPE:
            if(FINDPROC("genpiclist.sh"))
            {
                result = TRUE;
            }
            break;
        case VIDEO_FILE_TYPE:
            if(FINDPROC("genvidlist.sh"))
            {
                result = TRUE;
            }
            break;
        default:
            break;
    }

    return result;
}

/*****************************************************************
* NAME:    get_usb_port_settings_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int get_usb_port_settings_cb(HTTPS_CB *pkt)
{
    SAMBA_SETTING setting;
    char *return_str;
    int flag;

    if(NULL == pkt)
    {
        return -1;
    }

    memset(&setting, 0x00, sizeof(setting));

    api_get_integer_option(STORAGE_FUNCTION_SAMBAENABLE_OPTION, &flag);

    if(flag)
    {
        setting.mode = 1;
    }
    else
    {
        api_get_integer_option(STORAGE_FUNCTION_PRINTENABLE_OPTION, &flag);

        if(flag)
        {
            setting.mode = 2;
        }
    }

    api_get_string_option(SAMBA_SAMBA_NAME_OPTION, setting.server_name, sizeof(setting.server_name));
    api_get_string_option(SAMBA_SAMBA_WORKGROUP_OPTION, setting.work_group, sizeof(setting.work_group));
    api_get_string_option(SAMBA_SAMBA_DESCRIPTION_OPTION, setting.description, sizeof(setting.description));
    api_get_string_option(SAMBA_SAMBA_USERNAME_OPTION, setting.username, sizeof(setting.username));
    api_get_string_option(SAMBA_SAMBA_USERPASSWORD_OPTION, setting.password, sizeof(setting.password));

    if(pkt->json)
    {
        get_usb_port_settings_json_cb(pkt, &setting, OK_STR);
    }

    return 0;
}

/*****************************************************************
* NAME:    set_usb_port_settings_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int set_usb_port_settings_cb(HTTPS_CB *pkt)
{
    bool result = TRUE;
    int  flag;
    char *return_str;
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
    char *ptr1, *ptr2;
    char buf1[512];
    char buf2[512];
    char uuid[16], sec_type[16];
    char funclist[256];
    SAMBA_SETTING setting;

    // The default value in return packet is ERROR.
    return_str = ERROR_STR;

    if(NULL == pkt || 0 == strlen(query_str))
    {
        goto send_pkt;
    }

    memset(&setting, 0x00, sizeof(SAMBA_SETTING));

    if(pkt->json)
    {
        result = parse_usb_port_settings_json_cb(query_str, &setting, &return_str);
    }

    if(TRUE != result) goto send_pkt;

    result = API_RC_SUCCESS;

    if(1 == setting.mode)
    {
        /* Server mode */
        /* If Time Machine is Enabled, the user should disable this function in advance. */
        api_get_integer_option(STORAGE_FUNCTION_TIMEMACHINEENABLE_OPTION, &flag);

        if(flag)
        {
            return_str = ERROR_TIMEMACHINE_IS_ENABLE_STR;
            goto send_pkt;
        }

        memset(buf1, 0x00, sizeof(buf1));
        sys_interact(buf1, sizeof(buf1), "blkid | grep \"/dev/sd\"");

        if(strlen(buf1))
        {
            if(NULL != (ptr1 = strstr(buf1, "SEC_TYPE=\"")))
            {
                ptr1 += strlen("SEC_TYPE=\"");
                ptr2 = strchr(ptr1, '"');

                snprintf(sec_type, (ptr2 - ptr1) + 1, "%s", ptr1);
            }

            if(NULL != (ptr1 = strstr(buf1, "UUID=\"")))
            {
                ptr1 += strlen("UUID=\"");
                ptr2 = strchr(ptr1, '"');

                snprintf(uuid, (ptr2 - ptr1) + 1, "%s", ptr1);
            }

            memset(buf1, 0x00, sizeof(buf1));
            api_get_string_option(STORAGE_FUNCTION_FUNCLIST_OPTION, buf1, sizeof(buf1));

            if(NULL != (ptr1 = strstr(buf1, uuid)))
            {
                ptr2 = strchr(ptr1, ',');

                if((ptr1 - strlen(",on_share_")) >= buf1)
                {
                    ptr1 = strchr(ptr1 - strlen(",on_share_"), ',');
                    memset(buf2, 0x00, sizeof(buf2));
                    snprintf(buf2, ptr1 - buf1 + 1, "%s", buf1);
                    sprintf(funclist, "%s,on_share_%s_%s%s", buf2, uuid, sec_type, (NULL != ptr2)?ptr2:"");
                }
                else
                {
                    ptr1 = buf1;
                    sprintf(funclist, ",on_share_%s_%s%s", uuid, sec_type, (NULL != ptr2)?ptr2:"");
                }
            }
            else
            {
                sprintf(funclist, "%s,on_share_%s_%s", buf1, uuid, sec_type);
            }
            result |= api_set_string_option(STORAGE_FUNCTION_FUNCLIST_OPTION, funclist, sizeof(funclist));
        }

        result |= api_set_integer_option(STORAGE_FUNCTION_SAMBAENABLE_OPTION, 1);
        result |= api_set_integer_option(STORAGE_FUNCTION_PRINTENABLE_OPTION, 0);

        result |= api_set_string_option(SAMBA_SAMBA_NAME_OPTION, setting.server_name, sizeof(setting.server_name));
        result |= api_set_string_option(SAMBA_SAMBA_WORKGROUP_OPTION, setting.work_group, sizeof(setting.work_group));
        result |= api_set_string_option(SAMBA_SAMBA_DESCRIPTION_OPTION, setting.description, sizeof(setting.description));
        result |= api_set_string_option(SAMBA_SAMBA_USERNAME_OPTION, setting.username, sizeof(setting.username));
        result |= api_set_string_option(SAMBA_SAMBA_USERPASSWORD_OPTION, setting.password, sizeof(setting.password));

    }
    else if(2 == setting.mode)
    {
        /* NetUSB mode */
        result |= api_set_integer_option(STORAGE_FUNCTION_SAMBAENABLE_OPTION, 0);
        result |= api_set_integer_option(STORAGE_FUNCTION_PRINTENABLE_OPTION, 1);
    }

    if(API_RC_SUCCESS == result)
    {
        return_str = OK_STR;

        APPAGENT_SYSTEM("uci commit");
        APPAGENT_SYSTEM("/etc/init.d/storage reload &");
    }

send_pkt:
    /* Send response packet */
    send_simple_response(pkt, return_str);

    return result;
}

/*****************************************************************
* NAME:    get_storage_info_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
/* 
============================== Data Example ==============================
{ "GetStorageInfoResult": "OK", 
  "RemoteAccess": "ENABLE",
  "StorageInformation": [
     { "UsbName": "ESD-ISO",
       "UsbPath": "\/usb_admin\/sda1",
       "UsbGuestPath": "\/usb_admin\/sda1\/Guest",
       "TotalSize": "7814140",
       "LeftSize": "5521400",
       "UsedPrecentage": "29" } ] }
=========================================================================== 
*/ 
int get_storage_info_cb(HTTPS_CB *pkt)
{

#if 1
    int i;
    int remote_access;
    char buf[256], mount_point[128], real_mount_path[32];
    char *mount_ptr, *ptr;
    STORAGE_INFORMATION setting[8];
    int rval;
    char usb_num_str[16]={0};
    int usb_num=0;
    char usb_name[64];
    char usb_path[32];

#if 0//HAS_GUEST_ACCOUNT && HAS_PRIORITY_WEB_ACCOUNT
    webAccount_t acc;
#endif
    if(NULL == pkt)
    {
        return -1;
    }

#if 0//HAS_PBX
    memset(entalk_usb_info, 0 , sizeof(entalk_usb_info));
#endif

    i = 0;
    remote_access = 0;
    mount_ptr = NULL;
    ptr = NULL;
    memset(&setting, 0x00, sizeof(STORAGE_INFORMATION)*8);

    remote_access = 1;//apCfgGetIntValue(ENSHARE_REMOTE_ENABLE_TOK); //todo
#if 0
    sysconf_util_cmd("inform usb-mount-point ,", mount_point);
    mount_ptr = mount_point;
    mount_ptr = strtok(mount_ptr, ",");
    do
    {
        if(NULL == mount_ptr)
        {
            break;
        }
        if((0 != strlen(mount_ptr)) && (sysutil_check_file_existed(mount_ptr)))
        {
            /* Get partition path, e.g. "usb_admin/sdax" */
            // mount | grep "/mnt/sda1 on"
            rval = sysutil_interact(buf, sizeof(buf), "mount | grep \"%s on\"", mount_ptr);
            if(rval>=0)
            {
                // /mnt/sda1 on /mnt/usb_admin/sda1 type none (rw,bind)
                sscanf(buf, "%*s %*s /%*[^/]%s", setting[i].usb_path);
#if 0//HAS_PBX
                memcpy(entalk_usb_info[i], setting[i].usb_path, 32);
                printf("entalk_path:%s\n", entalk_usb_info[i]);
#endif
            }

#if 0//HAS_GUEST_ACCOUNT && HAS_PRIORITY_WEB_ACCOUNT   //todo
			// 20130913 liwei: get guest path.
			apCfgGetValue(WEB_ACCOUNT_02_TOK, &acc);

			if(acc.isEnable==1)
			{
				if(acc.isEnable==1)
				{
					sprintf(setting[i].usb_guest_path, "%s/%s", setting[i].usb_path, apCfgGetStrValue(GUEST_FOLDER_TOK));
				}
			}

#endif
            /* Get real point of mounted USB device. */
            sysutil_interact(buf, sizeof(buf), "mount | grep %s", mount_ptr);
            sscanf(buf, "%s %*[^'\n']s", real_mount_path);

            /* Get the name of mounted USB device */
            memset(buf, 0x00, sizeof(buf));
            sysutil_interact(buf, sizeof(buf), "ntfslabel -f %s", real_mount_path);

#if 0
            if(strstr(buf, "Failed to mount"))
            {
                sprintf(setting[i].usb_name, "disk");
            }
            else if(tmp_ptr = strstr(buf, "WARNING: "))
            {
                tmp_ptr = strchr(buf, '\n') + 1; /* +1 is used to skip the character '\n'*/
                sprintf(setting[i].usb_name, tmp_ptr);
            }
#else
			// 20130913 liwei: The usb name may be '\n'.
            if(strstr(buf, "---") || strcmp(buf,"\n")==0)
            {
                sprintf(setting[i].usb_name, "disk");
            }
#endif
            else
            {
                sscanf(buf, "%[^'\n']s", setting[i].usb_name);
            }

            memset(buf, 0x00, sizeof(buf));
            sysutil_interact(buf, sizeof(buf), "df %s | grep \"%s\"", mount_ptr, mount_ptr);

            // The output format of command df is as follows.
            // Filesystem           1k-blocks      Used Available Use% Mounted on
            // /dev/sda1               997072    454768    542304  46% /mnt/sda1
            sscanf(buf, "%*s %s %*s %s %s %*s", setting[i].total_size, setting[i].left_size, setting[i].used_precentage);

            /* Remove the symbol '%' */
            if('%' == setting[i].used_precentage[strlen(setting[i].used_precentage) - 1])
            {
                setting[i].used_precentage[strlen(setting[i].used_precentage) - 1] = '\0';
            }
        }

        i++;
    } while((mount_ptr = strtok(NULL, ",")));
#else
    //Support only one usb now ,hardcode ... todo..

    rval=sysutil_interact(usb_num_str, sizeof(usb_num_str), "mount |grep usb_admin |wc |awk '{print $1}'");
	if(rval<0) //Fail, No usb Plug in
	{
        cprintf("DEBUG %s[%d], ====== [No USB Plug in]\n", __FUNCTION__, __LINE__, buf);		
	}
	else
	{
		usb_num=atoi(usb_num_str);

        for(i=0 ;i<usb_num ;i++)
        {
            memset(usb_path, 0x0, sizeof(usb_path));
            sysutil_interact(usb_path, sizeof(usb_path), "mount |grep usb_admin |head -n %d |tail -n 1 |awk '{print $3}' |cut -c 5-20", i+1);

            //remove \n
            usb_path[strlen(usb_path)-1]='\0';

            sprintf(setting[i].usb_path, "%s", usb_path);
            sprintf(setting[i].usb_guest_path, "%s/Guest", usb_path);

            /* Get the name of mounted USB device */
            memset(buf, 0x0, sizeof(buf));
            sysutil_interact(buf, sizeof(buf), "cat /tmp/storage_vendor");  //2 usb? todo...
            //remove \n
            buf[strlen(buf)-1]='\0';
            sprintf(setting[i].usb_name, buf);

            /* Get size and used precentage */
            memset(buf, 0x00, sizeof(buf));
            sysutil_interact(buf, sizeof(buf), "df |grep \"%s\"", usb_path);
            // The output format of command df is as follows.
            // Filesystem           1k-blocks      Used Available Use% Mounted on
            // /dev/sda1               997072    454768    542304  46% /mnt/sda1
            sscanf(buf, "%*s %s %*s %s %s %*s", setting[i].total_size, setting[i].left_size, setting[i].used_precentage);

            /* Remove the symbol '%' */
            if('%' == setting[i].used_precentage[strlen(setting[i].used_precentage)-1])
            {
                setting[i].used_precentage[strlen(setting[i].used_precentage)-1] = '\0';
            }
        }
	}


#endif

  //  if(pkt->json)
  //  {
  //      get_usb_port_settings_json_cb(pkt, &setting, OK_STR);
  //  }


    if(pkt->json)
    {
        get_storage_info_json_cb(pkt, setting, remote_access);
    }

#endif
    return 0;
}

/*****************************************************************
* NAME:    check_ip_login_cb
* ---------------------------------------------------------------
* FUNCTION: check_ip_login_cb
* INPUT:    
* OUTPUT:   
* Author:   20161207 Jason
* Modify:   Used by lighttpd to ask is ip already login app_agent
******************************************************************/
int check_ip_login_cb(HTTPS_CB *pkt)
{
#if 0
    char *query_str="{\"IP\" : \"192.168.8.50\"}";
#else
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
#endif
    char ipstr[32]={0};
    char *result_str;
    int i=0, ipfind=0;
    LONG admin_time=0;
	char has_login_ip[64]={0};

    if(NULL == pkt)
    {
        return -1;
    }

    if(NULL != query_str)
    {
        if(get_json_string_from_query(query_str, ipstr, "IP"))
        {
        	for(i=0; i < NUM_APPAGENT_CONCURENT_LOGIN; i++)
        	{
#if 1//SUPPORT_IPV6_SETTING
        		AdminCfg_GetHasLoginIp(has_login_ip ,i);
                //cprintf("DEBUG %s[%d], ====== has_login_ip[%s]\n", __FUNCTION__, __LINE__, has_login_ip); 
        		if(strcmp(has_login_ip, ipstr)==0)
#else
        		AdminCfg_GetHasLoginIp(&has_login_ip ,i);
        		if(has_login_ip == ntohl(peer.sin_addr.s_addr))
#endif
        		{
                    ipfind=1;
                    result_str = OK_STR;                        
        			break;
        		}
        	}

            if(ipfind==0)  //IP never login
            {
                result_str = ERROR_STR;
            }
        }
    }

    send_simple_response(pkt, result_str);
    return 0;
}

/*****************************************************************
* NAME:    generate_file_list_by_type_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int generate_file_list_by_type_cb(HTTPS_CB *pkt)
{
#if 0
    char *query_str="{\n" \
                    "  \"FileType\" : \"PICTURE\"," \
                    "  \"UsbPath\" : \"/sda1\"\n" \
                    "}";
#else
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
#endif
    char file_type[16], usb_path[32], file_list_path[64];
    char mount_point[32];
    char *result_str;
    int index;
    bool result;

    result = FALSE;
    result_str = ERROR_STR;

    if(NULL == pkt)
    {
        return -1;
    }

    if(NULL != query_str)
    {
        if(get_json_string_from_query(query_str, file_type, "FileType"))
        {
            index = get_file_type(file_type);

            if(get_json_string_from_query(query_str, usb_path, "UsbPath"))
            {
                sprintf(mount_point, "%s%s%s", WEB_HOME_PATH, ('/' == usb_path[0])?"":"/", usb_path);

                sprintf(file_list_path, "%s/%s/%s",
                        mount_point,
                        VENDOR_CONFIG_FOLDER,
                        file_type_info[index].file_list_name);

                result = is_generating_file_list_running(file_type);

                if(FALSE == result)
                {
                    result = FALSE;
                    result = generateFileListByType(mount_point, file_list_path, file_type_info[index].file_type);
                    if(1 == result)
                    {
                        result_str = OK_STR;
                    }
                }
                else
                {
                    result_str = ERROR_PROCESS_IS_RUNNING_STR;
                }
            }
        }
    }

    send_simple_response(pkt, result_str);
    return 0;
}

/*****************************************************************
* NAME:    check_generate_process_by_type_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int check_generate_process_by_type_cb(HTTPS_CB *pkt)
{
#if 0
    char *query_str="{\n" \
                    "  \"FileType\" : \"PICTURE\"" \
                    "}";
#else
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
#endif
    char file_type[16];
    char *result_str;
    int index;
    bool result;

    result = FALSE;
    result_str = ERROR_STR;

    if(NULL == pkt)
    {
        return -1;
    }

    if(NULL != query_str)
    {
        if(get_json_string_from_query(query_str, file_type, "FileType"))
        {
            result = is_generating_file_list_running(file_type);

            if(FALSE == result)
            {
                result_str = OK_STR;
            }
            else
            {
                result_str = ERROR_PROCESS_IS_RUNNING_STR;
            }
        }
    }

    send_simple_response(pkt, result_str);
    return 0;

}

/*****************************************************************
* NAME:    get_file_list_by_type_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_file_list_by_type_cb(HTTPS_CB *pkt)
{
#if 0
    char *query_str="{\n" \
                    "  \"FileType\" : \"DOCUMENT\"," \
                    "  \"UsbPath\" : \"/sda1\"\n" \
                    "}";
#else
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
#endif
    char file_type[16];
    char file_list_path[256];
    char real_file_list_path[256];
    char usb_path[32];
    int index;
    bool result, is_file_existed;

    if(NULL == pkt)
    {
        return -1;
    }

    result = FALSE;
    is_file_existed = FALSE;

    if(NULL == query_str)
    {
        send_simple_response(pkt, ERROR_STR);
    }
    else
    {
        if((get_json_string_from_query(query_str, file_type, "FileType")) &&
           (get_json_string_from_query(query_str, usb_path, "UsbPath")))
        {
            printf("[app_agentd] GetFileListByType - %s \n", file_type);
            index = get_file_type(file_type);

            sprintf(file_list_path, "%s%s/%s/%s", ('/' == usb_path[0])?"":"/", usb_path, VENDOR_CONFIG_FOLDER, file_type_info[index].file_list_name);

            sprintf(real_file_list_path, "%s%s",
                    WEB_HOME_PATH,
                    file_list_path);
printf("[app_agentd] real_file_list_path - %s \n", real_file_list_path);
            if(sysutil_check_file_existed(real_file_list_path))
            {
                is_file_existed = TRUE;
            }
        }

        if(pkt->json)
        {
            get_file_list_by_type_json_cb(pkt, is_file_existed, file_list_path);
        }
    }

    return 0;

}

/*****************************************************************
* NAME:    add_file_into_file_list_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int add_file_into_file_list_cb(HTTPS_CB *pkt)
{
#if 0
    char *query_str="{\n" \
                    "  \"FileType\" : \"MUSIC\"," \
                    "  \"FileName\" : \"/sda1/Music/2011-01-14 黃小琥 - 如果能… 重來/01 愛情開箱文.mp3\"\n" \
                    "}";
#else
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
#endif
    char usb_path[32];
    char file_type[16];
    char file_list_path[64];
    char file_name[256];
    char *ptr;
    bool result;
    int index;

    if(NULL == pkt)
    {
        return -1;
    }

    result = FALSE;
    ptr = NULL;

    if(NULL != query_str)
    {
        get_json_string_from_query(query_str, file_name, "FileName");
		printf("[app_agentd] AddFileIntoFileList - Add %s into ", file_name);

		if('/' == file_name[0])
		{
			ptr = strchr(&file_name[1], '/') + 1; /* Skip the first '/' character */

			// 20130913 liwei: The guest path need skip the '/' more once.
			// file_name:/usb_guest/guest2/Music/2011-01-14 黃小琥 - 如果能… 重來/01 愛情開箱文.mp3
			// 20130923 liwei: The new format is /usb_admin/sda1/Music/XXX
			if(strncmp(&file_name[1],"usb_admin",9)==0)
			{
				ptr = strchr(ptr, '/') + 1;
			}
		}
		else
		{
			ptr = strchr(file_name, '/') + 1; /* Skip the first '/' character */

			// 20130913 liwei: The guest path need skip the '/' more once.
			// file_name:usb_guest/guest2/Music/2011-01-14 黃小琥 - 如果能… 重來/01 愛情開箱文.mp3
			// 20130923 liwei: The new format is usb_admin/sda1/Music/XXX
			if(strncmp(file_name,"usb_admin",9)==0)
			{
				ptr = strchr(ptr, '/') + 1;
			}
		}

        if(NULL != ptr)
        {
            snprintf(usb_path, ptr - file_name + 1, "%s", file_name);

            if(get_json_string_from_query(query_str, file_type, "FileType"))
            {
                result = FALSE;

                index = get_file_type(file_type);

                sprintf(file_list_path, "%s%s%s/%s", WEB_HOME_PATH, usb_path, VENDOR_CONFIG_FOLDER, file_type_info[index].file_list_name);
				printf("%s ... ", file_list_path);
                if(sysutil_check_file_existed(file_list_path))
                {
                    SYSTEM("echo \"%s\" >> %s", file_name, file_list_path);
					printf("Ok \n");
#if 1//def APP_DEBUG
					printf("[app_agentd] cat file contents - %s \n ", file_list_path);
					//SYSTEM("cat \"%s\" | grep \"%s\"", file_list_path, file_name);
#endif
                    result = TRUE;
                }
				else
				{
					printf("Failed \n");
				}
            }
        }
    }

    send_simple_response(pkt, (TRUE == result)?OK_STR:ERROR_STR);

    return 0;
}

/*****************************************************************
* NAME:    rename_file_in_file_list_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int rename_file_in_file_list_cb(HTTPS_CB *pkt)
{
#if 0
    char *query_str="{\n" \
                    "  \"FileType\" : \"MUSIC\"," \
                    "  \"OldName\" : \"/sda1/Music/2011-01-14 黃小琥 - 如果能… 重來/01 愛情開箱文.mp3\"\n" \
                    "  \"NewName\" : \"/sda1/Music/2011-01-14 黃小琥 - 如果能… 重來/01 愛情開箱文.mp3\"\n" \
                    "}";
#else
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
#endif
    char usb_path[32];
    char file_type[16];
    char file_list_path[64];
    char old_name[256];
    char new_name[256];
    char *ptr;
    char *result_ptr;
    bool result;
    int index;
    FILE *fp_file, *fp_tmp;
    char buf[256];
    int dev_num=0;

    if(NULL == pkt)
    {
        return -1;
    }

    result = FALSE;
    result_ptr = ERROR_STR;
    ptr = NULL;
    fp_file = NULL;
    fp_tmp = NULL;

    if(NULL != query_str)
    {
        result = get_json_string_from_query(query_str, old_name, "OldName");
        result &= get_json_string_from_query(query_str, new_name, "NewName");

        if(TRUE == result)
        {
            if(strlen(old_name) ==  strlen(new_name))
            {
                if(0 == strcmp(old_name, new_name))
                {
                    result_ptr = ERROR_THE_SAME_FILE_NAME_STR;
                    result = FALSE;
                }
            }
        }

        if(TRUE == result)
        {
            if('/' == old_name[0])
            {
                ptr = strchr(&old_name[1], '/') + 1; /* Skip the first '/' character */

				// 20130913 liwei: The guest path need skip the '/' more once.
				// file_name:/usb_guest/guest2/Music/2011-01-14 黃小琥 - 如果能… 重來/01 愛情開箱文.mp3
				// 20130923 liwei: The new format is /usb_admin/sda1/Music/XXX
				if(strncmp(&old_name[1],"usb_admin",9)==0)
				{
					ptr = strchr(ptr, '/') + 1;
				}
            }
            else
            {
                ptr = strchr(old_name, '/') + 1; /* Skip the first '/' character */

				// 20130913 liwei: The guest path need skip the '/' more once.
				// file_name:usb_guest/guest2/Music/2011-01-14 黃小琥 - 如果能… 重來/01 愛情開箱文.mp3
				// 20130923 liwei: The new format is usb_admin/sda1/Music/XXX
				if(strncmp(old_name,"usb_admin",9)==0)
				{
					ptr = strchr(ptr, '/') + 1;
				}
            }

            if(NULL != ptr)
            {
                snprintf(usb_path, ptr - old_name + 1, "%s", old_name);
                if('/' == usb_path[strlen(usb_path) - 1])
                {
                    usb_path[strlen(usb_path) - 1] = '\0';
                }

                if(get_json_string_from_query(query_str, file_type, "FileType"))
                {
                    index = get_file_type(file_type);

                    sprintf(file_list_path, "%s%s/%s/%s", WEB_HOME_PATH, usb_path, VENDOR_CONFIG_FOLDER, file_type_info[index].file_list_name);

                    if(sysutil_check_file_existed(file_list_path))
                    {
#if 0
                        SYSTEM("exec < \"%s\"; while read line; do if [ \"$line\" == \"%s\" ];" \
                                "then echo \"%s\" >> tmp_list_file;" \
                                "else echo $line >> tmp_list_file; fi; done;"
                                "mv tmp_list_file \"%s\"",
                                file_list_path, old_name, new_name, file_list_path);
                        //SYSTEM("echo \"%s\" >> %s", file_name, file_list_path);
                        result_ptr = OK_STR;
#else
                        if(NULL != (fp_file = fopen(file_list_path, "r")))
                        {
                            if(NULL != (fp_tmp = fopen("/tmp/tmp_file_list", "w")))
                            {
                                while(NULL != fgets(buf, sizeof(buf), fp_file))
                                {
                                    /* -1 is used to skip the character '\n' */
                                    if((strlen(old_name) == strlen(buf)-1) && (0 == strncmp(old_name, buf, strlen(old_name))))
                                    {
                                        fprintf(fp_tmp, "%s\n", new_name);
                                    }
                                    else
                                    {
                                        fprintf(fp_tmp, "%s", buf);
                                    }
                                }

                                fclose(fp_tmp);
                            }

                            fclose(fp_file);
                        }

                        SYSTEM("mv /tmp/tmp_file_list %s", file_list_path);

                        result_ptr = OK_STR;
#endif
                    }
                }
            }
        }
    }

    send_simple_response(pkt, result_ptr);

    return 0;
}

/*****************************************************************
* NAME:    delete_file_from_file_list_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int delete_file_from_file_list_cb(HTTPS_CB *pkt)
{
#if 0
    char *query_str="{\n" \
                    "  \"FileType\" : \"MUSIC\"," \
                    "  \"Filename\" : \" \
                    "  [" \
                    "     \"/sda1/Music/2011-01-14 黃小琥 - 如果能… 重來/01 愛情開箱文.mp3\",\n" \
                    "     \"/sda1/Music/2011-01-14 黃小琥 - 如果能… 重來/02 重來.mp3\"\n" \
                    "  ]" \
                    "}";
#else
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
#endif
    char usb_path[32];
    char file_type[16];
    char file_list_path[64];
    char file_name[128][256];
    char *ptr;
    bool result;
    int index, i;
    char filename[256];

    memset(filename, 0x0, sizeof(filename));
	memset(file_name, 0x0, sizeof(file_name));

    if(NULL == pkt)
    {
        return -1;
    }

    result = FALSE;
    ptr = NULL;

    if(NULL != query_str)
    {
        if(pkt->json)
        {
            result = parse_delete_files_name_json_cb(query_str, file_name);
        }

        /* Get the path of USB device. */
        if('/' == file_name[0][0])
        {
            ptr = strchr(&file_name[0][1], '/') + 1; /* Skip the first '/' character */

			// 20130913 liwei: The guest path need skip the '/' more once.
			// file_name:/usb_guest/guest2/Music/2011-01-14 黃小琥 - 如果能… 重來/01 愛情開箱文.mp3
			// 20130923 liwei: The new format is /usb_admin/sda1/Music/XXX
			if(strncmp(&file_name[0][1],"usb_admin",9)==0)
			{
				ptr = strchr(ptr, '/') + 1;
			}
        }
        else
        {
            ptr = strchr(file_name[0], '/') + 1; /* Skip the first '/' character */

			// 20130913 liwei: The guest path need skip the '/' more once.
			// file_name:usb_guest/guest2/Music/2011-01-14 黃小琥 - 如果能… 重來/01 愛情開箱文.mp3
			// 20130923 liwei: The new format is usb_admin/sda1/Music/XXX
			if(strncmp(file_name[0],"usb_admin",9)==0)
			{
				ptr = strchr(ptr, '/') + 1;
			}
        }

        if(NULL != ptr)
        {
            snprintf(usb_path, ptr - &(file_name[0][0]) + 1, "%s", &(file_name[0][0]));

            if(get_json_string_from_query(query_str, file_type, "FileType"))
            {
                index = get_file_type(file_type);

                sprintf(file_list_path, "%s%s%s/%s", WEB_HOME_PATH, usb_path, VENDOR_CONFIG_FOLDER, file_type_info[index].file_list_name);

                if(sysutil_check_file_existed(file_list_path))
                {
                    i = 0;
                    while(strlen(file_name[i]))
                    {
                        check_escape_character(file_name[i], filename);
                        SYSTEM("cat %s | grep -v \"^%s$\" > /tmp/tmp_list; mv /tmp/tmp_list %s",
                                file_list_path, filename, file_list_path);

#if 0
                        if(strchr(file_name[i], '\''))
                        {
                            sysCheckEscapeCharacter(file_name[i], filename);
                            SYSTEM("sed /^%s$/d -i %s", file_name[i], file_list_path);
                        }
                        else
                        {
                            SYSTEM("sed '\\#^%s$#d' -i %s", file_name[i], file_list_path);
                        }
#endif

                        i++;
                    }

                    result = TRUE;
                }
            }
        }
    }
    send_simple_response(pkt, (TRUE == result)?OK_STR:ERROR_STR);

    return 0;
}

/*****************************************************************
* NAME:    get_file_list_under_folder_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_file_list_under_folder_cb(HTTPS_CB *pkt)
{
#if 0
    char *query_str="{\n" \
                    "  \"CurrentFolderPath\" : \"/sda1/[2012年秋季日劇] PRICELESS 第1-10集\"\n" \
                    "}";
#else
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
#endif
    char current_folder[128];
    char current_folder_path[256];
    char buf[8];
    char file_type[16], file_name[256];
    int folder_index, file_index;
    bool result;

    if(NULL == pkt)
    {
        return -1;
    }

    result = FALSE;
    memset(current_folder_path, 0x00, sizeof(current_folder_path));

    if(NULL != query_str)
    {
        /* Remove previous file list */
        if(sysutil_check_file_existed(CURRENT_FOLDER_FILE_LIST_PATH))
        {
            SYSTEM("rm -rf %s", CURRENT_FOLDER_FILE_LIST_PATH);
        }

        result = get_json_string_from_query(query_str, current_folder, "CurrentFolderPath");

        if(TRUE == result)
        {
            /* Make sure the current folder is one of the partitions. */
            if((strlen(current_folder) > 2) && 0 == strncmp(USB_ADMIN_FOLDER, current_folder, 10))
            {
                sprintf(current_folder_path, "%s%s%s", WEB_HOME_PATH, ('/' == current_folder[0])?"":"/", current_folder);
                if(sysutil_check_file_existed(current_folder_path))
                {
                    /* Check if the path is a folder. */
                    sysutil_interact(buf, sizeof(buf), "test -d \"%s\" && echo 1 || echo 0", current_folder_path);

                    if(1 == atoi(buf))
                    {
                        SYSTEM("ls -alh \"%s\" > %s", current_folder_path, CURRENT_FOLDER_FILE_LIST_PATH);

                        if(pkt->json)
                        {
                            get_file_list_under_folder_json_cb(pkt);

                            return 1;
                        }
                    }
                }
            }
        }
    }

    send_simple_response(pkt, ERROR_STR);

    return 0;

}

/*****************************************************************
* NAME:    get_file_list_under_folder_in_file_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_file_list_under_folder_in_file_cb(HTTPS_CB *pkt)
{
#if 0
    char *query_str="{\n" \
                    "  \"CurrentFolderPath\" : \"/sda1/[2012年秋季日劇] PRICELESS 第1-10集\"\n" \
                    "}";
#else
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
#endif
    char current_folder[128];
    char current_folder_path[256];
    char buf[8];
    char file_type[16], file_name[256];
    int folder_index, file_index;
    bool result;

    if(NULL == pkt)
    {
        return -1;
    }

    result = FALSE;
    memset(current_folder_path, 0x00, sizeof(current_folder_path));

    if(NULL != query_str)
    {
        /* Remove previous file list */
        if(sysutil_check_file_existed(FOLDER_FILE_LIST_PATH))
        {
            SYSTEM("rm -rf %s", FOLDER_FILE_LIST_PATH);
        }

        result = get_json_string_from_query(query_str, current_folder, "CurrentFolderPath");
        if(TRUE == result)
        {
            /* Make sure the current folder is one of the partitions. */
            if((strlen(current_folder) > 2) && 0 == strncmp(USB_ADMIN_FOLDER, current_folder, 10))
            {
                sprintf(current_folder_path, "%s%s%s", WEB_HOME_PATH, ('/' == current_folder[0])?"":"/", current_folder);
                if(sysutil_check_file_existed(current_folder_path))
                {
                    /* Check if the path is a folder. */
                    sysutil_interact(buf, sizeof(buf), "test -d \"%s\" && echo 1 || echo 0", current_folder_path);
                    if(1 == atoi(buf))
                    {
                        SYSTEM("ls -alh \"%s\" > %s", current_folder_path, FOLDER_FILE_LIST_PATH);
                        usleep(500);

                        if(sysutil_check_file_existed(FOLDER_FILE_LIST_PATH))
                        {
                            if(pkt->json)
                            {
                                get_file_list_under_folder_in_file_json_cb(pkt);
                                return 1;
                            }
                        }
                    }
                }
            }
        }
    }

    send_simple_response(pkt, ERROR_STR);

    return 0;
}

/*****************************************************************
* NAME:    get_folder_path_by_file_name_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_folder_path_by_file_name_cb(HTTPS_CB *pkt)
{
#if 0
    char *query_str="{\n" \
                    "  \"UsbPath\" : \"/sda1"\n" \
                    "  \"FileName\" : \"\"\n" \
                    "}";
#else
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
#endif
    char *response;
    char usb_path[32], usb_full_path[32];
    char file_name[256];
    char folder_path[1024*8];
    int folder_index, file_index;
    bool result;

    response = ERROR_STR;
    memset(folder_path, 0x00, sizeof(folder_path));

    if(NULL == pkt)
    {
        goto out;
    }

    result = FALSE;

    if(NULL == query_str)
    {
        goto out;
    }
    else
    {
        result = get_json_string_from_query(query_str, usb_path, "UsbPath");
        sprintf(usb_full_path, "%s%s%s", WEB_HOME_PATH, ('/' == usb_path[0])?"":"/", usb_path);

        if((FALSE == result) || (!sysutil_check_file_existed(usb_full_path)))
        {
            response = ERROR_USB_PATH_STR;
            goto out;
        }

        result = get_json_string_from_query(query_str, file_name, "FileName");
        if(FALSE != result)
        {
                sysutil_interact(folder_path, sizeof(folder_path), "find \"%s\" -name \"%s\"", usb_full_path, file_name);
                if(0 == strcmp(folder_path, "---"))
                {
                    response = ERROR_FILE_NOT_EXISTED_STR;
                    goto out;
                }
                if(pkt->json)
                {
                    get_folder_path_by_file_name_json_cb(pkt, result, folder_path);
                }
        }
    }

    return 0;
out:
    send_simple_response(pkt, response);
    return -1;
}

/*****************************************************************
* NAME:    delete_file_by_name_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int delete_file_by_name_cb(HTTPS_CB *pkt)
{
#if 0
    char *query_str="{\n" \
                    "  \"Filename\" : " \
                    "  [ " \
                    "    \"11111\",\n" \
                    "    \"11112\",\n" \
                    "    \"11113\",\n" \
                    "    \"11114\",\n" \
                    "    \"11115\"\n" \
                    "  ], " \
                    "  \"Folder\" : \"/sda1\",\n" \
                    "}";
#else
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
#endif

    char filename[128][128];
    char broken_file_name[128][256];
    char folder[512];
    char full_filename[128][1024];
    char *result_str;
    bool result;
    int i, j;

    if(NULL == pkt)
    {
        return -1;
    }

    // The default value in return packet is ERROR.
    result = FALSE;
    result_str = ERROR_STR;
    i = 0, j = 0;
    memset(filename, 0x00, sizeof(filename));
    memset(broken_file_name, 0x00, sizeof(broken_file_name));
    memset(full_filename, 0x00, sizeof(full_filename));

    if(query_str)
    {
        if(pkt->json)
        {
            result = parse_delete_file_name_json_cb(query_str, filename, folder);
        }

        if(TRUE == result)
        {

            /* Check if file is existed. */
            while(strlen(filename[i]))
            {
                if('\0' == folder[0])
                {
                    sprintf(full_filename[i], "%s/%s",
                            WEB_HOME_PATH,
                            filename[i]);
                }
                else
                {
                    sprintf(full_filename[i], "%s%s%s%s%s",
                            WEB_HOME_PATH,
                            ('/' == folder[0])?"":"/",
                            folder,
                            ('/' == folder[strlen(folder)-1])?"":"/",
                            filename[i]);

                }

                if(!sysutil_check_file_existed(full_filename[i]))
                {
                    result = FALSE;
                    result_str = ERROR_FILE_NOT_EXISTED_STR;
                    break;
                }

                i++;
            }

            /* Remove file. */
            if(TRUE == result)
            {
                i = 0, j = 0;

                while(strlen(full_filename[i]))
                {
                    SYSTEM("rm -rf \"%s\"", full_filename[i]);

                    if(!sysutil_check_file_existed(full_filename[i]))
                    {
                        result_str = OK_STR;
                    }
                    else
                    {
                        result_str = ERROR_STR;
                        sprintf(broken_file_name[j], "%s", filename[i]);

                        j++;
                    }

                    i++;
                }
            }
        }
    }

    /* Send response packet */
    delete_file_response_json_cb(pkt, result_str, broken_file_name);

    return 0;
}

/*****************************************************************
* NAME:    delete_file_by_file_name_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int delete_file_by_file_name_cb(HTTPS_CB *pkt)
{
#if 0
    char *query_str="{\n" \
                    "  \"Filename\" : " \
                    "  [" \
                    "    \"/sda1/123.txt\",\n" \
                    "    \"/sda1/Music/123.txt\",\n" \
                    "    \"/sda1/video/123.txt\",\n" \
                    "    \"/sda1/Pic/123.txt\"\n" \
                    "  ]" \
                    "}";
#else
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
#endif
    char filename[128][256];
    char broken_file_name[128][256];
    char full_filename[128][1024];
    char *result_str;
    bool result;
    int i, j;

    if(NULL == pkt)
    {
        return -1;
    }

    // The default value in return packet is ERROR.
    result = FALSE;
    result_str = ERROR_STR;
    i = 0, j = 0;
    memset(filename, 0x00, sizeof(filename));
    memset(broken_file_name, 0x00, sizeof(broken_file_name));
    memset(full_filename, 0x00, sizeof(full_filename));

    if(query_str)
    {
        if(pkt->json)
        {
            result = parse_delete_files_name_json_cb(query_str, filename);
        }

        if(TRUE == result)
        {
            /* Check if file is existed. */
            while(strlen(filename[i]))
            {
                sprintf(full_filename[i], "%s%s%s",
                        WEB_HOME_PATH,
                        ('/' == filename[i][0])?"":"/",
                        filename[i]);

                if(!sysutil_check_file_existed(full_filename[i]))
                {
                    result = FALSE;
                    result_str = ERROR_FILE_NOT_EXISTED_STR;
                    break;
                }

                i++;
            }

            /* Remove file. */
            if(TRUE == result)
            {
                i = 0, j = 0;

                while(strlen(full_filename[i]))
                {
                    SYSTEM("rm -rf \"%s\"", full_filename[i]);

                    if(!sysutil_check_file_existed(full_filename[i]))
                    {
                        result_str = OK_STR;
                    }
                    else
                    {
                        result_str = ERROR_STR;
                        sprintf(broken_file_name[j], "%s", filename[i]);

                        j++;
                    }

                    i++;
                }
            }
        }
    }

    /* Send response packet */
    delete_file_response_json_cb(pkt, result_str, broken_file_name);

    return 0;
}

/*****************************************************************
* NAME:    edit_filename_by_name_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int edit_filename_by_name_cb(HTTPS_CB *pkt)
{
#if 0
    char *query_str="{\n" \
                    "  \"OldName\" : \"123\",\n" \
                    "  \"NewName\" : \"123.txt\",\n" \
                    "  \"Folder\" : \"/sda1\",\n" \
                    "}";
#else
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
#endif

    char old_name[128];
    char new_name[128];
    char folder[128];
    char full_old_name[512];
    char full_new_name[512];
    char *result_ptr;
    bool result;
    int len_old_name, len_new_name;

    if(NULL == pkt)
    {
        return -1;
    }

    // The default value in return packet is ERROR.
    result = FALSE;
    result_ptr = ERROR_STR;

    if(query_str)
    {
        if(pkt->json)
        {
            result = parse_edit_file_name_json_cb(query_str, old_name, new_name, folder);
        }
        if(TRUE == result)
        {
            if(strlen(old_name) ==  strlen(new_name))
            {
                if(0 == strcmp(old_name, new_name))
                {
                    result_ptr = ERROR_THE_SAME_FILE_NAME_STR;
                    result = FALSE;
                }
            }
        }

        if(TRUE == result)
        {
            /* Reset the result flag. */
            result = FALSE;

            sprintf(full_old_name, "%s%s%s%s%s",
                    WEB_HOME_PATH,
                    ('/' == folder[0])?"":"/",
                    folder,
                    ('/' == folder[strlen(folder)-1])?"":"/",
                    old_name);
            if(sysutil_check_file_existed(full_old_name))
            {
                sprintf(full_new_name, "%s%s%s%s%s",
                        WEB_HOME_PATH,
                        ('/' == folder[0])?"":"/",
                        folder,
                        ('/' == folder[strlen(folder)-1])?"":"/",
                        new_name);
                /* Do not change the file name if the file with the new name exists. */
                if(FALSE == sysutil_check_file_existed(full_new_name))
                {
					// 20130930 liwei: Use rename() to fix escape character '`', '$' issue.
					// Return 0 is successful in rename().
                    if(rename(full_old_name, full_new_name))
                    {
						// If rename() failed, use mv.
	                    SYSTEM("mv \"%s\" \"%s\"", full_old_name, full_new_name);
                    }

                    if(sysutil_check_file_existed(full_new_name))
                    {
                        result = TRUE;
                        result_ptr = OK_STR;
                    }
                }
                else
                {
                    result_ptr = ERROR_FILE_EXISTED_STR;
                }
            }
            else
            {
                result_ptr = ERROR_FILE_NOT_EXISTED_STR;
            }
        }
    }

    /* Send response packet */
    send_simple_response(pkt, result_ptr);

    return 0;
}

/*****************************************************************
* NAME:    add_file_into_favorite_list_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int add_file_into_favorite_list_cb(HTTPS_CB *pkt)
{
    bool result;
    char file_name[256];

    memset(file_name, 0x00, sizeof(file_name));

    result = add_file_into_specific_list_cb(pkt, USB_FAVORITE_LIST, file_name);

    send_simple_response(pkt, (TRUE == result)?OK_STR:ERROR_STR);

    return 0;
}

/*****************************************************************
* NAME:    delete_file_from_favorite_list_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int delete_file_from_favorite_list_cb(HTTPS_CB *pkt)
{
    bool result;

    result = delete_file_from_specific_list_cb(pkt, USB_FAVORITE_LIST);

    return 0;
}

/*****************************************************************
* NAME:    add_file_into_public_list_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int add_file_into_public_list_cb(HTTPS_CB *pkt)
{
    bool result;
    char *result_str;
#if HAS_ENGENIUS_DDNS
    char buf[8];
    char ddns_link[32];
    char public_link[512];
#endif
    char file_name[512];

    result_str = ERROR_STR;
    memset(file_name, 0x00, sizeof(file_name));

    result = add_file_into_specific_list_cb(pkt, USB_PUBLIC_LIST, file_name);
    if(strlen(file_name) > MAX_USB_FILE_NAME_LEN)
    {
        result = FALSE;
        result_str = ERROR_FILENAME_LENGTH_STR;
    }

#if 0//HAS_ENGENIUS_DDNS   //todo...public functoin is remove from Enshare now
    if((TRUE == result) && (1 == apCfgGetIntValue(DDNS_ENABLE_TOK)))
    {
        result_str = ERROR_NOT_SUPPORT_STR;

        if(sysutil_check_file_existed(ENGENIUS_DDNS_STATUS_PATH))
        {
            memset(buf, 0x00, sizeof(buf));

            sysutil_interact(buf, sizeof(buf), "cat %s", ENGENIUS_DDNS_STATUS_PATH);

            if(0 == strncmp(OK_STR, buf, strlen(OK_STR)))
            {

                if(0 != strlen(apCfgGetStrValue(DDNS_AG_DOMAINNAME_TOK))) // user definied
                {
                    sprintf(ddns_link, "%s.%s", apCfgGetStrValue(DDNS_AG_DOMAINNAME_TOK), ENGENIUS_DDNS_DOMAIN_NAME);
                }
                else // default
                {
                    sprintf(ddns_link, "%s.%s", GetEngeniusDdnsHost(), ENGENIUS_DDNS_DOMAIN_NAME);
                }

                sprintf(public_link, "http://%s:%d%s%s",
                        ddns_link, apCfgGetIntValue(LIGHTTPD_PORT_TOK),
                        ('/' == file_name[0])?"":"/", file_name);

                result_str = OK_STR;
            }
        }
    }

    add_file_into_public_list_json_cb(pkt, result_str, public_link);
#else
    send_simple_response(pkt, (TRUE == result)?OK_STR:ERROR_STR);
#endif
    return 0;
}

/*****************************************************************
* NAME:    delete_file_from_public_list_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int delete_file_from_public_list_cb(HTTPS_CB *pkt)
{
    bool result;

    result = delete_file_from_specific_list_cb(pkt, USB_PUBLIC_LIST);

    return 0;
}

/*****************************************************************
* NAME:    reload_downsized_picture_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int reload_downsized_picture_cb(HTTPS_CB *pkt)
{
#if 0
    char *query_str="{\n" \
                    "  \"UsbPath\" : \"/sda1\"\n" \
                    "}";
#else
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
#endif
    bool result;
    char *result_str;
    char mount_point[64];
    char usb_path[32], usb_path_tmp[32];
    char file_list_path[128];
    char buf[256];

    if(NULL == pkt)
    {
        return -1;
    }

    result = FALSE;
    result_str = ERROR_STR;

    result = get_json_string_from_query(query_str, usb_path_tmp, "UsbPath");
    sprintf(usb_path, "%s", usb_path_tmp + (('/' == usb_path_tmp[0])?1:0));

    if(TRUE == result)
    {
        sprintf(mount_point, "%s/%s", WEB_HOME_PATH, usb_path);

        sprintf(file_list_path, "%s/%s/"PICTURE_FILE_LIST_NAME, mount_point, VENDOR_CONFIG_FOLDER);
        if(sysutil_check_file_existed(file_list_path))
        {
            /* genthumbnail.sh --mount /tmp/usb --path sda1 --vendor .EnGenius -p /tmp/usb/sda1/picture_list */
            if(FINDPROC("genthumbnail.sh"))
            {
                result_str = ERROR_PROCESS_IS_RUNNING_STR;
            }
            else
            {
                SYSTEM("genthumbnail.sh --mount %s --path %s --vendor .%s -p %s &",
                        WEB_HOME_PATH, usb_path, VENDOR_NAME, file_list_path);
                result_str = OK_STR;
            }
        }
    }

    send_simple_response(pkt, result_str);
    return 0;
}
/*****************************************************************
* NAME:    get_music_information_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_music_information_cb(HTTPS_CB *pkt)
{
    if(NULL != pkt)
    {
        /* TODO : Parse the start index and desired amount. Then generate the music information. */
        if(pkt->json)
        {
            get_music_information_json_cb(pkt);
        }
    }
    return 0;
}

/*****************************************************************
* NAME:    search_file_by_name_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int search_file_by_name_cb(HTTPS_CB *pkt)
{
#if 0
    char *query_str="{\n" \
                    "  \"UsbPath\" : \"/sda1\"\n," \
                    "  \"FileName\" : \"123\"\n" \
                    "}";
#else
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
#endif
    bool result;
    char mount_point[64];
    char usb_path[32], usb_path_tmp[32];
    char file_name[128];
    char file_names[1024*8];

    if(NULL == pkt)
    {
        return -1;
    }

    result = FALSE;

    //sysconf_util_cmd("inform usb-mount-point ,", mount_point);

    result = get_json_string_from_query(query_str, usb_path_tmp, "UsbPath");

    sprintf(usb_path, "%s", usb_path_tmp + (('/' == usb_path_tmp[0])?1:0));

    result &= get_json_string_from_query(query_str, file_name, "FileName");

    if(TRUE == result)
    {
        result = FALSE;

        sprintf(mount_point, "%s/%s", WEB_HOME_PATH, usb_path);

#if 1 //def APP_DEBUG
		printf("[app_agentd] list directory contents - %s \n ", mount_point);
		SYSTEM("ls \"%s\"", mount_point);
#endif 

		printf("[app_agentd] SearchFileByName - %s/%s ", mount_point, file_name);
        if(sysutil_check_file_existed(mount_point))
        {
            sysutil_interact(file_names, sizeof(file_names), "cd %s;find ./%s -type f -name \"*%s*\"", WEB_HOME_PATH, usb_path, file_name);

            if(0 == strcmp(file_names, "---") || 0 == strcmp(file_names, ""))
            {
                memset(file_names, 0x00, sizeof(file_names));
                printf("is not existed. \n");
            }
            else
            {
                printf("is existed. \n");
            }
            result = TRUE;
        }
		else
		{
			printf("is not existed. \n");
		}
    }

    if(pkt->json)
    {
        search_file_by_name_json_cb(pkt, result, file_names);
    }
    return 0;
}

/*****************************************************************
* NAME:    create_folder_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int create_folder_cb(HTTPS_CB *pkt)
{
#if 0
    char *query_str="{\n" \
                    "  \"UsbPath\" : \"123\",\n" \
                    "  \"FolderName\" : \"123\"\n" \
                    "}";
#else
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
#endif
    bool result;
    char *return_str;
    char buf[4];
    char usb_path[32], usb_path_tmp[32];
    char folder_name[128], full_folder_path[256];

    if(NULL == pkt)
    {
        return -1;
    }

    result = FALSE;
    return_str = ERROR_STR;

    result = get_json_string_from_query(query_str, usb_path_tmp, "UsbPath");

    sprintf(usb_path, "%s", usb_path_tmp + (('/' == usb_path_tmp[0])?1:0));

    result &= get_json_string_from_query(query_str, folder_name, "FolderName");

    if(TRUE == result)
    {
        result = FALSE;

        sprintf(full_folder_path, "%s/%s/%s", WEB_HOME_PATH, usb_path, folder_name);

        if(sysutil_check_file_existed(full_folder_path))
        {
            /* Check if the path is a folder. */
            sysutil_interact(buf, sizeof(buf), "test -d \"%s\" && echo 1 || echo 0", full_folder_path);

            if(1 == atoi(buf))
            {
                return_str = ERROR_FILE_EXISTED_STR;
            }
            else if(0 == atoi(buf))
            {
                return_str = ERROR_THE_SAME_FILE_NAME_STR;
            }
        }
        else
        {
            SYSTEM("mkdir -p \"%s\"", full_folder_path);

            usleep(100);

            if(sysutil_check_file_existed(full_folder_path))
            {
                return_str = OK_STR;
            }
        }
    }

    send_simple_response(pkt, return_str);
    return 0;
}



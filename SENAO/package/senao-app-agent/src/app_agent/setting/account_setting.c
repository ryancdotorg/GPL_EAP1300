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
;    Creator : Robert Hong
;    File    : account_setting.c
;    Abstract:
;
;       Modification History:
;       By              Date        Ver.    Modification Description
;       --------------- --------    -----   -------------------------------------
;       Robert          2014/05/14          First commit
;****************************************************************************/

/*--------------------------------------------------------------------------*/
/*                           INCLUDE HEADER FILES                           */
/*--------------------------------------------------------------------------*/
#include "account_setting.h"
#include "app_agent.h"
#include "deviceinfo.h"
#include "json.h"

#define MAX_ACCOUNT_NAME_LEN 12
#define MAX_ACCOUNT_PW_LEN 12

void get_all_account(ACCOUNT_SETTING_T account_array[])
{
	char response[128]={0}, name[13]={0}, authority[5]={0};
	char *ptr=NULL, *pstr=NULL;
	int i=1;

	sysutil_interact(name, sizeof(name), "/lib/auth.sh get_alluser");
	if(NULL != (ptr = strchr(name, '\n')))
		*ptr = '\0';
	strcpy(account_array[0].user_name, name);

	sysutil_interact(authority, sizeof(authority), "/lib/auth.sh get_authority %s", name);
	if(NULL != (ptr = strchr(authority, '\n')))
		*ptr = '\0';
	account_array[0].privilege_level = atoi(authority);


	sysutil_interact(response, sizeof(response), "/lib/ipcam_auth.sh get_alluser");
	if(NULL != (ptr = strchr(response, '\n')))
		*ptr = '\0';

	pstr = strtok(response, " ");
	while (pstr != NULL)
	{
		strcpy(account_array[i].user_name, pstr);
		sysutil_interact(authority, sizeof(authority), "/lib/ipcam_auth.sh get_authority %s", pstr);
		//0=Administrator, 1=Viewer, 2=Onvif
		if (i==1)    // onvif account
		{
			account_array[i].privilege_level = 2;
		}
		else
		{
			account_array[i].privilege_level = atoi(authority);
		}
		pstr = strtok(NULL, " ");
		i++;
	}
}

/*****************************************************************
* NAME:    get_user_list_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int get_user_list_cb(HTTPS_CB *pkt)
{
	ACCOUNT_SETTING_T account_array[MAX_WEB_ACCOUNTS_NUM];
	memset(account_array, 0, sizeof(account_array));

	get_all_account(account_array);

	if(pkt->json)
	{
		get_user_list_json_cb(pkt, account_array);
	}

    return 0;
}

/*****************************************************************
* NAME:    update_user_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int update_user_cb(HTTPS_CB *pkt)
{
	char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
	char *return_str = ERROR_STR;
	char user_name[32]={0}, password[32]={0}, privilege[32]={0};
	int i, index=99, empty_index=99;
	ACCOUNT_SETTING_T account_array[MAX_WEB_ACCOUNTS_NUM];
	memset(account_array, 0, sizeof(account_array));

	char invalid_name[] = "root nobody daemon ftp network";

	struct json_object *jobj;
	struct json_object *jobj_user;

	if((NULL == pkt) || (NULL == query_str))
		goto send_pkt;

	if((jobj = json_tokener_parse(query_str)))
    {
        jobj_user = json_object_object_get(jobj,"User");
		if((FALSE == senao_json_object_get_string(jobj_user, "Name", user_name))
			|| (FALSE == senao_json_object_get_string(jobj_user, "Password", password))
			|| (FALSE == senao_json_object_get_string(jobj_user, "Privilege", privilege)))
		{
			goto send_pkt;
		}
	}

	if ( (strlen(user_name) > MAX_ACCOUNT_NAME_LEN)
		|| (strlen(password) < 3)
		|| (strlen(password) > MAX_ACCOUNT_PW_LEN)
		|| (strcmp(user_name, "admin")==0 && strcmp(password, "admin")==0)
		|| (strcmp(user_name, "guest")==0 && strcmp(password, "guest")==0) )
	{
		goto send_pkt;
	}

	if (strstr(invalid_name, user_name))
	{
		goto send_pkt;
	}

	get_all_account(account_array);

	for (i=0; i<MAX_WEB_ACCOUNTS_NUM; i++)
	{
		if (strcmp(user_name, account_array[i].user_name)==0)
		{
			index = i;
			break;
		}
		if (strlen(account_array[i].user_name)==0)
		{
			empty_index = i;
			break;
		}
	}

	if (strcmp(privilege, "Administrator")==0)
	{
		if (index==0 || index==99)
		{
			SYSTEM("/lib/auth.sh mod_multi_auth 1 %s %s 0", user_name, password);
			return_str = OK_STR;
		}
	}
	else if (strcmp(privilege, "Onvif")==0)
	{
		if (index==1 || index==99)
		{
			SYSTEM("/lib/ipcam_auth.sh mod_multi_auth 1 %s %s 0", user_name, password);
			return_str = OK_STR;
		}
	}
	else if (strcmp(privilege, "Guest")==0)
	{
		if (index>1 && index<MAX_WEB_ACCOUNTS_NUM)
		{
			SYSTEM("/lib/ipcam_auth.sh mod_multi_auth %d %s %s 1", index, user_name, password);
			return_str = OK_STR;
		}
		else if (index==99 && empty_index<MAX_WEB_ACCOUNTS_NUM)
		{
			SYSTEM("/lib/ipcam_auth.sh set_auth %s %s 1", user_name, password);
			return_str = OK_STR;
		}
	}

send_pkt:
	/* 3. send response packet */
	send_simple_response(pkt, return_str);
	return 0;
}


/*****************************************************************
* NAME:    remove_user_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int remove_user_cb(HTTPS_CB *pkt)
{
	char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
	char *return_str = ERROR_STR;
	char user_name[32]={0};
	int i, numOfLines=0;
	ACCOUNT_SETTING_T account_array[MAX_WEB_ACCOUNTS_NUM];
	memset(account_array, 0, sizeof(account_array));

	struct json_object *jobj;
	struct json_object *jobj_user;

	if((NULL == pkt) || (NULL == query_str))
		goto send_pkt;
	
	if((jobj = json_tokener_parse(query_str)))
    {
		jobj_user = json_object_object_get(jobj, "User");
		if(FALSE == senao_json_object_get_string(jobj_user, "Name", user_name))
		{
			goto send_pkt;
		}
	}

	get_all_account(account_array);
	sysutil_get_file_line_num("/etc/ipcampasswd", &numOfLines);

	// only viewer account can delete
	for (i=2; i<MAX_WEB_ACCOUNTS_NUM; i++)
	{
		if (strcmp(user_name, account_array[i].user_name)==0)
		{
			/* more than two viewer */
			if (numOfLines>2)
			{
				SYSTEM("/lib/ipcam_auth.sh del_user %s", user_name);
				return_str = OK_STR;
				break;
			}
		}
	}
	
send_pkt:
	/* 3. send response packet */
	send_simple_response(pkt, return_str);
	return 0;
}
/*****************************************************************
* NAME:    get_account_password_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
/*int get_account_password_cb(HTTPS_CB *pkt)
{
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
    char buf[64];

    if(NULL == pkt)
        return -1;

    get_json_sc_account_password_cb(pkt, OK_STR);

    return 0;
}*/
/*****************************************************************
* NAME:    set_account_password_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int set_account_password_cb(HTTPS_CB *pkt)
{
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");    
    char *return_str=OK_STR;
    char *ptr1, *ptr2;
    char password[128]={0};
    char username[128]={0};
    char current_password[128]={0};
    char buf[128]={0};
    char buf2[128]={0};

    if(NULL == pkt || 0 == strlen(query_str))
    {
        send_simple_response(pkt,ERROR_STR);
    }

    memset(buf, 0x00, sizeof(buf));
    get_json_string_from_query(query_str, buf, "CurrentPassword");
    sysCheckSingleQuoteOnString(buf, current_password);
    memset(buf, 0x00, sizeof(buf));
    get_json_string_from_query(query_str, buf, "Password");
    sysCheckSingleQuoteOnString(buf, password);
    memset(buf, 0x00, sizeof(buf));
    get_json_string_from_query(query_str, buf, "Username");
    sysCheckSingleQuoteOnString(buf, username);

    memset(buf, 0x00, sizeof(buf));
    memset(buf2, 0x00, sizeof(buf2));

    sysutil_interact(buf2, sizeof(buf2), "echo '%s' > /tmp/tmp_password;md5sum /tmp/tmp_password |awk '{print $1}' | tr -d '\n'", current_password);

    sysutil_interact(buf, sizeof(buf), "cat /etc/webpasswd | awk -F: {'printf $2'} | tr -d '\n'");
//    ptr2 = strrchr(buf, ':');
//    *ptr2 = '\0';
//    ptr1 = strrchr(buf, ':') + 1;
//
//    snprintf(buf, ptr2 - ptr1 + 1, ptr1);

    if (strcmp(buf2, buf)==0)
    {
        send_simple_response(pkt, OK_STR);
        sysutil_interact(buf, sizeof(buf), "echo '%s' > /tmp/tmp_password;md5sum /tmp/tmp_password |awk '{print $1}' | tr -d '\n'", password);
        //APPAGENT_SYSTEM("echo \"admin:%s:root@HUDDLE:\" > /etc/webpasswd", buf);
        APPAGENT_SYSTEM("echo '%s:%s:' > /etc/webpasswd", username, buf);
        if(sysutil_check_file_existed("/etc/config/samba"))
        {
			APPAGENT_SYSTEM("uci set samba.@samba[0].username='%s'", username);
			APPAGENT_SYSTEM("uci set samba.@samba[0].userpassword='%s'", password);
        }
    }
    else
    {
        send_simple_response(pkt,ERROR_PASSWORD_STR);
    }

	if(sysutil_check_file_existed("/etc/config/samba"))
	{
		APPAGENT_SYSTEM("luci-reload auto samba &");
	}
	else
	{
		APPAGENT_SYSTEM("mesh_sync sync_server_conf &");
	}

    return 0;
}

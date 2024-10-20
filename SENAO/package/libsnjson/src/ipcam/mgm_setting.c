#include "common.h"

int json_set_mgm_fw_upgrade(ResponseEntry *rep, char *query_str, char *json_reply)
{
	char buf[512];
	char *mode = NULL;

	ResponseStatus *res = rep->res;

	if(NULL != query_str)
	{
		ResponseEntry *rep = Response_create();
		struct json_object *jobj = NULL;

		if((jobj = jsonTokenerParseFromStack(rep, query_str)))
		{
			senao_json_object_get_and_create_string(rep, jobj, "mode", &mode);
			debug_print("--- mode: %s ----\n" , mode);

			if(strcmp(mode, "upgrade_locally") == 0)
			{
				sys_interact_long(buf, sizeof(buf), "/sbin/checkimg \"/tmp/firmware.img\" | tail -n 1 | tr -d \"\\n\"");

				debug_print("%s, buf: %s", buf, __FUNCTION__);
				if(strcmp(buf, "header: Return OK") == 0)
				{
					//system("echo \"sleep 3;/sbin/sysupgrade /tmp/firmware.img;\" > /tmp/sch_fwupg.sh; chmod 777 /tmp/sch_fwupg.sh; sh /tmp/sch_fwupg.sh &");
					// https://stackoverflow.com/questions/14612371/how-do-i-run-multiple-background-commands-in-bash-in-a-single-line

					system("(sleep 8; /sbin/sysupgrade /tmp/firmware.img) &");

					RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
				}
				else
				{
					system("(sleep 8; reboot -f) &");

					RET_GEN_ERRORMSG(res, API_INTERNAL_ERROR, "Format error");
				}
			}
			else if(strcmp(mode, "upgrade_from_server") == 0)
			{
			}
		}
	}
}

int json_set_mgm_restore_config(ResponseEntry *rep, char *query_str, char *json_reply)
{
	char buf[512];

	ResponseStatus *res = rep->res;

	sys_interact_long(buf, sizeof(buf), "sh /etc/cfgrestore.sh | tr -d \"\\n\"");

	debug_print("--- %s ---, buf: %s\n", __FUNCTION__, buf);
	if(strcmp(buf, "config: OK") == 0)
	{
		system("(sleep 8; /sbin/sysupgrade --restore-backup /tmp/restore.gz) &");

		RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
	}
	else
	{
		RET_GEN_ERRORMSG(res, API_INTERNAL_ERROR, "Format error");
	}
}

int json_set_mgm_reset_to_default(ResponseEntry *rep, char *query_str, char *json_reply)
{
	ResponseStatus *res = rep->res;
// kenny stop record
	system("ipcammgr_cli record stop");
//
	system("(rm -rf /overlay/*; sync; sleep 8; reboot -f) &");

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_set_mgm_reboot(ResponseEntry *rep, char *query_str, char *json_reply)
{
	ResponseStatus *res = rep->res;
// kenny stop record
	system("ipcammgr_cli record stop");
//

	system("(sleep 8; reboot -f) &");

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

#if 0
int json_set_mgm_reset_to_default(ResponseEntry *rep, char *query_str, char *json_reply)
{
	char json_data[MAX_JSON_REPLY_LEN] = {0};
	char cmd[MAX_JSON_REPLY_LEN] = {0};
	char buf[512];

	ResponseStatus *res = rep->res;

	if(NULL != query_str)
	{
		ResponseEntry *rep = Response_create();

		struct json_object *jobj;
		jobj = newObjectFromStack(rep);
		rep->jobj = jobj;

		system("(sleep 8; rm -rf /overlay/*; reboot -f) &");

		json_object_object_add(jobj, "result", json_object_new_string("OK"));

		memcpy(json_reply, json_object_to_json_string(jobj), MAX_JSON_REPLY_LEN);
		Response_destroy(rep);
	}

	// prevent { "mode": "OK", "result": "OK" }
	struct json_object *result_jobj = NULL;
	result_jobj = json_object_new_object();

	if(check_header)
		json_object_object_add(result_jobj, "result", json_object_new_string("OK"));
	else
		json_object_object_add(result_jobj, "result", json_object_new_string("ERROR"));

	memcpy(json_reply, json_object_to_json_string(result_jobj), MAX_JSON_REPLY_LEN);
	json_object_put(result_jobj);
	Response_destroy(rep);
}
#endif

#include <api_common.h>
#include <sys_common.h>
#include <sysCore.h>
// #include <api_wireless.h>
#include <variable.h>
#include <api_lan.h>
#include <api_rainier.h>
// #include <wireless_tokens.h>
// #include <integer_check.h>
#include <json_object.h>
#include <json_tokener.h>
#include <json_rainier.h>
#include <json_common.h>
#include <unistd.h>
#include <api_sys.h>
#include <sqlite3.h>
#include <time.h>
#include <sys/time.h>
#include "database_sqlite3.h"



/* definition */
#define LOG_DB_FILE_PATH	"/root/db_files/log.db"
#define LOG_DB_TABLE		"log"
#define LOG_DB_COLUMNS		"time,log_type,device,event_type,description"

#define NMS_RSSI_DIR			"/tmp/nms_rssi"
#define NMS_RSSI_FILE			"hs_rssi_"
#define NMS_RSSI_FILE_NUM		"hs_rssi_file_num"

#define NMS_HS_FW_DIR			"/tmp/nms_hs_fw"
#define NMS_HS_FW_FILE			"hs_fw_"
#define NMS_HS_FW_FILE_NUM		"hs_fw_file_num"

#define NMS_CALL_INFO_DIR			"/tmp/nms_call_info"
#define NMS_CALL_INFO_BS_FILE		"bs_call_info"
#define NMS_CALL_INFO_HS_FILE		"hs_call_info"
#define NMS_CALL_INFO_BS_INUSE_FILE	"bs_call_inuse"

#define NMS_DEVICE_DIR			"/tmp/nmsDevice"
#define NMS_DEVICE_BS			"BS"

#define AST_DEVICE_DIR          "/tmp/astDevice"
//#define AST_DEVICE_STATUS       "UserStatus"
#define AST_DEVICE_PEER         "peers.info"
#define AST_DEVICE_TRUNK        "registry.info"
#define AST_DEVICE_HINT         "hints.info"


#define DB_BACKUP_FILE_PATH "/www"
#define DB_BACKUP_FILE_PATH_TMP "/root/tmp"
#define DB_BACKUP_FILE_NAME "backup_database.tar.gz"
#define SDCARD_PATH	"/mnt/sda"

#if SUPPORT_SP938BS_OPENAPI_SERVER
	#define SP938BS_PID	"938b"
	#define	SP938BS_LOCAL_FW_FILENAME "/tmp/bs_firmware.img"
	#define	FW_FILENAME_BSC "/tmp/firmware.img"
	#define	FW_FILENAME_BS "/tmp/bs_firmware.img"
	#define	FW_FILENAME_HS "/tmp/hs_firmware.dlf"
#elif SUPPORT_ENBSC_OPENAPI_SERVER
	#define	FW_FILENAME_BSC "/root/tmp/firmware.img"
	#define	FW_FILENAME_BS "/root/tmp/bs_firmware.dlf"
	#define	FW_FILENAME_HS "/root/tmp/hs_firmware.dlf"
#else
	#define	FW_FILENAME_BSC "/tmp/firmware.img"
	#define	FW_FILENAME_BS "/tmp/bs_firmware.dlf"
	#define	FW_FILENAME_HS "/tmp/hs_firmware.dlf"
#endif


//#define DB_RESTORE_TMP_CEHCK	"/tmp/root/db_files"
#define DB_TAR_FILE_CEHCK	"root/db_files/"
#define DB_ROOT_DIRETERY	"/root/db_files"

/* local function */
int getBscId(char *id, int len);
int getBsCount();
int findBsConfigIdxByIndex(int index);
int findBsConfigIndexByMac(char *mac);
void uciClearBsIdxStatus(int bit_pos);
int getSipTrunkCount();
int json_get_rainier_handset_list_idx_2(ResponseEntry *rep, struct json_object *jobj, int hs_idx, int location_in);
int json_get_rainier_handset_list_of_base_idx(ResponseEntry *rep, struct json_object *jobj, int bs_idx, int hs_idx, int location_in, int hs_status_in);
int json_get_rainier_sip_trunk_acc_idx_2(ResponseEntry *rep, struct json_object *jobj, int sip_acc_idx, int trunk_status_in);

static bool sysIsFileExisted(const char *filename);

static sqlite3 *master_db = NULL;

char cdr_start_date[64] = {0};	
char cdr_end_date[64] = {0};
int cdr_select_base = 0;
int cdr_select_handset = 0;
char cdr_search_string[128] = {0}; 


static sqlite3 *cdr_db = NULL;
#define MAX_CDR_LOG_PER_PAGE	20

static int cdr_page = 0;
static int read_out_row = 0;
static int cdr_start_index = 0;
static int cdr_end_index = 0;

static int log_page = 0;
static int log_read_out_row = 0;
static int log_start_index = 0;
static int log_end_index = 0;


static overview_data_t overview_set;

static db_rssi_t *rssi_val = NULL;
static unsigned int rssi_num;
static int rssi_rows = 0;

int ra_print_enbale(void)
{
	FILE *file;
    
    if (file = fopen(RA_DEBUG_PRINT_FILE, "r")) 
    {
        fclose(file);
        return 1;
    }
    
    return 0;
}
void ra_debug_print( const char * format, ... )
{
  char buffer[1024];
  char con_buf[1024];

  va_list args;
  va_start (args, format);
  vsprintf (buffer,format, args);
  //perror (buffer);

  if(ra_print_enbale())
  {
  	snprintf(con_buf, sizeof(con_buf), "echo \"%s\" > /dev/console", buffer);
	system(con_buf);
  }

  va_end (args);
}


int get_call_graph_num(void)
{
	switch(overview_set.range)
	{
		case OVERVIEW_LAST_HOUR:
			return NUM_LAST_HOUR;

		case OVERVIEW_LAST_DAY:
			return NUM_LAST_DAY;

		case OVERVIEW_LAST_WEEK:
			return NUM_LAST_WEEK;
		default:
			break;
	
	}

	return 0;
}

int sys_interact_simple(char *output, int outputlen, char *cmd)
{
	char command[1024];
	int i, c;
	FILE *pipe;
	va_list ap;

	memset(command, 0, sizeof(command));
	sprintf(command, "%s", cmd);

	//debug_print(" Jason DEBUG %s[%d] command[%s] \n", __FUNCTION__, __LINE__, command);

	if((pipe = popen(command, "r")) == NULL)
	{
		goto err;
	}

	for(i = 0; ((c = fgetc(pipe)) != EOF) && (i < outputlen - 1); i++)
	{
		output[i] = (char) c;
	}
	output[i] = '\0';

	pclose(pipe);

	if(strlen(output) == 0)
	{
		goto err;
	}

	return 0;

err:
	strcpy(output, "---");
	return -1;
}
int sys_interact_long(char *output, int outputlen, char *fmt, ...)
{
	char command[1024];
	int i, c;
	FILE *pipe;
	va_list ap;

	memset(command, 0, sizeof(command));

	va_start(ap, fmt);
	vsnprintf(command, sizeof(command), fmt, ap);
	va_end(ap);

	memset(output, 0, outputlen);

	if((pipe = popen(command, "r")) == NULL)
	{
		goto err;
	}

	for(i = 0; ((c = fgetc(pipe)) != EOF) && (i < outputlen - 1); i++)
	{
		output[i] = (char) c;
	}
	output[i] = '\0';

	pclose(pipe);

	if(strlen(output) == 0)
	{
		goto err;
	}

	return 0;

err:
	strcpy(output, "---");
	return -1;
}

#if 0
static int burn_bs_firmware_(char *file_name, char *dev)
{
	char buf[512] = {0};

	snprintf(buf, sizeof(buf), "flash_eraseall %s", dev);

	debug_print("Jason DEBUG %s[%d] call system : [%s]\n ", __FUNCTION__, __LINE__, buf);
	SYSTEM(buf);

	memset(buf, 0 ,sizeof(buf));


	snprintf(buf, sizeof(buf), "flashcp -v %s %s", file_name, dev);

	debug_print("Jason DEBUG %s[%d] call system : [%s]\n ", __FUNCTION__, __LINE__, buf);
	SYSTEM(buf);

}

static int upgrade_firmware_from_bsc(char *file_name)
{
	#define FIRMWARE_FILE_NAME	"openwrt-am335x-sp938bs-norplusnand-fw-s.img"       
	#define ROOTFS_PARTITION	"/dev/mtd4"       
	#define SP938BS_PID	"938b"
	char cmd[512] = {0};
	char buf[512] = {0};
	char bsc_ip[32] = {0};
	int ret = -1;
	char fw_file[512] = {0};
	int retry = 10;
	
     	//snprintf(cmd, sizeof(cmd), "uci get base-station-controller.bsc_0.ip | tr -d \"\\n\"");
        //sys_interact(bsc_ip, sizeof(bsc_ip), cmd);
		api_get_string_option("base-station-controller.bsc_0.ip", bsc_ip, sizeof(bsc_ip));
	memset(cmd, 0, sizeof(cmd));

	debug_print("Jason DEBUG %s[%d] bsc_ip: [%s]\n ", __FUNCTION__, __LINE__, bsc_ip);

	snprintf(cmd, sizeof(cmd), "wget -P /tmp http://%s/%s", bsc_ip, file_name);

	debug_print("Jason DEBUG %s[%d] cmd: [%s]\n ", __FUNCTION__, __LINE__, cmd);

	sys_interact_long(buf, sizeof(buf), cmd);

	debug_print("Jason DEBUG %s[%d] buf: [%s]\n ", __FUNCTION__, __LINE__, buf);


#if 0
	if(strstr(buf, "100%"))
	{
		snprintf(fw_file, sizeof(fw_file), "/tmp/%s", file_name);
		debug_print("Jason DEBUG %s[%d] start burn \n ", __FUNCTION__, __LINE__);
		ret = burn_bs_firmware_(fw_file,  ROOTFS_PARTITION);
	}
#endif
	sleep(10);

	snprintf(fw_file, sizeof(fw_file), "/tmp/%s", file_name);
	debug_print("Jason DEBUG %s[%d] start burn \n ", __FUNCTION__, __LINE__);
#if 0	
	/* burn img without header */
	ret = burn_bs_firmware_(fw_file,  ROOTFS_PARTITION);
#else

	/* check header convert dlf to img, write info to uci config  */
	SYSTEM("rainier_web_fw -m 1 -d %s", fw_file);
	
	/* check product id , if ok, burn it   */
	SYSTEM("rainier_web_fw -m 2 -p %s&", SP938BS_PID);
#endif	

	return ret;

}

int json_get_bs_upgrade_status(ResponseEntry *rep, char *query_str, int idx)
{
	char username[32] = {0};
	char password[32] = {0};

	char bs_ip[32] = {0};

	char bsc_id[32] = {0};
        
	char cmd[1024] = {0};
        char buf[8192] = {0}, cmd_debug[1024] = {0};
        char device_token[1024] = {0};


	if((idx < RAINIER_BSID_MIN) && (idx > RAINIER_BSID_MAX))
        /* incorrect range */
        	RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "BASE ID");        

	if(query_str == NULL)
        	RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");

        if((jobj = jsonTokenerParseFromStack(rep, query_str)) == NULL)
            	RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "JSON");
	
	getBscId(bsc_id, sizeof(bsc_id));
	 
	//debug_print("Jason DEBUG %s[%d] bsc_id [%s] \n", __FUNCTION__, __LINE__, bsc_id);

        snprintf(username, sizeof(username), "%s", bsc_id);
        //snprintf(cmd, sizeof(cmd), "uci get base-station-list.@base-station[%d].bs_key | tr -d \"\\n\"", idx);
        //sys_interact(password, sizeof(password), cmd);
		api_get_string_option2(password, sizeof(password), "base-station-list.@base-station[%d].bs_key", idx);

	//debug_print("Jason DEBUG %s[%d] username [%s] \n", __FUNCTION__, __LINE__, username);
	//debug_print("Jason DEBUG %s[%d] password [%s] \n", __FUNCTION__, __LINE__, password);

        //snprintf(cmd, sizeof(cmd), "uci get base-station-list.@base-station[%d].ip | tr -d \"\\n\"", idx);
        //sys_interact(bs_ip, sizeof(bs_ip), cmd);
		api_get_string_option2(bs_ip, sizeof(bs_ip), "base-station-list.@base-station[%d].ip", idx);
		
	//debug_print("Jason DEBUG %s[%d] bs_ip [%s] \n", __FUNCTION__, __LINE__, bs_ip);

	/* login */

    snprintf(cmd, sizeof(cmd), "curl --connect-timeout 3 --max-time 20 -v -k -X POST \"https://%s:4430/api/sys/login\" -H \"accept: */*\" -H \"Content-Type: application/json\" -d \"{\\\"username\\\":\\\"%s\\\",\\\"password\\\":\\\"%s\\\"}\" | grep token | awk '{for (i=1;i<=NF;i++){if ($i ~/token/) {print $(i+1)}}}' | sed -e 's/\"//g' | tr -d \"\\n\"", bs_ip, username, password);
        debug_print("Jason DEBUG %s[%d] cmd: [--%s--] \n", __FUNCTION__, __LINE__, cmd);

        debug_print("Jason DEBUG %s[%d] cmd_debug: [??%s??] \n", __FUNCTION__, __LINE__, cmd_debug);
        system(cmd_debug);
        
	sys_interact_long(device_token, sizeof(device_token), cmd);
        debug_print("Jason DEBUG %s[%d] device_token: %s. \n", __FUNCTION__, __LINE__, device_token);

        if(strcmp(device_token, "---") == 0)
                goto EXIT;

        //debug_print("Jason DEBUG %s[%d] cmd_debug: [??%s??] \n", __FUNCTION__, __LINE__, cmd);
        sys_interact_long(buf, sizeof(buf), cmd);

        //debug_print("Jason DEBUG %s[%d] buf [%s] \n", __FUNCTION__, __LINE__, buf);
  	if((bs_time_jobj = jsonTokenerParseFromStack(rep, buf)))
        {
                //senao_json_object_get_and_create_string(rep, bs_time_jobj, "Manual_set", &restful_res_str);
                senao_json_object_get_and_create_string(rep, bs_time_jobj, "status", &restful_res_str);
        		//debug_print("Jason DEBUG %s[%d] restful_res_str [%s] \n", __FUNCTION__, __LINE__, restful_res_str);

  		  if((m_time_jobj = jsonTokenerParseFromStack(rep, restful_res_str)))
		  {
			  
                	senao_json_object_get_and_create_string(rep, m_time_jobj, "Manual_set", &manual_set_str);
        		//debug_print("Jason DEBUG %s[%d] manual_set_str [%s] \n", __FUNCTION__, __LINE__, manual_set_str);

                  	if(( res_jobj = jsonTokenerParseFromStack(rep, manual_set_str)))
                  	{
  
                          senao_json_object_get_and_create_string(rep, res_jobj, "date", &bs_date);
			  senao_json_object_get_and_create_string(rep, res_jobj, "time", &bs_time);
        			//debug_print("Jason DEBUG %s[%d] bs_date [%s] \n", __FUNCTION__, __LINE__, bs_date);
        			//debug_print("Jason DEBUG %s[%d] bs_time [%s] \n", __FUNCTION__, __LINE__, bs_time);

				sprintf(time_buf, "%s:00", bs_time);
	 			
				if(strlen(time_buf) >= time_len)
					 goto EXIT;	
				
				strncpy(time, time_buf, time_len - 1);
				return 0;
                  	}
		  }
  
  
          }
}


	json_object_object_add(jobj, "display_name", json_object_new_string(display_name));
#endif

static int json_post_start_bs_fw_upgrade(ResponseEntry *rep, char *query_str, int bs_idx)
{
	struct json_object *jobj = NULL;
	ResponseStatus *res = rep->res;

	char username[32] = {0};
	char password[32] = {0};

	char bs_ip[32] = {0};

	char bsc_id[32] = {0};
//	char bs_fw_filename[] = {"openwrt-am335x-sp938bs-norplusnand-fw-s.img"};
//	char bs_fw_filename[] = {"bs_firmware.dlf"};
        
	char cmd[1024] = {0};
        char buf[8192] = {0}, cmd_debug[1024] = {0};
        char device_token[1024] = {0};
	
	char upgrade_mode[] = {"Upgrade_from_server"};
	char upgrade_url[] = {"bsc_ip"};
#if 0
	int idx = bs_idx -1;
	if((idx < RAINIER_BSID_MIN) && (idx > RAINIER_BSID_MAX))
        /* incorrect range */
        	RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "BASE ID");        

	if(query_str == NULL)
        	RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
	
        if((jobj = jsonTokenerParseFromStack(rep, query_str)) == NULL)
            	RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "JSON");
#else
	int idx = bs_idx;
#endif	    	

 	debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);

	getBscId(bsc_id, sizeof(bsc_id));
	 
	debug_print("Jason DEBUG %s[%d] bsc_id [%s] \n", __FUNCTION__, __LINE__, bsc_id);

        snprintf(username, sizeof(username), "%s", bsc_id);
        //snprintf(cmd, sizeof(cmd), "uci get base-station-list.@base-station[%d].bs_key | tr -d \"\\n\"", idx);
        //sys_interact(password, sizeof(password), cmd);
		api_get_string_option2(password, sizeof(password), "base-station-list.@base-station[%d].bs_key", idx);

	debug_print("Jason DEBUG %s[%d] username [%s] \n", __FUNCTION__, __LINE__, username);
	debug_print("Jason DEBUG %s[%d] password [%s] \n", __FUNCTION__, __LINE__, password);

        //snprintf(cmd, sizeof(cmd), "uci get base-station-list.@base-station[%d].ip | tr -d \"\\n\"", idx);
        //sys_interact(bs_ip, sizeof(bs_ip), cmd);
		api_get_string_option2(bs_ip, sizeof(bs_ip), "base-station-list.@base-station[%d].ip", idx);
		
	debug_print("Jason DEBUG %s[%d] bs_ip [%s] \n", __FUNCTION__, __LINE__, bs_ip);

	/* login */

    snprintf(cmd, sizeof(cmd), "curl --connect-timeout 3 --max-time 20 -v -k -X POST \"https://%s:4430/api/sys/login\" -H \"accept: */*\" -H \"Content-Type: application/json\" -d \"{\\\"username\\\":\\\"%s\\\",\\\"password\\\":\\\"%s\\\"}\" | grep token | awk '{for (i=1;i<=NF;i++){if ($i ~/token/) {print $(i+1)}}}' | sed -e 's/\"//g' | tr -d \"\\n\"", bs_ip, username, password);
        debug_print("Jason DEBUG %s[%d] cmd: [--%s--] \n", __FUNCTION__, __LINE__, cmd);

        //debug_print("Jason DEBUG %s[%d] cmd_debug: [??%s??] \n", __FUNCTION__, __LINE__, cmd_debug);
        //system(cmd_debug);
        
	sys_interact_long(device_token, sizeof(device_token), cmd);
        debug_print("Jason DEBUG %s[%d] device_token: %s. \n", __FUNCTION__, __LINE__, device_token);

        if(strcmp(device_token, "---") == 0)
                goto EXIT;


	memset(cmd, 0 ,sizeof(cmd));
        snprintf(cmd, sizeof(cmd), "curl --connect-timeout 3 --max-time 20 -v -k -X POST \"https://%s:4430/api/mgm/bs_fw_upgrade\" -H \"accept: */*\" -H \"Authorization: Bearer %s\" -d \"{\\\"mode\\\":\\\"%s\\\",\\\"upgrade_from_server\\\":{\\\"upgrade_url\\\":\\\"%s\\\"}}\"" , bs_ip, device_token, upgrade_mode, upgrade_url);

	debug_print("cmd: %s.", cmd);
	sys_interact_long(buf, sizeof(buf), cmd);
	debug_print("buf: %s.", buf);

	if(strstr(buf, "OK") == NULL) // OK
	{
		debug_print("mgm/bs_fw_upgrade");
                goto EXIT;
	}
    	
	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
EXIT:
	RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "Unable to login Base");
}

static int json_post_start_hs_fw_upgrade(ResponseEntry *rep, char *query_str, int bs_idx)
{
	struct json_object *jobj = NULL;
	ResponseStatus *res = rep->res;

	char username[32] = {0};
	char password[32] = {0};
	char bs_ip[32] = {0};
	char bsc_id[32] = {0};
        
	char cmd[1024] = {0};
	char buf[8192] = {0}, cmd_debug[1024] = {0};
	char device_token[1024] = {0};
	
	char upgrade_mode[] = {"Upgrade_from_server"};
	char upgrade_url[] = {"bsc_ip"};

	int idx = bs_idx;

 	debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);

	getBscId(bsc_id, sizeof(bsc_id));
	 
	debug_print("Jason DEBUG %s[%d] bsc_id [%s] \n", __FUNCTION__, __LINE__, bsc_id);

	snprintf(username, sizeof(username), "%s", bsc_id);
	//snprintf(cmd, sizeof(cmd), "uci get base-station-list.@base-station[%d].bs_key | tr -d \"\\n\"", idx);
	//sys_interact(password, sizeof(password), cmd);
	api_get_string_option2(password, sizeof(password), "base-station-list.@base-station[%d].bs_key", idx);

	debug_print("Jason DEBUG %s[%d] username [%s] \n", __FUNCTION__, __LINE__, username);
	debug_print("Jason DEBUG %s[%d] password [%s] \n", __FUNCTION__, __LINE__, password);

	//snprintf(cmd, sizeof(cmd), "uci get base-station-list.@base-station[%d].ip | tr -d \"\\n\"", idx);
	//sys_interact(bs_ip, sizeof(bs_ip), cmd);
	api_get_string_option2(bs_ip, sizeof(bs_ip), "base-station-list.@base-station[%d].ip", idx);
		
	debug_print("Jason DEBUG %s[%d] bs_ip [%s] \n", __FUNCTION__, __LINE__, bs_ip);

	/* login */

    snprintf(cmd, sizeof(cmd), "curl --connect-timeout 3 --max-time 20 -v -k -X POST \"https://%s:4430/api/sys/login\" -H \"accept: */*\" -H \"Content-Type: application/json\" -d \"{\\\"username\\\":\\\"%s\\\",\\\"password\\\":\\\"%s\\\"}\" | grep token | awk '{for (i=1;i<=NF;i++){if ($i ~/token/) {print $(i+1)}}}' | sed -e 's/\"//g' | tr -d \"\\n\"", bs_ip, username, password);
	debug_print("Jason DEBUG %s[%d] cmd: [--%s--] \n", __FUNCTION__, __LINE__, cmd);

	//debug_print("Jason DEBUG %s[%d] cmd_debug: [??%s??] \n", __FUNCTION__, __LINE__, cmd_debug);
	//system(cmd_debug);
        
	sys_interact_long(device_token, sizeof(device_token), cmd);
	debug_print("Jason DEBUG %s[%d] device_token: %s. \n", __FUNCTION__, __LINE__, device_token);

	if(strcmp(device_token, "---") == 0)
		goto EXIT;


	memset(cmd, 0 ,sizeof(cmd));
	snprintf(cmd, sizeof(cmd), "curl --connect-timeout 3 --max-time 20 -v -k -X POST \"https://%s:4430/api/mgm/hs_fw_upgrade\" -H \"accept: */*\" -H \"Authorization: Bearer %s\" -d \"{\\\"mode\\\":\\\"%s\\\",\\\"upgrade_from_server\\\":{\\\"upgrade_url\\\":\\\"%s\\\"}}\"" , bs_ip, device_token, upgrade_mode, upgrade_url);

	debug_print("cmd: %s.", cmd);
	sys_interact_long(buf, sizeof(buf), cmd);
	debug_print("buf: %s.", buf);

	if(strstr(buf, "OK") == NULL) // OK
	{
		debug_print("mgm/hs_fw_upgrade");
                goto EXIT;
	}
    	
	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
EXIT:
	RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "Unable to login Base");
}

static int json_post_stop_hs_fw_upgrade(ResponseEntry *rep, char *query_str, int bs_idx)
{
	struct json_object *jobj = NULL;
	ResponseStatus *res = rep->res;

	char username[32] = {0};
	char password[32] = {0};
	char bs_ip[32] = {0};
	char bsc_id[32] = {0};
        
	char cmd[1024] = {0};
	char buf[8192] = {0}, cmd_debug[1024] = {0};
	char device_token[1024] = {0};
	
	char upgrade_mode[] = {"Upgrade_from_server"};
	char upgrade_url[] = {"bsc_ip"};

	int idx = bs_idx;

 	debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);

	getBscId(bsc_id, sizeof(bsc_id));
	 
	debug_print("Jason DEBUG %s[%d] bsc_id [%s] \n", __FUNCTION__, __LINE__, bsc_id);

	snprintf(username, sizeof(username), "%s", bsc_id);
	//snprintf(cmd, sizeof(cmd), "uci get base-station-list.@base-station[%d].bs_key | tr -d \"\\n\"", idx);
	//sys_interact(password, sizeof(password), cmd);
	api_get_string_option2(password, sizeof(password), "base-station-list.@base-station[%d].bs_key", idx);

	debug_print("Jason DEBUG %s[%d] username [%s] \n", __FUNCTION__, __LINE__, username);
	debug_print("Jason DEBUG %s[%d] password [%s] \n", __FUNCTION__, __LINE__, password);

	//snprintf(cmd, sizeof(cmd), "uci get base-station-list.@base-station[%d].ip | tr -d \"\\n\"", idx);
	//sys_interact(bs_ip, sizeof(bs_ip), cmd);
	api_get_string_option2(bs_ip, sizeof(bs_ip), "base-station-list.@base-station[%d].ip", idx);
		
	debug_print("Jason DEBUG %s[%d] bs_ip [%s] \n", __FUNCTION__, __LINE__, bs_ip);

	/* login */

    snprintf(cmd, sizeof(cmd), "curl --connect-timeout 3 --max-time 20 -v -k -X POST \"https://%s:4430/api/sys/login\" -H \"accept: */*\" -H \"Content-Type: application/json\" -d \"{\\\"username\\\":\\\"%s\\\",\\\"password\\\":\\\"%s\\\"}\" | grep token | awk '{for (i=1;i<=NF;i++){if ($i ~/token/) {print $(i+1)}}}' | sed -e 's/\"//g' | tr -d \"\\n\"", bs_ip, username, password);
	debug_print("Jason DEBUG %s[%d] cmd: [--%s--] \n", __FUNCTION__, __LINE__, cmd);

	//debug_print("Jason DEBUG %s[%d] cmd_debug: [??%s??] \n", __FUNCTION__, __LINE__, cmd_debug);
	//system(cmd_debug);
        
	sys_interact_long(device_token, sizeof(device_token), cmd);
	debug_print("Jason DEBUG %s[%d] device_token: %s. \n", __FUNCTION__, __LINE__, device_token);

	if(strcmp(device_token, "---") == 0)
		goto EXIT;


	memset(cmd, 0 ,sizeof(cmd));
	snprintf(cmd, sizeof(cmd), "curl --connect-timeout 3 --max-time 20 -v -k -X POST \"https://%s:4430/api/mgm/hs_fw_upgrade_abort\" -H \"accept: */*\" -H \"Authorization: Bearer %s\" -d \"{\\\"mode\\\":\\\"%s\\\",\\\"upgrade_from_server\\\":{\\\"upgrade_url\\\":\\\"%s\\\"}}\"" , bs_ip, device_token, upgrade_mode, upgrade_url);

	debug_print("cmd: %s.", cmd);
	sys_interact_long(buf, sizeof(buf), cmd);
	debug_print("buf: %s.", buf);

	if(strstr(buf, "OK") == NULL) // OK
	{
		debug_print("mgm/hs_fw_upgrade_abort");
                goto EXIT;
	}
    	
	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
EXIT:
	RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "Unable to login Base");
}

#if SUPPORT_SP938BS_OPENAPI_SERVER
int json_post_mgm_bs_fw_upgrade(ResponseEntry *rep, char *query_str)
{

	struct json_object *jobj = NULL;
	ResponseStatus *res = rep->res;
	char *fw_file_name;
#if 0
	if(NULL != query_str)
    	{
        	if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        	{
            		senao_json_object_get_and_create_string(rep, jobj, "filename", &fw_file_name);

			debug_print("Jason DEBUG %s[%d] filename: [%s]\n ", __FUNCTION__, __LINE__, fw_file_name);

			if(upgrade_firmware_from_bsc(fw_file_name) < 0)
            			RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "JSON");
		}
        	else
        	{	
            		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "JSON");
        	}
    	}	
    	else
    	{
        	RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
    	}

    	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
#else
	system("rainier_web_fw -m 6 -d bs_firmware.dlf -p 938b &");
    	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
#endif

}

int json_post_mgm_hs_fw_upgrade(ResponseEntry *rep, char *query_str)
{

	struct json_object *jobj = NULL;
	ResponseStatus *res = rep->res;
	char *fw_file_name;

	system("rainier_web_fw -m 6 -d hs_firmware.dlf -p 938a &");

   	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);

}

int json_post_mgm_hs_fw_upgrade_abort(ResponseEntry *rep, char *query_str)
{
	ResponseStatus *res = rep->res;

	system("system_cli -rav3 hs_fw stop &");

   	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}
#endif  /* #if SUPPORT_SP938BS_OPENAPI_SERVER */

static void ast_get_hs_fxo_status(int *hs_status, int *fxo_status)
{
	FILE * fp;
	char file_name[32];
	char buf[256];
	char str[6][32];
	char match_str[4] = {0};
	int i, j, match_i, matched;
	int ret;

	/* Get information from asterisk */
	SYSTEM("mkdir -p %s", AST_DEVICE_DIR);	
	SYSTEM("asterisk -rx \"core show hints\" > %s/%s", AST_DEVICE_DIR, AST_DEVICE_HINT);
	
	/* Parse hints information */
	sprintf(file_name, "%s/%s", AST_DEVICE_DIR, AST_DEVICE_HINT);
	fp = fopen(file_name , "r");
	if(fp == NULL)
	{
		debug_print("can not open file: %s\n", file_name);
		return;
	}
	
	while(fgets(buf, sizeof(buf), fp) != NULL)
	{
		if (ret = sscanf(buf, "%s %s %s %s %s %s",
					str[0], str[1], str[2], str[3], str[4], str[5]) != 6)
		{
			//debug_print("can not read all data, only read: %d data\n", ret);
			continue;
		}
		else
		{
			//debug_print("get data: %s %s %s %s %s %s\n", str[0], str[1], str[2], str[3], str[4], str[5]);

			match_i = 0;
			matched = 0;

			if(strncmp(str[0], "HS_", 3) == 0)
			{
				strncpy(match_str, &str[0][3], 2);
				match_i = atoi(match_str);

				//debug_print("match HS: %d\n", match_i);

				if((match_i >= RAINIER_HSID_MIN) && (match_i <= RAINIER_HSID_MAX))
				{
					matched = 1;	// match HS
				}
			}
			else if(strncmp(str[0], "FXO_", 4) == 0)
			{
				strncpy(match_str, &str[0][4], 2);
				match_i = atoi(match_str);

				//debug_print("match FXO: %d\n", match_i);

				if((match_i >= RAINIER_FXO_MIN) && (match_i <= RAINIER_FXO_MAX))
				{
					matched = 2;	// match FXO
				}
			}

			if(matched)
			{
				if(strcmp(str[3], "State:Idle") == 0)
				{
					//debug_print("match status: Idle\n");
					if(matched == 1)
					{
						hs_status[match_i - RAINIER_HSID_MIN] = 0;		// 0: Free
					}
					else if(matched == 2)
					{
						fxo_status[match_i] = 1;	// 1: free
					}
				}
				else if((strcmp(str[3], "State:Busy") == 0) || (strcmp(str[3], "State:InUse") == 0)
						|| (strcmp(str[3], "State:InUse&Ringing") == 0) || (strcmp(str[3], "State:Hold") == 0))
				{
					//debug_print("match status: Busy\n");
					if(matched == 1)
					{
						hs_status[match_i - RAINIER_HSID_MIN] = 1;		// 1: Busy
					}
					else if(matched == 2)
					{
						fxo_status[match_i] = 2;	// 2: in-used
					}
				}
				else if(strcmp(str[3], "State:Ringing") == 0)
				{
					//debug_print("match status: Ringing\n");
					if(matched == 1)
					{
						hs_status[match_i - RAINIER_HSID_MIN] = 2;		// 2: Ringing
					}
					else if(matched == 2)
					{
						fxo_status[match_i] = 2;	// 2: in-used
					}
				}
				else if(strcmp(str[3], "State:Unavailable") == 0)
				{
					//debug_print("match status: Unavailable\n");
					if(matched == 1)
					{
						hs_status[match_i - RAINIER_HSID_MIN] = 3;		// 3: Un-available
					}
					else if(matched == 2)
					{
						fxo_status[match_i] = 0;	// 0: n/a
					}
				}
			}
		}
	}
	fclose(fp);

	return;
}

static void ast_get_bs_voip_status(int *voip_status)
{
	FILE * fp;
	char file_name[32];
	char buf[256];
	char str[8][32];
	char match_str[4] = {0};
	int i, j, match_i, matched;
	int ret;

	/* Get information from asterisk */
	SYSTEM("mkdir -p %s", AST_DEVICE_DIR);	
	SYSTEM("asterisk -rx \"sip show peers\" > %s/%s", AST_DEVICE_DIR, AST_DEVICE_PEER);
	
	/* Parse peers information */
	sprintf(file_name, "%s/%s", AST_DEVICE_DIR, AST_DEVICE_PEER);
	fp = fopen(file_name , "r");
	if(fp == NULL)
	{
		debug_print("can not open file: %s\n", file_name);
		return;
	}
	
	while(fgets(buf, sizeof(buf), fp) != NULL)
	{
		ret = sscanf(buf, "%s %s %s %s %s %s %s %s",
					str[0], str[1], str[2], str[3], str[4], str[5], str[6], str[7]);
		if( (ret != 7) && (ret != 8) )	// due to ACL column may be empty, so it can be 7 or 8 column
		{
			//debug_print("can not read all data, only read: %d data\n", ret);
			continue;
		}
		else
		{
			//debug_print("get data: %s %s %s %s %s %s %s %s\n", str[0], str[1], str[2], str[3], str[4], str[5], str[6], str[7]);

			match_i = 0;
			matched = 0;

			if(strncmp(str[0], "Hs", 2) == 0)	// HsxxBsxx
			{
				if(strncmp(&str[0][4], "Bs", 2) == 0)
				{
					strncpy(match_str, &str[0][6], 2);
					match_i = atoi(match_str);

					//debug_print("match BS: %d\n", match_i);

					if((match_i >= RAINIER_BSID_MIN) && (match_i <= RAINIER_BSID_MAX))
					{
						matched = 1;	// match BS
					}
				}
			}

			if(matched)
			{
				//if(atoi(str[6]) != 0)	// port number != 0, means the extension registered to asterisk
				if(atoi(str[ret-2]) != 0)	// port number != 0, means the extension registered to asterisk
				{
					//debug_print("match port: %d\n", atoi(str[6]));
					voip_status[match_i - RAINIER_BSID_MIN] = 1;		// 1: connected
				}
			}
		}
	}
	fclose(fp);

	return;
}

static void ast_get_trunk_status(int *trunk_status)
{
	FILE * fp;
	char file_name[32];
	char buf[256];
	//char str[8][32];
	char str[6][135+1];	// 135 = 64(account_name) + 1(@) + 64(server_name) + 1(:) + 5(port)
	char match_str[4] = {0};
	int i, j, match_i, matched;
	char trunk_name[RAINIER_TRUNK_NUM][135+1];	// 135 = 64(account_name) + 1(@) + 64(server_name) + 1(:) + 5(port)
	char account[64] = {0};
	char server_ip[64] = {0}, outbound_proxy_ip[64] = {0};
	int server_port = 0; //, outbound_proxy_port = 0;
	int ret;
	int total_nums;
	int enable = 0;

	/* Get information from asterisk */
	SYSTEM("mkdir -p %s", AST_DEVICE_DIR);	
	SYSTEM("asterisk -rx \"sip show registry\" > %s/%s", AST_DEVICE_DIR, AST_DEVICE_TRUNK);
	
	api_get_string_option("rainier.voip_basic.server_ip", server_ip, sizeof(server_ip));
	api_get_string_option("rainier.voip_basic.outbound_proxy_ip", outbound_proxy_ip, sizeof(outbound_proxy_ip));

	api_get_integer_option("rainier.voip_basic.server_port", &server_port);
	//api_get_integer_option("rainier.voip_basic.outbound_proxy_port", &outbound_proxy_port);

	/* Reset user name */
	memset(trunk_name, 0, sizeof(trunk_name));

#if 1
	total_nums = getSipTrunkCount();
#else
	total_nums = RAINIER_TRUNK_NUM;	// test
#endif
	
	/* Get user name from sip_trunk config */
	for (i=1;i<=RAINIER_TRUNK_NUM;i++)
	{
		if(i <= total_nums)
		{
#if 1
			api_get_bool_option2(&enable, "sip_trunk.@sip_trunk[%d].enable", i-1);
   			api_get_string_option2(account, sizeof(account), "sip_trunk.@sip_trunk[%d].account", i-1);
#else
			api_get_bool_option2(&enable, "rainier.sip_trunk_%d.enable", i-1);
		    api_get_string_option2(account, sizeof(account), "rainier.sip_trunk_%d.account", i-1);
#endif

			if(enable)
			{
				if(outbound_proxy_ip && strlen(outbound_proxy_ip) > 0)
				{
					sprintf(trunk_name[i-1], "%s@%s:%d", account, server_ip, server_port);
				}
				else
				{
					strcpy(trunk_name[i-1], account);
				}
			}
			else
			{
				sprintf(trunk_name[i-1], "SipTrunk%02d_Disabled", i);
			}
		}
		else
		{
			sprintf(trunk_name[i-1], "SipTrunk%02d_Disabled", i);
		}
	}

	/* Parse registry information */
	sprintf(file_name, "%s/%s", AST_DEVICE_DIR, AST_DEVICE_TRUNK);
	fp = fopen(file_name , "r");
	if(fp == NULL)
	{
		debug_print("can not open file: %s\n", file_name);
		return;
	}
	
	while(fgets(buf, sizeof(buf), fp) != NULL)
	{
		if (ret = sscanf(buf, "%s %s %s %s %s %s",
					str[0], str[1], str[2], str[3], str[4], str[5]) != 6)
		{
			//debug_print("can not read all data, only read: %d data\n", ret);
			continue;
		}
		else
		{
			//debug_print("get data: %s %s %s %s %s %s\n", str[0], str[1], str[2], str[3], str[4], str[5]);

			match_i = 0;
			matched = 0;

			for(i=0;i<RAINIER_TRUNK_NUM;i++)
			{
				if(strcmp(str[2], trunk_name[i]) == 0)
				{
					match_i = i;
					matched = 1;

					//debug_print("match Trunk: %d\n", match_i+1);
					break;
				}
			}

			if(matched)
			{
				if(strcmp(str[4], "Registered") == 0)
				{
					trunk_status[match_i] = 1;		// 1: registered

					//debug_print("registered\n");
				}
			}
		}
	}
	fclose(fp);

	return;
}

static int tm_struct_print(struct tm *tm_info)
{
	char buffer[32] = {0};

	   strftime(buffer, sizeof(buffer), "%x - %I:%M:%S%p", tm_info);
	   debug_print("Jason DEBUG %s[%d] Formatted date & time : |%s| \n", __FUNCTION__, __LINE__, buffer );
	return 0;
}

static int tm_struct_print_int(struct tm *tm1)
{
	debug_print("Jason DEBUG %s[%d] tm -->  [%d] [%d][%d] [%d] [%d] [%d]   ## [%d] [%d] [%d] \n", __FUNCTION__, __LINE__, tm1->tm_year, tm1->tm_mon, tm1->tm_mday,   
		tm1->tm_hour, tm1->tm_min, tm1->tm_sec, tm1->tm_wday, tm1->tm_yday, tm1->tm_isdst);

}

static int db_time_string_to_struct_tm(char *time_str, char *format_str, struct tm *tm1)
{
	sscanf(time_str, format_str, &tm1->tm_year, &tm1->tm_mon, &tm1->tm_mday,   
		&tm1->tm_hour, &tm1->tm_min, &tm1->tm_sec);  
          
    	tm1->tm_year -= 1900;  
        tm1->tm_mon --;  
        tm1->tm_isdst = 0;
	return 0;
}
static double tm_diff_time(struct tm *tm1, struct tm *tm2)
{
  	double diff_t;
        time_t	time1, time2;


	time1 = mktime(tm1);
	time2 = mktime(tm2);

	diff_t = difftime(time1, time2);

#if 0
	tm_struct_print_int(tm1);
	tm_struct_print_int(tm2);
 	debug_print(" time1 = %d \n", time1);
 	debug_print(" time2 = %d \n", time2);
  	debug_print("Execution time = %f \n", diff_t);
#endif

	return diff_t;
}
#if 1



static int get_graph_index(overview_info_t *call_info, overview_db_data_t *db)
{
	int i = 0;

	struct tm time_tm;
	time_t db_time_t;
	graph_call_data_t *graph = call_info->graph_data;


	db_time_string_to_struct_tm(db->time, DB_TIME_FORMAT, &time_tm);

	db_time_t = mktime(&time_tm);

 	for(i = 0; i < CALL_GRAPH_NUM; i++)
	{
		if((db_time_t >= graph[i].start_s) && (db_time_t < graph[i].end_s))
		{
			return i;		
		}

	}

	return -1;
}


static void print_call_top_time(call_top_time_t *top, int nu)
{
	int i = 0;

	for(i = 0; i< nu; i++)
	{
		debug_print("Jason DEBUG %s[%d] top[%d] hsid[%d] [%d]\n", __FUNCTION__, __LINE__, i, 
			top[i].hs_id, top[i].time);
	}
}

static int put_call_overview_data(overview_info_t *call_info, overview_db_data_t *db)
{
	int hsid = db->hsid;
	int bsid = db->bsid;
	graph_call_data_t *graph_data = NULL;

	overview_data_t *search = call_info->overview_search;

	call_distribution_t *distr = &call_info->distribution;

	call_info->sql_callback += 1;
//	debug_print("Jason DEBUG %s[%d] sql_callback [%d] \n", __FUNCTION__, __LINE__, call_info->sql_callback);


	if((search->select_base == 0) || (search->select_base == bsid))
	{
		int graph_index = get_graph_index(call_info, db);
			
		//debug_print("Jason DEBUG %s[%d] graph_index [%d] \n", __FUNCTION__, __LINE__, graph_index);
		//debug_print("Jason DEBUG %s[%d] search->select_base [%d] bsid[%d] \n", __FUNCTION__, __LINE__, search->select_base, bsid);
		//debug_print("Jason DEBUG %s[%d] hsid[%d] \n", __FUNCTION__, __LINE__, hsid);

		if(graph_index >=  0)
		{	
			graph_data = call_info->graph_data;

			graph_data[graph_index].time += atoi(db->duration);
			graph_data[graph_index].number += 1;
			graph_data[graph_index].index = graph_index;
		}

		call_info->call_time[hsid].hs_id = hsid;
		call_info->call_time[hsid].time += atoi(db->duration);

		call_info->call_number[hsid].hs_id = hsid;
		call_info->call_number[hsid].time += 1;

		if(db->call_type == RAINIER_CALL_TYPE_FXO)
		{
			distr->total_fxo_call += 1;

			call_info->fxo_call_time[hsid].hs_id = hsid;
			call_info->fxo_call_time[hsid].time += atoi(db->duration);
	
			call_info->fxo_call_number[hsid].hs_id = hsid;
			call_info->fxo_call_number[hsid].time += 1;
			
		//	debug_print("Jason DEBUG %s[%d] hsid[%d] has fxo call \n", __FUNCTION__, __LINE__, hsid);
		//	debug_print("Jason DEBUG %s[%d] distr->total_fxo_call %d \n", __FUNCTION__, __LINE__, distr->total_fxo_call);

		}
		else
		{
			distr->total_sip_call += 1;
//			debug_print("Jason DEBUG %s[%d] hsid[%d] has sip call \n", __FUNCTION__, __LINE__, hsid);
//			debug_print("Jason DEBUG %s[%d] distr->total_fxo_call %d \n", __FUNCTION__, __LINE__, distr->total_sip_call);
		}
		/* for debug. dump date check */	
	}

	call_info->use_bs_number[bsid].hs_id = bsid;
	call_info->use_bs_number[bsid].time += 1;

	return 0;
}

static int time_all_call_read_callback(void *user, int argc, char **argv, 
		char **azColName) 
{
	int hs_id = 0;
	int bs_id = 0;
	int type = 0;
	int duration = 0;
	int ret = 0;

	overview_db_data_t db_data;

	overview_info_t *call_info = (overview_info_t *)user;

	for (int i = 0; i < argc; i++) 
	{
	   	debug_print("Jason DEBUG %s[%d] i [%d ]azColName |%s| argv[%s] \n", __FUNCTION__, __LINE__, i, azColName[i], argv[i]);
		if(strcmp(OVERVIEW_COL_HSID, azColName[i]) == 0)
		{
			db_data.hsid = atoi(argv[i]);
		}
		else if(strcmp(OVERVIEW_COL_BSID, azColName[i]) == 0)
		{
			db_data.bsid  = atoi(argv[i]);
		}
		else if(strcmp(OVERVIEW_COL_TYPE, azColName[i]) == 0)
		{
			db_data.call_type = atoi(argv[i]);
		}
		else if(strcmp(OVERVIEW_COL_DURATION, azColName[i]) == 0)
		{
			strncpy(db_data.duration, argv[i], sizeof(db_data.duration));
		}
		else if(strcmp(OVERVIEW_COL_TIME, azColName[i]) == 0)
		{
			strncpy(db_data.time, argv[i], sizeof(db_data.time));
		}
		else		
		{
			printf("%s: not recognized data [%s -> %s] \n", __FUNCTION__,
					azColName[i], argv[i]);
		}
	}

	ret = put_call_overview_data(call_info, &db_data);
	
	return ret;

}
#endif
static int call_duration_read_callback(void *user, int argc, char **argv, 
                    char **azColName) {


	db_time_t *dbtime = (db_time_t *) user;

	struct tm start_tm;
	struct tm end_tm;
	signed long diff_sec = 0;
	
	for (int i = 0; i < argc; i++) 
	{
#if 1
		if(!strcmp(DB_CDR_DURATION, azColName[i]))
			dbtime->total_sec += atoi(argv[i]);	
#else
		if(argv[i] != NULL)
		{
			dbtime->total_sec = atoi(argv[i]);	
			debug_print(" dbtime->total_sec %d \n", dbtime->total_sec);
		}
#endif
	   debug_print("Jason DEBUG %s[%d] i [%d ]azColName |%s| argv[%s] \n", __FUNCTION__, __LINE__, i, azColName[i], argv[i]);
	}

	return 0;	
}



static int call_number_read_callback(void *user, int argc, char **argv, 
                    char **azColName) {


	db_time_t *dbtime = (db_time_t *) user;

	struct tm start_tm;
	struct tm end_tm;
	signed long diff_sec = 0;

	dbtime->total_sec++;

	return 0;	
}
static int total_time_read_callback(void *user, int argc, char **argv, 
                    char **azColName) {


	db_time_t *dbtime = (db_time_t *) user;

	struct tm start_tm;
	struct tm end_tm;
	signed long diff_sec = 0;

    	for (int i = 0; i < argc; i++) 
	{
	//	 debug_print("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
		if(!strcmp(COLUMN_TIME_STR, azColName[i]))
		{
			memset(dbtime->current_time, 0, sizeof(dbtime->current_time)); 
			strncpy(dbtime->current_time, argv[i], sizeof(dbtime->current_time)); 
		}

		if(!strcmp(dbtime->column_str, azColName[i]))
		{
			dbtime->current_sts = atoi(argv[i]);

			if(dbtime->first_sts == -1)
			{
				dbtime->first_sts = dbtime->current_sts;
				memset(dbtime->first_sts_time, 0, sizeof(dbtime->first_sts_time)); 
				strncpy(dbtime->first_sts_time, dbtime->current_time, sizeof(dbtime->first_sts_time)); 
			}
		}
	}
	if(dbtime->current_sts == dbtime->last_sts)
		return 0;

	if((dbtime->current_sts == 0) && (dbtime->last_sts == 1))
	{
		db_time_string_to_struct_tm(dbtime->current_time, DB_TIME_FORMAT, &end_tm);
		db_time_string_to_struct_tm(dbtime->last_time, DB_TIME_FORMAT, &start_tm);

		//tm_struct_print(&start_tm);
		//tm_struct_print(&end_tm);
		diff_sec = (signed long)tm_diff_time(&end_tm, &start_tm);
		dbtime->total_sec += diff_sec;
	}
	memset(dbtime->last_time, 0, sizeof(dbtime->last_time)); 
	strncpy(dbtime->last_time, dbtime->current_time, sizeof(dbtime->last_time)); 
	dbtime->last_sts = dbtime->current_sts;

	return 0;	
}


static int total_count_string_read_callback(void *user, int argc, char **argv, 
                    char **azColName) {


	db_count_t *dbcount = (db_count_t*) user;

	struct tm start_tm;
	struct tm end_tm;

	dbcount->total_count++;
	return 0;

}


static int total_count_read_callback(void *user, int argc, char **argv, 
                    char **azColName) {


	db_count_t *dbcount = (db_count_t*) user;

	struct tm start_tm;
	struct tm end_tm;

	if(dbcount->column_str == NULL)
	{
		dbcount->total_count++;
		return 0;
	}

    	for (int i = 0; i < argc; i++) 
	{
	//	 debug_print("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
		if(!strcmp(dbcount->column_str, azColName[i]))
		{
			if(dbcount->check_val == -1)
			{
				dbcount->total_count++;
			}
			else
			{
				if(atoi(argv[i]) == dbcount->check_val)
				{
					dbcount->total_count++;
				}
			}
		}
		
	}

	return 0;
}


static int seconds_to_hms_str(signed long sec, char *str)
{
	int  h, m, s;
			
	h = (sec/3600); 
				
	m = (sec -(3600*h))/60;
					
	s = (sec -(3600*h)-(m*60));
						
	sprintf(str, "%02d:%02d:%02d\0", h, m, s);

	return 0;
}

static int get_cdr_dir_count(char *start, char *end, int bs_id, int hs_id, 
		char *db_file, char *table,  char *column, int *count, char *dir)
{
    	char buf[128] = {0};
    	char time[128] = {0};
	sqlite3 *sqldb = NULL;
	db_count_t dbcount;

	memset(&dbcount, 0, sizeof(db_count_t));

	if(get_database(db_file, &sqldb) < 0)
    		return -1;

       if((bs_id == 0) && (hs_id ==0))
        {
                /* search all data */
                sprintf(buf, "SELECT * FROM %s WHERE start>=\'%s\' AND end<\'%s\' AND dir =\'%s\'",
                    	table, start, end, dir);
        }
        else if((bs_id == 0) && (hs_id !=0))
        {
                sprintf(buf, "SELECT * FROM %s WHERE start>=\'%s\' AND end<\'%s\' AND (src=\'Hs%02d\' OR dst=\'Hs%02d\') AND dir =\'%s\'",
                       table, start, end, hs_id, hs_id, dir);
        }
        else if((bs_id != 0) && (hs_id ==0))
        {
                sprintf(buf, "SELECT * FROM %s WHERE start>=\'%s\' AND end<\'%s\' AND base LIKE \'%cBs%02d%c\' AND dir =\'%s\'",
                        table, start, end, '%', bs_id, '%', dir);
        }
        else
        {
                sprintf(buf, "SELECT * FROM %s WHERE start>=\'%s\' AND end<\'%s\' AND base LIKE \'%cBs%02d%c\' AND (src=\'Hs%02d\' OR dst=\'Hs%02d\') AND dir =\'%s\'",
                        table, start, end, '%', bs_id, '%', hs_id, hs_id, dir);
        }


	
	//debug_print(" Jason DEBUG %s[%d] sql read command  -> %s \n", __FUNCTION__, __LINE__, buf);

	read_database(buf, sqldb, total_count_string_read_callback, &dbcount);

	free_database(sqldb);

	*count = dbcount.total_count;
	
	return 0;
}

static int sql_cmd_add_search_string_in_cdr(char *cmd, int cmd_size, char *search_str)
{
	char *attach;
	int  org_len;
	

	org_len = strlen(cmd);


	attach = cmd + org_len;
//	sprintf(attach, " AND (trunk LIKE \'%c%s%c\' OR clid  LIKE \'%c%s%c\' OR src LIKE \'%c%s%c\' OR dst LIKE \'%c%s%c\' OR channel LIKE \'%c%s%c\' OR dstchannel LIKE \'%c%s%c\' OR disposition LIKE \'%c%s%c\')", 
	sprintf(attach, " AND (trunk LIKE \'%c%s%c\' OR clid  LIKE \'%c%s%c\' OR src LIKE \'%c%s%c\' OR dst LIKE \'%c%s%c\' OR disposition LIKE \'%c%s%c\')", 
			'%', search_str, '%',
			'%', search_str, '%',
			'%', search_str, '%',
			'%', search_str, '%',
			'%', search_str, '%',
			'%', search_str, '%',
			'%', search_str, '%'
			);
	return 0;
}

static int get_cdr_total_count(char *start, char *end, int bs_id, int hs_id, 
		char *db_file, char *table,  char *column, int *count, int val)
{
    	char buf[128] = {0};
    	char time[128] = {0};
	sqlite3 *sqldb = NULL;
	db_count_t dbcount;

	memset(&dbcount, 0, sizeof(db_count_t));

       if(get_database(db_file, &sqldb) < 0)
    		return -1;
       if((bs_id == 0) && (hs_id ==0))
        {
                /* search all data */
                sprintf(buf, "SELECT * FROM %s WHERE start>=\'%s\' AND end<\'%s\'",
                    	table, start, end);
        }
        else if((bs_id == 0) && (hs_id !=0))
        {
                sprintf(buf, "SELECT * FROM %s WHERE start>=\'%s\' AND end<\'%s\' AND (src=\'Hs%02d\' OR dst=\'Hs%02d\')",
                       table, start, end, hs_id, hs_id);
        }
        else if((bs_id != 0) && (hs_id ==0))
        {
                sprintf(buf, "SELECT * FROM %s WHERE start>=\'%s\' AND end<\'%s\' AND base LIKE \'%cBs%02d%c\'",
                        table, start, end, '%', bs_id, '%');
        }
        else
        {
                sprintf(buf, "SELECT * FROM %s WHERE start>=\'%s\' AND end<\'%s\' AND base LIKE \'%cBs%02d%c\' AND (src=\'Hs%02d\' OR dst=\'Hs%02d\')",
                        table, start, end, '%', bs_id, '%', hs_id, hs_id);
        }

	dbcount.column_str = column;
	dbcount.check_val = val;

	
	//debug_print(" Jason DEBUG %s[%d] sql read command  -> %s \n", __FUNCTION__, __LINE__, buf);

	read_database(buf, sqldb, total_count_read_callback, &dbcount);

	free_database(sqldb);

	*count = dbcount.total_count;
	
	return 0;

}


static int get_bsinfo_total_count(char *start, char *end, int bs_id, int hs_id, 
		char *db_file, char *table,  char *column, int *count, int val)
{
    	char buf[128] = {0};
    	char time[128] = {0};
	sqlite3 *sqldb = NULL;
	db_count_t dbcount;

	memset(&dbcount, 0, sizeof(db_count_t));

	if(get_database(db_file, &sqldb) < 0)
    		return -1;

	if(bs_id != 0) 
	{
		sprintf(buf, "SELECT * FROM %s WHERE time>=\'%s\' AND time<=\'%s\' AND bs_index=%d AND %s!=\'NULL\'", 
			table, start, end, bs_id, column);
	}
	else
	{
		sprintf(buf, "SELECT * FROM %s WHERE time>=\'%s\' AND time<=\'%s\' AND %s!=\'NULL\'", 
			table, start, end, column);
	
	}
	dbcount.column_str = column;
	dbcount.check_val = val;

	
	//debug_print(" Jason DEBUG %s[%d] sql read command  -> %s \n", __FUNCTION__, __LINE__, buf);

	read_database(buf, sqldb, total_count_read_callback, &dbcount);

	free_database(sqldb);

	*count = dbcount.total_count;
	
	return 0;

}

static signed long get_time_to_sec(char *start, char *end)
{

	struct tm start_tm;
	struct tm end_tm;

	db_time_string_to_struct_tm(start, DB_TIME_FORMAT, &start_tm);
	db_time_string_to_struct_tm(end, DB_TIME_FORMAT, &end_tm);

	return (signed long)tm_diff_time(&end_tm, &start_tm);
}

static signed long time_string_to_sec(char *time_string)
{

	int h;
	int m;
	int s;

	signed long sec = 0;

	sscanf(time_string, "%d:%d:%d", &h, &m, &s);

	sec = (h * 3600) + (m * 60) + s;

	return sec;

}

static int get_cdr_duration(char *start, char *end, int bs_id, int hs_id, int type, 
		char *db_file, char *table,  char *column, char *ret_time_str, int len_size)
{
	sqlite3 *sqldb = NULL;
	db_time_t dbtime;
	char peer_str[32] = {0};
	char use_base_str[32] = {0};
	char buf[256] = {0};
	/* init db time */
	memset(&dbtime, 0, sizeof(db_time_t));
	dbtime.first_sts = -1;

	char fxo_trunk_str[] = {"\'%fxo%\'"};
	char sip_trunk_str[] = {"\'%sip%\'"};

	/* create peer */

	/*   like Hs11Bs01 */
	if(hs_id == 0)
		sprintf(peer_str, "%sBs%02d%s", "%", bs_id, "%");
	else
		sprintf(peer_str, "%s%02dBs%02d%s", "%Hs", hs_id, bs_id, "%");

	sprintf(use_base_str, "%sBs%02d%s", "%", bs_id, "%");

	dbtime.column_str = column;

	if(get_database(db_file, &sqldb) < 0)
    		return -1;
#if 0
	switch(type)
	{
		case TOP_CALL_TYPE_ALL:
			sprintf(buf, "SELECT SUM(%s) FROM %s WHERE start>=\'%s\' AND end<=\'%s\' AND (channel LIKE \'%s\' OR dstchannel LIKE \'%s\') AND %s!=\'NULL\' GROUP BY SUM(%s) ORDER BY SUM(%s) DESC", 
				column, table, start, end, peer_str, peer_str, column, column, column);
		
			read_database(buf, sqldb, call_duration_read_callback, &dbtime);
			break;

		case TOP_CALL_TYPR_FXO:
	sprintf(buf, "SELECT SUM(%s) FROM %s WHERE start>=\'%s\' AND end<=\'%s\' AND trunk LIKE %s AND (channel LIKE \'%s\' OR dstchannel LIKE \'%s\') AND %s!=\'NULL\'  GROUP BY SUM(%s) ORDER BY SUM(%s) DESC", column, table, start, end, fxo_trunk_str, peer_str, peer_str, column, column, column);
			
			read_database(buf, sqldb, call_duration_read_callback, &dbtime);
			break;

		case TOP_CALL_TYPE_ALL_NUMBER:
			sprintf(buf, "SELECT * FROM %s WHERE start>=\'%s\' AND end<=\'%s\' AND (channel LIKE \'%s\' OR dstchannel LIKE \'%s\')", 
				table, start, end, peer_str, peer_str);
		
			read_database(buf, sqldb, call_number_read_callback, &dbtime);
			break;

		case TOP_CALL_TYPR_FXO_NUMBER:
	sprintf(buf, "SELECT * FROM %s WHERE start>=\'%s\' AND end<=\'%s\' AND trunk LIKE %s AND (channel LIKE \'%s\' OR dstchannel LIKE \'%s\')", 
			table, start, end, fxo_trunk_str, peer_str, peer_str);
			
			read_database(buf, sqldb, call_number_read_callback, &dbtime);
			break;
		
		case TOP_USE_BASE_TYPE_NUMBER:
			sprintf(buf, "SELECT * FROM %s WHERE start>=\'%s\' AND end<=\'%s\' AND (channel LIKE \'%s\' OR dstchannel LIKE \'%s\')", 
				table, start, end, use_base_str, use_base_str);
		
			debug_print(" Jason DEBUG %s[%d] sql read command  -> [%s] \n", __FUNCTION__, __LINE__, buf);
			read_database(buf, sqldb, call_number_read_callback, &dbtime);
			break;

		case DISTRIBUTION_CALL_SIP:
			sprintf(buf, "SELECT * FROM %s WHERE start>=\'%s\' AND end<=\'%s\' AND trunk LIKE %s", 
				table, start, end, sip_trunk_str);
			
			read_database(buf, sqldb, call_number_read_callback, &dbtime);
			break;
		
		case DISTRIBUTION_CALL_FXO:
			sprintf(buf, "SELECT * FROM %s WHERE start>=\'%s\' AND end<=\'%s\' AND trunk LIKE %s", 
				table, start, end, fxo_trunk_str);
			
			read_database(buf, sqldb, call_number_read_callback, &dbtime);
			break;

		default:
			free_database(sqldb);
			return -1;

#else	
	switch(type)
	{
		case TOP_CALL_TYPE_ALL:
			sprintf(buf, "SELECT * FROM %s WHERE start>=\'%s\' AND end<=\'%s\' AND (channel LIKE \'%s\' OR dstchannel LIKE \'%s\') AND %s!=\'NULL\'", 
				table, start, end, peer_str, peer_str, column);
		
			read_database(buf, sqldb, call_duration_read_callback, &dbtime);
			break;

		case TOP_CALL_TYPR_FXO:
	sprintf(buf, "SELECT * FROM %s WHERE start>=\'%s\' AND end<=\'%s\' AND trunk LIKE %s AND (channel LIKE \'%s\' OR dstchannel LIKE \'%s\') AND %s!=\'NULL\'", 
			table, start, end, fxo_trunk_str, peer_str, peer_str, column);
			
			read_database(buf, sqldb, call_duration_read_callback, &dbtime);
			break;

		case TOP_CALL_TYPE_ALL_NUMBER:
			sprintf(buf, "SELECT * FROM %s WHERE start>=\'%s\' AND end<=\'%s\' AND (channel LIKE \'%s\' OR dstchannel LIKE \'%s\')", 
				table, start, end, peer_str, peer_str);
		
			read_database(buf, sqldb, call_number_read_callback, &dbtime);
			break;

		case TOP_CALL_TYPR_FXO_NUMBER:
	sprintf(buf, "SELECT * FROM %s WHERE start>=\'%s\' AND end<=\'%s\' AND trunk LIKE %s AND (channel LIKE \'%s\' OR dstchannel LIKE \'%s\')", 
			table, start, end, fxo_trunk_str, peer_str, peer_str);
			
			read_database(buf, sqldb, call_number_read_callback, &dbtime);
			break;
		
		case TOP_USE_BASE_TYPE_NUMBER:
			sprintf(buf, "SELECT * FROM %s WHERE start>=\'%s\' AND end<=\'%s\' AND (channel LIKE \'%s\' OR dstchannel LIKE \'%s\')", 
				table, start, end, use_base_str, use_base_str);
		
			debug_print(" Jason DEBUG %s[%d] sql read command  -> [%s] \n", __FUNCTION__, __LINE__, buf);
			read_database(buf, sqldb, call_number_read_callback, &dbtime);
			break;

		case DISTRIBUTION_CALL_SIP:
			sprintf(buf, "SELECT * FROM %s WHERE start>=\'%s\' AND end<=\'%s\' AND trunk LIKE %s", 
				table, start, end, sip_trunk_str);
			
			read_database(buf, sqldb, call_number_read_callback, &dbtime);
			break;
		
		case DISTRIBUTION_CALL_FXO:
			sprintf(buf, "SELECT * FROM %s WHERE start>=\'%s\' AND end<=\'%s\' AND trunk LIKE %s", 
				table, start, end, fxo_trunk_str);
			
			read_database(buf, sqldb, call_number_read_callback, &dbtime);
			break;

		default:
			free_database(sqldb);
			return -1;
#endif	
	}
	
//	debug_print(" Jason DEBUG %s[%d] sql read command  -> [%s] \n", __FUNCTION__, __LINE__, buf);


//	read_database(buf, sqldb, call_duration_read_callback, &dbtime);

	snprintf(ret_time_str, len_size, "%d", dbtime.total_sec);

	free_database(sqldb);

	return 0;
}


static int get_total_time(char *start, char *end, int bs_id, int hs_id, 
		char *db_file, char *table,  char *column, char *ret_time_str, int len_size)
{
    	char buf[128] = {0};
    	char time[128] = {0};
	sqlite3 *sqldb = NULL;
	db_time_t dbtime;
	signed long diff_sec = 0; 
	memset(&dbtime, 0, sizeof(db_time_t));

	dbtime.first_sts = -1;


	if(get_database(db_file, &sqldb) < 0)
    		return -1;

	if((bs_id != 0) && (hs_id != 0))
	{
		sprintf(buf, "SELECT * FROM %s WHERE time>=\'%s\' AND time<=\'%s\' AND bs_index=%d AND call_hsid=%d AND %s!=\'NULL\'", 
			table, start, end, bs_id, hs_id, column);
	}
	else if((bs_id != 0) && (hs_id == 0))
	{
		sprintf(buf, "SELECT * FROM %s WHERE time>=\'%s\' AND time<=\'%s\' AND bs_index=%d AND %s!=\'NULL\'", 
			table, start, end, bs_id, column);
	
	}
	else if((bs_id == 0) && (hs_id != 0))
	{
		sprintf(buf, "SELECT * FROM %s WHERE time>=\'%s\' AND time<=\'%s\' AND call_hsid=%d AND %s!=\'NULL\'", 
			table, start, end, hs_id, column);

	}
	else
	{
		sprintf(buf, "SELECT * FROM %s WHERE time>=\'%s\' AND time<=\'%s\' AND %s!=\'NULL\'", 
			table, start, end, column);
	
	}

	dbtime.column_str = column;

	
	//debug_print(" Jason DEBUG %s[%d] sql read command  -> %s \n", __FUNCTION__, __LINE__, buf);

	read_database(buf, sqldb, total_time_read_callback, &dbtime);

	debug_print("Jason DEBUG %s[%d]  dbtime.current_sts -> [%d]\n", __FUNCTION__, __LINE__, dbtime.current_sts);
	if(dbtime.current_sts)
	{


		diff_sec = get_time_to_sec(dbtime.current_time, end);
		debug_print("Jason DEBUG %s[%d]  diff_sec -> [%d]\n", __FUNCTION__, __LINE__, diff_sec);

		if(diff_sec > 0)
			dbtime.total_sec += diff_sec;

	}

	debug_print("Jason DEBUG %s[%d]  dbtime.first_sts -> [%d]\n", __FUNCTION__, __LINE__, dbtime.first_sts);

	db_time_t b_dbtime;

	memset(&b_dbtime, 0, sizeof(b_dbtime));

	b_dbtime.first_sts = -1;

	memset(buf, 0, sizeof(buf));

	if((bs_id != 0) && (hs_id != 0))
	{
		sprintf(buf, "SELECT * FROM %s WHERE time<\'%s\' AND bs_index=%d AND call_hsid=%d AND %s!=\'NULL\'", 
			table, start, bs_id, hs_id, column);
	}
	else if((bs_id != 0) && (hs_id == 0))
	{
		sprintf(buf, "SELECT * FROM %s WHERE time<\'%s\' AND bs_index=%d AND %s!=\'NULL\'", 
			table, start, bs_id, column);

	}
	else if((bs_id == 0) && (hs_id != 0))
	{
		sprintf(buf, "SELECT * FROM %s WHERE time<\'%s\' AND call_hsid=%d AND %s!=\'NULL\'", 
		table, start, hs_id, column);

	}
	else
	{
		sprintf(buf, "SELECT * FROM %s WHERE time<\'%s\' AND %s!=\'NULL\'", 
			table, start, column);
	
	}

	b_dbtime.column_str = column;

	//debug_print("Jason DEBUG %s[%d]  b_dbtime.column_str -> [%s]\n", __FUNCTION__, __LINE__, b_dbtime.column_str);
	//debug_print("Jason DEBUG %s[%d]  sql vuf  -> [%s]\n", __FUNCTION__, __LINE__, buf);

	read_database(buf, sqldb, total_time_read_callback, &b_dbtime);

	//debug_print("Jason DEBUG %s[%d]  b_dbtime.current_time -> [%s]\n", __FUNCTION__, __LINE__, b_dbtime.current_time);
	//debug_print("Jason DEBUG %s[%d]  dbtime.first_sts_time -> [%s]\n", __FUNCTION__, __LINE__, dbtime.first_sts_time);

	if(b_dbtime.current_sts)	
	{	
		if((strlen(b_dbtime.current_time) > 8) && strlen(dbtime.first_sts_time) > 8)
		{
			diff_sec = get_time_to_sec(start, dbtime.first_sts_time);
		//	debug_print("Jason DEBUG %s[%d]  diff_sec -> [%d]\n", __FUNCTION__, __LINE__, diff_sec);

			if(diff_sec > 0)
				dbtime.total_sec += diff_sec;
		}
	}

	free_database(sqldb);

	seconds_to_hms_str(dbtime.total_sec, time);


	//debug_print("  Jason DEBUG %s[%d] total %d sec  -> %s \n", __FUNCTION__, __LINE__, dbtime.total_sec, time);

	if(strlen(time) >= len_size)
		return -1;

	strncpy(ret_time_str, time, len_size);

	return 0;

}

static int get_air_time(char *start, char *end, int bs_id, int hs_id, char *str, int len_size)
{

	return get_total_time(start, end, bs_id, hs_id, 
		DB_BSINFO_PATH, DB_BSINFO_TABLE,  AIR_USE_COLUMN, str, len_size);
}

static int get_busy_time(char *start, char *end, int bs_id, int hs_id, char *str, int len_size)
{

	return get_total_time(start, end, bs_id, hs_id, 
		DB_BSINFO_PATH, DB_BSINFO_TABLE,  BS_BUSY_COLUMN, str, len_size);
}

static int get_busy_count(char *start, char *end, int bs_id, int hs_id, int *count, int val)
{

	return get_bsinfo_total_count(start, end, bs_id, hs_id, 
		DB_BSINFO_PATH, DB_BSINFO_TABLE,  BS_BUSY_COLUMN, count, val);
}

static int get_handover_count(char *start, char *end, int bs_id, int hs_id, int *count, int val)
{

	return get_bsinfo_total_count(start, end, bs_id, hs_id, 
		DB_BSINFO_PATH, DB_BSINFO_TABLE,  BS_HANDOVER_HSID_COLUMN, count, val);
}
static int get_ptp_fail_count(char *start, char *end, int bs_id, int hs_id, int *count, int val)
{

	return get_bsinfo_total_count(start, end, bs_id, hs_id, 
		DB_PTP_PATH, DB_PTP_SYNC_TABLE,  BS_PTP_STS_COLUMN, count, val);
}

static int get_sip_reg_fail_count(char *start, char *end, int bs_id, int hs_id, int *count, int val)
{

	return get_bsinfo_total_count(start, end, bs_id, hs_id, 
		DB_BSINFO_PATH, DB_BSINFO_TABLE,  SIP_REG_FAIL_COLUMN, count, val);
}

static int get_sip_call_count(char *start, char *end, int bs_id, int hs_id, int *count, int val)
{

	return get_cdr_total_count(start, end, bs_id, hs_id, 
		DB_CDR_PATH, DB_CDR_TABLE, NULL, count, val);
}
#if 0
static int get_sip_call_time(char *start, char *end, int bs_id, int hs_id, int *count, int val)
{

	return get_total_time(start, end, bs_id, hs_id, 
		DB_CDR_PATH, DB_BSINFO_TABLE,  AIR_USE_COLUMN, str, len_size);
}
#endif
static int get_sip_call_dir_count(char *start, char *end, int bs_id, int hs_id, int *count, char *dir)
{

	return get_cdr_dir_count(start, end, bs_id, hs_id, 
		DB_CDR_PATH, DB_CDR_TABLE, SIP_CALL_DIR_COLUMN, count, dir);
}
static int get_search_time(char *start, char *end, char *time)
{

	struct tm start_tm;
	struct tm end_tm;
	signed long diff_sec = 0;

	db_time_string_to_struct_tm(end, DB_TIME_FORMAT, &end_tm);
	db_time_string_to_struct_tm(start, DB_TIME_FORMAT, &start_tm);

	//tm_struct_print(&start_tm);
	//tm_struct_print(&end_tm);
	
	diff_sec = (signed long)tm_diff_time(&end_tm, &start_tm);

	//debug_print("Jason DEBUG %s[%d] diff_sec[%d]\n", __FUNCTION__, __LINE__, diff_sec);
	seconds_to_hms_str(diff_sec, time);
	//debug_print("Jason DEBUG %s[%d] [%s]\n", __FUNCTION__, __LINE__, time);
	return 0;
}
static int get_bs_time(ResponseEntry *rep, struct json_object *jobj, char *time, int time_len, int idx)
{
        char cmd[1024] = {0};
        char buf[8192] = {0}, cmd_debug[1024] = {0};
        char device_token[1024] = {0};
	char *restful_res_str = NULL;
	char *time_info_str = NULL;
	struct json_object *bs_time_jobj = NULL, *sys_info_jobj = NULL, *m_time_jobj = NULL;
	struct json_object *res_jobj = NULL;

	char *bs_date = NULL;
	char *bs_time = NULL;

	char time_buf[64] = {0};
	char *manual_set_str = NULL;

	char username[32] = {0};
	char password[32] = {0};

	char bs_ip[32] = {0};

 	//debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);

	char bsc_id[32] = {0};
	getBscId(bsc_id, sizeof(bsc_id));
	 
	//debug_print("Jason DEBUG %s[%d] bsc_id [%s] \n", __FUNCTION__, __LINE__, bsc_id);

        snprintf(username, sizeof(username), "%s", bsc_id);
        //snprintf(cmd, sizeof(cmd), "uci get base-station-list.@base-station[%d].bs_key | tr -d \"\\n\"", idx);
        //sys_interact(password, sizeof(password), cmd);
		api_get_string_option2(password, sizeof(password), "base-station-list.@base-station[%d].bs_key", idx);

	//debug_print("Jason DEBUG %s[%d] username [%s] \n", __FUNCTION__, __LINE__, username);
	//debug_print("Jason DEBUG %s[%d] password [%s] \n", __FUNCTION__, __LINE__, password);

        //snprintf(cmd, sizeof(cmd), "uci get base-station-list.@base-station[%d].ip | tr -d \"\\n\"", idx);
        //sys_interact(bs_ip, sizeof(bs_ip), cmd);
		api_get_string_option2(bs_ip, sizeof(bs_ip), "base-station-list.@base-station[%d].ip", idx);
		
	//debug_print("Jason DEBUG %s[%d] bs_ip [%s] \n", __FUNCTION__, __LINE__, bs_ip);

	/* login */

    snprintf(cmd, sizeof(cmd), "curl --connect-timeout 3 --max-time 20 -v -k -X POST \"https://%s:4430/api/sys/login\" -H \"accept: */*\" -H \"Content-Type: application/json\" -d \"{\\\"username\\\":\\\"%s\\\",\\\"password\\\":\\\"%s\\\"}\" | grep token | awk '{for (i=1;i<=NF;i++){if ($i ~/token/) {print $(i+1)}}}' | sed -e 's/\"//g' | tr -d \"\\n\"", bs_ip, username, password);
        //debug_print("Jason DEBUG %s[%d] cmd: [--%s--] \n", __FUNCTION__, __LINE__, cmd);

        sprintf(cmd_debug, "echo \"%s\" > /tmp/cmd_debug", cmd);
        //debug_print("Jason DEBUG %s[%d] cmd_debug: [??%s??] \n", __FUNCTION__, __LINE__, cmd_debug);
        system(cmd_debug);
        
	sys_interact_long(device_token, sizeof(device_token), cmd);
        //debug_print("Jason DEBUG %s[%d] device_token: %s. \n", __FUNCTION__, __LINE__, device_token);

        if(strcmp(device_token, "---") == 0)
                goto EXIT;



	/* get time */

        snprintf(cmd, sizeof(cmd), "curl --connect-timeout 3 --max-time 20 -v -k -X GET \"https://%s:4430/api/sys/time_config\" -H \"accept: */*\" -H \"Authorization: Bearer %s\"" , bs_ip, device_token);

        //debug_print("Jason DEBUG %s[%d] cmd_debug: [??%s??] \n", __FUNCTION__, __LINE__, cmd);
        sys_interact_long(buf, sizeof(buf), cmd);

        //debug_print("Jason DEBUG %s[%d] buf [%s] \n", __FUNCTION__, __LINE__, buf);
  	if((bs_time_jobj = jsonTokenerParseFromStack(rep, buf)))
        {
                //senao_json_object_get_and_create_string(rep, bs_time_jobj, "Manual_set", &restful_res_str);
                senao_json_object_get_and_create_string(rep, bs_time_jobj, "data", &restful_res_str);
        		//debug_print("Jason DEBUG %s[%d] restful_res_str [%s] \n", __FUNCTION__, __LINE__, restful_res_str);

  		  if((m_time_jobj = jsonTokenerParseFromStack(rep, restful_res_str)))
		  {
			  
                	senao_json_object_get_and_create_string(rep, m_time_jobj, "Manual_set", &manual_set_str);
        		//debug_print("Jason DEBUG %s[%d] manual_set_str [%s] \n", __FUNCTION__, __LINE__, manual_set_str);

                  	if(( res_jobj = jsonTokenerParseFromStack(rep, manual_set_str)))
                  	{
  
                          senao_json_object_get_and_create_string(rep, res_jobj, "date", &bs_date);
			  senao_json_object_get_and_create_string(rep, res_jobj, "time", &bs_time);
        			//debug_print("Jason DEBUG %s[%d] bs_date [%s] \n", __FUNCTION__, __LINE__, bs_date);
        			//debug_print("Jason DEBUG %s[%d] bs_time [%s] \n", __FUNCTION__, __LINE__, bs_time);

				sprintf(time_buf, "%s:00", bs_time);
	 			
				if(strlen(time_buf) >= time_len)
					 goto EXIT;	
				
				strncpy(time, time_buf, time_len - 1);
				return 0;
                  	}
		  }
  
  
          }


EXIT:
	return -1;

}


int get_cdr_base_id_string(char *arg, char *bsid, int id_len)
{
	char buf[8] = {0};
	int bsid1 = 0, bsid2 = 0;

	if((arg == NULL) || (bsid == NULL))
		return 0;
	
	if(strlen(arg) > strlen("Bsxx"))
	{
		/* for intercom bsid string ex. bs01,bs02*/
		sscanf(arg, "Bs%d,Bs%d", &bsid1, &bsid2);
		
		sprintf(buf, "%01d,%01d", bsid1, bsid2);

		strncpy(bsid, buf, id_len);
	}
	else
	{
		strncpy(bsid, arg + 3, id_len);
	}

	return 1;
}

int get_rssi_read_out_value(db_rssi_t *rssi, unsigned int nu, char *cdr_time)
{
	int i = 0;


	for(i = 0; i < nu; i++)
	{
		
		rssi++;
	}

}

int cdr_read_callback(void *jarr_cdr, int argc, char **argv, 
                    char **azColName) {

	struct json_object *jobj_cdrinfo = NULL;
   	int rssi = 0;
	int j = 0;
	char base_id[8] ={0};
	int hs_id = 0;
	int hs_id2 = 0;
	char src[8] ={0};
	char dst[8] ={0};
	char time[32] = {0};	
	#define  MAX_API_JSON_LEN	11
	char *rssi_srt = NULL;
	char intercom_rssi_srt[16] = {0};
	int call_type = 0;
	char *db_tiem[] = {"start", "dir", "trunk", "clid", "src", "dst",  
				"base", "disposition", "duration", "rssi", NULL};

	char *json_tiem[] = {"start_time", "dir", "trunk", "cid", "from", "to", "base", 
				"disposition", "duration", "rssi", NULL};

	if((cdr_page != 0) && ((read_out_row < cdr_start_index) ||  (read_out_row > cdr_end_index)))
	{
		read_out_row++;
		return 0;
	}
	
	read_out_row++;

    	struct json_object *jarr_cdrinfo = (struct json_object *)jarr_cdr;

	jobj_cdrinfo = json_object_new_object();

    //	debug_print("cdr_read_callback \n");
	

    	for (int i = 0; i < argc; i++) 
	{
      //  	debug_print("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");

		for(j = 0; j < MAX_API_JSON_LEN; j++)
		{
			if((db_tiem[j] != NULL) && (json_tiem[j] != NULL))
			{
    				//debug_print(" db_tiem %d [%s], azColName %d [%s] \n", j, json_tiem[j], i, azColName[i]);
				if(!strcmp(db_tiem[j], azColName[i]))
				{
					if(!strcmp(azColName[i], "base"))
					{
    						//json_object_object_add(jobj_cdrinfo, json_tiem[j], json_object_new_string(argv[i] + (sizeof(char) * 3)));
						if(get_cdr_base_id_string(argv[i], base_id, sizeof(base_id)))
						{
    							json_object_object_add(jobj_cdrinfo, json_tiem[j], json_object_new_string(base_id));
    							debug_print(" add to json [%s] [%s] \n", json_tiem[j], argv[i]);

						}
					}
					else if(!strcmp(azColName[i], "dir"))
					{
						if(!strcmp(argv[i], "in"))
							call_type = CDR_TYPE_IN;
						else if(!strcmp(argv[i], "out"))
							call_type = CDR_TYPE_OUT;
						else if(!strcmp(argv[i], "broadcast"))
							call_type = CDR_TYPE_BRAODCAST;
						else if(!strcmp(argv[i], "intercom"))
							call_type = CDR_TYPE_INTERCOM;
						else
							call_type = CDR_TYPE_EMPTY;

    						json_object_object_add(jobj_cdrinfo, json_tiem[j], json_object_new_string(argv[i]));
    						debug_print(" add to json [%s] [%s] \n", json_tiem[j], argv[i]);
					}
					else if(!strcmp(azColName[i], "src"))
					{
						strncpy(src, argv[i], sizeof(src));
    						json_object_object_add(jobj_cdrinfo, json_tiem[j], json_object_new_string(argv[i]));
    						debug_print(" add to json [%s] [%s] \n", json_tiem[j], argv[i]);
					}
					else if(!strcmp(azColName[i], "dst"))
					{
						strncpy(dst, argv[i], sizeof(dst));
    						json_object_object_add(jobj_cdrinfo, json_tiem[j], json_object_new_string(argv[i]));
    						debug_print(" add to json [%s] [%s] \n", json_tiem[j], argv[i]);
					}
					else
					{
    						json_object_object_add(jobj_cdrinfo, json_tiem[j], json_object_new_string(argv[i]));
    						debug_print(" add to json [%s] [%s] \n", json_tiem[j], argv[i]);
					}
				}
			}
			else
			{
				break;
			}
		}
		
#if 1
		if(!strcmp(azColName[i], "end"))
		{
			memset(time, 0, sizeof(time));
			strncpy(time, argv[i], sizeof(time));
		}
		else if(!strcmp(azColName[i], "rssi"))
		{
    			debug_print(" call_type [%d] \n", call_type);

			switch (call_type)
			{
				case CDR_TYPE_OUT:
				case CDR_TYPE_BRAODCAST:
					sscanf(src, "Hs%02d", &hs_id);
					rssi_srt = get_rssi_by_time(time, atoi(base_id), hs_id);
					break;
				case CDR_TYPE_IN:

					if(strstr(dst, "Grp"))
					{
						rssi_srt = get_grp_in_rssi_by_time(time, atoi(base_id));
					
					}
					else
					{
						sscanf(dst, "Hs%02d", &hs_id);
						rssi_srt = get_rssi_by_time(time, atoi(base_id), hs_id);
					}
					break;
				case CDR_TYPE_INTERCOM:
					sscanf(src, "Hs%02d", &hs_id);
					sscanf(dst, "Hs%02d", &hs_id2);
					get_int_rssi_by_time(time, base_id, hs_id, hs_id2, intercom_rssi_srt, sizeof(intercom_rssi_srt));
					rssi_srt = intercom_rssi_srt;
					break;
				default:
					break;
			}


			if((rssi_srt != NULL) && (strlen(rssi_srt) > 0))
			{
    				//debug_print(" rssi_srt   ---> [%s] \n", rssi_srt);
    				json_object_object_add(jobj_cdrinfo, "rssi", json_object_new_string(rssi_srt));
				
			}
			else
			{
    				debug_print(" rssi_srt   ---> NULL \n");
			}
		}
#endif		
    	}
    
	json_object_array_add(jarr_cdrinfo, jobj_cdrinfo);
    	debug_print("\n");



    return 0;
}


static int get_column_count_callback(void *user, int argc, char **argv, 
                    char **azColName) {
	int  i = 0;
	unsigned int *count = (unsigned int*) user;
	for (int i = 0; i < argc; i++) 
	{

        	//ra_debug_print("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");

		*count = atoi(argv[i]);
	}
	return 0;

}

unsigned int get_column_count_in_time(char *db_file, char *db_table, char *start, char *end, char *columns)
{
	char buf[1024] = {0};
	unsigned int cnt = 0;
	sqlite3 *sqldb = NULL;
	
	if(get_database(db_file, &sqldb) < 0)
    		return -1;


	snprintf(buf, sizeof(buf), "SELECT COUNT(%s) FROM %s WHERE time>=\'%s\' AND time<\'%s\' AND %s!=\'NULL\'", columns, db_table, start, end, columns);

	//ra_debug_print(" Jason DEBUG %s[%d] sql [%s] !! \n", __FUNCTION__, __LINE__, buf);

	read_database(buf, sqldb, get_column_count_callback, &cnt);

	free_database(sqldb);

	return cnt;
}

int get_int_rssi_by_time(char *time, char *bsid, int hsid_1, int hsid_2, char *rssi_str, int size)
{
	int  i = 0;
	int len = rssi_num;
	struct tm time_tm;
	int bs1 = 0;
	int bs2 = 0;
	char rssi_1[8] = {0};
	char rssi_2[8] = {0};
	db_rssi_t *rssi_info;


	//debug_print(" Jason DEBUG %s[%d] bsifno has %d rssi values!! \n", __FUNCTION__, __LINE__, len);

	if((time == NULL) || (bsid == NULL))
		return -1;

	db_time_string_to_struct_tm(time, DB_TIME_FORMAT, &time_tm);
	time_t cdr_time = mktime(&time_tm);

	if(strlen(bsid) > 2)
		sscanf(bsid, "%1d,%1d", &bs1, &bs2);
	else
		bs2 = bs1 = atoi(bsid);

	if(rssi_val == NULL)
		return -1;

	rssi_info = rssi_val;

	for (i = 0; i < len; i++) 
	{
		debug_print(" Jason DEBUG %s[%d] cdr time_t [%d] bsinfo [%d]!! \n", __FUNCTION__, __LINE__, cdr_time, rssi_info->date_time);

		if((cdr_time <= (rssi_info->date_time + 1)) && (cdr_time >= (rssi_info->date_time - 1)))
		{
			//debug_print(" Jason DEBUG %s[%d] rssi_info->hsid [%d] hsid [%d]!! \n", __FUNCTION__, __LINE__, rssi_info->hsid, hsid_1);
			//debug_print(" Jason DEBUG %s[%d] rssi_info->bsid [%d] bsid [%d]!! \n", __FUNCTION__, __LINE__, rssi_info->bsid, bs1);

			if((rssi_info->hsid == hsid_1) && (rssi_info->bsid == bs1))	
			{
				strncpy(rssi_1, rssi_info->rssi, sizeof(rssi_1));
			}
		}
		rssi_info++;
	}
	
	rssi_info = rssi_val;

	for (i = 0; i < len; i++) 
	{
		//debug_print(" Jason DEBUG %s[%d] cdr time_t [%d] bsinfo [%d]!! \n", __FUNCTION__, __LINE__, cdr_time, rssi_info->date_time);

		if((cdr_time <= (rssi_info->date_time + 1)) && (cdr_time >= (rssi_info->date_time - 1)))
		{
			//debug_print(" Jason DEBUG %s[%d] rssi_info->hsid [%d] hsid [%d]!! \n", __FUNCTION__, __LINE__, rssi_info->hsid, hsid_2);
			//debug_print(" Jason DEBUG %s[%d] rssi_info->bsid [%d] bsid [%d]!! \n", __FUNCTION__, __LINE__, rssi_info->bsid, bs2);

			if((rssi_info->hsid == hsid_2) && (rssi_info->bsid == bs2))	
			{
				strncpy(rssi_2, rssi_info->rssi, sizeof(rssi_2));
			}
		}
		rssi_info++;
	}

	if((strlen(rssi_1) > 0) && (strlen(rssi_2) > 0))
	{
		snprintf(rssi_str, size, "%s,%s", rssi_1, rssi_2);
	}
	else if(strlen(rssi_1) > 0)
	{
		snprintf(rssi_str, size, "%s", rssi_1);
	}
	else if(strlen(rssi_2) > 0)
	{
		snprintf(rssi_str, size, "%s", rssi_2);
	}
	
	return 0;
}



char* get_grp_in_rssi_by_time(char *time, int bsid)
{
	int  i = 0;
	int len = rssi_num;
	struct tm time_tm;
	int in_range = 0;	
	char *rssi = NULL;
	//debug_print(" Jason DEBUG %s[%d] bsifno has %d rssi values!! \n", __FUNCTION__, __LINE__, len);

	if(time == NULL)
		return NULL;

	db_time_string_to_struct_tm(time, DB_TIME_FORMAT, &time_tm);
	time_t cdr_time = mktime(&time_tm);

	if(rssi_val == NULL)
		return NULL;

	db_rssi_t *rssi_info = rssi_val;

	for (i = 0; i < len; i++) 
	{
		//debug_print(" Jason DEBUG %s[%d] cdr time_t [%d] bsinfo [%d]!! \n", __FUNCTION__, __LINE__, cdr_time, rssi_info->date_time);

		if((cdr_time <= (rssi_info->date_time + 1)) && (cdr_time >= (rssi_info->date_time - 1)))
		{
			//debug_print(" Jason DEBUG %s[%d] rssi_info->bsid [%d] bsid [%d]!! \n", __FUNCTION__, __LINE__, rssi_info->bsid, bsid);

			if(rssi_info->bsid == bsid)
			{
				in_range++;
				rssi = rssi_info->rssi;
			}	
		}
		rssi_info++;
	}

	if(in_range != 1)
		return NULL;
	return rssi;
}
char* get_rssi_by_time(char *time, int bsid, int hsid)
{
	int  i = 0;
	int len = rssi_num;
	struct tm time_tm;
	
	//debug_print(" Jason DEBUG %s[%d] bsifno has %d rssi values!! \n", __FUNCTION__, __LINE__, len);

	if(time == NULL)
		return NULL;

	if(rssi_val == NULL)
		return NULL;

	db_time_string_to_struct_tm(time, DB_TIME_FORMAT, &time_tm);
	time_t cdr_time = mktime(&time_tm);

	db_rssi_t *rssi_info = rssi_val;

	for (i = 0; i < len; i++) 
	{
		//debug_print(" Jason DEBUG %s[%d] cdr time_t [%d] bsinfo [%d]!! \n", __FUNCTION__, __LINE__, cdr_time, rssi_info->date_time);

		if((cdr_time <= (rssi_info->date_time + 1)) && (cdr_time >= (rssi_info->date_time - 1)))
		{
			//debug_print(" Jason DEBUG %s[%d] rssi_info->hsid [%d] hsid [%d]!! \n", __FUNCTION__, __LINE__, rssi_info->hsid, hsid);
			//debug_print(" Jason DEBUG %s[%d] rssi_info->bsid [%d] bsid [%d]!! \n", __FUNCTION__, __LINE__, rssi_info->bsid, bsid);

			if((rssi_info->hsid == hsid) && (rssi_info->bsid == bsid))	
				return rssi_info->rssi;
		}
		rssi_info++;
	}

	return NULL;
}

static int get_rssi_val_callback(void *user, int argc, char **argv, 
                    char **azColName) {
	int  i = 0;

	struct tm time_tm;

	if((rssi_rows >= rssi_num) || (rssi_rows < 0))
		return 0;

	if(rssi_val == NULL)
		return 0;

	db_rssi_t *rssi_info = rssi_val + rssi_rows;
	for (int i = 0; i < argc; i++) 
	{

        //ra_debug_print("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");

		if(!strcmp(azColName[i], "call_hsid"))
		{
			if(argv[i] != NULL)
			{
				rssi_info->hsid = atoi(argv[i]);
				//ra_debug_print(" Jason DEBUG %s[%d] call_hsid!! \n", __FUNCTION__, __LINE__);
			}

		}
		else if(!strcmp(azColName[i], "bs_index"))
		{
			if(argv[i] != NULL)
			{
				rssi_info->bsid = atoi(argv[i]);
				//ra_debug_print(" Jason DEBUG %s[%d] bs_index!! \n", __FUNCTION__, __LINE__);
			}

		}
		else if(!strcmp(azColName[i], "call_rssi"))
		{
			if(argv[i] != NULL)
			{
				strncpy(rssi_info->rssi, argv[i] , sizeof(rssi_info->rssi));
				//ra_debug_print(" Jason DEBUG %s[%d] call_rssi!! \n", __FUNCTION__, __LINE__);
			}
			
		}
		else if(!strcmp(azColName[i], "time"))
		{
			if(argv[i] != NULL)
			{
				strncpy(rssi_info->time_str, argv[i] , sizeof(rssi_info->time_str));

				db_time_string_to_struct_tm(argv[i], DB_TIME_FORMAT, &time_tm);
				
				rssi_info->date_time = mktime(&time_tm);
				//ra_debug_print(" Jason DEBUG %s[%d] time!! \n", __FUNCTION__, __LINE__);

			}

		}
		else
		{

		}

	}

	rssi_rows++;

	return 0;

}
unsigned int get_rssi_vale_list(char *db_file, char *db_table, char *start, char *end, char *columns, db_rssi_t *rssi, unsigned int nu, int bs, int hs)
{
	char buf[1024] = {0};
	unsigned int cnt = 0;
	sqlite3 *sqldb = NULL;

	if(get_database(db_file, &sqldb) < 0)
    		return -1;

	//bs_index, call_hsid,

    	if((bs == 0) && (hs ==0)) 
	{
		/* search all data */
		snprintf(buf, sizeof(buf), "SELECT * FROM %s WHERE time>=\'%s\' AND time<\'%s\' AND %s!=\'NULL\'", db_table, start, end, columns);
	}
	else if((bs != 0) && (hs ==0)) 
	{
		snprintf(buf, sizeof(buf), "SELECT * FROM %s WHERE time>=\'%s\' AND time<\'%s\' AND %s!=\'NULL\' AND bs_index=%01d", db_table, start, end, columns, bs);
	}
	else if((bs == 0) && (hs !=0)) 
	{
		snprintf(buf, sizeof(buf), "SELECT * FROM %s WHERE time>=\'%s\' AND time<\'%s\' AND %s!=\'NULL\' AND call_hsid=%02d", db_table, start, end, columns, hs);
	}
	else
	{
		snprintf(buf, sizeof(buf), "SELECT * FROM %s WHERE time>=\'%s\' AND time<\'%s\' AND %s!=\'NULL\' AND call_hsid=%02d AND bs_index=%01d", db_table, start, end, columns, hs, bs);
	}


	//ra_debug_print(" Jason DEBUG %s[%d] sql [%s] !! \n", __FUNCTION__, __LINE__, buf);
	rssi_rows = 0;
	read_database(buf, sqldb, get_rssi_val_callback, NULL);

	free_database(sqldb);

	return cnt;
}

int data_base_read(int hs ,int bs, char *start_date, char *end_date, struct json_object *jarr_cdrinfo, int page_index, char *search_str)
{
 	sqlite3 *db;
	char *err_msg = 0;
	char buf[2048] = {0};

	cdr_page = page_index;

	debug_print("data_base_read \n");

	debug_print("cdr_start_date [%s] \n", start_date);
	debug_print("cdr_end_date [%s] \n", end_date);
	int rc = sqlite3_open(DB_CDR_PATH, &db);
    
	if (rc != SQLITE_OK) {
		 debug_print("open error !! \n");
	 }
	 else
	 {
		debug_print("open done !! \n");

	 }

    if (rc != SQLITE_OK) {
        
        debug_print("Cannot open database: %s\n", 
                sqlite3_errmsg(db));
        sqlite3_close(db);
        
        return 1;
    }

    	if((bs == 0) && (hs ==0)) 
	{
		/* search all data */
		sprintf(buf, "SELECT * FROM cdr WHERE start>=\'%s\' AND end<\'%s\'", 
			start_date, end_date);
	}
	else if((bs == 0) && (hs !=0)) 
	{
		sprintf(buf, "SELECT * FROM cdr WHERE start>=\'%s\' AND end<\'%s\' AND (src=\'Hs%02d\' OR dst=\'Hs%02d\')", 
			start_date, end_date, hs, hs);
	}
	else if((bs != 0) && (hs ==0)) 
	{
		sprintf(buf, "SELECT * FROM cdr WHERE start>=\'%s\' AND end<\'%s\' AND base LIKE \'%cBs%02d%c\'", 
			start_date, end_date, '%', bs, '%');
	}
	else
	{
		sprintf(buf, "SELECT * FROM cdr WHERE start>=\'%s\' AND end<\'%s\' AND base LIKE \'%cBs%02d%c\' AND (src=\'Hs%02d\' OR dst=\'Hs%02d\')", 
			start_date, end_date, '%', bs, '%', hs, hs);
	}
    
	//debug_print(" Jason DEBUG %s[%d] search_str [%s] !! \n", __FUNCTION__, __LINE__, search_str);

	if(strlen(search_str))
	{
		if(sql_cmd_add_search_string_in_cdr(buf, sizeof(buf), search_str) < 0)
		{
			debug_print(" Jason DEBUG %s[%d] sql attach search string failure !! \n", __FUNCTION__, __LINE__);
		}
			
	}

	read_out_row = 0;
	cdr_start_index = (page_index * MAX_CDR_LOG_PER_PAGE) - MAX_CDR_LOG_PER_PAGE;
	cdr_end_index =   cdr_start_index + MAX_CDR_LOG_PER_PAGE - 1 ;

	debug_print("cdr_start_index [%d] \n", cdr_start_index);
	debug_print("cdr_end_index [%d] \n", cdr_end_index);

    	char *sql = buf;    
	debug_print("sql cmd [%s] \n", sql);
	rc = sqlite3_exec(db, sql, cdr_read_callback, (void *)jarr_cdrinfo, &err_msg);



    
    if (rc != SQLITE_OK ) {
        
        debug_print("Failed to select data\n");
        debug_print("SQL error: %s\n", err_msg);

        sqlite3_free(err_msg);
        sqlite3_close(db);
        
        return 1;
    } 
    
    	sqlite3_close(db);

    return 0;
}

#define  MAX_LOG_API_JSON_LEN	6
int log_read_callback(void *jarr_log, int argc, char **argv, char **azColName)
{
	struct json_object *jobj_loginfo = NULL;
	int j = 0;
	char *db_tiem[] = {"time", "log_type", "device", "event_type", "description", NULL};
	char *json_tiem[] = {"time", "log_type", "device", "event_type", "description", NULL};
	int bs_idx, hs_idx;
	char device_string[16] = {0};

	struct json_object *jarr_loginfo = (struct json_object *)jarr_log;

	if((log_page != 0) && ((log_read_out_row < log_start_index) || (log_read_out_row > log_end_index)))
	{
		log_read_out_row++;
		return 0;
	}
	
	log_read_out_row++;

	jobj_loginfo = json_object_new_object();

	for (int i = 0; i < argc; i++) 
	{
      // debug_print("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");

		for(j = 0; j < MAX_LOG_API_JSON_LEN; j++)
		{
			if((db_tiem[j] != NULL) && (json_tiem[j] != NULL))
			{
				//debug_print(" db_tiem %d [%s], azColName %d [%s] \n", j, json_tiem[j], i, azColName[i]);
				if(!strcmp(db_tiem[j], azColName[i]))
				{
					// modify device name
					if(strcmp(db_tiem[j], "device") == 0)
					{
						if(strncmp(argv[i], "BSC", 3) == 0)
						{
							json_object_object_add(jobj_loginfo, json_tiem[j], json_object_new_string("Controller"));
						}
						else if(strncmp(argv[i], "BS", 2) == 0)
						{
							bs_idx = atoi(&argv[i][2]);

							if((bs_idx >= RAINIER_BSID_MIN) && (bs_idx <= RAINIER_BSID_MAX))
							{
								sprintf(device_string, "Base_%d", bs_idx);
								json_object_object_add(jobj_loginfo, json_tiem[j], json_object_new_string(device_string));
							}
							else
							{
								json_object_object_add(jobj_loginfo, json_tiem[j], json_object_new_string(argv[i]));
							}
						}
						else if(strncmp(argv[i], "HS", 2) == 0)
						{
							hs_idx = atoi(&argv[i][2]);

							if((hs_idx >= RAINIER_HSID_MIN) && (hs_idx <= RAINIER_HSID_MAX))
							{
								sprintf(device_string, "Handset_%d", hs_idx);
								json_object_object_add(jobj_loginfo, json_tiem[j], json_object_new_string(device_string));
							}
							else
							{
								json_object_object_add(jobj_loginfo, json_tiem[j], json_object_new_string(argv[i]));
							}
						}
						else
						{
							json_object_object_add(jobj_loginfo, json_tiem[j], json_object_new_string(argv[i]));
						}
					}
					else
					{
						json_object_object_add(jobj_loginfo, json_tiem[j], json_object_new_string(argv[i]));
						//debug_print(" add to json [%s] [%s] \n", json_tiem[j], argv[i]);
					}
				}
			}
			else
			{
				break;
			}
		}
	}
    
	json_object_array_add(jarr_loginfo, jobj_loginfo);

    return 0;
}

int log_data_base_read(int device, char *start_date, char *end_date, char *search_string, struct json_object *jarr_log, int page_index)
{
 	sqlite3 *sqldb = NULL;
	char *err_msg = 0;
	char buf[200] = {0};

	log_page = page_index;

	//debug_print("log_start_date [%s] \n", start_date);
	//debug_print("log_end_date [%s] \n", end_date);

	if(open_database(LOG_DB_TABLE, LOG_DB_COLUMNS, LOG_DB_FILE_PATH, &sqldb) < 0)
	{
		debug_print("Jason DEBUG %s[%d] open database fail !!\n", __FUNCTION__, __LINE__);
		return -1;
	}

	if(strcmp(search_string, "") == 0)	// no search string
	{
		if((device >= 1) && (device <= 8)) 
		{
			/* search BS log */
			sprintf(buf, "SELECT * FROM log WHERE time>=\'%s\' AND time<\'%s\' AND device=\'BS%d\'", 
				start_date, end_date, device);
		}
		else if(device == 9)
		{
			/* search BSC log */
			sprintf(buf, "SELECT * FROM log WHERE time>=\'%s\' AND time<\'%s\' AND device=\'BSC\'", 
				start_date, end_date);
		}
		else if((device >= 10) && (device <= 99)) 
		{
			/* search HS log */
			sprintf(buf, "SELECT * FROM log WHERE time>=\'%s\' AND time<\'%s\' AND device=\'HS%d\'", 
				start_date, end_date, device);
		}
		else //if(device == 0)
		{
			/* search all device log */
			sprintf(buf, "SELECT * FROM log WHERE time>=\'%s\' AND time<\'%s\'", 
				start_date, end_date);
		}
	}
	else
	{
		if((device >= 1) && (device <= 8)) 
		{
			/* search BS log */
			sprintf(buf, "SELECT * FROM log WHERE time>=\'%s\' AND time<\'%s\' AND device=\'BS%d\' AND (event_type like \'%%%s%%\' OR description like \'%%%s%%\')", 
				start_date, end_date, device, search_string, search_string);
		}
		else if(device == 9)
		{
			/* search BSC log */
			sprintf(buf, "SELECT * FROM log WHERE time>=\'%s\' AND time<\'%s\' AND device=\'BSC\' AND (event_type like \'%%%s%%\' OR description like \'%%%s%%\')", 
				start_date, end_date, search_string, search_string);
		}
		else if((device >= 10) && (device <= 99)) 
		{
			/* search HS log */
			sprintf(buf, "SELECT * FROM log WHERE time>=\'%s\' AND time<\'%s\' AND device=\'HS%d\' AND (event_type like \'%%%s%%\' OR description like \'%%%s%%\')", 
				start_date, end_date, device, search_string, search_string);
		}
		else //if(device == 0)
		{
			/* search all device log */
			sprintf(buf, "SELECT * FROM log WHERE time>=\'%s\' AND time<\'%s\' AND (event_type like \'%%%s%%\' OR description like \'%%%s%%\')", 
				start_date, end_date, search_string, search_string);
		}
	}
    
	log_read_out_row = 0;
	log_start_index = (page_index * RAINIER_LOG_PAGE_SIZE) - RAINIER_LOG_PAGE_SIZE;
	log_end_index = log_start_index + RAINIER_LOG_PAGE_SIZE - 1 ;

	//debug_print("log_start_index [%d] \n", log_start_index);
	//debug_print("log_end_index [%d] \n", log_end_index);

	read_database(buf, sqldb, log_read_callback, (void *)jarr_log);

	free_database(sqldb);
}

int log_data_base_write(char *log_type, char *device, char *event_type, char *description)
{
	sqlite3 *sqldb = NULL;
	char buf[256] = {0};
	char date[32] = {0};
	struct tm *tm_ptr;
	time_t now;

	if(open_database(LOG_DB_TABLE, LOG_DB_COLUMNS, LOG_DB_FILE_PATH, &sqldb) < 0)
	{
		debug_print("Jason DEBUG %s[%d] open database fail !!\n", __FUNCTION__, __LINE__);
		return -1;
	}

#if 1
	time(&now);
	tm_ptr = localtime(&now);
	strftime(date, 256, "%Y-%m-%d %H:%M:%S", tm_ptr);
	//debug_print("DateTime: %s\n", date);
#else
   	sys_interact(date, sizeof(date), "%s", "date +\"%Y-%m-%d %H:%M:%S\"");
	date[strlen(date) - 1] = '\0';
#endif

	sprintf(buf, "'%s','%s','%s','%s','%s'", date, log_type, device, event_type, description);
	
	if(sqldb != NULL)
	{
		insert_to_table(sqldb, LOG_DB_TABLE, LOG_DB_COLUMNS, buf);
		free_database(sqldb);
		return 0;
	}

    return -1;
}

int json_get_sys_info_rainier(ResponseEntry *rep, struct json_object *jobj)
{
    int cpu_load = 0, memUsage = 0, mem_total_available = 0, mem_Buffers = 0, mem_Free = 0, mem_Cached = 0;
    char *pt, fwVersion[24]={0}, cpu[5]={0}, mem[5]={0}, memTotalAvailable[5]={0}, memBuffers[5]={0}, memFree[5]={0}, memCached[5]={0}, deviceVersion[15]={0};
	char fwVersion_tmp[24]={0};
    char sysName[33]={0}, uptime[64] = {0}, date[32] = {0};
    char memBuffersSize[15]={0}, memFreeSize[15]={0}, memCachedSize[15]={0};
    int mem_total_available_size = 0, mem_capacity_size = 0, mem_usage_size = 0;
	char storage_capacity[15]={0}, storage_used[15]={0}, storage_available[15]={0}, storage_usage[5]={0};
    ResponseStatus *res = rep->res;

    char hostName[30] = {0}, modelName[30] = {0}, serial_num[30] = {0}, mac[30] = {0};

    char *mode, ip[16]={0}, mask[16]={0}, gateway[16]={0}, pDns[16]={0}, sDns[16]={0};
    char *dns_mode;
    char cur_lan_mask[32] = {0}, cur_dnslist[64] = {0}, cur_dns1[32] = {0}, cur_dns2[32] = {0};
    char cur_lan_ip[32] = {0}, cur_lan_gateway[32] = {0};
    int val = 0;


    /* mode */
    api_get_lan_proto_option(NETWORK_LAN_PROTO_OPTION, &val);
    mode = (val == 0) ? "Static" : "DHCP";

    /* dns_mode */
    val = 1; // peerdns default = 1
    api_get_integer_option("network.lan.peerdns",&val);
    dns_mode = (val == 0) ? "Static" : "DHCP";

#if SUPPORT_SP938BS_OPENAPI_SERVER
    /* current lan ip */
    sys_interact(cur_lan_ip, sizeof(cur_lan_ip), "ifconfig eth0 |grep Bcast |awk {'printf $2'} |awk -F : {'printf $2'}");

    /* current lan mask */
    sys_interact(cur_lan_mask, sizeof(cur_lan_mask), "ifconfig eth0 | grep 'inet addr' | awk -F ' ' '{print $4}' | awk -F ':' '{print $2}'");
    if ( cur_lan_mask[strlen(cur_lan_mask)-1] == '\n' )
        cur_lan_mask[strlen(cur_lan_mask)-1] = 0;

    /* current mac */
    sys_interact(mac, sizeof(mac), "ifconfig eth0 |grep HWaddr |awk  {'printf $5'}");
#else
    /* current lan ip */
    sys_interact(cur_lan_ip, sizeof(cur_lan_ip), "ifconfig br-lan |grep Bcast |awk {'printf $2'} |awk -F : {'printf $2'}");

    /* current lan mask */
    sys_interact(cur_lan_mask, sizeof(cur_lan_mask), "ifconfig br-lan | grep 'inet addr' | awk -F ' ' '{print $4}' | awk -F ':' '{print $2}'");
    if ( cur_lan_mask[strlen(cur_lan_mask)-1] == '\n' )
        cur_lan_mask[strlen(cur_lan_mask)-1] = 0;

    /* current mac */
    sys_interact(mac, sizeof(mac), "ifconfig br-lan |grep HWaddr |awk  {'printf $5'}");
#endif

    /* current lan gateway */
    sys_interact(cur_lan_gateway, sizeof(cur_lan_gateway), "route -n| grep 'UG[ \t]'|awk '{ printf $2 }'");

    sys_interact(cur_dnslist, sizeof(cur_dnslist), "cat /tmp/resolv.conf.auto | grep nameserver| grep -v ':'| cut -d ' ' -f 2");
    sscanf(cur_dnslist, "%s %s", cur_dns1, cur_dns2);
#if 1
    if(strcmp(cur_dns1, "0.0.0.0") == 0)
        strcpy(cur_dns1, "");
    if(strcmp(cur_dns2, "0.0.0.0") == 0)
        strcpy(cur_dns2, "");
#else
    if(strcmp(cur_dns1, "") == 0)
        strcpy(cur_dns1, "N/A");
    if(strcmp(cur_dns2, "") == 0)
        strcpy(cur_dns2, "N/A");
#endif
    //debug_print("============cur_dns1:[%s]==========cur_dns2:[%s]\n", cur_dns1, cur_dns2);

    /* ip */
    api_get_lan_ipaddr_option(NETWORK_LAN_IPADDR_OPTION, ip, sizeof(ip));

    /* mask */
    api_get_lan_netmask_option(NETWORK_LAN_NETMASK_OPTION, mask, sizeof(mask));

    /* gateway */
    api_get_lan_gateway_option(NETWORK_LAN_GATEWAY_OPTION, gateway, sizeof(gateway));

    /* primary_dns */
    api_get_lan_dns_option(NETWORK_LAN_DNS_OPTION, 1, pDns, sizeof(pDns));

    /* secondary_dns */
    api_get_lan_dns_option(NETWORK_LAN_DNS_OPTION, 2, sDns, sizeof(sDns));
#if 1
    if(strcmp(gateway, "0.0.0.0") == 0)
        strcpy(gateway, "");
    if(strcmp(pDns, "0.0.0.0") == 0)
        strcpy(pDns, "");
    if(strcmp(sDns, "0.0.0.0") == 0)
        strcpy(sDns, "");
#endif

    /* firmware_version */
    if(sys_check_file_existed("/etc/custom_version"))
    {
        if(sys_interact(fwVersion, sizeof(fwVersion), "cat /etc/custom_version | grep Firmware | awk \'BEGIN{FS= \" \"} {print $4}\'") > 0)
        {
            if ( (pt = strstr(fwVersion, "\n")) ) { /* delete tail "\n" */
                *pt = '\0';
            }
        }
        else
        {
            snprintf(fwVersion, sizeof(fwVersion), "%s", "unknown");
        }
    }
    else
    {
        if(sys_interact(fwVersion, sizeof(fwVersion), "cat /etc/version | grep Firmware | awk \'BEGIN{FS= \" \"} {print $4}\'") > 0)
        {
            if ( (pt = strstr(fwVersion, "\n")) ) { /* delete tail "\n" */
                *pt = '\0';
            }
        }
        else
        {
            snprintf(fwVersion, sizeof(fwVersion), "%s", "unknown");
        }
    }
	// add version prefix with letter 'v'
	if((fwVersion[0] >= '0') && (fwVersion[0] <= '9'))
	{
		strncpy(fwVersion_tmp, fwVersion, sizeof(fwVersion_tmp));
		snprintf(fwVersion, sizeof(fwVersion), "v%s", fwVersion_tmp);
	}

    /* cpu_loading */
    //cpu_load = sys_get_cpu_cur_usage();
    cpu_load = sys_get_cpu_cur_usage_by_top();
    snprintf(cpu, sizeof(cpu), "%d%%", cpu_load);
    
    /* mem_usage */
    memUsage = sys_get_mem_usage();
    snprintf(mem, sizeof(mem), "%d%%", memUsage);

    /* mem_total_available */
    mem_total_available = sys_get_mem_total_available();
    snprintf(memTotalAvailable, sizeof(memTotalAvailable), "%d%%", mem_total_available);

    /* mem_Buffers */
    mem_Buffers = sys_get_mem_buffers();
    snprintf(memBuffers, sizeof(memBuffers), "%d%%", mem_Buffers);

    /* mem_buffers_size */
    sys_interact(memBuffersSize, sizeof(memBuffersSize), "cat /proc/meminfo | grep Buffers | awk '{print $2}'");
    if ( memBuffersSize[strlen(memBuffersSize)-1] == '\n' )
        memBuffersSize[strlen(memBuffersSize)-1] = 0;

    /* mem_Free */
    mem_Free = sys_get_mem_free();
    snprintf(memFree, sizeof(memFree), "%d%%", mem_Free);

    /* mem_free_size */
    sys_interact(memFreeSize, sizeof(memFreeSize), "cat /proc/meminfo | grep MemFree | awk '{print $2}'");
    if ( memFreeSize[strlen(memFreeSize)-1] == '\n' )
        memFreeSize[strlen(memFreeSize)-1] = 0;

    /* mem_Cached */
    mem_Cached = sys_get_mem_cached();
    snprintf(memCached, sizeof(memCached), "%d%%", mem_Cached);

    /* mem_cached_size */
    sys_interact(memCachedSize, sizeof(memCachedSize), "cat /proc/meminfo | grep Cached | grep -v Swap | awk '{print $2}'");
    if ( memCachedSize[strlen(memCachedSize)-1] == '\n' )
        memCachedSize[strlen(memCachedSize)-1] = 0;

    /* mem_capacity */
    mem_capacity_size = sys_get_mem_total();

    /* mem_total_available_size */
    mem_total_available_size = atoi(memBuffersSize) + atoi(memFreeSize) + atoi(memCachedSize);

    /* mem_usage_size */
    mem_usage_size = mem_capacity_size - atoi(memFreeSize);

    /* storage_capacity */
    sys_interact(storage_capacity, sizeof(storage_capacity), "df /root/db_files | tail -n +2 | awk '{print $2}'");
    if ( storage_capacity[strlen(storage_capacity)-1] == '\n' )
        storage_capacity[strlen(storage_capacity)-1] = 0;

    /* storage_used */
    sys_interact(storage_used, sizeof(storage_used), "df /root/db_files | tail -n +2 | awk '{print $3}'");
    if ( storage_used[strlen(storage_used)-1] == '\n' )
        storage_used[strlen(storage_used)-1] = 0;

    /* storage_available */
    sys_interact(storage_available, sizeof(storage_available), "df /root/db_files | tail -n +2 | awk '{print $4}'");
    if ( storage_available[strlen(storage_available)-1] == '\n' )
        storage_available[strlen(storage_available)-1] = 0;

    /* storage_usage */
    sys_interact(storage_usage, sizeof(storage_usage), "df /root/db_files | tail -n +2 | awk '{print $5}'");
    if ( storage_usage[strlen(storage_usage)-1] == '\n' )
        storage_usage[strlen(storage_usage)-1] = 0;

    /* uptime */
    memset(uptime,0,sizeof(uptime));
    sys_get_uptime(uptime,sizeof(uptime));

    /* date */
    memset(date,0,sizeof(date));
    sys_interact(date, sizeof(date), "date");
	if ( date[strlen(date)-1] == '\n' )
        date[strlen(date)-1] = 0;

    /* system_name */
    if (api_get_system_hostname_option(SYSTEM_SYSTEM_SYSTEMNAME_OPTION, sysName, sizeof(sysName)))
    {
        RET_GEN_ERRORMSG(res, API_INTERNAL_ERROR, "GET SYSTEM HOSTNAME");
    }

    /* model_name */
    if (api_get_string_option(SYSPRODUCTINFO_MODEL_MODELNAME_OPTION, modelName, sizeof(modelName)))
    {
        RET_GEN_ERRORMSG(res, API_INTERNAL_ERROR, "GET MODEL NAME");
    }

    // Rename default model_name to DuraFon Roam
    if((strcmp(modelName, "SP938BSC") == 0) || (strcmp(modelName, "EnBSC") == 0))
    {
        strcpy(modelName, "DuraFon Roam BSC");
    }

    /* device_version */
    if ( modelName[strlen(modelName)-2] == 'v')
    {
        sprintf(deviceVersion, "%c%s", modelName[strlen(modelName)-1], ".0");
    }
    else
    {
        strcpy(deviceVersion, "1.0");
    }

    /* model name */
    api_get_system_hostname_option(SYSTEM_SYSTEM_HOSTNAME_OPTION, hostName, sizeof(hostName));
    
    /* serial number */
    memset(serial_num,0,sizeof(serial_num));
    sys_interact(serial_num, sizeof(serial_num), "setconfig -g 19");
    if ( strlen(serial_num) == 0 || strstr(serial_num,"00000000000000000000") )
    {
        memset(serial_num,0,sizeof(serial_num));
        sys_interact(serial_num, sizeof(serial_num), "setconfig -g 0");
    }
    if ( serial_num[strlen(serial_num)-1] == '\n' )
        serial_num[strlen(serial_num)-1] = 0;


    json_object_object_add(jobj, "model", json_object_new_string(modelName));
    //json_object_object_add(jobj, "device_name", json_object_new_string(sysName));
    //json_object_object_add(jobj, "product_name", json_object_new_string(hostName));
    
    json_object_object_add(jobj, "firmware_version", json_object_new_string(fwVersion));
    json_object_object_add(jobj, "device_version", json_object_new_string(deviceVersion));
    json_object_object_add(jobj, "serial_number", json_object_new_string(serial_num));

    json_object_object_add(jobj, "cpu_loading", json_object_new_string(cpu));
    json_object_object_add(jobj, "mem_capacity", json_object_new_int(mem_capacity_size));
    json_object_object_add(jobj, "mem_usage", json_object_new_string(mem));
    json_object_object_add(jobj, "mem_usage_size", json_object_new_int(mem_usage_size));
    json_object_object_add(jobj, "mem_total_available", json_object_new_string(memTotalAvailable));
    json_object_object_add(jobj, "mem_total_available_size", json_object_new_int(mem_total_available_size));
    json_object_object_add(jobj, "mem_buffers", json_object_new_string(memBuffers));
    json_object_object_add(jobj, "mem_buffers_size", json_object_new_int(atoi(memBuffersSize)));
    json_object_object_add(jobj, "mem_free", json_object_new_string(memFree));
    json_object_object_add(jobj, "mem_free_size", json_object_new_int(atoi(memFreeSize)));
    json_object_object_add(jobj, "mem_cached", json_object_new_string(memCached));
    json_object_object_add(jobj, "mem_cached_size", json_object_new_int(atoi(memCachedSize)));
	json_object_object_add(jobj, "storage_capacity", json_object_new_int(atoi(storage_capacity)));
	json_object_object_add(jobj, "storage_used", json_object_new_int(atoi(storage_used)));
	json_object_object_add(jobj, "storage_available", json_object_new_int(atoi(storage_available)));
	json_object_object_add(jobj, "storage_usage", json_object_new_string(storage_usage));
    
    json_object_object_add(jobj, "uptime", json_object_new_string(uptime));
    json_object_object_add(jobj, "date", json_object_new_string(date));
    
    json_object_object_add(jobj, "mac_address", json_object_new_string(mac));

    if (strcmp(mode, "Static") == 0)
    {
        json_object_object_add(jobj, "ip", json_object_new_string(ip));
        json_object_object_add(jobj, "mask", json_object_new_string(mask));
        json_object_object_add(jobj, "gateway", json_object_new_string(gateway));
        json_object_object_add(jobj, "primary_dns", json_object_new_string(pDns));
        json_object_object_add(jobj, "secondary_dns", json_object_new_string(sDns));
    }
    else	/* mode == DHCP */
    {
        json_object_object_add(jobj, "ip", json_object_new_string(cur_lan_ip));
        json_object_object_add(jobj, "mask", json_object_new_string(cur_lan_mask));
        json_object_object_add(jobj, "gateway", json_object_new_string(cur_lan_gateway));

        if (strcmp(dns_mode, "DHCP") == 0)
        {
            json_object_object_add(jobj, "primary_dns", json_object_new_string(cur_dns1));
            json_object_object_add(jobj, "secondary_dns", json_object_new_string(cur_dns2));
        }
        else	/* dns_mode == Static */
        {
            json_object_object_add(jobj, "primary_dns", json_object_new_string(pDns));
            json_object_object_add(jobj, "secondary_dns", json_object_new_string(sDns));
        }
    }

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int set_system_time(int y, int m, int d, char *time)
{
	char buf[128] = {0};

	if((y == 0) || (m == 0) || (d == 0) || (time == NULL))
		return -1;

	sprintf(buf, "date -s \"%d-%d-%d %s\"", y ,m ,d, time);
	system(buf);

	debug_print("%s %s \n", __FUNCTION__, buf);
	

	return 0;
}

int json_post_sys_time_config(ResponseEntry *rep, char *query_str)
{
    char *time_zone = NULL;
	char *timezone=NULL;
	char *zone_name=NULL;
	int ret = 0;
	int i = 0, length = T_NUM_OF_ELEMENTS(api_timezone_table);
	char *mode = NULL, *date = NULL, *time = NULL;
    char *ntp_server = NULL;
	int  daylight_saving = 0;
	int year = 0, month = 0, day = 0, hour = 0, minute = 0;
	struct json_object *jobj = NULL;
    ResponseStatus *res = rep->res;

	char *jobj_manual_set_string=NULL, *jobj_auto_set_string=NULL;
	struct json_object *jobj_manual_set = NULL, *jobj_auto_set = NULL;
	char buf[256] = {0};
	char cmd[256] = {0};

    if(NULL != query_str)
    {
        if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
            /* get data from api */
            	senao_json_object_get_and_create_string(rep, jobj, "mode", &mode);

		senao_json_object_get_and_create_string(rep, jobj, "Manual_set", &jobj_manual_set_string);
		if((jobj_manual_set = jsonTokenerParseFromStack(rep, jobj_manual_set_string)))
            	{	
                	senao_json_object_get_and_create_string(rep, jobj_manual_set, "date", &date);
                	senao_json_object_get_and_create_string(rep, jobj_manual_set, "time", &time);

			debug_print("%s Manual mode \n", __FUNCTION__);

                	if ( sscanf(date, "%d-%d-%d", &year, &month, &day) != 3 )
			{
				debug_print("%s date format error !! \n", __FUNCTION__);
				debug_print(" year %s date format error !! \n", year);
				debug_print(" month %s date format error !! \n", month);
				debug_print(" day %s date format error !! \n", day);

                    		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "DATE");
			}
                	if ( sscanf(time, "%d:%d", &hour, &minute) != 2 )
			{
				debug_print("%s time format error !! \n", __FUNCTION__);
				debug_print(" hour %s date format error !! \n", hour);
				debug_print(" mimute %s date format error !! \n", minute);
                    		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "TIME");
			}
            	}

		senao_json_object_get_and_create_string(rep, jobj, "Auto_set", &jobj_auto_set_string);

            	if((jobj_auto_set = jsonTokenerParseFromStack(rep, jobj_auto_set_string)))
            	{
                	senao_json_object_get_and_create_string(rep, jobj_auto_set, "ntp_server", &ntp_server);
            	}

		//senao_json_object_get_and_create_string(rep, jobj, "time_zone", &time_zone);

				/* -----------------------------------------------------------------------*/
				/* time_zone */
                //if (json_object_object_get(jobj_time, "time_zone") != NULL)
				//if(senao_json_object_get_and_create_string(rep, jobj, "time_zone", &time_zone);)
				senao_json_object_get_and_create_string(rep, jobj, "time_zone", &timezone);
                {
                   // senao_json_object_get_and_create_string(rep, jobj_time, "time_zone", &timezone);

                    for( i=0 ; i < length; i++)
                    {
                        if (strcasecmp(api_timezone_table[i].zonename, timezone) == 0)
                        {
                            ret = API_RC_SUCCESS ;
                            ret |= api_set_string_option(SYSTEM_SYSTEM_ZONENAME_OPTION, api_timezone_table[i].zonename, sizeof(api_timezone_table[i].zonename));
                            ret |= api_set_string_option(SYSTEM_SYSTEM_TIMEZONE_OPTION, api_timezone_table[i].timezone, sizeof(api_timezone_table[i].timezone));
                            ret |= api_set_integer_option(SYSTEM_SYSTEM_ZONENUMBER_OPTION, i);

							time_zone = api_timezone_table[i].timezone;
							//zone_name = api_timezone_table[i].zonename;
                            if ( ret != API_RC_SUCCESS )
                                RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "TIME ZONE");
                                break;
                        }
                    }
                        if ( i == length )
                            RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "TIME ZONE");
                }

		/* -----------------------------------------------------------------------*/
		senao_json_object_get_boolean(jobj, "daylight_saving", (bool *)&daylight_saving);

            	/* TODO - insert your code here */
		/* save time zone config  */
		snprintf(cmd, sizeof(cmd), "echo \"%s\" > /etc/TZ", time_zone);
		sys_interact(buf, sizeof(buf), cmd);
		debug_print("%s : %s \n", __FUNCTION__, cmd);

		if(!strcmp(mode, "Manual"))
		{
			/* Manual mode set system date and time from api */
			//snprintf(cmd, sizeof(cmd), "uci set system.ntp.enable_server=%d", 0);
			//sys_interact(buf, sizeof(buf), cmd);
			//debug_print("%s : %s \n", __FUNCTION__, cmd);
			api_set_integer_option("system.ntp.enable_server", 0);

			system("/etc/init.d/sysntpd stop");
			system("/etc/init.d/sysntpd disable");
			debug_print("%s : sysntpd stop !! \n", __FUNCTION__);

			set_system_time(year, month, day, time);
		}
		else if(!strcmp(mode, "Auto"))
		{
			/* auto mode , ntp enable */
			//snprintf(cmd, sizeof(cmd), "uci set system.ntp.enable_server=%d", 1);
			//sys_interact(buf, sizeof(buf), cmd);
			//debug_print("%s : %s \n", __FUNCTION__, cmd);
			api_set_integer_option("system.ntp.enable_server", 1);
		}
		else
		{
            		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "MODE");
		}
		
		debug_print("%s set_system_time mode %s \n", __FUNCTION__, mode);
		debug_print("%s set_system_time timezone %s \n", __FUNCTION__, time_zone);
		debug_print("%s set_system_time daylight_saving %d \n", __FUNCTION__, daylight_saving);
		debug_print("%s set_system_time ntp_server %s \n", __FUNCTION__, ntp_server);

		/* write data */
		//snprintf(cmd, sizeof(cmd), "uci set ntpclient.daylightsaving.DayLightEnable=%d", daylight_saving);
		//sys_interact(buf, sizeof(buf), cmd);
		//debug_print("%s : %s \n", __FUNCTION__, cmd);
		api_set_integer_option("ntpclient.daylightsaving.DayLightEnable", daylight_saving);
		
		//snprintf(cmd, sizeof(cmd), "uci set system.ntp.server='%s'", ntp_server);
		//sys_interact(buf, sizeof(buf), cmd);
		//debug_print("%s : %s \n", __FUNCTION__, cmd);
		api_set_string_option("system.ntp.server", ntp_server, sizeof(ntp_server));

		//snprintf(cmd, sizeof(cmd), "uci commit system");
		//sys_interact(buf, sizeof(buf), cmd);
		//debug_print("%s : %s \n", __FUNCTION__, cmd);
		api_commit_option("system");

		//snprintf(cmd, sizeof(cmd), "uci commit ntpclient");
		//sys_interact(buf, sizeof(buf), cmd);
		//debug_print("%s : %s \n", __FUNCTION__, cmd);
		api_commit_option("ntpclient");

		if(!strcmp(mode, "Auto"))
		{
			system("/etc/init.d/sysntpd enable");
			system("/etc/init.d/sysntpd start");
			
			debug_print("%s : sysntpd start !! \n", __FUNCTION__);
		}
        }
        else
        {
            RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
        }
    }
    else
    {
        RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
    }

    	system("sh /sbin/local_time.sh");
    	sleep(3);
	system("asterisk -rx \"core restart now\"");

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_get_sys_time_config(ResponseEntry *rep, struct json_object *jobj)
{
	char time_zone[128] = {0};
	char mode[7] = {0}, date[11] = {0}, time[6] = {0};
	    char ntp_server[256] = {0};
	int  daylight_saving = 0;
	ResponseStatus *res = rep->res;

	struct json_object *jobj_manual_set = NULL, *jobj_auto_set = NULL;

	jobj_manual_set = json_object_new_object();
    jobj_auto_set = json_object_new_object();


	/* TODO - insert your code here */
    	char cmd[256] = {0};
	char buf[256] = {0};

   	sys_interact(date, sizeof(date), "%s", "date +%Y-%m-%d");
    sys_interact(time, sizeof(time), "%s", "date +%H:%M");

	debug_print("%s %d : time[%s] \n", __FUNCTION__, __LINE__, time);
	debug_print("%s %d : date[%s] \n", __FUNCTION__, __LINE__, date);

	//snprintf(cmd, sizeof(cmd), "uci get system.ntp.server | tr -d \"\\n\"");
	//sys_interact(buf, sizeof(buf), cmd);
	//strncpy(ntp_server, buf, sizeof(ntp_server));
	api_get_string_option("system.ntp.server", ntp_server, sizeof(ntp_server));
	debug_print("%s %d : ntp_server[%s] \n", __FUNCTION__, __LINE__, ntp_server);

	//snprintf(cmd, sizeof(cmd), "uci get system.ntp.enable_server | tr -d \"\\n\"");
	//sys_interact(buf, sizeof(buf), cmd);
	api_get_string_option("system.ntp.enable_server", buf, sizeof(buf));

	if(atoi(buf) == 0)
		strncpy(mode, "Manual", sizeof(mode));
	else
		strncpy(mode, "Auto", sizeof(mode));


	debug_print("%s %d : mode[%s] \n", __FUNCTION__, __LINE__, mode);

	//snprintf(cmd, sizeof(cmd), "uci get system.@system[0].zonename | tr -d \"\\n\"");
	//sys_interact(buf, sizeof(buf), cmd);
	//strncpy(time_zone, buf, sizeof(time_zone));
	api_get_string_option("system.@system[0].zonename", time_zone, sizeof(time_zone));
	debug_print("%s %d : time_zone[%s] \n", __FUNCTION__, __LINE__, time_zone);

	//snprintf(cmd, sizeof(cmd), "uci get ntpclient.daylightsaving.DayLightEnable | tr -d \"\\n\"");
	//sys_interact(buf, sizeof(buf), cmd);
	api_get_string_option("ntpclient.daylightsaving.DayLightEnable", buf, sizeof(buf));
	daylight_saving = atoi(buf);
	debug_print("%s %d : daylight_saving[%d] \n", __FUNCTION__, __LINE__, daylight_saving);

	/* use switch api to get port date */

	/* END - insert your code here */

	
	//debug_print("uplink_port: %d\n", uplink_port);
    
    json_object_object_add(jobj, "mode", json_object_new_string(mode));
    json_object_object_add(jobj_manual_set, "date", json_object_new_string(date));
    json_object_object_add(jobj_manual_set, "time", json_object_new_string(time));
    json_object_object_add(jobj, "Manual_set", jobj_manual_set);
    json_object_object_add(jobj_auto_set, "ntp_server", json_object_new_string(ntp_server));
    json_object_object_add(jobj, "Auto_set", jobj_auto_set);
    json_object_object_add(jobj, "time_zone", json_object_new_string(time_zone));
    json_object_object_add(jobj, "daylight_saving", json_object_new_boolean(daylight_saving));

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_get_ethernet_rainier(ResponseEntry *rep, struct json_object *jobj)
{
    int val = 0, vlanEn=0, vlanId=0, vlanPr=0;
    char *mode, ip[16]={0}, mask[16]={0}, gateway[16]={0}, pDns[16]={0}, sDns[16]={0};
	char *dns_mode;
    char cur_lan_mask[32] = {0}, cur_dnslist[64] = {0}, cur_dns1[32] = {0}, cur_dns2[32] = {0};
    char cur_lan_ip[32] = {0}, cur_lan_gateway[32] = {0};
    struct json_object *jobj_ipv4;
    struct json_object *jobj_static;
    jobj_ipv4 = json_object_new_object();
    jobj_static = json_object_new_object();
    ResponseStatus *res = rep->res;

    /* mode */
    api_get_lan_proto_option(NETWORK_LAN_PROTO_OPTION, &val);
    mode = (val == 0) ? "Static" : "DHCP";

	/* dns_mode */
	val = 1; // peerdns default = 1
	api_get_integer_option("network.lan.peerdns",&val);
    dns_mode = (val == 0) ? "Static" : "DHCP";

#if SUPPORT_SP938BS_OPENAPI_SERVER
    /* current lan ip */
    sys_interact(cur_lan_ip, sizeof(cur_lan_ip), "ifconfig eth0 |grep Bcast |awk {'printf $2'} |awk -F : {'printf $2'}");

    /* current lan mask */
    sys_interact(cur_lan_mask, sizeof(cur_lan_mask), "ifconfig eth0 | grep 'inet addr' | awk -F ' ' '{print $4}' | awk -F ':' '{print $2}'");
    if ( cur_lan_mask[strlen(cur_lan_mask)-1] == '\n' )
        cur_lan_mask[strlen(cur_lan_mask)-1] = 0;
#else
    /* current lan ip */
    sys_interact(cur_lan_ip, sizeof(cur_lan_ip), "ifconfig br-lan |grep Bcast |awk {'printf $2'} |awk -F : {'printf $2'}");

    /* current lan mask */
    sys_interact(cur_lan_mask, sizeof(cur_lan_mask), "ifconfig br-lan | grep 'inet addr' | awk -F ' ' '{print $4}' | awk -F ':' '{print $2}'");
    if ( cur_lan_mask[strlen(cur_lan_mask)-1] == '\n' )
        cur_lan_mask[strlen(cur_lan_mask)-1] = 0;
#endif

    /* current lan gateway */
    sys_interact(cur_lan_gateway, sizeof(cur_lan_gateway), "route -n| grep 'UG[ \t]'|awk '{ printf $2 }'");

    sys_interact(cur_dnslist, sizeof(cur_dnslist), "cat /tmp/resolv.conf.auto | grep nameserver| grep -v ':'| cut -d ' ' -f 2");
    sscanf(cur_dnslist, "%s %s", cur_dns1, cur_dns2);
#if 1
    if(strcmp(cur_dns1, "0.0.0.0") == 0)
        strcpy(cur_dns1, "");
    if(strcmp(cur_dns2, "0.0.0.0") == 0)
        strcpy(cur_dns2, "");
#else
    if(strcmp(cur_dns1, "") == 0)
        strcpy(cur_dns1, "N/A");
    if(strcmp(cur_dns2, "") == 0)
        strcpy(cur_dns2, "N/A");
#endif
    //debug_print("============cur_dns1:[%s]==========cur_dns2:[%s]\n", cur_dns1, cur_dns2);

    /* ip */
    api_get_lan_ipaddr_option(NETWORK_LAN_IPADDR_OPTION, ip, sizeof(ip));

    /* mask */
    api_get_lan_netmask_option(NETWORK_LAN_NETMASK_OPTION, mask, sizeof(mask));

    /* gateway */
    api_get_lan_gateway_option(NETWORK_LAN_GATEWAY_OPTION, gateway, sizeof(gateway));

    /* primary_dns */
    api_get_lan_dns_option(NETWORK_LAN_DNS_OPTION, 1, pDns, sizeof(pDns));

    /* secondary_dns */
    api_get_lan_dns_option(NETWORK_LAN_DNS_OPTION, 2, sDns, sizeof(sDns));
#if 1
    if(strcmp(gateway, "0.0.0.0") == 0)
        strcpy(gateway, "");
    if(strcmp(pDns, "0.0.0.0") == 0)
        strcpy(pDns, "");
    if(strcmp(sDns, "0.0.0.0") == 0)
        strcpy(sDns, "");
#endif

    /* vlan enable */
    api_get_integer_option(NETWORK_SYSTEM_WLANVLANENABLE_OPTION, &vlanEn);

    /* vlan id */
    api_get_integer_option(NETWORK_SYSTEM_MANAGEMENTVLANID_OPTION, &vlanId);

	/* get vlan priority */
	vlanPr = 0; // default = 0
	api_get_integer_option("network.sys.ManagementVLANPriority",&vlanPr);
    

	if (strcmp(mode, "Static") == 0)
	{
		json_object_object_add(jobj_static, "ip", json_object_new_string(ip));
    	json_object_object_add(jobj_static, "mask", json_object_new_string(mask));
    	json_object_object_add(jobj_static, "gateway", json_object_new_string(gateway));
    	json_object_object_add(jobj_static, "primary_dns", json_object_new_string(pDns));
    	json_object_object_add(jobj_static, "secondary_dns", json_object_new_string(sDns));
	}
	else	/* mode == DHCP */
	{
    	json_object_object_add(jobj_static, "ip", json_object_new_string(cur_lan_ip));
    	json_object_object_add(jobj_static, "mask", json_object_new_string(cur_lan_mask));
    	json_object_object_add(jobj_static, "gateway", json_object_new_string(cur_lan_gateway));
    	
		if (strcmp(dns_mode, "DHCP") == 0)
		{
			json_object_object_add(jobj_static, "primary_dns", json_object_new_string(cur_dns1));
	    	json_object_object_add(jobj_static, "secondary_dns", json_object_new_string(cur_dns2));
		}
		else	/* dns_mode == Static */
		{
			json_object_object_add(jobj_static, "primary_dns", json_object_new_string(pDns));
    		json_object_object_add(jobj_static, "secondary_dns", json_object_new_string(sDns));
		}
	}

	json_object_object_add(jobj_ipv4, "mode", json_object_new_string(mode));
	json_object_object_add(jobj_ipv4, "dns_mode", json_object_new_string(dns_mode));

	json_object_object_add(jobj_ipv4, "Static_mode", jobj_static);

    json_object_object_add(jobj, "vlan_enable", json_object_new_boolean(vlanEn));
    json_object_object_add(jobj, "vlan_id", json_object_new_int(vlanId));
	json_object_object_add(jobj, "vlan_priority", json_object_new_int(vlanPr));
    
    json_object_object_add(jobj, "ipv4", jobj_ipv4);

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_set_ethernet_rainier(ResponseEntry *rep, char *query_str)
{
    char *mode=NULL, *p=NULL, *ip=NULL, *mask=NULL, *gateway=NULL, *pDns=NULL, *sDns=NULL, *ipv4_obj_str=NULL, *static_obj_str=NULL;
	char *dns_mode=NULL;
    int mode_val = 0, vlanId = 0 ,vlanPr=0;
    bool vlanEn = 0;
	int vlanEnPrev=0, vlanIdPrev=0;
	
	char cmd[1024] = {0};
	char buf[256] = {0};
	char device_token[256] = {0};

    struct json_object *jobj=NULL;
    struct json_object *ipv4_obj = NULL;
    struct json_object *static_obj = NULL;
    ResponseStatus *res = rep->res;

    if(NULL != query_str)
    {
        if(jobj = jsonTokenerParseFromStack(rep, query_str))
        {
            senao_json_object_get_boolean(jobj, "vlan_enable", &vlanEn);
            senao_json_object_get_integer(jobj, "vlan_id", &vlanId);
			senao_json_object_get_integer(jobj, "vlan_priority", &vlanPr);
            senao_json_object_get_and_create_string(rep, jobj, "ipv4", &ipv4_obj_str);
        }
    }
    if((ipv4_obj = jsonTokenerParseFromStack(rep, ipv4_obj_str)))
    {
        senao_json_object_get_and_create_string(rep, ipv4_obj, "mode", &mode);
		senao_json_object_get_and_create_string(rep, ipv4_obj, "dns_mode", &dns_mode);
		senao_json_object_get_and_create_string(rep, ipv4_obj, "Static_mode", &static_obj_str);
    
		//if ((strcmp(mode, "Static") == 0) || (strcmp(dns_mode, "Static") == 0))
    	//{
	        if((static_obj = jsonTokenerParseFromStack(rep, static_obj_str)))
        	{
	            senao_json_object_get_and_create_string(rep, static_obj, "ip", &ip);
            	senao_json_object_get_and_create_string(rep, static_obj, "mask", &mask);
            	senao_json_object_get_and_create_string(rep, static_obj, "gateway", &gateway);
            	senao_json_object_get_and_create_string(rep, static_obj, "primary_dns", &pDns);
            	senao_json_object_get_and_create_string(rep, static_obj, "secondary_dns", &sDns);
        	}
    	//}
	}


	//if ((vlanId < 2) || (vlanId > 4094))
	if ((vlanId < 2) || (vlanId > 4093))	// 4094 is for BSC-SWITCH internal use
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "VLAN ID");
	
	if ((vlanPr < 0) || (vlanPr > 7))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "VLAN PRIORITY");

	
    /* mode */
    for ( p = mode; *p; ++p) *p = tolower(*p);
    mode_val = ((strcmp(mode,"static") == 0)?0:1); 
    if (api_set_lan_proto_option(NETWORK_LAN_PROTO_OPTION, mode_val)) 
        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "MODE");

	/* dns_mode */
	for ( p = dns_mode; *p; ++p) *p = tolower(*p);
	mode_val = ((strcmp(dns_mode,"static") == 0)?0:1); 
	if (api_set_integer_option("network.lan.peerdns", mode_val))
	    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "DNS_MODE");

#if 1
    if(strcmp(gateway, "") == 0)
        strcpy(gateway, "0.0.0.0");
    if(strcmp(pDns, "") == 0)
        strcpy(pDns, "0.0.0.0");
    if(strcmp(sDns, "") == 0)
        strcpy(sDns, "0.0.0.0");
#endif

    /* clear dnsmasq server list */
    //SYSTEM("uci del dhcp.@dnsmasq[0].server");
	api_delete_option("dhcp.@dnsmasq[0].server", "");

    if (strcmp(mode, "static") == 0)
    {
        /* ip */
        if (api_set_lan_ipaddr_option(NETWORK_LAN_IPADDR_OPTION, ip, strlen(ip)))
            RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "IP");

        /* mask */
        if (api_set_lan_netmask_option(NETWORK_LAN_NETMASK_OPTION, mask, strlen(mask)))
            RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "MASK");

        /* gateway */
        if (api_set_lan_gateway_option(NETWORK_LAN_GATEWAY_OPTION, gateway, strlen(gateway)))
            RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "GATEWAY");

        /* primary_dns */
        if (api_set_lan_dns_option(NETWORK_LAN_DNS_OPTION, 1, pDns, strlen(pDns)))
            RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "PRIMARY DNS");

        /* secondary_dns */
        if (api_set_lan_dns_option(NETWORK_LAN_DNS_OPTION, 2, sDns, strlen(sDns)))
            RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "SECONDARY DNS");
    }
	else	/* mode == dhcp */
    {
		if (strcmp(dns_mode, "static") == 0)
		{
        	/* primary_dns */
        	if (api_set_lan_dns_option(NETWORK_LAN_DNS_OPTION, 1, pDns, strlen(pDns)))
	            RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "PRIMARY DNS");

        	/* secondary_dns */
        	if (api_set_lan_dns_option(NETWORK_LAN_DNS_OPTION, 2, sDns, strlen(sDns)))
	            RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "SECONDARY DNS");

			/* set dnsmasq server list */
			if (api_check_dns_addr(pDns)==true)
			{
				//SYSTEM("uci add_list dhcp.@dnsmasq[0].server=\"%s\"", pDns);
				api_add_list("dhcp.@dnsmasq[0].server", pDns, strlen(pDns));
			}
			if (api_check_dns_addr(sDns)==true)
			{
				//SYSTEM("uci add_list dhcp.@dnsmasq[0].server=\"%s\"", sDns);
				api_add_list("dhcp.@dnsmasq[0].server", sDns, strlen(sDns));
			}
		}
    }
	//SYSTEM("uci commit dhcp");
	api_commit_option("dhcp");

    /* vlan enable prev */
    api_get_integer_option(NETWORK_SYSTEM_WLANVLANENABLE_OPTION, &vlanEnPrev);

    /* vlan id prev */
    api_get_integer_option(NETWORK_SYSTEM_MANAGEMENTVLANID_OPTION, &vlanIdPrev);

	/* vlan enable */
    if (api_set_integer_option(NETWORK_SYSTEM_WLANVLANENABLE_OPTION, vlanEn))
        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "VLAN ENABLE");

	/* vlan id */
	if (api_set_integer_option(NETWORK_SYSTEM_MANAGEMENTVLANID_OPTION, vlanId))
        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "VLAN ID");
		
	/* set vlan priority */
	if (api_set_integer_option("network.sys.ManagementVLANPriority", vlanPr))
        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "VLAN PRIORITY");

#if !SUPPORT_SP938BS_OPENAPI_SERVER
    if (vlanEn == true) 
    {
#if SUPPORT_SP938BSC_OPENAPI_SERVER
		// For SP938BSC
		api_delete_option("network.@switch_vlan[2]", "");
		api_add_new_section("network", "switch_vlan");
		api_set_string_option("network.@switch_vlan[2].device", "switch0", sizeof("switch0"));
		api_set_integer_option("network.@switch_vlan[2].vlan", vlanId);
		api_set_string_option("network.@switch_vlan[2].ports", "0t 1t 2t 3t 4t 5t", sizeof("0t 1t 2t 3t 4t 5t"));

		snprintf(buf, sizeof(buf), "eth0.%d", vlanId);
		api_set_string_option("network.lan.ifname", buf, sizeof(buf));
#else
		// For EnBSC
		api_delete_option("network.@switch_vlan[1]", "");
		api_add_new_section("network", "switch_vlan");
		api_set_string_option("network.@switch_vlan[1].device", "switch0", sizeof("switch0"));
		api_set_integer_option("network.@switch_vlan[1].vlan", vlanId);
		api_set_string_option("network.@switch_vlan[1].ports", "0t 4t 5t", sizeof("0t 4t 5t"));

		snprintf(buf, sizeof(buf), "eth1.%d", vlanId);
		api_set_string_option("network.lan.ifname", buf, sizeof(buf));
#endif

		if ((vlanEnPrev != vlanEn) || (vlanIdPrev != vlanId))
		{
			/* remove switch old vlan and add new vlan setting */
			snprintf(cmd, sizeof(cmd), "curl --connect-timeout 3 --max-time 20 -v -k -X PATCH \"http://%s:8000/api/system/login\" -H \"accept: */*\" -H \"Content-Type: application/json\" -d \"{\\\"username\\\":\\\"admin\\\",\\\"password\\\":\\\"password\\\"}\" | grep token | awk -F\\\" '{for (i=1;i<=NF;i++){if ($i ~/token/) {print $(i+2)}}}' | tr -d \"\\n\"", SWITCH_IP);
			debug_print("cmd: %s.", cmd);
			sys_interact_long(device_token, sizeof(device_token), cmd);
			debug_print("device_token: %s.", device_token);

			if(strcmp(device_token, "---") == 0)
			{
				debug_print("switch api login fail.");
			}

			snprintf(cmd, sizeof(cmd), "curl --connect-timeout 3 --max-time 20 -v -k -X POST \"http://%s:8000/api/delete_vlans\" -H \"accept: */*\" -H \"Authorization: Bearer %s\" -d \"[{\\\"vlanID\\\":%d}]\" | awk -F\\\" '{for (i=1;i<=NF;i++){if ($i ~/errCode/) {print $(i+1)}}}' | tr -d \"\\n\"", SWITCH_IP, device_token, vlanIdPrev);
			debug_print("cmd: %s.", cmd);
			sys_interact_long(buf, sizeof(buf), cmd);
			debug_print("buf: %s.", buf);

			if(strcmp(buf, ":0,") != 0) // OK
			{
				debug_print("switch delete vlan fail.");
			}
			
			snprintf(cmd, sizeof(cmd), "curl --connect-timeout 3 --max-time 20 -v -k -X POST \"http://%s:8000/api/vlans\" -H \"accept: */*\" -H \"Authorization: Bearer %s\" -d \"[{\\\"vlanID\\\":%d,\\\"vlanName\\\":\\\"vlan%d\\\",\\\"tagged_ports\\\":\\\"1-12,t1-t8\\\",\\\"untagged_ports\\\":\\\"\\\"}]\" | awk -F\\\" '{for (i=1;i<=NF;i++){if ($i ~/errCode/) {print $(i+1)}}}' | tr -d \"\\n\"", SWITCH_IP, device_token, vlanId, vlanId);
			debug_print("cmd: %s.", cmd);
			sys_interact_long(buf, sizeof(buf), cmd);
			debug_print("buf: %s.", buf);

			if(strcmp(buf, ":0,") != 0) // OK
			{
				debug_print("switch set vlan fail.");
			}

			snprintf(cmd, sizeof(cmd), "curl --connect-timeout 3 --max-time 20 -v -k -X POST \"http://%s:8000/api/system/save\" -H \"accept: */*\" -H \"Authorization: Bearer %s\" | awk -F\\\" '{for (i=1;i<=NF;i++){if ($i ~/errCode/) {print $(i+1)}}}' | tr -d \"\\n\"", SWITCH_IP, device_token);
			debug_print("cmd: %s.", cmd);
			sys_interact_long(buf, sizeof(buf), cmd);
			debug_print("buf: %s.", buf);

			if(strcmp(buf, ":0,") != 0) // OK
			{
				debug_print("switch save fail.");
			}

			snprintf(cmd, sizeof(cmd), "curl --connect-timeout 3 --max-time 20 -v -k -X PATCH \"http://%s:8000/api/system/logout\" -H \"accept: */*\" -H \"Authorization: Bearer %s\" | awk -F\\\" '{for (i=1;i<=NF;i++){if ($i ~/errCode/) {print $(i+1)}}}' | tr -d \"\\n\"", SWITCH_IP, device_token);
			debug_print("cmd: %s.", cmd);
			sys_interact_long(buf, sizeof(buf), cmd);
			debug_print("buf: %s.", buf);

			if(strcmp(buf, ":0,") != 0) // OK
			{
				debug_print("switch logout fail.");
			}
		}
    }
	else
	{
#if SUPPORT_SP938BSC_OPENAPI_SERVER
		// For SP938BSC
		api_delete_option("network.@switch_vlan[2]", "");

		api_set_string_option("network.lan.ifname", "eth0", sizeof("eth0"));
#else
		// For EnBSC
		api_delete_option("network.@switch_vlan[1]", "");

		api_set_string_option("network.lan.ifname", "eth1", sizeof("eth1"));
#endif

		if (vlanEnPrev == true)
		{
			/* remove switch vlan setting */
			snprintf(cmd, sizeof(cmd), "curl --connect-timeout 3 --max-time 20 -v -k -X PATCH \"http://%s:8000/api/system/login\" -H \"accept: */*\" -H \"Content-Type: application/json\" -d \"{\\\"username\\\":\\\"admin\\\",\\\"password\\\":\\\"password\\\"}\" | grep token | awk -F\\\" '{for (i=1;i<=NF;i++){if ($i ~/token/) {print $(i+2)}}}' | tr -d \"\\n\"", SWITCH_IP);
			debug_print("cmd: %s.", cmd);
			sys_interact_long(device_token, sizeof(device_token), cmd);
			debug_print("device_token: %s.", device_token);

			if(strcmp(device_token, "---") == 0)
			{
				debug_print("switch api login fail.");
			}

			snprintf(cmd, sizeof(cmd), "curl --connect-timeout 3 --max-time 20 -v -k -X POST \"http://%s:8000/api/delete_vlans\" -H \"accept: */*\" -H \"Authorization: Bearer %s\" -d \"[{\\\"vlanID\\\":%d}]\" | awk -F\\\" '{for (i=1;i<=NF;i++){if ($i ~/errCode/) {print $(i+1)}}}' | tr -d \"\\n\"", SWITCH_IP, device_token, vlanIdPrev);
			debug_print("cmd: %s.", cmd);
			sys_interact_long(buf, sizeof(buf), cmd);
			debug_print("buf: %s.", buf);

			if(strcmp(buf, ":0,") != 0) // OK
			{
				debug_print("switch delete vlan fail.");
			}
			
			snprintf(cmd, sizeof(cmd), "curl --connect-timeout 3 --max-time 20 -v -k -X POST \"http://%s:8000/api/system/save\" -H \"accept: */*\" -H \"Authorization: Bearer %s\" | awk -F\\\" '{for (i=1;i<=NF;i++){if ($i ~/errCode/) {print $(i+1)}}}' | tr -d \"\\n\"", SWITCH_IP, device_token);
			debug_print("cmd: %s.", cmd);
			sys_interact_long(buf, sizeof(buf), cmd);
			debug_print("buf: %s.", buf);

			if(strcmp(buf, ":0,") != 0) // OK
			{
				debug_print("switch save fail.");
			}

			snprintf(cmd, sizeof(cmd), "curl --connect-timeout 3 --max-time 20 -v -k -X PATCH \"http://%s:8000/api/system/logout\" -H \"accept: */*\" -H \"Authorization: Bearer %s\" | awk -F\\\" '{for (i=1;i<=NF;i++){if ($i ~/errCode/) {print $(i+1)}}}' | tr -d \"\\n\"", SWITCH_IP, device_token);
			debug_print("cmd: %s.", cmd);
			sys_interact_long(buf, sizeof(buf), cmd);
			debug_print("buf: %s.", buf);

			if(strcmp(buf, ":0,") != 0) // OK
			{
				debug_print("switch logout fail.");
			}
		}
	}
	
#endif  /* #if !SUPPORT_SP938BS_OPENAPI_SERVER */

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

process_table_t process_table[] = 
{
    {"asterisk",	"",	"asterisk"},
    {"bsc_monitor",	"",	"bsc-monitor"},
	{"nmsconfd",	"",	"nms"}
};

int process_table_len = (sizeof(process_table)/sizeof(process_table[0]));

int json_get_mgm_process_status(ResponseEntry *rep, struct json_object *jobj)
{
	int process_status = 0;
	int i;
	char cmd[1024] = {0};
	char buf[256] = {0};
	ResponseStatus *res = rep->res;

	struct json_object *jarr_process_list = NULL, *jobj_process_list = NULL;
	jarr_process_list = json_object_new_array();

	for(i = 0; i < process_table_len; i++)
	{
		/* check process status */
		snprintf(cmd, sizeof(cmd), "ps | grep %s | grep -v grep", process_table[i].name);
		sys_interact(buf, sizeof(buf), cmd);
		if(strlen(buf) != 0)
		{
			process_status = 1;
		}
		else
		{
			process_status = 0;
		}

		jobj_process_list = json_object_new_object();
		json_object_object_add(jobj_process_list, "process_name", json_object_new_string(process_table[i].name));
		json_object_object_add(jobj_process_list, "process_description", json_object_new_string(process_table[i].description));
		json_object_object_add(jobj_process_list, "process_status", json_object_new_int(process_status));
		json_object_array_add(jarr_process_list, jobj_process_list);
	}

	json_object_object_add(jobj, "process_list", jarr_process_list);

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_set_mgm_process_restart(ResponseEntry *rep, char *query_str)
{
	char *process_name = NULL;
	int i;
	struct json_object *jobj = NULL;
	ResponseStatus *res = rep->res;

	if(NULL != query_str)
	{
		if((jobj = jsonTokenerParseFromStack(rep, query_str)))
		{
			senao_json_object_get_and_create_string(rep, jobj, "process_name", &process_name);
		}
		else
		{
			RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
		}
	}
	else
	{
		RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
	}

	for(i = 0; i < process_table_len; i++)
	{
		/* find the process */
		if(strcmp(process_table[i].name, process_name) == 0)
		{
			SYSTEM("/etc/init.d/%s restart", process_table[i].restart_script);
			
			RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
		}
	}

	RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "PROCESS NAME");
}

int json_get_mgm_port_status(ResponseEntry *rep, struct json_object *jobj)
{
	char *power_draw = NULL;
	int uplink_port = -1, maxbudget = 0;
	int port_id = 0;
	bool enable = 0;
	char link_speed[16] = {0};
	int idx;
	int total_ports = 9;
	ResponseStatus *res = rep->res;

	char cmd[1024] = {0};
	char buf[8192] = {0}, cmd_debug[1024] = {0};
	char device_token[1024] = {0};

	int i = 0;
	int bs_index;

	char *restful_res_str = NULL, *mgmtInterface_str = NULL;
	char *sys_info_str = NULL, port_poe_str = NULL;

	struct json_object *res_jobj = NULL;
	struct json_object *if_jobj = NULL;
	struct json_object *mgm_jobj = NULL;
	struct json_object *port_jobj = NULL;
	struct json_object *port_cfg_jobj = NULL;
	struct json_object *port_poe_cfg_jobj = NULL;
	struct json_object *jarr_obj = NULL;
	struct json_object *poe_jobj = NULL;
	struct json_object *lldp_jobj = NULL;
	struct json_object *lldp_remote_info_jobj = NULL;

    int portid[9] = {-1};
	bool port_enable[9] = {-1};
	char *port_link_speed[9] = {NULL};

	int port_no[9] = {0};
	bool poe_enable[9] = {-1};
	char *poe_power_draw[9] = {NULL};
	char *poe_status[9] = {NULL};
	
	int lldp_port_no[9] = {0};
	char lldp_chassis_id[9][20] = {0};
	char lldp_remote_ip[9][20] = {0};
	char lldp_sys_name[9][20] = {0};
	int lldp_portNo = 0;
	char *lldp_chassisID = NULL;
	char *lldp_remoteIp = NULL;
	char *lldp_sysName = NULL;

	int arraylen = 0;
	int poe_port_num = 0;

	struct json_object *sys_status_jobj = NULL, *sys_info_jobj = NULL;

	struct json_object *jarr_port_status = NULL, *jobj_port_status = NULL;
	jarr_port_status = json_object_new_array();

	struct json_object *jarr_poe_status = NULL, *jobj_poe_status = NULL;
	jarr_poe_status = json_object_new_array();

	struct json_object *jarr_lldp_status = NULL, *jobj_lldp_status = NULL;
	jarr_lldp_status = json_object_new_array();


	/* TODO - insert your code here */
	snprintf(cmd, sizeof(cmd), "curl --connect-timeout 3 --max-time 20 -X PATCH \"http://%s:8000/api/system/login\" -H \"accept: application/json\" -H \"Content-Type: application/json\" -d \"{'user':'admin','password':'password'}\" | grep token | awk -F\\\" '{for (i=1;i<=NF;i++){if ($i ~/token/) {print $(i+2)}}}' | tr -d \"\\n\"", SWITCH_IP);
	sys_interact_long(device_token, sizeof(device_token), cmd);
	//debug_print("device_token: %s. \n", device_token);



	snprintf(cmd, sizeof(cmd), "curl --connect-timeout 3 --max-time 20 -v -k -X GET \"http://%s:8000/api/system/status\" -H \"accept: */*\" -H \"Content-Type: application/json\" -H \"Authorization: Bearer %s\"", SWITCH_IP, device_token); 
	sys_interact_long(buf, sizeof(buf), cmd);
	//debug_print("-- buf: %s. \n\n\n", buf);

	if((sys_status_jobj = jsonTokenerParseFromStack(rep, buf)))
	{
		senao_json_object_get_and_create_string(rep, sys_status_jobj, "restful_res", &restful_res_str);
		if(( res_jobj = jsonTokenerParseFromStack(rep, restful_res_str)))
		{
			
			senao_json_object_get_and_create_string(rep, res_jobj, "systemInfo", &sys_info_str);
			if((sys_info_jobj = jsonTokenerParseFromStack(rep, sys_info_str)))
			{
				senao_json_object_get_integer(sys_info_jobj, "maxbudget", &maxbudget);
				senao_json_object_get_and_create_string(rep, sys_info_jobj, "powerDraw", &power_draw);

				debug_print("maxbudget[%d] \n", maxbudget);
				debug_print("powerDraw[%s] \n", power_draw);
				
			}
		}

		
	}

	snprintf(cmd, sizeof(cmd), "curl --connect-timeout 3 --max-time 20 -v -k -X GET \"http://%s:8000/api/system/settings/mgmtinterface\" -H \"accept: */*\" -H \"Content-Type: application/json\" -H \"Authorization: Bearer %s\"", SWITCH_IP, device_token); 
	sys_interact_long(buf, sizeof(buf), cmd);
	//debug_print("-- buf: %s. \n\n\n", buf);

	if((mgm_jobj = jsonTokenerParseFromStack(rep, buf)))
	{

		senao_json_object_get_and_create_string(rep, mgm_jobj, "restful_res", &restful_res_str);
		//debug_print("-- restful_res_str: %s. \n\n\n", restful_res_str);
		if((res_jobj = jsonTokenerParseFromStack(rep, restful_res_str)))
		{
			senao_json_object_get_and_create_string(rep, res_jobj, "mgmtInterface", &mgmtInterface_str);
			//debug_print("-- mgmtInterface: %s. \n\n\n", mgmtInterface_str);

			if((if_jobj = jsonTokenerParseFromStack(rep, mgmtInterface_str)))
			{
				senao_json_object_get_integer(if_jobj, "uplinkPort", &uplink_port);
				debug_print("uplink_port[%d] \n", uplink_port);
			}
			
		}
		

	}

	snprintf(cmd, sizeof(cmd), "curl --connect-timeout 3 --max-time 20 -v -k -X GET \"http://%s:8000/api/ports\" -H \"accept: */*\" -H \"Content-Type: application/json\" -H \"Authorization: Bearer %s\"", SWITCH_IP, device_token); 
	sys_interact_long(buf, sizeof(buf), cmd);
	//debug_print("-- buf: %s. \n\n\n", buf);



    if((port_jobj = jsonTokenerParseFromStack(rep, buf)))
    {
		senao_json_object_get_and_create_string(rep, port_jobj, "restful_res", &restful_res_str);
			
		if((res_jobj = jsonTokenerParseFromStack(rep, restful_res_str)))
		{
			port_cfg_jobj = json_object_object_get(res_jobj, "portConfs");
           	arraylen = json_object_array_length(port_cfg_jobj) ;

			for (i = 0; i < total_ports; i++) 
            {
				jarr_obj = json_object_array_get_idx(port_cfg_jobj, i);

				senao_json_object_get_integer(jarr_obj, "portID", &portid[i]);
				senao_json_object_get_boolean(jarr_obj, "enable", &port_enable[i]);
				senao_json_object_get_and_create_string(rep, jarr_obj, "linkSpeed", &port_link_speed[i]);

				debug_print("portID[%d] \n", portid[i]);
				debug_print("enable_flag[%d] \n", port_enable[i]);
				debug_print("link_speed[%s] \n", port_link_speed[i]);			
				
			}
		}
	}


	snprintf(cmd, sizeof(cmd), "curl --connect-timeout 3 --max-time 20 -v -k -X GET \"http://%s:8000/api/ports/poe\" -H \"accept: */*\" -H \"Content-Type: application/json\" -H \"Authorization: Bearer %s\"", SWITCH_IP, device_token); 
	sys_interact_long(buf, sizeof(buf), cmd);

	//debug_print("-- poe buf: %s. \n\n\n", buf);
   	
	if((poe_jobj = jsonTokenerParseFromStack(rep, buf)))
    {
		senao_json_object_get_and_create_string(rep, poe_jobj, "restful_res", &restful_res_str);
		//debug_print("****  restful_res_str: %s. \n\n\n", restful_res_str);
		if((res_jobj = jsonTokenerParseFromStack(rep, restful_res_str)))
		{
			port_poe_cfg_jobj = json_object_object_get(res_jobj, "portPoeConfs");

           	poe_port_num  = json_object_array_length(port_poe_cfg_jobj) ;		

			for (i = 0; i < poe_port_num; i++) 
            {
				jarr_obj = json_object_array_get_idx(port_poe_cfg_jobj, i);

				senao_json_object_get_integer(jarr_obj, "portNo", &port_no[i]);
				senao_json_object_get_boolean(jarr_obj, "enable", &poe_enable[i]);
				senao_json_object_get_and_create_string(rep, jarr_obj, "powerDraw", &poe_power_draw[i]);
				senao_json_object_get_and_create_string(rep, jarr_obj, "status", &poe_status[i]);
				
				debug_print("portID[%d] \n", port_no[i]);
				debug_print("poe_enable[%d] \n", poe_enable[i]);
				debug_print("poe_power_draw[%s] \n", poe_power_draw[i]);
				debug_print("poe_status[%s] \n", poe_status[i]);
			}
		}
	}

	snprintf(cmd, sizeof(cmd), "curl --connect-timeout 3 --max-time 20 -v -k -X GET \"http://%s:8000/api/lldp/remote\" -H \"accept: */*\" -H \"Content-Type: application/json\" -H \"Authorization: Bearer %s\"", SWITCH_IP, device_token); 
	sys_interact_long(buf, sizeof(buf), cmd);

	//debug_print("-- lldp buf: %s. \n\n\n", buf);
   	
	if((lldp_jobj = jsonTokenerParseFromStack(rep, buf)))
    {
		senao_json_object_get_and_create_string(rep, lldp_jobj, "restful_res", &restful_res_str);
		//debug_print("****  restful_res_str: %s. \n\n\n", restful_res_str);
		if((res_jobj = jsonTokenerParseFromStack(rep, restful_res_str)))
		{
			lldp_remote_info_jobj = json_object_object_get(res_jobj, "lldpRemoteInfo");

           	arraylen  = json_object_array_length(lldp_remote_info_jobj) ;		

			for (i = 0; i < arraylen; i++) 
            {
				jarr_obj = json_object_array_get_idx(lldp_remote_info_jobj, i);

				senao_json_object_get_integer(jarr_obj, "portNo", &lldp_portNo);
				senao_json_object_get_and_create_string(rep, jarr_obj, "chassisID", &lldp_chassisID);
				senao_json_object_get_and_create_string(rep, jarr_obj, "remoteIp", &lldp_remoteIp);
				senao_json_object_get_and_create_string(rep, jarr_obj, "sysName", &lldp_sysName);
				
				debug_print("lldp_portNo[%d] \n", lldp_portNo);
				debug_print("lldp_chassisID[%s] \n", lldp_chassisID);
				debug_print("lldp_remoteIp[%s] \n", lldp_remoteIp);
				debug_print("lldp_sysName[%s] \n", lldp_sysName);

				if((lldp_portNo > total_ports) || (lldp_portNo < 1))
				{
					continue;
				}

				if (lldp_port_no[lldp_portNo-1] == 0)	// first time found an lldp device on this port
				{
					lldp_port_no[lldp_portNo-1] = lldp_portNo;
					strncpy(lldp_chassis_id[lldp_portNo-1], lldp_chassisID, sizeof(lldp_chassis_id[lldp_portNo-1]));
					strncpy(lldp_remote_ip[lldp_portNo-1], lldp_remoteIp, sizeof(lldp_remote_ip[lldp_portNo-1]));
					strncpy(lldp_sys_name[lldp_portNo-1], lldp_sysName, sizeof(lldp_sys_name[lldp_portNo-1]));
				}
				else	// more than one lldp device on this port, set sys_name to Unknown Device
				{
					lldp_port_no[lldp_portNo-1] = lldp_portNo;
					strncpy(lldp_chassis_id[lldp_portNo-1], "00:00:00:00:00:00", sizeof(lldp_chassis_id[lldp_portNo-1]));
					strncpy(lldp_remote_ip[lldp_portNo-1], "0.0.0.0", sizeof(lldp_remote_ip[lldp_portNo-1]));
					strncpy(lldp_sys_name[lldp_portNo-1], "Unknown Device", sizeof(lldp_sys_name[lldp_portNo-1]));
				}
			}
		}
	}

	snprintf(cmd, sizeof(cmd), "curl --connect-timeout 3 --max-time 20 -v -k -X PATCH \"http://%s:8000/api/system/logout\" -H \"accept: */*\" -H \"Authorization: Bearer %s\" | awk -F\\\" '{for (i=1;i<=NF;i++){if ($i ~/errCode/) {print $(i+1)}}}' | tr -d \"\\n\"", SWITCH_IP, device_token);
	sys_interact_long(buf, sizeof(buf), cmd);
	
	//debug_print("uplink_port: %d\n", uplink_port);
    json_object_object_add(jobj, "uplink_port", json_object_new_int(uplink_port));
	json_object_object_add(jobj, "maxbudget", json_object_new_int(maxbudget));
	json_object_object_add(jobj, "power_draw", json_object_new_string(power_draw));

	for(idx = 0; idx < total_ports; idx++)
	{
		jobj_port_status = json_object_new_object();
		json_object_object_add(jobj_port_status, "port_id", json_object_new_int(portid[idx]));
		json_object_object_add(jobj_port_status, "enable", json_object_new_boolean(port_enable[idx]));
		json_object_object_add(jobj_port_status, "link_speed", json_object_new_string(port_link_speed[idx]));
		json_object_array_add(jarr_port_status, jobj_port_status);
	}

	json_object_object_add(jobj, "port_status", jarr_port_status);

	for(idx = 0; idx < poe_port_num; idx++)
	{
		jobj_poe_status = json_object_new_object();
		json_object_object_add(jobj_poe_status, "port_id", json_object_new_int( port_no[idx]));
		json_object_object_add(jobj_poe_status, "enable", json_object_new_boolean(poe_enable[idx]));
		json_object_object_add(jobj_poe_status, "power_draw", json_object_new_string(poe_power_draw[idx]));
		json_object_object_add(jobj_poe_status, "status", json_object_new_string(poe_status[idx]));
		json_object_array_add(jarr_poe_status, jobj_poe_status);
	}

	json_object_object_add(jobj, "poe_status", jarr_poe_status);

	for(idx = 0; idx < total_ports; idx++)
	{
		jobj_lldp_status = json_object_new_object();
		
		json_object_object_add(jobj_lldp_status, "port_id", json_object_new_int(idx+1));

		//debug_print("lldp_portNo[%d] = %d \n", idx+1, lldp_port_no[idx]);
		
		if(lldp_port_no[idx])	// this port found lldp device
		{
			bs_index = findBsConfigIndexByMac(lldp_chassis_id[idx]);

			//debug_print("bs_index = %d \n", bs_index);

			if(bs_index >= 0)	// SP938BS connect to this port
			{
				sprintf(buf, "Base_%d", bs_index);
				json_object_object_add(jobj_lldp_status, "name", json_object_new_string(buf));
			}
			else if(strcmp(lldp_sys_name[idx], "SP938BS") == 0)
			{
				//json_object_object_add(jobj_lldp_status, "name", json_object_new_string(lldp_sys_name[idx]));
				json_object_object_add(jobj_lldp_status, "name", json_object_new_string("Discovered Base"));
			}
			else
			{
				//json_object_object_add(jobj_lldp_status, "name", json_object_new_string(lldp_sys_name[idx]));
				json_object_object_add(jobj_lldp_status, "name", json_object_new_string("Unknown Device"));
			}
		}
		else	// this port not found lldp device
		{
			if(strcmp(port_link_speed[idx], "Disabled") != 0)	// this port is connected to an device
			{
				json_object_object_add(jobj_lldp_status, "name", json_object_new_string("Unknown Device"));
			}
			else
			{
				json_object_object_add(jobj_lldp_status, "name", json_object_new_string("N/A"));
			}
		}
		json_object_array_add(jarr_lldp_status, jobj_lldp_status);
	}

	json_object_object_add(jobj, "lldp_status", jarr_lldp_status);

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}


int get_sdcard_mount_info(char *sd_dir, int size, int rescan)
{

	char buf[128] = {0};

        sys_interact(buf, sizeof(buf), "df | grep mnt/sd | awk '{print $6}'");

	if(strlen(buf) > 1)
	{
		if(strlen(buf) >= size)
			return -1;
		buf[strlen(buf) - 1] = '\0';
		strncpy(sd_dir, buf, size);
		ra_debug_print("%s -> sdcard  mount at [%s]", __FUNCTION__, buf);
		return 0;
	}


	if(rescan)
	{
		system("udevadm trigger");
		sleep(20);

        	sys_interact(buf, sizeof(buf), "df | grep mnt/sd | awk '{print $6}'");

		if((strlen(buf) < 1) || (strlen(buf) >= size))
			return -1;

		buf[strlen(buf) - 1] = '\0';
		strncpy(sd_dir, buf, size);

		ra_debug_print("%s -> sdcard  mount a t [%s]", __FUNCTION__, buf);

		return 0;
	}
}

int sync_write_to_sdcard(void)
{

	char sddev[64] = {0};
	char buf[128] = {0};


        sys_interact(sddev, sizeof(sddev), "df | grep mnt/sd | awk '{print $1}'");

	if(strlen(sddev) < 1) 
		return -1;

	snprintf(buf, sizeof(buf), "sync %s", sddev);
	system(buf);

	ra_debug_print("%s -> %s", __FUNCTION__, buf);
	return 0;
}
int json_get_mgm_sdcard_file_list(ResponseEntry *rep, struct json_object *jobj)
{
	ResponseStatus *res = rep->res;
 	int number_of_files = 0;
	char tmp[128] = {0};
	char ret_str[128] = {0};

	char file_name[128] = {0};
       	char file_size[64] = {0};
	char date[64] = {0};
	char sd_directory[32] = {0};

	struct json_object *jobj_file_info = NULL, *jarr_file = NULL;


	if(get_sdcard_mount_info(sd_directory, sizeof(sd_directory), 1) < 0)
		RET_GEN_ERRORMSG(res, API_INTERNAL_ERROR, "SD card Not found !!");

	snprintf(tmp, sizeof(tmp), "ls -l %s/*.tar.gz | grep -v ^l | wc -l", sd_directory);
        sys_interact(ret_str, sizeof(ret_str), tmp);

	//ra_debug_print("%s -> sys_interact[%s]", __FUNCTION__, tmp);
	ra_debug_print("%s -> number_of_files[%s]", __FUNCTION__, ret_str);

	number_of_files = atoi(ret_str);

	ra_debug_print("%s -> number_of_files[%d]", __FUNCTION__, number_of_files);


	jarr_file = json_object_new_array();

	for( int i = 0; i < number_of_files; i++)
	{
		jobj_file_info = json_object_new_object();

		memset(file_name, 0, sizeof(file_name));
		memset(file_size, 0, sizeof(file_size));
		memset(date, 0, sizeof(date));

       		//sys_interact(file_name, sizeof(file_name), "ls -lch /mnt/sda/*.tar.gz | awk \'NR == 1 {print $9}\'");
       		//ra_debug_print("ls -lch /mnt/sda/*.tar.gz | awk \'NR == 1 {print $9}\'");
       		//sys_interact(file_name, sizeof(file_name), "ls -lch %s/*.tar.gz | awk \'NR == %d {print $9}\'", sd_directory, (i+1));
       		//ra_debug_print("ls -lch %s/*.tar.gz | awk \'NR == %d {print $9}\'", sd_directory, i+1);

		snprintf(tmp, sizeof(tmp), "ls -lch %s/*.tar.gz | awk \'NR == %d {print $9}\' | awk -F\'/\' \'{print $4}\'\0", sd_directory, i+1);
        	sys_interact(file_name, sizeof(file_name), tmp);
		file_name[strlen(file_name) -1] = '\0';
		ra_debug_print("%s -> sys_interact[%s]", __FUNCTION__, tmp);

		snprintf(tmp, sizeof(tmp), "ls -lch %s/*.tar.gz | awk \'NR == %d {print $5}\'\0", sd_directory, i+1);
        	sys_interact(file_size, sizeof(file_size), tmp);
		ra_debug_print("%s -> sys_interact[%s]", __FUNCTION__, tmp);

		snprintf(tmp, sizeof(tmp), "ls -lch %s/*.tar.gz | awk \'NR == %d {print $6\" \"$7\" \"$8}\'\0", sd_directory, i+1);
        	sys_interact(date, sizeof(date), tmp);
		ra_debug_print("%s -> sys_interact[%s]", __FUNCTION__, tmp);


		json_object_object_add(jobj_file_info, "file_name", json_object_new_string(file_name));
		json_object_object_add(jobj_file_info, "file_size", json_object_new_string(file_size));
		json_object_object_add(jobj_file_info, "date", json_object_new_string(date));

		ra_debug_print("%s -> file_name[%s] file_size[%s] date[%s]", __FUNCTION__, 
				file_name, file_size, date);
		
//		json_object_object_add(jobj, "file_list", jobj_file_info);
		json_object_array_add(jarr_file, jobj_file_info);
	}
	json_object_object_add(jobj, "file_list", jarr_file);

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}
int json_post_mgm_backup_database_sdcard(ResponseEntry *rep, char *query_str)
{
	ResponseStatus *res = rep->res;
	char buf[128] = {0};
	char sdcard_backup_file[128] = {0};

	char sd_directory[32] = {0};
	char file_time[64] = {0};
	int i = 0;

	if(get_sdcard_mount_info(sd_directory, sizeof(sd_directory), 1) < 0)
		RET_GEN_ERRORMSG(res, API_INTERNAL_ERROR, "SD card Not found !!");

        sys_interact(file_time, sizeof(file_time), "date +%s", "%y%m%d-%H%M");

	file_time[strlen(file_time) - 1] = '\0';

	ra_debug_print("%s -> file_time[%s]\n", __FUNCTION__, file_time);


	do
	{
		i++;
		memset(sdcard_backup_file, 0, sizeof(sdcard_backup_file));
		sprintf(sdcard_backup_file, "%s/database_%s-%d.tar.gz", sd_directory, file_time, i);

		if(i > 10)
			RET_GEN_ERRORMSG(res, API_INTERNAL_ERROR, "SD card ERROR !!");

	}
	while(sysIsFileExisted(sdcard_backup_file) == TRUE);

	ra_debug_print("%s -> sdcard_backup_file[%s]\n", __FUNCTION__, sdcard_backup_file);
	sprintf(buf, "tar -zcvf %s %s/*.db", sdcard_backup_file, DB_ROOT_DIRETERY);

	ra_debug_print("%s -> %s", __FUNCTION__, buf);
	system(buf);

	if(sysIsFileExisted(sdcard_backup_file) == TRUE)
	{
		sync_write_to_sdcard();
		RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
	}
	else
	{
		RET_GEN_ERRORMSG(res, API_INTERNAL_ERROR, "Unable to write on SD card !!");
	}
}

int json_post_mgm_restore_database_sdcard(ResponseEntry *rep, char *query_str)
{
	ResponseStatus *res = rep->res;
	char buf[128] = {0};
	char cmd[128] = {0};
	char sdcard_backup_file[128] = {0};
	char *file_name = NULL;

    	struct json_object *jobj;
	char sd_directory[128] = {0};

	ra_debug_print("%s -> query_str[%s]\n", __FUNCTION__, query_str);
    	if(NULL != query_str)
    	{
        	if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        	{
            		senao_json_object_get_and_create_string(rep, jobj, "file_name", &file_name);
			ra_debug_print("%s -> file_name[%s]\n", __FUNCTION__, file_name);
		}

	}

	if(strlen(file_name) < strlen("1.tar.gz"))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "Invalid file name !!");

	if(strstr(file_name, "database_") == NULL)
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "Invalid file name !!");

	if(get_sdcard_mount_info(sd_directory, sizeof(sd_directory), 0) < 0)
		RET_GEN_ERRORMSG(res, API_INTERNAL_ERROR, "SD card Not found !!");

	sprintf(sdcard_backup_file, "%s/%s", sd_directory, file_name);

	ra_debug_print("%s -> sdcard_backup_file[%s]\n", __FUNCTION__, sdcard_backup_file);

	if(sysIsFileExisted(sdcard_backup_file) != TRUE)
	{
		RET_GEN_ERRORMSG(res, API_INTERNAL_ERROR, "Unable to read from SD card !!");
	}
#if 0
	sys_interact(buf, sizeof(buf), "rm -rf %s", DB_RESTORE_TMP_CEHCK);
	sys_interact(buf, sizeof(buf), "tar -zxvf %s -C /tmp", sdcard_backup_file);

	if(sysIsFileExisted(DB_RESTORE_TMP_CEHCK) != TRUE)
	{
		ra_debug_print("DB_RESTORE_TMP_CEHCK -> ERROR !!");
		RET_GEN_ERRORMSG(res, API_INTERNAL_ERROR, "No backup file to restore !!");
	}
#else
	//snprintf(cmd, sizeof(cmd), "tar -ztvf %s | grep %s | awk \'NR == 1 {print $6}\'", sdcard_backup_file, DB_TAR_FILE_CEHCK);
	snprintf(cmd, sizeof(cmd), "tar -tvf %s | grep %s | awk \'NR == 1 {print $6}\'", sdcard_backup_file, DB_TAR_FILE_CEHCK);

	ra_debug_print("%s -> %s", __FUNCTION__, cmd);
        sys_interact(buf, sizeof(buf), cmd);

	ra_debug_print(" sdcard restore tar file list line 1 ->[%s] \n", buf);

	if(strncmp(buf, DB_TAR_FILE_CEHCK, strlen(DB_TAR_FILE_CEHCK)) != 0)
	{
		ra_debug_print("DB_TAR_FILE_CEHCK -> ERROR !!");
		RET_GEN_ERRORMSG(res, API_INTERNAL_ERROR, "Invalid date for database !!");
	}

#endif	
        memset(cmd, 0 ,sizeof(cmd));
	//snprintf(cmd, sizeof(cmd), "tar -zxvf %s %s*.db -C /", sdcard_backup_file, DB_TAR_FILE_CEHCK);
	snprintf(cmd, sizeof(cmd), "tar -xvf %s %s*.db -C /", sdcard_backup_file, DB_TAR_FILE_CEHCK);

	ra_debug_print("%s -> %s", __FUNCTION__, cmd);

	system(cmd);

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}


int json_post_mgm_sdcard_remove_file(ResponseEntry *rep, char *query_str)
{
	ResponseStatus *res = rep->res;
	char buf[128] = {0};
	char sdcard_backup_file[128] = {0};
	char sd_directory[32] = {0};
	char *file_name = NULL;
	int i = 0, arraylen = 0;

	struct json_object *jobj = NULL, *jarr = NULL, *jarr_obj = NULL;

	if(get_sdcard_mount_info(sd_directory, sizeof(sd_directory), 0) < 0)
		RET_GEN_ERRORMSG(res, API_INTERNAL_ERROR, "SD card Not found !!");

	if(NULL != query_str)
	{

		if((jobj = jsonTokenerParseFromStack(rep, query_str)))
		{

			jarr = json_object_object_get(jobj, "file_list");
		        arraylen = json_object_array_length(jarr) ;

			ra_debug_print("%s -> arraylen %d", __FUNCTION__, arraylen);

            		for (i = 0; i < arraylen; i++) 
            		{
               			jarr_obj = json_object_array_get_idx(jarr, i);
 	         		senao_json_object_get_and_create_string(rep, jarr_obj, "file_name", &file_name);

				ra_debug_print("%s -> file_name [%s]", __FUNCTION__, file_name);

				sprintf(sdcard_backup_file, "%s/%s", sd_directory, file_name);

				if(sysIsFileExisted(sdcard_backup_file) == TRUE)
				{
					snprintf(buf, sizeof(buf), "rm %s", sdcard_backup_file);
					ra_debug_print("%s -> %s", __FUNCTION__, buf);
					system(buf);
				}
            		}
		}
	}
	else
	{
		RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
	}

	sync_write_to_sdcard();
	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);

}


int json_post_mgm_backup_config_sdcard(ResponseEntry *rep, char *query_str)
{
	ResponseStatus *res = rep->res;
	char buf[128] = {0};
	char sdcard_backup_file[128] = {0};

	char sd_directory[32] = {0};
	char file_time[64] = {0};
	int i = 0;

	if(get_sdcard_mount_info(sd_directory, sizeof(sd_directory), 1) < 0)
		RET_GEN_ERRORMSG(res, API_INTERNAL_ERROR, "SD card Not found !!");

        sys_interact(file_time, sizeof(file_time), "date +%s", "%y%m%d-%H%M");

	file_time[strlen(file_time) - 1] = '\0';

	ra_debug_print("%s -> file_time[%s]\n", __FUNCTION__, file_time);

	do
	{
		i++;
		memset(sdcard_backup_file, 0, sizeof(sdcard_backup_file));
		sprintf(sdcard_backup_file, "%s/config_%s-%d.tar.gz", sd_directory, file_time, i);

		if(i > 10)
			RET_GEN_ERRORMSG(res, API_INTERNAL_ERROR, "SD card ERROR !!");

	}
	while(sysIsFileExisted(sdcard_backup_file) == TRUE);

	/* generate config tar file to sdcard */

    	sprintf(buf, "sysupgrade --create-backup %s", sdcard_backup_file);
	system(buf);

	if(sysIsFileExisted(sdcard_backup_file) == TRUE)
	{
		sync_write_to_sdcard();
		RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
	}
	else
	{
		RET_GEN_ERRORMSG(res, API_INTERNAL_ERROR, "Unable to write on SD card !!");
	}
}

int json_post_mgm_restore_config_sdcard(ResponseEntry *rep, char *query_str)
{
	#define CONFIG_RESTORE_TMP_FILE "/tmp/restore.gz"
	ResponseStatus *res = rep->res;
	char buf[128] = {0};
	char sdcard_backup_file[128] = {0};
	char *file_name = NULL;

    	struct json_object *jobj;
	char sd_directory[32] = {0};

    	if(NULL != query_str)
    	{
        	if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        	{
            		senao_json_object_get_and_create_string(rep, jobj, "file_name", &file_name);
		}

	}

	if(strlen(file_name) < strlen("1.tar.gz"))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "Invalid file name !!");

	if(strstr(file_name, "config_") == NULL)
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "Invalid file name !!");

	if(get_sdcard_mount_info(sd_directory, sizeof(sd_directory), 0) < 0)
		RET_GEN_ERRORMSG(res, API_INTERNAL_ERROR, "SD card Not found !!");

	sprintf(sdcard_backup_file, "%s/%s", sd_directory, file_name);

	if(sysIsFileExisted(sdcard_backup_file) != TRUE)
	{
		RET_GEN_ERRORMSG(res, API_INTERNAL_ERROR, "Unable to read from SD card !!");
	}

	sprintf(buf, "cp %s %s", sdcard_backup_file, CONFIG_RESTORE_TMP_FILE);
	ra_debug_print("%s -> %s", __FUNCTION__, buf);
	system(buf);
#if SUPPORT_SYSTEM_LOG
	system("echo restore>/mnt/rebootType");
#endif
    	system("sh /etc/cfgrestore.sh;sleep 1;reboot &");
	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_post_mgm_backup_database(ResponseEntry *rep, char *query_str)
{
	ResponseStatus *res = rep->res;
	char buf[128] = {0};
	char backup_file[64] = {0};

	sprintf(backup_file, "%s/%s", DB_BACKUP_FILE_PATH_TMP, DB_BACKUP_FILE_NAME);

	//sprintf(buf, "tar -zcvf %s %s/*.db", backup_file, DB_ROOT_DIRETERY);
	sprintf(buf, "tar -cvf %s %s/*.db", backup_file, DB_ROOT_DIRETERY);

	ra_debug_print("%s -> %s", __FUNCTION__, buf);
	system(buf);

	if(sysIsFileExisted(backup_file) == TRUE)
	{
		SYSTEM("ln -s -f %s/%s %s/%s", DB_BACKUP_FILE_PATH_TMP, DB_BACKUP_FILE_NAME, DB_BACKUP_FILE_PATH, DB_BACKUP_FILE_NAME);
		RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
	}
	else
	{
		RET_GEN_ERRORMSG(res, API_INTERNAL_ERROR, "Unable to genarate backup file !!");
	}
}

int json_get_mgm_backup_database(ResponseEntry *rep, struct json_object *jobj)
{
	ResponseStatus *res = rep->res;
	char buf[128] = {0};
	char backup_file[64] = {0};

    	char lanIP[128] = {0};
	char type[10] ={0};

	sprintf(backup_file, "%s/%s", DB_BACKUP_FILE_PATH, DB_BACKUP_FILE_NAME);

	ra_debug_print("%s -> backup_file [%s]", __FUNCTION__, backup_file);

	if(sysIsFileExisted(backup_file) == FALSE)
	{
		ra_debug_print("No backup file to get !!");
		RET_GEN_ERRORMSG(res, API_INTERNAL_ERROR, "No backup file to get !!");
	}

    	api_get_string_option(NETWORK_LAN_PROTO_OPTION, type, sizeof(type));
    
	ra_debug_print("%s -> lan type [%s]", __FUNCTION__, type);

    	if(!strcmp(type,"dhcp"))
    	{
        	sys_interact(lanIP, sizeof(lanIP), "cat /tmp/dhcp_addr |awk {'printf $2'}");
    	}
    	else
    	{
        	api_get_lan_ipaddr_option(NETWORK_LAN_IPADDR_OPTION, lanIP, sizeof(lanIP));
    	}

	ra_debug_print("%s -> lanIP [%s]", __FUNCTION__, lanIP);

    	snprintf(buf, sizeof(buf), "%s/%s", lanIP, DB_BACKUP_FILE_NAME);

	ra_debug_print("%s -> buf [%s]", __FUNCTION__, buf);

	json_object_object_add(jobj, "backup_file_path", json_object_new_string(backup_file));
    	json_object_object_add(jobj, "backup_file", json_object_new_string(buf));
	
	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}


int json_post_mgm_restore_database(ResponseEntry *rep, char *query_str)
{
	ResponseStatus *res = rep->res;
	char file_check[64] = {0};
	char buf[128] = {0};
	char cmd[128] = {0};

	//snprintf(cmd, sizeof(cmd), "tar -ztvf %s | grep %s | awk \'NR == 1 {print $6}\'", "/root/tmp/restore.gz", DB_TAR_FILE_CEHCK);
	snprintf(cmd, sizeof(cmd), "tar -tvf %s | grep %s | awk \'NR == 1 {print $6}\'", "/root/tmp/restore.gz", DB_TAR_FILE_CEHCK);

	ra_debug_print("%s -> %s", __FUNCTION__, cmd);
	sys_interact(buf, sizeof(buf), cmd);

	ra_debug_print("restore tar file list line 1 ->[%s] \n", buf);

	if(strncmp(buf, DB_TAR_FILE_CEHCK, strlen(DB_TAR_FILE_CEHCK)) != 0)
	{
		ra_debug_print("DB_TAR_FILE_CEHCK -> ERROR !!");
		RET_GEN_ERRORMSG(res, API_INTERNAL_ERROR, "Invalid date for database !!");
	}

	//snprintf(cmd, sizeof(cmd), "tar -zxvf /root/tmp/restore.gz %s*.db -C /", DB_TAR_FILE_CEHCK);
	snprintf(cmd, sizeof(cmd), "tar -xvf /root/tmp/restore.gz %s*.db -C /", DB_TAR_FILE_CEHCK);
	system(cmd);

	system("rm -f /root/tmp/restore.gz");

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_post_mgm_clear_database(ResponseEntry *rep, char *query_str)
{
	ResponseStatus *res = rep->res;
	char buf[128] = {0};

	snprintf(buf, sizeof(buf), "rm -rf %s/*.db", DB_ROOT_DIRETERY);

	system(buf);

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}
#if SUPPORT_SP938BS_OPENAPI_SERVER
int json_post_mgm_rainier_pb_transfer(ResponseEntry *rep, char *query_str)
{
	bool pb_transfer_enable = 0;
	char *phbook_name[RAINIER_PHONEBOOK_NUM] = {NULL}, *phbook_number[RAINIER_PHONEBOOK_NUM] = {NULL};
	int phbook_index[RAINIER_PHONEBOOK_NUM] = {0};
	int arraylen = 0, i, idx;
	struct json_object *jobj = NULL, *jarr = NULL, *jarr_obj = NULL;
	ResponseStatus *res = rep->res;
	FILE *fp;
	
	/* SP938BS receives indicator from BSC and starts to do phonebook transfer to handset */
	if(NULL != query_str)
	{
		if((jobj = jsonTokenerParseFromStack(rep, query_str)))
		{
			//senao_json_object_get_boolean(jobj, "enable", &pb_transfer_enable);

			jarr = json_object_object_get(jobj, "phbooks");
            arraylen = json_object_array_length(jarr) ;
			
			if(arraylen > RAINIER_PHONEBOOK_NUM)
			{
				RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "Too many Phonebook index");
			}

            for (i = 0; i < arraylen; i++) 
            {
                jarr_obj = json_object_array_get_idx(jarr, i);
				senao_json_object_get_integer(jarr_obj, "index", &phbook_index[i]);
				senao_json_object_get_and_create_string(rep, jarr_obj, "name", &phbook_name[i]);
            	senao_json_object_get_and_create_string(rep, jarr_obj, "number", &phbook_number[i]);

				//debug_print("phbook index: %d.\n", phbook_index[i]);
				//debug_print("phbook name: %s.\n", phbook_name[i]);
				//debug_print("phbook number: %s.\n", phbook_number[i]);
            }
		}
		else
		{
			RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
		}
	}
	else
	{
		RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
	}

#if 0
	if(pb_transfer_enable)
	{
		/* start phonebook transfer */
	}
	else
	{
		/* stop phonebook transfer */
	}
#endif

	/* write phbook to tmp file */
	if((fp = fopen("/tmp/phbook", "w")) != NULL)
	{
		for (i = 0; i < arraylen; i++)
		{
			fprintf(fp, "%s:%s\n", phbook_name[i], phbook_number[i]);
		}
		fclose(fp);
	}
	else
	{
		RET_GEN_ERRORMSG(res, API_INTERNAL_ERROR, "Unable to write Phonebook data");
	}

	/* load tmp file to apcfg and trigger rainier phbook transfer */
	SYSTEM("system_cli -rav3 bsc_pb_transfer &");


	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_post_mgm_rainier_cfg_reload(ResponseEntry *rep, char *query_str)
{
    int reg_mode = 0;
    ResponseStatus *res = rep->res;
    /* BSC indicates SP938BS to reload the configuration again */
    /* data NULL? */
    /* insert code here */
    //system("reboot &");/* do reboot target device */
    /*
     * 1. GET data from BSC
     * 2. Reload relative daemon or module.
     *
     */

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

/* SP938BS receives indication and enters special mode for reg and dereg
 *
 */
int json_post_mgm_rainier_post_base_reg_mode(ResponseEntry *rep, char *query_str)
{
    int reg_mode = 0;/* 0/1/2 = normal/reg/dereg */
    struct json_object *jobj = NULL;
    ResponseStatus *res = rep->res;
    /* BSC indicates SP938BS to reload the configuration again */
    /* insert code here */
    /*
     * 1. GET data from BSC: normal, reg, dereg
     */
    if(NULL != query_str)
    {
        if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
            senao_json_object_get_integer(jobj, "reg_mode", &reg_mode);
        }
        else
        {
            RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
        }
    }
    else
    {
        RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
    }
    
	if((reg_mode >= 0) && (reg_mode <= 2))
	{
		SYSTEM("system_cli -rav3 bs_register_mode %d &", reg_mode);
	}
	else
	{
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "REG MODE");
	}

	
    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

/* SP938BS receives handset runtime config update from BSC
 *
 */
int json_patch_rainier_hs_cfg_update(ResponseEntry *rep, char *query_str, int hs_idx)
{
	int rainier_group = 0, subscribe_bs = 0;
	//bool enable = 0;
	char *display_name = NULL;
	char display_name_cli[64] = "No_Data";
	struct json_object *jobj = NULL;
	ResponseStatus *res = rep->res;
	int /*has_enable = 0,*/ has_display_name = 0, has_rainier_group = 0, has_subscribe_bs = 0;
	int update_flag;

	if(NULL != query_str)
	{
		if((jobj = jsonTokenerParseFromStack(rep, query_str)))
		{
			//if (json_object_object_get(jobj, "enable") != NULL)
			//{
			//	has_enable = 1;
			//	senao_json_object_get_boolean(jobj, "enable", &enable);
			//}
			if (json_object_object_get(jobj, "display_name") != NULL)
			{
				has_display_name = 1;
				senao_json_object_get_and_create_string(rep, jobj, "display_name", &display_name);
				strncpy(display_name_cli, display_name, sizeof(display_name_cli));
			}
			if (json_object_object_get(jobj, "rainier_group") != NULL)
			{
				has_rainier_group = 1;
				senao_json_object_get_integer(jobj, "rainier_group", &rainier_group);
			}
			if (json_object_object_get(jobj, "subscribe_bs") != NULL)
			{
				has_subscribe_bs = 1;
				senao_json_object_get_integer(jobj, "subscribe_bs", &subscribe_bs);
			}
		}
		else
		{
			RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
		}
	}
	else
	{
		RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
	}

	if(strlen(display_name) > 16)
	{
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "DISPLAY NAME");
	}

	if((hs_idx >= RAINIER_HSID_MIN) && (hs_idx <= RAINIER_HSID_MAX))
    {
        // if handset display_name, rainier_group, subscribe_bs modified, send data to rainierAgent
		if(has_display_name || has_rainier_group || has_subscribe_bs)
		{
			update_flag = ((has_rainier_group & 0x1) << HS_FLAG_GROUP_BIT) | ((has_subscribe_bs & 0x1) << HS_FLAG_BASE_BIT) | ((has_display_name & 0x1) << HS_FLAG_NAME_BIT);
			SYSTEM("system_cli -rav3 hs_cfg_update %d %d %d %d '%s' &", 
					hs_idx, update_flag, rainier_group, subscribe_bs, display_name_cli);
		}
    }
    else
    {
        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "HS ID");
    }

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}
#endif  /* #if SUPPORT_SP938BS_OPENAPI_SERVER */

#if 0   // the base_list function is implemented by base_station_list, this api can be removed or need to modify it to be an api to get base_info
int json_get_rainier_base_list(ResponseEntry *rep, struct json_object *jobj)
{
    int i;
    char idx_buf[6];
    struct json_object *jobj_info = NULL;
	ResponseStatus *res = rep->res;
    /* Get all bases information */
    /* TODO */
    for(i = RAINIER_BSID_MIN; i <= RAINIER_BSID_MAX; i++)
    {
        sprintf(idx_buf, "%d", i);/* index RAINIER_BSID_MIN ~  RAINIER_BSID_MAX */
        jobj_info = newObjectFromStack(rep);
        json_get_rainier_base_list_idx(rep, jobj_info, i);
        json_object_object_add(jobj, idx_buf, jobj_info);
    }

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

/*
        active:
          type: integer        
          enum:
            - 0
            - 1
          description: |
            Specific value:
              * 0 - disable
              * 1 - enable
        status:
          type: integer
          enum:
            - 0
            - 1
          description: |
            Specific value:
              * 0 - off line 
              * 1 - on line       
        sync_status:
          type: integer
          enum:
            - 0
            - 1
          description: |
            Specific value:
              * 0 - un-sync
              * 1 - sync                
        mac_address:
          type: string      
        ip_address:
          type: string          
        description:
          type: string
*/
int json_get_rainier_base_list_idx(ResponseEntry *rep, struct json_object *jobj, int bs_idx)
{
    int active = 0, status = 0, sync_status = 0;
    char mac_address[18] = {0} , ip_address[16] = {0}, description[32] = {0};
    ResponseStatus *res = rep->res;
#if 0
    /* Get base information */
    /* TODO - insert your code here */
#endif
    json_object_object_add(jobj, "active", json_object_new_int(active));
    json_object_object_add(jobj, "status", json_object_new_int(status));
    json_object_object_add(jobj, "sync_status", json_object_new_int(sync_status));
    json_object_object_add(jobj, "mac_address", json_object_new_string(mac_address));
    json_object_object_add(jobj, "ip_address", json_object_new_string(ip_address));
    json_object_object_add(jobj, "description", json_object_new_string(description));
    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}
#endif

int json_post_mgm_rainier_log_search_range(ResponseEntry *rep, char *query_str)
{
    char *start_date = NULL, *end_date = NULL, *search_string = NULL;
    int select_device = 0;
	char log_start_date[64] = {0};
	char log_end_date[64] = {0};
	char log_search_string[64] = {0};
	int log_select_device = 0;
	struct json_object *jobj = NULL;
    ResponseStatus *res = rep->res;

    if(NULL != query_str)
    {
        if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
            /* get data from api */
			senao_json_object_get_and_create_string(rep, jobj, "start_date", &start_date);
			senao_json_object_get_and_create_string(rep, jobj, "end_date", &end_date);
            senao_json_object_get_integer(jobj, "select_device", &select_device);
			senao_json_object_get_and_create_string(rep, jobj, "search_string", &search_string);

			memset(log_start_date, 0, sizeof(log_start_date));
			memset(log_end_date, 0, sizeof(log_end_date));
			memset(log_search_string, 0, sizeof(log_search_string));

			strncpy(log_start_date, start_date, sizeof(log_start_date));
			strncpy(log_end_date, end_date, sizeof(log_end_date));
			strncpy(log_search_string, search_string, sizeof(log_search_string));
			log_select_device = select_device;

			debug_print("log_start_date [%s] \n", log_start_date);
			debug_print("log_end_date [%s] \n", log_end_date);
			debug_print("log_select_device [%d] \n", log_select_device);
			debug_print("log_search_string [%s] \n", log_search_string);

			/* save log search range to uci config */
			SYSTEM("uci set senao-bsc-api.log=log");
			SYSTEM("uci set senao-bsc-api.log.start_date='%s'", log_start_date);
			SYSTEM("uci set senao-bsc-api.log.end_date='%s'", log_end_date);
			SYSTEM("uci set senao-bsc-api.log.select_device=%d", log_select_device);
			SYSTEM("uci set senao-bsc-api.log.search_string='%s'", log_search_string);
			SYSTEM("uci commit senao-bsc-api");
        }
        else
        {
            RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
        }
    }
    else
    {
        RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
    }
    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_post_mgm_rainier_log(ResponseEntry *rep, char *query_str)
{
	char /**time = NULL,*/ *log_type = NULL, *device = NULL, *event_type = NULL, *description = NULL;
	int ret;
	
    struct json_object *jobj = NULL;
    ResponseStatus *res = rep->res;

    if(NULL != query_str)
    {
        if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
            /* Base POST data to BSC */
            //senao_json_object_get_and_create_string(rep, jobj, "time", &time);
            senao_json_object_get_and_create_string(rep, jobj, "log_type", &log_type);
            senao_json_object_get_and_create_string(rep, jobj, "device", &device);
            senao_json_object_get_and_create_string(rep, jobj, "event_type", &event_type);
            senao_json_object_get_and_create_string(rep, jobj, "description", &description);
        }
        else
        {
            RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
        }
    }
    else
    {
        RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
    }

    /* check parameters if valid or not */

    /* Save recieved data into database or tmp */
	ret = log_data_base_write(log_type, device, event_type, description);

	if(ret < 0)
	{
		RET_GEN_ERRORMSG(res, API_INTERNAL_ERROR, "Fail to write data to log");
	}

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}


int json_get_mgm_rainier_log(ResponseEntry *rep, struct json_object *jobj, int page_idx)
{
    int i;
	int log_idx_start;
	int total_log_page = 1;
	//int current_log_page = 1;
	char log_start_date[64] = {0};
	char log_end_date[64] = {0};
	char log_search_string[64] = {0};
	int log_select_device = 0;
	ResponseStatus *res = rep->res;

	struct json_object *jarr_log = NULL, *jobj_log = NULL;
	jarr_log = json_object_new_array();

	if(page_idx < 1)
	{
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "Page index less than 1");
	}

	/* get log search range from uci config */
	api_get_string_option("senao-bsc-api.log.start_date", log_start_date, sizeof(log_start_date));
	api_get_string_option("senao-bsc-api.log.end_date", log_end_date, sizeof(log_end_date));
	api_get_integer_option("senao-bsc-api.log.select_device", &log_select_device);
	api_get_string_option("senao-bsc-api.log.search_string", log_search_string, sizeof(log_search_string));
	
#if 1
	log_data_base_read(log_select_device, log_start_date, log_end_date, log_search_string, jarr_log, page_idx);

	if(log_read_out_row == 0)
	{
		total_log_page = 1;
	}
	else
	{
		total_log_page = (log_read_out_row / RAINIER_LOG_PAGE_SIZE);

		if(log_read_out_row % RAINIER_LOG_PAGE_SIZE)
			total_log_page++;
	}

	if(page_idx > total_log_page)
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "Page index greater than total page");
#else
	log_idx_start = ((page_idx - 1) * RAINIER_LOG_PAGE_SIZE) + 1;

    for(i = log_idx_start; i < (log_idx_start+RAINIER_LOG_PAGE_SIZE); i++)
    {
        jobj_log = json_object_new_object();
        json_get_mgm_rainier_log_idx(rep, jobj_log, i);
		json_object_array_add(jarr_log, jobj_log);
    }
#endif 

	json_object_object_add(jobj, "total_log_page", json_object_new_int(total_log_page));
	json_object_object_add(jobj, "current_log_page", json_object_new_int(page_idx));
	json_object_object_add(jobj, "log", jarr_log);

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}
#if 0	// useless
int json_get_mgm_rainier_log_of_base(ResponseEntry *rep, struct json_object *jobj, int base_idx, int page_idx)
{
    int i;
	int log_idx_start;
	int total_log_page = 1;
	int current_log_page = 1;
	ResponseStatus *res = rep->res;

	struct json_object *jarr_log = NULL, *jobj_log = NULL;
	jarr_log = json_object_new_array();

	/* check base_idx valid or not */
    if( (base_idx < RAINIER_BSID_MIN) || (base_idx > RAINIER_BSID_MAX))
    {
        /* base_idx is not in range */
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "BASE ID");
    }

	log_idx_start = ((page_idx - 1) * RAINIER_LOG_PAGE_SIZE) + 1;

    for(i = log_idx_start; i < (log_idx_start+RAINIER_LOG_PAGE_SIZE); i++)
    {
        jobj_log = json_object_new_object();
        json_get_mgm_rainier_log_of_base_idx(rep, jobj_log, base_idx, i);
		json_object_array_add(jarr_log, jobj_log);
    } 

	json_object_object_add(jobj, "total_log_page", json_object_new_int(total_log_page));
	json_object_object_add(jobj, "current_log_page", json_object_new_int(current_log_page));
	json_object_object_add(jobj, "log", jarr_log);

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_get_mgm_rainier_log_idx(ResponseEntry *rep, struct json_object *jobj, int log_idx)
{
    char log_type[32] = {0}, time[32] = {0} , device[32] = {0}, event_type[32] = {0}, description[64] = {0};
    ResponseStatus *res = rep->res;
#if 0
    /* Get log from database by index */
    /* TODO - insert your code here */
#endif
    json_object_object_add(jobj, "time", json_object_new_string(time));
    json_object_object_add(jobj, "log_type", json_object_new_string(log_type));
    json_object_object_add(jobj, "device", json_object_new_string(device));
    json_object_object_add(jobj, "event_type", json_object_new_string(event_type));
    json_object_object_add(jobj, "description", json_object_new_string(description));
    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_get_mgm_rainier_log_of_base_idx(ResponseEntry *rep, struct json_object *jobj, int base_idx, int log_idx)
{
    char log_type[32] = {0}, time[32] = {0} , device[32] = {0}, event_type[32] = {0}, description[64] = {0};
    ResponseStatus *res = rep->res;
#if 0
    /* Get log belong to base_idx from database by index */
    /* TODO - insert your code here */
#endif
    json_object_object_add(jobj, "time", json_object_new_string(time));
    json_object_object_add(jobj, "log_type", json_object_new_string(log_type));
    json_object_object_add(jobj, "device", json_object_new_string(device));
    json_object_object_add(jobj, "event_type", json_object_new_string(event_type));
    json_object_object_add(jobj, "description", json_object_new_string(description));
    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}
#endif	// useless

int json_post_mgm_rainier_reboot_base(ResponseEntry *rep, char *query_str, int base_idx)
{
    //struct json_object *jobj = NULL;
	int idx = -1;
	char cmd[1024] = {0};
	char buf[256] = {0};
    char cmd_debug[1024] = {0};
	char username[32] = {0};
	char password[32] = {0};

	char device_token[256] = {0};

	char bs_ip[32] = {0};


    ResponseStatus *res = rep->res;
    if((base_idx >= RAINIER_BSID_MIN) && (base_idx <= RAINIER_BSID_MAX))
    {
        /* check the base_idx is registered to BSC or not */
		idx = findBsConfigIdxByIndex(base_idx);
		
		if(idx == -1)
			RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "BASE ID");
    }
    else
    {
        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "BASE ID");
    }

    /* TODO - insert your code here */
	char *bsc_id = (char*)malloc(13*sizeof(char));
	getBscId(bsc_id, 13);

	snprintf(username, sizeof(username), "%s", bsc_id);
	//snprintf(cmd, sizeof(cmd), "uci get base-station-list.@base-station[%d].bs_key | tr -d \"\\n\"", idx);
	//sys_interact(password, sizeof(password), cmd);	
	api_get_string_option2(password, sizeof(password), "base-station-list.@base-station[%d].bs_key", idx);
  

	//snprintf(cmd, sizeof(cmd), "uci get base-station-list.@base-station[%d].ip | tr -d \"\\n\"", idx);
	//sys_interact(bs_ip, sizeof(bs_ip), cmd);	
	api_get_string_option2(bs_ip, sizeof(bs_ip), "base-station-list.@base-station[%d].ip", idx);

    snprintf(cmd, sizeof(cmd), "curl --connect-timeout 3 --max-time 20 -v -k -X POST \"https://%s:4430/api/sys/login\" -H \"accept: */*\" -H \"Content-Type: application/json\" -d \"{\\\"username\\\":\\\"%s\\\",\\\"password\\\":\\\"%s\\\"}\" | grep token | awk '{for (i=1;i<=NF;i++){if ($i ~/token/) {print $(i+1)}}}' | sed -e 's/\"//g' | tr -d \"\\n\"", bs_ip, username, password);
	//debug_print("cmd: [--%s--] \n", cmd);
  
	sprintf(cmd_debug, "echo \"%s\" > /tmp/cmd_debug", cmd);
	//debug_print("cmd_debug: [??%s??] \n", cmd_debug);
	system(cmd_debug);

	sys_interact_long(device_token, sizeof(device_token), cmd);
	debug_print("device_token: %s. \n", device_token);

	if(strcmp(device_token, "---") == 0)
		goto EXIT;

	snprintf(cmd, sizeof(cmd), "curl --connect-timeout 3 --max-time 20 -v -k -X POST \"https://%s:4430/api/mgm/reboot\" -H \"accept: */*\" -H \"Authorization: Bearer %s\" -d \"{}\" | awk '{for (i=1;i<=NF;i++){if ($i ~/status_code/) {print $(i+1)}}}' | sed -e 's/\"//g' | tr -d \"\\n\"", bs_ip, device_token );
	sys_interact_long(buf, sizeof(buf), cmd);
	//debug_print("cmd: [22%s22] \n", cmd);
	debug_print("-- buf: %s. \n", buf);

	if(strcmp(buf, "200,") == 0) // OK
	{
		free(bsc_id);
   		RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
	}
	/* do the procedure to reboot base */
    /* END - insert your code here */
EXIT:	
	free(bsc_id);
	RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "Unable to reboot Base");
}

int json_post_rainier_fxo_idx(ResponseEntry *rep, char *query_str, int fxo_idx)
{
    char *base_name = NULL, *description = NULL;
	char *phone_number = NULL;
	int line_in_dedicate = 0, line_out_dedicate = 0;
	char inbound_route[8] = {0}, outbound_route[8] = {0};
	int idx;
	char cmd[1024] = {0};
	char buf[256] = {0};
    struct json_object *jobj = NULL;
    ResponseStatus *res = rep->res;
    
	/* check fxo_idx valid or not */
    if(!((fxo_idx <= RAINIER_BSID_MAX) && (fxo_idx >= RAINIER_BSID_MIN)))
    {
        /* fxo_idx is not in range */
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "BASE ID");
    }
	
	idx = findBsConfigIdxByIndex(fxo_idx);
	if(idx == -1) // bs is not in the list
	{
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "BASE ID");
	}

    if(NULL != query_str)
    {
        if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
            /* Base POST data to BSC */
            //senao_json_object_get_and_create_string(rep, jobj, "base_name", &base_name);
			//senao_json_object_get_and_create_string(rep, jobj, "description", &description);
			senao_json_object_get_and_create_string(rep, jobj, "phone_number", &phone_number);
			senao_json_object_get_integer(jobj, "line_in_dedicate", &line_in_dedicate);
			senao_json_object_get_integer(jobj, "line_out_dedicate", &line_out_dedicate);
        }
        else
        {
            RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
        }
    }
    else
    {
        RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
    }

    /* check string valid or not */
	if((line_in_dedicate >= RAINIER_HSID_MIN) && (line_in_dedicate <= RAINIER_HSID_MAX))
    {
        // inbound_route: "HS10" - "HS99"
		sprintf(inbound_route, "HS%d", line_in_dedicate);
    }
	else if((line_in_dedicate >= RAINIER_GROUP_MIN) && (line_in_dedicate <= RAINIER_GROUP_MAX))
    {
        // inbound_route: "GRP0" - "GRP7"
		sprintf(inbound_route, "GRP%d", line_in_dedicate);
    }
	else
	{
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "IN DEDICATE");
	}

	if((line_out_dedicate >= RAINIER_HSID_MIN) && (line_out_dedicate <= RAINIER_HSID_MAX))
    {
        // outbound_route: "HS10" - "HS99"
		sprintf(outbound_route, "HS%d", line_out_dedicate);
    }
	else if((line_out_dedicate >= RAINIER_GROUP_MIN) && (line_out_dedicate <= RAINIER_GROUP_MAX))
    {
        // outbound_route: "GRP0" - "GRP7"
		sprintf(outbound_route, "GRP%d", line_out_dedicate);
    }
	else
	{
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "OUT DEDICATE");
	}

	//snprintf(cmd, sizeof(cmd), "uci set base-station-list.@base-station[%d].phone_number='%s'", idx, phone_number);
	//	sys_interact(buf, sizeof(buf), cmd);
	api_set_string_option2(phone_number, sizeof(phone_number), "base-station-list.@base-station[%d].phone_number", idx);
	
#if 1
	if (api_set_string_option2(inbound_route, sizeof(inbound_route), "sip_fxo.sip_fxo_%d.in_dedicate", idx) != API_RC_SUCCESS)
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "IN DEDICATE");
	
	if (api_set_string_option2(outbound_route, sizeof(outbound_route), "sip_fxo.sip_fxo_%d.out_dedicate", idx) != API_RC_SUCCESS)
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "OUT DEDICATE");

	api_commit_option("sip_fxo");
#else
	if (api_set_string_option2(inbound_route, sizeof(inbound_route), "rainier.route_fxo_trunk_%d.inbound", idx) != API_RC_SUCCESS)
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "IN DEDICATE");
	
	if (api_set_string_option2(outbound_route, sizeof(outbound_route), "rainier.route_fxo_trunk_%d.outbound", idx) != API_RC_SUCCESS)
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "OUT DEDICATE");

	api_commit_option("rainier");
#endif

	//snprintf(cmd, sizeof(cmd), "uci commit base-station-list");
	//sys_interact(buf, sizeof(buf), cmd);
	api_commit_option("base-station-list");

	// reload asterisk
	SYSTEM("astgen");
	SYSTEM("asterisk -rx 'core reload'");

	// NMS send new FXO setting to BS
	SYSTEM("nmsconf_cli conf_mgm sendFxoSetting");

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_get_rainier_fxo(ResponseEntry *rep, struct json_object *jobj)
{
    ResponseStatus *res = rep->res;
    int i;

	struct json_object *jarr_fxo = NULL, *jobj_fxo = NULL;
	jarr_fxo = json_object_new_array();

    /* Get fxo information from BSC to GUI */
    for(i = RAINIER_BSID_MIN; i <= RAINIER_BSID_MAX; i++)
    {
		if(findBsConfigIdxByIndex(i) != -1) // bs is in the list
		{
			jobj_fxo = json_object_new_object();
        	json_get_rainier_fxo_idx(rep, jobj_fxo, i);
			json_object_array_add(jarr_fxo, jobj_fxo);
		}
	}

	json_object_object_add(jobj, "fxo", jarr_fxo);

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_get_rainier_fxo_idx(ResponseEntry *rep, struct json_object *jobj, int fxo_idx)
{
    char phone_number[16] = {0}, base_name[32] = {0}, description[64] = {0};
	int line_in_dedicate = 0, line_out_dedicate = 0;
	char inbound_route[8] = {0}, outbound_route[8] = {0};
	int idx;
	char cmd[1024] = {0};
    ResponseStatus *res = rep->res;
    
	/* check fxo_idx valid or not */
    if(!((fxo_idx <= RAINIER_BSID_MAX) && (fxo_idx >= RAINIER_BSID_MIN)))
    {
        /* fxo_idx is not in range */
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "BASE ID");
    }
	
	idx = findBsConfigIdxByIndex(fxo_idx);
	if(idx == -1) // bs is not in the list
	{
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "BASE ID");
	}

	//snprintf(cmd, sizeof(cmd), "uci get base-station-list.@base-station[%d].name | tr -d \"\\n\"", idx);
	//sys_interact(base_name, sizeof(base_name), cmd);
	api_get_string_option2(base_name, sizeof(base_name), "base-station-list.@base-station[%d].name", idx);
	//snprintf(cmd, sizeof(cmd), "uci get base-station-list.@base-station[%d].description | tr -d \"\\n\"", idx);
	//sys_interact(description, sizeof(description), cmd);
	api_get_string_option2(description, sizeof(description), "base-station-list.@base-station[%d].description", idx);
	//snprintf(cmd, sizeof(cmd), "uci get base-station-list.@base-station[%d].phone_number | tr -d \"\\n\"", idx);
	//sys_interact(phone_number, sizeof(phone_number), cmd);
	api_get_string_option2(phone_number, sizeof(phone_number), "base-station-list.@base-station[%d].phone_number", idx);
#if 1
	api_get_string_option2(inbound_route, sizeof(inbound_route), "sip_fxo.sip_fxo_%d.in_dedicate", idx);
#else
	api_get_string_option2(inbound_route, sizeof(inbound_route), "rainier.route_fxo_trunk_%d.inbound", idx);
#endif
    if(strncmp(inbound_route, "HS", 2) == 0)
    {
        // line_in_dedicate: HS, 10 - 99
        line_in_dedicate = atoi(&inbound_route[2]);
    }
    else if(strncmp(inbound_route, "GRP", 3) == 0)
    {
        // line_in_dedicate: GRP, 0 - 7
        line_in_dedicate = atoi(&inbound_route[3]);
    }
#if 1
	api_get_string_option2(outbound_route, sizeof(outbound_route), "sip_fxo.sip_fxo_%d.out_dedicate", idx);
#else
    api_get_string_option2(outbound_route, sizeof(outbound_route), "rainier.route_fxo_trunk_%d.outbound", idx);
#endif
    if(strncmp(outbound_route, "HS", 2) == 0)
    {
        // line_out_dedicate: HS, 10 - 99
        line_out_dedicate = atoi(&outbound_route[2]);
    }
    else if(strncmp(outbound_route, "GRP", 3) == 0)
    {
        // line_out_dedicate: GRP, 0 - 7
        line_out_dedicate = atoi(&outbound_route[3]);
    }
	//debug_print("curr idx: %d, base_name: %s.\n", idx, base_name);
	//debug_print("curr idx: %d, description: %s.\n", idx, description);
	//debug_print("curr idx: %d, phone_number: %s.\n", idx, phone_number);
	//debug_print("curr idx: %d, phone_number: %s.\n", idx, inbound_route);
	//debug_print("curr idx: %d, phone_number: %s.\n", idx, outbound_route);


    json_object_object_add(jobj, "bs_id", json_object_new_int(fxo_idx));
	json_object_object_add(jobj, "base_name", json_object_new_string(base_name));
	json_object_object_add(jobj, "description", json_object_new_string(description));
	json_object_object_add(jobj, "phone_number", json_object_new_string(phone_number));
    json_object_object_add(jobj, "line_in_dedicate", json_object_new_int(line_in_dedicate));
	json_object_object_add(jobj, "line_out_dedicate", json_object_new_int(line_out_dedicate));

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_post_rainier_statistic_search_range(ResponseEntry *rep, char *query_str)
{
    char *start_date = NULL, *end_date = NULL, *search_string = NULL;
    int select_base = 0, select_handset = 0;
	struct json_object *jobj = NULL;
    ResponseStatus *res = rep->res;

    if(NULL != query_str)
    {
        if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
            /* get data from api */
			senao_json_object_get_and_create_string(rep, jobj, "start_date", &start_date);
			senao_json_object_get_and_create_string(rep, jobj, "end_date", &end_date);
			senao_json_object_get_and_create_string(rep, jobj, "search_string", &search_string);
		        senao_json_object_get_integer(jobj, "select_base", &select_base);
			senao_json_object_get_integer(jobj, "select_handset", &select_handset);

            /* TODO - insert your code here */
			SYSTEM("uci set senao-bsc-api.cdr=cdr");
			SYSTEM("uci set senao-bsc-api.cdr.start_date='%s'", start_date);
			SYSTEM("uci set senao-bsc-api.cdr.end_date='%s'", end_date);
			SYSTEM("uci set senao-bsc-api.cdr.search_string=%s", search_string);
			SYSTEM("uci set senao-bsc-api.cdr.select_handset='%d'", select_handset);
			SYSTEM("uci set senao-bsc-api.cdr.select_base='%d'", select_base);
			SYSTEM("uci commit senao-bsc-api");

        }
        else
        {
            RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
        }
    }
    else
    {
        RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
    }


#if 0
	/* read rssi */
	if(rssi_val != NULL)
	{
		free(rssi_val);
		rssi_val = NULL;
	}
	
	rssi_num = get_column_count_in_time(DB_BSINFO_PATH, DB_BSINFO_TABLE, start_date, end_date, "call_rssi");
	
	if(rssi_num > 0)
	{
		rssi_val = (db_rssi_t *)malloc(sizeof(db_rssi_t) * rssi_num);

		if(rssi_val != NULL)
			get_rssi_vale_list(DB_BSINFO_PATH, DB_BSINFO_TABLE, cdr_start_date, cdr_end_date, 
					"call_rssi", rssi_val, rssi_num, cdr_select_base, cdr_select_handset);
	}
#endif
    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_get_rainier_all_base_statistic(ResponseEntry *rep, struct json_object *jobj)
{
    struct json_object *jobj_statistic = NULL;
    char idx_buf[4];
    ResponseStatus *res = rep->res;
    int i, result;

	struct json_object *jarr_statistic = NULL;
	jarr_statistic = json_object_new_array();

    /* Get statistic data of each BS */
	for(i = RAINIER_BSID_MIN; i <= RAINIER_BSID_MAX; i++)
    {
        jobj_statistic = json_object_new_object();
        result = json_get_rainier_base_statistic(rep, jobj_statistic, i);
		if(result == API_SUCCESS)
		{
			json_object_array_add(jarr_statistic, jobj_statistic);
		}
    }

	json_object_object_add(jobj, "base_statistic", jarr_statistic);

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_post_rainier_base_statistic(ResponseEntry *rep, char *query_str, int index)
{
    char *base_info = NULL, *time = NULL, *air_time = NULL, *busy_time = NULL;
    int busy_count = 0, total_calls = 0, handover_count = 0, ieee1588_failed_count = 0, sip_failed_count = 0;
    struct json_object *jobj = NULL;
    ResponseStatus *res = rep->res;

    if(NULL != query_str)
    {
        if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
            /* Base POST data to BSC */
            senao_json_object_get_and_create_string(rep, jobj, "base_info", &base_info);
            senao_json_object_get_and_create_string(rep, jobj, "time", &time);
            senao_json_object_get_and_create_string(rep, jobj, "air_time", &air_time);
            senao_json_object_get_integer(jobj, "busy_count", &busy_count);
            senao_json_object_get_and_create_string(rep, jobj, "busy_time", &busy_time);
            senao_json_object_get_integer(jobj, "total_calls", &total_calls);
            senao_json_object_get_integer(jobj, "handover_count", &handover_count);
            senao_json_object_get_integer(jobj, "ieee1588_failed_count", &ieee1588_failed_count);
            senao_json_object_get_integer(jobj, "sip_failed_count", &sip_failed_count);
        }
        else
        {
            RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
        }
    }
    else
    {
        RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
    }
    /* TODO - insert your code here */
    /* check parameters if valid or not */

    /* Save recieved data into database or tmp */
    
    /* END - insert your code here */
    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_get_rainier_base_statistic(ResponseEntry *rep, struct json_object *jobj, int base_idx)
{
    char base_info[32] = {0};
    char time[32] = {0}, air_time[12] = {0}, busy_time[12] = {0}; /* format "DD-HH:MM:SS"*/
    int busy_count = 0, total_calls = 0, handover_count = 0, ieee1588_failed_count = 0, sip_failed_count = 0;
    ResponseStatus *res = rep->res;
#if 0
    char start_tm_str[32] = "2010-07-01 07:50:20";
    char end_tm_str[32] = "2020-08-02 07:58:46";
#else
    char *start_tm_str = cdr_start_date;
    char *end_tm_str = cdr_end_date;

#endif    
    int bsid = base_idx;
    int hsid = 0;
    char bs_busy[32] = {0};
    int sip_call_count = 0;
    int sip_call_fail_count = 0;
    int ptp_fail_count = 0;
	
    int idx = 0;

	debug_print("Jason DEBUG %s[%d] base_idx  -> %d  \n",__FUNCTION__, __LINE__, base_idx);

    if((base_idx >= RAINIER_BSID_MIN) && (base_idx <= RAINIER_BSID_MAX))
    {
        /* check the base_idx is registered to BSC or not */
		idx = findBsConfigIdxByIndex(base_idx);
		debug_print("Jason DEBUG %s[%d] idx  -> %d  \n",__FUNCTION__, __LINE__, idx);
		if(idx == -1)
			RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "BASE ID");
    }
    else
    {
	debug_print("Jason DEBUG %s[%d] API_INVALID_ARGUMENTS  \n",__FUNCTION__, __LINE__);
        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "BASE ID");
    }

#if 1
   	sprintf(base_info, "%d", base_idx);
#else
   	sprintf(base_info, "BS%1d", base_idx);
#endif	
#if 1
    	if(get_search_time(start_tm_str, end_tm_str, time))
	{
		memset(time, 0 ,sizeof(time));
		sprintf(time, "---");
	}		
#else	
    	if(get_bs_time(rep, jobj, bs_time, sizeof(bs_time), idx) != 0)
	{
		memset(bs_time, 0 ,sizeof(bs_time));
		sprintf(bs_time, "---");
	}		
#endif
	if(get_air_time(start_tm_str, end_tm_str, bsid, hsid, air_time, sizeof(air_time)) != 0)
	{
		memset(air_time, 0 ,sizeof(air_time));
		sprintf(air_time, "---");
	}
	
	if(get_busy_time(start_tm_str, end_tm_str, bsid, hsid, bs_busy, sizeof(bs_busy)) != 0)
	{
		memset(bs_busy, 0 ,sizeof(bs_busy));
		sprintf(bs_busy, "---");
	}

	if(get_busy_count(start_tm_str, end_tm_str, bsid, hsid, &busy_count, 1) != 0)
		busy_count = 0;


	if(get_sip_call_count(start_tm_str, end_tm_str, bsid, hsid, &sip_call_count, -1) != 0)
		sip_call_count = 0;


	if(get_handover_count(start_tm_str, end_tm_str, bsid, hsid, &handover_count, -1) != 0)
		handover_count = 0;

	if(get_ptp_fail_count(start_tm_str, end_tm_str, bsid, hsid, &ptp_fail_count, 0) != 0)
		ptp_fail_count = 0;

	if(get_sip_reg_fail_count(start_tm_str, end_tm_str, bsid, hsid, &sip_call_fail_count, 1) != 0)
		sip_call_fail_count = 0;

	debug_print("Jason DEBUG %s[%d] base_info  -> %s  \n",__FUNCTION__, __LINE__, base_info);

	debug_print("Jason DEBUG %s[%d] bs time  -> %s  \n",__FUNCTION__, __LINE__, time);
#if 1
	debug_print("Jason DEBUG %s[%d] total air_tim sec  -> %s sec \n",__FUNCTION__, __LINE__, air_time);
	debug_print("Jason DEBUG %s[%d] total bs_busy sec  -> %s sec \n",__FUNCTION__, __LINE__, bs_busy);
	debug_print("Jason DEBUG %s[%d] total busy count  -> %d  \n",__FUNCTION__, __LINE__, busy_count);
	debug_print("Jason DEBUG %s[%d] total sip_call_count  -> %d  \n",__FUNCTION__, __LINE__, sip_call_count);
	debug_print("Jason DEBUG %s[%d] total handover count  -> %d  \n",__FUNCTION__, __LINE__, handover_count);
	debug_print("Jason DEBUG %s[%d] ptp_fail_count  -> %d  \n",__FUNCTION__, __LINE__, ptp_fail_count);
	debug_print("Jason DEBUG %s[%d] sip_call_fail_count  -> %d  \n",__FUNCTION__, __LINE__, sip_call_fail_count);
#endif
    /* Get dat from SQL database */
    /* TODO - insert your code here */
    /* END - insert your code here */

#if 1
    /* put data in json format */
    /* rainier base should be calculate the result */
    json_object_object_add(jobj, "base_info", json_object_new_string(base_info));
    json_object_object_add(jobj, "time", json_object_new_string(time));
    json_object_object_add(jobj, "air_time", json_object_new_string(air_time));
    json_object_object_add(jobj, "busy_count", json_object_new_int(busy_count));
    json_object_object_add(jobj, "busy_time", json_object_new_string(bs_busy));
    json_object_object_add(jobj, "total_calls", json_object_new_int(sip_call_count));
    json_object_object_add(jobj, "handover_count", json_object_new_int(handover_count));
    json_object_object_add(jobj, "ieee1588_failed_count", json_object_new_int(ptp_fail_count));
    json_object_object_add(jobj, "sip_failed_count", json_object_new_int(sip_call_fail_count));
#endif
	debug_print("Jason DEBUG %s[%d] API_SUCCESS  \n",__FUNCTION__, __LINE__);

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_get_rainier_traffic_statistic(ResponseEntry *rep, struct json_object *jobj)
{
    char current_time[20] = {0}, start_time[20] = {0};/* format "YYYY-MM-DD HH:MM:SS" */
    int incoming_call = 0, outgoing_call = 0, intercom_call = 0, handover_count = 0, overall = 0, broadcast_call = 0;
    ResponseStatus *res = rep->res;


    char *start_tm_str = cdr_start_date;
    char *end_tm_str = cdr_end_date;

	get_sip_call_dir_count(start_tm_str, end_tm_str, 0, 0, &incoming_call, "in");
	get_sip_call_dir_count(start_tm_str, end_tm_str, 0, 0, &outgoing_call, "out");
	get_sip_call_dir_count(start_tm_str, end_tm_str, 0, 0, &intercom_call, "intercom");
	get_sip_call_dir_count(start_tm_str, end_tm_str, 0, 0, &broadcast_call, "broadcast");

	get_handover_count(start_tm_str, end_tm_str, 0, 0, &handover_count, -1);

	overall = incoming_call + outgoing_call + intercom_call + handover_count;
#if 1
    /* Get dat from SQL database */
    /* TODO - insert your code here */
    /* END - insert your code here */

    /* put data in json format */
    /* rainier base should be calculate the result */
    json_object_object_add(jobj, "end_time", json_object_new_string(end_tm_str));
    json_object_object_add(jobj, "start_time", json_object_new_string(start_tm_str));
    json_object_object_add(jobj, "incoming_call", json_object_new_int(incoming_call));
    json_object_object_add(jobj, "outgoing_call", json_object_new_int(outgoing_call));
    json_object_object_add(jobj, "intercom_call", json_object_new_int(intercom_call));
    json_object_object_add(jobj, "broadcast_call", json_object_new_int(broadcast_call));
    json_object_object_add(jobj, "handover_count", json_object_new_int(handover_count));
    json_object_object_add(jobj, "overall", json_object_new_int(overall));
#endif
    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

/* BSC return CDR(call statistic) data to GUI.
*/
int json_get_rainier_cdr_info(ResponseEntry *rep, struct json_object *jobj, int page_idx)
{
    char start_time[20] = {0};/* format "YYYY-MM-DD HH:MM:SS" */
    char dir[32] = {0}, trunk[32] = {0}, cid[32] = {0}, from[32] = {0}, to[32] = {0}, disposition[32] = {0};
    char duration[9] = {0};/* format: "HH:MM:SS"*/
    int base = 0, rssi = 0;
	int idx;
	int total_cdr = 100;
	int cdr_total_page = 0;

    ResponseStatus *res = rep->res;

	struct json_object *jarr_cdrinfo = NULL, *jobj_cdrinfo = NULL;
	jarr_cdrinfo = json_object_new_array();

	if(page_idx < 1)
	{
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "Page index less than 1");
	}

	debug_print(" -- json_get_rainier_cdr_info  page %d  -- \n", page_idx);

	/* read cdr search condition from uci config */

	memset(cdr_start_date, 0, sizeof(cdr_start_date));
	memset(cdr_end_date, 0, sizeof(cdr_end_date));
	memset(cdr_search_string, 0, sizeof(cdr_search_string));

	api_get_string_option("senao-bsc-api.cdr.start_date", cdr_start_date, sizeof(cdr_start_date));
	api_get_string_option("senao-bsc-api.cdr.end_date", cdr_end_date, sizeof(cdr_end_date));
	api_get_string_option("senao-bsc-api.cdr.search_string", cdr_search_string, sizeof(cdr_search_string));
	
	api_get_integer_option("senao-bsc-api.cdr.select_handset", &cdr_select_handset);
	api_get_integer_option("senao-bsc-api.cdr.select_base", &cdr_select_base);

	debug_print("cdr_start_date [%s] \n", cdr_start_date);
	debug_print("cdr_end_date [%s] \n", cdr_end_date);

	debug_print("cdr_select_handset [%d] \n", cdr_select_handset);
	debug_print("cdr_select_base [%d] \n", cdr_select_base);
	
	debug_print("cdr_search_string [%s] \n", cdr_search_string);

	/*  read rssi value */

	if(rssi_val != NULL)
	{
		free(rssi_val);
		rssi_val = NULL;
	}
	
	rssi_num = get_column_count_in_time(DB_BSINFO_PATH, DB_BSINFO_TABLE, cdr_start_date, cdr_end_date, "call_rssi");
	
	if(rssi_num > 0)
	{
		rssi_val = (db_rssi_t *)malloc(sizeof(db_rssi_t) * rssi_num);

		if(rssi_val != NULL)
			get_rssi_vale_list(DB_BSINFO_PATH, DB_BSINFO_TABLE, cdr_start_date, cdr_end_date, 
					"call_rssi", rssi_val, rssi_num, cdr_select_base, cdr_select_handset);
	}


	data_base_read(cdr_select_handset, cdr_select_base, 
			cdr_start_date, cdr_end_date, jarr_cdrinfo, page_idx, cdr_search_string);

	if(read_out_row == 0)
	{
		cdr_total_page = 1;
	}
	else
	{
		cdr_total_page = (read_out_row / MAX_CDR_LOG_PER_PAGE);

		if(read_out_row % MAX_CDR_LOG_PER_PAGE)
			cdr_total_page++;
	}

	if(page_idx > cdr_total_page)
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "Page index greater than total page");

    	json_object_object_add(jobj, "total_cdr_page", json_object_new_int(cdr_total_page));
    	json_object_object_add(jobj, "current_cdr_page", json_object_new_int(page_idx));
#if 0
    /* Get dat from SQL database */
    /* TODO - insert your code here */
    
	for(idx = 0; idx < total_cdr; idx++)
	{
		jobj_cdrinfo = json_object_new_object();
		
		/* put data */
    	json_object_object_add(jobj_cdrinfo, "start_time", json_object_new_string(start_time));
    	json_object_object_add(jobj_cdrinfo, "dir", json_object_new_string(dir));
    	json_object_object_add(jobj_cdrinfo, "trunk", json_object_new_string(trunk));
    	json_object_object_add(jobj_cdrinfo, "cid", json_object_new_string(cid));
    	json_object_object_add(jobj_cdrinfo, "from", json_object_new_string(from));
    	json_object_object_add(jobj_cdrinfo, "to", json_object_new_string(to));
    	json_object_object_add(jobj_cdrinfo, "base", json_object_new_int(base));/* base id, end call or answer call? */
    	json_object_object_add(jobj_cdrinfo, "disposition", json_object_new_string(disposition));
    	json_object_object_add(jobj_cdrinfo, "duration", json_object_new_string(duration));
    	json_object_object_add(jobj_cdrinfo, "rssi", json_object_new_int(rssi));/* rssi value on which base */
		
		json_object_array_add(jarr_cdrinfo, jobj_cdrinfo);
	}

#endif
	json_object_object_add(jobj, "cdrinfo", jarr_cdrinfo);
    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

/*****************************************************************
* NAME: sysIsFileExisted
* ---------------------------------------------------------------
* FUNCTION:  Check whether the file is existed 
* INPUT:    
* OUTPUT:   
* Author:   kenny ,2006-12-29
* Modify:   
****************************************************************/
static bool sysIsFileExisted(const char *filename)
{
    int rval;

    // Check file existence.
    rval = access(filename, F_OK);

    return rval ? FALSE : TRUE;
}

int getHandsetRssiData(int *hs_id, int *HsRxBsRssi, int *BsRxHsRssi, int *location)
{
	FILE *fp;
	int file_num = 0;
	char filename[32];
	char cmd[256] = {0};
	char buf[256] = {0};
	char hs_id_str[8] = {0};
	int talk_under_bs;
	int rssi_under_bs;
	int rssi_hs_rx[RAINIER_BASE_NUM];
	int rssi_bs_rx[RAINIER_BASE_NUM];
	int hs_num = 0;
	int ret;

	/* get handset rssi data */
#if 1
	sprintf(filename, "%s/%s", NMS_RSSI_DIR, NMS_RSSI_FILE_NUM);
	if(sysIsFileExisted(filename) == TRUE)
	{
		if((fp = fopen(filename, "r")) != NULL)
		{
			if(fgets(buf, sizeof(buf), fp) != NULL)
			{
				if (ret = sscanf(buf, "%d", &file_num) == 1)
				{
					// success read file_num
					//debug_print("file_num = %d\n", file_num);
					fclose(fp);
				}
				else
				{
					debug_print("can not read parameter from file: %s with error %d\n", filename, ret);
					fclose(fp);
					return 0;
				}
			}
			else
			{
				debug_print("can not read line from file: %s\n", filename);
				fclose(fp);
				return 0;
			}
		}
		else
		{
			debug_print("can not open file: %s\n", filename);
			return 0;
		}
	}
	else
	{
		debug_print("file: %s is not exist\n", filename);
		return 0;
	}
#else
	snprintf(cmd, sizeof(cmd), "cat %s/%s 2>/dev/null", NMS_RSSI_DIR, NMS_RSSI_FILE_NUM);
	sys_interact(buf, sizeof(buf), cmd);
	if((strcmp(buf, "0") == 0) || (strcmp(buf, "1") == 0))
	{
		file_num = atoi(buf);
	}
	else
	{
		return 0;
	}
#endif
	
	sprintf(filename, "%s/%s%d", NMS_RSSI_DIR, NMS_RSSI_FILE, file_num);
	if(sysIsFileExisted(filename) == TRUE)
	{
		if((fp = fopen(filename, "r")) != NULL)
		{
			while(fgets(buf, sizeof(buf), fp) != NULL && (hs_num < RAINIER_HS_NUMBER))
			{
				if (ret = sscanf(buf, "%s %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
							hs_id_str, &talk_under_bs, &rssi_under_bs,
							&rssi_hs_rx[0], &rssi_bs_rx[0], &rssi_hs_rx[1], &rssi_bs_rx[1],
							&rssi_hs_rx[2], &rssi_bs_rx[2], &rssi_hs_rx[3], &rssi_bs_rx[3],
							&rssi_hs_rx[4], &rssi_bs_rx[4], &rssi_hs_rx[5], &rssi_bs_rx[5],
							&rssi_hs_rx[6], &rssi_bs_rx[6], &rssi_hs_rx[7], &rssi_bs_rx[7]) != 19)
				{
					debug_print("can not read all data, only read: %d data\n", ret);
					break;
				}
				else
				{
					if(strncmp(hs_id_str, "HS", 2) == 0)
                    {
                        // HS, 10 - 99
						hs_id[hs_num] = atoi(&hs_id_str[2]);
						if(talk_under_bs)
						{
							HsRxBsRssi[hs_num] = rssi_hs_rx[talk_under_bs-1];
							BsRxHsRssi[hs_num] = rssi_bs_rx[talk_under_bs-1];
							location[hs_num] = talk_under_bs;
						}
						else if(rssi_under_bs)
						{
							HsRxBsRssi[hs_num] = rssi_hs_rx[rssi_under_bs-1];
							BsRxHsRssi[hs_num] = rssi_bs_rx[rssi_under_bs-1];
							location[hs_num] = rssi_under_bs;
						}
						else
						{
							HsRxBsRssi[hs_num] = 0;
							BsRxHsRssi[hs_num] = 0;
							location[hs_num] = 0;
						}

						hs_num++;
                    }
					else
					{
						break;
					}
				}
			}

			fclose(fp);
		}
		else
		{
			debug_print("can not open file: %s\n", filename);
		}
	}
	else
	{
		debug_print("file: %s is not exist\n", filename);
	}
	
	return hs_num;
}

int getHandsetFwStatusData(int *hs_id, int *hs_progress, int *hs_status, int *hs_under_bs, int *bs_progress, int *bs_status)
{
	FILE *fp;
	int file_num = 0;
	char filename[32];
	char cmd[256] = {0};
	char buf[256] = {0};
	char id_str[8] = {0};
	int idx;
	int progress, status, under_bs;
	int hs_num = 0, bs_num = 0;
	int ret;

	/* get handset rssi data */
#if 1
	sprintf(filename, "%s/%s", NMS_HS_FW_DIR, NMS_HS_FW_FILE_NUM);
	if(sysIsFileExisted(filename) == TRUE)
	{
		if((fp = fopen(filename, "r")) != NULL)
		{
			if(fgets(buf, sizeof(buf), fp) != NULL)
			{
				if (ret = sscanf(buf, "%d", &file_num) == 1)
				{
					// success read file_num
					//debug_print("file_num = %d\n", file_num);
					fclose(fp);
				}
				else
				{
					debug_print("can not read parameter from file: %s with error %d\n", filename, ret);
					fclose(fp);
					return 0;
				}
			}
			else
			{
				debug_print("can not read line from file: %s\n", filename);
				fclose(fp);
				return 0;
			}
		}
		else
		{
			debug_print("can not open file: %s\n", filename);
			return 0;
		}
	}
	else
	{
		debug_print("file: %s is not exist\n", filename);
		return 0;
	}
#else
	snprintf(cmd, sizeof(cmd), "cat %s/%s 2>/dev/null", NMS_HS_FW_DIR, NMS_HS_FW_FILE_NUM);
	sys_interact(buf, sizeof(buf), cmd);
	if((strcmp(buf, "0") == 0) || (strcmp(buf, "1") == 0))
	{
		file_num = atoi(buf);
	}
	else
	{
		return 0;
	}
#endif
	
	sprintf(filename, "%s/%s%d", NMS_HS_FW_DIR, NMS_HS_FW_FILE, file_num);
	if(sysIsFileExisted(filename) == TRUE)
	{
		if((fp = fopen(filename, "r")) != NULL)
		{
			while(fgets(buf, sizeof(buf), fp) != NULL && (hs_num < RAINIER_HS_NUMBER))
			{
				ret = sscanf(buf, "%s %d %d %d", id_str, &progress, &status, &under_bs);
				if (ret == 3)
				{
					if((strncmp(id_str, "BS", 2) == 0) && (bs_num < RAINIER_BASE_NUM))
					{
						// BS, 1 - 8
						idx = atoi(&id_str[2]);
						bs_progress[idx-RAINIER_BSID_MIN] = progress;
						bs_status[idx-RAINIER_BSID_MIN] = status;
						bs_num++;
					}
					else
					{
						break;
					}
				}
				else if (ret == 4)
				{
					if(strncmp(id_str, "HS", 2) == 0)
					{
						// HS, 10 - 99
						idx = atoi(&id_str[2]);
						hs_id[idx-RAINIER_HSID_MIN] = idx;
						hs_progress[idx-RAINIER_HSID_MIN] = progress;
						hs_status[idx-RAINIER_HSID_MIN] = status;
						hs_under_bs[idx-RAINIER_HSID_MIN] = under_bs;
						hs_num++;
					}
					else
					{
						break;
					}
				}
				else
				{
					debug_print("can not read all data, only read: %d data\n", ret);
					break;
				}
			}

			fclose(fp);
		}
		else
		{
			debug_print("can not open file: %s\n", filename);
		}
	}
	else
	{
		debug_print("file: %s is not exist\n", filename);
	}
	
	return hs_num;
}

int getBaseCallInfoData(bs_call_info_t *bs_call_info)
{
	FILE *fp;
	char filename[64];
	char buf[256] = {0};
	int bsid, slot, hsid, call_idx, call_type, call_dir, call_status;
	int json_call_type;
	int dst_hsid;
	char call_num[32], call_name[16];
	char display_name[64] = {0};
	int ret;
	char *pt;

	/* nms write base call info to file */
	SYSTEM("rm %s/%s", NMS_CALL_INFO_DIR, NMS_CALL_INFO_BS_FILE);
	SYSTEM("nmsconf_cli table dumpBsCallInfo");
	msleep(500);
	
	/* get base call info data */
	sprintf(filename, "%s/%s", NMS_CALL_INFO_DIR, NMS_CALL_INFO_BS_FILE);
	if(sysIsFileExisted(filename) == TRUE)
	{
		if((fp = fopen(filename, "r")) != NULL)
		{
			// skip first line
			fgets(buf, sizeof(buf), fp);

			while(fgets(buf, sizeof(buf), fp) != NULL)
			{
				ret = sscanf(buf, "%d %d %d %d %d %d %d %s %s",
					&bsid, &slot, &hsid, &call_idx, &call_type, &call_dir, &call_status, call_num, call_name);
				if((ret == 8) || (ret == 9))
				{
					if( (bsid >= RAINIER_BSID_MIN) && (bsid <= RAINIER_BSID_MAX) 
						&& (slot >= 1) && (slot <= 4)
						&& (call_idx >= 1) && (call_idx <= 2) )
					{
						bs_call_info[bsid-RAINIER_BSID_MIN].slot[slot-1].slot_in_used = 1;

						if(hsid == 100)	// rainier group all
						{
							hsid = 0;	// json group all
						}

						bs_call_info[bsid-RAINIER_BSID_MIN].slot[slot-1].hsId = hsid;
						bs_call_info[bsid-RAINIER_BSID_MIN].slot[slot-1].call[call_idx-1].call_in_used = 1;
						
						switch(call_type)
						{
							case CT_SIP:
								json_call_type = 0;	// SIP
								break;
							case CT_FXO:
							case CT_SIPFXO:
								json_call_type = 1;	// FXO
								break;
							case CT_H2H_INT_TRU_BS:
								json_call_type = 2;	// Intercom
								break;
							case CT_H2H_PA_TRU_BS:
								json_call_type = 5;	// Broadcast-HS
								break;
							case CT_F2H_PA_TRU_BS:
								json_call_type = 4;	// Broadcast-FXO
								break;
							default:
								json_call_type = 0;	// SIP
								break;
						}
						bs_call_info[bsid-RAINIER_BSID_MIN].slot[slot-1].call[call_idx-1].type = json_call_type;
						
						bs_call_info[bsid-RAINIER_BSID_MIN].slot[slot-1].call[call_idx-1].dir = call_dir;
						
						bs_call_info[bsid-RAINIER_BSID_MIN].slot[slot-1].call[call_idx-1].state = 2;	// set to 2:Talk
						//bs_call_info[bsid-RAINIER_BSID_MIN].slot[slot-1].call[call_idx-1].state = call_status;

						// call_num for handset to handset PA 88xx
						if((call_type == CT_H2H_PA_TRU_BS) && (call_dir == 0) && (strlen(call_num) == 4))
						{
							strncpy(bs_call_info[bsid-RAINIER_BSID_MIN].slot[slot-1].call[call_idx-1].num, &call_num[2], sizeof(call_num));
						}
						else
						{
							strncpy(bs_call_info[bsid-RAINIER_BSID_MIN].slot[slot-1].call[call_idx-1].num, call_num, sizeof(call_num));
						}

						// call_name for handset to handset PA 88xx
						if((call_type == CT_H2H_PA_TRU_BS) && (call_dir == 0) && (strlen(call_num) == 4))
						{
							dst_hsid = atoi(&call_num[2]);
							if( (dst_hsid >= RAINIER_HSID_MIN) && (dst_hsid <= RAINIER_HSID_MAX) )
							{
								api_get_string_option2(display_name, sizeof(display_name), "sip_hs.sip_hs_%d.display_name", dst_hsid);
								strncpy(bs_call_info[bsid-RAINIER_BSID_MIN].slot[slot-1].call[call_idx-1].name, display_name, sizeof(call_name));
							}
						}
						// call_name for handset to handset intercom
						else if((call_type == CT_H2H_INT_TRU_BS) && (call_dir == 0) && (strlen(call_num) == 2))
						{
							dst_hsid = atoi(call_num);
							if( (dst_hsid >= RAINIER_HSID_MIN) && (dst_hsid <= RAINIER_HSID_MAX) )
							{
								api_get_string_option2(display_name, sizeof(display_name), "sip_hs.sip_hs_%d.display_name", dst_hsid);
								strncpy(bs_call_info[bsid-RAINIER_BSID_MIN].slot[slot-1].call[call_idx-1].name, display_name, sizeof(call_name));
							}
						}
						else if(ret == 9)
						{
							//buf[] content format "%4d  %4d  %4d  %8d  %9d  %8d  %11d  %-32s  %s\n"
							strncpy(call_name, &buf[96], sizeof(call_name));
							if ( (pt = strstr(call_name, "\n")) ) { /* delete tail "\n" */
								*pt = '\0';
							}
							strncpy(bs_call_info[bsid-RAINIER_BSID_MIN].slot[slot-1].call[call_idx-1].name, call_name, sizeof(call_name));
						}
					}
					else
					{
						break;
					}
				}
				else
				{
					debug_print("can not read all data, only read: %d data\n", ret);
					break;
				}
			}

			fclose(fp);
		}
		else
		{
			debug_print("can not open file: %s\n", filename);
			return 0;
		}
	}
	else
	{
		debug_print("file: %s is not exist\n", filename);
		return 0;
	}
	
	return 1;
}

int getHandsetCallInfoData(hs_call_info_t *hs_call_info)
{
	FILE *fp;
	char filename[64];
	char buf[256] = {0};
	int hsid, under_bs, status, call_idx, call_type, call_dir, call_status;
	int json_status, json_call_type;
	int dst_hsid;
	char call_num[32], call_name[16];
	char display_name[64] = {0};
	int ret;
	char *pt;

	/* nms write base call info to file */
	SYSTEM("rm %s/%s", NMS_CALL_INFO_DIR, NMS_CALL_INFO_HS_FILE);
	SYSTEM("nmsconf_cli table dumpHsCallInfo");
	msleep(500);
	
	/* get base call info data */
	sprintf(filename, "%s/%s", NMS_CALL_INFO_DIR, NMS_CALL_INFO_HS_FILE);
	if(sysIsFileExisted(filename) == TRUE)
	{
		if((fp = fopen(filename, "r")) != NULL)
		{
			// skip first line
			fgets(buf, sizeof(buf), fp);

			while(fgets(buf, sizeof(buf), fp) != NULL)
			{
				ret = sscanf(buf, "%d %d %d %d %d %d %d %s %s",
					&hsid, &under_bs, &status, &call_idx, &call_type, &call_dir, &call_status, call_num, call_name);
				if((ret == 8) || (ret == 9))
				{
					if( (hsid >= RAINIER_HSID_MIN) && (hsid <= RAINIER_HSID_MAX)
						&& (call_idx >= 1) && (call_idx <= 2) )
					{
						switch(status)
						{
							case DEAD:
							case NMSRPT_HS_IDLE:
								json_status = 0;	// Free
								break;
							case NMSRPT_HS_SIP_TALK:
							case NMSRPT_HS_SIP_HOLD:
							case NMSRPT_HS_SFXO_TALK:
							case NMSRPT_HS_SFXO_HOLD:
							case NMSRPT_HS_SERVICE_LINK_BS:
								json_status = 1;	// Busy
								break;
							case NMSRPT_HS_SIP_RING:
							case NMSRPT_HS_SFXO_RING:
								json_status = 2;	// Ringing
								break;
							default:
								json_status = 0;	// Free
								break;
						}
						hs_call_info[hsid-RAINIER_HSID_MIN].status = json_status;
						hs_call_info[hsid-RAINIER_HSID_MIN].call[call_idx-1].call_in_used = 1;
						
						switch(call_type)
						{
							case CT_SIP:
								json_call_type = 0;	// SIP
								break;
							case CT_FXO:
							case CT_SIPFXO:
								json_call_type = 1;	// FXO
								break;
							case CT_H2H_INT_TRU_BS:
								json_call_type = 2;	// Intercom
								break;
							case CT_H2H_PA_TRU_BS:
								json_call_type = 5;	// Broadcast-HS
								break;
							case CT_F2H_PA_TRU_BS:
								json_call_type = 4;	// Broadcast-FXO
								break;
							default:
								json_call_type = 0;	// SIP
								break;
						}
						hs_call_info[hsid-RAINIER_HSID_MIN].call[call_idx-1].type = json_call_type;
						
						hs_call_info[hsid-RAINIER_HSID_MIN].call[call_idx-1].dir = call_dir;
						
						hs_call_info[hsid-RAINIER_HSID_MIN].call[call_idx-1].state = 2;	// set to 2:Talk
						//hs_call_info[hsid-RAINIER_HSID_MIN].call[call_idx-1].state = call_status;

						// call_num for handset to handset PA 88xx
						if((call_type == CT_H2H_PA_TRU_BS) && (call_dir == 0) && (strlen(call_num) == 4))
						{
							strncpy(hs_call_info[hsid-RAINIER_HSID_MIN].call[call_idx-1].num, &call_num[2], sizeof(call_num));
						}
						else
						{
							strncpy(hs_call_info[hsid-RAINIER_HSID_MIN].call[call_idx-1].num, call_num, sizeof(call_num));
						}

						// call_name for handset to handset PA 88xx
						if((call_type == CT_H2H_PA_TRU_BS) && (call_dir == 0) && (strlen(call_num) == 4))
						{
							dst_hsid = atoi(&call_num[2]);
							if( (dst_hsid >= RAINIER_HSID_MIN) && (dst_hsid <= RAINIER_HSID_MAX) )
							{
								api_get_string_option2(display_name, sizeof(display_name), "sip_hs.sip_hs_%d.display_name", dst_hsid);
								strncpy(hs_call_info[hsid-RAINIER_HSID_MIN].call[call_idx-1].name, display_name, sizeof(call_name));
							}
						}
						// call_name for handset to handset intercom
						else if((call_type == CT_H2H_INT_TRU_BS) && (call_dir == 0) && (strlen(call_num) == 2))
						{
							dst_hsid = atoi(call_num);
							if( (dst_hsid >= RAINIER_HSID_MIN) && (dst_hsid <= RAINIER_HSID_MAX) )
							{
								api_get_string_option2(display_name, sizeof(display_name), "sip_hs.sip_hs_%d.display_name", dst_hsid);
								strncpy(hs_call_info[hsid-RAINIER_HSID_MIN].call[call_idx-1].name, display_name, sizeof(call_name));
							}
						}
						else if(ret == 9)
						{
							//buf[] content format "%4d  %8d  %6d  %8d  %9d  %8d  %11d  %-32s  %s\n"
							strncpy(call_name, &buf[102], sizeof(call_name));
							if ( (pt = strstr(call_name, "\n")) ) { /* delete tail "\n" */
								*pt = '\0';
							}
							strncpy(hs_call_info[hsid-RAINIER_HSID_MIN].call[call_idx-1].name, call_name, sizeof(call_name));
						}
					}
					else
					{
						break;
					}
				}
				else if(ret == 3)
				{
					if( (hsid >= RAINIER_HSID_MIN) && (hsid <= RAINIER_HSID_MAX) )
					{
						switch(status)
						{
							case DEAD:
							case NMSRPT_HS_IDLE:
								json_status = 0;	// Free
								break;
							case NMSRPT_HS_SIP_TALK:
							case NMSRPT_HS_SIP_HOLD:
							case NMSRPT_HS_SFXO_TALK:
							case NMSRPT_HS_SFXO_HOLD:
							case NMSRPT_HS_SERVICE_LINK_BS:
								json_status = 1;	// Busy
								break;
							case NMSRPT_HS_SIP_RING:
							case NMSRPT_HS_SFXO_RING:
								json_status = 2;	// Ringing
								break;
							default:
								json_status = 0;	// Free
								break;
						}
						hs_call_info[hsid-RAINIER_HSID_MIN].status = json_status;
					}
					else
					{
						break;
					}
				}
				else
				{
					debug_print("can not read all data, only read: %d data\n", ret);
					break;
				}
			}

			fclose(fp);
		}
		else
		{
			debug_print("can not open file: %s\n", filename);
			return 0;
		}
	}
	else
	{
		debug_print("file: %s is not exist\n", filename);
		return 0;
	}
	
	return 1;
}

int getBaseCallInUseData(int *bs_call_inuse)
{
	FILE *fp;
	char filename[64];
	char buf[256] = {0};
	int bsid, inuse;
	int ret;

	/* nms write base call info to file */
	SYSTEM("rm %s/%s", NMS_CALL_INFO_DIR, NMS_CALL_INFO_BS_INUSE_FILE);
	SYSTEM("nmsconf_cli table dumpBsCallInUse");
	msleep(500);
	
	/* get base call info data */
	sprintf(filename, "%s/%s", NMS_CALL_INFO_DIR, NMS_CALL_INFO_BS_INUSE_FILE);
	if(sysIsFileExisted(filename) == TRUE)
	{
		if((fp = fopen(filename, "r")) != NULL)
		{
			// skip first line
			fgets(buf, sizeof(buf), fp);

			while(fgets(buf, sizeof(buf), fp) != NULL)
			{
				ret = sscanf(buf, "%d %d", &bsid, &inuse);
				if(ret == 2)
				{
					if( (bsid >= RAINIER_BSID_MIN) && (bsid <= RAINIER_BSID_MAX) )
					{
						bs_call_inuse[bsid-RAINIER_BSID_MIN] = inuse;
					}
					else
					{
						break;
					}
				}
				else
				{
					debug_print("can not read all data, only read: %d data\n", ret);
					break;
				}
			}

			fclose(fp);
		}
		else
		{
			debug_print("can not open file: %s\n", filename);
			return 0;
		}
	}
	else
	{
		debug_print("file: %s is not exist\n", filename);
		return 0;
	}
	
	return 1;
}

int json_get_rainier_handset_rssi_list(ResponseEntry *rep, struct json_object *jobj)
{
	//int hs_id = 0, HsRxBsRssi = 0, BsRxHsRssi = 0, location = 0;
	int hs_id[RAINIER_HS_NUMBER] = {0};
	int HsRxBsRssi[RAINIER_HS_NUMBER] = {0};
	int BsRxHsRssi[RAINIER_HS_NUMBER] = {0};
	int location[RAINIER_HS_NUMBER] = {0};
	char display_name[64] = {0};
	char bs_name[32] = {0};
	int hs_num = 0;
	int i;
	int idx;
	ResponseStatus *res = rep->res;

	struct json_object *jarr_hs_rssi = NULL, *jobj_hs_rssi = NULL;
	jarr_hs_rssi = json_object_new_array();


    /* get handset rssi data */
	hs_num = getHandsetRssiData(hs_id, HsRxBsRssi, BsRxHsRssi, location);

#if 1
	for(i = 0; i < hs_num; i++)
	{
		jobj_hs_rssi = json_object_new_object();
		json_object_object_add(jobj_hs_rssi, "hs_id", json_object_new_int(hs_id[i]));
		api_get_string_option2(display_name, sizeof(display_name), "sip_hs.sip_hs_%d.display_name", hs_id[i]);
		json_object_object_add(jobj_hs_rssi, "display_name", json_object_new_string(display_name));
		json_object_object_add(jobj_hs_rssi, "HsRxBsRssi", json_object_new_int(HsRxBsRssi[i]));
		json_object_object_add(jobj_hs_rssi, "BsRxHsRssi", json_object_new_int(BsRxHsRssi[i]));
		json_object_object_add(jobj_hs_rssi, "location", json_object_new_int(location[i]));

		/* check the base is registered to BSC or not */
		idx = findBsConfigIdxByIndex(location[i]);
		if(idx != -1) // bs is in the list
		{
			api_get_string_option2(bs_name, sizeof(bs_name), "base-station-list.@base-station[%d].name", idx);
			json_object_object_add(jobj_hs_rssi, "bs_name", json_object_new_string(bs_name));
		}
		else
		{
			json_object_object_add(jobj_hs_rssi, "bs_name", json_object_new_string(""));
		}

		json_object_array_add(jarr_hs_rssi, jobj_hs_rssi);

		//debug_print("handset rssi: HS%d %d/%d under BS%d\n", hs_id[i], HsRxBsRssi[i], BsRxHsRssi[i], location[i]);
	}
#else
	for(i = RAINIER_HSID_MIN; i <= RAINIER_HSID_MAX; i++)
	{
		jobj_hs_rssi = json_object_new_object();
		json_object_object_add(jobj_hs_rssi, "hs_id", json_object_new_int(hs_id));
		json_object_object_add(jobj_hs_rssi, "HsRxBsRssi", json_object_new_int(HsRxBsRssi));
		json_object_object_add(jobj_hs_rssi, "BsRxHsRssi", json_object_new_int(BsRxHsRssi));
		json_object_object_add(jobj_hs_rssi, "location", json_object_new_int(location));
		json_object_array_add(jarr_hs_rssi, jobj_hs_rssi);
	}
#endif

	json_object_object_add(jobj, "hs_rssi", jarr_hs_rssi);

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_post_rainier_bsRxhsRssi(ResponseEntry *rep, char *query_str)
{
    struct json_object *jobj = NULL;
    ResponseStatus *res = rep->res;
    int bs_id = 0, hs_id = 0, rssi = 0;

    if(NULL != query_str)
    {
        if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
            /* get data from base */
            senao_json_object_get_integer(jobj, "bs_id", &bs_id);
            senao_json_object_get_integer(jobj, "hs_id", &hs_id);
            senao_json_object_get_integer(jobj, "rssi", &rssi);
            /* save into table */
            /* TODO - insert your code here */
        }
        else
        {
            RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
        }
    }
    else
    {
        RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
    }
    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_post_rainier_hsRxbsRssi(ResponseEntry *rep, char *query_str)
{
    struct json_object *jobj = NULL;
    ResponseStatus *res = rep->res;
    int bs_id = 0, hs_id = 0, rssi = 0;

    if(NULL != query_str)
    {
        if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
            /* get data from base */
            senao_json_object_get_integer(jobj, "bs_id", &bs_id);
            senao_json_object_get_integer(jobj, "hs_id", &hs_id);
            senao_json_object_get_integer(jobj, "rssi", &rssi);
            /* save into table */
            /* TODO - insert your code here */

        }
        else
        {
            RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
        }
    }
    else
    {
        RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
    }
    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_get_hs_rssi_idx(ResponseEntry *rep, struct json_object *jobj, int hs_idx)
{
    int bsRxHsRssi = 0, hsRxBsRssi = 0;
    ResponseStatus *res = rep->res;
#if 1
    /* get data from database or tmp file */

#endif
    /* put data */
    json_object_object_add(jobj, "bsRxHsRssi", json_object_new_int(bsRxHsRssi));
    json_object_object_add(jobj, "hsRxBsRssi", json_object_new_int(hsRxBsRssi));

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_get_bs_rssi_idx(ResponseEntry *rep, struct json_object *jobj, int bs_idx)
{
    int bsRxHsRssi = 0, hsRxBsRssi = 0;
    ResponseStatus *res = rep->res;
#if 1
    /* get data from database or tmp file */

#endif
    /* put data */
    json_object_object_add(jobj, "bsRxHsRssi", json_object_new_int(bsRxHsRssi));
    json_object_object_add(jobj, "hsRxBsRssi", json_object_new_int(hsRxBsRssi));

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_get_rainier_bs_rssi_info(ResponseEntry *rep, struct json_object *jobj, int bs_idx)
{
    /* get base index then return all handset's rssi info on this specific base */
    int i;
    char idx_buf[4];
    struct json_object *jobj_rssi = NULL;
    ResponseStatus *res = rep->res;
    /* TODO - insert your code here */
    /* Get bs_id(1-8) or hs_id(10-99) */
	if((bs_idx >= RAINIER_BSID_MIN) && (bs_idx <= RAINIER_BSID_MAX))
    {
        /* return all hs' rssi on this base */
        for(i = RAINIER_HSID_MIN; i <= RAINIER_HSID_MAX; i++)
        {
            sprintf(idx_buf, "%d", i);/* index 10~99 for hsid */
            jobj_rssi = newObjectFromStack(rep);
            /* get data from tmp file */
            json_get_hs_rssi_idx(rep, jobj_rssi, i);/* get all hs' rssi data on this bs */
            /* put data in json format */
            json_object_object_add(jobj, idx_buf, jobj_rssi);
        }
    }
    else
    {
        /* incorrect range */
        RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "BASE ID");        
    }
    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_get_rainier_hs_rssi_info(ResponseEntry *rep, struct json_object *jobj, int hs_idx)
{
    int i;
    char idx_buf[4];
    struct json_object *jobj_rssi = NULL;
    ResponseStatus *res = rep->res;
    /* TODO */
    /* hs_id(10-99) */
    if((hs_idx >= RAINIER_HSID_MIN) && (hs_idx <= RAINIER_HSID_MAX))
    {
        /* return hs' rssi with bs1-8 */
        for(i = RAINIER_BSID_MIN; i <= RAINIER_BSID_MAX; i++)
        {
            sprintf(idx_buf, "%d", i);/* index 1~8 for bsid */
            jobj_rssi = newObjectFromStack(rep);
            /* get data from tmp file */
            json_get_bs_rssi_idx(rep, jobj_rssi, i);/* get all bs' rssi with the specific hsid */
            /* put data in json format */
            json_object_object_add(jobj, idx_buf, jobj_rssi);
        }

    }
    else
    {
        /* incorrect range */
        RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "HS ID");        
    }
    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

/* Base report result to BSC */
int json_post_hsid(ResponseEntry *rep, char *query_str)
{
    int hs_id = 0;
	int hs_group = 0;
	int reg_hs_id = 0;
	int reg_result = 0;
	char *fw_version = NULL, *hw_version = NULL, *serial_number = NULL;
    struct json_object *jobj = NULL;
    ResponseStatus *res = rep->res;
	char cmd[256] = {0};
	char buf[256] = {0};
	char reg_mode[8] = {0};
	char reg_state[8] = {0};

    /* base return hs_id is register or not. */
    if(NULL != query_str)
    {
        if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
            /* get which hsid */
            senao_json_object_get_integer(jobj, "hs_id", &reg_hs_id);
            /* get result */
            senao_json_object_get_integer(jobj, "result", &reg_result);
            /* get fw_version */
            senao_json_object_get_and_create_string(rep, jobj, "fw_version", &fw_version);
            /* get hw_version */
            senao_json_object_get_and_create_string(rep, jobj, "hw_version", &hw_version);
            /* get serial_number */
            senao_json_object_get_and_create_string(rep, jobj, "serial_number", &serial_number);

			sys_interact(buf, sizeof(buf), "cat /tmp/rainier_hs_reg 2>/dev/null");
			sscanf(buf, "%s hs_%d grp_%d %s", reg_mode, &hs_id, &hs_group, reg_state);
			
			/* check hs_id is in range or not (10-99) */
			if((hs_id >= RAINIER_HSID_MIN) && (hs_id <= RAINIER_HSID_MAX))
			{
	            /* action */
    	        if(reg_result == REG_DEREG_SUCCESS)
        	    {
                	/* record this hs_id is inuse or clean and update hsid table */
					if((strcmp(reg_mode, "reg") == 0) && (strcmp(reg_state, "rsvd") == 0) && (reg_hs_id == hs_id))
					{
						/* check hs_id, hs_group is in range or not (10-99) */

						// set fw_version, hw_version, serial_number
						//snprintf(cmd, sizeof(cmd), "uci set sip_hs.sip_hs_%d.fw_version='%s'", hs_id, fw_version);
						//sys_interact(buf, sizeof(buf), cmd);
						api_set_string_option2(fw_version, sizeof(fw_version), "sip_hs.sip_hs_%d.fw_version", hs_id);
						//snprintf(cmd, sizeof(cmd), "uci set sip_hs.sip_hs_%d.hw_version='%s'", hs_id, hw_version);
						//sys_interact(buf, sizeof(buf), cmd);
						api_set_string_option2(hw_version, sizeof(hw_version), "sip_hs.sip_hs_%d.hw_version", hs_id);
						//snprintf(cmd, sizeof(cmd), "uci set sip_hs.sip_hs_%d.serial_number='%s'", hs_id, serial_number);
						//sys_interact(buf, sizeof(buf), cmd);
						api_set_string_option2(serial_number, sizeof(serial_number), "sip_hs.sip_hs_%d.serial_number", hs_id);

						// record this hs_id is register ok
						SYSTEM("echo reg hs_%d grp_%d ok > /tmp/rainier_hs_reg", hs_id, hs_group);
					}
					else if((strcmp(reg_mode, "dereg") == 0) && (strcmp(reg_state, "rsvd") == 0 && (reg_hs_id == hs_id)))
					{
						/* check hs_id, hs_group is in range or not (10-99) */

						// record this hs_id is de-register ok
						SYSTEM("echo dereg hs_%d grp_%d ok > /tmp/rainier_hs_reg", hs_id, hs_group);
					}
					else
					{
						RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "HS ID");
					}
            	}
            	else if(reg_result == REG_DEREG_FAIL)
            	{
	                /* record this hs_id is free to use. this condition is used for register fail or de-register */
					//api_set_bool_option2(0, "sip_hs.sip_hs_%d.rainierReg", hs_id);
					if((strcmp(reg_mode, "reg") == 0) && (strcmp(reg_state, "rsvd") == 0) && (reg_hs_id == hs_id))
					{
						/* check hs_id, hs_group is in range or not (10-99) */

						// record this hs_id is register ng
						SYSTEM("echo reg hs_%d grp_%d ng > /tmp/rainier_hs_reg", hs_id, hs_group);
					}
					else if((strcmp(reg_mode, "dereg") == 0) && (strcmp(reg_state, "rsvd") == 0) && (reg_hs_id == hs_id))
					{
						/* check hs_id, hs_group is in range or not (10-99) */

						// record this hs_id is de-register ng
						SYSTEM("echo dereg hs_%d grp_%d ng > /tmp/rainier_hs_reg", hs_id, hs_group);
					}
					else
					{
						RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "HS ID");
					}
            	}
				else
				{
					RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "RESULT");
				}
			}
			else
			{
				RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "HS ID");
			}
			
        }
        else
        {
            RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
        }
    }
    else
    {
        RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
    }
    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

/* BASE request hsid from BSC for reg/dereg. */
int json_get_hsid(ResponseEntry *rep, struct json_object *jobj)
{
    int hs_id = 0;
	int hs_group = 0;
	int rainierReg = 0;
	int i;
    ResponseStatus *res = rep->res;
	char cmd[256] = {0};
	char buf[256] = {0};
	char reg_mode[8] = {0};
	char reg_state[8] = {0};
    
	/* get hs_id and return valid hs id to base */
#if 1
	sys_interact(buf, sizeof(buf), "cat /tmp/rainier_hs_reg 2>/dev/null");
	sscanf(buf, "%s hs_%d grp_%d %s", reg_mode, &hs_id, &hs_group, reg_state);

	if((strcmp(reg_mode, "reg") == 0) && (strcmp(reg_state, "start") == 0))
	{
		/* check hs_id, hs_group is in range or not (10-99) */

		// record this hs_id is reserved by one BS
		SYSTEM("echo reg hs_%d grp_%d rsvd > /tmp/rainier_hs_reg", hs_id, hs_group);
	}
	else if((strcmp(reg_mode, "dereg") == 0) && (strcmp(reg_state, "start") == 0))
	{
		/* check hs_id, hs_group is in range or not (10-99) */

		// record this hs_id is reserved by one BS
		SYSTEM("echo dereg hs_%d grp_%d rsvd > /tmp/rainier_hs_reg", hs_id, hs_group);
	}
	else
	{
		hs_id = 0;
		hs_group = 0;
	}
	
#else
	for(i=RAINIER_HSID_MIN;i<=RAINIER_HSID_MAX;i++)
	{
		if(api_get_bool_option2(&rainierReg, "sip_hs.sip_hs_%d.rainierReg", i) == API_RC_SUCCESS)
		{
			if(rainierReg == 0)
			{
				/* record this hs_id is used */
				api_set_bool_option2(1, "sip_hs.sip_hs_%d.rainierReg", i);
				hs_id = i;
				break;
			}
		}
	}

    /* check hs_id is in range or not (10-99) */
#endif

    /* transmit hs_id, hs_group to base */
    json_object_object_add(jobj, "hs_id", json_object_new_int(hs_id));
	json_object_object_add(jobj, "hs_group", json_object_new_int(hs_group));
    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int getPhbookCount()
{
	char cmd[256] = {0};
	char buf[256] = {0};

	//snprintf(cmd, sizeof(cmd), "grep -w \"phbook\" -c /etc/config/phbook");
	snprintf(cmd, sizeof(cmd), "uci show phbook | grep -w -c \"=phbook\"");
	sys_interact(buf, sizeof(buf), cmd);

	return atoi(buf);
}

int findPhbookIdxByIndex(int index)
{
	char cmd[256] = {0};
	char buf[256] = {0};
	int i;
	int total_nums;

	total_nums = getPhbookCount();

	for(i = 0; i < total_nums; i++)
	{
		//snprintf(cmd, sizeof(cmd), "uci get phbook.@phbook[%d].index | tr -d \"\\n\"", i);
		//sys_interact(buf, sizeof(buf), cmd);
		api_get_string_option2(buf, sizeof(buf), "phbook.@phbook[%d].index", i);

		if(atoi(buf) == index)
		{
			return i;
		}
	}

	return -1;
}

int reorderPhbookIndex()
{
	int i;
	int total_nums;

	total_nums = getPhbookCount();

	for(i = 0; i < total_nums; i++)
	{
		//SYSTEM("uci set phbook.@phbook[%d].index=%d", i, i+1);
		api_set_integer_option2(i+1, "phbook.@phbook[%d].index", i);
	}

	//SYSTEM("uci commit phbook");
	api_commit_option("phbook");

	return 0;
}

int deleteAllPhbook()
{
	int i;
	int total_nums;

	total_nums = getPhbookCount();

	for(i = 0; i < total_nums; i++)
	{
		//SYSTEM("uci delete phbook.@phbook[0]");
		api_delete_option("phbook.@phbook[0]", "");
	}

	//SYSTEM("uci commit phbook");
	api_commit_option("phbook");

	return 0;
}

// "BSC" -> BS (BSC side)
int set_pb_transfer(char *pb_str)
{
	char cmd[1024] = {0};
	char buf[256] = {0};

	char username[32] = {0};
	char password[32] = {0};

	char device_token[256] = {0};
	char *bsc_id;

	char bs_ip[32] = {0};

	int idx = -1;
	int bs_num;
	int i;
	int ret = -1;

	debug_print("Jason DEBUG %s[%d] phbook: %s\n", __FUNCTION__, __LINE__, pb_str);

	// record phbook json string to tmp file
	SYSTEM("echo '%s' > /tmp/phbook", pb_str);

	bsc_id = (char*)malloc(13*sizeof(char));
	getBscId(bsc_id, 13);

	bs_num = getBsCount();

	for(i = 0; i < bs_num; i++)
	{
#if 1	// use NMS report BS Status
		//snprintf(cmd, sizeof(cmd), "uci get base-station-list.@base-station[%d].index | tr -d \"\\n\"", i);
		//sys_interact(buf, sizeof(buf), cmd);
		api_get_string_option2(buf, sizeof(buf), "base-station-list.@base-station[%d].index", i);
		idx = atoi(buf);

		sprintf(cmd, "%s/%s_%d", NMS_DEVICE_DIR, NMS_DEVICE_BS, idx);
		if(sysIsFileExisted(cmd) != TRUE)
		{
			debug_print("BS%d offline, continue to next base.\n", idx);
			continue;	// the base offline, continue to next base
		}
#endif

		snprintf(username, sizeof(username), "%s", bsc_id);

		//snprintf(cmd, sizeof(cmd), "uci get base-station-list.@base-station[%d].bs_key | tr -d \"\\n\"", i);
		//sys_interact(password, sizeof(password), cmd);
		api_get_string_option2(password, sizeof(password), "base-station-list.@base-station[%d].bs_key", i);

		//snprintf(cmd, sizeof(cmd), "uci get base-station-list.@base-station[%d].ip | tr -d \"\\n\"", i);
		//sys_interact(bs_ip, sizeof(bs_ip), cmd);
		api_get_string_option2(bs_ip, sizeof(bs_ip), "base-station-list.@base-station[%d].ip", i);

		snprintf(cmd, sizeof(cmd), "curl --connect-timeout 3 --max-time 20 -v -k -X POST \"https://%s:4430/api/sys/login\" -H \"accept: */*\" -H \"Content-Type: application/json\" -d \"{\\\"username\\\":\\\"%s\\\",\\\"password\\\":\\\"%s\\\"}\" | grep token | awk '{for (i=1;i<=NF;i++){if ($i ~/token/) {print $(i+1)}}}' | sed -e 's/\"//g' | tr -d \"\\n\"", bs_ip, username, password);
		debug_print("cmd: %s.", cmd);
		sys_interact_long(device_token, sizeof(device_token), cmd);
		debug_print("device_token: %s.", device_token);

		if(strcmp(device_token, "---") == 0)
		{
			continue;	// login fails, continue to next base
		}

		snprintf(cmd, sizeof(cmd), "curl --connect-timeout 3 --max-time 20 -v -k -X POST \"https://%s:4430/api/mgm/rainier/pb_transfer\" -H \"accept: */*\" -H \"Authorization: Bearer %s\" -d @/tmp/phbook | awk '{for (i=1;i<=NF;i++){if ($i ~/status_code/) {print $(i+1)}}}' | sed -e 's/\"//g' | tr -d \"\\n\"", bs_ip, device_token);
		sys_interact_long(buf, sizeof(buf), cmd);
		debug_print("buf: %s.", buf);

		if(strcmp(buf, "200,") == 0) // OK
		{
			// at least one BS receive register mode command and enter the corresponding register/de-register/normal mode. 
			ret = 0;
		}
	}

	free(bsc_id);

	return ret;
}

int json_post_push_phbook(ResponseEntry *rep, char *query_str)
{
	bool push_phbook_enable = 0;
	struct json_object *jobj = NULL;
	ResponseStatus *res = rep->res;
	int i;
	int total_nums;
	char phbook_name[32] = {0}, phbook_number[32] = {0};
	int phbook_index = 0;
	int status;
	
	struct json_object *jarr_phbook = NULL, *jobj_phbook = NULL, *jobj_tmp = NULL;

	/* BSC receives indicator from GUI and starts to do phonebook transfer to handset */
#if 0
	if(NULL != query_str)
	{
		if((jobj = jsonTokenerParseFromStack(rep, query_str)))
		{
			senao_json_object_get_boolean(jobj, "enable", &push_phbook_enable);
		}
		else
		{
			RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
		}
	}
	else
	{
		RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
	}

	if(push_phbook_enable)
	{
		/* start phonebook transfer */
	}
	else
	{
		/* stop phonebook transfer */
	}
#endif
	
	jobj_tmp = json_object_new_object();
	jarr_phbook = json_object_new_array();
	total_nums = getPhbookCount();

    /* Get phonebook data */
	for(i = RAINIER_PHONEBOOK_MIN; i <= total_nums; i++)
    {
		jobj_phbook = json_object_new_object();
		
		api_get_integer_option2(&phbook_index, "phbook.@phbook[%d].index", i-1);
    	api_get_string_option2(phbook_name, sizeof(phbook_name), "phbook.@phbook[%d].name", i-1);
		api_get_string_option2(phbook_number, sizeof(phbook_number), "phbook.@phbook[%d].number", i-1);

    	json_object_object_add(jobj_phbook, "index", json_object_new_int(phbook_index));
		json_object_object_add(jobj_phbook, "name", json_object_new_string(phbook_name));
    	json_object_object_add(jobj_phbook, "number", json_object_new_string(phbook_number));

		json_object_array_add(jarr_phbook, jobj_phbook);
	}

	json_object_object_add(jobj_tmp, "phbooks", jarr_phbook);


	status = set_pb_transfer((char *)json_object_to_json_string(jobj_tmp));

	json_object_put(jobj_tmp);

	if(status == -1)
	{
		RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "Unable to push phonebook to Base");
	}

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

/* Used for import phbook, delete all the existing phbook, and add phbook from the data */
int json_post_phbooks(ResponseEntry *rep, char *query_str)
{
    char *phbook_name[RAINIER_PHONEBOOK_NUM] = {NULL}, *phbook_number[RAINIER_PHONEBOOK_NUM] = {NULL};
	int phbook_index[RAINIER_PHONEBOOK_NUM] = {0};
	int arraylen = 0, i, idx;
	struct json_object *jobj = NULL, *jarr = NULL, *jarr_obj = NULL;
    ResponseStatus *res = rep->res;
    
	/* POST phonebook data to BSC */
    if(NULL != query_str)
    {
        if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
			jarr = json_object_object_get(jobj, "phbooks");
            arraylen = json_object_array_length(jarr) ;
			
			if(arraylen > RAINIER_PHONEBOOK_NUM)
			{
				RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "Too many Phonebook index");
			}

            for (i = 0; i < arraylen; i++) 
            {
                jarr_obj = json_object_array_get_idx(jarr, i);
				senao_json_object_get_integer(jarr_obj, "index", &phbook_index[i]);
				senao_json_object_get_and_create_string(rep, jarr_obj, "name", &phbook_name[i]);
            	senao_json_object_get_and_create_string(rep, jarr_obj, "number", &phbook_number[i]);

				//debug_print("phbook index: %d.\n", phbook_index[i]);
				//debug_print("phbook name: %s.\n", phbook_name[i]);
				//debug_print("phbook number: %s.\n", phbook_number[i]);
            }
        }
        else
        {
            RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
        }
    }
    else
    {
        RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
    }

	deleteAllPhbook();
    
	for (i = 0; i < arraylen; i++)
    {
#if 1
		api_add_new_section("phbook", "phbook");
		api_set_integer_option("phbook.@phbook[-1].index", i+1);
		api_set_string_option("phbook.@phbook[-1].name", phbook_name[i], sizeof(phbook_name[i]));
		api_set_string_option("phbook.@phbook[-1].number", phbook_number[i], sizeof(phbook_number[i]));
#else
		SYSTEM("uci add phbook phbook");
		SYSTEM("uci set phbook.@phbook[-1].index=%d", i+1);
		SYSTEM("uci set phbook.@phbook[-1].name='%s'", phbook_name[i]);
		SYSTEM("uci set phbook.@phbook[-1].number='%s'", phbook_number[i]);
#endif
    }
	//SYSTEM("uci commit phbook");
	api_commit_option("phbook");

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

/* client request phonebook data from BSC.
 *
 */
int json_get_phbooks(ResponseEntry *rep, struct json_object *jobj)
{
    //char idx_buf[4];
    ResponseStatus *res = rep->res;
    int i;
	int total_nums;
	
	struct json_object *jarr_phbook = NULL, *jobj_phbook = NULL;
	jarr_phbook = json_object_new_array();

	total_nums = getPhbookCount();

    /* Get phonebook data from BSC to SP938BS/GUI */
	for(i = RAINIER_PHONEBOOK_MIN; i <= total_nums; i++)
    {
		jobj_phbook = json_object_new_object();
		json_get_phbook_idx(rep, jobj_phbook, i);
		json_object_array_add(jarr_phbook, jobj_phbook);
	}

	json_object_object_add(jobj, "phbooks", jarr_phbook);

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_post_phbook_idx(ResponseEntry *rep, char *query_str, int phbook_idx)
{
    char *phbook_name = NULL, *phbook_number = NULL;
	int phbook_index = 0;
	int total_nums;
    struct json_object *jobj = NULL;
    ResponseStatus *res = rep->res;

	total_nums = getPhbookCount();

	if((phbook_idx < RAINIER_PHONEBOOK_MIN) || (phbook_idx > RAINIER_PHONEBOOK_MAX) || (phbook_idx > total_nums))
	{
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "PHBOOK ID");
	}
	
    /* POST phonebook data to BSC */
    if(NULL != query_str)
    {
        if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
            //senao_json_object_get_integer(jobj, "index", &phbook_index);
			senao_json_object_get_and_create_string(rep, jobj, "name", &phbook_name);
            senao_json_object_get_and_create_string(rep, jobj, "number", &phbook_number);
        }
        else
        {
            RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
        }
    }
    else
    {
        RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
    }
    
	//api_set_integer_option2(phbook_index, "phbook.@phbook[%d].index", phbook_idx-1);
	api_set_string_option2(phbook_name, sizeof(phbook_name), "phbook.@phbook[%d].name", phbook_idx-1);
	api_set_string_option2(phbook_number, sizeof(phbook_number), "phbook.@phbook[%d].number", phbook_idx-1);

	api_commit_option("phbook");
	//SYSTEM("uci commit phbook");


    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_get_phbook_idx(ResponseEntry *rep, struct json_object *jobj, int phbook_idx)
{
    char phbook_name[32] = {0}, phbook_number[32] = {0};
	int phbook_index = 0;
	int total_nums;
    ResponseStatus *res = rep->res;

	total_nums = getPhbookCount();

	if((phbook_idx < RAINIER_PHONEBOOK_MIN) || (phbook_idx > RAINIER_PHONEBOOK_MAX) || (phbook_idx > total_nums))
	{
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "PHBOOK ID");
	}

    api_get_integer_option2(&phbook_index, "phbook.@phbook[%d].index", phbook_idx-1);
    api_get_string_option2(phbook_name, sizeof(phbook_name), "phbook.@phbook[%d].name", phbook_idx-1);
	api_get_string_option2(phbook_number, sizeof(phbook_number), "phbook.@phbook[%d].number", phbook_idx-1);

    /* put data */
    json_object_object_add(jobj, "index", json_object_new_int(phbook_index));
	json_object_object_add(jobj, "name", json_object_new_string(phbook_name));
    json_object_object_add(jobj, "number", json_object_new_string(phbook_number));

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_post_add_phbook(ResponseEntry *rep, char *query_str)
{
    char *phbook_name = NULL, *phbook_number = NULL;
	int phbook_index = 0;
	int total_nums;
    struct json_object *jobj = NULL;
    ResponseStatus *res = rep->res;

	total_nums = getPhbookCount();

	if(total_nums >= RAINIER_PHONEBOOK_NUM)
	{
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "Phonebook full");
	}

    /* POST phonebook data to BSC */
    if(NULL != query_str)
    {
        if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
            senao_json_object_get_and_create_string(rep, jobj, "name", &phbook_name);
            senao_json_object_get_and_create_string(rep, jobj, "number", &phbook_number);
        }
        else
        {
            RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
        }
    }
    else
    {
        RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
    }
    
	phbook_index = total_nums + 1;

#if 1
	api_add_new_section("phbook", "phbook");
	api_set_integer_option("phbook.@phbook[-1].index", phbook_index);
	api_set_string_option("phbook.@phbook[-1].name", phbook_name, sizeof(phbook_name));
	api_set_string_option("phbook.@phbook[-1].number", phbook_number, sizeof(phbook_number));
	api_commit_option("phbook");
#else
	SYSTEM("uci add phbook phbook");
	SYSTEM("uci set phbook.@phbook[-1].index=%d", phbook_index);
	SYSTEM("uci set phbook.@phbook[-1].name='%s'", phbook_name);
	SYSTEM("uci set phbook.@phbook[-1].number='%s'", phbook_number);
	SYSTEM("uci commit phbook");
#endif

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_post_delete_phbooks(ResponseEntry *rep, char *query_str)
{
    //char *phbook_name = NULL, *phbook_number = NULL;
	int phbook_index[RAINIER_PHONEBOOK_NUM] = {0};
	int arraylen = 0, i, idx;
	struct json_object *jobj = NULL, *jarr = NULL, *jarr_obj = NULL;
    ResponseStatus *res = rep->res;
    /* POST phonebook data to BSC */
    if(NULL != query_str)
    {
        if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
			jarr = json_object_object_get(jobj, "phbooks");
            arraylen = json_object_array_length(jarr) ;
			
			if(arraylen > RAINIER_PHONEBOOK_NUM)
			{
				RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "Too many Phonebook index");
			}

            for (i = 0; i < arraylen; i++) 
            {
                jarr_obj = json_object_array_get_idx(jarr, i);
				senao_json_object_get_integer(jarr_obj, "index", &phbook_index[i]);

				//debug_print("delete phbook index: %d.\n", phbook_index[i]);
            }
        }
        else
        {
            RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
        }
    }
    else
    {
        RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
    }
    
	for (i = 0; i < arraylen; i++)
    {
		idx = findPhbookIdxByIndex(phbook_index[i]);
		if(idx == -1) // phbook index is not in the list
		{
			RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "PHBOOK ID");
		}
		//SYSTEM("uci delete phbook.@phbook[%d]", idx);
		api_delete_option2("", "phbook.@phbook[%d]", idx);
    }
	//SYSTEM("uci commit phbook");
	api_commit_option("phbook");

	reorderPhbookIndex();
	
    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

/* 
 *
 */
int json_post_bsids(ResponseEntry *rep, char *query_str)
{
    struct json_object *jobj = NULL;
    ResponseStatus *res = rep->res;
    /* POST all bases data to BSC */
    /* TODO */
    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

/* client request ALL bases info
 *
 */
int json_get_bsids(ResponseEntry *rep, struct json_object *jobj)
{
    struct json_object *jobj_bss = NULL;
    char idx_buf[4];
    ResponseStatus *res = rep->res;
    int i;
    /* Get all bases data from BSC */
    /* TODO */
    for(i = RAINIER_BSID_MIN; i <= RAINIER_BSID_MAX; i++)
    {
        sprintf(idx_buf, "%d", i);/* index RAINIER_BSID_MIN ~ RAINIER_BSID_MAX */
        jobj_bss = newObjectFromStack(rep);
        json_get_bsid_idx(rep, jobj_bss, i);
        json_object_object_add(jobj, idx_buf, jobj_bss);
    }

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

/* POST by index, get data from client
{
  "rainier_basic": {
    "system_id": 0,
    "auto_hangup_enable": true,
    "line_detection_enable": true,
    "mwi_enable": true,
    "base_int_auto_answer_enable": true,
    "dtmf_cid_format": 0,
    "base_access_pwd": "string",
    "base_id": 0,
    "line_in_dedicate": 0,
    "line_out_dedicate": 0,
    "dsp_line_in_gain": 0,
    "dsp_line_out_gain": 0,
    "flashtime": 100,
    "dtmf_duration": 80
  },
  "rainier_voip": {
    "server_ip": "198.51.100.42",
    "server_port": 0,
    "outbound_proxy_ip": "198.51.100.42",
    "outbound_proxy_port": 0,
    "sip_local_port": 0,
    "rtp_start_port": 0,
    "stun_enable": true,
    "stun_ip": "198.51.100.42",
    "stun_port": 0,
    "external_ip": "198.51.100.42",
    "user_agent_header": "string",
    "dns_srv_enable": true,
    "dns_srv_ip": "198.51.100.42",
    "session_timer_enable": true,
    "reg_expire_time": 0,
    "nat_keep_alive_interval": 0,
    "nat_keep_alive_method": 0,
    "mwi_subscribe_enable": true,
    "hash_terminate_enable": true,
    "qos_enable": true,
    "sdp_nat_rewrite_enable": true,
    "call_hold_type": 0,
    "dial_method": 0,
    "sip_transport": 0,
    "primary_codec": 0,
    "secondary_codec": 0,
	"tertiary_codec": 0,
    "rtp_ptime": 10
  },
  "rainier_daa": {
    "line_in_gain": 0,
    "line_out_gain": 0,
    "dtmf_out_gain": 0,
    "ac_impedance": 0,
    "dc_impedance": 0
  },
  "rainier_codec": {
    "microphone_gain": 0,
    "music_on_hold_gain": 0,
    "speaker_gain": 0
  }
}
*/
int json_post_bsid_idx(ResponseEntry *rep, char *query_str, int base_idx)
{
    struct json_object *jobj = NULL;
    ResponseStatus *res = rep->res;
    if((base_idx >= RAINIER_BSID_MIN) && (base_idx <= RAINIER_BSID_MAX))
    {
        /* get data from client and save for specific base */
    }
    else
    {
        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "BASE ID");
    }

    json_post_rainier_basic_base(rep, query_str);
    json_post_rainier_voip_basic(rep, query_str);
    json_post_rainier_daa(rep, query_str);
    json_post_rainier_codec(rep, query_str);
}

/* GET by index, send data to client
  "data": {
    "rainier_basic": {
      "system_id": 0,
      "auto_hangup_enable": true,
      "line_detection_enable": true,
      "mwi_enable": true,
      "base_int_auto_answer_enable": true,
      "dtmf_cid_format": 0,
      "base_access_pwd": "string",
      "base_id": 0,
      "line_in_dedicate": 0,
      "line_out_dedicate": 0,
      "dsp_line_in_gain": 0,
      "dsp_line_out_gain": 0,
      "flashtime": 100,
      "dtmf_duration": 80
    },
    "rainier_voip": {
      "server_ip": "198.51.100.42",
      "server_port": 0,
      "outbound_proxy_ip": "198.51.100.42",
      "outbound_proxy_port": 0,
      "sip_local_port": 0,
      "rtp_start_port": 0,
      "stun_enable": true,
      "stun_ip": "198.51.100.42",
      "stun_port": 0,
      "external_ip": "198.51.100.42",
      "user_agent_header": "string",
      "dns_srv_enable": true,
      "dns_srv_ip": "198.51.100.42",
      "session_timer_enable": true,
      "reg_expire_time": 0,
      "nat_keep_alive_interval": 0,
      "nat_keep_alive_method": 0,
      "mwi_subscribe_enable": true,
      "hash_terminate_enable": true,
      "qos_enable": true,
      "sdp_nat_rewrite_enable": true,
      "call_hold_type": 0,
      "dial_method": 0,
      "sip_transport": 0,
      "primary_codec": 0,
      "secondary_codec": 0,
	  "tertiary_codec": 0,
      "rtp_ptime": 10
    },
    "rainier_daa": {
      "line_in_gain": 0,
      "line_out_gain": 0,
      "dtmf_out_gain": 0,
      "ac_impedance": 0,
      "dc_impedance": 0
    },
    "rainier_codec": {
      "microphone_gain": 0,
      "music_on_hold_gain": 0,
      "speaker_gain": 0
    }
  }
*/
int json_get_bsid_idx(ResponseEntry *rep, struct json_object *jobj, int base_idx)
{
    //struct json_object *jobj = NULL;
    struct json_object *jobj_rainier_basic, *jobj_rainier_voip, *jobj_rainier_daa, *jobj_rainier_codec;
    ResponseStatus *res = rep->res;
    /* get base_idx */
    /* get data and save to json format */
    jobj_rainier_basic = newObjectFromStack(rep);
    json_get_rainier_basic_base(rep, jobj_rainier_basic);
    json_object_object_add(jobj, "rainier_basic", jobj_rainier_basic);

    jobj_rainier_voip = newObjectFromStack(rep);
    json_get_rainier_voip_basic(rep, jobj_rainier_voip);
    json_object_object_add(jobj, "rainier_voip", jobj_rainier_voip);

    jobj_rainier_daa = newObjectFromStack(rep);
    json_get_rainier_daa(rep, jobj_rainier_daa);
    json_object_object_add(jobj, "rainier_daa", jobj_rainier_daa);

    jobj_rainier_codec = newObjectFromStack(rep);
    json_get_rainier_codec(rep, jobj_rainier_codec);
    json_object_object_add(jobj, "rainier_codec", jobj_rainier_codec);
}

int json_post_rainier_basic_base(ResponseEntry *rep, char *query_str)
{
	int system_id = 0, dtmf_cid_format = 0, base_id = 0, line_in_dedicate = 0, line_out_dedicate = 0;
	int dsp_line_in_gain = 0, dsp_line_out_gain = 0, flashtime = 0, dtmf_duration = 0;
	bool auto_hangup_enable = 0, line_detection_enable = 0, mwi_enable = 0, base_int_auto_answer_enable = 0, handover_enable = 0;
	char *base_access_pwd = NULL;
	struct json_object *jobj = NULL;
	ResponseStatus *res = rep->res;
	
	if(NULL != query_str)
	{
		if((jobj = jsonTokenerParseFromStack(rep, query_str)))
		{
			senao_json_object_get_integer(jobj, "system_id", &system_id);
			senao_json_object_get_boolean(jobj, "auto_hangup_enable", &auto_hangup_enable);
			senao_json_object_get_boolean(jobj, "line_detection_enable", &line_detection_enable);
			senao_json_object_get_boolean(jobj, "mwi_enable", &mwi_enable);
			senao_json_object_get_boolean(jobj, "base_int_auto_answer_enable", &base_int_auto_answer_enable);
			senao_json_object_get_integer(jobj, "dtmf_cid_format", &dtmf_cid_format);
			senao_json_object_get_and_create_string(rep, jobj, "base_access_pwd", &base_access_pwd);
			senao_json_object_get_integer(jobj, "base_id", &base_id);
			senao_json_object_get_integer(jobj, "line_in_dedicate", &line_in_dedicate);
			senao_json_object_get_integer(jobj, "line_out_dedicate", &line_out_dedicate);
			senao_json_object_get_integer(jobj, "dsp_line_in_gain", &dsp_line_in_gain);
			senao_json_object_get_integer(jobj, "dsp_line_out_gain", &dsp_line_out_gain);
			senao_json_object_get_integer(jobj, "flashtime", &flashtime);
			senao_json_object_get_integer(jobj, "dtmf_duration", &dtmf_duration);            
			senao_json_object_get_boolean(jobj, "handover_enable", &handover_enable);
		}
		else
		{
			RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
		}
	}
	else
	{
		RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
	}

	int system_id_start = 1, system_id_end = 65535;
	if (!(system_id >= system_id_start && system_id <= system_id_end))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "SYSTEM ID");

	if (!(dtmf_cid_format == 0 || dtmf_cid_format == 1))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "DTMF CID FORMAT");

	int base_id_start = 101, base_id_end = 108;
	if (!(base_id >= base_id_start && base_id <= base_id_end))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "BASE ID");

	int line_in_dedicate_start = RAINIER_SIP_ACC_MIN, line_in_dedicate_end = RAINIER_SIP_ACC_MAX;
	if (!(line_in_dedicate >= line_in_dedicate_start && line_in_dedicate <= line_in_dedicate_end))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "LINE IN DEDICATE");

	int line_out_dedicate_start = RAINIER_SIP_ACC_MIN, line_out_dedicate_end = RAINIER_SIP_ACC_MAX;
	if (!(line_out_dedicate >= line_out_dedicate_start && line_out_dedicate <= line_out_dedicate_end))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "LINE OUT DEDICATE");

	int dsp_line_in_gain_start = -9, dsp_line_in_gain_end = 9;
	if (!(dsp_line_in_gain >= dsp_line_in_gain_start && dsp_line_in_gain <= dsp_line_in_gain_end))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "DSP LINE IN GAIN");

	int dsp_line_out_gain_start = -9, dsp_line_out_gain_end = 9;
	if (!(dsp_line_out_gain >= dsp_line_out_gain_start && dsp_line_out_gain <= dsp_line_out_gain_end))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "DSP LINE OUT GAIN");

	if (!(flashtime == 100 || flashtime == 200 || flashtime == 300 || flashtime == 400 || flashtime == 500 || flashtime == 600 || flashtime == 700 || flashtime == 800 || flashtime == 900 || flashtime == 1000))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "FLASHTIME");

	int dtmf_duration_start = 80, dtmf_duration_end = 250;
	if (!(dtmf_duration >= dtmf_duration_start && dtmf_duration <= dtmf_duration_end && (dtmf_duration % 10 == 0)))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "DTMF DURATION");
	
	api_set_integer_option("rainier.rainier_basic_base.system_id", system_id);
	api_set_rainier_auto_hangup_option("rainier.rainier_basic_base.auto_hangup_enable", auto_hangup_enable);
	api_set_rainier_line_detection_option("rainier.rainier_basic_base.line_detection_enable", line_detection_enable);
	api_set_rainier_mwi_option("rainier.rainier_basic_base.mwi_enable", mwi_enable);
	api_set_rainier_base_int_auto_answer_option("rainier.rainier_basic_base.base_int_auto_answer_enable", base_int_auto_answer_enable);
	api_set_integer_option("rainier.rainier_basic_base.dtmf_cid_format", dtmf_cid_format);
	api_set_rainier_handover_enable_option("rainier.rainier_basic_base.handover_enable", handover_enable);

	if(api_set_rainier_base_access_pwd("rainier.rainier_basic_base.base_access_pwd", base_access_pwd, sizeof(base_access_pwd)))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "BASE ACCESS PWD");

	api_set_integer_option("rainier.rainier_basic_base.base_id", base_id);
	api_set_integer_option("rainier.rainier_basic_base.line_in_dedicate", line_in_dedicate);
	api_set_integer_option("rainier.rainier_basic_base.line_out_dedicate", line_out_dedicate);
	api_set_integer_option("rainier.rainier_basic_base.dsp_line_in_gain", dsp_line_in_gain);
	api_set_integer_option("rainier.rainier_basic_base.dsp_line_out_gain", dsp_line_out_gain);
	api_set_integer_option("rainier.rainier_basic_base.flashtime", flashtime);
	api_set_integer_option("rainier.rainier_basic_base.dtmf_duration", dtmf_duration);

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

/*
{
  "system_id": 0,
  "auto_hangup_enable": true,
  "line_detection_enable": true,
  "mwi_enable": true,
  "base_int_auto_answer_enable": true,
  "dtmf_cid_format": 0,
  "base_access_pwd": "string",
  "base_id": 0,
  "line_in_dedicate": 0,
  "line_out_dedicate": 0,
  "dsp_line_in_gain": 0,
  "dsp_line_out_gain": 0,
  "flashtime": 100,
  "dtmf_duration": 80
}
 */
int json_get_rainier_basic_base(ResponseEntry *rep, struct json_object *jobj)
{
	int system_id = 0, dtmf_cid_format = 0, base_id = 0, line_in_dedicate = 0, line_out_dedicate = 0;
	int dsp_line_in_gain = 0, dsp_line_out_gain = 0, flashtime = 0, dtmf_duration = 0;
	char buf[4] = {0}, base_access_pwd[16] = {0};
	bool auto_hangup_enable = 0, line_detection_enable = 0, mwi_enable = 0, base_int_auto_answer_enable = 0, handover_enable = 0;
	ResponseStatus *res = rep->res;

	api_get_integer_option("rainier.rainier_basic_base.system_id", &system_id);
	api_get_integer_option("rainier.rainier_basic_base.dtmf_cid_format", &dtmf_cid_format);
	api_get_integer_option("rainier.rainier_basic_base.base_id", &base_id);
	api_get_integer_option("rainier.rainier_basic_base.line_in_dedicate", &line_in_dedicate);
	api_get_integer_option("rainier.rainier_basic_base.line_out_dedicate", &line_out_dedicate);
	api_get_integer_option("rainier.rainier_basic_base.dsp_line_in_gain", &dsp_line_in_gain);
	api_get_integer_option("rainier.rainier_basic_base.dsp_line_out_gain", &dsp_line_out_gain);

	if (api_get_integer_option("rainier.rainier_basic_base.flashtime", &flashtime) != API_RC_SUCCESS )
	{
		flashtime = 100;
	}
	if (api_get_integer_option("rainier.rainier_basic_base.dtmf_duration", &dtmf_duration) != API_RC_SUCCESS )
	{
		dtmf_duration = 80;
	}
	
	api_get_string_option("rainier.rainier_basic_base.base_access_pwd", base_access_pwd, sizeof(base_access_pwd));

	//sys_interact(buf, sizeof(buf), "uci get rainier.rainier_basic_base.auto_hangup_enable 2>/dev/null");
	api_get_string_option("rainier.rainier_basic_base.auto_hangup_enable", buf, sizeof(buf));
	auto_hangup_enable = (atoi(buf) == 1)?true:false;
	//sys_interact(buf, sizeof(buf), "uci get rainier.rainier_basic_base.line_detection_enable 2>/dev/null");
	api_get_string_option("rainier.rainier_basic_base.line_detection_enable", buf, sizeof(buf));
	line_detection_enable = (atoi(buf) == 1)?true:false;
	//sys_interact(buf, sizeof(buf), "uci get rainier.rainier_basic_base.mwi_enable 2>/dev/null");
	api_get_string_option("rainier.rainier_basic_base.mwi_enable", buf, sizeof(buf));
	mwi_enable = (atoi(buf) == 1)?true:false;
	//sys_interact(buf, sizeof(buf), "uci get rainier.rainier_basic_base.base_int_auto_answer_enable 2>/dev/null");
	api_get_string_option("rainier.rainier_basic_base.base_int_auto_answer_enable", buf, sizeof(buf));
	base_int_auto_answer_enable = (atoi(buf) == 1)?true:false;
	//sys_interact(buf, sizeof(buf), "uci get rainier.rainier_basic_base.handover_enable 2>/dev/null");
	api_get_string_option("rainier.rainier_basic_base.handover_enable", buf, sizeof(buf));
	handover_enable = (atoi(buf) == 1)?true:false;

	json_object_object_add(jobj, "system_id", json_object_new_int(system_id));
	json_object_object_add(jobj, "auto_hangup_enable", json_object_new_boolean(auto_hangup_enable));
	json_object_object_add(jobj, "line_detection_enable", json_object_new_boolean(line_detection_enable));
	json_object_object_add(jobj, "mwi_enable", json_object_new_boolean(mwi_enable));
	json_object_object_add(jobj, "base_int_auto_answer_enable", json_object_new_boolean(base_int_auto_answer_enable));
	json_object_object_add(jobj, "dtmf_cid_format", json_object_new_int(dtmf_cid_format));
	json_object_object_add(jobj, "base_access_pwd", json_object_new_string(base_access_pwd));
	json_object_object_add(jobj, "base_id", json_object_new_int(base_id));
	json_object_object_add(jobj, "line_in_dedicate", json_object_new_int(line_in_dedicate));
	json_object_object_add(jobj, "line_out_dedicate", json_object_new_int(line_out_dedicate));
	json_object_object_add(jobj, "dsp_line_in_gain", json_object_new_int(dsp_line_in_gain));
	json_object_object_add(jobj, "dsp_line_out_gain", json_object_new_int(dsp_line_out_gain));
	json_object_object_add(jobj, "flashtime", json_object_new_int(flashtime));
	json_object_object_add(jobj, "dtmf_duration", json_object_new_int(dtmf_duration));
	json_object_object_add(jobj, "handover_enable", json_object_new_boolean(handover_enable));

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_post_rainier_voip_basic(ResponseEntry *rep, char *query_str)
{
	char *server_ip = NULL, *outbound_proxy_ip = NULL, *stun_ip = NULL, *external_ip = NULL, *user_agent_header = NULL;//, *dns_srv_ip = NULL;
	int server_port = 0, outbound_proxy_port = 0, sip_local_port = 0, rtp_start_port = 0, stun_port = 0, reg_expire_time = 0;
	int nat_keep_alive_interval = 0, nat_keep_alive_method = 0, call_hold_type = 0, dial_method = 0, sip_transport = 0;
	int primary_codec = 0, secondary_codec = 0, tertiary_codec = 0, rtp_ptime = 0;
	bool stun_enable = 0, dns_srv_enable = 0, session_timer_enable = 0, mwi_subscribe_enable = 0, hash_terminate_enable = 0;
	bool qos_enable = 0, sdp_nat_rewrite_enable = 0;
	struct json_object *jobj = NULL;
	ResponseStatus *res = rep->res;

	if(NULL != query_str)
	{
		if((jobj = jsonTokenerParseFromStack(rep, query_str)))
		{
			senao_json_object_get_and_create_string(rep, jobj, "server_ip", &server_ip);
			senao_json_object_get_integer(jobj, "server_port", &server_port);
			senao_json_object_get_and_create_string(rep, jobj, "outbound_proxy_ip", &outbound_proxy_ip);
			senao_json_object_get_integer(jobj, "outbound_proxy_port", &outbound_proxy_port);
			senao_json_object_get_integer(jobj, "sip_local_port", &sip_local_port);
			senao_json_object_get_integer(jobj, "rtp_start_port", &rtp_start_port);
			senao_json_object_get_boolean(jobj, "stun_enable", &stun_enable);
			senao_json_object_get_and_create_string(rep, jobj, "stun_ip", &stun_ip);
			senao_json_object_get_integer(jobj, "stun_port", &stun_port);
			senao_json_object_get_and_create_string(rep, jobj, "external_ip", &external_ip);
			senao_json_object_get_and_create_string(rep, jobj, "user_agent_header", &user_agent_header);
			senao_json_object_get_boolean(jobj, "dns_srv_enable", &dns_srv_enable);
			//senao_json_object_get_and_create_string(rep, jobj, "dns_srv_ip", &dns_srv_ip);
			senao_json_object_get_boolean(jobj, "session_timer_enable", &session_timer_enable);
			senao_json_object_get_integer(jobj, "reg_expire_time", &reg_expire_time);
			senao_json_object_get_integer(jobj, "nat_keep_alive_interval", &nat_keep_alive_interval);
			senao_json_object_get_integer(jobj, "nat_keep_alive_method", &nat_keep_alive_method);
			senao_json_object_get_boolean(jobj, "mwi_subscribe_enable", &mwi_subscribe_enable);
			senao_json_object_get_boolean(jobj, "hash_terminate_enable", &hash_terminate_enable);
			senao_json_object_get_boolean(jobj, "qos_enable", &qos_enable);
			senao_json_object_get_boolean(jobj, "sdp_nat_rewrite_enable", &sdp_nat_rewrite_enable);	
			senao_json_object_get_integer(jobj, "call_hold_type", &call_hold_type);
			senao_json_object_get_integer(jobj, "dial_method", &dial_method);
			senao_json_object_get_integer(jobj, "sip_transport", &sip_transport);
			senao_json_object_get_integer(jobj, "primary_codec", &primary_codec);
			senao_json_object_get_integer(jobj, "secondary_codec", &secondary_codec);
			senao_json_object_get_integer(jobj, "tertiary_codec", &tertiary_codec);
			senao_json_object_get_integer(jobj, "rtp_ptime", &rtp_ptime);
		}
		else
		{
			RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
		}
	}
	else
	{
		RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
	}

	int reg_expire_time_start = 30, reg_expire_time_end = 65535;
	if (!(reg_expire_time >= reg_expire_time_start && reg_expire_time <= reg_expire_time_end))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "REG EXPIRE TIME");

	int nat_keep_alive_interval_start = 15, nat_keep_alive_interval_end = 200;
	if (!(nat_keep_alive_interval >= nat_keep_alive_interval_start && nat_keep_alive_interval <= nat_keep_alive_interval_end))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "NAT KEEP ALIVE INTERVAL");

	if (!(nat_keep_alive_method == 0 || nat_keep_alive_method == 1 || nat_keep_alive_method == 2))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "NAT KEEP ALIVE METHOD");

	if (!(call_hold_type == 0 || call_hold_type == 1))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "CALL HOLD TYPE");

	if (!(dial_method == 0 || dial_method == 1 || dial_method == 2 || dial_method == 3))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "DIAL METHOD");

	if (!(sip_transport == 0 || sip_transport == 1 || sip_transport == 2 || sip_transport == 3))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "SIP TRANSPORT");

	if (!(primary_codec == 0 || primary_codec == 8 || primary_codec == 18 || primary_codec == 255))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "PRIMARY CODEC");

	if (!(secondary_codec == 0 || secondary_codec == 8 || secondary_codec == 18 || secondary_codec == 255))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "SECONDARY CODEC");
		
	if (!(tertiary_codec == 0 || tertiary_codec == 8 || tertiary_codec == 18 || tertiary_codec == 255))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "TERTIARY CODEC");

	int rtp_ptime_start = 10, rtp_ptime_end = 150;
	if (!(rtp_ptime >= rtp_ptime_start && rtp_ptime <= rtp_ptime_end && (rtp_ptime % 10 == 0)))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "RTP PTIME");

	if(api_set_rainier_voip_basic_server_ip("rainier.voip_basic.server_ip", server_ip, sizeof(server_ip)))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "SERVER IP");

	if(api_set_rainier_voip_basic_server_port("rainier.voip_basic.server_port", server_port))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "SERVER PORT");

	if(api_set_rainier_voip_basic_outbound_proxy_ip("rainier.voip_basic.outbound_proxy_ip", outbound_proxy_ip, sizeof(outbound_proxy_ip)))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "OUTBOUND PROXY IP");

	if(api_set_rainier_voip_basic_outbound_proxy_port("rainier.voip_basic.outbound_proxy_port", outbound_proxy_port))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "OUTBOUND PROXY PORT");

	if(api_set_rainier_voip_basic_sip_local_port("rainier.voip_basic.sip_local_port", sip_local_port))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "SIP LOCAL PORT");

	if(api_set_rainier_voip_basic_rtp_start_port("rainier.voip_basic.rtp_start_port", rtp_start_port))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "RTP START PORT");

	api_set_rainier_voip_basic_stun_option("rainier.voip_basic.stun_enable", stun_enable);

	if(api_set_rainier_voip_basic_stun_ip("rainier.voip_basic.stun_ip", stun_ip, sizeof(stun_ip)))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "STUN IP");

	if(api_set_rainier_voip_basic_stun_port("rainier.voip_basic.stun_port", stun_port))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "STUN PORT");

	if(api_set_rainier_voip_basic_external_ip("rainier.voip_basic.external_ip", external_ip, sizeof(external_ip)))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "EXTERNAL IP");

	if(api_set_rainier_voip_basic_user_agent_header("rainier.voip_basic.user_agent_header", user_agent_header, sizeof(user_agent_header)))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "USER AGENT HEADER");

	api_set_rainier_voip_basic_dns_srv_option("rainier.voip_basic.dns_srv_enable", dns_srv_enable);

	//if(api_set_rainier_voip_basic_dns_srv_ip("rainier.voip_basic.dns_srv_ip", dns_srv_ip, sizeof(dns_srv_ip)))
	//	RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "DNS SERVER IP");

	api_set_rainier_voip_basic_session_timer_option("rainier.voip_basic.session_timer_enable", session_timer_enable);
	api_set_integer_option("rainier.voip_basic.reg_expire_time", reg_expire_time);
	api_set_integer_option("rainier.voip_basic.nat_keep_alive_interval", nat_keep_alive_interval);
	api_set_integer_option("rainier.voip_basic.nat_keep_alive_method", nat_keep_alive_method);
	api_set_rainier_voip_basic_mwi_subscribe_option("rainier.voip_basic.mwi_subscribe_enable", mwi_subscribe_enable);
	api_set_rainier_voip_basic_hash_terminate_option("rainier.voip_basic.hash_terminate_enable", hash_terminate_enable);
	api_set_rainier_voip_basic_qos_option("rainier.voip_basic.qos_enable", qos_enable);
	api_set_rainier_voip_basic_sdp_nat_rewrite_option("rainier.voip_basic.sdp_nat_rewrite_enable", sdp_nat_rewrite_enable);
	api_set_integer_option("rainier.voip_basic.call_hold_type", call_hold_type);
	api_set_integer_option("rainier.voip_basic.dial_method", dial_method);
	api_set_integer_option("rainier.voip_basic.sip_transport", sip_transport);
	api_set_integer_option("rainier.voip_basic.primary_codec", primary_codec);
	api_set_integer_option("rainier.voip_basic.secondary_codec", secondary_codec);
	api_set_integer_option("rainier.voip_basic.tertiary_codec", tertiary_codec);
	api_set_integer_option("rainier.voip_basic.rtp_ptime", rtp_ptime);

	api_commit_option("rainier");

	// NMS send new VoIP setting to BS
	SYSTEM("nmsconf_cli conf_mgm sendVoipSetting");

	// restart asterisk
	SYSTEM("astgen");
	SYSTEM("asterisk -rx 'core restart now'");

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_get_rainier_voip_basic(ResponseEntry *rep, struct json_object *jobj)
{
	char buf[4] = {0}, server_ip[64] = {0}, outbound_proxy_ip[64] = {0}, stun_ip[64] = {0}, external_ip[16] = {0};
	char user_agent_header[64] = {0};//, dns_srv_ip[16] = {0};
	int server_port = 0, outbound_proxy_port = 0, sip_local_port = 0, rtp_start_port = 0, stun_port = 0, reg_expire_time = 0;
	int nat_keep_alive_interval = 0, nat_keep_alive_method = 0, call_hold_type = 0, dial_method = 0, sip_transport = 0;
	int primary_codec = 0, secondary_codec = 0, tertiary_codec = 0, rtp_ptime = 0;
	bool stun_enable = 0, dns_srv_enable = 0, session_timer_enable = 0, mwi_subscribe_enable = 0, hash_terminate_enable = 0;
	bool qos_enable = 0, sdp_nat_rewrite_enable = 0;
	ResponseStatus *res = rep->res;

	api_get_string_option("rainier.voip_basic.server_ip", server_ip, sizeof(server_ip));
	api_get_string_option("rainier.voip_basic.outbound_proxy_ip", outbound_proxy_ip, sizeof(outbound_proxy_ip));
	api_get_string_option("rainier.voip_basic.stun_ip", stun_ip, sizeof(stun_ip));
	api_get_string_option("rainier.voip_basic.external_ip", external_ip, sizeof(external_ip));
	api_get_string_option("rainier.voip_basic.user_agent_header", user_agent_header, sizeof(user_agent_header));
	//api_get_string_option("rainier.voip_basic.dns_srv_ip", dns_srv_ip, sizeof(dns_srv_ip));

	api_get_integer_option("rainier.voip_basic.server_port", &server_port);
	api_get_integer_option("rainier.voip_basic.outbound_proxy_port", &outbound_proxy_port);
	api_get_integer_option("rainier.voip_basic.sip_local_port", &sip_local_port);
	api_get_integer_option("rainier.voip_basic.rtp_start_port", &rtp_start_port);
	api_get_integer_option("rainier.voip_basic.stun_port", &stun_port);
	api_get_integer_option("rainier.voip_basic.reg_expire_time", &reg_expire_time);
	api_get_integer_option("rainier.voip_basic.nat_keep_alive_interval", &nat_keep_alive_interval);
	api_get_integer_option("rainier.voip_basic.nat_keep_alive_method", &nat_keep_alive_method);
	api_get_integer_option("rainier.voip_basic.call_hold_type", &call_hold_type);
	api_get_integer_option("rainier.voip_basic.dial_method", &dial_method);
	api_get_integer_option("rainier.voip_basic.sip_transport", &sip_transport);
	api_get_integer_option("rainier.voip_basic.primary_codec", &primary_codec);
	api_get_integer_option("rainier.voip_basic.secondary_codec", &secondary_codec);
	api_get_integer_option("rainier.voip_basic.tertiary_codec", &tertiary_codec);
	api_get_integer_option("rainier.voip_basic.rtp_ptime", &rtp_ptime);

	//sys_interact(buf, sizeof(buf), "uci get rainier.voip_basic.stun_enable 2>/dev/null");
	api_get_string_option("rainier.voip_basic.stun_enable", buf, sizeof(buf));
	stun_enable = (atoi(buf) == 1)?true:false;
	//sys_interact(buf, sizeof(buf), "uci get rainier.voip_basic.dns_srv_enable 2>/dev/null");
	api_get_string_option("rainier.voip_basic.dns_srv_enable", buf, sizeof(buf));
	dns_srv_enable = (atoi(buf) == 1)?true:false;
	//sys_interact(buf, sizeof(buf), "uci get rainier.voip_basic.session_timer_enable 2>/dev/null");
	api_get_string_option("rainier.voip_basic.session_timer_enable", buf, sizeof(buf));
	session_timer_enable = (atoi(buf) == 1)?true:false;
	//sys_interact(buf, sizeof(buf), "uci get rainier.voip_basic.mwi_subscribe_enable 2>/dev/null");
	api_get_string_option("rainier.voip_basic.mwi_subscribe_enable", buf, sizeof(buf));
	mwi_subscribe_enable = (atoi(buf) == 1)?true:false;
	//sys_interact(buf, sizeof(buf), "uci get rainier.voip_basic.hash_terminate_enable 2>/dev/null");
	api_get_string_option("rainier.voip_basic.hash_terminate_enable", buf, sizeof(buf));
	hash_terminate_enable = (atoi(buf) == 1)?true:false;
	//sys_interact(buf, sizeof(buf), "uci get rainier.voip_basic.qos_enable 2>/dev/null");
	api_get_string_option("rainier.voip_basic.qos_enable", buf, sizeof(buf));
	qos_enable = (atoi(buf) == 1)?true:false;
	//sys_interact(buf, sizeof(buf), "uci get rainier.voip_basic.sdp_nat_rewrite_enable 2>/dev/null");
	api_get_string_option("rainier.voip_basic.sdp_nat_rewrite_enable", buf, sizeof(buf));
	sdp_nat_rewrite_enable = (atoi(buf) == 1)?true:false;

	json_object_object_add(jobj, "server_ip", json_object_new_string(server_ip));
	json_object_object_add(jobj, "server_port", json_object_new_int(server_port));
	json_object_object_add(jobj, "outbound_proxy_ip", json_object_new_string(outbound_proxy_ip));
	json_object_object_add(jobj, "outbound_proxy_port", json_object_new_int(outbound_proxy_port));
	json_object_object_add(jobj, "sip_local_port", json_object_new_int(sip_local_port));
	json_object_object_add(jobj, "rtp_start_port", json_object_new_int(rtp_start_port));
	json_object_object_add(jobj, "stun_enable", json_object_new_boolean(stun_enable));
	json_object_object_add(jobj, "stun_ip", json_object_new_string(stun_ip));
	json_object_object_add(jobj, "stun_port", json_object_new_int(stun_port));
	json_object_object_add(jobj, "external_ip", json_object_new_string(external_ip));
	json_object_object_add(jobj, "user_agent_header", json_object_new_string(user_agent_header));
	json_object_object_add(jobj, "dns_srv_enable", json_object_new_boolean(dns_srv_enable));
	//json_object_object_add(jobj, "dns_srv_ip", json_object_new_string(dns_srv_ip));
	json_object_object_add(jobj, "session_timer_enable", json_object_new_boolean(session_timer_enable));
	json_object_object_add(jobj, "reg_expire_time", json_object_new_int(reg_expire_time));
	json_object_object_add(jobj, "nat_keep_alive_interval", json_object_new_int(nat_keep_alive_interval));
	json_object_object_add(jobj, "nat_keep_alive_method", json_object_new_int(nat_keep_alive_method));
	json_object_object_add(jobj, "mwi_subscribe_enable", json_object_new_boolean(mwi_subscribe_enable));
	json_object_object_add(jobj, "hash_terminate_enable", json_object_new_boolean(hash_terminate_enable));
	json_object_object_add(jobj, "qos_enable", json_object_new_boolean(qos_enable));
	json_object_object_add(jobj, "sdp_nat_rewrite_enable", json_object_new_boolean(sdp_nat_rewrite_enable));
	json_object_object_add(jobj, "call_hold_type", json_object_new_int(call_hold_type));
	json_object_object_add(jobj, "dial_method", json_object_new_int(dial_method));
	json_object_object_add(jobj, "sip_transport", json_object_new_int(sip_transport));
	json_object_object_add(jobj, "primary_codec", json_object_new_int(primary_codec));
	json_object_object_add(jobj, "secondary_codec", json_object_new_int(secondary_codec));
	json_object_object_add(jobj, "tertiary_codec", json_object_new_int(tertiary_codec));
	json_object_object_add(jobj, "rtp_ptime", json_object_new_int(rtp_ptime));

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_post_rainier_sip_acc(ResponseEntry *rep, char *query_str, int sip_acc_idx)
{
	int index = 0, location = 0, rainier_group = 0;
	int in_dedicate = 0, out_dedicate = 0;
	bool enable = 0, inuse = 0, rainierReg = 0, sipReg = 0;
	char *sip_type = NULL, *account = NULL, *password = NULL, *auth_name = NULL, *display_name = NULL, *extension_digit = NULL;
	struct json_object *jobj = NULL;
	ResponseStatus *res = rep->res;

	if(NULL != query_str)
	{
		if((jobj = jsonTokenerParseFromStack(rep, query_str)))
		{
			senao_json_object_get_integer(jobj, "index", &index);
			senao_json_object_get_boolean(jobj, "enable", &enable);
			senao_json_object_get_boolean(jobj, "inuse", &inuse);
			senao_json_object_get_boolean(jobj, "rainierReg", &rainierReg);
			senao_json_object_get_boolean(jobj, "sipReg", &sipReg);
			senao_json_object_get_and_create_string(rep, jobj, "sip_type", &sip_type);
			senao_json_object_get_and_create_string(rep, jobj, "account", &account);
			senao_json_object_get_and_create_string(rep, jobj, "password", &password);
			senao_json_object_get_and_create_string(rep, jobj, "auth_name", &auth_name);
			senao_json_object_get_and_create_string(rep, jobj, "display_name", &display_name);
			senao_json_object_get_and_create_string(rep, jobj, "extension_digit", &extension_digit);
			senao_json_object_get_integer(jobj, "location", &location);
			senao_json_object_get_integer(jobj, "rainier_group", &rainier_group);
			senao_json_object_get_integer(jobj, "in_dedicate", &in_dedicate);
			senao_json_object_get_integer(jobj, "out_dedicate", &out_dedicate);
		}
		else
		{
			RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
		}
	}
	else
	{
		RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
	}

	int index_start = RAINIER_SIP_ACC_MIN, index_end = RAINIER_SIP_ACC_MAX;
	if (!(index >= index_start && index <= index_end))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "INDEX");

	api_set_integer_option("rainier.sip_acc.index", index);
	api_set_bool_option("rainier.sip_acc.enable", enable);
	api_set_bool_option("rainier.sip_acc.inuse", inuse);
	api_set_bool_option("rainier.sip_acc.rainierReg", rainierReg);
	api_set_bool_option("rainier.sip_acc.sipReg", sipReg);

	if(api_set_rainier_sip_acc_sip_type("rainier.sip_acc.sip_type", sip_type, sizeof(sip_type)))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "SIP TYPE");

	if(api_set_rainier_sip_acc_account("rainier.sip_acc.account", account, sizeof(account)))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "ACCOUNT");

	if(api_set_rainier_sip_acc_password("rainier.sip_acc.password", password, sizeof(password)))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "PASSWORD");

	if(api_set_rainier_sip_acc_auth_name("rainier.sip_acc.auth_name", auth_name, sizeof(auth_name)))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "AUTH NAME");

	if(api_set_rainier_sip_acc_display_name("rainier.sip_acc.display_name", display_name, sizeof(display_name)))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "DISPLAY NAME");

	if(api_set_rainier_sip_acc_extension_digit("rainier.sip_acc.extension_digit", extension_digit, sizeof(extension_digit)))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "EXTENSION DIGIT");

	api_set_integer_option("rainier.sip_acc.location", location);
	api_set_integer_option("rainier.sip_acc.rainier_group", rainier_group);

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

#if 0
int json_post_rainier_sip_acc_type_idx(ResponseEntry *rep, char *query_str, char *sip_acc_type, int sip_acc_idx)
{
	int index = 0, location = 0, rainier_group = 0;
	int in_dedicate = 0, out_dedicate = 0;
	bool enable = 0, inuse = 0, rainierReg = 0, sipReg = 0;
	char *sip_type = NULL, *account = NULL, *password = NULL, *auth_name = NULL, *display_name = NULL, *extension_digit = NULL;
	char inbound_route[8] = {0}, outbound_route[8] = {0};
	char buf[64] = {0};
	struct json_object *jobj = NULL;
	ResponseStatus *res = rep->res;

	if(NULL != query_str)
	{
		if((jobj = jsonTokenerParseFromStack(rep, query_str)))
		{
			senao_json_object_get_integer(jobj, "index", &index);
			senao_json_object_get_boolean(jobj, "enable", &enable);
			senao_json_object_get_boolean(jobj, "inuse", &inuse);
			senao_json_object_get_boolean(jobj, "rainierReg", &rainierReg);
			senao_json_object_get_boolean(jobj, "sipReg", &sipReg);
			senao_json_object_get_and_create_string(rep, jobj, "sip_type", &sip_type);
			senao_json_object_get_and_create_string(rep, jobj, "account", &account);
			senao_json_object_get_and_create_string(rep, jobj, "password", &password);
			senao_json_object_get_and_create_string(rep, jobj, "auth_name", &auth_name);
			senao_json_object_get_and_create_string(rep, jobj, "display_name", &display_name);
			senao_json_object_get_and_create_string(rep, jobj, "extension_digit", &extension_digit);
			senao_json_object_get_integer(jobj, "location", &location);
			senao_json_object_get_integer(jobj, "rainier_group", &rainier_group);
			senao_json_object_get_integer(jobj, "in_dedicate", &in_dedicate);
			senao_json_object_get_integer(jobj, "out_dedicate", &out_dedicate);
		}
		else
		{
			RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
		}
	}
	else
	{
		RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
	}

	int index_start = RAINIER_SIP_ACC_MIN, index_end = RAINIER_SIP_ACC_MAX;
	if (!(index >= index_start && index <= index_end))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "INDEX");

	if((in_dedicate >= RAINIER_HSID_MIN) && (in_dedicate <= RAINIER_HSID_MAX))
    {
        // inbound_route: "HS10" - "HS99"
		sprintf(inbound_route, "HS%d", in_dedicate);
    }
	else if((in_dedicate >= RAINIER_GROUP_MIN) && (in_dedicate <= RAINIER_GROUP_MAX))
    {
        // inbound_route: "GRP0" - "GRP7"
		sprintf(inbound_route, "GRP%d", in_dedicate);
    }
	else
	{
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "IN DEDICATE");
	}

	if((out_dedicate >= RAINIER_HSID_MIN) && (out_dedicate <= RAINIER_HSID_MAX))
    {
        // outbound_route: "HS10" - "HS99"
		sprintf(outbound_route, "HS%d", out_dedicate);
    }
	else if((out_dedicate >= RAINIER_GROUP_MIN) && (out_dedicate <= RAINIER_GROUP_MAX))
    {
        // outbound_route: "GRP0" - "GRP7"
		sprintf(outbound_route, "GRP%d", out_dedicate);
    }
	else
	{
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "OUT DEDICATE");
	}
	

	api_set_integer_option2(index, "rainier.%s_%d.index", sip_acc_type, sip_acc_idx);
	api_set_bool_option2(enable, "rainier.%s_%d.enable", sip_acc_type, sip_acc_idx);
	api_set_bool_option2(inuse, "rainier.%s_%d.inuse", sip_acc_type, sip_acc_idx);
	api_set_bool_option2(rainierReg, "rainier.%s_%d.rainierReg", sip_acc_type, sip_acc_idx);
	api_set_bool_option2(sipReg, "rainier.%s_%d.sipReg", sip_acc_type, sip_acc_idx);

	sprintf(buf, "rainier.%s_%d.sip_type", sip_acc_type, sip_acc_idx);
	if(api_set_rainier_sip_acc_sip_type(buf, sip_type, sizeof(sip_type)))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "SIP TYPE");

	sprintf(buf, "rainier.%s_%d.account", sip_acc_type, sip_acc_idx);
	if(api_set_rainier_sip_acc_account(buf, account, sizeof(account)))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "ACCOUNT");

	sprintf(buf, "rainier.%s_%d.password", sip_acc_type, sip_acc_idx);
	if(api_set_rainier_sip_acc_password(buf, password, sizeof(password)))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "PASSWORD");

	sprintf(buf, "rainier.%s_%d.auth_name", sip_acc_type, sip_acc_idx);
	if(api_set_rainier_sip_acc_auth_name(buf, auth_name, sizeof(auth_name)))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "AUTH NAME");

	sprintf(buf, "rainier.%s_%d.display_name", sip_acc_type, sip_acc_idx);
	if(api_set_rainier_sip_acc_display_name(buf, display_name, sizeof(display_name)))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "DISPLAY NAME");

	sprintf(buf, "rainier.%s_%d.extension_digit", sip_acc_type, sip_acc_idx);
	if(api_set_rainier_sip_acc_extension_digit(buf, extension_digit, sizeof(extension_digit)))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "EXTENSION DIGIT");

	api_set_integer_option2(location, "rainier.%s_%d.location", sip_acc_type, sip_acc_idx);
	api_set_integer_option2(rainier_group, "rainier.%s_%d.rainier_group", sip_acc_type, sip_acc_idx);

	if (api_set_string_option2(inbound_route, sizeof(inbound_route), "rainier.route_%s_%d.inbound", sip_acc_type, sip_acc_idx) != API_RC_SUCCESS)
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "IN DEDICATE");
	
	if (api_set_string_option2(outbound_route, sizeof(outbound_route), "rainier.route_%s_%d.outbound", sip_acc_type, sip_acc_idx) != API_RC_SUCCESS)
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "OUT DEDICATE");

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}
#endif

int json_get_rainier_sip_acc(ResponseEntry *rep, struct json_object *jobj, int sip_acc_idx)
{
	int index = 0, location = 0, rainier_group = 0;
	int in_dedicate = 0, out_dedicate = 0;
	bool enable = 0, inuse = 0, rainierReg = 0, sipReg = 0;
	char buf[4] = {0};
	char sip_type[16] = {0}, account[64] = {0}, password[64] = {0}, auth_name[64] = {0}, display_name[64] = {0}, extension_digit[6] = {0};
	ResponseStatus *res = rep->res;

	api_get_integer_option("rainier.sip_acc.index", &index);
	api_get_integer_option("rainier.sip_acc.location", &location);
	api_get_integer_option("rainier.sip_acc.rainier_group", &rainier_group);

	//sys_interact(buf, sizeof(buf), "uci get rainier.sip_acc.enable 2>/dev/null");
	api_get_string_option("rainier.sip_acc.enable", buf, sizeof(buf));
	enable = (atoi(buf) == 1)?true:false;
	//sys_interact(buf, sizeof(buf), "uci get rainier.sip_acc.inuse 2>/dev/null");
	api_get_string_option("rainier.sip_acc.inuse", buf, sizeof(buf));
	inuse = (atoi(buf) == 1)?true:false;
	//sys_interact(buf, sizeof(buf), "uci get rainier.sip_acc.rainierReg 2>/dev/null");
	api_get_string_option("rainier.sip_acc.rainierReg", buf, sizeof(buf));
	rainierReg = (atoi(buf) == 1)?true:false;
	//sys_interact(buf, sizeof(buf), "uci get rainier.sip_acc.sipReg 2>/dev/null");
	api_get_string_option("rainier.sip_acc.sipReg", buf, sizeof(buf));
	sipReg = (atoi(buf) == 1)?true:false;

	api_get_string_option("rainier.sip_acc.sip_type", sip_type, sizeof(sip_type));
	api_get_string_option("rainier.sip_acc.account", account, sizeof(account));
	api_get_string_option("rainier.sip_acc.password", password, sizeof(password));
	api_get_string_option("rainier.sip_acc.auth_name", auth_name, sizeof(auth_name));
	api_get_string_option("rainier.sip_acc.display_name", display_name, sizeof(display_name));
	api_get_string_option("rainier.sip_acc.extension_digit", extension_digit, sizeof(extension_digit));

	json_object_object_add(jobj, "index", json_object_new_int(index));
	json_object_object_add(jobj, "enable", json_object_new_boolean(enable));
	json_object_object_add(jobj, "inuse", json_object_new_boolean(inuse));
	json_object_object_add(jobj, "rainierReg", json_object_new_boolean(rainierReg));
	json_object_object_add(jobj, "sipReg", json_object_new_boolean(sipReg));
	json_object_object_add(jobj, "sip_type", json_object_new_string(sip_type));
	json_object_object_add(jobj, "account", json_object_new_string(account));
	json_object_object_add(jobj, "password", json_object_new_string(password));
	json_object_object_add(jobj, "auth_name", json_object_new_string(auth_name));
	json_object_object_add(jobj, "display_name", json_object_new_string(display_name));
	json_object_object_add(jobj, "extension_digit", json_object_new_string(extension_digit));
	json_object_object_add(jobj, "location", json_object_new_int(location));
	json_object_object_add(jobj, "rainier_group", json_object_new_int(rainier_group));
	json_object_object_add(jobj, "in_dedicate", json_object_new_int(in_dedicate));
	json_object_object_add(jobj, "out_dedicate", json_object_new_int(out_dedicate));

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

#if 0
int json_get_rainier_sip_acc_type_idx(ResponseEntry *rep, struct json_object *jobj, char *sip_acc_type, int sip_acc_idx)
{
    int index = 0, location = 0, rainier_group = 0;
	int in_dedicate = 0, out_dedicate = 0;
    bool enable = 0, inuse = 0, rainierReg = 0, sipReg = 0;
	int tmp = 0;
    char buf[4] = {0};
    char sip_type[16] = {0}, account[64] = {0}, password[64] = {0}, auth_name[64] = {0}, display_name[64] = {0}, extension_digit[6] = {0};
	char inbound_route[8] = {0}, outbound_route[8] = {0};
    ResponseStatus *res = rep->res;

    api_get_integer_option2(&index, "rainier.%s_%d.index", sip_acc_type, sip_acc_idx);
    api_get_integer_option2(&location, "rainier.%s_%d.location", sip_acc_type, sip_acc_idx);
    api_get_integer_option2(&rainier_group, "rainier.%s_%d.rainier_group", sip_acc_type, sip_acc_idx);

    api_get_bool_option2(&tmp, "rainier.%s_%d.enable", sip_acc_type, sip_acc_idx);
	enable = (tmp == 1)?true:false;
    api_get_bool_option2(&tmp, "rainier.%s_%d.inuse", sip_acc_type, sip_acc_idx);
	inuse = (tmp == 1)?true:false;
    api_get_bool_option2(&tmp, "rainier.%s_%d.rainierReg", sip_acc_type, sip_acc_idx);
	rainierReg = (tmp == 1)?true:false;
    api_get_bool_option2(&tmp, "rainier.%s_%d.sipReg", sip_acc_type, sip_acc_idx);
	sipReg = (tmp == 1)?true:false;

    api_get_string_option2(sip_type, sizeof(sip_type), "rainier.%s_%d.sip_type", sip_acc_type, sip_acc_idx);
    api_get_string_option2(account, sizeof(account), "rainier.%s_%d.account", sip_acc_type, sip_acc_idx);
    api_get_string_option2(password, sizeof(password), "rainier.%s_%d.password", sip_acc_type, sip_acc_idx);
    api_get_string_option2(auth_name, sizeof(auth_name), "rainier.%s_%d.auth_name", sip_acc_type, sip_acc_idx);
    api_get_string_option2(display_name, sizeof(display_name), "rainier.%s_%d.display_name", sip_acc_type, sip_acc_idx);
    api_get_string_option2(extension_digit, sizeof(extension_digit), "rainier.%s_%d.extension_digit", sip_acc_type, sip_acc_idx);

    api_get_string_option2(inbound_route, sizeof(inbound_route), "rainier.route_%s_%d.inbound", sip_acc_type, sip_acc_idx);
    if(strncmp(inbound_route, "HS", 2) == 0)
    {
        // in_dedicate: HS, 10 - 99
        in_dedicate = atoi(&inbound_route[2]);
    }
    else if(strncmp(inbound_route, "GRP", 3) == 0)
    {
        // in_dedicate: GRP, 0 - 7
        in_dedicate = atoi(&inbound_route[3]);
    }
    api_get_string_option2(outbound_route, sizeof(outbound_route), "rainier.route_%s_%d.outbound", sip_acc_type, sip_acc_idx);
    if(strncmp(outbound_route, "HS", 2) == 0)
    {
        // out_dedicate: HS, 10 - 99
        out_dedicate = atoi(&outbound_route[2]);
    }
    else if(strncmp(outbound_route, "GRP", 3) == 0)
    {
        // out_dedicate: GRP, 0 - 7
        out_dedicate = atoi(&outbound_route[3]);
    }

    json_object_object_add(jobj, "index", json_object_new_int(index));
    json_object_object_add(jobj, "enable", json_object_new_boolean(enable));
    json_object_object_add(jobj, "inuse", json_object_new_boolean(inuse));
    json_object_object_add(jobj, "rainierReg", json_object_new_boolean(rainierReg));
    json_object_object_add(jobj, "sipReg", json_object_new_boolean(sipReg));
    json_object_object_add(jobj, "sip_type", json_object_new_string(sip_type));
    json_object_object_add(jobj, "account", json_object_new_string(account));
    json_object_object_add(jobj, "password", json_object_new_string(password));
    json_object_object_add(jobj, "auth_name", json_object_new_string(auth_name));
    json_object_object_add(jobj, "display_name", json_object_new_string(display_name));
    json_object_object_add(jobj, "extension_digit", json_object_new_string(extension_digit));
    json_object_object_add(jobj, "location", json_object_new_int(location));
    json_object_object_add(jobj, "rainier_group", json_object_new_int(rainier_group));
	json_object_object_add(jobj, "in_dedicate", json_object_new_int(in_dedicate));
	json_object_object_add(jobj, "out_dedicate", json_object_new_int(out_dedicate));

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}
#endif

#if 0   // unused
int json_get_rainier_sip_fxo_acc(ResponseEntry *rep, struct json_object *jobj)
{
    struct json_object *jobj_fxo = NULL;
    char idx_buf[16];
    char account_type[] = "sip_fxo";
    ResponseStatus *res = rep->res;
    int i;
    /* Get all fxo sip account from BSC */
    for(i = RAINIER_FXO_MIN; i <= RAINIER_FXO_MAX; i++)
    {
        sprintf(idx_buf, "%s_%d", account_type, i);/* index RAINIER_FXO_MIN ~ RAINIER_FXO_MAX */
        jobj_fxo = newObjectFromStack(rep);
        json_get_rainier_sip_acc_type_idx(rep, jobj_fxo, account_type, i);
        json_object_object_add(jobj, idx_buf, jobj_fxo);
    }

	//RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_get_rainier_sip_group_acc(ResponseEntry *rep, struct json_object *jobj)
{
    struct json_object *jobj_group = NULL;
    char idx_buf[16];
    char account_type[] = "sip_group";
    ResponseStatus *res = rep->res;
    int i;
    /* Get all group sip account from BSC */
    /* TODO */
    for(i = RAINIER_GROUP_MIN; i <= RAINIER_GROUP_MAX; i++)
    {
        sprintf(idx_buf, "%s_%d", account_type, i);/* index RAINIER_GROUP_MIN ~ RAINIER_GROUP_MAX */
        jobj_group = newObjectFromStack(rep);
        json_get_rainier_sip_acc_type_idx(rep, jobj_group, account_type, i);
        json_object_object_add(jobj, idx_buf, jobj_group);
    }

	//RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}
#endif

int getSipTrunkCount()
{
	char cmd[256] = {0};
	char buf[256] = {0};

	//snprintf(cmd, sizeof(cmd), "grep -w \"sip_trunk\" -c /etc/config/sip_trunk");
	snprintf(cmd, sizeof(cmd), "uci show sip_trunk | grep -w -c \"=sip_trunk\"");
	sys_interact(buf, sizeof(buf), cmd);

	return atoi(buf);
}

int findSipTrunkIdxByIndex(int index)
{
	char cmd[256] = {0};
	char buf[256] = {0};
	int i;
	int total_nums;

	total_nums = getSipTrunkCount();

	for(i = 0; i < total_nums; i++)
	{
		//snprintf(cmd, sizeof(cmd), "uci get sip_trunk.@sip_trunk[%d].index | tr -d \"\\n\"", i);
		//sys_interact(buf, sizeof(buf), cmd);
		api_get_string_option2(buf, sizeof(buf), "sip_trunk.@sip_trunk[%d].index", i);

		if(atoi(buf) == index)
		{
			return i;
		}
	}

	return -1;
}

int findSipTrunkIdxByAccountName(char *account)
{
	char cmd[256] = {0};
	char buf[256] = {0};
	int i;
	int total_nums;

	total_nums = getSipTrunkCount();

	for(i = 0; i < total_nums; i++)
	{
		//snprintf(cmd, sizeof(cmd), "uci get sip_trunk.@sip_trunk[%d].account | tr -d \"\\n\"", i);
		//sys_interact(buf, sizeof(buf), cmd);
		api_get_string_option2(buf, sizeof(buf), "sip_trunk.@sip_trunk[%d].account", i);

		if(strcmp(buf, account) == 0)
		{
			return i;
		}
	}

	return -1;
}

int reorderSipTrunkIndex()
{
	int i;
	int total_nums;

	total_nums = getSipTrunkCount();

	for(i = 0; i < total_nums; i++)
	{
		//SYSTEM("uci set sip_trunk.@sip_trunk[%d].index=%d", i, i+1);
		api_set_integer_option2(i+1, "sip_trunk.@sip_trunk[%d].index", i);
	}

	//SYSTEM("uci commit sip_trunk");
	api_commit_option("sip_trunk");

	return 0;
}

int deleteAllSipTrunk()
{
	int i;
	int total_nums;

	total_nums = getSipTrunkCount();

	for(i = 0; i < total_nums; i++)
	{
		//SYSTEM("uci delete sip_trunk.@sip_trunk[0]");
		api_delete_option("sip_trunk.@sip_trunk[0]", "");
	}

	//SYSTEM("uci commit sip_trunk");
	api_commit_option("sip_trunk");

	return 0;
}

int checkSipTrunkSameAccountName(char *account[], int arraylen)
{
	char cmd[256] = {0};
	char buf[256] = {0};
	int i, j;
	int total_nums;

	total_nums = arraylen;

	//debug_print("total_nums: %d.\n", total_nums);

	for(i = 0; i < total_nums; i++)
	{
		for(j = i+1; j < total_nums; j++)
		{
			//debug_print("i=%d, j=%d\n", i, j);
			//debug_print("account[i]: %s\n", account[i]);
			//debug_print("account[j]: %s\n", account[j]);

			if(strcmp(account[i], account[j]) == 0)
			{
				return 1;
			}
		}
	}

	return 0;
}

int json_post_rainier_sip_trunk_acc(ResponseEntry *rep, char *query_str)
{
	int index[RAINIER_TRUNK_NUM] = {0};
	int in_dedicate[RAINIER_TRUNK_NUM] = {0}, out_dedicate[RAINIER_TRUNK_NUM] = {0};
	bool enable[RAINIER_TRUNK_NUM] = {0};//, sipReg = 0;
	char *account[RAINIER_TRUNK_NUM] = {NULL}, *password[RAINIER_TRUNK_NUM] = {NULL}, *auth_name[RAINIER_TRUNK_NUM] = {NULL}, *display_name[RAINIER_TRUNK_NUM] = {NULL};
	char inbound_route[RAINIER_TRUNK_NUM][8] = {0}, outbound_route[RAINIER_TRUNK_NUM][8] = {0};
	char buf[64] = {0};
	int total_nums;

	int arraylen = 0, i, idx;
	struct json_object *jobj = NULL, *jarr = NULL, *jarr_obj = NULL;
    ResponseStatus *res = rep->res;
    
	total_nums = getSipTrunkCount();

	/* Set trunk sip account */
    if(NULL != query_str)
    {
        if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
			jarr = json_object_object_get(jobj, "sip_acc");
            arraylen = json_object_array_length(jarr) ;
			
			if((arraylen > RAINIER_TRUNK_NUM) || (arraylen > total_nums))
			{
				RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "Too many SIP account");
			}
#if 0	// remove this check according web UI request
			else if(arraylen <= 0)
			{
				RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "No SIP account");
			}
#endif

            for (i = 0; i < arraylen; i++) 
            {
                jarr_obj = json_object_array_get_idx(jarr, i);
				senao_json_object_get_integer(jarr_obj, "index", &index[i]);
				senao_json_object_get_boolean(jarr_obj, "enable", &enable[i]);
				senao_json_object_get_and_create_string(rep, jarr_obj, "account", &account[i]);
				senao_json_object_get_and_create_string(rep, jarr_obj, "password", &password[i]);
				senao_json_object_get_and_create_string(rep, jarr_obj, "auth_name", &auth_name[i]);
				senao_json_object_get_and_create_string(rep, jarr_obj, "display_name", &display_name[i]);
				senao_json_object_get_integer(jarr_obj, "in_dedicate", &in_dedicate[i]);
				senao_json_object_get_integer(jarr_obj, "out_dedicate", &out_dedicate[i]);

				//debug_print("index: %d.\n", index[i]);
				//debug_print("enable: %d.\n", enable[i]);
				//debug_print("account: %s.\n", account[i]);
				//debug_print("password: %s.\n", password[i]);
				//debug_print("auth_name: %s.\n", auth_name[i]);
				//debug_print("display_name: %s.\n", display_name[i]);
				//debug_print("in_dedicate: %d.\n", in_dedicate[i]);
				//debug_print("out_dedicate: %d.\n", out_dedicate[i]);
            }
        }
        else
        {
            RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
        }
    }
    else
    {
        RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
    }

	if(checkSipTrunkSameAccountName(account, arraylen))
	{
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "Same SIP Number with other account");
	}

	// check input data
	for (i = 0; i < arraylen; i++)
	{
		if((index[i] < 1) || (index[i] > RAINIER_TRUNK_NUM) || (index[i] > arraylen))
		{
			RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "INDEX");
		}

		if((in_dedicate[i] >= RAINIER_HSID_MIN) && (in_dedicate[i] <= RAINIER_HSID_MAX))
	    {
	        // inbound_route: "HS10" - "HS99"
			sprintf(inbound_route[i], "HS%d", in_dedicate[i]);
	    }
		else if((in_dedicate[i] >= RAINIER_GROUP_MIN) && (in_dedicate[i] <= RAINIER_GROUP_MAX))
	    {
	        // inbound_route: "GRP0" - "GRP7"
			sprintf(inbound_route[i], "GRP%d", in_dedicate[i]);
	    }
		else
		{
			RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "IN DEDICATE");
		}

		if((out_dedicate[i] >= RAINIER_HSID_MIN) && (out_dedicate[i] <= RAINIER_HSID_MAX))
	    {
	        // outbound_route: "HS10" - "HS99"
			sprintf(outbound_route[i], "HS%d", out_dedicate[i]);
	    }
		else if((out_dedicate[i] >= RAINIER_GROUP_MIN) && (out_dedicate[i] <= RAINIER_GROUP_MAX))
	    {
	        // outbound_route: "GRP0" - "GRP7"
			sprintf(outbound_route[i], "GRP%d", out_dedicate[i]);
	    }
		else
		{
			RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "OUT DEDICATE");
		}

		if(api_check_rainier_sip_acc_account(NULL, account[i]))
			RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "ACCOUNT");

		if(api_check_rainier_sip_acc_password(NULL, password[i]))
			RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "PASSWORD");

		if(api_check_rainier_sip_acc_auth_name(NULL, auth_name[i]))
			RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "AUTH NAME");

		if(api_check_rainier_sip_acc_display_name(NULL, display_name[i]))
			RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "DISPLAY NAME");
	}

#if 0	// update new config
	for (i = 0; i < arraylen; i++)
    {
		//api_set_integer_option2(index[i], "sip_trunk.@sip_trunk[%d].index", i);
		api_set_bool_option2(enable[i], "sip_trunk.@sip_trunk[%d].enable", index[i]-1);

		sprintf(buf, "sip_trunk.@sip_trunk[%d].account", index[i]-1);
		api_set_rainier_sip_acc_account(buf, account[i], sizeof(account[i]));

		sprintf(buf, "sip_trunk.@sip_trunk[%d].password", index[i]-1);
		api_set_rainier_sip_acc_password(buf, password[i], sizeof(password[i]));

		sprintf(buf, "sip_trunk.@sip_trunk[%d].auth_name", index[i]-1);
		api_set_rainier_sip_acc_auth_name(buf, auth_name[i], sizeof(auth_name[i]));

		sprintf(buf, "sip_trunk.@sip_trunk[%d].display_name", index[i]-1);
		api_set_rainier_sip_acc_display_name(buf, display_name[i], sizeof(display_name[i]));

		api_set_string_option2(inbound_route[i], sizeof(inbound_route[i]), "sip_trunk.@sip_trunk[%d].in_dedicate", index[i]-1);
		api_set_string_option2(outbound_route[i], sizeof(outbound_route[i]), "sip_trunk.@sip_trunk[%d].out_dedicate", index[i]-1);
	}
	api_commit_option("sip_trunk");

#else	// delete all old config, and add all new config
	deleteAllSipTrunk();
    
	for (i = 0; i < arraylen; i++)
    {
#if 1
		api_add_new_section("sip_trunk", "sip_trunk");
		api_set_integer_option("sip_trunk.@sip_trunk[-1].index", i+1);
		api_set_bool_option("sip_trunk.@sip_trunk[-1].enable", enable[i]);
		api_set_rainier_sip_acc_account("sip_trunk.@sip_trunk[-1].account", account[i], sizeof(account[i]));
		api_set_rainier_sip_acc_password("sip_trunk.@sip_trunk[-1].password", password[i], sizeof(password[i]));
		api_set_rainier_sip_acc_auth_name("sip_trunk.@sip_trunk[-1].auth_name", auth_name[i], sizeof(auth_name[i]));
		api_set_rainier_sip_acc_display_name("sip_trunk.@sip_trunk[-1].display_name", display_name[i], sizeof(display_name[i]));
		api_set_string_option("sip_trunk.@sip_trunk[-1].in_dedicate", inbound_route[i], sizeof(inbound_route[i]));
		api_set_string_option("sip_trunk.@sip_trunk[-1].out_dedicate", outbound_route[i], sizeof(outbound_route[i]));
#else
		SYSTEM("uci add sip_trunk sip_trunk");
		SYSTEM("uci set sip_trunk.@sip_trunk[-1].index=%d", i+1);
		SYSTEM("uci set sip_trunk.@sip_trunk[-1].enable=%d", enable[i]);
		SYSTEM("uci set sip_trunk.@sip_trunk[-1].account='%s'", account[i]);
		SYSTEM("uci set sip_trunk.@sip_trunk[-1].password='%s'", password[i]);
		SYSTEM("uci set sip_trunk.@sip_trunk[-1].auth_name='%s'", auth_name[i]);
		SYSTEM("uci set sip_trunk.@sip_trunk[-1].display_name='%s'", display_name[i]);
		SYSTEM("uci set sip_trunk.@sip_trunk[-1].in_dedicate='%s'", inbound_route[i]);
		SYSTEM("uci set sip_trunk.@sip_trunk[-1].out_dedicate='%s'", outbound_route[i]);
#endif
    }
	//SYSTEM("uci commit sip_trunk");
	api_commit_option("sip_trunk");
#endif

	// reload asterisk
	SYSTEM("astgen");
	SYSTEM("asterisk -rx 'core reload'");

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_get_rainier_sip_trunk_acc(ResponseEntry *rep, struct json_object *jobj)
{
#if 1
    //char idx_buf[4];
	int trunk_status[RAINIER_TRUNK_NUM] = {0};
    ResponseStatus *res = rep->res;
    int i;
	int total_nums;
	
	struct json_object *jarr_sip_acc = NULL, *jobj_sip_acc = NULL;
	jarr_sip_acc = json_object_new_array();

	total_nums = getSipTrunkCount();

	/* get sip trunk status */
	ast_get_trunk_status(trunk_status);

    /* Get all trunk sip account */
	for(i = 1; i <= total_nums; i++)
    {
		jobj_sip_acc = json_object_new_object();
		json_get_rainier_sip_trunk_acc_idx_2(rep, jobj_sip_acc, i, trunk_status[i-1]);
		json_object_array_add(jarr_sip_acc, jobj_sip_acc);
	}

	json_object_object_add(jobj, "sip_acc", jarr_sip_acc);

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);

#else
    char account_type[] = "sip_trunk";
	ResponseStatus *res = rep->res;
    int i;

	struct json_object *jarr_sip_acc = NULL, *jobj_sip_acc = NULL;
	jarr_sip_acc = json_object_new_array();

    /* Get all trunk sip account */
	for(i = RAINIER_TRUNK_MIN; i <= RAINIER_TRUNK_MAX; i++)
    {
		jobj_sip_acc = json_object_new_object();
		json_get_rainier_sip_acc_type_idx(rep, jobj_sip_acc, account_type, i);
		json_object_array_add(jarr_sip_acc, jobj_sip_acc);
	}

	json_object_object_add(jobj, "sip_acc", jarr_sip_acc);

	//RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
#endif
}

int json_get_rainier_sip_trunk_acc_idx(ResponseEntry *rep, struct json_object *jobj, int sip_acc_idx)
{
	int result;
	int trunk_status[RAINIER_TRUNK_NUM] = {0};

    /* get sip trunk status */
	ast_get_trunk_status(trunk_status);

	result = json_get_rainier_sip_trunk_acc_idx_2(rep, jobj, sip_acc_idx, trunk_status[sip_acc_idx-1]);

	return result;
}

int json_get_rainier_sip_trunk_acc_idx_2(ResponseEntry *rep, struct json_object *jobj, int sip_acc_idx, int trunk_status_in)
{
#if 1
    int index = 0;
	int in_dedicate = 0, out_dedicate = 0;
    bool enable = 0, sipReg = 0;
	int tmp = 0;
    //char buf[4] = {0};
    char account[64] = {0}, password[64] = {0}, auth_name[64] = {0}, display_name[64] = {0};
	char inbound_route[8] = {0}, outbound_route[8] = {0};
	int total_nums;
    ResponseStatus *res = rep->res;

	total_nums = getSipTrunkCount();

	if((sip_acc_idx < 1) || (sip_acc_idx > RAINIER_TRUNK_NUM) || (sip_acc_idx > total_nums))
	{
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "SIP_TRUNK ID");
	}

    api_get_integer_option2(&index, "sip_trunk.@sip_trunk[%d].index", sip_acc_idx-1);

	api_get_bool_option2(&tmp, "sip_trunk.@sip_trunk[%d].enable", sip_acc_idx-1);
	enable = (tmp == 1)?true:false;

	sipReg = trunk_status_in;

    api_get_string_option2(account, sizeof(account), "sip_trunk.@sip_trunk[%d].account", sip_acc_idx-1);
    api_get_string_option2(password, sizeof(password), "sip_trunk.@sip_trunk[%d].password", sip_acc_idx-1);
    api_get_string_option2(auth_name, sizeof(auth_name), "sip_trunk.@sip_trunk[%d].auth_name", sip_acc_idx-1);
    api_get_string_option2(display_name, sizeof(display_name), "sip_trunk.@sip_trunk[%d].display_name", sip_acc_idx-1);

    api_get_string_option2(inbound_route, sizeof(inbound_route), "sip_trunk.@sip_trunk[%d].in_dedicate", sip_acc_idx-1);
    if(strncmp(inbound_route, "HS", 2) == 0)
    {
        // in_dedicate: HS, 10 - 99
        in_dedicate = atoi(&inbound_route[2]);
    }
    else if(strncmp(inbound_route, "GRP", 3) == 0)
    {
        // in_dedicate: GRP, 0 - 7
        in_dedicate = atoi(&inbound_route[3]);
    }
    api_get_string_option2(outbound_route, sizeof(outbound_route), "sip_trunk.@sip_trunk[%d].out_dedicate", sip_acc_idx-1);
    if(strncmp(outbound_route, "HS", 2) == 0)
    {
        // out_dedicate: HS, 10 - 99
        out_dedicate = atoi(&outbound_route[2]);
    }
    else if(strncmp(outbound_route, "GRP", 3) == 0)
    {
        // out_dedicate: GRP, 0 - 7
        out_dedicate = atoi(&outbound_route[3]);
    }

    /* put data */
	json_object_object_add(jobj, "index", json_object_new_int(index));
    json_object_object_add(jobj, "enable", json_object_new_boolean(enable));
    json_object_object_add(jobj, "sipReg", json_object_new_boolean(sipReg));
    json_object_object_add(jobj, "account", json_object_new_string(account));
    json_object_object_add(jobj, "password", json_object_new_string(password));
    json_object_object_add(jobj, "auth_name", json_object_new_string(auth_name));
    json_object_object_add(jobj, "display_name", json_object_new_string(display_name));
	json_object_object_add(jobj, "in_dedicate", json_object_new_int(in_dedicate));
	json_object_object_add(jobj, "out_dedicate", json_object_new_int(out_dedicate));

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);

#else
    char account_type[] = "sip_trunk";
	ResponseStatus *res = rep->res;

	/* check sip_acc_idx valid or not */
    if((sip_acc_idx < RAINIER_TRUNK_MIN) || (sip_acc_idx > RAINIER_TRUNK_MAX))
    {
        /* sip_acc_idx is not in range */
        RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "INDEX");
    }

    /* Get trunk sip account */
	json_get_rainier_sip_acc_type_idx(rep, jobj, account_type, sip_acc_idx);

	//RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
#endif
}

int json_post_rainier_sip_trunk_acc_idx(ResponseEntry *rep, char *query_str, int sip_acc_idx)
{
#if 1
	int index = 0;
	int in_dedicate = 0, out_dedicate = 0;
	bool enable = 0;//, sipReg = 0;
	char *account = NULL, *password = NULL, *auth_name = NULL, *display_name = NULL;
	char inbound_route[8] = {0}, outbound_route[8] = {0};
	char buf[64] = {0};
	int total_nums;
	int idx;
    struct json_object *jobj = NULL;
    ResponseStatus *res = rep->res;

	total_nums = getSipTrunkCount();

	if((sip_acc_idx < 1) || (sip_acc_idx > RAINIER_TRUNK_NUM) || (sip_acc_idx > total_nums))
	{
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "SIP_TRUNK ID");
	}
	
    /* Set trunk sip account */
    if(NULL != query_str)
    {
        if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
			//senao_json_object_get_integer(jobj, "index", &index);
			senao_json_object_get_boolean(jobj, "enable", &enable);
			senao_json_object_get_and_create_string(rep, jobj, "account", &account);
			senao_json_object_get_and_create_string(rep, jobj, "password", &password);
			senao_json_object_get_and_create_string(rep, jobj, "auth_name", &auth_name);
			senao_json_object_get_and_create_string(rep, jobj, "display_name", &display_name);
			senao_json_object_get_integer(jobj, "in_dedicate", &in_dedicate);
			senao_json_object_get_integer(jobj, "out_dedicate", &out_dedicate);
        }
        else
        {
            RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
        }
    }
    else
    {
        RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
    }

	idx = findSipTrunkIdxByAccountName(account);
	if((idx != -1) && (idx != sip_acc_idx-1))	// find an index, and the index is not self
	{
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "Same SIP Number with other account");
	}

	if((in_dedicate >= RAINIER_HSID_MIN) && (in_dedicate <= RAINIER_HSID_MAX))
    {
        // inbound_route: "HS10" - "HS99"
		sprintf(inbound_route, "HS%d", in_dedicate);
    }
	else if((in_dedicate >= RAINIER_GROUP_MIN) && (in_dedicate <= RAINIER_GROUP_MAX))
    {
        // inbound_route: "GRP0" - "GRP7"
		sprintf(inbound_route, "GRP%d", in_dedicate);
    }
	else
	{
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "IN DEDICATE");
	}

	if((out_dedicate >= RAINIER_HSID_MIN) && (out_dedicate <= RAINIER_HSID_MAX))
    {
        // outbound_route: "HS10" - "HS99"
		sprintf(outbound_route, "HS%d", out_dedicate);
    }
	else if((out_dedicate >= RAINIER_GROUP_MIN) && (out_dedicate <= RAINIER_GROUP_MAX))
    {
        // outbound_route: "GRP0" - "GRP7"
		sprintf(outbound_route, "GRP%d", out_dedicate);
    }
	else
	{
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "OUT DEDICATE");
	}
    
	//api_set_integer_option2(index, "sip_trunk.@sip_trunk[%d].index", sip_acc_idx-1);

	api_set_bool_option2(enable, "sip_trunk.@sip_trunk[%d].enable", sip_acc_idx-1);

	sprintf(buf, "sip_trunk.@sip_trunk[%d].account", sip_acc_idx-1);
	if(api_set_rainier_sip_acc_account(buf, account, sizeof(account)))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "ACCOUNT");

	sprintf(buf, "sip_trunk.@sip_trunk[%d].password", sip_acc_idx-1);
	if(api_set_rainier_sip_acc_password(buf, password, sizeof(password)))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "PASSWORD");

	sprintf(buf, "sip_trunk.@sip_trunk[%d].auth_name", sip_acc_idx-1);
	if(api_set_rainier_sip_acc_auth_name(buf, auth_name, sizeof(auth_name)))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "AUTH NAME");

	sprintf(buf, "sip_trunk.@sip_trunk[%d].display_name", sip_acc_idx-1);
	if(api_set_rainier_sip_acc_display_name(buf, display_name, sizeof(display_name)))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "DISPLAY NAME");

	if (api_set_string_option2(inbound_route, sizeof(inbound_route), "sip_trunk.@sip_trunk[%d].in_dedicate", sip_acc_idx-1) != API_RC_SUCCESS)
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "IN DEDICATE");
	
	if (api_set_string_option2(outbound_route, sizeof(outbound_route), "sip_trunk.@sip_trunk[%d].out_dedicate", sip_acc_idx-1) != API_RC_SUCCESS)
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "OUT DEDICATE");

	api_commit_option("sip_trunk");
	//SYSTEM("uci commit sip_trunk");

	// reload asterisk
	SYSTEM("astgen");
	SYSTEM("asterisk -rx 'core reload'");

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);

#else
    char account_type[] = "sip_trunk";
	ResponseStatus *res = rep->res;

    /* check sip_acc_idx valid or not */
    if((sip_acc_idx < RAINIER_TRUNK_MIN) || (sip_acc_idx > RAINIER_TRUNK_MAX))
    {
        /* sip_acc_idx is not in range */
        RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "INDEX");
    }

	/* Set trunk sip account */
	json_post_rainier_sip_acc_type_idx(rep, query_str, account_type, sip_acc_idx);

	//RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
#endif
}

int json_post_add_rainier_sip_trunk_acc(ResponseEntry *rep, char *query_str)
{
    int index = 0;
	int in_dedicate = 0, out_dedicate = 0;
	bool enable = 0;//, sipReg = 0;
	char *account = NULL, *password = NULL, *auth_name = NULL, *display_name = NULL;
	char inbound_route[8] = {0}, outbound_route[8] = {0};
	//char buf[64] = {0};
	int total_nums;
    struct json_object *jobj = NULL;
    ResponseStatus *res = rep->res;

	total_nums = getSipTrunkCount();

	if(total_nums >= RAINIER_TRUNK_NUM)
	{
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "The maximum number of SIP accounts is 32");
	}

    /* POST sip_trunk data to BSC */
    if(NULL != query_str)
    {
        if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
			//senao_json_object_get_integer(jobj, "index", &index);
			senao_json_object_get_boolean(jobj, "enable", &enable);
			//senao_json_object_get_boolean(jobj, "sipReg", &sipReg);
			senao_json_object_get_and_create_string(rep, jobj, "account", &account);
			senao_json_object_get_and_create_string(rep, jobj, "password", &password);
			senao_json_object_get_and_create_string(rep, jobj, "auth_name", &auth_name);
			senao_json_object_get_and_create_string(rep, jobj, "display_name", &display_name);
			senao_json_object_get_integer(jobj, "in_dedicate", &in_dedicate);
			senao_json_object_get_integer(jobj, "out_dedicate", &out_dedicate);
        }
        else
        {
            RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
        }
    }
    else
    {
        RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
    }

	if(findSipTrunkIdxByAccountName(account) != -1)
	{
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "Same SIP Number with other account");
	}

	if((in_dedicate >= RAINIER_HSID_MIN) && (in_dedicate <= RAINIER_HSID_MAX))
    {
        // inbound_route: "HS10" - "HS99"
		sprintf(inbound_route, "HS%d", in_dedicate);
    }
	else if((in_dedicate >= RAINIER_GROUP_MIN) && (in_dedicate <= RAINIER_GROUP_MAX))
    {
        // inbound_route: "GRP0" - "GRP7"
		sprintf(inbound_route, "GRP%d", in_dedicate);
    }
	else
	{
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "IN DEDICATE");
	}

	if((out_dedicate >= RAINIER_HSID_MIN) && (out_dedicate <= RAINIER_HSID_MAX))
    {
        // outbound_route: "HS10" - "HS99"
		sprintf(outbound_route, "HS%d", out_dedicate);
    }
	else if((out_dedicate >= RAINIER_GROUP_MIN) && (out_dedicate <= RAINIER_GROUP_MAX))
    {
        // outbound_route: "GRP0" - "GRP7"
		sprintf(outbound_route, "GRP%d", out_dedicate);
    }
	else
	{
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "OUT DEDICATE");
	}

	if(api_check_rainier_sip_acc_account(NULL, account))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "ACCOUNT");

	if(api_check_rainier_sip_acc_password(NULL, password))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "PASSWORD");

	if(api_check_rainier_sip_acc_auth_name(NULL, auth_name))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "AUTH NAME");

	if(api_check_rainier_sip_acc_display_name(NULL, display_name))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "DISPLAY NAME");

    
	index = total_nums + 1;

#if 1
	api_add_new_section("sip_trunk", "sip_trunk");
	api_set_integer_option("sip_trunk.@sip_trunk[-1].index", index);
	api_set_bool_option("sip_trunk.@sip_trunk[-1].enable", enable);
	api_set_rainier_sip_acc_account("sip_trunk.@sip_trunk[-1].account", account, sizeof(account));
	api_set_rainier_sip_acc_password("sip_trunk.@sip_trunk[-1].password", password, sizeof(password));
	api_set_rainier_sip_acc_auth_name("sip_trunk.@sip_trunk[-1].auth_name", auth_name, sizeof(auth_name));
	api_set_rainier_sip_acc_display_name("sip_trunk.@sip_trunk[-1].display_name", display_name, sizeof(display_name));
	api_set_string_option("sip_trunk.@sip_trunk[-1].in_dedicate", inbound_route, sizeof(inbound_route));
	api_set_string_option("sip_trunk.@sip_trunk[-1].out_dedicate", outbound_route, sizeof(outbound_route));
	api_commit_option("sip_trunk");
#else
	SYSTEM("uci add sip_trunk sip_trunk");
	SYSTEM("uci set sip_trunk.@sip_trunk[-1].index=%d", index);
	SYSTEM("uci set sip_trunk.@sip_trunk[-1].enable=%d", enable);
	SYSTEM("uci set sip_trunk.@sip_trunk[-1].account='%s'", account);
	SYSTEM("uci set sip_trunk.@sip_trunk[-1].password='%s'", password);
	SYSTEM("uci set sip_trunk.@sip_trunk[-1].auth_name='%s'", auth_name);
	SYSTEM("uci set sip_trunk.@sip_trunk[-1].display_name='%s'", display_name);
	SYSTEM("uci set sip_trunk.@sip_trunk[-1].in_dedicate='%s'", inbound_route);
	SYSTEM("uci set sip_trunk.@sip_trunk[-1].out_dedicate='%s'", outbound_route);
	SYSTEM("uci commit sip_trunk");
#endif

	// reload asterisk
	SYSTEM("astgen");
	SYSTEM("asterisk -rx 'core reload'");

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_post_delete_rainier_sip_trunk_acc(ResponseEntry *rep, char *query_str)
{
	int index[RAINIER_TRUNK_NUM] = {0};
	int arraylen = 0, i, idx;
	struct json_object *jobj = NULL, *jarr = NULL, *jarr_obj = NULL;
    ResponseStatus *res = rep->res;
    /* POST sip_trunk data to BSC */
    if(NULL != query_str)
    {
        if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
			jarr = json_object_object_get(jobj, "sip_acc");
            arraylen = json_object_array_length(jarr) ;
			
			if(arraylen > RAINIER_TRUNK_NUM)
			{
				RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "Too many SIP account index");
			}

            for (i = 0; i < arraylen; i++) 
            {
                jarr_obj = json_object_array_get_idx(jarr, i);
				senao_json_object_get_integer(jarr_obj, "index", &index[i]);

				//debug_print("delete sip_trunk index: %d.\n", index[i]);
            }
        }
        else
        {
            RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
        }
    }
    else
    {
        RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
    }
    
	for (i = 0; i < arraylen; i++)
    {
		idx = findSipTrunkIdxByIndex(index[i]);
		if(idx == -1) // sip_trunk index is not in the list
		{
			RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "SIP_TRUNK ID");
		}
		//SYSTEM("uci delete sip_trunk.@sip_trunk[%d]", idx);
		api_delete_option2("", "sip_trunk.@sip_trunk[%d]", idx);
    }
	//SYSTEM("uci commit sip_trunk");
	api_commit_option("sip_trunk");

	reorderSipTrunkIndex();

	// reload asterisk
	SYSTEM("astgen");
	SYSTEM("asterisk -rx 'core reload'");
	
    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_post_rainier_daa(ResponseEntry *rep, char *query_str)
{
	int i = 0, line_in_gain = 0, line_out_gain = 0, dtmf_out_gain = 0, ac_impedance = 0, dc_impedance = 0;
	bool ac_impedance_flag = 0;
	struct json_object *jobj = NULL;
	ResponseStatus *res = rep->res;

	if(NULL != query_str)
	{
		if((jobj = jsonTokenerParseFromStack(rep, query_str)))
		{
			senao_json_object_get_integer(jobj, "line_in_gain", &line_in_gain);
			senao_json_object_get_integer(jobj, "line_out_gain", &line_out_gain);
			senao_json_object_get_integer(jobj, "dtmf_out_gain", &dtmf_out_gain);
			senao_json_object_get_integer(jobj, "ac_impedance", &ac_impedance);
			senao_json_object_get_integer(jobj, "dc_impedance", &dc_impedance);
		}
		else
		{
			RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
		}
	}
	else
	{
		RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
	}

	int line_in_gain_start = -15, line_in_gain_end = 12;
	if (!(line_in_gain >= line_in_gain_start && line_in_gain <= line_in_gain_end))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "LINE IN GAIN");

	int line_out_gain_start = -15, line_out_gain_end = 12;
	if (!(line_out_gain >= line_out_gain_start && line_out_gain <= line_out_gain_end))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "LINE OUT GAIN");

	int dtmf_out_gain_start = -15, dtmf_out_gain_end = 12;
	if (!(dtmf_out_gain >= dtmf_out_gain_start && dtmf_out_gain_start <= line_in_gain_end))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "DTMF OUT GAIN");

	if (!(dc_impedance == 0 || dc_impedance == 1 || dc_impedance == 2 || dc_impedance == 3))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "DC IMPEDANCE");

	for(i = 0; i <=15; i++)
	{
		if (ac_impedance == i)	
			break;
		else if (i == 15)
			RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "AC IMPEDANCE");
	}

	api_set_integer_option("rainier.daa.line_in_gain", line_in_gain);
	api_set_integer_option("rainier.daa.line_out_gain", line_out_gain);
	api_set_integer_option("rainier.daa.dtmf_out_gain", dtmf_out_gain);
	api_set_integer_option("rainier.daa.ac_impedance", ac_impedance);
	api_set_integer_option("rainier.daa.dc_impedance", dc_impedance);

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_get_rainier_daa(ResponseEntry *rep, struct json_object *jobj)
{
	int line_in_gain = 0, line_out_gain = 0, dtmf_out_gain = 0, ac_impedance = 0, dc_impedance = 0;
	ResponseStatus *res = rep->res;

	api_get_integer_option("rainier.daa.line_in_gain", &line_in_gain);
	api_get_integer_option("rainier.daa.line_out_gain", &line_out_gain);
	api_get_integer_option("rainier.daa.dtmf_out_gain", &dtmf_out_gain);
	api_get_integer_option("rainier.daa.ac_impedance", &ac_impedance);
	api_get_integer_option("rainier.daa.dc_impedance", &dc_impedance);

	json_object_object_add(jobj, "line_in_gain", json_object_new_int(line_in_gain));
	json_object_object_add(jobj, "line_out_gain", json_object_new_int(line_out_gain));
	json_object_object_add(jobj, "dtmf_out_gain", json_object_new_int(dtmf_out_gain));
	json_object_object_add(jobj, "ac_impedance", json_object_new_int(ac_impedance));
	json_object_object_add(jobj, "dc_impedance", json_object_new_int(dc_impedance));

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_post_rainier_fxo_setting(ResponseEntry *rep, char *query_str)
{
	int i = 0, line_in_gain = 0, line_out_gain = 0, dtmf_out_gain = 0, ac_impedance = 0, dc_impedance = 0;
	int dsp_line_in_gain = 0, dsp_line_out_gain = 0, flashtime = 0, dtmf_duration = 0;
	bool ac_impedance_flag = 0;
	struct json_object *jobj = NULL;
	ResponseStatus *res = rep->res;

	if(NULL != query_str)
	{
		if((jobj = jsonTokenerParseFromStack(rep, query_str)))
		{
			senao_json_object_get_integer(jobj, "line_in_gain", &line_in_gain);
			senao_json_object_get_integer(jobj, "line_out_gain", &line_out_gain);
			senao_json_object_get_integer(jobj, "dc_impedance", &dc_impedance);
			senao_json_object_get_integer(jobj, "ac_impedance", &ac_impedance);
			senao_json_object_get_integer(jobj, "flashtime", &flashtime);
			senao_json_object_get_integer(jobj, "dtmf_out_gain", &dtmf_out_gain);
			senao_json_object_get_integer(jobj, "dtmf_duration", &dtmf_duration);
			senao_json_object_get_integer(jobj, "dsp_line_in_gain", &dsp_line_in_gain);
			senao_json_object_get_integer(jobj, "dsp_line_out_gain", &dsp_line_out_gain);
		}
		else
		{
			RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
		}
	}
	else
	{
		RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
	}

	int line_in_gain_start = -15, line_in_gain_end = 12;
	if (!(line_in_gain >= line_in_gain_start && line_in_gain <= line_in_gain_end))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "LINE IN GAIN");

	int line_out_gain_start = -15, line_out_gain_end = 12;
	if (!(line_out_gain >= line_out_gain_start && line_out_gain <= line_out_gain_end))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "LINE OUT GAIN");

	int dtmf_out_gain_start = -15, dtmf_out_gain_end = 12;
	if (!(dtmf_out_gain >= dtmf_out_gain_start && dtmf_out_gain_start <= line_in_gain_end))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "DTMF OUT GAIN");

	if (!(dc_impedance == 0 || dc_impedance == 1 || dc_impedance == 2 || dc_impedance == 3))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "DC IMPEDANCE");

	for(i = 0; i <=15; i++)
	{
		if (ac_impedance == i)	
			break;
		else if (i == 15)
			RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "AC IMPEDANCE");
	}

	int dsp_line_in_gain_start = -9, dsp_line_in_gain_end = 9;
	if (!(dsp_line_in_gain >= dsp_line_in_gain_start && dsp_line_in_gain <= dsp_line_in_gain_end))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "DSP LINE IN GAIN");

	int dsp_line_out_gain_start = -9, dsp_line_out_gain_end = 9;
	if (!(dsp_line_out_gain >= dsp_line_out_gain_start && dsp_line_out_gain <= dsp_line_out_gain_end))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "DSP LINE OUT GAIN");

	if (!(flashtime == 100 || flashtime == 200 || flashtime == 300 || flashtime == 400 || flashtime == 500 || flashtime == 600 || flashtime == 700 || flashtime == 800 || flashtime == 900 || flashtime == 1000))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "FLASHTIME");

	int dtmf_duration_start = 80, dtmf_duration_end = 250;
	if (!(dtmf_duration >= dtmf_duration_start && dtmf_duration <= dtmf_duration_end && (dtmf_duration % 10 == 0)))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "DTMF DURATION");

	api_set_integer_option("rainier.daa.line_in_gain", line_in_gain);
	api_set_integer_option("rainier.daa.line_out_gain", line_out_gain);
	api_set_integer_option("rainier.daa.dtmf_out_gain", dtmf_out_gain);
	api_set_integer_option("rainier.daa.ac_impedance", ac_impedance);
	api_set_integer_option("rainier.daa.dc_impedance", dc_impedance);

	api_set_integer_option("rainier.rainier_basic_base.dsp_line_in_gain", dsp_line_in_gain);
	api_set_integer_option("rainier.rainier_basic_base.dsp_line_out_gain", dsp_line_out_gain);
	api_set_integer_option("rainier.rainier_basic_base.flashtime", flashtime);
	api_set_integer_option("rainier.rainier_basic_base.dtmf_duration", dtmf_duration);

	api_commit_option("rainier");

	// NMS send new FXO setting to BS
	SYSTEM("nmsconf_cli conf_mgm sendFxoSetting");

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_get_rainier_fxo_setting(ResponseEntry *rep, struct json_object *jobj)
{
	int line_in_gain = 0, line_out_gain = 0, dtmf_out_gain = 0, ac_impedance = 0, dc_impedance = 0;
	int dsp_line_in_gain = 0, dsp_line_out_gain = 0, flashtime = 0, dtmf_duration = 0;
	ResponseStatus *res = rep->res;

	api_get_integer_option("rainier.daa.line_in_gain", &line_in_gain);
	api_get_integer_option("rainier.daa.line_out_gain", &line_out_gain);
	api_get_integer_option("rainier.daa.dtmf_out_gain", &dtmf_out_gain);
	api_get_integer_option("rainier.daa.ac_impedance", &ac_impedance);
	api_get_integer_option("rainier.daa.dc_impedance", &dc_impedance);

	api_get_integer_option("rainier.rainier_basic_base.dsp_line_in_gain", &dsp_line_in_gain);
	api_get_integer_option("rainier.rainier_basic_base.dsp_line_out_gain", &dsp_line_out_gain);

	if (api_get_integer_option("rainier.rainier_basic_base.flashtime", &flashtime) != API_RC_SUCCESS )
	{
		flashtime = 100;
	}
	if (api_get_integer_option("rainier.rainier_basic_base.dtmf_duration", &dtmf_duration) != API_RC_SUCCESS )
	{
		dtmf_duration = 80;
	}

	json_object_object_add(jobj, "line_in_gain", json_object_new_int(line_in_gain));
	json_object_object_add(jobj, "line_out_gain", json_object_new_int(line_out_gain));
	json_object_object_add(jobj, "dc_impedance", json_object_new_int(dc_impedance));
	json_object_object_add(jobj, "ac_impedance", json_object_new_int(ac_impedance));
	json_object_object_add(jobj, "flashtime", json_object_new_int(flashtime));
	json_object_object_add(jobj, "dtmf_out_gain", json_object_new_int(dtmf_out_gain));
	json_object_object_add(jobj, "dtmf_duration", json_object_new_int(dtmf_duration));
	json_object_object_add(jobj, "dsp_line_in_gain", json_object_new_int(dsp_line_in_gain));
	json_object_object_add(jobj, "dsp_line_out_gain", json_object_new_int(dsp_line_out_gain));

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_post_rainier_codec(ResponseEntry *rep, char *query_str)
{
	int microphone_gain = 0, music_on_hold_gain = 0, speaker_gain = 0;
	struct json_object *jobj = NULL;
	ResponseStatus *res = rep->res;

	if(NULL != query_str)
	{
		if((jobj = jsonTokenerParseFromStack(rep, query_str)))
		{
			senao_json_object_get_integer(jobj, "microphone_gain", &microphone_gain);
			senao_json_object_get_integer(jobj, "music_on_hold_gain", &music_on_hold_gain);
			senao_json_object_get_integer(jobj, "speaker_gain", &speaker_gain);
		}
		else
		{
			RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
		}
	}
	else
	{
		RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
	}

	api_set_integer_option("rainier.codec.microphone_gain", microphone_gain);
	api_set_integer_option("rainier.codec.music_on_hold_gain", music_on_hold_gain);
	api_set_integer_option("rainier.codec.speaker_gain", speaker_gain);

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_get_rainier_codec(ResponseEntry *rep, struct json_object *jobj)
{
	int microphone_gain = 0, music_on_hold_gain = 0, speaker_gain = 0;
	ResponseStatus *res = rep->res;

	api_get_integer_option("rainier.codec.microphone_gain", &microphone_gain);
	api_get_integer_option("rainier.codec.music_on_hold_gain", &music_on_hold_gain);
	api_get_integer_option("rainier.codec.speaker_gain", &speaker_gain);

	json_object_object_add(jobj, "microphone_gain", json_object_new_int(microphone_gain));
	json_object_object_add(jobj, "music_on_hold_gain", json_object_new_int(music_on_hold_gain));
	json_object_object_add(jobj, "speaker_gain", json_object_new_int(speaker_gain));

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int getBscId(char *id, int len)
{
	char cmd[256] = {0};

	snprintf(cmd, sizeof(cmd), "setconfig -g 6 | awk '{$1=$1;print}' | awk -F\":\" '{print $1$2$3$4$5$6}' | tr -d \"\\n\""); // WAN MAC address, prevent trailing writespace
	sys_interact_long(id, len, cmd);
	debug_print("curr_bsc_id: %s.\n", id);
}

int getBscName(char *name, int len)
{
	char cmd[256] = {0};

	//snprintf(cmd, sizeof(cmd), "uci get system.@system[0].hostname | tr -d \"\\n\"");
	//sys_interact_long(name, len, cmd);
	api_get_string_option("system.@system[0].hostname", name, len);
	debug_print("curr_bsc_name: %s.\n", name);
}

int getBsCount()
{
	char cmd[256] = {0};
	char buf[256] = {0};

	//snprintf(cmd, sizeof(cmd), "wc -l < /etc/config/base-station-list");
	//snprintf(cmd, sizeof(cmd), "grep -w \"config base-station\" -c /etc/config/base-station-list");
	snprintf(cmd, sizeof(cmd), "uci show base-station-list | grep -w -c \"=base-station\"");
	sys_interact(buf, sizeof(buf), cmd);

	return atoi(buf);
}

int findBsConfigIdx(char *id)
{
	char cmd[256] = {0};
	char buf[256] = {0};
	int i;
	int total_nums;

	total_nums = getBsCount();

	for(i = 0; i < total_nums; i++)
	{
		//snprintf(cmd, sizeof(cmd), "uci get base-station-list.@base-station[%d].id | tr -d \"\\n\"", i);
		//sys_interact(buf, sizeof(buf), cmd);
		api_get_string_option2(buf, sizeof(buf), "base-station-list.@base-station[%d].id", i);

		if(strcmp(buf, id) == 0)
		{
			return i;
		}
	}

	return -1;
}

int findBsConfigIdxByIndex(int index)
{
	char cmd[256] = {0};
	char buf[256] = {0};
	int i;
	int total_nums;

	total_nums = getBsCount();

	for(i = 0; i < total_nums; i++)
	{
		//snprintf(cmd, sizeof(cmd), "uci get base-station-list.@base-station[%d].index | tr -d \"\\n\"", i);
		//sys_interact(buf, sizeof(buf), cmd);
		api_get_string_option2(buf, sizeof(buf), "base-station-list.@base-station[%d].index", i);

		if(atoi(buf) == index)
		{
			return i;
		}
	}

	return -1;
}

static char *strupr(char *str)
{
    char *orign=str;

    for (; *str!='\0'; str++)
        *str = toupper(*str);

    return orign;
}

int findBsConfigIndexByMac(char *mac)
{
	char cmd[256] = {0};
	char buf[256] = {0};
	int i;
	int total_nums;

	if(mac == NULL)
	{
		return -1;
	}

	total_nums = getBsCount();

	for(i = 0; i < total_nums; i++)
	{
		//snprintf(cmd, sizeof(cmd), "uci get base-station-list.@base-station[%d].mac | tr -d \"\\n\"", i);
		//sys_interact(buf, sizeof(buf), cmd);
		api_get_string_option2(buf, sizeof(buf), "base-station-list.@base-station[%d].mac", i);

		if(strcmp(strupr(buf), strupr(mac)) == 0)
		{
			//snprintf(cmd, sizeof(cmd), "uci get base-station-list.@base-station[%d].index | tr -d \"\\n\"", i);
			//sys_interact(buf, sizeof(buf), cmd);
			api_get_string_option2(buf, sizeof(buf), "base-station-list.@base-station[%d].index", i);

			return atoi(buf);
		}
	}

	return -1;
}

int resetBsListStatus()
{
	char cmd[256] = {0};
	char buf[256] = {0};
	int i;
	int total_nums;

	total_nums = getBsCount();

	for(i = 0; i < total_nums; i++)
	{
		//snprintf(cmd, sizeof(cmd), "uci set base-station-list.@base-station[%d].status='%s'", i, "offline");
		//sys_interact(buf, sizeof(buf), cmd);
		api_set_string_option2("offline", sizeof("offline"), "base-station-list.@base-station[%d].status", i);
	}

	//snprintf(cmd, sizeof(cmd), "uci commit base-station-list");
	//sys_interact(buf, sizeof(buf), cmd);
	api_commit_option("base-station-list");

	return 0;
}

void removeBsFromList(int idx)
{
	char cmd[256] = {0};
	char buf[256] = {0};

	int bs_index;

	//snprintf(cmd, sizeof(cmd), "uci get base-station-list.@base-station[%d].index | tr -d \"\\n\"", idx);
	//sys_interact(buf, sizeof(buf), cmd);
	api_get_string_option2(buf, sizeof(buf), "base-station-list.@base-station[%d].index", idx);

	bs_index = atoi(buf);
	uciClearBsIdxStatus(bs_index - 1);	/* shift to 0~7 */

	//snprintf(cmd, sizeof(cmd), "uci delete base-station-list.@base-station[%d]", idx);
	//sys_interact(buf, sizeof(buf), cmd);
	api_delete_option2("", "base-station-list.@base-station[%d]", idx);
}

int get_avahi_info(char *avahi_str, char *find, char *result, size_t result_size)
{
	char *pt1, *pt2, *pt3;

	//debug_print("avahi_str[%s]\n", avahi_str);
	//debug_print("find[%s]\n", find);

	if ( (pt1 = strstr(avahi_str, find)) ) 
	{
		//debug_print("pt1[%s]\n", pt1);

		if ( (pt2 = strstr(pt1, "=")) ) 
		{
			pt2++;

			//debug_print("pt2[%s]\n", pt2);

			if ( (pt3 = strstr(pt2, "\"")) ) 
			{
				strncpy(result, pt2, result_size);

				if((pt3 - pt2) < result_size)
				{
					*(result+(pt3-pt2)) = '\0';
				}
				else
				{
					*(result+(result_size-1)) = '\0';
				}

				//debug_print("result[%s]\n", result);

				return 0;
			}
		}
	}
	return -1;
}

int json_get_rainier_base_station_list(ResponseEntry *rep, struct json_object *jobj)
{
	char buf[1024] = {0};

	ResponseStatus *res = rep->res;

	FILE *fptr;
	char cmd[1024] = {0};
	char *path = "/etc/avahi_device_list";
	char line[1024] = {0};

	int file_EOF = 0;

	char other[1024] = {0};
	char bs_id[32] = {0};
	char bs_name[32] = {0};
	char bs_ip[32] = {0};
	char bs_mac[32] = {0};
	char bsc_id[32] = {0};
	char bsc_name[32] = {0};
	char bs_status[32] = {0};
	int bs_index = 0;;

	/* Richard add 2020-0220 */
	int sync_status = 0;
	char serial_number[32] = {0};
	char subnet_mask[32] = {0};
	char gateway[32] = {0};	
	/* */
	char curr_ip[32] = {0};
	char curr_mac[32] = {0};
	char description[64] = {0};
	char fw_version[32] = {0};
	char fw_version_tmp[32] = {0};
	char *curr_bsc_id;
	int idx;
	char model_name[32] = {0};

	struct json_object *jarr_unreg_BS_table, *jobj_unreg_BS_table;
	jarr_unreg_BS_table = json_object_new_array();

	struct json_object *jarr_curr_BS_table, *jobj_curr_BS_table;
	jarr_curr_BS_table = json_object_new_array();

	int count, i;
	int ch; // the return value from getc() is an int
	int len;

	int total_nums;

	api_fetch_rainier_base_station_list(buf, sizeof(buf));
	resetBsListStatus();

	if ((fptr = fopen(path, "r")) == NULL)
	{
		snprintf(cmd, sizeof(cmd), "echo failed > /etc/avahi_fail");
		sys_interact_long(buf, sizeof(buf), cmd);
		//return -1;
		RET_GEN_ERRORMSG(res, API_INTERNAL_ERROR, "Unable to open avahi device list");
	}

	sys_interact(curr_ip, sizeof(curr_ip), "ifconfig br-lan | grep Bcast | awk {'printf $2'} | awk -F':' '{printf $2}'");
	sys_interact(curr_mac, sizeof(curr_mac), "ifconfig br-lan | grep HWaddr | awk {'printf $5'}");

	for(i = 0; i < sizeof(curr_mac); i++)
		curr_mac[i] = toupper(curr_mac[i]);

	curr_bsc_id = (char*)malloc(13*sizeof(char));
	getBscId(curr_bsc_id, 13);

	// Unregistered Base Station List
	//while((read = getline(&line, &len, fptr)) != -1) why
	while(!feof(fptr))
	{
		/*memset(line, 0, sizeof(line));
		memset(bs_ip, 0, sizeof(bs_ip));
		memset(bs_id, 0, sizeof(bs_id));
		memset(bs_name, 0, sizeof(bs_name));
		memset(bs_mac, 0, sizeof(bs_mac));
		memset(bsc_id, 0, sizeof(bsc_id));
		memset(bsc_name, 0, sizeof(bsc_name));
		memset(other, 0, sizeof(other));*/

		count = 0;

		while(1)
		{
			ch = getc(fptr);
			if(ch == EOF)
			{
				file_EOF = 1;
				break;
			}
			line[count] = ch;

			if(line[count] == '\n')
				break;

			count++;
		}

		if(file_EOF)
			break;
		//fscanf(fptr, "%[^\n]\n", line); why
		sscanf(line, "%s%[^\n]\n", bs_ip, other);

#if 1
		get_avahi_info(other, "device_id", bs_id, sizeof(bs_id));
#if 1
		// default set empty bs_name
		strcpy(bs_name, "");
#else
		get_avahi_info(other, "device_name", bs_name, sizeof(bs_name));
		// Rename default name to DuraFon Roam BU
		if(strcmp(bs_name, "SP938BS") == 0)
		{
			strcpy(bs_name, "DuraFon Roam BU");
		}
#endif
		get_avahi_info(other, "mac", bs_mac, sizeof(bs_mac));
		get_avahi_info(other, "bsc_id", bsc_id, sizeof(bsc_id));
		get_avahi_info(other, "bsc_name", bsc_name, sizeof(bsc_name));
#if 0 /* use post bsc notify */		
		get_avahi_info(other, "sync_status", sync_status, sizeof(sync_status));
#endif
		get_avahi_info(other, "serial_number", serial_number, sizeof(serial_number));
		get_avahi_info(other, "version", fw_version, sizeof(fw_version));
		// add version prefix with letter 'v'
		if((fw_version[0] >= '0') && (fw_version[0] <= '9'))
		{
			strncpy(fw_version_tmp, fw_version, sizeof(fw_version_tmp));
			snprintf(fw_version, sizeof(fw_version), "v%s", fw_version_tmp);
		}
		get_avahi_info(other, "subnet_mask", subnet_mask, sizeof(subnet_mask));
		get_avahi_info(other, "gateway", gateway, sizeof(gateway));
		get_avahi_info(other, "modelname", model_name, sizeof(model_name));
		// Rename default model_name to DuraFon Roam
		if(strcmp(model_name, "SP938BS") == 0)
		{
			strcpy(model_name, "DuraFon Roam");
		}

#else
		snprintf(cmd, sizeof(cmd), "echo %s | awk '{for (i=1;i<=NF;i++){if ($i ~/device_id/) {print $i}}}' | sed -e 's/\"//g' | awk -F'=' '{print $2}' 2>/dev/null | tr -d \"\\n\"", other);
		sys_interact_long(bs_id, sizeof(bs_id), cmd);

		snprintf(cmd, sizeof(cmd), "echo %s | awk '{for (i=1;i<=NF;i++){if ($i ~/device_name/) {print $i}}}' | sed -e 's/\"//g' | awk -F'=' '{print $2}' 2>/dev/null | tr -d \"\\n\"", other);
		sys_interact_long(bs_name, sizeof(bs_name), cmd);

		// Rename default name to DuraFon Roam BU
		if(strcmp(bs_name, "SP938BS") == 0)
		{
			strcpy(bs_name, "DuraFon Roam BU");
		}

		snprintf(cmd, sizeof(cmd), "echo %s | awk '{for (i=1;i<=NF;i++){if ($i ~/mac/) {print $i}}}' | sed -e 's/\"//g' | awk -F'=' '{if (length($1)==3 ){print $2}}' 2>/dev/null | tr -d \"\\n\"", other);
		sys_interact_long(bs_mac, sizeof(bs_mac), cmd);

		snprintf(cmd, sizeof(cmd), "echo %s | awk '{for (i=1;i<=NF;i++){if ($i ~/bsc_id/) {print $i}}}' | sed -e 's/\"//g' | awk -F'=' '{print $2}' 2>/dev/null | tr -d \"\\n\"", other);
		sys_interact_long(bsc_id, sizeof(bsc_id), cmd);

		snprintf(cmd, sizeof(cmd), "echo %s | awk '{for (i=1;i<=NF;i++){if ($i ~/bsc_name/) {print $i}}}' | sed -e 's/\"//g' | awk -F'=' '{print $2}' 2>/dev/null | tr -d \"\\n\"", other);
		sys_interact_long(bsc_name, sizeof(bsc_name), cmd);

		/* */
#if 0 /* use post bsc notify */		
		snprintf(cmd, sizeof(cmd), "echo %s | awk '{for (i=1;i<=NF;i++){if ($i ~/sync_status/) {print $i}}}' | sed -e 's/\"//g' | awk -F'=' '{print $2}' 2>/dev/null | tr -d \"\\n\"", other);
		sys_interact_long(sync_status, sizeof(sync_status), cmd);
#endif
		snprintf(cmd, sizeof(cmd), "echo %s | awk '{for (i=1;i<=NF;i++){if ($i ~/serial_number/) {print $i}}}' | sed -e 's/\"//g' | awk -F'=' '{print $2}' 2>/dev/null | tr -d \"\\n\"", other);
		sys_interact_long(serial_number, sizeof(serial_number), cmd);

		snprintf(cmd, sizeof(cmd), "echo %s | awk '{for (i=1;i<=NF;i++){if ($i ~/version/) {print $i}}}' | sed -e 's/\"//g' | awk -F'=' '{print $2}' 2>/dev/null | tr -d \"\\n\"", other);
		sys_interact_long(fw_version, sizeof(fw_version), cmd);

		snprintf(cmd, sizeof(cmd), "echo %s | awk '{for (i=1;i<=NF;i++){if ($i ~/subnet_mask/) {print $i}}}' | sed -e 's/\"//g' | awk -F'=' '{print $2}' 2>/dev/null | tr -d \"\\n\"", other);
		sys_interact_long(subnet_mask, sizeof(subnet_mask), cmd);

		snprintf(cmd, sizeof(cmd), "echo %s | awk '{for (i=1;i<=NF;i++){if ($i ~/gateway/) {print $i}}}' | sed -e 's/\"//g' | awk -F'=' '{print $2}' 2>/dev/null | tr -d \"\\n\"", other);
		sys_interact_long(gateway, sizeof(gateway), cmd);
		/* */
		snprintf(cmd, sizeof(cmd), "echo %s | awk '{for (i=1;i<=NF;i++){if ($i ~/modelname/) {print $i}}}' | sed -e 's/\"//g' | awk -F'=' '{print $2}' 2>/dev/null | tr -d \"\\n\"", other);
		sys_interact_long(model_name, sizeof(model_name), cmd);
#endif

		for(i = 0; i < sizeof(bs_mac); i++)
			bs_mac[i] = toupper(bs_mac[i]);

		//debug_print("Jason DEBUG %s[%d] curr_mac: %s, curr_ip: %s.\n", __FUNCTION__, __LINE__, curr_mac, curr_ip);
		//debug_print("Jason DEBUG %s[%d] client_mac: %s, client_ip: %s.\n", __FUNCTION__, __LINE__, bs_mac, bs_ip);

		idx = findBsConfigIdx(bs_id);
		//debug_print("list idx: %d, bs_index: %s.\n", idx, bs_index);
		//  && (strcmp(curr_bsc_id, bsc_id) != 0)
		//if(strcmp(curr_mac, bs_mac) != 0 && strcmp(curr_ip, bs_ip) != 0)
		if(idx == -1) // new device
		{
			debug_print("unreg idx: %d, bs_id: %s.\n", idx, bs_id);
			debug_print("unreg idx: %d, bs_name: %s.\n", idx, bs_name);
			debug_print("unreg idx: %d, bs_ip: %s.\n", idx, bs_ip);
			debug_print("unreg idx: %d, bs_mac: %s.\n", idx, bs_mac);
			debug_print("unreg idx: %d, bsc_id: %s.\n", idx, bsc_id);
			debug_print("unreg idx: %d, bsc_name: %s.\n", idx, bsc_name);
			debug_print("unreg idx: %d, sync_status: %d.\n", idx, sync_status);
			debug_print("unreg idx: %d, model_name: %s.\n", idx, model_name);
			debug_print("unreg idx: %d, serial_number: %s.\n", idx, serial_number);
			debug_print("unreg idx: %d, fw_version: %s.\n", idx, fw_version);
			debug_print("unreg idx: %d, subnet_mask: %s.\n", idx, subnet_mask);
			debug_print("unreg idx: %d, gateway: %s.\n", idx, gateway);

			jobj_unreg_BS_table = json_object_new_object();
			json_object_object_add(jobj_unreg_BS_table, "bs_id", json_object_new_string(bs_id));
			json_object_object_add(jobj_unreg_BS_table, "bs_name", json_object_new_string(bs_name));
			json_object_object_add(jobj_unreg_BS_table, "bs_ip", json_object_new_string(bs_ip));
			json_object_object_add(jobj_unreg_BS_table, "bs_mac", json_object_new_string(bs_mac));
			json_object_object_add(jobj_unreg_BS_table, "bsc_id", json_object_new_string(bsc_id));
			json_object_object_add(jobj_unreg_BS_table, "bsc_name", json_object_new_string(bsc_name));
			json_object_object_add(jobj_unreg_BS_table, "description", json_object_new_string("---"));
			json_object_object_add(jobj_unreg_BS_table, "sync_status", json_object_new_int(0));
			json_object_object_add(jobj_unreg_BS_table, "model_name", json_object_new_string(model_name));
			json_object_object_add(jobj_unreg_BS_table, "serial_number", json_object_new_string(serial_number));
			json_object_object_add(jobj_unreg_BS_table, "fw_version", json_object_new_string(fw_version));
			json_object_object_add(jobj_unreg_BS_table, "subnet_mask", json_object_new_string(subnet_mask));
			json_object_object_add(jobj_unreg_BS_table, "gateway", json_object_new_string(gateway));
			json_object_array_add(jarr_unreg_BS_table, jobj_unreg_BS_table);
		}
		else // bs is in the list
		{

			if( (strcmp("-", bsc_id) == 0) // no valid BSC id (BS is reset -> auto remove BS)
				|| (strcmp(curr_bsc_id, bsc_id) != 0) ) // no valid BSC id (BS is binded to other BSC -> auto remove BS)
			{
				debug_print("auto remove idx: %d, id: %s.\n", idx, bs_id);
				removeBsFromList(idx);

				debug_print("unreg idx: %d, bs_id: %s.\n", idx, bs_id);
				debug_print("unreg idx: %d, bs_name: %s.\n", idx, bs_name);
				debug_print("unreg idx: %d, bs_ip: %s.\n", idx, bs_ip);
				debug_print("unreg idx: %d, bs_mac: %s.\n", idx, bs_mac);
				debug_print("unreg idx: %d, bsc_id: %s.\n", idx, bsc_id);
				debug_print("unreg idx: %d, bsc_name: %s.\n", idx, bsc_name);
				debug_print("unreg idx: %d, sync_status: %d.\n", idx, sync_status);
				debug_print("unreg idx: %d, model_name: %s.\n", idx, model_name);
				debug_print("unreg idx: %d, serial_number: %s.\n", idx, serial_number);
				debug_print("unreg idx: %d, fw_version: %s.\n", idx, fw_version);
				debug_print("unreg idx: %d, subnet_mask: %s.\n", idx, subnet_mask);
				debug_print("unreg idx: %d, gateway: %s.\n", idx, gateway);				
				
				jobj_unreg_BS_table = json_object_new_object();
				json_object_object_add(jobj_unreg_BS_table, "bs_id", json_object_new_string(bs_id));
				json_object_object_add(jobj_unreg_BS_table, "bs_name", json_object_new_string(bs_name));
				json_object_object_add(jobj_unreg_BS_table, "bs_ip", json_object_new_string(bs_ip));
				json_object_object_add(jobj_unreg_BS_table, "bs_mac", json_object_new_string(bs_mac));
				json_object_object_add(jobj_unreg_BS_table, "bsc_id", json_object_new_string(bsc_id));
				json_object_object_add(jobj_unreg_BS_table, "bsc_name", json_object_new_string(bsc_name));
				json_object_object_add(jobj_unreg_BS_table, "description", json_object_new_string("---"));
				json_object_object_add(jobj_unreg_BS_table, "sync_status", json_object_new_int(0));
				json_object_object_add(jobj_unreg_BS_table, "model_name", json_object_new_string(model_name));
				json_object_object_add(jobj_unreg_BS_table, "serial_number", json_object_new_string(serial_number));
				json_object_object_add(jobj_unreg_BS_table, "fw_version", json_object_new_string(fw_version));
				json_object_object_add(jobj_unreg_BS_table, "subnet_mask", json_object_new_string(subnet_mask));
				json_object_object_add(jobj_unreg_BS_table, "gateway", json_object_new_string(gateway));				
				json_object_array_add(jarr_unreg_BS_table, jobj_unreg_BS_table);
			}
			else
			{
				//snprintf(cmd, sizeof(cmd), "uci set base-station-list.@base-station[%d].status='%s'", idx, "online");
				//sys_interact(buf, sizeof(buf), cmd);
				api_set_string_option2("online", sizeof("online"), "base-station-list.@base-station[%d].status", idx);
			}
		}
	}

	//snprintf(cmd, sizeof(cmd), "uci commit base-station-list");
	//sys_interact(buf, sizeof(buf), cmd);
	api_commit_option("base-station-list");

	json_object_object_add(jobj, "unreg_base_station_table", jarr_unreg_BS_table);

	// Current Base Station List
	total_nums = getBsCount();

	for(idx = 0; idx < total_nums; idx++)
	{
#if 1
		api_get_string_option2(bs_id, sizeof(bs_id), "base-station-list.@base-station[%d].id", idx);
		api_get_string_option2(bs_name, sizeof(bs_name), "base-station-list.@base-station[%d].name", idx);
		api_get_string_option2(bs_ip, sizeof(bs_ip), "base-station-list.@base-station[%d].ip", idx);
		api_get_string_option2(bs_mac, sizeof(bs_mac), "base-station-list.@base-station[%d].mac", idx);
		api_get_integer_option2(&sync_status, "base-station-list.@base-station[%d].sync_status", idx);
		api_get_string_option2(model_name, sizeof(model_name), "base-station-list.@base-station[%d].model_name", idx);
		api_get_string_option2(serial_number, sizeof(serial_number), "base-station-list.@base-station[%d].serial_number", idx);
		api_get_string_option2(subnet_mask, sizeof(subnet_mask), "base-station-list.@base-station[%d].subnet_mask", idx);
		api_get_string_option2(gateway, sizeof(gateway), "base-station-list.@base-station[%d].gateway", idx);
		api_get_string_option2(bs_status, sizeof(bs_status), "base-station-list.@base-station[%d].status", idx);
		api_get_string_option2(description, sizeof(description), "base-station-list.@base-station[%d].description", idx);
		api_get_integer_option2(&bs_index, "base-station-list.@base-station[%d].index", idx);
		api_get_string_option2(fw_version, sizeof(fw_version), "base-station-list.@base-station[%d].fw_version", idx);
		// add version prefix with letter 'v'
		if((fw_version[0] >= '0') && (fw_version[0] <= '9'))
		{
			strncpy(fw_version_tmp, fw_version, sizeof(fw_version_tmp));
			snprintf(fw_version, sizeof(fw_version), "v%s", fw_version_tmp);
		}
		debug_print("curr idx: %d, fw_version: %s.\n", idx, fw_version);
#else
		snprintf(cmd, sizeof(cmd), "uci get base-station-list.@base-station[%d].id | tr -d \"\\n\"", idx);
		sys_interact(bs_id, sizeof(bs_id), cmd);

		snprintf(cmd, sizeof(cmd), "uci get base-station-list.@base-station[%d].name | tr -d \"\\n\"", idx);
		sys_interact(bs_name, sizeof(bs_name), cmd);

		snprintf(cmd, sizeof(cmd), "uci get base-station-list.@base-station[%d].ip | tr -d \"\\n\"", idx);
		sys_interact(bs_ip, sizeof(bs_ip), cmd);

		snprintf(cmd, sizeof(cmd), "uci get base-station-list.@base-station[%d].mac | tr -d \"\\n\"", idx);
		sys_interact(bs_mac, sizeof(bs_mac), cmd);
#if 0
		snprintf(cmd, sizeof(cmd), "uci get base-station-list.@base-station[%d].sync_status | tr -d \"\\n\"", idx);
		sys_interact(sync_status, sizeof(sync_status), cmd);
#else
		api_get_integer_option2(&sync_status, "base-station-list.@base-station[%d].sync_status", idx);
#endif		
		snprintf(cmd, sizeof(cmd), "uci get base-station-list.@base-station[%d].model_name | tr -d \"\\n\"", idx);
		sys_interact(model_name, sizeof(model_name), cmd);

		snprintf(cmd, sizeof(cmd), "uci get base-station-list.@base-station[%d].serial_number | tr -d \"\\n\"", idx);
		sys_interact(serial_number, sizeof(serial_number), cmd);

		snprintf(cmd, sizeof(cmd), "uci get base-station-list.@base-station[%d].subnet_mask | tr -d \"\\n\"", idx);
		sys_interact(subnet_mask, sizeof(subnet_mask), cmd);

		snprintf(cmd, sizeof(cmd), "uci get base-station-list.@base-station[%d].gateway | tr -d \"\\n\"", idx);
		sys_interact(gateway, sizeof(gateway), cmd);

		snprintf(cmd, sizeof(cmd), "uci get base-station-list.@base-station[%d].status | tr -d \"\\n\"", idx);
		sys_interact(bs_status, sizeof(bs_status), cmd);

		snprintf(cmd, sizeof(cmd), "uci get base-station-list.@base-station[%d].description | tr -d \"\\n\"", idx);
		sys_interact(description, sizeof(description), cmd);

		api_get_integer_option2(&bs_index, "base-station-list.@base-station[%d].index", idx);

		snprintf(cmd, sizeof(cmd), "uci get base-station-list.@base-station[%d].fw_version | tr -d \"\\n\"", idx);
		sys_interact(fw_version, sizeof(fw_version), cmd);
		debug_print("curr idx: %d, fw_version: %s.\n", idx, fw_version);
#endif

		jobj_curr_BS_table = json_object_new_object();
		json_object_object_add(jobj_curr_BS_table, "bs_id", json_object_new_string(bs_id));
		json_object_object_add(jobj_curr_BS_table, "bs_name", json_object_new_string(bs_name));
		json_object_object_add(jobj_curr_BS_table, "bs_ip", json_object_new_string(bs_ip));
		json_object_object_add(jobj_curr_BS_table, "bs_mac", json_object_new_string(bs_mac));
		json_object_object_add(jobj_curr_BS_table, "bs_status", json_object_new_string(bs_status));
		json_object_object_add(jobj_curr_BS_table, "bs_index", json_object_new_int(bs_index));
		json_object_object_add(jobj_curr_BS_table, "description", json_object_new_string(description));
		json_object_object_add(jobj_curr_BS_table, "sync_status", json_object_new_int(sync_status));
		json_object_object_add(jobj_curr_BS_table, "model_name", json_object_new_string(model_name));
		json_object_object_add(jobj_curr_BS_table, "serial_number", json_object_new_string(serial_number));
		json_object_object_add(jobj_curr_BS_table, "fw_version", json_object_new_string(fw_version));
		json_object_object_add(jobj_curr_BS_table, "subnet_mask", json_object_new_string(subnet_mask));
		json_object_object_add(jobj_curr_BS_table, "gateway", json_object_new_string(gateway));
		json_object_array_add(jarr_curr_BS_table, jobj_curr_BS_table);

		debug_print("curr idx: %d, bs_id: %s.\n", idx, bs_id);
		debug_print("curr idx: %d, bs_name: %s.\n", idx, bs_name);
		debug_print("curr idx: %d, bs_ip: %s.\n", idx, bs_ip);
		debug_print("curr idx: %d, bs_mac: %s.\n", idx, bs_mac);
		debug_print("curr idx: %d, bs_status: %s.\n", idx, bs_status);
		debug_print("curr idx: %d, bs_index: %d.\n", idx, bs_index);
		debug_print("curr idx: %d, description: %s.\n", idx, description);
		debug_print("curr idx: %d, sync_status: %d\n", idx, sync_status);
		debug_print("curr idx: %d, serial_number: %s.\n", idx, serial_number);
		debug_print("curr idx: %d, fw_version: %s.\n", idx, fw_version);
		debug_print("curr idx: %d, subnet_mask: %s.\n", idx, subnet_mask);
		debug_print("curr idx: %d, gateway: %s.\n", idx, gateway);
	}

	json_object_object_add(jobj, "curr_base_station_table", jarr_curr_BS_table);

	json_object_object_add(jobj, "curr_bsc_id", json_object_new_string(curr_bsc_id));

	free(curr_bsc_id);
	fclose(fptr);

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_post_rainier_base_station_list_idx(ResponseEntry *rep, char *query_str, int bs_idx)
{
	char *bs_name = NULL, *description = NULL;
	char buf[128] = {0};
	char cmd[128] = {0};
	struct json_object *jobj = NULL;
	ResponseStatus *res = rep->res;
	int idx = -1;
	if(NULL != query_str)
	{
		if((jobj = jsonTokenerParseFromStack(rep, query_str)))
		{
			senao_json_object_get_and_create_string(rep, jobj, "bs_name", &bs_name);
			senao_json_object_get_and_create_string(rep, jobj, "description", &description);
		}
		else
		{
			RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
		}
	}
	else
	{
		RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
	}

	if(strlen(bs_name) > 10)
	{
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "BS NAME");
	}

	if((bs_idx >= RAINIER_BSID_MIN) && (bs_idx <= RAINIER_BSID_MAX))
   	{
		/* check the bs_idx is registered to BSC or not */
		idx = findBsConfigIdxByIndex(bs_idx);
		
		if(idx == -1)
		{
			RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "BS ID");
		}
   	}
   	else
   	{
       	RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "BS ID");
   	}

    /* TODO - insert your code here */
    /* set base list data */
    
	debug_print(" DEBUG[%s][%d] set BSID:[%d] , name:[%s], description:[%s] at index[%d]\n",
			__FUNCTION__, __LINE__,
				bs_idx, bs_name, description, idx);

	if(idx != -1)
	{
		//snprintf(cmd, sizeof(cmd), "uci set base-station-list.@base-station[%d].description='%s'", idx, description);
		//sys_interact(buf, sizeof(buf), cmd);
		api_set_string_option2(description, sizeof(description), "base-station-list.@base-station[%d].description", idx);

		//snprintf(cmd, sizeof(cmd), "uci set base-station-list.@base-station[%d].name='%s'", idx, bs_name);
		//sys_interact(buf, sizeof(buf), cmd);
		api_set_string_option2(bs_name, sizeof(bs_name), "base-station-list.@base-station[%d].name", idx);

		//snprintf(cmd, sizeof(cmd), "uci commit base-station-list");
		//sys_interact(buf, sizeof(buf), cmd);
		api_commit_option("base-station-list");
	}
    /* END - insert your code here */

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_get_rainier_base_station_list_idx(ResponseEntry *rep, struct json_object *jobj, int bs_idx)
{
	char bs_id[32] = {0};
	char bs_name[32] = {0};
	char bs_ip[32] = {0};
	char bs_mac[32] = {0};
	char bsc_id[32] = {0};
	char bsc_name[32] = {0};
	char bs_status[32] = {0};
	int bs_index;
	char description[64] = {0}, model_name[32] = {0}, serial_number[32] = {0}, fw_version[32] = {0};
	char fw_version_tmp[32] = {0};
	char subnet_mask[32] = {0}, gateway[32] = {0};
	int sync_status = 0;
	ResponseStatus *res = rep->res;

	char cmd[1024] = {0};
	int idx = -1;

	int index_value = -1;
	char index_char[8] = {0};

	debug_print("DEBUG %s[%d] bs_idx[%d]\n", __FUNCTION__, __LINE__, bs_idx);

    if((bs_idx >= RAINIER_BSID_MIN) && (bs_idx <= RAINIER_BSID_MAX))
    {
       	/* check the bs_idx is registered to BSC or not */
		idx = findBsConfigIdxByIndex(bs_idx);
		
		if(idx == -1)
		{
			RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "BS ID");
		}

		// Current Base Station List
		//snprintf(cmd, sizeof(cmd), "uci get base-station-list.@base-station[%d].id | tr -d \"\\n\"", idx);
		//sys_interact(bs_id, sizeof(bs_id), cmd);
		api_get_string_option2(bs_id, sizeof(bs_id), "base-station-list.@base-station[%d].id", idx);
		debug_print("curr idx: %d, bs_id: %s.\n", idx, bs_id);

		//snprintf(cmd, sizeof(cmd), "uci get base-station-list.@base-station[%d].name | tr -d \"\\n\"", idx);
		//sys_interact(bs_name, sizeof(bs_name), cmd);
		api_get_string_option2(bs_name, sizeof(bs_name), "base-station-list.@base-station[%d].name", idx);
		debug_print("curr idx: %d, bs_name: %s.\n", idx, bs_name);
		//snprintf(cmd, sizeof(cmd), "uci get base-station-list.@base-station[%d].ip | tr -d \"\\n\"", idx);
		//sys_interact(bs_ip, sizeof(bs_ip), cmd);
		api_get_string_option2(bs_ip, sizeof(bs_ip), "base-station-list.@base-station[%d].ip", idx);
		debug_print("curr idx: %d, bs_ip: %s.\n", idx, bs_ip);
		//snprintf(cmd, sizeof(cmd), "uci get base-station-list.@base-station[%d].mac | tr -d \"\\n\"", idx);
		//sys_interact(bs_mac, sizeof(bs_mac), cmd);
		api_get_string_option2(bs_mac, sizeof(bs_mac), "base-station-list.@base-station[%d].mac", idx);
		debug_print("curr idx: %d, bs_mac: %s.\n", idx, bs_mac);
#if 0
		snprintf(cmd, sizeof(cmd), "uci get base-station-list.@base-station[%d].sync_status | tr -d \"\\n\"", idx);
		sys_interact(sync_status, sizeof(sync_status), cmd);
		debug_print("curr idx: %d, sync_status: %s.\n", idx, sync_status);
#else
		api_get_integer_option2(&sync_status, "base-station-list.@base-station[%d].sync_status", idx);
		debug_print("curr idx: %d, sync_status: %d\n", idx, sync_status);
#endif		
		//snprintf(cmd, sizeof(cmd), "uci get base-station-list.@base-station[%d].model_name | tr -d \"\\n\"", idx);
		//sys_interact(model_name, sizeof(model_name), cmd);
		api_get_string_option2(model_name, sizeof(model_name), "base-station-list.@base-station[%d].model_name", idx);
		//snprintf(cmd, sizeof(cmd), "uci get base-station-list.@base-station[%d].serial_number | tr -d \"\\n\"", idx);
		//sys_interact(serial_number, sizeof(serial_number), cmd);
		api_get_string_option2(serial_number, sizeof(serial_number), "base-station-list.@base-station[%d].serial_number", idx);
		debug_print("curr idx: %d, serial_number: %s.\n", idx, serial_number);
		//snprintf(cmd, sizeof(cmd), "uci get base-station-list.@base-station[%d].subnet_mask | tr -d \"\\n\"", idx);
		//sys_interact(subnet_mask, sizeof(subnet_mask), cmd);
		api_get_string_option2(subnet_mask, sizeof(subnet_mask), "base-station-list.@base-station[%d].subnet_mask", idx);
		debug_print("curr idx: %d, subnet_mask: %s.\n", idx, subnet_mask);
		//snprintf(cmd, sizeof(cmd), "uci get base-station-list.@base-station[%d].gateway | tr -d \"\\n\"", idx);
		//sys_interact(gateway, sizeof(gateway), cmd);
		api_get_string_option2(gateway, sizeof(gateway), "base-station-list.@base-station[%d].gateway", idx);
		debug_print("curr idx: %d, gateway: %s.\n", idx, gateway);
		//snprintf(cmd, sizeof(cmd), "uci get base-station-list.@base-station[%d].status | tr -d \"\\n\"", idx);
		//sys_interact(bs_status, sizeof(bs_status), cmd);
		api_get_string_option2(bs_status, sizeof(bs_status), "base-station-list.@base-station[%d].status", idx);
		debug_print("curr idx: %d, bs_status: %s.\n", idx, bs_status);
		//snprintf(cmd, sizeof(cmd), "uci get base-station-list.@base-station[%d].index | tr -d \"\\n\"", idx);
		//sys_interact(index_char, sizeof(index_char), cmd);
		api_get_string_option2(index_char, sizeof(index_char), "base-station-list.@base-station[%d].index", idx);
		index_value = atoi(index_char);
		debug_print("curr idx: %d, bs_index: %d.\n", idx, index_value);
		bs_index = index_value;
		//snprintf(cmd, sizeof(cmd), "uci get base-station-list.@base-station[%d].description | tr -d \"\\n\"", idx);
		//sys_interact(description, sizeof(description), cmd);
		api_get_string_option2(description, sizeof(description), "base-station-list.@base-station[%d].description", idx);
		debug_print("curr idx: %d, description: %s.\n", idx, description);
		//snprintf(cmd, sizeof(cmd), "uci get base-station-list.@base-station[%d].fw_version | tr -d \"\\n\"", idx);
		//sys_interact(fw_version, sizeof(fw_version), cmd);
		api_get_string_option2(fw_version, sizeof(fw_version), "base-station-list.@base-station[%d].fw_version", idx);
		// add version prefix with letter 'v'
		if((fw_version[0] >= '0') && (fw_version[0] <= '9'))
		{
			strncpy(fw_version_tmp, fw_version, sizeof(fw_version_tmp));
			snprintf(fw_version, sizeof(fw_version), "v%s", fw_version_tmp);
		}
		debug_print("curr idx: %d, fw_version: %s.\n", idx, fw_version);
		
		json_object_object_add(jobj, "bs_id", json_object_new_string(bs_id));
		json_object_object_add(jobj, "bs_name", json_object_new_string(bs_name));
		json_object_object_add(jobj, "bs_ip", json_object_new_string(bs_ip));
		json_object_object_add(jobj, "bs_mac", json_object_new_string(bs_mac));
		json_object_object_add(jobj, "bs_status", json_object_new_string(bs_status));
		json_object_object_add(jobj, "bs_index", json_object_new_int(bs_index));
		json_object_object_add(jobj, "description", json_object_new_string(description));
		json_object_object_add(jobj, "sync_status", json_object_new_int(sync_status));
		json_object_object_add(jobj, "model_name", json_object_new_string(model_name));
		json_object_object_add(jobj, "serial_number", json_object_new_string(serial_number));
		json_object_object_add(jobj, "fw_version", json_object_new_string(fw_version));
		json_object_object_add(jobj, "subnet_mask", json_object_new_string(subnet_mask));
		json_object_object_add(jobj, "gateway", json_object_new_string(gateway));
		
		RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
    }
    else
    {
        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "BS ID");
    }
}

void generateRandomToken(char *var, int len)
{
	int i;
	char hexString[] = "abcdefghijklmnopqrstuvwxyz0123456789";
	struct timeval tv;

	gettimeofday(&tv, NULL);
	srand((unsigned int)tv.tv_usec);

	for(i = 0 ; i < len ; i++)
	{
		sprintf(var + i, "%c", hexString[random()%36]);
	}
	var[len] = '\0';
}

void setBit(int *number, int bit_pos)
{
	// 7 6 5 4 3 2 1 0
	*number |= (1 << bit_pos);
}

void clearBit(int *number, int bit_pos)
{
	// 7 6 5 4 3 2 1 0
	*number &= ~(1 << bit_pos);
}

void toggleBit(int *number, int bit_pos)
{
	// 7 6 5 4 3 2 1 0
	*number ^= (1 << bit_pos);
}

int checkBit(int *number, int bit_pos)
{
	// 7 6 5 4 3 2 1 0
	return (*number >> bit_pos) & 1;
}

int uciGetBsIdxStatus()
{
	char cmd[256] = {0};
	char buf[256] = {0};

	//snprintf(cmd, sizeof(cmd), "uci get base-station-list.@index-status[0].val | tr -d \"\\n\"");
	//sys_interact(buf, sizeof(buf), cmd);
	api_get_string_option("base-station-list.@index-status[0].val", buf, sizeof(buf));

	return atoi(buf);
}

void uciSetBsIdxStatus(int bit_pos)
{
	char cmd[256] = {0};
	char buf[256] = {0};

	int indexStatus = uciGetBsIdxStatus();

	setBit(&indexStatus, bit_pos);

	//snprintf(cmd, sizeof(cmd), "uci set base-station-list.@index-status[0].val=%d", indexStatus);
	//sys_interact(buf, sizeof(buf), cmd);
	api_set_integer_option("base-station-list.@index-status[0].val", indexStatus);
}

void uciClearBsIdxStatus(int bit_pos)
{
	char cmd[256] = {0};
	char buf[256] = {0};

	int indexStatus = uciGetBsIdxStatus();

	clearBit(&indexStatus, bit_pos);

	//snprintf(cmd, sizeof(cmd), "uci set base-station-list.@index-status[0].val=%d", indexStatus);
	//sys_interact(buf, sizeof(buf), cmd);
	api_set_integer_option("base-station-list.@index-status[0].val", indexStatus);
}

int getAvailableBsIndex()
{
	int idx, indexStatus;

	indexStatus = uciGetBsIdxStatus();

	if (indexStatus >= 255) // 2^8 - 1
		return -1;

	for (idx = 0; idx < 8; idx++)
	{
		if (!checkBit(&indexStatus, idx))
			return idx;
	}
}


int get_avahi_line_info(char *id, char *sync, char *sn, char *sub_mask, char *gw, char *model, char *fw_v)
{
	
	char sync_status[32] = {0};
	char serial_number[32] = {0};
	char subnet_mask[32] = {0};
	char gateway[32] = {0};	
	char bs_id[32] = {0};	
	char bs_ip[32] = {0};	
	char model_name[32] = {0};
	char version[32] = {0};
	FILE *fptr;
	char cmd[1024] = {0};
	char *path = "/etc/avahi_device_list";
	char line[1024] = {0};
	char other[1024] = {0};
	int count, i, ch;

	int file_EOF = 0;

	if ((fptr = fopen(path, "r")) == NULL)
		return -1;

	while(!feof(fptr))
	{

		count = 0;

		while(1)
		{
			ch = getc(fptr);
			if(ch == EOF)
			{
				file_EOF = 1;
				break;
			}
			line[count] = ch;

			if(line[count] == '\n')
				break;

			count++;
		}

		if(file_EOF)
			break;
		//fscanf(fptr, "%[^\n]\n", line); why
		sscanf(line, "%s%[^\n]\n", bs_ip, other);

#if 1
		get_avahi_info(other, "device_id", bs_id, sizeof(bs_id));
		get_avahi_info(other, "sync_status", sync_status, sizeof(sync_status));
		get_avahi_info(other, "serial_number", serial_number, sizeof(serial_number));
		get_avahi_info(other, "version", version, sizeof(version));
		get_avahi_info(other, "subnet_mask", subnet_mask, sizeof(subnet_mask));
		get_avahi_info(other, "gateway", gateway, sizeof(gateway));
		get_avahi_info(other, "modelname", model_name, sizeof(model_name));

#else
		snprintf(cmd, sizeof(cmd), "echo %s | awk '{for (i=1;i<=NF;i++){if ($i ~/device_id/) {print $i}}}' | sed -e 's/\"//g' | awk -F'=' '{print $2}' 2>/dev/null | tr -d \"\\n\"", other);
		sys_interact_long(bs_id, sizeof(bs_id), cmd);

		/* */
		snprintf(cmd, sizeof(cmd), "echo %s | awk '{for (i=1;i<=NF;i++){if ($i ~/sync_status/) {print $i}}}' | sed -e 's/\"//g' | awk -F'=' '{print $2}' 2>/dev/null | tr -d \"\\n\"", other);
		sys_interact_long(sync_status, sizeof(sync_status), cmd);

		snprintf(cmd, sizeof(cmd), "echo %s | awk '{for (i=1;i<=NF;i++){if ($i ~/serial_number/) {print $i}}}' | sed -e 's/\"//g' | awk -F'=' '{print $2}' 2>/dev/null | tr -d \"\\n\"", other);
		sys_interact_long(serial_number, sizeof(serial_number), cmd);

		snprintf(cmd, sizeof(cmd), "echo %s | awk '{for (i=1;i<=NF;i++){if ($i ~/subnet_mask/) {print $i}}}' | sed -e 's/\"//g' | awk -F'=' '{print $2}' 2>/dev/null | tr -d \"\\n\"", other);
		sys_interact_long(subnet_mask, sizeof(subnet_mask), cmd);

		snprintf(cmd, sizeof(cmd), "echo %s | awk '{for (i=1;i<=NF;i++){if ($i ~/gateway/) {print $i}}}' | sed -e 's/\"//g' | awk -F'=' '{print $2}' 2>/dev/null | tr -d \"\\n\"", other);
		sys_interact_long(gateway, sizeof(gateway), cmd);


		snprintf(cmd, sizeof(cmd), "echo %s | awk '{for (i=1;i<=NF;i++){if ($i ~/modelname/) {print $i}}}' | sed -e 's/\"//g' | awk -F'=' '{print $2}' 2>/dev/null | tr -d \"\\n\"", other);
		sys_interact_long(model_name, sizeof(model_name), cmd);

		snprintf(cmd, sizeof(cmd), "echo %s | awk '{for (i=1;i<=NF;i++){if ($i ~/version/) {print $i}}}' | sed -e 's/\"//g' | awk -F'=' '{print $2}' 2>/dev/null | tr -d \"\\n\"", other);
		sys_interact_long(version, sizeof(version), cmd);
#endif

		/* */
		if(strcmp(bs_id, id) == 0)
		{
			debug_print("Jason DEBUG %s[%d] find bs id: %s.\n", __FUNCTION__, __LINE__, bs_id);
			debug_print("Jason DEBUG %s[%d] sync_status: %s.\n", __FUNCTION__, __LINE__, sync_status);
			debug_print("Jason DEBUG %s[%d] serial_number: %s.\n", __FUNCTION__, __LINE__, serial_number);
			debug_print("Jason DEBUG %s[%d] subnet_mask: %s.\n", __FUNCTION__, __LINE__, subnet_mask);
			debug_print("Jason DEBUG %s[%d] gateway: %s.\n", __FUNCTION__, __LINE__, gateway);
			debug_print("Jason DEBUG %s[%d] version: %s.\n", __FUNCTION__, __LINE__, version);
			debug_print("Jason DEBUG %s[%d] model_name: %s.\n", __FUNCTION__, __LINE__, model_name);

			strcpy(sync, sync_status);
			strcpy(sn, serial_number);
			strcpy(sub_mask, subnet_mask);
			strcpy(gw, gateway);
			strcpy(model, model_name);
			strcpy(fw_v, version);
			return 1;
		}

	}

	return -1;
}



// "BSC" -> BS (BSC side)
int bind_base_station(int bind, char *bs_id, char *bs_name, char *bs_mac, char *bs_ip)
{
	char cmd[1024] = {0};
	char buf[256] = {0};
	int ret = 0;

	char username[32] = {0};
	char password[32] = {0};

	char device_token[256] = {0};
	char *bsc_id;
	char *bsc_name;
	char *bsc_key;
	char *bs_key;

	char curr_ip[32] = {0};
	char curr_mac[32] = {0};

	char sync_status[32] = {0};
	char serial_number[32] = {0};
	char subnet_mask[32] = {0};
	char gateway[32] = {0};	

	char model_name[32] = {0};
	char fw_version[32] = {0};	


	int bs_index = -1; // index for BS
	int idx = -1; // index for uci config

	int iSystemID = 0;

	// If the System ID is uninitialized or is a reserved value, generate a random System ID
	// System ID 0x0000 0xFFFF 0x1234 is for special usage
	api_get_integer_option("rainier.rainier_basic_base.system_id", &iSystemID);
	if( (iSystemID == 0x0000) || (iSystemID == 0xFFFF) || (iSystemID == 0x1234) )
	{
		srand(time(NULL));
		while( (iSystemID == 0x0000) || (iSystemID == 0xFFFF) || (iSystemID == 0x1234) )
		{
			// reserved value value, generate a random iSystemID
			iSystemID = rand()%65536;
		}
		api_set_integer_option("rainier.rainier_basic_base.system_id", iSystemID);
		api_commit_option("rainier");
	}

	debug_print("Jason DEBUG %s[%d] bind: %d.\n", __FUNCTION__, __LINE__, bind);
	debug_print("Jason DEBUG %s[%d] bs_id: %s.\n", __FUNCTION__, __LINE__, bs_id);
	debug_print("Jason DEBUG %s[%d] bs_name: %s.\n", __FUNCTION__, __LINE__, bs_name);
	debug_print("Jason DEBUG %s[%d] bs_mac: %s.\n", __FUNCTION__, __LINE__, bs_mac);
	debug_print("Jason DEBUG %s[%d] bs_ip: %s.\n", __FUNCTION__, __LINE__, bs_ip);

	bsc_id = (char*)malloc(13*sizeof(char));
	getBscId(bsc_id, 13);

	bsc_name = (char*)malloc(32*sizeof(char));
	getBscName(bsc_name, 32);

	bsc_key = (char*)malloc(11*sizeof(char));
	generateRandomToken(bsc_key, 10);

	bs_key = (char*)malloc(11*sizeof(char));
	generateRandomToken(bs_key, 10);

	sys_interact(curr_ip, sizeof(curr_ip), "ifconfig br-lan | grep Bcast | awk {'printf $2'} | awk -F':' '{printf $2}'");
	sys_interact(curr_mac, sizeof(curr_mac), "ifconfig br-lan | grep HWaddr | awk {'printf $5'}");

	idx = findBsConfigIdx(bs_id);

	if((bind) && (idx != -1))
	{
		debug_print("Jason DEBUG %s[%d] bind bs, but the bs is already bind\n", __FUNCTION__, __LINE__);
		ret = -3;
		goto EXIT;
	}
	else if((!bind) && (idx == -1))
	{
		debug_print("Jason DEBUG %s[%d] unbind bs, but the bs is not yet bind\n", __FUNCTION__, __LINE__);
		ret = -3;
		goto EXIT;
	}

	if(bind)
	{
		bs_index = getAvailableBsIndex();
		if(bs_index == -1) // Full
		{
			debug_print("Jason DEBUG %s[%d] bs_index full\n", __FUNCTION__, __LINE__);
			ret = -2;
			goto EXIT;
		}

		/* add offset from (0~7) to (1~8) */
		bs_index += 1; 
	}
	else
	{
		api_get_integer_option2(&bs_index, "base-station-list.@base-station[%d].index", idx);
	}

	debug_print("Jason DEBUG %s[%d] idx: %d bs_index: %d \n", __FUNCTION__, __LINE__, idx, bs_index);

	if(idx != -1)
	{
		snprintf(username, sizeof(username), "%s", bsc_id);
		//snprintf(cmd, sizeof(cmd), "uci get base-station-list.@base-station[%d].bs_key | tr -d \"\\n\"", idx);
		//sys_interact(password, sizeof(password), cmd);
		api_get_string_option2(password, sizeof(password), "base-station-list.@base-station[%d].bs_key", idx);
	}
	else
	{
		snprintf(username, sizeof(username), "admin");
		snprintf(password, sizeof(password), "admin");
	}

	snprintf(cmd, sizeof(cmd), "curl --connect-timeout 3 --max-time 20 -v -k -X POST \"https://%s:4430/api/sys/login\" -H \"accept: */*\" -H \"Content-Type: application/json\" -d \"{\\\"username\\\":\\\"%s\\\",\\\"password\\\":\\\"%s\\\"}\" | grep token | awk '{for (i=1;i<=NF;i++){if ($i ~/token/) {print $(i+1)}}}' | sed -e 's/\"//g' | tr -d \"\\n\"", bs_ip, username, password);
	debug_print("cmd: %s.", cmd);
	sys_interact_long(device_token, sizeof(device_token), cmd);
	debug_print("device_token: %s.", device_token);

	if(strcmp(device_token, "---") == 0)
	{
		debug_print("Jason DEBUG %s[%d] fail to login bs\n", __FUNCTION__, __LINE__);
		ret = -1;
		goto EXIT;
	}

	snprintf(cmd, sizeof(cmd), "curl --connect-timeout 3 --max-time 20 -v -k -X POST \"https://%s:4430/api/rainier/bind_bs\" -H \"accept: */*\" -H \"Authorization: Bearer %s\" -d \"{\\\"bind\\\":\\\"%d\\\",\\\"bsc_id\\\":\\\"%s\\\",\\\"bsc_name\\\":\\\"%s\\\",\\\"bsc_key\\\":\\\"%s\\\",\\\"bs_key\\\":\\\"%s\\\",\\\"bsc_ip\\\":\\\"%s\\\",\\\"bsc_mac\\\":\\\"%s\\\",\\\"bs_index\\\":\\\"%d\\\"}\" | awk '{for (i=1;i<=NF;i++){if ($i ~/status_code/) {print $(i+1)}}}' | sed -e 's/\"//g' | tr -d \"\\n\"", bs_ip, device_token, bind, bsc_id, bsc_name, bsc_key, bs_key, curr_ip, curr_mac, bs_index);
	sys_interact_long(buf, sizeof(buf), cmd);
	debug_print("-- buf: %s.", buf);

	if(strcmp(buf, "200,") == 0) // OK
	{
		if(bind)
		{
			if(idx != -1) // duplicate case -> just update
			{
				debug_print("duplicate: %d.\n", idx);
			}
			else // normal case -> add bs at last postion of list
			{
				//snprintf(cmd, sizeof(cmd), "uci add base-station-list base-station");
				//sys_interact(buf, sizeof(buf), cmd);
				api_add_new_section("base-station-list", "base-station");
				//idx = -1;
			}

#if 1
			api_set_string_option2(bs_id, sizeof(bs_id), "base-station-list.@base-station[%d].id", idx);
			api_set_string_option2(bs_name, sizeof(bs_name), "base-station-list.@base-station[%d].name", idx);
			api_set_string_option2(bs_mac, sizeof(bs_mac), "base-station-list.@base-station[%d].mac", idx);
			api_set_string_option2(bs_ip, sizeof(bs_ip), "base-station-list.@base-station[%d].ip", idx);
			api_set_string_option2(bsc_key, sizeof(bsc_key), "base-station-list.@base-station[%d].bsc_key", idx);
			api_set_string_option2(bs_key, sizeof(bs_key), "base-station-list.@base-station[%d].bs_key", idx);
			api_set_string_option2("online", sizeof("online"), "base-station-list.@base-station[%d].status", idx);
			api_set_integer_option2(bs_index, "base-station-list.@base-station[%d].index", idx);
#else
			snprintf(cmd, sizeof(cmd), "uci set base-station-list.@base-station[%d].id='%s'", idx, bs_id);
			sys_interact(buf, sizeof(buf), cmd);

			snprintf(cmd, sizeof(cmd), "uci set base-station-list.@base-station[%d].name='%s'", idx, bs_name);
			sys_interact(buf, sizeof(buf), cmd);

			snprintf(cmd, sizeof(cmd), "uci set base-station-list.@base-station[%d].mac='%s'", idx, bs_mac);
			sys_interact(buf, sizeof(buf), cmd);

			snprintf(cmd, sizeof(cmd), "uci set base-station-list.@base-station[%d].ip='%s'", idx, bs_ip);
			sys_interact(buf, sizeof(buf), cmd);

			snprintf(cmd, sizeof(cmd), "uci set base-station-list.@base-station[%d].bsc_key='%s'", idx, bsc_key);
			sys_interact(buf, sizeof(buf), cmd);

			snprintf(cmd, sizeof(cmd), "uci set base-station-list.@base-station[%d].bs_key='%s'", idx, bs_key);
			sys_interact(buf, sizeof(buf), cmd);

			snprintf(cmd, sizeof(cmd), "uci set base-station-list.@base-station[%d].status='%s'", idx, "online");
			sys_interact(buf, sizeof(buf), cmd);

			snprintf(cmd, sizeof(cmd), "uci set base-station-list.@base-station[%d].index=%d", idx, bs_index);
			sys_interact(buf, sizeof(buf), cmd);
#endif
			
			/* Richard added 2020-0224*/
			if(get_avahi_line_info(bs_id, sync_status, serial_number, subnet_mask, gateway, model_name, fw_version) > 0)
			{
#if 1
				api_set_string_option2(sync_status, sizeof(sync_status), "base-station-list.@base-station[%d].sync_status", idx);
				api_set_string_option2(serial_number, sizeof(serial_number), "base-station-list.@base-station[%d].serial_number", idx);
				api_set_string_option2(subnet_mask, sizeof(subnet_mask), "base-station-list.@base-station[%d].subnet_mask", idx);
				api_set_string_option2(gateway, sizeof(gateway), "base-station-list.@base-station[%d].gateway", idx);
				// Rename default model_name to DuraFon Roam
				if(strcmp(model_name, "SP938BS") == 0)
				{
					strcpy(model_name, "DuraFon Roam");
				}
				api_set_string_option2(model_name, sizeof(model_name), "base-station-list.@base-station[%d].model_name", idx);
				api_set_string_option2(fw_version, sizeof(fw_version), "base-station-list.@base-station[%d].fw_version", idx);
				api_set_string_option2("---", sizeof("---"), "base-station-list.@base-station[%d].description", idx);
#else
				snprintf(cmd, sizeof(cmd), "uci set base-station-list.@base-station[%d].sync_status='%s'", idx, sync_status);
				sys_interact(buf, sizeof(buf), cmd);

				snprintf(cmd, sizeof(cmd), "uci set base-station-list.@base-station[%d].serial_number='%s'", idx, serial_number);
				sys_interact(buf, sizeof(buf), cmd);

				snprintf(cmd, sizeof(cmd), "uci set base-station-list.@base-station[%d].subnet_mask='%s'", idx, subnet_mask);
				sys_interact(buf, sizeof(buf), cmd);

				snprintf(cmd, sizeof(cmd), "uci set base-station-list.@base-station[%d].gateway='%s'", idx, gateway);
				sys_interact(buf, sizeof(buf), cmd);

				// Rename default model_name to DuraFon Roam
				if(strcmp(model_name, "SP938BS") == 0)
				{
					strcpy(model_name, "DuraFon Roam");
				}
				snprintf(cmd, sizeof(cmd), "uci set base-station-list.@base-station[%d].model_name='%s'", idx, model_name);
				sys_interact(buf, sizeof(buf), cmd);

				snprintf(cmd, sizeof(cmd), "uci set base-station-list.@base-station[%d].fw_version='%s'", idx, fw_version);
				sys_interact(buf, sizeof(buf), cmd);

				snprintf(cmd, sizeof(cmd), "uci set base-station-list.@base-station[%d].description='%s'", idx, "---");
				sys_interact(buf, sizeof(buf), cmd);
#endif
			}

			/* shift to 0~7 */
			uciSetBsIdxStatus(bs_index - 1);

			//snprintf(cmd, sizeof(cmd), "uci commit base-station-list");
			//sys_interact(buf, sizeof(buf), cmd);
			api_commit_option("base-station-list");
	
			// if bind/unbind success, reload NMS BS tale
			SYSTEM("nmsconf_cli table reloadBsTable");

			// event log
			{
				char device[8] = {0};
				sprintf(device, "BS%d", bs_index);
				log_data_base_write("General", device, "DUT Status", "Base managed.");
			}
		}
		else // unbind
		{
			if(idx != -1)
			{
				debug_print("unbind idx: %d, %s. %s.\n", idx, buf, bs_id);
				removeBsFromList(idx);

				//snprintf(cmd, sizeof(cmd), "uci commit base-station-list");
				//sys_interact(buf, sizeof(buf), cmd);
				api_commit_option("base-station-list");
	
				// if bind/unbind success, reload NMS BS tale
				SYSTEM("nmsconf_cli table reloadBsTable");

				// event log
				{
					char device[8] = {0};
					sprintf(device, "BS%d", bs_index);
					log_data_base_write("General", device, "DUT Status", "Base unmanaged.");
				}
			}
			else
			{
				debug_print("Jason DEBUG %s[%d] unbind bs, but the bs is not yet bind\n", __FUNCTION__, __LINE__);
				ret = -1;
			}
		}
	}
	else
	{
		debug_print("Jason DEBUG %s[%d] fail to bind or unbind bs\n", __FUNCTION__, __LINE__);
		ret = -1;
	}

EXIT:

	free(bsc_id);
	free(bsc_name);
	free(bsc_key);
	free(bs_key);

	return ret;
}

// "BSC" -> BS (BSC side)
int json_post_rainier_bind_bs_info(ResponseEntry *rep, char *query_str)
{
	int bind;
	char *bs_id = NULL;
	char *bs_name = NULL;
	char *bs_mac = NULL;
	char *bs_ip = NULL;

	int status;

	struct json_object *jobj = NULL;
	ResponseStatus *res = rep->res;

	if(NULL != query_str)
	{
		if((jobj = jsonTokenerParseFromStack(rep, query_str)))
		{
			senao_json_object_get_integer(jobj, "bind", &bind);
			senao_json_object_get_and_create_string(rep, jobj, "bs_id", &bs_id);
			senao_json_object_get_and_create_string(rep, jobj, "bs_name", &bs_name);
			senao_json_object_get_and_create_string(rep, jobj, "bs_mac", &bs_mac);
			senao_json_object_get_and_create_string(rep, jobj, "bs_ip", &bs_ip);

			if(strlen(bs_name) > 10)
			{
				RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "BS NAME");
			}

			status = bind_base_station(bind, bs_id, bs_name, bs_mac, bs_ip);

			if(status == -1)
			{
				RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "Unable to bind or unbind Base");
			}
			else if(status == -2)
			{
				RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "The number of base stations should be smaller than 8");
			}
			else if(status == -3)
			{
				RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "bind or unbind info error");
			}
		}
		else
		{
			RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
		}
	}
	else
	{
		RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
	}

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_post_rainier_remove_bs_info(ResponseEntry *rep, char *query_str)
{
	char cmd[128] = {0};
	char buf[128] = {0};

	char *bs_id = NULL;
	int idx;
	int bs_index = 0;

	struct json_object *jobj = NULL;
	ResponseStatus *res = rep->res;

	if(NULL != query_str)
	{
		if((jobj = jsonTokenerParseFromStack(rep, query_str)))
		{
			senao_json_object_get_and_create_string(rep, jobj, "bs_id", &bs_id);

			idx = findBsConfigIdx(bs_id);
			if(idx == -1)
			{
				debug_print("Jason DEBUG %s[%d] remove bs, but the bs is not yet bind\n", __FUNCTION__, __LINE__);
				RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "bind or unbind info error");
			}

			api_get_integer_option2(&bs_index, "base-station-list.@base-station[%d].index", idx);
			
			debug_print("delete idx: %d, id: %s.\n", idx, bs_id);
			removeBsFromList(idx);

			//snprintf(cmd, sizeof(cmd), "uci commit base-station-list");
			//sys_interact(buf, sizeof(buf), cmd);
			api_commit_option("base-station-list");

			// if remove success, reload NMS BS tale
			SYSTEM("nmsconf_cli table reloadBsTable");

			// event log
			{
				char device[8] = {0};
				sprintf(device, "BS%d", bs_index);
				log_data_base_write("General", device, "DUT Status", "Base removed.");
			}
		}
		else
		{
			RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
		}
	}
	else
	{
		RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
	}

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int updateBsIp(char *bs_id, char *bs_ip)
{
	char cmd[1024] = {0};
	char buf[256] = {0};

	char list_ip[32] = {0};
	char curr_mac[32] = {0};

	int idx = -1;

	debug_print("Jason DEBUG %s[%d] bs_id: %s.\n", __FUNCTION__, __LINE__, bs_id);
	debug_print("Jason DEBUG %s[%d] bs_ip: %s.\n", __FUNCTION__, __LINE__, bs_ip);

	idx = findBsConfigIdx(bs_id);

	if(idx == -1)
		return -1;
	else
	{
		//snprintf(cmd, sizeof(cmd), "uci get base-station-list.@base-station[%d].ip | tr -d \"\\n\"", idx);
		//sys_interact(list_ip, sizeof(list_ip), cmd);
		api_get_string_option2(list_ip, sizeof(list_ip), "base-station-list.@base-station[%d].ip", idx);

		if(strcmp(bs_ip, list_ip) != 0)
		{
			//snprintf(cmd, sizeof(cmd), "uci set base-station-list.@base-station[%d].ip='%s'", idx, bs_ip);
			//sys_interact(buf, sizeof(buf), cmd);
			api_set_string_option2(bs_ip, sizeof(bs_ip), "base-station-list.@base-station[%d].ip", idx);

			//snprintf(cmd, sizeof(cmd), "uci commit base-station-list");
			//sys_interact(buf, sizeof(buf), cmd);
			api_commit_option("base-station-list");
		}
	}

	return 0;
}

int updateBsFwVersion(char *bs_id, char *fw_version)
{
	char cmd[1024] = {0};
	char buf[256] = {0};

	char list_fw_ver[32] = {0};
	//char curr_mac[32] = {0};

	int idx = -1;

	debug_print("Jason DEBUG %s[%d] bs_id: %s.\n", __FUNCTION__, __LINE__, bs_id);
	debug_print("Jason DEBUG %s[%d] fw_version: %s.\n", __FUNCTION__, __LINE__, fw_version);

	idx = findBsConfigIdx(bs_id);

	if(idx == -1)
		return -1;
	else
	{
		//snprintf(cmd, sizeof(cmd), "uci get base-station-list.@base-station[%d].fw_version | tr -d \"\\n\"", idx);
		//sys_interact(list_fw_ver, sizeof(list_fw_ver), cmd);
		api_get_string_option2(list_fw_ver, sizeof(list_fw_ver), "base-station-list.@base-station[%d].fw_version", idx);

		if(strcmp(fw_version, list_fw_ver) != 0)
		{
			//snprintf(cmd, sizeof(cmd), "uci set base-station-list.@base-station[%d].fw_version='%s'", idx, fw_version);
			//sys_interact(buf, sizeof(buf), cmd);
			api_set_string_option2(fw_version, sizeof(fw_version), "base-station-list.@base-station[%d].fw_version", idx);

			//snprintf(cmd, sizeof(cmd), "uci commit base-station-list");
			//sys_interact(buf, sizeof(buf), cmd);
			api_commit_option("base-station-list");
		}
	}

	return 0;
}

int ptp_status_to_database(int ptp_sts, int bs_idx)
{
	sqlite3 *sqldb = NULL;
	char columns[] = "time, bs_index, ptp_sync";
	char buf[256] = {0};
	char date[32] = {0};
	char time[16] = {0};

	if(open_database(DB_PTP_SYNC_TABLE, columns, DB_PTP_PATH, &sqldb) < 0)
	{
		debug_print("Jason DEBUG %s[%d] open database fail !!\n", __FUNCTION__, __LINE__);
		return -1;
	}

   	sys_interact(date, sizeof(date), "%s", "date +\"%Y-%m-%d %H:%M:%S\"");

	date[strlen(date) - 1] = '\0';

	sprintf(buf, "'%s',%d,%d", 
			 date, bs_idx + 1, ptp_sts);
	if(sqldb != NULL)
	{
		insert_to_table(sqldb, DB_PTP_SYNC_TABLE, columns, buf);
		free_database(sqldb);
	}
	return 0;
}


int update_sync_status(char *bs_id, int sts)
{

	int idx = -1;
	char cmd[1024] = {0};
	char buf[256] = {0};
	int  conf_stat;

	idx = findBsConfigIdx(bs_id);

	if(idx == -1)
	{
	
		return -1;
	}
	else
	{
		//snprintf(cmd, sizeof(cmd), "uci get base-station-list.@base-station[%d].sync_status | tr -d \"\\n\"", idx);
		//sys_interact(buf, sizeof(buf), cmd);
		api_get_string_option2(buf, sizeof(buf), "base-station-list.@base-station[%d].sync_status", idx);

		buf[1] = '\0';

		if(atoi(buf) == sts)
			return 0;

		//snprintf(cmd, sizeof(cmd), "uci set base-station-list.@base-station[%d].sync_status=%d", idx, sts);
		//sys_interact(buf, sizeof(buf), cmd);
		api_set_integer_option2(sts, "base-station-list.@base-station[%d].sync_status", idx);
		
		debug_print("Jason DEBUG %s[%d] update sync_sts: %d\n", __FUNCTION__, __LINE__, sts);

		//snprintf(cmd, sizeof(cmd), "uci commit base-station-list");
		//sys_interact(buf, sizeof(buf), cmd);
		api_commit_option("base-station-list");


		ptp_status_to_database(sts, idx);
	}
	
	return 0;
}


int json_post_rainier_notify_bsc(ResponseEntry *rep, char *query_str)
{
	char *bs_id = NULL;
	char *bs_ip = NULL;
	char *fw_version = NULL;
	int  sync_sts;

	int status;

	struct json_object *jobj = NULL;
	ResponseStatus *res = rep->res;

	if(NULL != query_str)
	{
		if((jobj = jsonTokenerParseFromStack(rep, query_str)))
		{
			senao_json_object_get_and_create_string(rep, jobj, "bs_id", &bs_id);
			senao_json_object_get_and_create_string(rep, jobj, "bs_ip", &bs_ip);
			senao_json_object_get_and_create_string(rep, jobj, "fw_version", &fw_version);

			senao_json_object_get_integer(jobj, "sync_status", &sync_sts);


			status = updateBsIp(bs_id, bs_ip);

			if(status == -1)
				RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "BS ID");

			status = updateBsFwVersion(bs_id, fw_version);

			if(status == -1)
				RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "BS ID");

			status = update_sync_status(bs_id, sync_sts);

			if(status == -1)
				RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "BS ID");
		}
		else
		{
			RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
		}
	}
	else
	{
		RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
	}

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

// BSC -> "BS" (BS side)
int json_post_rainier_bind_bs(ResponseEntry *rep, char *query_str)
{
	int bind;
	char *bsc_id = NULL;
	char *bsc_name = NULL;
	char *bs_key = NULL;
	char *bsc_key = NULL;
	char *bsc_ip = NULL;
	char *bsc_mac = NULL;
	int bs_index = -1;
	char device_token[1024] = {0};

	char bs_key_md5[40] = {0};

	char cmd[1024] = {0};
	char buf[128] = {0};

	char dash[2] = {'-'};
	char *service_path = "/etc/avahi/services/bonjour.service";

	int val;

	struct json_object *jobj = NULL;
	ResponseStatus *res = rep->res;

	if(NULL != query_str)
	{
		if((jobj = jsonTokenerParseFromStack(rep, query_str)))
		{
			senao_json_object_get_integer(jobj, "bind", &bind);
			senao_json_object_get_and_create_string(rep, jobj, "bsc_id", &bsc_id);
			senao_json_object_get_and_create_string(rep, jobj, "bsc_name", &bsc_name);
			senao_json_object_get_and_create_string(rep, jobj, "bsc_key", &bsc_key);
			senao_json_object_get_and_create_string(rep, jobj, "bs_key", &bs_key);
			senao_json_object_get_and_create_string(rep, jobj, "bsc_ip", &bsc_ip);
			senao_json_object_get_and_create_string(rep, jobj, "bsc_mac", &bsc_mac);
			senao_json_object_get_integer(jobj, "bs_index", &bs_index);

			debug_print("Jason DEBUG %s[%d] bind: %d\n", __FUNCTION__, __LINE__, bind);
			debug_print("Jason DEBUG %s[%d] bsc_id: %s\n", __FUNCTION__, __LINE__, bsc_id);
			debug_print("Jason DEBUG %s[%d] bsc_name: %s\n", __FUNCTION__, __LINE__, bsc_name);
			debug_print("Jason DEBUG %s[%d] bsc_key: %s\n", __FUNCTION__, __LINE__, bsc_key);
			debug_print("Jason DEBUG %s[%d] bs_key: %s\n", __FUNCTION__, __LINE__, bs_key);
			debug_print("Jason DEBUG %s[%d] bsc_ip: %s\n", __FUNCTION__, __LINE__, bsc_ip);
			debug_print("Jason DEBUG %s[%d] bsc_mac: %s\n", __FUNCTION__, __LINE__, bsc_mac);
			debug_print("Jason DEBUG %s[%d] bs_index: %d\n", __FUNCTION__, __LINE__, bs_index);

			if(bind)
			{
				snprintf(cmd, sizeof(cmd), "echo %s | md5sum | awk '{print $1}' | tr -d \"\n\"", bs_key);
				sys_interact(bs_key_md5, sizeof(bs_key_md5), cmd);
#if CREATE_BSC_BIND_AUTH
				debug_print("Jason DEBUG %s[%d] bs_key[%s]  bs_key_md5[%s]\n", __FUNCTION__, __LINE__, bs_key, bs_key_md5);
				if (api_set_string_option("base-station-controller.bsc_0.login_key", bs_key, sizeof(bs_key)) != API_RC_SUCCESS)
					RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "BSC LOGIN_KEY");

#else
				snprintf(cmd, sizeof(cmd), "/lib/auth.sh set_restfulAPI_auth \"%s\" \"%s\"", bsc_id, bs_key_md5);
				debug_print("cmd: %s.", cmd);
				sys_interact(buf, sizeof(buf), cmd);
#endif

				if (api_set_string_option("base-station-controller.bsc_0.id", bsc_id, sizeof(bsc_id)) != API_RC_SUCCESS)
					RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "BSC ID");

				if (api_set_string_option("base-station-controller.bsc_0.name", bsc_name, sizeof(bsc_name)) != API_RC_SUCCESS)
					RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "BSC NAME");

				if (api_set_string_option("base-station-controller.bsc_0.key", bsc_key, sizeof(bsc_key)) != API_RC_SUCCESS)
					RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "BSC KEY");

				if (api_set_string_option("base-station-controller.bsc_0.ip", bsc_ip, sizeof(bsc_ip)) != API_RC_SUCCESS)
					RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "BSC IP");

				if (api_set_string_option("base-station-controller.bsc_0.mac", bsc_mac, sizeof(bsc_mac)) != API_RC_SUCCESS)
					RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "BSC MAC");

				if (api_set_integer_option("base-station-controller.bsc_0.bs_index", bs_index) != API_RC_SUCCESS)
					RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "BS INDEX");

				snprintf(cmd, sizeof(cmd), "sed -i 's/bsc_id=.*</bsc_id=%s</g' %s", bsc_id, service_path);
				debug_print("sed cmd: %s.", cmd);
				sys_interact(buf, sizeof(buf), cmd);

				snprintf(cmd, sizeof(cmd), "sed -i 's/bsc_name=.*</bsc_name=%s</g' %s", bsc_name, service_path);
				debug_print("sed cmd: %s.", cmd);
				sys_interact(buf, sizeof(buf), cmd);
			}
			else
			{
#if CREATE_BSC_BIND_AUTH
				if (api_set_string_option("base-station-controller.bsc_0.login_key", dash, sizeof(dash)) != API_RC_SUCCESS)
					RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "BSC LOGIN_KEY");

#else
				snprintf(cmd, sizeof(cmd), "/lib/auth.sh set_restfulAPI_auth \"%s\" \"%s\"", "admin", "456b7016a916a4b178dd72b947c152b7");
				debug_print("cmd: %s.", cmd);
				sys_interact(buf, sizeof(buf), cmd);
#endif
				if (api_set_string_option("base-station-controller.bsc_0.id", dash, sizeof(dash)) != API_RC_SUCCESS)
					RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "BSC ID");

				if (api_set_string_option("base-station-controller.bsc_0.name", dash, sizeof(dash)) != API_RC_SUCCESS)
					RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "BSC NAME");

				if (api_set_string_option("base-station-controller.bsc_0.key", dash, sizeof(dash)) != API_RC_SUCCESS)
					RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "BSC KEY");

				if (api_set_string_option("base-station-controller.bsc_0.ip", dash, sizeof(dash)) != API_RC_SUCCESS)
					RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "BSC IP");

				if (api_set_string_option("base-station-controller.bsc_0.mac", dash, sizeof(dash)) != API_RC_SUCCESS)
					RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "BSC MAC");

				if (api_set_integer_option("base-station-controller.bsc_0.bs_index", -1) != API_RC_SUCCESS)
					RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "BS INDEX");

				snprintf(cmd, sizeof(cmd), "sed -i 's/bsc_id=.*</bsc_id=%s</g' %s", dash, service_path);
				debug_print("sed cmd: %s.", cmd);
				sys_interact(buf, sizeof(buf), cmd);

				snprintf(cmd, sizeof(cmd), "sed -i 's/bsc_name=.*</bsc_name=%s</g' %s", dash, service_path);
				debug_print("sed cmd: %s.", cmd);
				sys_interact(buf, sizeof(buf), cmd);
			}

			//snprintf(cmd, sizeof(cmd), "uci commit base-station-controller");
			//sys_interact(buf, sizeof(buf), cmd);
			api_commit_option("base-station-controller");

			if(bind)
			{
				snprintf(cmd, sizeof(cmd), "pgrep bs_monitor | tr -d \"\n\"");
				sys_interact(buf, sizeof(buf), cmd);
				val = atoi(buf);
				if(val > 0)
				{
					debug_print("bs_monitor is running: %d\n", val);
				}
				else
				{
					//snprintf(cmd, sizeof(cmd), "bs-monitor &");
					system("/usr/shc/bs-monitor start &");
				}

				// if bind success, NMS send server discover
				SYSTEM("nmsconf_cli conf_mgm nmsServDiscover");
			}
			else
			{
				system("/usr/shc/bs-monitor stop &");

				// if unbind success, NMS device reset
				SYSTEM("nmsconf_cli conf_mgm deviceReset");


			}
			
			/* reload avahi-daemon config */
			system("/etc/init.d/avahi-daemon reload");
			debug_print("Jason DEBUG %s[%d] avahi-daemon reload  !!\n", __FUNCTION__, __LINE__);
		}
		else
		{
			RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
		}
	}
	else
	{
		RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
	}

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

// "BSC" -> BS (BSC side)
int set_register_mode(int hs_reg)
{
	char cmd[1024] = {0};
	char buf[256] = {0};

	char username[32] = {0};
	char password[32] = {0};

	char device_token[256] = {0};
	char *bsc_id;

	char bs_ip[32] = {0};

	int idx = -1;
	int bs_num;
	int i;
	int ret = -1;

	debug_print("Jason DEBUG %s[%d] hs_reg: %d.\n", __FUNCTION__, __LINE__, hs_reg);

	bsc_id = (char*)malloc(13*sizeof(char));
	getBscId(bsc_id, 13);

	bs_num = getBsCount();

	for(i = 0; i < bs_num; i++)
	{
#if 1	// use NMS report BS Status
		//snprintf(cmd, sizeof(cmd), "uci get base-station-list.@base-station[%d].index | tr -d \"\\n\"", i);
		//sys_interact(buf, sizeof(buf), cmd);
		api_get_string_option2(buf, sizeof(buf), "base-station-list.@base-station[%d].index", i);
		idx = atoi(buf);

		sprintf(cmd, "%s/%s_%d", NMS_DEVICE_DIR, NMS_DEVICE_BS, idx);
		if(sysIsFileExisted(cmd) != TRUE)
		{
			debug_print("BS%d offline, continue to next base.\n", idx);
			continue;	// the base offline, continue to next base
		}
#endif

		snprintf(username, sizeof(username), "%s", bsc_id);

		//snprintf(cmd, sizeof(cmd), "uci get base-station-list.@base-station[%d].bs_key | tr -d \"\\n\"", i);
		//sys_interact(password, sizeof(password), cmd);
		api_get_string_option2(password, sizeof(password), "base-station-list.@base-station[%d].bs_key", i);

		//snprintf(cmd, sizeof(cmd), "uci get base-station-list.@base-station[%d].ip | tr -d \"\\n\"", i);
		//sys_interact(bs_ip, sizeof(bs_ip), cmd);
		api_get_string_option2(bs_ip, sizeof(bs_ip), "base-station-list.@base-station[%d].ip", i);

		snprintf(cmd, sizeof(cmd), "curl --connect-timeout 3 --max-time 20 -v -k -X POST \"https://%s:4430/api/sys/login\" -H \"accept: */*\" -H \"Content-Type: application/json\" -d \"{\\\"username\\\":\\\"%s\\\",\\\"password\\\":\\\"%s\\\"}\" | grep token | awk '{for (i=1;i<=NF;i++){if ($i ~/token/) {print $(i+1)}}}' | sed -e 's/\"//g' | tr -d \"\\n\"", bs_ip, username, password);
		debug_print("cmd: %s.", cmd);
		sys_interact_long(device_token, sizeof(device_token), cmd);
		debug_print("device_token: %s.", device_token);

		if(strcmp(device_token, "---") == 0)
		{
			continue;	// login fails, continue to next base
		}

		snprintf(cmd, sizeof(cmd), "curl --connect-timeout 3 --max-time 20 -v -k -X POST \"https://%s:4430/api/mgm/rainier/reg_mode\" -H \"accept: */*\" -H \"Authorization: Bearer %s\" -d \"{\\\"reg_mode\\\":\\\"%d\\\"}\" | awk '{for (i=1;i<=NF;i++){if ($i ~/status_code/) {print $(i+1)}}}' | sed -e 's/\"//g' | tr -d \"\\n\"", bs_ip, device_token, hs_reg);
		sys_interact_long(buf, sizeof(buf), cmd);
		debug_print("buf: %s.", buf);

		if(strcmp(buf, "200,") == 0) // OK
		{
			// at least one BS receive register mode command and enter the corresponding register/de-register/normal mode. 
			ret = 0;
		}
	}

	free(bsc_id);

	return ret;
}

// "BSC" -> BS (BSC side)
int json_post_rainier_hs_reg(ResponseEntry *rep, char *query_str)
{
	int hs_reg;
	int hs_id;
	int hs_group;
	int status;
	int rainierReg = 0;
	char buf_ok[32] = {0};
	char buf_ng[32] = {0};
	char buf_rsvd[32] = {0};
	char result[32] = {0};
	//char reg_mode[8] = {0};
	char buf[64] = {0};
	char display_name[64] = {0};
	int wait_time = 0;
	int wait_time_extended = 0;

	struct json_object *jobj = NULL;
	ResponseStatus *res = rep->res;

	if(NULL != query_str)
	{
		if((jobj = jsonTokenerParseFromStack(rep, query_str)))
		{
			senao_json_object_get_integer(jobj, "hs_reg", &hs_reg);
			senao_json_object_get_integer(jobj, "hs_id", &hs_id);
			senao_json_object_get_integer(jobj, "hs_group", &hs_group);

			if(hs_reg == 1)	// register mode
			{
				// check hs_id is registered or not.
				if(api_get_bool_option2(&rainierReg, "sip_hs.sip_hs_%d.rainierReg", hs_id) != API_RC_SUCCESS)
				{
					RET_GEN_ERRORMSG(res, API_INTERNAL_ERROR, "GET HS REG");
				}

				if(rainierReg == 1)
				{
					/* the hs_id is already registered */
					RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "Handset ID is already registered");
				}

				// record hs_id, hs_group to be register to tmp file
				SYSTEM("echo reg hs_%d grp_%d start > /tmp/rainier_hs_reg", hs_id, hs_group);
			}
			else if(hs_reg == 2)	// de-register mode
			{
				// check hs_id is registered or not.
				if(api_get_bool_option2(&rainierReg, "sip_hs.sip_hs_%d.rainierReg", hs_id) != API_RC_SUCCESS)
				{
					RET_GEN_ERRORMSG(res, API_INTERNAL_ERROR, "GET HS REG");
				}

				if(rainierReg == 0)
				{
					/* the hs_id is not yet registered */
					RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "Handset ID is not registered");
				}

				// record hs_id, hs_group to be de-register to tmp file
				SYSTEM("echo dereg hs_%d grp_%d start > /tmp/rainier_hs_reg", hs_id, hs_group);
			}
			else	// normal mode 
			{
				// remove tmp file
				SYSTEM("rm /tmp/rainier_hs_reg");
			}
			
			status = set_register_mode(hs_reg);

			if(status == -1)
			{
				// remove tmp file
				SYSTEM("rm /tmp/rainier_hs_reg");
				RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "Unable to set register mode to Base");
			}
			
			// register/de-register mode wait HS register/de-register result
			if((hs_reg == 1) || (hs_reg == 2))
			{
				if(hs_reg == 1)
				{
					//snprintf(reg_mode, sizeof(reg_mode), "reg");
					snprintf(buf_ok, sizeof(buf_ok), "reg hs_%d grp_%d ok\n", hs_id, hs_group);
					snprintf(buf_ng, sizeof(buf_ng), "reg hs_%d grp_%d ng\n", hs_id, hs_group);
					snprintf(buf_rsvd, sizeof(buf_rsvd), "reg hs_%d grp_%d rsvd\n", hs_id, hs_group);
				}
				else if(hs_reg == 2)
				{
					snprintf(buf_ok, sizeof(buf_ok), "dereg hs_%d grp_%d ok\n", hs_id, hs_group);
					snprintf(buf_ng, sizeof(buf_ng), "dereg hs_%d grp_%d ng\n", hs_id, hs_group);
					snprintf(buf_rsvd, sizeof(buf_rsvd), "dereg hs_%d grp_%d rsvd\n", hs_id, hs_group);
				}

				//wait_time = 20;	// 20 * 1s
				wait_time = 40;		// 40 * 500ms
				while (wait_time >= 0)
				{
					//sleep(1);
					msleep(500);
					//debug_print("wait_time[%d]\n", wait_time);
					wait_time--;

					if(sysIsFileExisted("/tmp/rainier_hs_reg") != TRUE)
					{
						RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "Handset register or de-register Stop");
					}

					sys_interact(result, sizeof(result), "cat /tmp/rainier_hs_reg 2>/dev/null");

					if(strcmp(result, buf_ok) == 0) // OK
					{
						if(hs_reg == 1)
						{
							/* record this hs_id is registered */
							api_set_bool_option2(1, "sip_hs.sip_hs_%d.rainierReg", hs_id);
							api_set_integer_option2(hs_group, "sip_hs.sip_hs_%d.rainier_group", hs_id);

							//api_commit_option("rainier");
							//SYSTEM("uci commit rainier");
							api_commit_option("sip_hs");

							// event log
							{
								char device[8] = {0};
								sprintf(device, "HS%d", hs_id);
								log_data_base_write("General", device, "Status", "Handset registered.");
							}
						}
						else if(hs_reg == 2)
						{
							/* record this hs_id is de-registered */
							api_set_bool_option2(0, "sip_hs.sip_hs_%d.rainierReg", hs_id);
							api_set_integer_option2(0, "sip_hs.sip_hs_%d.rainier_group", hs_id);
							api_set_integer_option2(255, "sip_hs.sip_hs_%d.subscribe_bs", hs_id);
							api_set_bool_option2(1, "sip_hs.sip_hs_%d.enable", hs_id);
							sprintf(buf, "sip_hs.sip_hs_%d.display_name", hs_id);
							//sprintf(display_name, "%d", hs_id);
							sprintf(display_name, "");	// default set empty display name
							api_set_rainier_sip_acc_display_name(buf, display_name, sizeof(display_name));

							//api_commit_option("rainier");
							//SYSTEM("uci commit rainier");
							api_commit_option("sip_hs");

							// event log
							{
								char device[8] = {0};
								sprintf(device, "HS%d", hs_id);
								log_data_base_write("General", device, "Status", "Handset de-registered.");
							}
						}
						
						// set all BS to normal mode
						SYSTEM("rm /tmp/rainier_hs_reg");
						set_register_mode(0);

						// reload asterisk and NMS HS tale
						SYSTEM("astgen");
						SYSTEM("asterisk -rx 'core reload'");
						SYSTEM("nmsconf_cli table reloadHsTable %d", hs_id - 9);
						SYSTEM("nmsconf_cli table sendHsTable %d", hs_id - 9);

						RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
					}
					else if(strcmp(result, buf_ng) == 0) // NG
					{
						// set all BS to normal mode
						SYSTEM("rm /tmp/rainier_hs_reg");
						set_register_mode(0);
						RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "Handset register or de-register fail");
					}
					else if(strcmp(result, buf_rsvd) == 0) // RSVD
					{
						// extend timeout if base get hsid and not yet post hsid
						if(!wait_time_extended)
						{
							//wait_time += 5;	// 5 * 1s
							wait_time += 10;	// 10 * 500ms
							wait_time_extended = 1;
						}
					}
				}
				
				SYSTEM("rm /tmp/rainier_hs_reg");
				set_register_mode(0);
				RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "Handset register or de-register timeout");
			}
		}
		else
		{
			RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
		}
	}
	else
	{
		RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
	}

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_post_rainier_hs_remove(ResponseEntry *rep, char *query_str)
{
	int hsid[RAINIER_HS_NUMBER] = {0};
	int rainierReg = 0;
	int arraylen = 0;
	int i;
	char buf[64] = {0};
	char display_name[64] = {0};
	struct json_object *jobj = NULL, *jarr = NULL, *jarr_obj = NULL;
    ResponseStatus *res = rep->res;

    /* POST hsid data to BSC */
    if(NULL != query_str)
    {
        if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
			jarr = json_object_object_get(jobj, "hsids");
            arraylen = json_object_array_length(jarr) ;
			
			if(arraylen > RAINIER_HS_NUMBER)
			{
				RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "Too many Handset ID");
			}
			else if(arraylen <= 0)
			{
				RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "No Handset ID");
			}

            for (i = 0; i < arraylen; i++) 
            {
                jarr_obj = json_object_array_get_idx(jarr, i);
				senao_json_object_get_integer(jarr_obj, "hsid", &hsid[i]);

				if((hsid[i] >= RAINIER_HSID_MIN) && (hsid[i] <= RAINIER_HSID_MAX))
				{
					// check hsid is registered or not.
					if(api_get_bool_option2(&rainierReg, "sip_hs.sip_hs_%d.rainierReg", hsid[i]) != API_RC_SUCCESS)
					{
						RET_GEN_ERRORMSG(res, API_INTERNAL_ERROR, "GET HS REG");
					}
	
					if(rainierReg == 0)
					{
						/* the hsid is not yet registered */
						RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "Handset ID is not registered");
					}
	
					debug_print("remove handset id: %d.\n", hsid[i]);
				}
				else
				{
			        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "Invalid Handset ID");
				}
				
            }
        }
        else
        {
            RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
        }
    }
    else
    {
        RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
    }
    
	for (i = 0; i < arraylen; i++)
    {
		/* record this hsid is de-registered */
		api_set_bool_option2(0, "sip_hs.sip_hs_%d.rainierReg", hsid[i]);
		api_set_integer_option2(0, "sip_hs.sip_hs_%d.rainier_group", hsid[i]);
		api_set_integer_option2(255, "sip_hs.sip_hs_%d.subscribe_bs", hsid[i]);
		api_set_bool_option2(1, "sip_hs.sip_hs_%d.enable", hsid[i]);
		sprintf(buf, "sip_hs.sip_hs_%d.display_name", hsid[i]);
		//sprintf(display_name, "%d", hsid[i]);
		sprintf(display_name, "");	// default set empty display name
		api_set_rainier_sip_acc_display_name(buf, display_name, sizeof(display_name));

		// event log
		{
			char device[8] = {0};
			sprintf(device, "HS%d", hsid[i]);
			log_data_base_write("General", device, "Status", "Handset removed.");
		}
    }

	//api_commit_option("rainier");
	api_commit_option("sip_hs");

	// reload asterisk and NMS HS tale
	SYSTEM("astgen");
	SYSTEM("asterisk -rx 'core reload'");
	SYSTEM("nmsconf_cli table reloadHsTable");
	SYSTEM("nmsconf_cli table sendHsTable");
	
    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_get_rainier_topology(ResponseEntry *rep, struct json_object *jobj)
{
	char bs_name[32] = {0}, bs_ip[32] = {0}, bs_mac[32] = {0}, bs_status[32] = {0};
	int bs_poe_status;
	//int bs_index = 0 ,hs_id = 0, hs_status = 0, hs_type = 0;
	int hs_status[RAINIER_HS_NUMBER] = {0};
	int hs_type[RAINIER_HS_NUMBER] = {0};
	int hs_id[RAINIER_HS_NUMBER] = {0};
	int HsRxBsRssi[RAINIER_HS_NUMBER] = {0};
	int BsRxHsRssi[RAINIER_HS_NUMBER] = {0};
	int location[RAINIER_HS_NUMBER] = {0};
	int fxo_status[RAINIER_FXO_NUM] = {0};
	char display_name[64] = {0};
	int hs_num = 0;
	int idx;
	int i, j;
	//char cmd[256] = {0};
	ResponseStatus *res = rep->res;

	char cmd[1024] = {0};
	char buf[8192] = {0};
	char device_token[1024] = {0};

	int arraylen = 0;
	int poe_port_num = 0;
	int total_ports = 9;
	int bs_index;

	int port_no[9] = {0};
	bool poe_enable[9] = {-1};
	char *poe_power_draw[9] = {NULL};
	char *poe_status[9] = {NULL};
	
	int lldp_port_no[9] = {0};
	char lldp_chassis_id[9][20] = {0};
	char lldp_remote_ip[9][20] = {0};
	char lldp_sys_name[9][20] = {0};
	int lldp_portNo = 0;
	char *lldp_chassisID = NULL;
	char *lldp_remoteIp = NULL;
	char *lldp_sysName = NULL;

	char *restful_res_str = NULL;

	struct json_object *res_jobj = NULL;
	struct json_object *port_poe_cfg_jobj = NULL;
	struct json_object *jarr_obj = NULL;
	struct json_object *poe_jobj = NULL;
	struct json_object *lldp_jobj = NULL;
	struct json_object *lldp_remote_info_jobj = NULL;

	struct json_object *jarr_topology_base_list = NULL, *jobj_topology_base_list = NULL;
	jarr_topology_base_list = json_object_new_array();

	struct json_object *jarr_topology_handset_list = NULL, *jobj_topology_handset_list = NULL;
	//jarr_topology_handset_list = json_object_new_array();

	struct json_object *jarr_univailable_handset_list = NULL, *jobj_univailable_handset_list = NULL;
	jarr_univailable_handset_list = json_object_new_array();


	/* get handset rssi data */
	hs_num = getHandsetRssiData(hs_id, HsRxBsRssi, BsRxHsRssi, location);

    /* get handset status */
	ast_get_hs_fxo_status(hs_status, fxo_status);
    
	
	/* TODO - insert your code here */
    
	/* get handset type */

    /* END - insert your code here */

	/* get base poe and lldp status */
	snprintf(cmd, sizeof(cmd), "curl --connect-timeout 3 --max-time 20 -X PATCH \"http://%s:8000/api/system/login\" -H \"accept: application/json\" -H \"Content-Type: application/json\" -d \"{'user':'admin','password':'password'}\" | grep token | awk -F\\\" '{for (i=1;i<=NF;i++){if ($i ~/token/) {print $(i+2)}}}' | tr -d \"\\n\"", SWITCH_IP);
	sys_interact_long(device_token, sizeof(device_token), cmd);
	//debug_print("device_token: %s. \n", device_token);

	snprintf(cmd, sizeof(cmd), "curl --connect-timeout 3 --max-time 20 -v -k -X GET \"http://%s:8000/api/ports/poe\" -H \"accept: */*\" -H \"Content-Type: application/json\" -H \"Authorization: Bearer %s\"", SWITCH_IP, device_token); 
	sys_interact_long(buf, sizeof(buf), cmd);

	//debug_print("-- poe buf: %s. \n\n\n", buf);
   	
	if((poe_jobj = jsonTokenerParseFromStack(rep, buf)))
    {
		senao_json_object_get_and_create_string(rep, poe_jobj, "restful_res", &restful_res_str);
		//debug_print("****  restful_res_str: %s. \n\n\n", restful_res_str);
		if((res_jobj = jsonTokenerParseFromStack(rep, restful_res_str)))
		{
			port_poe_cfg_jobj = json_object_object_get(res_jobj, "portPoeConfs");

           	poe_port_num  = json_object_array_length(port_poe_cfg_jobj) ;		

			for (i = 0; i < poe_port_num; i++) 
            {
				jarr_obj = json_object_array_get_idx(port_poe_cfg_jobj, i);

				senao_json_object_get_integer(jarr_obj, "portNo", &port_no[i]);
				senao_json_object_get_boolean(jarr_obj, "enable", &poe_enable[i]);
				senao_json_object_get_and_create_string(rep, jarr_obj, "powerDraw", &poe_power_draw[i]);
				senao_json_object_get_and_create_string(rep, jarr_obj, "status", &poe_status[i]);
				
				//debug_print("portID[%d] \n", port_no[i]);
				//debug_print("poe_enable[%d] \n", poe_enable[i]);
				//debug_print("poe_power_draw[%s] \n", poe_power_draw[i]);
				//debug_print("poe_status[%s] \n", poe_status[i]);
			}
		}
	}

	snprintf(cmd, sizeof(cmd), "curl --connect-timeout 3 --max-time 20 -v -k -X GET \"http://%s:8000/api/lldp/remote\" -H \"accept: */*\" -H \"Content-Type: application/json\" -H \"Authorization: Bearer %s\"", SWITCH_IP, device_token); 
	sys_interact_long(buf, sizeof(buf), cmd);

	//debug_print("-- lldp buf: %s. \n\n\n", buf);
   	
	if((lldp_jobj = jsonTokenerParseFromStack(rep, buf)))
    {
		senao_json_object_get_and_create_string(rep, lldp_jobj, "restful_res", &restful_res_str);
		//debug_print("****  restful_res_str: %s. \n\n\n", restful_res_str);
		if((res_jobj = jsonTokenerParseFromStack(rep, restful_res_str)))
		{
			lldp_remote_info_jobj = json_object_object_get(res_jobj, "lldpRemoteInfo");

           	arraylen  = json_object_array_length(lldp_remote_info_jobj) ;		

			for (i = 0; i < arraylen; i++) 
            {
				jarr_obj = json_object_array_get_idx(lldp_remote_info_jobj, i);

				senao_json_object_get_integer(jarr_obj, "portNo", &lldp_portNo);
				senao_json_object_get_and_create_string(rep, jarr_obj, "chassisID", &lldp_chassisID);
				senao_json_object_get_and_create_string(rep, jarr_obj, "remoteIp", &lldp_remoteIp);
				senao_json_object_get_and_create_string(rep, jarr_obj, "sysName", &lldp_sysName);
				
				//debug_print("lldp_portNo[%d] \n", lldp_portNo);
				//debug_print("lldp_chassisID[%s] \n", lldp_chassisID);
				//debug_print("lldp_remoteIp[%s] \n", lldp_remoteIp);
				//debug_print("lldp_sysName[%s] \n", lldp_sysName);

				if((lldp_portNo > total_ports) || (lldp_portNo < 1))
				{
					continue;
				}

				if (lldp_port_no[lldp_portNo-1] == 0)	// first time found an lldp device on this port
				{
					lldp_port_no[lldp_portNo-1] = lldp_portNo;
					strncpy(lldp_chassis_id[lldp_portNo-1], lldp_chassisID, sizeof(lldp_chassis_id[lldp_portNo-1]));
					strncpy(lldp_remote_ip[lldp_portNo-1], lldp_remoteIp, sizeof(lldp_remote_ip[lldp_portNo-1]));
					strncpy(lldp_sys_name[lldp_portNo-1], lldp_sysName, sizeof(lldp_sys_name[lldp_portNo-1]));
				}
				else	// more than one lldp device on this port, set sys_name to Unknown Device
				{
					lldp_port_no[lldp_portNo-1] = lldp_portNo;
					strncpy(lldp_chassis_id[lldp_portNo-1], "00:00:00:00:00:00", sizeof(lldp_chassis_id[lldp_portNo-1]));
					strncpy(lldp_remote_ip[lldp_portNo-1], "0.0.0.0", sizeof(lldp_remote_ip[lldp_portNo-1]));
					strncpy(lldp_sys_name[lldp_portNo-1], "Unknown Device", sizeof(lldp_sys_name[lldp_portNo-1]));
				}
			}
		}
	}

	snprintf(cmd, sizeof(cmd), "curl --connect-timeout 3 --max-time 20 -v -k -X PATCH \"http://%s:8000/api/system/logout\" -H \"accept: */*\" -H \"Authorization: Bearer %s\" | awk -F\\\" '{for (i=1;i<=NF;i++){if ($i ~/errCode/) {print $(i+1)}}}' | tr -d \"\\n\"", SWITCH_IP, device_token);
	sys_interact_long(buf, sizeof(buf), cmd);

#if 1
	for(i = RAINIER_BSID_MIN; i <= RAINIER_BSID_MAX; i++)
	{
		idx = findBsConfigIdxByIndex(i);
		if(idx != -1) // bs is in the list
		{
			//snprintf(cmd, sizeof(cmd), "uci get base-station-list.@base-station[%d].name | tr -d \"\\n\"", idx);
			//sys_interact(bs_name, sizeof(bs_name), cmd);
			api_get_string_option2(bs_name, sizeof(bs_name), "base-station-list.@base-station[%d].name", idx);

			//snprintf(cmd, sizeof(cmd), "uci get base-station-list.@base-station[%d].ip | tr -d \"\\n\"", idx);
			//sys_interact(bs_ip, sizeof(bs_ip), cmd);
			api_get_string_option2(bs_ip, sizeof(bs_ip), "base-station-list.@base-station[%d].ip", idx);

			//snprintf(cmd, sizeof(cmd), "uci get base-station-list.@base-station[%d].mac | tr -d \"\\n\"", idx);
			//sys_interact(bs_mac, sizeof(bs_mac), cmd);
			api_get_string_option2(bs_mac, sizeof(bs_mac), "base-station-list.@base-station[%d].mac", idx);

#if 1	// use NMS report BS Status
			sprintf(cmd, "%s/%s_%d", NMS_DEVICE_DIR, NMS_DEVICE_BS, i);
			if(sysIsFileExisted(cmd) == TRUE)
			{
				strcpy(bs_status, "online");
			}
			else
			{
				strcpy(bs_status, "offline");
			}
#else
			//snprintf(cmd, sizeof(cmd), "uci get base-station-list.@base-station[%d].status | tr -d \"\\n\"", idx);
			//sys_interact(bs_status, sizeof(bs_status), cmd);
			api_get_string_option2(bs_status, sizeof(bs_status), "base-station-list.@base-station[%d].status", idx);
#endif

			/* check poe status */
			bs_poe_status = 0;	// Not PoE
			for(j = 0; j < total_ports; j++)
			{
				//debug_print("lldp_portNo[%d] = %d \n", j+1, lldp_port_no[j]);

				if(lldp_port_no[j])	// this port found lldp device
				{
					if(strcmp(strupr(bs_mac), strupr(lldp_chassis_id[j])) == 0)	// the bs is at this port
					{
						//debug_print("bs is at port[%d] \n", j+1);

						if((j < poe_port_num) && (poe_power_draw[j] != NULL))
						{
							if(strcmp(poe_power_draw[j], "0.0") != 0)	// this port has poe client
							{
								bs_poe_status = 1;	// PoE
							}
						}

						break;
					}
				}
			}

			jobj_topology_base_list = json_object_new_object();
			json_object_object_add(jobj_topology_base_list, "bs_index", json_object_new_int(i));
			json_object_object_add(jobj_topology_base_list, "bs_name", json_object_new_string(bs_name));
			json_object_object_add(jobj_topology_base_list, "bs_ip", json_object_new_string(bs_ip));
			json_object_object_add(jobj_topology_base_list, "bs_mac", json_object_new_string(bs_mac));
			json_object_object_add(jobj_topology_base_list, "bs_status", json_object_new_string(bs_status));
			json_object_object_add(jobj_topology_base_list, "bs_poe_status", json_object_new_int(bs_poe_status));
	
			jarr_topology_handset_list = json_object_new_array();
	
			for(j = 0; j < hs_num; j++)
			{
				if(location[j] == i)
				{
					jobj_topology_handset_list = json_object_new_object();
					json_object_object_add(jobj_topology_handset_list, "hs_id", json_object_new_int(hs_id[j]));
					json_object_object_add(jobj_topology_handset_list, "hs_status", json_object_new_int(hs_status[hs_id[j]-RAINIER_HSID_MIN]));
					json_object_object_add(jobj_topology_handset_list, "hs_type", json_object_new_int(hs_type[hs_id[j]-RAINIER_HSID_MIN]));
					api_get_string_option2(display_name, sizeof(display_name), "sip_hs.sip_hs_%d.display_name", hs_id[j]);
					json_object_object_add(jobj_topology_handset_list, "display_name", json_object_new_string(display_name));
					json_object_array_add(jarr_topology_handset_list, jobj_topology_handset_list);

					//debug_print("HS%d status:%d type:%d under BS%d\n", hs_id[j], hs_status[hs_id[j]-RAINIER_HSID_MIN], hs_type[hs_id[j]-RAINIER_HSID_MIN], location[j]);
				}
			}
	
			json_object_object_add(jobj_topology_base_list, "topology_handset_list", jarr_topology_handset_list);
	
			json_object_array_add(jarr_topology_base_list, jobj_topology_base_list);
		}
	}

	json_object_object_add(jobj, "topology_base_list", jarr_topology_base_list);

	for(i = 0; i < hs_num; i++)
	{
		if(location[i] == 0)
		{
			jobj_univailable_handset_list = json_object_new_object();
			json_object_object_add(jobj_univailable_handset_list, "hs_id", json_object_new_int(hs_id[i]));
			json_object_object_add(jobj_univailable_handset_list, "hs_status", json_object_new_int(/*hs_status[hs_id[i]-RAINIER_HSID_MIN]*/ 3));	// 3: Un-available
			json_object_object_add(jobj_univailable_handset_list, "hs_type", json_object_new_int(hs_type[hs_id[i]-RAINIER_HSID_MIN]));
			api_get_string_option2(display_name, sizeof(display_name), "sip_hs.sip_hs_%d.display_name", hs_id[i]);
			json_object_object_add(jobj_univailable_handset_list, "display_name", json_object_new_string(display_name));
			json_object_array_add(jarr_univailable_handset_list, jobj_univailable_handset_list);

			//debug_print("HS%d status:%d type:%d under no BS\n", hs_id[i], hs_status[hs_id[i]-RAINIER_HSID_MIN], hs_type[hs_id[i]-RAINIER_HSID_MIN]);
		}
	}

	json_object_object_add(jobj, "univailable_handset_list", jarr_univailable_handset_list);

#else
	for(i = RAINIER_BSID_MIN; i <= RAINIER_BSID_MAX; i++)
	{
		jobj_topology_base_list = json_object_new_object();
		json_object_object_add(jobj_topology_base_list, "bs_index", json_object_new_int(bs_index));
		json_object_object_add(jobj_topology_base_list, "bs_name", json_object_new_string(bs_name));
		json_object_object_add(jobj_topology_base_list, "bs_ip", json_object_new_string(bs_ip));
		json_object_object_add(jobj_topology_base_list, "bs_mac", json_object_new_string(bs_mac));
		json_object_object_add(jobj_topology_base_list, "bs_status", json_object_new_string(bs_status));

		jarr_topology_handset_list = json_object_new_array();

		for(j = RAINIER_HSID_MIN; j <= RAINIER_HSID_MAX; j++)
		{
			jobj_topology_handset_list = json_object_new_object();
			json_object_object_add(jobj_topology_handset_list, "hs_id", json_object_new_int(hs_id));
			json_object_object_add(jobj_topology_handset_list, "hs_status", json_object_new_int(hs_status));
			json_object_object_add(jobj_topology_handset_list, "hs_type", json_object_new_int(hs_type));
			json_object_array_add(jarr_topology_handset_list, jobj_topology_handset_list);
		}

		json_object_object_add(jobj_topology_base_list, "topology_handset_list", jarr_topology_handset_list);

		json_object_array_add(jarr_topology_base_list, jobj_topology_base_list);
	}

	json_object_object_add(jobj, "topology_base_list", jarr_topology_base_list);

	for(i = RAINIER_HSID_MIN; i <= RAINIER_HSID_MAX; i++)
	{
		jobj_univailable_handset_list = json_object_new_object();
		json_object_object_add(jobj_univailable_handset_list, "hs_id", json_object_new_int(hs_id));
		json_object_object_add(jobj_univailable_handset_list, "hs_status", json_object_new_int(hs_status));
		json_object_object_add(jobj_univailable_handset_list, "hs_type", json_object_new_int(hs_type));
		json_object_array_add(jarr_univailable_handset_list, jobj_univailable_handset_list);
	}

	json_object_object_add(jobj, "univailable_handset_list", jarr_univailable_handset_list);
#endif

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

// "BSC" -> BS (BSC side)
int set_hs_cfg_update(int hs_id, char *hs_cfg)
{
	char cmd[1024] = {0};
	char buf[256] = {0};

	char username[32] = {0};
	char password[32] = {0};

	char device_token[256] = {0};
	char *bsc_id;

	char bs_ip[32] = {0};

	int idx = -1;
	int bs_num;
	int i;
	int ret = -1;

	debug_print("Jason DEBUG %s[%d] hs_cfg: %s\n", __FUNCTION__, __LINE__, hs_cfg);

	// record handset config json string to tmp file
	SYSTEM("echo '%s' > /tmp/hs_cfg", hs_cfg);

	bsc_id = (char*)malloc(13*sizeof(char));
	getBscId(bsc_id, 13);

	bs_num = getBsCount();

	for(i = 0; i < bs_num; i++)
	{
#if 1	// use NMS report BS Status
		//snprintf(cmd, sizeof(cmd), "uci get base-station-list.@base-station[%d].index | tr -d \"\\n\"", i);
		//sys_interact(buf, sizeof(buf), cmd);
		api_get_string_option2(buf, sizeof(buf), "base-station-list.@base-station[%d].index", i);
		idx = atoi(buf);

		sprintf(cmd, "%s/%s_%d", NMS_DEVICE_DIR, NMS_DEVICE_BS, idx);
		if(sysIsFileExisted(cmd) != TRUE)
		{
			debug_print("BS%d offline, continue to next base.\n", idx);
			continue;	// the base offline, continue to next base
		}
#endif

		snprintf(username, sizeof(username), "%s", bsc_id);

		//snprintf(cmd, sizeof(cmd), "uci get base-station-list.@base-station[%d].bs_key | tr -d \"\\n\"", i);
		//sys_interact(password, sizeof(password), cmd);
		api_get_string_option2(password, sizeof(password), "base-station-list.@base-station[%d].bs_key", i);

		//snprintf(cmd, sizeof(cmd), "uci get base-station-list.@base-station[%d].ip | tr -d \"\\n\"", i);
		//sys_interact(bs_ip, sizeof(bs_ip), cmd);
		api_get_string_option2(bs_ip, sizeof(bs_ip), "base-station-list.@base-station[%d].ip", i);

		snprintf(cmd, sizeof(cmd), "curl --connect-timeout 3 --max-time 20 -v -k -X POST \"https://%s:4430/api/sys/login\" -H \"accept: */*\" -H \"Content-Type: application/json\" -d \"{\\\"username\\\":\\\"%s\\\",\\\"password\\\":\\\"%s\\\"}\" | grep token | awk '{for (i=1;i<=NF;i++){if ($i ~/token/) {print $(i+1)}}}' | sed -e 's/\"//g' | tr -d \"\\n\"", bs_ip, username, password);
		debug_print("cmd: %s.", cmd);
		sys_interact_long(device_token, sizeof(device_token), cmd);
		debug_print("device_token: %s.", device_token);

		if(strcmp(device_token, "---") == 0)
		{
			continue;	// login fails, continue to next base
		}

		snprintf(cmd, sizeof(cmd), "curl --connect-timeout 3 --max-time 20 -v -k -X PATCH \"https://%s:4430/api/rainier/hs_config_update/%d\" -H \"accept: */*\" -H \"Authorization: Bearer %s\" -d @/tmp/hs_cfg | awk '{for (i=1;i<=NF;i++){if ($i ~/status_code/) {print $(i+1)}}}' | sed -e 's/\"//g' | tr -d \"\\n\"", bs_ip, hs_id, device_token);
		sys_interact_long(buf, sizeof(buf), cmd);
		debug_print("buf: %s.", buf);

		if(strcmp(buf, "200,") == 0) // OK
		{
			// at least one BS receive cfg_update.
			ret = 0;
		}
	}

	free(bsc_id);

	return ret;
}

int json_get_rainier_handset_list(ResponseEntry *rep, struct json_object *jobj)
{
    ResponseStatus *res = rep->res;
    int i, result;
    int hs_id[RAINIER_HS_NUMBER] = {0};
	int HsRxBsRssi[RAINIER_HS_NUMBER] = {0};
	int BsRxHsRssi[RAINIER_HS_NUMBER] = {0};
	int location[RAINIER_HS_NUMBER] = {0};
	int hs_num = 0;
	int idx;
	int location_in;

	struct json_object *jarr_handset_list = NULL, *jobj_handset_list = NULL;
	jarr_handset_list = json_object_new_array();

    /* get handset rssi data */
	hs_num = getHandsetRssiData(hs_id, HsRxBsRssi, BsRxHsRssi, location);

	/* Get all handset list */
	for(i = RAINIER_HSID_MIN; i <= RAINIER_HSID_MAX; i++)
    {
		jobj_handset_list = json_object_new_object();

		location_in = 0;
		for(idx = 0; idx < hs_num; idx++)
		{
			if(hs_id[idx] == i)
			{
				location_in = location[idx];
			}
		}

        result = json_get_rainier_handset_list_idx_2(rep, jobj_handset_list, i, location_in);
		if(result == API_SUCCESS)
		{
			json_object_array_add(jarr_handset_list, jobj_handset_list);
		}
	}

	json_object_object_add(jobj, "handset_list", jarr_handset_list);

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_get_rainier_handset_list_of_base(ResponseEntry *rep, struct json_object *jobj, int bs_idx)
{
    ResponseStatus *res = rep->res;
    int i, result;
	int hs_id[RAINIER_HS_NUMBER] = {0};
	int HsRxBsRssi[RAINIER_HS_NUMBER] = {0};
	int BsRxHsRssi[RAINIER_HS_NUMBER] = {0};
	int location[RAINIER_HS_NUMBER] = {0};
	int hs_status[RAINIER_HS_NUMBER] = {0};
	int fxo_status[RAINIER_FXO_NUM] = {0};
	int hs_num = 0;
	int idx;
	int location_in;

	struct json_object *jarr_handset_list = NULL, *jobj_handset_list = NULL;
	jarr_handset_list = json_object_new_array();

	/* check bs_idx valid or not */
    if( (bs_idx < RAINIER_BSID_MIN) || (bs_idx > RAINIER_BSID_MAX))
    {
        /* bs_idx is not in range */
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "BASE ID");
    }

	idx = findBsConfigIdxByIndex(bs_idx);
	if(idx == -1) // bs is not in the list
	{
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "BASE ID");
	}

	/* get handset rssi data */
	hs_num = getHandsetRssiData(hs_id, HsRxBsRssi, BsRxHsRssi, location);

	/* get handset status */
	ast_get_hs_fxo_status(hs_status, fxo_status);

    /* Get all handset list located at bs_idx */
	for(i = RAINIER_HSID_MIN; i <= RAINIER_HSID_MAX; i++)
    {
		jobj_handset_list = json_object_new_object();

		location_in = 0;
		for(idx = 0; idx < hs_num; idx++)
		{
			if(hs_id[idx] == i)
			{
				location_in = location[idx];
			}
		}

        result = json_get_rainier_handset_list_of_base_idx(rep, jobj_handset_list, bs_idx, i, location_in, hs_status[i-RAINIER_HSID_MIN]);
		if(result == API_SUCCESS)
		{
			json_object_array_add(jarr_handset_list, jobj_handset_list);
		}
	}

	json_object_object_add(jobj, "handset_list", jarr_handset_list);

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_post_rainier_handset_list_idx(ResponseEntry *rep, char *query_str, int hs_idx)
{
	//int hs_id = 0, location = 0;
	int rainier_group = 0, subscribe_bs = 0;
	bool enable = 0, rainierReg = 0;
	int tmp = 0;
	char *display_name = NULL/*, *model_name = NULL, *serial_number = NULL, *fw_version = NULL*/;
	int rainier_group_ori = 0, subscribe_bs_ori = 0;
	char display_name_ori[64] = {0};
	char cmd[256] = {0};
	char buf[256] = {0};
	struct json_object *jobj = NULL, *jobj_tmp = NULL;
	ResponseStatus *res = rep->res;
	int status;

	if(NULL != query_str)
	{
		if((jobj = jsonTokenerParseFromStack(rep, query_str)))
		{
			//senao_json_object_get_integer(jobj, "hs_id", &hs_id);
			senao_json_object_get_boolean(jobj, "enable", &enable);
			senao_json_object_get_and_create_string(rep, jobj, "display_name", &display_name);
			//senao_json_object_get_integer(jobj, "location", &location);
			senao_json_object_get_integer(jobj, "rainier_group", &rainier_group);
			senao_json_object_get_integer(jobj, "subscribe_bs", &subscribe_bs);
			//senao_json_object_get_and_create_string(rep, jobj, "model_name", &model_name);
			//senao_json_object_get_and_create_string(rep, jobj, "serial_number", &serial_number);
			//senao_json_object_get_and_create_string(rep, jobj, "fw_version", &fw_version);

		}
		else
		{
			RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
		}
	}
	else
	{
		RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
	}

	if(strlen(display_name) > 16)
	{
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "DISPLAY NAME");
	}

	if((hs_idx >= RAINIER_HSID_MIN) && (hs_idx <= RAINIER_HSID_MAX))
    {
        /* check the hs_idx is registered or not */
    	api_get_bool_option2(&tmp, "sip_hs.sip_hs_%d.rainierReg", hs_idx);
		rainierReg = (tmp == 1)?true:false;

		if(rainierReg == true)
		{
			/* backup original data */
			api_get_integer_option2(&rainier_group_ori, "sip_hs.sip_hs_%d.rainier_group", hs_idx);
			api_get_integer_option2(&subscribe_bs_ori, "sip_hs.sip_hs_%d.subscribe_bs", hs_idx);
    		api_get_string_option2(display_name_ori, sizeof(display_name_ori), "sip_hs.sip_hs_%d.display_name", hs_idx);

    		/* set handset list data */
			api_set_bool_option2(enable, "sip_hs.sip_hs_%d.enable", hs_idx);

			sprintf(buf, "sip_hs.sip_hs_%d.display_name", hs_idx);
			if(api_set_rainier_sip_acc_display_name(buf, display_name, sizeof(display_name)))
				RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "DISPLAY NAME");
			
			api_set_integer_option2(rainier_group, "sip_hs.sip_hs_%d.rainier_group", hs_idx);
			api_set_integer_option2(subscribe_bs, "sip_hs.sip_hs_%d.subscribe_bs", hs_idx);
		}
		else
    	{
	        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "HS ID");
	    }
    }
    else
    {
        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "HS ID");
    }


	jobj_tmp = json_object_new_object();
   	//json_object_object_add(jobj_tmp, "enable", json_object_new_boolean(enable));
	json_object_object_add(jobj_tmp, "display_name", json_object_new_string(display_name));
	json_object_object_add(jobj_tmp, "rainier_group", json_object_new_int(rainier_group));
	json_object_object_add(jobj_tmp, "subscribe_bs", json_object_new_int(subscribe_bs));

	// send handset display_name, rainier_group, subscribe_bs to BS
	status = set_hs_cfg_update(hs_idx, (char *)json_object_to_json_string(jobj_tmp));

	json_object_put(jobj_tmp);

	if(status == -1)
	{
		/* restore original data */
		sprintf(buf, "sip_hs.sip_hs_%d.display_name", hs_idx);
		api_set_rainier_sip_acc_display_name(buf, display_name_ori, sizeof(display_name_ori));
		api_set_integer_option2(rainier_group_ori, "sip_hs.sip_hs_%d.rainier_group", hs_idx);
		api_set_integer_option2(subscribe_bs_ori, "sip_hs.sip_hs_%d.subscribe_bs", hs_idx);

		//snprintf(cmd, sizeof(cmd), "uci set sip_hs.sip_hs_%d.setting_update_status=%d", hs_idx, HS_UPDATE_FAIL_BS_OFFLINE);	// update fail
		//sys_interact(buf, sizeof(buf), cmd);
		api_set_integer_option2(HS_UPDATE_FAIL_BS_OFFLINE, "sip_hs.sip_hs_%d.setting_update_status", hs_idx);	// update fail

		//snprintf(cmd, sizeof(cmd), "uci commit sip_hs");
		//sys_interact(buf, sizeof(buf), cmd);
		api_commit_option("sip_hs");

		RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "Unable to send handset config to Base");
	}

	/* save backup original data to uci config */
#if 1
	api_set_string_option2(display_name_ori, sizeof(display_name_ori), "sip_hs.sip_hs_%d.display_name_ori", hs_idx);
	api_set_integer_option2(rainier_group_ori, "sip_hs.sip_hs_%d.rainier_group_ori", hs_idx);
	api_set_integer_option2(subscribe_bs_ori, "sip_hs.sip_hs_%d.subscribe_bs_ori", hs_idx);
	api_set_integer_option2(HS_UPDATE_PROCESSING, "sip_hs.sip_hs_%d.setting_update_status", hs_idx);	// update processing

	// bsc_monitor wait for if BS timeout, revert display_name, rainier_group, subscribe_bs data
	api_set_integer_option("sip_hs.bsc_check_flag.check_hs_cfg", 1);	// bsc_monitor will check BS timeout
#else
	snprintf(cmd, sizeof(cmd), "uci set sip_hs.sip_hs_%d.display_name_ori='%s'", hs_idx, display_name_ori);
	sys_interact(buf, sizeof(buf), cmd);

	snprintf(cmd, sizeof(cmd), "uci set sip_hs.sip_hs_%d.rainier_group_ori=%d", hs_idx, rainier_group_ori);
	sys_interact(buf, sizeof(buf), cmd);

	snprintf(cmd, sizeof(cmd), "uci set sip_hs.sip_hs_%d.subscribe_bs_ori=%d", hs_idx, subscribe_bs_ori);
	sys_interact(buf, sizeof(buf), cmd);

	snprintf(cmd, sizeof(cmd), "uci set sip_hs.sip_hs_%d.setting_update_status=%d", hs_idx, HS_UPDATE_PROCESSING);	// update processing
	sys_interact(buf, sizeof(buf), cmd);


	// bsc_monitor wait for if BS timeout, revert display_name, rainier_group, subscribe_bs data
	snprintf(cmd, sizeof(cmd), "uci set sip_hs.bsc_check_flag.check_hs_cfg=1");	// bsc_monitor will check BS timeout
	sys_interact(buf, sizeof(buf), cmd);
#endif

	//api_commit_option("rainier");
	api_commit_option("sip_hs");

	// reload asterisk
	SYSTEM("astgen");
	SYSTEM("asterisk -rx 'core reload'");

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_patch_rainier_handset_list_idx(ResponseEntry *rep, char *query_str, int hs_idx)
{
	//int hs_id = 0, location = 0;
	int rainier_group = 0, subscribe_bs = 0;
	bool enable = 0, rainierReg = 0;
	int tmp = 0;
	char *display_name = NULL/*, *model_name = NULL, *serial_number = NULL, *fw_version = NULL*/;
	int rainier_group_ori = 0, subscribe_bs_ori = 0;
	char display_name_ori[64] = {0};
	char cmd[256] = {0};
	char buf[256] = {0};
	struct json_object *jobj = NULL, *jobj_tmp = NULL;
	ResponseStatus *res = rep->res;
	int status;
	int has_enable = 0, has_display_name = 0, has_rainier_group = 0, has_subscribe_bs = 0;

	if(NULL != query_str)
	{
		if((jobj = jsonTokenerParseFromStack(rep, query_str)))
		{
			if (json_object_object_get(jobj, "enable") != NULL)
			{
				has_enable = 1;
				senao_json_object_get_boolean(jobj, "enable", &enable);
			}
			if (json_object_object_get(jobj, "display_name") != NULL)
			{
				has_display_name = 1;
				senao_json_object_get_and_create_string(rep, jobj, "display_name", &display_name);
			}
			if (json_object_object_get(jobj, "rainier_group") != NULL)
			{
				has_rainier_group = 1;
				senao_json_object_get_integer(jobj, "rainier_group", &rainier_group);
			}
			if (json_object_object_get(jobj, "subscribe_bs") != NULL)
			{
				has_subscribe_bs = 1;
				senao_json_object_get_integer(jobj, "subscribe_bs", &subscribe_bs);
			}
		}
		else
		{
			RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
		}
	}
	else
	{
		RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
	}

	if(strlen(display_name) > 16)
	{
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "DISPLAY NAME");
	}

	if((hs_idx >= RAINIER_HSID_MIN) && (hs_idx <= RAINIER_HSID_MAX))
    {
        /* check the hs_idx is registered or not */
    	api_get_bool_option2(&tmp, "sip_hs.sip_hs_%d.rainierReg", hs_idx);
		rainierReg = (tmp == 1)?true:false;

		if(rainierReg == true)
		{
			/* backup original data */
			api_get_integer_option2(&rainier_group_ori, "sip_hs.sip_hs_%d.rainier_group", hs_idx);
			api_get_integer_option2(&subscribe_bs_ori, "sip_hs.sip_hs_%d.subscribe_bs", hs_idx);
    		api_get_string_option2(display_name_ori, sizeof(display_name_ori), "sip_hs.sip_hs_%d.display_name", hs_idx);

    		/* set handset list data */
			if(has_enable)
			{
				api_set_bool_option2(enable, "sip_hs.sip_hs_%d.enable", hs_idx);
			}
			if(has_display_name)
			{
				sprintf(buf, "sip_hs.sip_hs_%d.display_name", hs_idx);
				if(api_set_rainier_sip_acc_display_name(buf, display_name, sizeof(display_name)))
					RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "DISPLAY NAME");
			}
			if(has_rainier_group)
			{
				api_set_integer_option2(rainier_group, "sip_hs.sip_hs_%d.rainier_group", hs_idx);
			}
			if(has_subscribe_bs)
			{
				api_set_integer_option2(subscribe_bs, "sip_hs.sip_hs_%d.subscribe_bs", hs_idx);
			}
		}
		else
    	{
	        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "HS ID");
	    }
    }
    else
    {
        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "HS ID");
    }


	// if handset display_name, rainier_group, subscribe_bs modified, send data to BS
	if(has_display_name || has_rainier_group || has_subscribe_bs)
	{
		jobj_tmp = json_object_new_object();
   		//json_object_object_add(jobj_tmp, "enable", json_object_new_boolean(enable));
		if(has_display_name)
		{
			json_object_object_add(jobj_tmp, "display_name", json_object_new_string(display_name));
		}
		if(has_rainier_group)
		{
			json_object_object_add(jobj_tmp, "rainier_group", json_object_new_int(rainier_group));
		}
		if(has_subscribe_bs)
		{
			json_object_object_add(jobj_tmp, "subscribe_bs", json_object_new_int(subscribe_bs));
		}

		status = set_hs_cfg_update(hs_idx, (char *)json_object_to_json_string(jobj_tmp));

		json_object_put(jobj_tmp);

		if(status == -1)
		{
			/* restore original data */
			sprintf(buf, "sip_hs.sip_hs_%d.display_name", hs_idx);
			api_set_rainier_sip_acc_display_name(buf, display_name_ori, sizeof(display_name_ori));
			api_set_integer_option2(rainier_group_ori, "sip_hs.sip_hs_%d.rainier_group", hs_idx);
			api_set_integer_option2(subscribe_bs_ori, "sip_hs.sip_hs_%d.subscribe_bs", hs_idx);

			//snprintf(cmd, sizeof(cmd), "uci set sip_hs.sip_hs_%d.setting_update_status=%d", hs_idx, HS_UPDATE_FAIL_BS_OFFLINE);	// update fail
			//sys_interact(buf, sizeof(buf), cmd);
			api_set_integer_option2(HS_UPDATE_FAIL_BS_OFFLINE, "sip_hs.sip_hs_%d.setting_update_status", hs_idx);	// update fail

			//snprintf(cmd, sizeof(cmd), "uci commit sip_hs");
			//sys_interact(buf, sizeof(buf), cmd);
			api_commit_option("sip_hs");

			RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "Unable to send handset config to Base");
		}

		/* save backup original data to uci config */
#if 1
		api_set_string_option2(display_name_ori, sizeof(display_name_ori), "sip_hs.sip_hs_%d.display_name_ori", hs_idx);
		api_set_integer_option2(rainier_group_ori, "sip_hs.sip_hs_%d.rainier_group_ori", hs_idx);
		api_set_integer_option2(subscribe_bs_ori, "sip_hs.sip_hs_%d.subscribe_bs_ori", hs_idx);
		api_set_integer_option2(HS_UPDATE_PROCESSING, "sip_hs.sip_hs_%d.setting_update_status", hs_idx);	// update processing

		// bsc_monitor wait for if BS timeout, revert display_name, rainier_group, subscribe_bs data
		api_set_integer_option("sip_hs.bsc_check_flag.check_hs_cfg", 1);	// bsc_monitor will check BS timeout
#else
		snprintf(cmd, sizeof(cmd), "uci set sip_hs.sip_hs_%d.display_name_ori='%s'", hs_idx, display_name_ori);
		sys_interact(buf, sizeof(buf), cmd);

		snprintf(cmd, sizeof(cmd), "uci set sip_hs.sip_hs_%d.rainier_group_ori=%d", hs_idx, rainier_group_ori);
		sys_interact(buf, sizeof(buf), cmd);

		snprintf(cmd, sizeof(cmd), "uci set sip_hs.sip_hs_%d.subscribe_bs_ori=%d", hs_idx, subscribe_bs_ori);
		sys_interact(buf, sizeof(buf), cmd);

		snprintf(cmd, sizeof(cmd), "uci set sip_hs.sip_hs_%d.setting_update_status=%d", hs_idx, HS_UPDATE_PROCESSING);	// update processing
		sys_interact(buf, sizeof(buf), cmd);


		// bsc_monitor wait for if BS timeout, revert display_name, rainier_group, subscribe_bs data
		snprintf(cmd, sizeof(cmd), "uci set sip_hs.bsc_check_flag.check_hs_cfg=1");	// bsc_monitor will check BS timeout
		sys_interact(buf, sizeof(buf), cmd);
#endif
	}
	else
	{
		//snprintf(cmd, sizeof(cmd), "uci set sip_hs.sip_hs_%d.setting_update_status=%d", hs_idx, HS_UPDATE_SUCCESS);	// update success
		//sys_interact(buf, sizeof(buf), cmd);
		api_set_integer_option2(HS_UPDATE_SUCCESS, "sip_hs.sip_hs_%d.setting_update_status", hs_idx);	// update success
	}
	
	
	//api_commit_option("rainier");
	api_commit_option("sip_hs");

	// reload asterisk
	SYSTEM("astgen");
	SYSTEM("asterisk -rx 'core reload'");

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_get_rainier_handset_list_idx(ResponseEntry *rep, struct json_object *jobj, int hs_idx)
{
	int result;
    int hs_id[RAINIER_HS_NUMBER] = {0};
	int HsRxBsRssi[RAINIER_HS_NUMBER] = {0};
	int BsRxHsRssi[RAINIER_HS_NUMBER] = {0};
	int location[RAINIER_HS_NUMBER] = {0};
	int hs_num = 0;
	int idx;
	int location_in;

    /* get handset rssi data */
	hs_num = getHandsetRssiData(hs_id, HsRxBsRssi, BsRxHsRssi, location);

	/* Get all handset list */
	location_in = 0;
	for(idx = 0; idx < hs_num; idx++)
	{
		if(hs_id[idx] == hs_idx)
		{
			location_in = location[idx];
		}
	}

	result = json_get_rainier_handset_list_idx_2(rep, jobj, hs_idx, location_in);

	return result;
}

int json_get_rainier_handset_list_idx_2(ResponseEntry *rep, struct json_object *jobj, int hs_idx, int location_in)
{
	int hs_id = 0, location = 0, rainier_group = 0, subscribe_bs = 0;
	int setting_update_status = HS_UPDATE_IDLE;
	int tmp = 0;
	bool enable = 0, rainierReg = 0;
	char display_name[64] = {0}, model_name[32] = {0}, serial_number[32] = {0}, fw_version[32] = {0}, hw_version[32] = {0};
	char fw_version_tmp[32] = {0};
	ResponseStatus *res = rep->res;

#if 0
	char cmd[256] = {0};
	char buf[256] = {0};

	if((hs_idx >= RAINIER_HSID_MIN) && (hs_idx <= RAINIER_HSID_MAX))
    {
		/* check the hs_idx is registered or not */
		snprintf(cmd, sizeof(cmd), "uci get sip_hs.sip_hs_%d.rainierReg | tr -d \"\\n\"", hs_idx);
		sys_interact(buf, sizeof(buf), cmd);
		tmp = atoi(buf);
		rainierReg = (tmp == 1)?true:false;

		if(rainierReg == true)
		{
    		/* get handset list data */
			//snprintf(cmd, sizeof(cmd), "uci get sip_hs.sip_hs_%d.location | tr -d \"\\n\"", hs_idx);
			//sys_interact(buf, sizeof(buf), cmd);
			//location = atoi(buf);
			location = location_in;

			snprintf(cmd, sizeof(cmd), "uci get sip_hs.sip_hs_%d.rainier_group | tr -d \"\\n\"", hs_idx);
			sys_interact(buf, sizeof(buf), cmd);
			rainier_group = atoi(buf);

			snprintf(cmd, sizeof(cmd), "uci get sip_hs.sip_hs_%d.subscribe_bs | tr -d \"\\n\"", hs_idx);
			sys_interact(buf, sizeof(buf), cmd);
			subscribe_bs = atoi(buf);

			snprintf(cmd, sizeof(cmd), "uci get sip_hs.sip_hs_%d.enable | tr -d \"\\n\"", hs_idx);
			sys_interact(buf, sizeof(buf), cmd);
			tmp = atoi(buf);
			enable = (tmp == 1)?true:false;

    		snprintf(cmd, sizeof(cmd), "uci get sip_hs.sip_hs_%d.display_name | tr -d \"\\n\"", hs_idx);
			sys_interact(buf, sizeof(buf), cmd);
			strncpy(display_name, buf, sizeof(display_name));
		}
		else
    	{
	        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "HS ID");
	    }
    }
#else
    if((hs_idx >= RAINIER_HSID_MIN) && (hs_idx <= RAINIER_HSID_MAX))
    {
		/* check the hs_idx is registered or not */
    	api_get_bool_option2(&tmp, "sip_hs.sip_hs_%d.rainierReg", hs_idx);
		rainierReg = (tmp == 1)?true:false;

		if(rainierReg == true)
		{
    		/* get handset list data */
    		//api_get_integer_option2(&location, "sip_hs.sip_hs_%d.location", hs_idx);
			location = location_in;

    		api_get_integer_option2(&rainier_group, "sip_hs.sip_hs_%d.rainier_group", hs_idx);
			api_get_integer_option2(&subscribe_bs, "sip_hs.sip_hs_%d.subscribe_bs", hs_idx);
			api_get_integer_option2(&setting_update_status, "sip_hs.sip_hs_%d.setting_update_status", hs_idx);

    		api_get_bool_option2(&tmp, "sip_hs.sip_hs_%d.enable", hs_idx);
			enable = (tmp == 1)?true:false;

    		api_get_string_option2(display_name, sizeof(display_name), "sip_hs.sip_hs_%d.display_name", hs_idx);
			//api_get_string_option2(model_name, sizeof(model_name), "sip_hs.sip_hs_%d.model_name", hs_idx);
			strcpy(model_name, "DuraFon Roam");
			api_get_string_option2(serial_number, sizeof(serial_number), "sip_hs.sip_hs_%d.serial_number", hs_idx);
			api_get_string_option2(fw_version, sizeof(fw_version), "sip_hs.sip_hs_%d.fw_version", hs_idx);
			// add version prefix with letter 'v'
			if((fw_version[0] >= '0') && (fw_version[0] <= '9'))
			{
				strncpy(fw_version_tmp, fw_version, sizeof(fw_version_tmp));
				snprintf(fw_version, sizeof(fw_version), "v%s", fw_version_tmp);
			}
			api_get_string_option2(hw_version, sizeof(hw_version), "sip_hs.sip_hs_%d.hw_version", hs_idx);
		}
		else
    	{
	        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "HS ID");
	    }
    }
#endif
    else
    {
        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "HS ID");
    }
	
	json_object_object_add(jobj, "hs_id", json_object_new_int(hs_idx));
	json_object_object_add(jobj, "enable", json_object_new_boolean(enable));
	json_object_object_add(jobj, "display_name", json_object_new_string(display_name));
	json_object_object_add(jobj, "location", json_object_new_int(location));
	json_object_object_add(jobj, "rainier_group", json_object_new_int(rainier_group));
	json_object_object_add(jobj, "subscribe_bs", json_object_new_int(subscribe_bs));
	json_object_object_add(jobj, "model_name", json_object_new_string(model_name));
	json_object_object_add(jobj, "serial_number", json_object_new_string(serial_number));
	json_object_object_add(jobj, "fw_version", json_object_new_string(fw_version));
	json_object_object_add(jobj, "hw_version", json_object_new_string(hw_version));
	json_object_object_add(jobj, "setting_update_status", json_object_new_int(setting_update_status));
	
	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_get_rainier_handset_list_of_base_idx(ResponseEntry *rep, struct json_object *jobj, int bs_idx, int hs_idx, int location_in, int hs_status_in)
{
	int hs_id = 0, location = 0, rainier_group = 0, hs_status = 0;
	int tmp = 0;
	bool enable = 0, rainierReg = 0;
	char display_name[64] = {0};
	ResponseStatus *res = rep->res;

    if((hs_idx >= RAINIER_HSID_MIN) && (hs_idx <= RAINIER_HSID_MAX))
    {
		/* check the hs_idx is registered or not */
    	api_get_bool_option2(&tmp, "sip_hs.sip_hs_%d.rainierReg", hs_idx);
		rainierReg = (tmp == 1)?true:false;

		if(rainierReg == true)
		{
    		/* get handset list data */
    		//api_get_integer_option2(&location, "sip_hs.sip_hs_%d.location", hs_idx);
			location = location_in;

			if(location != bs_idx)
			{
				RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "HS ID");
			}

    		api_get_integer_option2(&rainier_group, "sip_hs.sip_hs_%d.rainier_group", hs_idx);

    		api_get_bool_option2(&tmp, "sip_hs.sip_hs_%d.enable", hs_idx);
			enable = (tmp == 1)?true:false;

    		api_get_string_option2(display_name, sizeof(display_name), "sip_hs.sip_hs_%d.display_name", hs_idx);

			hs_status = hs_status_in;
		}
		else
    	{
	        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "HS ID");
	    }
    }
    else
    {
        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "HS ID");
    }
	
	json_object_object_add(jobj, "hs_id", json_object_new_int(hs_idx));
	json_object_object_add(jobj, "enable", json_object_new_boolean(enable));
	json_object_object_add(jobj, "display_name", json_object_new_string(display_name));
	json_object_object_add(jobj, "location", json_object_new_int(location));
	json_object_object_add(jobj, "rainier_group", json_object_new_int(rainier_group));
	json_object_object_add(jobj, "hs_status", json_object_new_int(hs_status));
	
	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

/* Base report result to BSC */
int json_post_rainier_hs_config_result(ResponseEntry *rep, char *query_str)
{
    int hs_id = 0;
	int result = 0;
	int setting_update_status = HS_UPDATE_IDLE;
	bool rainierReg = 0;
	int tmp = 0;
	int rainier_group_ori = 0, subscribe_bs_ori = 0;
	char display_name_ori[64] = {0};
    struct json_object *jobj = NULL;
    ResponseStatus *res = rep->res;
	char cmd[256] = {0};
	char buf[256] = {0};
	

    /* base return handset config update result. */
    if(NULL != query_str)
    {
        if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
            /* get which hsid */
            senao_json_object_get_integer(jobj, "hs_id", &hs_id);
            /* get result */
            senao_json_object_get_integer(jobj, "result", &result);

			/* check hs_id is in range or not (10-99) */
			if((hs_id >= RAINIER_HSID_MIN) && (hs_id <= RAINIER_HSID_MAX))
			{
				/* check the hs_idx is registered or not */
    			api_get_bool_option2(&tmp, "sip_hs.sip_hs_%d.rainierReg", hs_id);
				rainierReg = (tmp == 1)?true:false;

				/* check the hs_id setting_update_status is processing or not */
				api_get_integer_option2(&setting_update_status, "sip_hs.sip_hs_%d.setting_update_status", hs_id);

				if((rainierReg == true) && (setting_update_status == HS_UPDATE_PROCESSING))
				{
					/* action */
					if(result == HS_RESULT_SUCCESS)
					{
						//snprintf(cmd, sizeof(cmd), "uci set sip_hs.sip_hs_%d.setting_update_status=%d", hs_id, HS_UPDATE_SUCCESS);	// update success
						//sys_interact(buf, sizeof(buf), cmd);
						api_set_integer_option2(HS_UPDATE_SUCCESS, "sip_hs.sip_hs_%d.setting_update_status", hs_id);	// update success

						//snprintf(cmd, sizeof(cmd), "uci commit sip_hs");
						//sys_interact(buf, sizeof(buf), cmd);
						api_commit_option("sip_hs");
					}
					else if( (result >= HS_RESULT_NOT_FOUND) && (result <= HS_RESULT_FAIL2UPDATE) )
					{
						/* get backup original data */
						api_get_integer_option2(&rainier_group_ori, "sip_hs.sip_hs_%d.rainier_group_ori", hs_id);
						api_get_integer_option2(&subscribe_bs_ori, "sip_hs.sip_hs_%d.subscribe_bs_ori", hs_id);
		    			api_get_string_option2(display_name_ori, sizeof(display_name_ori), "sip_hs.sip_hs_%d.display_name_ori", hs_id);

						/* restore original data */
						sprintf(buf, "sip_hs.sip_hs_%d.display_name", hs_id);
						api_set_rainier_sip_acc_display_name(buf, display_name_ori, sizeof(display_name_ori));
						api_set_integer_option2(rainier_group_ori, "sip_hs.sip_hs_%d.rainier_group", hs_id);
						api_set_integer_option2(subscribe_bs_ori, "sip_hs.sip_hs_%d.subscribe_bs", hs_id);

						if(result == HS_RESULT_NOT_FOUND)
						{
							//snprintf(cmd, sizeof(cmd), "uci set sip_hs.sip_hs_%d.setting_update_status=%d", hs_id, HS_UPDATE_FAIL_HS_NOT_FOUND);	// update fail
							api_set_integer_option2(HS_UPDATE_FAIL_HS_NOT_FOUND, "sip_hs.sip_hs_%d.setting_update_status", hs_id);	// update fail
						}
						else if(result == HS_RESULT_BUSY)
						{
							//snprintf(cmd, sizeof(cmd), "uci set sip_hs.sip_hs_%d.setting_update_status=%d", hs_id, HS_UPDATE_FAIL_HS_BUSY);	// update fail
							api_set_integer_option2(HS_UPDATE_FAIL_HS_BUSY, "sip_hs.sip_hs_%d.setting_update_status", hs_id);	// update fail
						}
						else if(result == HS_RESULT_LINK_LOSS)
						{
							//snprintf(cmd, sizeof(cmd), "uci set sip_hs.sip_hs_%d.setting_update_status=%d", hs_id, HS_UPDATE_FAIL_HS_LINK_LOSS);	// update fail
							api_set_integer_option2(HS_UPDATE_FAIL_HS_LINK_LOSS, "sip_hs.sip_hs_%d.setting_update_status", hs_id);	// update fail
						}
						else if(result == HS_RESULT_FAIL2UPDATE)
						{
							//snprintf(cmd, sizeof(cmd), "uci set sip_hs.sip_hs_%d.setting_update_status=%d", hs_id, HS_UPDATE_FAIL_HS_FAIL2UPDATE);	// update fail
							api_set_integer_option2(HS_UPDATE_FAIL_HS_FAIL2UPDATE, "sip_hs.sip_hs_%d.setting_update_status", hs_id);	// update fail
						}
						sys_interact(buf, sizeof(buf), cmd);

						//snprintf(cmd, sizeof(cmd), "uci commit sip_hs");
						//sys_interact(buf, sizeof(buf), cmd);
						api_commit_option("sip_hs");

						// reload asterisk
						SYSTEM("astgen");
						SYSTEM("asterisk -rx 'core reload'");
	            	}
					else
					{
						RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "RESULT");
					}
				}
				else
		    	{
			        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "HS ID");
			    }
			}
			else
			{
				RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "HS ID");
			}
			
        }
        else
        {
            RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
        }
    }
    else
    {
        RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
    }
    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_get_rainier_base_status(ResponseEntry *rep, struct json_object *jobj)
{
    int i, j;
    char idx_buf[6];
	//int active = 0, status = 0, voip_status = 0, fxo_status = 0;
	int status = 0;
	int hs_status[RAINIER_HS_NUMBER] = {0};
	int fxo_status[RAINIER_FXO_NUM] = {0};
	int voip_status[RAINIER_FXO_NUM] = {0};
	int idx;
	char cmd[256] = {0};
	//char bs_status[32] = {0};
	char bs_name[32] = {0};
	int bs_call_inuse[RAINIER_BASE_NUM];
    struct json_object *jobj_info = NULL;
	ResponseStatus *res = rep->res;
    
	
	/* get fxo status */
	ast_get_hs_fxo_status(hs_status, fxo_status);

	/* get voip status */
	ast_get_bs_voip_status(voip_status);

	/* get base call inuse */
	memset(bs_call_inuse, 0, sizeof(bs_call_inuse));
	getBaseCallInUseData(bs_call_inuse);

	/* Get all bases information */
    for(i = RAINIER_BSID_MIN; i <= RAINIER_BSID_MAX; i++)
    {
        sprintf(idx_buf, "%d", i);/* index RAINIER_BSID_MIN ~  RAINIER_BSID_MAX */
        jobj_info = newObjectFromStack(rep);
        
		idx = findBsConfigIdxByIndex(i);
		if(idx != -1) // bs is in the list
		{
#if 1	// use NMS report BS Status
			if(bs_call_inuse[i-RAINIER_BSID_MIN])
			{
				voip_status[i-RAINIER_BSID_MIN] = 2;	// in-used
			}

			sprintf(cmd, "%s/%s_%d", NMS_DEVICE_DIR, NMS_DEVICE_BS, i);
			if(sysIsFileExisted(cmd) == TRUE)
			{
				status = 1;
			}
			else
			{
				status = 0;
				voip_status[i-RAINIER_BSID_MIN] = 0;
				fxo_status[i-RAINIER_BSID_MIN] = 0;
			}
#else
			//snprintf(cmd, sizeof(cmd), "uci get base-station-list.@base-station[%d].status | tr -d \"\\n\"", idx);
			//sys_interact(bs_status, sizeof(bs_status), cmd);
			api_get_string_option2(bs_status, sizeof(bs_status), "base-station-list.@base-station[%d].status", idx);
			if(strcmp(bs_status, "online") == 0)
			{
				status = 1;
			}
			else //if(strcmp(bs_status, "offline") == 0)
			{
				status = 0;
				voip_status[i-RAINIER_BSID_MIN] = 0;
				fxo_status[i-RAINIER_BSID_MIN] = 0;
			}
#endif
			
			json_object_object_add(jobj_info, "active", json_object_new_int(1));
    		json_object_object_add(jobj_info, "status", json_object_new_int(status));
    		json_object_object_add(jobj_info, "voip_status", json_object_new_int(voip_status[i-RAINIER_BSID_MIN]));
			json_object_object_add(jobj_info, "fxo_status", json_object_new_int(fxo_status[i-RAINIER_BSID_MIN]));
			api_get_string_option2(bs_name, sizeof(bs_name), "base-station-list.@base-station[%d].name", idx);
			json_object_object_add(jobj_info, "bs_name", json_object_new_string(bs_name));
		}
		else
		{	// bs is not in the list
			json_object_object_add(jobj_info, "active", json_object_new_int(0));		// 0: disable
    		json_object_object_add(jobj_info, "status", json_object_new_int(0));		// 0: off line
    		json_object_object_add(jobj_info, "voip_status", json_object_new_int(0));	// 0: n/a
			json_object_object_add(jobj_info, "fxo_status", json_object_new_int(0));	// 0: n/a
			json_object_object_add(jobj_info, "bs_name", json_object_new_string(""));
		}
        
		json_object_object_add(jobj, idx_buf, jobj_info);
    }

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_get_rainier_base_call_status(ResponseEntry *rep, struct json_object *jobj)
{
	char bs_name[32] = {0};
	char display_name[64] = {0};
	bs_call_info_t  bs_call_info[RAINIER_BASE_NUM];
	int i, j, k;
	int idx;
	ResponseStatus *res = rep->res;

	struct json_object *jarr_base_list = NULL, *jobj_base_list = NULL;
	jarr_base_list = json_object_new_array();
	
	struct json_object *jarr_call_list = NULL, *jobj_call_list = NULL;
	//jarr_call_list = json_object_new_array();

	struct json_object *jarr_call_info = NULL, *jobj_call_info = NULL;
	//jarr_call_info = json_object_new_array();


	/* get base call info */
	memset(bs_call_info, 0, sizeof(bs_call_info));
	getBaseCallInfoData(bs_call_info);

	for(i = RAINIER_BSID_MIN; i <= RAINIER_BSID_MAX; i++)
	{
		/* check the base is registered to BSC or not */
		idx = findBsConfigIdxByIndex(i);
		if(idx != -1) // bs is in the list
		{
			jobj_base_list = json_object_new_object();
			json_object_object_add(jobj_base_list, "bs_id", json_object_new_int(i));

			api_get_string_option2(bs_name, sizeof(bs_name), "base-station-list.@base-station[%d].name", idx);
			json_object_object_add(jobj_base_list, "bs_name", json_object_new_string(bs_name));

			jarr_call_list = json_object_new_array();
			for(j = 0; j < 4; j++)
			{
				if(bs_call_info[i-RAINIER_BSID_MIN].slot[j].slot_in_used)
				{
					jobj_call_list = json_object_new_object();
					json_object_object_add(jobj_call_list, "hs_id", json_object_new_int(bs_call_info[i-RAINIER_BSID_MIN].slot[j].hsId));
					api_get_string_option2(display_name, sizeof(display_name), "sip_hs.sip_hs_%d.display_name", bs_call_info[i-RAINIER_BSID_MIN].slot[j].hsId);
					json_object_object_add(jobj_call_list, "display_name", json_object_new_string(display_name));

					jarr_call_info = json_object_new_array();
					for(k = 0; k < 2; k++)
					{
						if(bs_call_info[i-RAINIER_BSID_MIN].slot[j].call[k].call_in_used)
						{
							jobj_call_info = json_object_new_object();
							json_object_object_add(jobj_call_info, "call_type", json_object_new_int(bs_call_info[i-RAINIER_BSID_MIN].slot[j].call[k].type));
							json_object_object_add(jobj_call_info, "call_status", json_object_new_int(bs_call_info[i-RAINIER_BSID_MIN].slot[j].call[k].state));
							json_object_object_add(jobj_call_info, "call_dir", json_object_new_int(bs_call_info[i-RAINIER_BSID_MIN].slot[j].call[k].dir));
							json_object_object_add(jobj_call_info, "call_num", json_object_new_string(bs_call_info[i-RAINIER_BSID_MIN].slot[j].call[k].num));
							json_object_object_add(jobj_call_info, "call_name", json_object_new_string(bs_call_info[i-RAINIER_BSID_MIN].slot[j].call[k].name));
							json_object_array_add(jarr_call_info, jobj_call_info);
						}
					}
					json_object_object_add(jobj_call_list, "call_info", jarr_call_info);

					json_object_array_add(jarr_call_list, jobj_call_list);
				}
			}
			json_object_object_add(jobj_base_list, "call_list", jarr_call_list);

			json_object_array_add(jarr_base_list, jobj_base_list);
		}
	}

	json_object_object_add(jobj, "base_list", jarr_base_list);

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_get_rainier_handset_status(ResponseEntry *rep, struct json_object *jobj)
{
	//int hs_id = 0, hs_status = 0, hs_type = 0;
	int hs_id[RAINIER_HS_NUMBER] = {0};
	int HsRxBsRssi[RAINIER_HS_NUMBER] = {0};
	int BsRxHsRssi[RAINIER_HS_NUMBER] = {0};
	int location[RAINIER_HS_NUMBER] = {0};
#if 1
	hs_call_info_t  hs_call_info[RAINIER_HS_NUMBER];
#else
	int hs_status[RAINIER_HS_NUMBER] = {0};
	int fxo_status[RAINIER_FXO_NUM] = {0};
#endif
	int hs_type[RAINIER_HS_NUMBER] = {0};
	char display_name[64] = {0};
	int hs_num = 0;
	int tmp = 0;
	bool enable = 0, rainierReg = 0;
	int i, j;
	int idx;
	int location_in;
	ResponseStatus *res = rep->res;

	struct json_object *jarr_handset_list = NULL, *jobj_handset_list = NULL;
	jarr_handset_list = json_object_new_array();

	struct json_object *jarr_call_info = NULL, *jobj_call_info = NULL;
	//jarr_call_info = json_object_new_array();


    /* get handset rssi data */
	hs_num = getHandsetRssiData(hs_id, HsRxBsRssi, BsRxHsRssi, location);
	
#if 1	// get handset status and call info from nms
	/* get handset call info */
	memset(hs_call_info, 0, sizeof(hs_call_info));
	getHandsetCallInfoData(hs_call_info);

	for(i = RAINIER_HSID_MIN; i <= RAINIER_HSID_MAX; i++)
	{
		/* check the hs_idx is registered or not */
    	api_get_bool_option2(&tmp, "sip_hs.sip_hs_%d.rainierReg", i);
		rainierReg = (tmp == 1)?true:false;

		if(rainierReg == true)
		{
    		jobj_handset_list = json_object_new_object();
			json_object_object_add(jobj_handset_list, "hs_id", json_object_new_int(i));

			location_in = 0;
			for(idx = 0; idx < hs_num; idx++)
			{
				if(hs_id[idx] == i)
				{
					location_in = location[idx];
				}
			}
			if( (location_in != 0) || (hs_call_info[i-RAINIER_HSID_MIN].status) )
			{
				json_object_object_add(jobj_handset_list, "hs_status", json_object_new_int(hs_call_info[i-RAINIER_HSID_MIN].status));
			}
			else	/* if RSSI data is un-available, hs_status is un-available */
			{
				json_object_object_add(jobj_handset_list, "hs_status", json_object_new_int(3));	// 3: Un-available
			}
			
			json_object_object_add(jobj_handset_list, "hs_type", json_object_new_int(hs_type[i-RAINIER_HSID_MIN]));
			api_get_string_option2(display_name, sizeof(display_name), "sip_hs.sip_hs_%d.display_name", i);
			json_object_object_add(jobj_handset_list, "display_name", json_object_new_string(display_name));

			jarr_call_info = json_object_new_array();
			for(j = 0; j < 2; j++)
			{
				if(hs_call_info[i-RAINIER_HSID_MIN].call[j].call_in_used)
				{
					jobj_call_info = json_object_new_object();
					json_object_object_add(jobj_call_info, "call_type", json_object_new_int(hs_call_info[i-RAINIER_HSID_MIN].call[j].type));
					json_object_object_add(jobj_call_info, "call_status", json_object_new_int(hs_call_info[i-RAINIER_HSID_MIN].call[j].state));
					json_object_object_add(jobj_call_info, "call_dir", json_object_new_int(hs_call_info[i-RAINIER_HSID_MIN].call[j].dir));
					json_object_object_add(jobj_call_info, "call_num", json_object_new_string(hs_call_info[i-RAINIER_HSID_MIN].call[j].num));
					json_object_object_add(jobj_call_info, "call_name", json_object_new_string(hs_call_info[i-RAINIER_HSID_MIN].call[j].name));
					json_object_array_add(jarr_call_info, jobj_call_info);
				}
			}
			json_object_object_add(jobj_handset_list, "call_info", jarr_call_info);

			json_object_array_add(jarr_handset_list, jobj_handset_list);

			//debug_print("HS%d status:%d type:%d\n", i, hs_status[i-RAINIER_HSID_MIN], hs_type[i-RAINIER_HSID_MIN]);
		}
	}

#else	// get handset status from ast
	/* get handset status */
	ast_get_hs_fxo_status(hs_status, fxo_status);

#if 0	/* handset num use RSSI data */
	for(i = 0; i < hs_num; i++)
	{
		jobj_handset_list = json_object_new_object();
		json_object_object_add(jobj_handset_list, "hs_id", json_object_new_int(hs_id[i]));
		if(location[i] != 0)
		{
			json_object_object_add(jobj_handset_list, "hs_status", json_object_new_int(hs_status[hs_id[i]-RAINIER_HSID_MIN]));
		}
		else	/* if RSSI data is un-available, hs_status is un-available */
		{
			json_object_object_add(jobj_handset_list, "hs_status", json_object_new_int(/*hs_status[hs_id[i]-RAINIER_HSID_MIN]*/ 3));	// 3: Un-available
		}
		json_object_object_add(jobj_handset_list, "hs_type", json_object_new_int(hs_type[hs_id[i]-RAINIER_HSID_MIN]));
		api_get_string_option2(display_name, sizeof(display_name), "sip_hs.sip_hs_%d.display_name", hs_id[i]);
		json_object_object_add(jobj_handset_list, "display_name", json_object_new_string(display_name));
		json_object_array_add(jarr_handset_list, jobj_handset_list);

		//debug_print("HS%d status:%d type:%d\n", hs_id[i], hs_status[hs_id[i]-RAINIER_HSID_MIN], hs_type[hs_id[i]-RAINIER_HSID_MIN]);
	}

#else	/* handset num use rainierReg data */
	for(i = RAINIER_HSID_MIN; i <= RAINIER_HSID_MAX; i++)
	{
		/* check the hs_idx is registered or not */
    	api_get_bool_option2(&tmp, "sip_hs.sip_hs_%d.rainierReg", i);
		rainierReg = (tmp == 1)?true:false;

		if(rainierReg == true)
		{
    		jobj_handset_list = json_object_new_object();
			json_object_object_add(jobj_handset_list, "hs_id", json_object_new_int(i));

			location_in = 0;
			for(idx = 0; idx < hs_num; idx++)
			{
				if(hs_id[idx] == i)
				{
					location_in = location[idx];
				}
			}
			if(location_in != 0)
			{
				json_object_object_add(jobj_handset_list, "hs_status", json_object_new_int(hs_status[i-RAINIER_HSID_MIN]));
			}
			else	/* if RSSI data is un-available, hs_status is un-available */
			{
				json_object_object_add(jobj_handset_list, "hs_status", json_object_new_int(/*hs_status[i-RAINIER_HSID_MIN]*/ 3));	// 3: Un-available
			}
			
			json_object_object_add(jobj_handset_list, "hs_type", json_object_new_int(hs_type[i-RAINIER_HSID_MIN]));
			api_get_string_option2(display_name, sizeof(display_name), "sip_hs.sip_hs_%d.display_name", i);
			json_object_object_add(jobj_handset_list, "display_name", json_object_new_string(display_name));
			json_object_array_add(jarr_handset_list, jobj_handset_list);

			//debug_print("HS%d status:%d type:%d\n", i, hs_status[i-RAINIER_HSID_MIN], hs_type[i-RAINIER_HSID_MIN]);
		}
	}
#endif
#endif	// get handset status from ast

	json_object_object_add(jobj, "handset_list", jarr_handset_list);

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

void printf_graph_time_string(graph_call_data_t *data, int nu)
{

	int i = 0;
	
	for(i =0 ; i < nu; i++)
	{
		debug_print(" Jason DEBUG %s:%d graph index %d start[%s] end[%s] \n", __FUNCTION__, __LINE__, 
				data[i].index, data[i].start_time, data[i].end_time);
	}
}


int graph_time_correction(struct tm *endtm, int range, int *intval)
{
	time_t end;
	int sec_c;
	struct tm *tm1;
	end = mktime(endtm);
	int interval_sec = 0;

	debug_print(" Jason DEBUG %s:%d graph end  \n", __FUNCTION__, __LINE__);
	tm_struct_print(endtm);

	switch(range)
	{
		
		case OVERVIEW_LAST_HOUR:
				if(endtm->tm_sec == 0)
					goto no_corec;
				sec_c = 60 - endtm->tm_sec;

				*intval = 60; /* 1 mins*/
			break;
	

		case OVERVIEW_LAST_DAY:
				if(((endtm->tm_min % 30) == 0) && (endtm->tm_sec == 0))
					goto no_corec;

				sec_c = 60 - endtm->tm_sec;
				sec_c += (29 - endtm->tm_min ) * 60;


				*intval = 30 * 60; /* 30 min to sec */
			break;
		
		case OVERVIEW_LAST_WEEK:
				if((endtm->tm_hour % 3 == 0) && (endtm->tm_min == 0) && (endtm->tm_sec == 0))
					goto no_corec;
				
				sec_c = 60 - endtm->tm_sec;
				sec_c += (59 - endtm->tm_min ) * 60;
				sec_c += (2 - (endtm->tm_hour % 3)) * 3600;


				*intval = 3 * 60 * 60; /* 3 hour to sec */
			break;
		default:
			goto no_corec;
	}

	end += sec_c;

	return end;
no_corec:
	return 0;
	
}
#if 1
int get_graph_time(char *start, char *end, int interval_sec, int nu, graph_call_data_t *data, int range)
{
	int i = 0;
	char start_str[32] = {0};
	time_t start_t;
	char end_str[32] = {0};
	time_t end_t;

	struct tm start_tm;
	struct tm end_tm;

	int intvl_sec;
	int index; 

	db_time_string_to_struct_tm(end, DB_TIME_FORMAT, &end_tm);

	end_t = graph_time_correction(&end_tm, range, &intvl_sec);

	for(i =0 ; i < nu; i++)
	{
		index = nu - i - 1;
		
		start_t = end_t - intvl_sec;

		strftime(data[index].end_time, sizeof(data[index].end_time),
			       	"%Y-%m-%d %H:%M:%S", localtime(&end_t));

		strftime(data[index].start_time, sizeof(data[index].start_time),
			       	"%Y-%m-%d %H:%M:%S", localtime(&start_t));

		data[index].start_s = start_t;
		data[index].end_s = end_t;

		end_t = start_t;

	}

	return 0;
}


#else	
int get_graph_time(char *start, char *end, int interval_sec, int nu, graph_call_data_t *data)
{
	int i = 0;
	char start_str[32] = {0};
	time_t start_t;
	char end_str[32] = {0};
	time_t end_t;

	struct tm start_tm;
	struct tm end_tm;

	strcpy(start_str, start);


	for(i =0 ; i < nu; i++)
	{

		db_time_string_to_struct_tm(start_str, DB_TIME_FORMAT, &start_tm);
		
		start_t = mktime(&start_tm);
		end_t = start_t + interval_sec;

		data[i].start_s = start_t;
		data[i].end_s = end_t;


		/* copy this interval time to start and end string */
		strncpy(data[i].start_time, start_str, sizeof(data[i].start_time));
		strftime(data[i].end_time, sizeof(data[i].end_time),
			       	"%Y-%m-%d %H:%M:%S", localtime(&end_t));

		/* next start = this end + 1 sec  */
		start_t = end_t + 1;
		strftime(start_str, sizeof(start_str),
			       	"%Y-%m-%d %H:%M:%S", localtime(&start_t));
	}

	return 0;
}
#endif
int get_overview_time(char *start, int len_s, char *end, int len_e, int range)
{
	char buf[256] = {0};
	char buff[256] = {0};
	char cmd[256] = {0};
	
	time_t end_sec;


	sys_interact_simple(end, len_e, "date \"+%Y-%m-%d %H:%M:%S\"");

	debug_print(" Jason DEBUG %s[%d] end[%s] \n", __FUNCTION__, __LINE__, end);

	sys_interact_simple(buf, sizeof(buf), "date +%s");
	end_sec = atoi(buf);
	

	switch(range)
	{
		case OVERVIEW_LAST_DAY:
			end_sec = end_sec - 86400;
			break;
		
		case OVERVIEW_LAST_WEEK:
			end_sec = end_sec - (86400 * 7);
			break;

		case OVERVIEW_LAST_HOUR:
			defafult:
			end_sec = end_sec - 3600;
			break;
	}
	
	strftime(start, len_s, "%Y-%m-%d %H:%M:%S", localtime(&end_sec));
 
	debug_print("time start  %s \n", start);

	return 0;
}

int call_top_sorting(call_top_time_t *top, int nu)
{
	int i, j;
	int tmp_id; 
	signed long tmp_time;

	for(i = 0; i < nu - 1; i++)
	{
		for(j = 0; j < nu - 1; j++)
		{
			if(top[j].time < top[j + 1].time)
			{
				tmp_id = top[j].hs_id;		
				tmp_time = top[j].time;		
				
				top[j].hs_id = top[j + 1].hs_id;
				top[j].time = top[j + 1].time;

				top[j + 1].hs_id = tmp_id;
				top[j + 1].time = tmp_time;
			}

		}

	}

}

int call_distribution_number(call_top_time_t *top, char *start, char *end, int nu, int type)
{
	char str[32] = {0};

	get_cdr_duration(start, end, 0, 0, type, 
			DB_CDR_PATH, DB_CDR_TABLE,  NULL, str, sizeof(str));

	top[0].hs_id = 1;
	top[0].time = atoi(str);
}

int top_use_base_number(call_top_time_t *top, char *start, char *end, int nu, int type)
{
	int bsid = 0;
	int max_bsid = 8;
	int empty = 0;
	int i = 0;
	int sec = 0;

	char str[32] = {0};



	for(bsid = RAINIER_BSID_MIN; bsid <= RAINIER_BSID_MAX; bsid++)
	{

		empty = nu - 1;
		/* find if has empty at array */
		for(i = 0; i < nu; i++)
		{
			if(top[i].hs_id == 0)
			{
				empty = i;
				break;
			}
		}

		memset(str, 0, sizeof(str));


		get_cdr_duration(start, end, bsid, 0, type, 
			DB_CDR_PATH, DB_CDR_TABLE,  DB_CDR_DURATION, str, sizeof(str));
#if 0
		debug_print("overveiw start  %s to end %s \n", start, end);
		debug_print("overveiw bsid  %d \n", bsid);
		debug_print("overveiw hsid  %d \n", hs_id);
  		/* convert string to second */
		debug_print("overveiw str  %s \n", str);
#endif
		//sec = time_string_to_sec(str);	
		sec = atoi(str);	
		
		if(sec == 0)
			continue;

		top[empty].hs_id = bsid;
		top[empty].time = sec;

		debug_print("top use base [%d] bsid %d , time %d \n", empty, top[empty].hs_id, top[empty].time);
	}

	call_top_sorting(top, nu);

	return 0;
}

int top_use_handset_base_number(call_top_time_t *top, char *start, char *end, int bsid, int hsid, int nu, int type)
{
	int i = 0;
	int hs_id = 0;
	int max_hsid = 99;
	char str[32] = {0};
	int sec;
	int empty = 0;	

	for(hs_id = 0; hs_id <= max_hsid; hs_id++)
	{
		empty = nu - 1;
		/* find if has empty at array */
		for(i = 0; i < nu; i++)
		{
			if(top[i].hs_id == 0)
			{
				empty = i;
				break;
			}
		}

		memset(str, 0, sizeof(str));


		get_cdr_duration(start, end, bsid, hs_id, type, 
			DB_CDR_PATH, DB_CDR_TABLE,  DB_CDR_DURATION, str, sizeof(str));
#if 0
		debug_print("overveiw start  %s to end %s \n", start, end);
		debug_print("overveiw bsid  %d \n", bsid);
		debug_print("overveiw hsid  %d \n", hs_id);
  		/* convert string to second */
		debug_print("overveiw str  %s \n", str);
#endif
		//sec = time_string_to_sec(str);	
		sec = atoi(str);	
		
		if(sec == 0)
			continue;

		top[empty].hs_id = hs_id;
		top[empty].time = sec;

//		debug_print("top[%d] hsid %d , time %d \n", empty, top[empty].hs_id, top[empty].time);
	}

	call_top_sorting(top, nu);
}
int top_nu_time_db_base(call_top_time_t *top, int bsid, int hsid, char *start, char *end, int nu, int type)
{

	if(type == TOP_USE_BASE_TYPE_NUMBER)
		return top_use_base_number(top, start, end, nu, type);
	else if((type == DISTRIBUTION_CALL_SIP) || (type == DISTRIBUTION_CALL_FXO))
		return call_distribution_number(top, start, end, nu, type);
	else
		return top_use_handset_base_number(top, start, end, bsid, hsid, nu, type);

}
int get_graph_interval(int range, int nu)
{
	int sec = 0;

	switch(range)
	{
		case OVERVIEW_LAST_DAY:
			sec = 86400 / nu;
			break;
		
		case OVERVIEW_LAST_WEEK:
			sec = (86400 * 7) / nu;
			break;

		case OVERVIEW_LAST_HOUR:
			defafult:
			sec = 3600 / nu;
			break;
	}

	return sec;
}
call_top_time_t* get_top_call_time(int bsid, int hsid, int range, int nu, call_top_time_t *top, int type)
{
	char start_time[32] = {0};
	char end_time[32] = {0};

	/*  fine top call time in database */

 	get_overview_time(start_time, sizeof(start_time), end_time, sizeof(end_time), range);

	top_nu_time_db_base(top, bsid, hsid, start_time, end_time, nu, type);
}

int get_graph_data(int bsid, int range, int nu, graph_call_data_t *data)
{
	int i = 0;
	int sip_call_count = 0;
	char start_time[32] = {0};
	char end_time[32] = {0};
	char time_buf[32] = {0};
	/*  fine top call time in database */

 	get_overview_time(start_time, sizeof(start_time), end_time, sizeof(end_time), range);

	for(i = 0; i < nu; i++)
	{
		data[i].index = i;
		memset(data[i].start_time, 0, sizeof(data[i].start_time));
		memset(data[i].end_time, 0, sizeof(data[i].end_time));
		data[i].time = 0;
		data[i].number = 0;
	}

	int interval_sec = get_graph_interval(range, nu);
	
	if(get_graph_time(start_time, end_time, interval_sec, nu, data, range) == 0)
	{
		printf_graph_time_string(data, nu);
	}

	for(i = 0; i < nu; i++)
	{

		if(get_sip_call_count(data[i].start_time, data[i].end_time, bsid, 0, &sip_call_count, -1) == 0)
		{
			data[i].number = sip_call_count;
		}


		if(get_cdr_duration(data[i].start_time, data[i].end_time, bsid, 0, TOP_CALL_TYPE_ALL, 
			DB_CDR_PATH, DB_CDR_TABLE,  DB_CDR_DURATION, time_buf, sizeof(time_buf)) == 0)
		{

			data[i].time = atoi(time_buf);
		}
	}
}
int json_post_rainier_call_overview_range(ResponseEntry *rep, char *query_str)
{
    int range = 0, select_base = 0;
	struct json_object *jobj = NULL;
    ResponseStatus *res = rep->res;

    if(NULL != query_str)
    {
        if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
            /* get data from api */
            senao_json_object_get_integer(jobj, "range", &range);
			senao_json_object_get_integer(jobj, "select_base", &select_base);

            /* TODO - insert your code here */
        }
        else
        {
            RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
        }
    }
    else
    {
        RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
    }
	//overview_set.range = range;
	//overview_set.select_base = select_base;
	SYSTEM("uci set senao-bsc-api.overview=overview");
	SYSTEM("uci set senao-bsc-api.overview.range='%d'", range);
	SYSTEM("uci set senao-bsc-api.overview.select_base='%d'", select_base);
	SYSTEM("uci commit senao-bsc-api");	

	//debug_print("post overveiw range %d bs %d \n", overview_set.range, overview_set.select_base);
	

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}


int get_overview_time_zone(char *start_time, char *end_time, int range, 
		int nu, overview_info_t *info)
{
	int i = 0;
	graph_call_data_t *data = info->graph_data;

	for(i = 0; i < nu; i++)
	{
		data[i].index = i;
		memset(data[i].start_time, 0, sizeof(data[i].start_time));
		memset(data[i].end_time, 0, sizeof(data[i].end_time));
		data[i].time = 0;
		data[i].number = 0;
	}

	int interval_sec = get_graph_interval(range, nu);
	
	if(get_graph_time(start_time, end_time, interval_sec, nu, data, range) == 0)
	{
		printf_graph_time_string(data, nu);
	}
}
int read_data_from_table(char *db_file, char *table, overview_info_t *info)
{
	char buf[256] = {0};
	char start_time[32] = {0};
	char end_time[32] = {0};

	sqlite3 *sqldb = NULL;

	int range = info->overview_search->range;


	get_overview_time(start_time, sizeof(start_time), end_time, sizeof(end_time), range);

	get_overview_time_zone(start_time, end_time, range, CALL_GRAPH_NUM, info);

	if(get_database(db_file, &sqldb) < 0)
  		return -1;

	sprintf(buf, "SELECT * FROM %s WHERE time>=\'%s\' AND time<\'%s\'", 
						table, start_time, end_time);
				
	read_database(buf, sqldb, time_all_call_read_callback, info);


	free_database(sqldb);



	return 0;
}

int check_in_top_list(call_top_time_t *new, call_top_time_t *dst)
{
	int i = 0;
	

	for(i = 0; i < OVERVIEW_TOP_NUM; i++)
	{
		//debug_print(" xxx check_in_top_list i[%d]   dst[i].hs_id[%d]  new.hs_id [%d] \n", i,  dst[i].hs_id, new->hs_id);
		if(new->hs_id == 0)
			return -1;

		if(new->hs_id == dst[i].hs_id)
			return 1;
	}
	
	return 0;
}

int save_in_top_list(call_top_time_t new, call_top_time_t *top)
{
	int i = 0;

	for(i = 0; i < OVERVIEW_TOP_NUM; i++)
	{
		if(top[i].hs_id == 0)
		{
			top[i].hs_id = new.hs_id;
			top[i].time = new.time;
			//sdebug_print(" xxx save_in_top_list i[%d]  top[i].hs_id[%d]  top[i].time[%d] \n", i, top[i].hs_id, top[i].time);
		
			return 0;
		}
	}
	
	return -1;
}
int max_id_value(call_top_time_t *dst, call_top_time_t *src, int top_num, int max_id)
{
	int top_index = 0;
	int src_index = 0;
	int i = 0, j = 0;

	call_top_time_t max;
#if 0
	debug_print("================  src top_call_time =================\n ");
	print_call_top_time(src, 30);
	debug_print("=================================================\n ");


	debug_print("================  dst top_call_time =================\n ");
	print_call_top_time(dst, 11);
	debug_print("=================================================\n ");
#endif
	for(i = 0; i < top_num; i++)
	{
		memset(&max, 0, sizeof(max));

		max.hs_id = src[0].hs_id;
		max.time = src[0].time;


		for(j = 1; j < max_id; j++)
		{
			if(src[j].time > max.time)
			{
				 //debug_print("src[%d].time[%d] > max.time [%d]\n", j, src[j].time, max.time);
				if(check_in_top_list(&src[j], dst) == 0)
				{
					max.hs_id = src[j].hs_id;
					max.time = src[j].time;
					//debug_print(" max.hs_id[%d]  max.time [%d] not in the list  \n", max.hs_id, max.time);
				}
				else
				{
					//debug_print(" max.hs_id[%d]  max.time [%d] in the list  \n", max.hs_id, max.time);
					/* code */
				}
				
					
			}	
		}
		save_in_top_list(max, dst);
	}


	return 0;
}



int get_top_value(overview_info_t *info, call_top_time_t *top, int top_type)
{
	call_top_time_t *src;

	switch(top_type)
	{

		case TOP_CALL_TYPE_ALL:
			src = info->call_time;
			max_id_value(top, src, OVERVIEW_TOP_NUM, RAINIER_HSID_MAX + 1);
			break;
		case TOP_CALL_TYPE_ALL_NUMBER:
			src = info->call_number;
			max_id_value(top, src, OVERVIEW_TOP_NUM, RAINIER_HSID_MAX + 1);
			break;
		case TOP_CALL_TYPR_FXO:
			src = info->fxo_call_time;
			max_id_value(top, src, OVERVIEW_TOP_NUM, RAINIER_HSID_MAX + 1);
			break;
		case TOP_CALL_TYPR_FXO_NUMBER:
			src = info->fxo_call_number;
			max_id_value(top, src, OVERVIEW_TOP_NUM, RAINIER_HSID_MAX + 1);
			break;
		case TOP_USE_BASE_TYPE_NUMBER:
			src = info->use_bs_number;
			max_id_value(top, src, RAINIER_BSID_MAX, RAINIER_BSID_MAX + 1);
			break;
	}

	return 0;
}

int json_get_rainier_call_overview(ResponseEntry *rep, struct json_object *jobj)
{
	int index = 0, call_time = 0, call_number = 0;
	int hs_id = 0, bs_id = 0;
	int total_sip_call = 0, total_fxo_call = 0;
	int i;
	int idx;
	int total_graph_num;
	int total_top_call_num = 10;

	ResponseStatus *res = rep->res;
	graph_call_data_t graph_data[100];

	call_top_time_t top_call_time[11] = {0};
	call_top_time_t top_fxo_call_time[11] = {0};
	call_top_time_t top_call_number[11] = {0};
	call_top_time_t top_fxo_call_number[11] = {0};
	call_top_time_t top_base_use_number[RAINIER_BSID_MAX + 1] = {0};
	call_top_time_t call_distribution_sip[11] = {0};
	call_top_time_t call_distribution_fxo[11] = {0};

	char display_name[64] = {0};

	api_get_integer_option("senao-bsc-api.overview.range", &overview_set.range);
	api_get_integer_option("senao-bsc-api.overview.select_base", &overview_set.select_base);


	total_graph_num = CALL_GRAPH_NUM;
#if 0
	get_graph_data(overview_set.select_base, overview_set.range, total_graph_num, graph_data);

	get_top_call_time(overview_set.select_base, 0, overview_set.range, total_top_call_num, top_call_time, TOP_CALL_TYPE_ALL);
	get_top_call_time(overview_set.select_base, 0, overview_set.range, total_top_call_num, top_fxo_call_time, TOP_CALL_TYPR_FXO);
	get_top_call_time(overview_set.select_base, 0, overview_set.range, total_top_call_num, top_call_number, TOP_CALL_TYPE_ALL_NUMBER);
	get_top_call_time(overview_set.select_base, 0, overview_set.range, total_top_call_num, top_fxo_call_number, TOP_CALL_TYPR_FXO_NUMBER);
	get_top_call_time(overview_set.select_base, 0, overview_set.range, total_top_call_num, top_base_use_number, TOP_USE_BASE_TYPE_NUMBER);
	get_top_call_time(overview_set.select_base, 0, overview_set.range, total_top_call_num, call_distribution_sip, DISTRIBUTION_CALL_SIP);
	get_top_call_time(overview_set.select_base, 0, overview_set.range, total_top_call_num, call_distribution_fxo, DISTRIBUTION_CALL_FXO);
#else

	overview_info_t call_overview_var;
	memset(&call_overview_var, 0, sizeof(overview_info_t));

	call_overview_var.overview_search = &overview_set;
	call_overview_var.graph_data = graph_data;

	call_distribution_t *distr = &call_overview_var.distribution;

	read_data_from_table(DB_CALL_OVERVIEW_PATH, OVERVIEW_TABLE_NAME, &call_overview_var);
	get_top_value(&call_overview_var, top_call_time, TOP_CALL_TYPE_ALL);
	get_top_value(&call_overview_var, top_fxo_call_time, TOP_CALL_TYPR_FXO);
	get_top_value(&call_overview_var, top_call_number, TOP_CALL_TYPE_ALL_NUMBER);
	get_top_value(&call_overview_var, top_fxo_call_number, TOP_CALL_TYPR_FXO_NUMBER);
	get_top_value(&call_overview_var, top_base_use_number, TOP_USE_BASE_TYPE_NUMBER);

#endif
	struct json_object *jarr_call_graph = NULL, *jobj_call_graph = NULL;
	jarr_call_graph = json_object_new_array();

	struct json_object *jarr_top_call_times = NULL, *jobj_top_call_times = NULL;
	jarr_top_call_times = json_object_new_array();

	struct json_object *jarr_top_call_numbers = NULL, *jobj_top_call_numbers = NULL;
	jarr_top_call_numbers = json_object_new_array();

	struct json_object *jarr_top_call_times_fxo = NULL, *jobj_top_call_times_fxo = NULL;
	jarr_top_call_times_fxo = json_object_new_array();

	struct json_object *jarr_top_call_numbers_fxo = NULL, *jobj_top_call_numbers_fxo = NULL;
	jarr_top_call_numbers_fxo = json_object_new_array();

	struct json_object *jarr_top_use_base = NULL, *jobj_top_use_base = NULL;
	jarr_top_use_base = json_object_new_array();

	struct json_object *jobj_call_distribution = NULL;


	/* TODO - insert your code here */
    /* use switch api to get port date */

    /* END - insert your code here */

	

	for(idx = 0; idx < total_graph_num; idx++)
	{
		jobj_call_graph = json_object_new_object();
		json_object_object_add(jobj_call_graph, "index", json_object_new_int(graph_data[idx].index));
		json_object_object_add(jobj_call_graph, "date_time", json_object_new_string(graph_data[idx].start_time));
		json_object_object_add(jobj_call_graph, "call_time", json_object_new_int(graph_data[idx].time));
		json_object_object_add(jobj_call_graph, "call_number", json_object_new_int(graph_data[idx].number));
		json_object_array_add(jarr_call_graph, jobj_call_graph);
	}

	json_object_object_add(jobj, "call_graph", jarr_call_graph);

	for(idx = 0; idx < total_top_call_num; idx++)
	{
		if(!top_call_time[idx].hs_id)
			continue;
		jobj_top_call_times = json_object_new_object();

		json_object_object_add(jobj_top_call_times, "hs_id", json_object_new_int(top_call_time[idx].hs_id));

		memset(display_name, 0, sizeof(display_name));
		api_get_string_option2(display_name, sizeof(display_name), "sip_hs.sip_hs_%d.display_name", top_call_time[idx].hs_id);
		json_object_object_add(jobj_top_call_times, "display_name", json_object_new_string(display_name));

		json_object_object_add(jobj_top_call_times, "call_time", json_object_new_int(top_call_time[idx].time));

		json_object_array_add(jarr_top_call_times, jobj_top_call_times);
	}

	json_object_object_add(jobj, "top_call_times", jarr_top_call_times);

	for(idx = 0; idx < total_top_call_num; idx++)
	{
		if(!top_call_number[idx].hs_id)
			continue;

		jobj_top_call_numbers = json_object_new_object();

		json_object_object_add(jobj_top_call_numbers, "hs_id", json_object_new_int(top_call_number[idx].hs_id));


		memset(display_name, 0, sizeof(display_name));
		api_get_string_option2(display_name, sizeof(display_name), "sip_hs.sip_hs_%d.display_name", top_call_number[idx].hs_id);
		json_object_object_add(jobj_top_call_numbers, "display_name", json_object_new_string(display_name));


		json_object_object_add(jobj_top_call_numbers, "call_number", json_object_new_int(top_call_number[idx].time));

		json_object_array_add(jarr_top_call_numbers, jobj_top_call_numbers);
	}

	json_object_object_add(jobj, "top_call_numbers", jarr_top_call_numbers);

	for(idx = 0; idx < total_top_call_num; idx++)
	{
		if(!top_fxo_call_time[idx].hs_id)
			continue;

		jobj_top_call_times_fxo = json_object_new_object();

		json_object_object_add(jobj_top_call_times_fxo, "hs_id", json_object_new_int(top_fxo_call_time[idx].hs_id));


		memset(display_name, 0, sizeof(display_name));
		api_get_string_option2(display_name, sizeof(display_name), "sip_hs.sip_hs_%d.display_name", top_fxo_call_time[idx].hs_id);
		json_object_object_add(jobj_top_call_times_fxo, "display_name", json_object_new_string(display_name));


		json_object_object_add(jobj_top_call_times_fxo, "call_time", json_object_new_int(top_fxo_call_time[idx].time));

		json_object_array_add(jarr_top_call_times_fxo, jobj_top_call_times_fxo);
	}

	json_object_object_add(jobj, "top_call_times_fxo", jarr_top_call_times_fxo);

	for(idx = 0; idx < total_top_call_num; idx++)
	{
		if(!top_fxo_call_number[idx].hs_id)
			continue;

		jobj_top_call_numbers_fxo = json_object_new_object();

		json_object_object_add(jobj_top_call_numbers_fxo, "hs_id", json_object_new_int(top_fxo_call_number[idx].hs_id));


		memset(display_name, 0, sizeof(display_name));
		api_get_string_option2(display_name, sizeof(display_name), "sip_hs.sip_hs_%d.display_name", top_fxo_call_number[idx].hs_id);
		json_object_object_add(jobj_top_call_numbers_fxo, "display_name", json_object_new_string(display_name));

		json_object_object_add(jobj_top_call_numbers_fxo, "call_number", json_object_new_int(top_fxo_call_number[idx].time));

		json_object_array_add(jarr_top_call_numbers_fxo, jobj_top_call_numbers_fxo);
	}

	json_object_object_add(jobj, "top_call_numbers_fxo", jarr_top_call_numbers_fxo);

	/* top use base */

	for(i = RAINIER_BSID_MIN; i <= RAINIER_BSID_MAX; i++)
	{
		if(!top_base_use_number[i - 1].hs_id)
			continue;

		jobj_top_use_base = json_object_new_object();

		json_object_object_add(jobj_top_use_base, "bs_id", json_object_new_int(top_base_use_number[i - 1].hs_id));


		memset(display_name, 0, sizeof(display_name));
		api_get_string_option2(display_name, sizeof(display_name), "base-station-list.@base-station[%d].name", 
				top_base_use_number[i - 1].hs_id - 1);

		json_object_object_add(jobj_top_use_base, "bs_name", json_object_new_string(display_name));

		json_object_object_add(jobj_top_use_base, "call_number", json_object_new_int(top_base_use_number[i - 1].time));

		json_object_array_add(jarr_top_use_base, jobj_top_use_base);


	}

	json_object_object_add(jobj, "top_use_base", jarr_top_use_base);


	/* distribution */
	
	jobj_call_distribution = json_object_new_object();

	json_object_object_add(jobj_call_distribution, "total_sip_call", json_object_new_int(distr->total_sip_call));
	json_object_object_add(jobj_call_distribution, "total_fxo_call", json_object_new_int(distr->total_fxo_call));

	json_object_object_add(jobj, "call_distribution", jobj_call_distribution);

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_post_rainier_broadcast_setting(ResponseEntry *rep, char *query_str)
{
	int hs_broadcast_enable = 0, hs_broadcast_timeout = 0, hs_broadcast_fallback_p2p_timeout = 0;
	char *hs_broadcast_feature_code = NULL;
	int fxo_broadcast_enable = 0, fxo_broadcast_timeout = 0;

	struct json_object *jobj = NULL;
	ResponseStatus *res = rep->res;

	if(NULL != query_str)
	{
		if((jobj = jsonTokenerParseFromStack(rep, query_str)))
		{
			senao_json_object_get_integer(jobj, "hs_broadcast_enable", &hs_broadcast_enable);
			senao_json_object_get_integer(jobj, "hs_broadcast_timeout", &hs_broadcast_timeout);
			senao_json_object_get_integer(jobj, "hs_broadcast_fallback_p2p_timeout", &hs_broadcast_fallback_p2p_timeout);
			senao_json_object_get_and_create_string(rep, jobj, "hs_broadcast_feature_code", &hs_broadcast_feature_code);
			senao_json_object_get_integer(jobj, "fxo_broadcast_enable", &fxo_broadcast_enable);
			senao_json_object_get_integer(jobj, "fxo_broadcast_timeout", &fxo_broadcast_timeout);
		}
		else
		{
			RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
		}
	}
	else
	{
		RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
	}

	if ((hs_broadcast_enable < 0) || (hs_broadcast_enable > 255))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "HS BROADCAST ENABLE");

	if ((hs_broadcast_timeout < 30) || (hs_broadcast_timeout > 120))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "HS BROADCAST TIMEOUT");

	if ((hs_broadcast_fallback_p2p_timeout < 1) || (hs_broadcast_fallback_p2p_timeout > 5))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "HS BROADCAST FALLBACK P2P TIMEOUT");

	if ((fxo_broadcast_enable < 0) || (fxo_broadcast_enable > 255))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "FXO BROADCAST ENABLE");

	if ((fxo_broadcast_timeout < 30) || (fxo_broadcast_timeout > 120))
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "FXO BROADCAST TIMEOUT");


	api_set_integer_option("rainier.voip_basic.hs_broadcast_enable", hs_broadcast_enable);
	api_set_integer_option("rainier.voip_basic.hs_broadcast_timeout", hs_broadcast_timeout);
	api_set_integer_option("rainier.voip_basic.hs_broadcast_fallback_p2p_timeout", hs_broadcast_fallback_p2p_timeout);
	api_set_string_option("rainier.voip_basic.hs_broadcast_feature_code", hs_broadcast_feature_code, sizeof(hs_broadcast_feature_code));
	api_set_integer_option("rainier.rainier_basic_base.fxo_broadcast_enable", fxo_broadcast_enable);
	api_set_integer_option("rainier.rainier_basic_base.fxo_broadcast_timeout", fxo_broadcast_timeout);

	api_commit_option("rainier");

	// NMS send new FXO setting to BS
	SYSTEM("nmsconf_cli conf_mgm sendFxoSetting");

	// NMS send new VoIP setting to BS
	SYSTEM("nmsconf_cli conf_mgm sendVoipSetting");

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_get_rainier_broadcast_setting(ResponseEntry *rep, struct json_object *jobj)
{
	int hs_broadcast_enable = 0, hs_broadcast_timeout = 0, hs_broadcast_fallback_p2p_timeout = 0;
	char hs_broadcast_feature_code[8] = {0};
	int fxo_broadcast_enable = 0, fxo_broadcast_timeout = 0;
	char bs_name[32] = {0};
	int idx;
	int i;
	ResponseStatus *res = rep->res;

	struct json_object *jarr_base_list = NULL, *jobj_base_list = NULL;
	jarr_base_list = json_object_new_array();

	api_get_integer_option("rainier.voip_basic.hs_broadcast_enable", &hs_broadcast_enable);
	api_get_integer_option("rainier.voip_basic.hs_broadcast_timeout", &hs_broadcast_timeout);
	api_get_integer_option("rainier.voip_basic.hs_broadcast_fallback_p2p_timeout", &hs_broadcast_fallback_p2p_timeout);
	api_get_string_option("rainier.voip_basic.hs_broadcast_feature_code", hs_broadcast_feature_code, sizeof(hs_broadcast_feature_code));
	api_get_integer_option("rainier.rainier_basic_base.fxo_broadcast_enable", &fxo_broadcast_enable);
	api_get_integer_option("rainier.rainier_basic_base.fxo_broadcast_timeout", &fxo_broadcast_timeout);

	json_object_object_add(jobj, "hs_broadcast_enable", json_object_new_int(hs_broadcast_enable));
	json_object_object_add(jobj, "hs_broadcast_timeout", json_object_new_int(hs_broadcast_timeout));
	json_object_object_add(jobj, "hs_broadcast_fallback_p2p_timeout", json_object_new_int(hs_broadcast_fallback_p2p_timeout));
	json_object_object_add(jobj, "hs_broadcast_feature_code", json_object_new_string(hs_broadcast_feature_code));
	json_object_object_add(jobj, "fxo_broadcast_enable", json_object_new_int(fxo_broadcast_enable));
	json_object_object_add(jobj, "fxo_broadcast_timeout", json_object_new_int(fxo_broadcast_timeout));

	for(i = RAINIER_BSID_MIN; i <= RAINIER_BSID_MAX; i++)
    {
		/* check the base is registered to BSC or not */
		idx = findBsConfigIdxByIndex(i);
		if(idx != -1) // bs is in the list
		{
			jobj_base_list = json_object_new_object();
			json_object_object_add(jobj_base_list, "bs_id", json_object_new_int(i));

			api_get_string_option2(bs_name, sizeof(bs_name), "base-station-list.@base-station[%d].name", idx);
			json_object_object_add(jobj_base_list, "bs_name", json_object_new_string(bs_name));
			json_object_array_add(jarr_base_list, jobj_base_list);
		}
	}

	json_object_object_add(jobj, "base_list", jarr_base_list);

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_get_rainier_base_name(ResponseEntry *rep, struct json_object *jobj)
{
	char bs_name[32] = {0};
	int idx;
	int i;
	ResponseStatus *res = rep->res;

	struct json_object *jarr_base_list = NULL, *jobj_base_list = NULL;
	jarr_base_list = json_object_new_array();

	for(i = RAINIER_BSID_MIN; i <= RAINIER_BSID_MAX; i++)
    {
		/* check the base is registered to BSC or not */
		idx = findBsConfigIdxByIndex(i);
		if(idx != -1) // bs is in the list
		{
			jobj_base_list = json_object_new_object();
			json_object_object_add(jobj_base_list, "bs_id", json_object_new_int(i));

			api_get_string_option2(bs_name, sizeof(bs_name), "base-station-list.@base-station[%d].name", idx);
			json_object_object_add(jobj_base_list, "bs_name", json_object_new_string(bs_name));
			json_object_array_add(jarr_base_list, jobj_base_list);
		}
	}

	json_object_object_add(jobj, "base_list", jarr_base_list);

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_get_rainier_base_name_idx(ResponseEntry *rep, struct json_object *jobj, int bs_idx)
{
	char bs_name[32] = {0};
	int idx;
	ResponseStatus *res = rep->res;

    if((bs_idx >= RAINIER_BSID_MIN) && (bs_idx <= RAINIER_BSID_MAX))
    {
		/* check the base is registered to BSC or not */
		idx = findBsConfigIdxByIndex(bs_idx);
		if(idx != -1) // bs is in the list
		{
    		/* get base data */
    		api_get_string_option2(bs_name, sizeof(bs_name), "base-station-list.@base-station[%d].name", idx);
		}
		else
    	{
	        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "BS ID");
	    }
    }
    else
    {
        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "BS ID");
    }
	
	json_object_object_add(jobj, "bs_id", json_object_new_int(bs_idx));
	json_object_object_add(jobj, "bs_name", json_object_new_string(bs_name));
	
	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_get_rainier_handset_name(ResponseEntry *rep, struct json_object *jobj)
{
	char display_name[64] = {0};
	int tmp = 0;
	bool rainierReg = 0;
	int i;
	ResponseStatus *res = rep->res;

	struct json_object *jarr_handset_list = NULL, *jobj_handset_list = NULL;
	jarr_handset_list = json_object_new_array();

	for(i = RAINIER_HSID_MIN; i <= RAINIER_HSID_MAX; i++)
	{
		/* check the hs_idx is registered or not */
    	api_get_bool_option2(&tmp, "sip_hs.sip_hs_%d.rainierReg", i);
		rainierReg = (tmp == 1)?true:false;

		if(rainierReg == true)
		{
    		jobj_handset_list = json_object_new_object();
			json_object_object_add(jobj_handset_list, "hs_id", json_object_new_int(i));

			api_get_string_option2(display_name, sizeof(display_name), "sip_hs.sip_hs_%d.display_name", i);
			json_object_object_add(jobj_handset_list, "display_name", json_object_new_string(display_name));
			json_object_array_add(jarr_handset_list, jobj_handset_list);
		}
	}

	json_object_object_add(jobj, "handset_list", jarr_handset_list);

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_get_rainier_handset_name_idx(ResponseEntry *rep, struct json_object *jobj, int hs_idx)
{
	char display_name[64] = {0};
	int tmp = 0;
	bool rainierReg = 0;
	ResponseStatus *res = rep->res;

    if((hs_idx >= RAINIER_HSID_MIN) && (hs_idx <= RAINIER_HSID_MAX))
    {
		/* check the hs_idx is registered or not */
    	api_get_bool_option2(&tmp, "sip_hs.sip_hs_%d.rainierReg", hs_idx);
		rainierReg = (tmp == 1)?true:false;

		if(rainierReg == true)
		{
    		/* get handset data */
    		api_get_string_option2(display_name, sizeof(display_name), "sip_hs.sip_hs_%d.display_name", hs_idx);
		}
		else
    	{
	        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "HS ID");
	    }
    }
    else
    {
        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "HS ID");
    }
	
	json_object_object_add(jobj, "hs_id", json_object_new_int(hs_idx));
	json_object_object_add(jobj, "display_name", json_object_new_string(display_name));
	
	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int burn_fw_bsc()
{
    int led = 0, senaowrt = 0;
    char buf[10] = {0};
    
    api_get_integer_option("functionlist.functionlist.SUPPORT_LED_MODULE_NAME", &led);
    api_get_integer_option("functionlist.functionlist.SUPPORT_SENAOWRT_IMAGE", &senaowrt);

    memset(buf,0,sizeof(buf));
    sys_interact(buf, sizeof(buf), "pgrep avahi-daemon");
    if ( strlen(buf) > 0 )
        system("/etc/init.d/avahi-daemon stop");

#if SUPPORT_ENBSC_OPENAPI_SERVER
	SYSTEM("cp %s /tmp/firmware.img", FW_FILENAME_BSC);
#endif

    if (led == 1) 
    {
        if (senaowrt == 1)
        {
            system("(sleep 3; killall dropbear uhttpd lighttpd web ; sleep 1; echo timer > /sys/class/leds/power1_led/trigger;/etc/fwupgrade.sh /tmp/firmware.img) &");
        }
        else
        {
            system("(sleep 3; killall dropbear uhttpd lighttpd web ; sleep 1; echo timer > /sys/class/leds/power1_led/trigger;/sbin/sysupgrade /tmp/firmware.img) &");
        }
    }
    else
    {
        if (senaowrt == 1)
        {
            system("(sleep 3; killall dropbear uhttpd lighttpd web ; sleep 1; echo timer > /sys/class/leds/power/trigger;/etc/fwupgrade.sh /tmp/firmware.img) &");
        }
        else
        {
            system("(sleep 3; killall dropbear uhttpd lighttpd web ; sleep 1; echo timer > /sys/class/leds/power/trigger;/sbin/sysupgrade /tmp/firmware.img) &");
        }
    }
    return 0;
}

int json_post_mgm_fw_upgrade_bsc(ResponseEntry *rep, char *query_str)
{
    char *upgrade_from_server=NULL, *mode=NULL;
    // bool doReset = 0;
    int hs_upgrade_status = 0, bs_upgrade_status = 0;
    char upgrade_bsc_fw_version[32] = {0};
    struct json_object *jobj;
    ResponseStatus *res = rep->res;

	api_get_integer_option("firmware_info.hs_fw.upgrade_processing", &hs_upgrade_status);
	api_get_integer_option("firmware_info.bs_fw.upgrade_processing", &bs_upgrade_status);

	if(hs_upgrade_status || bs_upgrade_status)
	{
		RET_GEN_ERRORMSG(res, API_SERVICE_ERROR, "FW UPGRADE PROCESSING");
	}

#if SUPPORT_SYSTEM_LOG
    system("echo upgrade>/mnt/rebootType");
#endif

    if(NULL != query_str)
    {
        if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
            senao_json_object_get_and_create_string(rep, jobj, "mode", &mode);
            // senao_json_object_get_boolean(jobj, "do_reset_after_upgrade", &doReset);
            if (strcmp(mode, "Upgrade_from_server") == 0)
            {
                senao_json_object_get_and_create_string(rep, jobj, "upgrade_from_server", &upgrade_from_server);
                //return json_fw_upgrade_fromserver(rep, upgrade_from_server);
                RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "Upgrade from server not yet support");
            }
            else if (strcmp(mode, "Upgrade_locally") == 0)
            {
                if(api_get_string_option("firmware_info.938c.version", upgrade_bsc_fw_version, sizeof(upgrade_bsc_fw_version)) != API_RC_SUCCESS)
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "No BSC firmware");

                if(strcmp(upgrade_bsc_fw_version, "---") == 0)
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "No BSC firmware");

                if(sysIsFileExisted(FW_FILENAME_BSC) != TRUE)
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "No BSC firmware");

                burn_fw_bsc();
            }
        }
    }
    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);

}

int json_post_mgm_fw_upgrade_bs(ResponseEntry *rep, char *query_str)
{
	int target_base = 0;
	int idx;
	char *upgrade_from_server=NULL, *mode=NULL;
	int hs_upgrade_status = 0, bs_upgrade_status = 0;
	int i;
	int status = -1;
	int ret;
	char upgrade_bs_fw_version[32] = {0};

	struct json_object *jobj = NULL;
	ResponseStatus *res = rep->res;

	api_get_integer_option("firmware_info.hs_fw.upgrade_processing", &hs_upgrade_status);
	api_get_integer_option("firmware_info.bs_fw.upgrade_processing", &bs_upgrade_status);

	if(hs_upgrade_status || bs_upgrade_status)
	{
		RET_GEN_ERRORMSG(res, API_SERVICE_ERROR, "FW UPGRADE PROCESSING");
	}

	if(NULL != query_str)
	{
		if((jobj = jsonTokenerParseFromStack(rep, query_str)))
		{
			senao_json_object_get_integer(jobj, "target_base", &target_base);

			senao_json_object_get_and_create_string(rep, jobj, "mode", &mode);
            // senao_json_object_get_boolean(jobj, "do_reset_after_upgrade", &doReset);
            if (strcmp(mode, "Upgrade_from_server") == 0)
            {
                senao_json_object_get_and_create_string(rep, jobj, "upgrade_from_server", &upgrade_from_server);
                //return json_fw_upgrade_fromserver(rep, upgrade_from_server);
				RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "Upgrade from server not yet support");
            }
            else if (strcmp(mode, "Upgrade_locally") == 0)
            {
#if SUPPORT_SP938BS_OPENAPI_SERVER
		SYSTEM("rainier_web_fw -m 3 -p %s -i %s &", SP938BS_PID, SP938BS_LOCAL_FW_FILENAME);
		RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
#endif
                //burn_fw();
				// continue to post api to base
            }
			else
			{
				RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "MODE");
			}
		}
	}

	if(!target_base)
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "TARGET BASE");
	
	if(api_get_string_option("firmware_info.938b.version", upgrade_bs_fw_version, sizeof(upgrade_bs_fw_version)) != API_RC_SUCCESS)
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "No BS firmware");
			
	if(strcmp(upgrade_bs_fw_version, "---") == 0)
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "No BS firmware");
			
	if(sysIsFileExisted(FW_FILENAME_BS) != TRUE)
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "No BS firmware");

	for(i = RAINIER_BSID_MIN; i <= RAINIER_BSID_MAX; i++)
    {
		if(target_base & (1 << (i-1)))
		{
			/* check the base is registered to BSC or not */
			idx = findBsConfigIdxByIndex(i);

			//if(idx == -1)
			//	RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "TARGET_BASE");
			
			if(idx != -1) // bs is in the list
			{
				ret = json_post_start_bs_fw_upgrade(rep, query_str, idx);
				if(ret == API_SUCCESS)
				{
					status = API_SUCCESS;
				}
			}
		}
	}

	if(status == -1)
	{
		RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "Unable to set bs_fw_upgrade to Base");
	}

	api_set_integer_option("firmware_info.bs_fw.upgrade_processing", 1);
	api_commit_option("firmware_info");
	
	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);

#if 0
	if((target_base >= RAINIER_BSID_MIN) && (target_base <= RAINIER_BSID_MAX))
	{
        	/* check the base_idx is registered to BSC or not */
		idx = findBsConfigIdxByIndex(target_base);

		if(idx == -1)
			RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "BASE ID");

		return json_post_start_bs_fw_upgrade(rep, query_str, idx);
	}
        
	RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "BASE ID");        
#endif
#if 0	
	char cmd[256] = {0};
	char bs_ip[32] = {0};

    	if((base_idx >= RAINIER_BSID_MIN) && (base_idx <= RAINIER_BSID_MAX))
    	{
        	/* check the base_idx is registered to BSC or not */
		idx = findBsConfigIdxByIndex(base_idx);

		if(idx == -1)
			RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "BASE ID");

		snprintf(cmd, sizeof(cmd), "uci get base-station-list.@base-station[%d].ip | tr -d \"\\n\"", idx);
		sys_interact(bs_ip, sizeof(bs_ip), cmd);
		debug_print("curr idx: %d, bs_ip: %s.\n", idx, bs_ip);


	}
	else
	{
        	RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "BASE ID");        
	}
			
	return json_post_start_bs_fw_upgrade(rep, query_str, base_idx);
	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
#endif	
}

int json_post_mgm_fw_upgrade_hs(ResponseEntry *rep, char *query_str)
{
	//int base_id = -1;
	int idx;
	char *upgrade_from_server=NULL, *mode=NULL;
	int hs_upgrade_status = 0, bs_upgrade_status = 0;
	int i;
	int status = -1;
	int ret;
	char upgrade_hs_fw_version[32] = {0};

	struct json_object *jobj = NULL;
	ResponseStatus *res = rep->res;

	api_get_integer_option("firmware_info.hs_fw.upgrade_processing", &hs_upgrade_status);
	api_get_integer_option("firmware_info.bs_fw.upgrade_processing", &bs_upgrade_status);

	if(hs_upgrade_status || bs_upgrade_status)
	{
		RET_GEN_ERRORMSG(res, API_SERVICE_ERROR, "FW UPGRADE PROCESSING");
	}
	
	if(NULL != query_str)
	{
		if((jobj = jsonTokenerParseFromStack(rep, query_str)))
		{
			senao_json_object_get_and_create_string(rep, jobj, "mode", &mode);
            // senao_json_object_get_boolean(jobj, "do_reset_after_upgrade", &doReset);
            if (strcmp(mode, "Upgrade_from_server") == 0)
            {
                senao_json_object_get_and_create_string(rep, jobj, "upgrade_from_server", &upgrade_from_server);
                //return json_fw_upgrade_fromserver(rep, upgrade_from_server);
				RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "Upgrade from server not yet support");
            }
            else if (strcmp(mode, "Upgrade_locally") == 0)
            {
                //burn_fw();
				// continue to post api to base
            }
			else
			{
				RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "MODE");
			}
		}
	}

	if(api_get_string_option("firmware_info.938a.version", upgrade_hs_fw_version, sizeof(upgrade_hs_fw_version)) != API_RC_SUCCESS)
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "No HS firmware");
			
	if(strcmp(upgrade_hs_fw_version, "---") == 0)
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "No HS firmware");

	if(sysIsFileExisted(FW_FILENAME_HS) != TRUE)
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "No HS firmware");

	for(i = RAINIER_BSID_MIN; i <= RAINIER_BSID_MAX; i++)
    {
		/* check the base is registered to BSC or not */
		idx = findBsConfigIdxByIndex(i);

		//if(idx == -1)
		//	RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "TARGET_BASE");
		
		if(idx != -1) // bs is in the list
		{
			ret = json_post_start_hs_fw_upgrade(rep, query_str, idx);
			if(ret == API_SUCCESS)
			{
				status = API_SUCCESS;
			}
		}
	}

	if(status == -1)
	{
		RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "Unable to set hs_fw_upgrade to Base");
	}

	api_set_integer_option("firmware_info.hs_fw.upgrade_processing", 1);
	api_commit_option("firmware_info");

	// NMS reset handset fw upgrade status
	SYSTEM("nmsconf_cli bs resetHsFwStatus");
	
	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_post_mgm_fw_upgrade_hs_abort(ResponseEntry *rep, char *query_str)
{
	//int base_id = -1;
	int idx;
	//char *upgrade_from_server=NULL, *mode=NULL;
	int hs_upgrade_status = 0;
	int i;
	int status = -1;
	int ret;
	int hs_id[RAINIER_HS_NUMBER] = {0};
	int hs_progress[RAINIER_HS_NUMBER] = {0};
	int hs_status[RAINIER_HS_NUMBER] = {0};
	int hs_under_bs[RAINIER_HS_NUMBER] = {0};
	int bs_progress[RAINIER_BASE_NUM] = {0};
	int bs_status[RAINIER_BASE_NUM] = {0};
	int hs_num = 0;
	int hs_upgrade_started = 0;

	struct json_object *jobj = NULL;
	ResponseStatus *res = rep->res;

	api_get_integer_option("firmware_info.hs_fw.upgrade_processing", &hs_upgrade_status);

	if(!hs_upgrade_status)
	{
		RET_GEN_ERRORMSG(res, API_SERVICE_ERROR, "NO FW UPGRADE PROCESSING");
	}
	
	/* get handset fw update status data */
	hs_num = getHandsetFwStatusData(hs_id, hs_progress, hs_status, hs_under_bs, bs_progress, bs_status);

	for(i = RAINIER_BSID_MIN; i <= RAINIER_BSID_MAX; i++)
	{
		if(bs_progress[i-1] > 0)
		{
			hs_upgrade_started = 1;
			break;
		}
	}
	if(!hs_upgrade_started)
	{
		RET_GEN_ERRORMSG(res, API_SERVICE_ERROR, "FW UPGRADE NOT YET STARTED");
	}

	for(i = RAINIER_BSID_MIN; i <= RAINIER_BSID_MAX; i++)
	{
		if(bs_progress[i-1] > 90)
		{
			RET_GEN_ERRORMSG(res, API_SERVICE_ERROR, "FW UPGRADE ALMOST DONE");
		}
	}

	for(i = RAINIER_BSID_MIN; i <= RAINIER_BSID_MAX; i++)
    {
		/* check the base is registered to BSC or not */
		idx = findBsConfigIdxByIndex(i);

		//if(idx == -1)
		//	RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "TARGET_BASE");
		
		if(idx != -1) // bs is in the list
		{
			ret = json_post_stop_hs_fw_upgrade(rep, query_str, idx);
			if(ret == API_SUCCESS)
			{
				status = API_SUCCESS;
			}
		}
	}

	if(status == -1)
	{
		RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "Unable to set hs_fw_upgrade_abort to Base");
	}

	api_set_integer_option("firmware_info.hs_fw.upgrade_processing", 0);
	api_commit_option("firmware_info");

	// NMS reset handset fw upgrade status
	//SYSTEM("nmsconf_cli bs resetHsFwStatus");
	
	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_post_mgm_fw_upgrade_scheduled(ResponseEntry *rep, char *query_str)
{
	//int base_id = -1;
	//int idx;
	char *upgrade_from_server=NULL, *mode=NULL;
	int hs_upgrade_status = 0, bs_upgrade_status = 0;
	char *scheduled_time = NULL;
	int target_base = 0;
	bool enable = 0, upgrade_bsc = 0, upgrade_bs = 0, upgrade_hs = 0;
	char upgrade_bsc_fw_version[32] = {0};
	char upgrade_bs_fw_version[32] = {0};
	char upgrade_hs_fw_version[32] = {0};
	char buf[128];
	int year, month, day, hour, minute, second;
	int ret;

	struct json_object *jobj = NULL;
	ResponseStatus *res = rep->res;

	api_get_integer_option("firmware_info.hs_fw.upgrade_processing", &hs_upgrade_status);
	api_get_integer_option("firmware_info.bs_fw.upgrade_processing", &bs_upgrade_status);

	if(hs_upgrade_status || bs_upgrade_status)
	{
		RET_GEN_ERRORMSG(res, API_SERVICE_ERROR, "FW UPGRADE PROCESSING");
	}

	if(NULL != query_str)
	{
		if((jobj = jsonTokenerParseFromStack(rep, query_str)))
		{
			senao_json_object_get_boolean(jobj, "enable", &enable);
			senao_json_object_get_boolean(jobj, "upgrade_bsc", &upgrade_bsc);
			senao_json_object_get_boolean(jobj, "upgrade_bs", &upgrade_bs);
			senao_json_object_get_boolean(jobj, "upgrade_hs", &upgrade_hs);

			senao_json_object_get_integer(jobj, "target_base", &target_base);

			senao_json_object_get_and_create_string(rep, jobj, "scheduled_time", &scheduled_time);
			senao_json_object_get_and_create_string(rep, jobj, "mode", &mode);
            // senao_json_object_get_boolean(jobj, "do_reset_after_upgrade", &doReset);
            if (strcmp(mode, "Upgrade_from_server") == 0)
            {
                senao_json_object_get_and_create_string(rep, jobj, "upgrade_from_server", &upgrade_from_server);
                //return json_fw_upgrade_fromserver(rep, upgrade_from_server);
				RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "Upgrade from server not yet support");
            }
            else if (strcmp(mode, "Upgrade_locally") == 0)
            {
                //burn_fw();
				// continue to post api to base
            }
			else
			{
				RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "MODE");
			}
		}
	}

	// check firmware valid if enable
	if(enable)
	{
		if((!upgrade_bsc) && (!upgrade_bs) && (!upgrade_hs))
			RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "ENABLE");	// enable, but bsc,bs,hs all disable

		if(upgrade_bsc)
		{
			if(api_get_string_option("firmware_info.938c.version", upgrade_bsc_fw_version, sizeof(upgrade_bsc_fw_version)) != API_RC_SUCCESS)
				RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "No BSC firmware");

			if(strcmp(upgrade_bsc_fw_version, "---") == 0)
				RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "No BSC firmware");

			if(sysIsFileExisted(FW_FILENAME_BSC) != TRUE)
				RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "No BSC firmware");
		}
		if(upgrade_bs)
		{
			if(!target_base)
				RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "TARGET BASE");

			if(api_get_string_option("firmware_info.938b.version", upgrade_bs_fw_version, sizeof(upgrade_bs_fw_version)) != API_RC_SUCCESS)
				RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "No BS firmware");
			
			if(strcmp(upgrade_bs_fw_version, "---") == 0)
				RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "No BS firmware");
			
			if(sysIsFileExisted(FW_FILENAME_BS) != TRUE)
				RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "No BS firmware");
		}
		if(upgrade_hs)
		{
			if(api_get_string_option("firmware_info.938a.version", upgrade_hs_fw_version, sizeof(upgrade_hs_fw_version)) != API_RC_SUCCESS)
				RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "No HS firmware");
			
			if(strcmp(upgrade_hs_fw_version, "---") == 0)
				RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "No HS firmware");

			if(sysIsFileExisted(FW_FILENAME_HS) != TRUE)
				RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "No HS firmware");
		}
	}

	// Remove original update time.
    system("sed -i '/rainier_fw_update/d' /etc/crontabs/root");

	if(enable)
	{
		/* format "YYYY-MM-DD HH:MM:SS" */
		ret = sscanf(scheduled_time, "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second);
	
		if (6 == ret && (2000 <= year && 3000 > year) && (1 <= month && 13 > month) && (1 <= day && 32 > day)
			&& (0 <= hour && 24 > hour) && (0 <= minute && 60 > minute))
		{
			// Add new update time.
			sprintf(buf,
					"echo \"%d %d %d %d * [[ \\$(date '+%%Y') == %d ]] && /bin/rainier_fw_update & \" >> /etc/crontabs/root",
					minute, hour, day, month, year);
			
			//debug_print("crontab[%s]\n", buf);
			system(buf);
		}
		else
		{
			RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "SCHEDULED TIME");
		}
	}

	// modify the data to be saved
	if(enable)
	{
		// if no upgrade bs, set target_base to 0
		if(!upgrade_bs)
		{
			target_base = 0;
		}
	}
	else
	{
		// if disable, set all data to 0
		upgrade_bsc = 0;
		upgrade_bs = 0;
		target_base = 0;
		upgrade_hs = 0;
		strcpy(scheduled_time, "");
	}
	

	// set uci config
#if 1
	api_set_string_option("firmware_info.scheduled", "fw_scheduled", sizeof("fw_scheduled"));
	api_set_integer_option("firmware_info.scheduled.enable", enable);
	api_set_string_option("firmware_info.scheduled.scheduled_time", scheduled_time, sizeof(scheduled_time));
	api_set_integer_option("firmware_info.scheduled.upgrade_bsc", upgrade_bsc);
	api_set_integer_option("firmware_info.scheduled.upgrade_bs", upgrade_bs);
	api_set_integer_option("firmware_info.scheduled.target_base", target_base);
	api_set_integer_option("firmware_info.scheduled.upgrade_hs", upgrade_hs);
	api_set_string_option("firmware_info.scheduled.mode", mode, sizeof(mode));
	api_commit_option("firmware_info");
#else
	SYSTEM("uci set firmware_info.scheduled=fw_scheduled");
	SYSTEM("uci set firmware_info.scheduled.enable=%d", enable);
	SYSTEM("uci set firmware_info.scheduled.scheduled_time='%s'", scheduled_time);
	SYSTEM("uci set firmware_info.scheduled.upgrade_bsc=%d", upgrade_bsc);
	SYSTEM("uci set firmware_info.scheduled.upgrade_bs=%d", upgrade_bs);
	SYSTEM("uci set firmware_info.scheduled.target_base=%d", target_base);
	SYSTEM("uci set firmware_info.scheduled.upgrade_hs=%d", upgrade_hs);
	SYSTEM("uci set firmware_info.scheduled.mode='%s'", mode);
	SYSTEM("uci commit firmware_info");
#endif

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_get_mgm_fw_upgrade_status(ResponseEntry *rep, struct json_object *jobj)
{
	//char buf[4] = {0};
	char cmd[1024] = {0};
	char current_bsc_fw_version[32] = {0};
	char current_bs_fw_version[RAINIER_BASE_NUM][32] = {0};
	char current_hs_fw_version[RAINIER_HS_NUMBER][32] = {0};
	char upgrade_bsc_fw_version[32] = {0};
	char upgrade_bs_fw_version[32] = {0};
	char upgrade_hs_fw_version[32] = {0};
	char fw_version_tmp[32] = {0};
	//int bs_index = 0;
	char scheduled_time[20] = {0};/* format "YYYY-MM-DD HH:MM:SS" */
	//char upgrade_from_server[32] = {0}, mode[32] = {0};;
	int target_base = 0;
	int enable = 0, upgrade_bsc = 0, upgrade_bs = 0, upgrade_hs = 0;
	int hs_upgrade_status = 0, bs_upgrade_status = 0;
	int i;
	int idx;
	int rainierReg = 0;
	char *pt;
	ResponseStatus *res = rep->res;

	struct json_object *jobj_scheduled = NULL;
	struct json_object *jobj_from_server = NULL;
	struct json_object *jarr_bs_fw = NULL, *jobj_bs_fw = NULL;
	struct json_object *jarr_hs_fw = NULL, *jobj_hs_fw = NULL;
	jarr_bs_fw = json_object_new_array();
	jarr_hs_fw = json_object_new_array();

    
#if 1
    if(sys_interact(current_bsc_fw_version, sizeof(current_bsc_fw_version), "cat /etc/version | grep Firmware | awk \'BEGIN{FS= \" \"} {print $4}\'") > 0)
    {
        if ( (pt = strstr(current_bsc_fw_version, "\n")) ) { /* delete tail "\n" */
            *pt = '\0';
        }
    }
    else
    {
        snprintf(current_bsc_fw_version, sizeof(current_bsc_fw_version), "%s", "unknown");
    }
	// add version prefix with letter 'v'
	if((current_bsc_fw_version[0] >= '0') && (current_bsc_fw_version[0] <= '9'))
	{
		strncpy(fw_version_tmp, current_bsc_fw_version, sizeof(fw_version_tmp));
		snprintf(current_bsc_fw_version, sizeof(current_bsc_fw_version), "v%s", fw_version_tmp);
	}

	for(i = RAINIER_BSID_MIN; i <= RAINIER_BSID_MAX; i++)
    {
		/* check the base is registered to BSC or not */
		idx = findBsConfigIdxByIndex(i);
		if(idx != -1) // bs is in the list
		{
			//snprintf(cmd, sizeof(cmd), "uci get base-station-list.@base-station[%d].fw_version | tr -d \"\\n\"", idx);
			//sys_interact(current_bs_fw_version[i-1], sizeof(current_bs_fw_version[i-1]), cmd);
			api_get_string_option2(current_bs_fw_version[i-1], sizeof(current_bs_fw_version[i-1]), "base-station-list.@base-station[%d].fw_version", idx);
			
			// add version prefix with letter 'v'
			if((current_bs_fw_version[i-1][0] >= '0') && (current_bs_fw_version[i-1][0] <= '9'))
			{
				strncpy(fw_version_tmp, current_bs_fw_version[i-1], sizeof(fw_version_tmp));
				snprintf(current_bs_fw_version[i-1], sizeof(current_bs_fw_version[i-1]), "v%s", fw_version_tmp);
			}
		}
	}

#if !SUPPORT_SP938BS_OPENAPI_SERVER
	for(i = RAINIER_HSID_MIN; i <= RAINIER_HSID_MAX; i++)
    {
		// check hs_id is registered or not.
		if(api_get_bool_option2(&rainierReg, "sip_hs.sip_hs_%d.rainierReg", i) != API_RC_SUCCESS)
		{
			RET_GEN_ERRORMSG(res, API_INTERNAL_ERROR, "GET HS REG");
		}
		if(rainierReg == 1)
		{
			api_get_string_option2(current_hs_fw_version[i-RAINIER_HSID_MIN], sizeof(current_hs_fw_version[i-RAINIER_HSID_MIN]), "sip_hs.sip_hs_%d.fw_version", i);

			// add version prefix with letter 'v'
			if((current_hs_fw_version[i-RAINIER_HSID_MIN][0] >= '0') && (current_hs_fw_version[i-RAINIER_HSID_MIN][0] <= '9'))
			{
				strncpy(fw_version_tmp, current_hs_fw_version[i-RAINIER_HSID_MIN], sizeof(fw_version_tmp));
				snprintf(current_hs_fw_version[i-RAINIER_HSID_MIN], sizeof(current_hs_fw_version[i-RAINIER_HSID_MIN]), "v%s", fw_version_tmp);
			}
		}
	}
#endif

	if((api_get_string_option("firmware_info.938c.version", upgrade_bsc_fw_version, sizeof(upgrade_bsc_fw_version)) != API_RC_SUCCESS) ||
		(sysIsFileExisted(FW_FILENAME_BSC) != TRUE))
		strcpy(upgrade_bsc_fw_version, "---");

	if((api_get_string_option("firmware_info.938b.version", upgrade_bs_fw_version, sizeof(upgrade_bs_fw_version)) != API_RC_SUCCESS) ||
		(sysIsFileExisted(FW_FILENAME_BS) != TRUE))
		strcpy(upgrade_bs_fw_version, "---");

	if((api_get_string_option("firmware_info.938a.version", upgrade_hs_fw_version, sizeof(upgrade_hs_fw_version)) != API_RC_SUCCESS) ||
		(sysIsFileExisted(FW_FILENAME_HS) != TRUE))
		strcpy(upgrade_hs_fw_version, "---");
	
	// add version prefix with letter 'v'
	if((upgrade_bsc_fw_version[0] >= '0') && (upgrade_bsc_fw_version[0] <= '9'))
	{
		strncpy(fw_version_tmp, upgrade_bsc_fw_version, sizeof(fw_version_tmp));
		snprintf(upgrade_bsc_fw_version, sizeof(upgrade_bsc_fw_version), "v%s", fw_version_tmp);
	}
	if((upgrade_bs_fw_version[0] >= '0') && (upgrade_bs_fw_version[0] <= '9'))
	{
		strncpy(fw_version_tmp, upgrade_bs_fw_version, sizeof(fw_version_tmp));
		snprintf(upgrade_bs_fw_version, sizeof(upgrade_bs_fw_version), "v%s", fw_version_tmp);
	}
	if((upgrade_hs_fw_version[0] >= '0') && (upgrade_hs_fw_version[0] <= '9'))
	{
		strncpy(fw_version_tmp, upgrade_hs_fw_version, sizeof(fw_version_tmp));
		snprintf(upgrade_hs_fw_version, sizeof(upgrade_hs_fw_version), "v%s", fw_version_tmp);
	}

	api_get_bool_option("firmware_info.scheduled.enable", &enable);
	api_get_bool_option("firmware_info.scheduled.upgrade_bsc", &upgrade_bsc);
	api_get_bool_option("firmware_info.scheduled.upgrade_bs", &upgrade_bs);
	api_get_bool_option("firmware_info.scheduled.upgrade_hs", &upgrade_hs);
	
	api_get_integer_option("firmware_info.scheduled.target_base", &target_base);
	
	api_get_string_option("firmware_info.scheduled.scheduled_time", scheduled_time, sizeof(scheduled_time));

	api_get_integer_option("firmware_info.hs_fw.upgrade_processing", &hs_upgrade_status);

	api_get_integer_option("firmware_info.bs_fw.upgrade_processing", &bs_upgrade_status);
	
	if(sysIsFileExisted(FW_FILENAME_BSC) != TRUE)
		upgrade_bsc = 0;
	if(sysIsFileExisted(FW_FILENAME_BS) != TRUE)
		upgrade_bs = 0;
	if(sysIsFileExisted(FW_FILENAME_HS) != TRUE)
		upgrade_hs = 0;

#else
	/* gen test data */
	strcpy(current_bsc_fw_version, "1.2.0");
	for(i = RAINIER_BSID_MIN; i <= RAINIER_BSID_MAX; i++)
	{
		strcpy(current_bs_fw_version[i-1], "2.2.0");
	}
	strcpy(upgrade_bsc_fw_version, "2.0.0");
	strcpy(upgrade_bs_fw_version, "3.0.0");
	strcpy(upgrade_hs_fw_version, "2.5.0");

	strcpy(scheduled_time, "2020-11-12 13:14:15");
#endif

#if SUPPORT_SP938BS_OPENAPI_SERVER
	json_object_object_add(jobj, "current_bsc_fw_version", json_object_new_string("--"));

	jobj_bs_fw = json_object_new_object();
	json_object_object_add(jobj_bs_fw, "bs_index", json_object_new_int(RAINIER_BSID_MIN));
	json_object_object_add(jobj_bs_fw, "fw_version", json_object_new_string(current_bsc_fw_version));
	json_object_array_add(jarr_bs_fw, jobj_bs_fw);
	json_object_object_add(jobj, "current_bs_fw_version", jarr_bs_fw);

	json_object_object_add(jobj, "current_hs_fw_version", jarr_hs_fw);

	if(sysIsFileExisted(SP938BS_LOCAL_FW_FILENAME) != TRUE)
	{

		debug_print(" Jason DEBUG %s[%d] no file[%s]!! \n", __FUNCTION__, __LINE__, SP938BS_LOCAL_FW_FILENAME);
		memset(upgrade_bs_fw_version, 0 , sizeof(upgrade_bs_fw_version));
		strcpy(upgrade_bs_fw_version, "---");
	}
	else
	{	
		debug_print(" Jason DEBUG %s[%d] file[%s] exist\n", __FUNCTION__, __LINE__, SP938BS_LOCAL_FW_FILENAME);
	}
#else
	json_object_object_add(jobj, "current_bsc_fw_version", json_object_new_string(current_bsc_fw_version));

	for(i = RAINIER_BSID_MIN; i <= RAINIER_BSID_MAX; i++)
    {
		if(findBsConfigIdxByIndex(i) != -1) // bs is in the list
		{
			jobj_bs_fw = json_object_new_object();
			json_object_object_add(jobj_bs_fw, "bs_index", json_object_new_int(i));
			json_object_object_add(jobj_bs_fw, "fw_version", json_object_new_string(current_bs_fw_version[i-1]));
			json_object_array_add(jarr_bs_fw, jobj_bs_fw);
		}
	}
	json_object_object_add(jobj, "current_bs_fw_version", jarr_bs_fw);

	for(i = RAINIER_HSID_MIN; i <= RAINIER_HSID_MAX; i++)
    {
		// check hs_id is registered or not.
		if(api_get_bool_option2(&rainierReg, "sip_hs.sip_hs_%d.rainierReg", i) != API_RC_SUCCESS)
		{
			RET_GEN_ERRORMSG(res, API_INTERNAL_ERROR, "GET HS REG");
		}
		if(rainierReg == 1)
		{
			jobj_hs_fw = json_object_new_object();
			json_object_object_add(jobj_hs_fw, "hs_id", json_object_new_int(i));
			json_object_object_add(jobj_hs_fw, "fw_version", json_object_new_string(current_hs_fw_version[i-RAINIER_HSID_MIN]));
			json_object_array_add(jarr_hs_fw, jobj_hs_fw);
		}
	}
	json_object_object_add(jobj, "current_hs_fw_version", jarr_hs_fw);
#endif

	json_object_object_add(jobj, "upgrade_bsc_fw_version", json_object_new_string(upgrade_bsc_fw_version));
	json_object_object_add(jobj, "upgrade_bs_fw_version", json_object_new_string(upgrade_bs_fw_version));
	json_object_object_add(jobj, "upgrade_hs_fw_version", json_object_new_string(upgrade_hs_fw_version));

	jobj_from_server = json_object_new_object();
	json_object_object_add(jobj_from_server, "upgrade_bsc_url", json_object_new_string("string"));
	json_object_object_add(jobj_from_server, "upgrade_bs_url", json_object_new_string("string"));
	json_object_object_add(jobj_from_server, "upgrade_hs_url", json_object_new_string("string"));

	jobj_scheduled = json_object_new_object();
	json_object_object_add(jobj_scheduled, "enable", json_object_new_boolean(enable));
	json_object_object_add(jobj_scheduled, "scheduled_time", json_object_new_string(scheduled_time));
	json_object_object_add(jobj_scheduled, "upgrade_bsc", json_object_new_boolean(upgrade_bsc));
	json_object_object_add(jobj_scheduled, "upgrade_bs", json_object_new_boolean(upgrade_bs));
	json_object_object_add(jobj_scheduled, "target_base", json_object_new_int(target_base));
	json_object_object_add(jobj_scheduled, "upgrade_hs", json_object_new_boolean(upgrade_hs));
	json_object_object_add(jobj_scheduled, "mode", json_object_new_string("Upgrade_locally"));
	json_object_object_add(jobj_scheduled, "upgrade_from_server", jobj_from_server);
	json_object_object_add(jobj, "upgrade_scheduled", jobj_scheduled);

	json_object_object_add(jobj, "hs_upgrade_status", json_object_new_int(hs_upgrade_status));
	json_object_object_add(jobj, "bs_upgrade_status", json_object_new_int(bs_upgrade_status));

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_get_mgm_fw_upgrade_status_hs(ResponseEntry *rep, struct json_object *jobj)
{
	//char buf[4] = {0};
	char cmd[1024] = {0};
	ResponseStatus *res = rep->res;

    int i, result;
	int hs_id[RAINIER_HS_NUMBER] = {0};
	int hs_progress[RAINIER_HS_NUMBER] = {0};
	int hs_status[RAINIER_HS_NUMBER] = {0};
	int hs_under_bs[RAINIER_HS_NUMBER] = {0};
	int bs_progress[RAINIER_BASE_NUM] = {0};
	int bs_status[RAINIER_BASE_NUM] = {0};
	int hs_num = 0;
	int idx;
	int rainierReg = 0;

	struct json_object *jarr_handset_list = NULL, *jobj_handset_list = NULL;
	jarr_handset_list = json_object_new_array();

	struct json_object *jarr_base_list = NULL, *jobj_base_list = NULL;
	jarr_base_list = json_object_new_array();

	/* get handset fw update status data */
	hs_num = getHandsetFwStatusData(hs_id, hs_progress, hs_status, hs_under_bs, bs_progress, bs_status);

	/* Get all base list */
	for(i = RAINIER_BSID_MIN; i <= RAINIER_BSID_MAX; i++)
	{
		if(findBsConfigIdxByIndex(i) != -1) // bs is in the list
		{
			jobj_base_list = json_object_new_object();
			json_object_object_add(jobj_base_list, "bs_id", json_object_new_int(i));
			json_object_object_add(jobj_base_list, "bs_hs_fw_progress", json_object_new_int(bs_progress[i-RAINIER_BSID_MIN]));
			json_object_object_add(jobj_base_list, "bs_status", json_object_new_int(bs_status[i-RAINIER_BSID_MIN]));
			json_object_array_add(jarr_base_list, jobj_base_list);
		}
	}

	json_object_object_add(jobj, "base_list", jarr_base_list);

	/* Get all handset list */
	for(i = RAINIER_HSID_MIN; i <= RAINIER_HSID_MAX; i++)
	{
		// check hs_id is registered or not.
		if(api_get_bool_option2(&rainierReg, "sip_hs.sip_hs_%d.rainierReg", i) != API_RC_SUCCESS)
		{
			RET_GEN_ERRORMSG(res, API_INTERNAL_ERROR, "GET HS REG");
		}
		if(rainierReg == 1)
		{
			jobj_handset_list = json_object_new_object();
			json_object_object_add(jobj_handset_list, "hs_id", json_object_new_int(i));
			json_object_object_add(jobj_handset_list, "hs_fw_progress", json_object_new_int(hs_progress[i-RAINIER_HSID_MIN]));
			json_object_object_add(jobj_handset_list, "hs_status", json_object_new_int(hs_status[i-RAINIER_HSID_MIN]));
			json_object_array_add(jarr_handset_list, jobj_handset_list);
		}
	}

	json_object_object_add(jobj, "handset_list", jarr_handset_list);

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

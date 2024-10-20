#ifndef _JSON_RAINIER_H_
#define _JSON_RAINIER_H_


#include <json_object.h>
#include <json_tokener.h>
#include <api_response.h>
#include <time.h>

/* definition for rainier system(SP938) */
#define RAINIER_BASE_NUM       8
#define RAINIER_BSID_MIN       1
#define RAINIER_BSID_MAX       8

#define RAINIER_PHONEBOOK_NUM  90
#define RAINIER_PHONEBOOK_MIN  1
#define RAINIER_PHONEBOOK_MAX  90

#define RAINIER_HS_NUMBER      90
#define RAINIER_HSID_MIN       10
#define RAINIER_HSID_MAX       99
#define RAINIER_GROUP_NUMBER   7

#define RAINIER_SIP_ACC_MIN    0
#define RAINIER_SIP_ACC_MAX    99
#define RAINIER_SPI_ACC_NUM    100

#define RAINIER_FXO_MIN         0
#define RAINIER_FXO_MAX         7
#define RAINIER_FXO_NUM         8

#define RAINIER_GROUP_MIN       0
#define RAINIER_GROUP_MAX       7
#define RAINIER_GROUP_NUM       8

#define RAINIER_TRUNK_MIN       0
#define RAINIER_TRUNK_MAX       31
#define RAINIER_TRUNK_NUM       32

#define RAINIER_LOG_PAGE_SIZE  50

#define MODE_NORMAL            0
#define MODE_REG               1
#define MODE_DEREG             2

#define REG_DEREG_SUCCESS      1
#define REG_DEREG_FAIL         2

#define HS_UPDATE_IDLE                      0
#define HS_UPDATE_PROCESSING                1
#define HS_UPDATE_SUCCESS                   2
#define HS_UPDATE_FAIL_BS_OFFLINE           3
#define HS_UPDATE_FAIL_HS_NOT_FOUND         4
#define HS_UPDATE_FAIL_HS_BUSY              5
#define HS_UPDATE_FAIL_HS_LINK_LOSS         6
#define HS_UPDATE_FAIL_HS_FAIL2UPDATE       7

#define HS_RESULT_SUCCESS      0
#define HS_RESULT_NOT_FOUND    1
#define HS_RESULT_BUSY         2
#define HS_RESULT_LINK_LOSS    3
#define HS_RESULT_FAIL2UPDATE  4

#define HS_FLAG_GROUP_BIT      0
#define HS_FLAG_BASE_BIT       1
#define HS_FLAG_NAME_BIT       2

#define TOP_CALL_TYPE_ALL		0
#define TOP_CALL_TYPR_FXO		1
#define TOP_CALL_TYPE_ALL_NUMBER	2
#define TOP_CALL_TYPR_FXO_NUMBER 	3
#define TOP_USE_BASE_TYPE_NUMBER 	4
#define DISTRIBUTION_CALL_SIP 		5
#define DISTRIBUTION_CALL_FXO 		6

#define GRAPH_CALL_TIME		0
#define GRAPH_CALL_NUMBER	1

#define CDR_TYPE_EMPTY 		0
#define CDR_TYPE_IN 		1
#define CDR_TYPE_OUT 		2
#define CDR_TYPE_INTERCOM 	3
#define CDR_TYPE_BRAODCAST 	4


/* MACRO DEFINE */
#define msleep(x)		usleep(x*1000)




/* bsinfo */

#define DB_TIME_FORMAT "%d-%d-%d %d:%d:%d"
#define COLUMN_TIME_STR "time"
#define DB_BSINFO_PATH "/root/db_files/bsinfo.db"
#define DB_PTP_PATH "/root/db_files/ptp.db"
#define DB_CDR_PATH "/root/db_files/master.db"
#define DB_CALL_OVERVIEW_PATH "/root/db_files/call_overview.db"
#define DB_BSINFO_TABLE "bsinfo"
#define OVERVIEW_TABLE_NAME "call_log"
#define DB_PTP_SYNC_TABLE "ptp_sync_tb"
#define DB_CDR_TABLE "cdr"
#define AIR_USE_COLUMN "air_inuse"
#define BS_BUSY_COLUMN "bs_busy"
#define BS_PTP_STS_COLUMN "ptp_sync"
#define BS_HANDOVER_HSID_COLUMN "handover_hsid"
#define SIP_REG_FAIL_COLUMN "sip_reg_fail"
#define SIP_CALL_DIR_COLUMN "dir"
#define DB_CDR_DURATION "duration"
#define DB_CDR_TRUNK "trunk"


#define OVERVIEW_LAST_HOUR 	0
#define OVERVIEW_LAST_DAY 	1
#define OVERVIEW_LAST_WEEK 	2
#define GRAPH_INTARVAL_SEC	3

#define NUM_LAST_WEEK		56
#define NUM_LAST_DAY		48
#define NUM_LAST_HOUR		60

#define OVERVIEW_COL_HSID		"hsid"
#define OVERVIEW_COL_BSID		"bsid"
#define OVERVIEW_COL_TYPE		"type"
#define OVERVIEW_COL_DURATION	"duration"
#define OVERVIEW_COL_TIME		"time"


#define RAINIER_CALL_TYPE_FXO		1


//#define SWITCH_IP		"192.168.0.239"		// For EnBSC
#define SWITCH_IP		"192.168.255.2"		// For SP938BSC


typedef struct db_time
{
	char *column_str;
	signed long total_sec;
	char last_time[32];
	char current_time[32];
	int last_sts;
	int current_sts;
	struct tm start_tm;
	struct tm end_tm;
	int t_hr;
	int t_mi;
	int t_sc;
	int first_sts;
	char first_sts_time[32];
}db_time_t;



typedef struct db_count
{
	char *column_str;
	int total_count;
        int check_val;	
}db_count_t;

typedef struct call_top_time
{
	int hs_id;
	int time;
}call_top_time_t;

typedef struct overview_data
{
	int range;
	int select_base;
}overview_data_t;

#if 0
#define CALL_GRAPH_NUM 50
#else
#define CALL_GRAPH_NUM get_call_graph_num()
#endif
#define CALL_TIME_NUM 10

typedef struct graph_call_data
{
	int index;
	char start_time[32];
	char end_time[32];
	time_t start_s;
	time_t end_s;
	int time;
	int number;
}graph_call_data_t;

typedef struct overview_db_data
{
	int hsid;
	int bsid;
	int call_type;
	char duration[64];
	char time[64];
}overview_db_data_t;
#if 0
typedef struct call_graph
{
	int index;
	int call_time;
	int call_number;
}call_graph_t;
#endif
typedef struct call_date
{
	unsigned int call_time;
	unsigned int call_number;
	unsigned int fxo_time;
	unsigned int fxo_number;
}call_date_t;


typedef struct call_distribution
{
	int total_sip_call;
	int total_fxo_call;
}call_distribution_t;

typedef struct over_view_info
{
#define OVERVIEW_TOP_NUM	10
	unsigned int 		sql_callback;
	overview_data_t		*overview_search;
	graph_call_data_t 	*graph_data;
	call_distribution_t	distribution;
	call_top_time_t		call_time[RAINIER_HSID_MAX];
	call_top_time_t		call_number[RAINIER_HSID_MAX];
	call_top_time_t		fxo_call_time[RAINIER_HSID_MAX];
	call_top_time_t		fxo_call_number[RAINIER_HSID_MAX];
	call_top_time_t		use_bs_number[RAINIER_HSID_MAX];
}overview_info_t;

typedef struct db_rssi
{
	char  	time_str[32];
	int 	hsid;
	int 	bsid;
	char 	rssi[8];
	time_t 	date_time;
}db_rssi_t;

#define DEAD                         0
#define NMSRPT_HS_IDLE               1
#define NMSRPT_HS_SIP_RING           2  /* HS acknowledged */
#define NMSRPT_HS_SIP_TALK           3
#define NMSRPT_HS_SIP_HOLD           4	/* Reserved */
#define NMSRPT_HS_SFXO_RING          5	/* It's SIP-FXO. Pure FXO is used only in network unavailable, so it won't be shown here. */
#define NMSRPT_HS_SFXO_TALK          6	/* It's SIP-FXO. Pure FXO is used only in network unavailable, so it won't be shown here. */
#define NMSRPT_HS_SFXO_HOLD          7	/* Reserved */
#define NMSRPT_HS_SERVICE_LINK_BS    16	/* HS do service link with BS. */


#define RA_DEBUG_PRINT_FILE	"/tmp/ra_print_enable"

enum {
    CT_NONE = 0,
    CT_FXO,	/* Note: FXO type won't be reported to BSC since it is only available in fail of network state on BS. */
    CT_SIP,
    CT_SIPFXO,	/* FXO call will be routed through SIP when network is available on BS.  */
    CT_H2H_INT_TRU_BS, /* HS to HS intercom through BS. */
    CT_H2H_PA_TRU_BS, /*5, HS to HS broadcast (PA) through BS.*/
    CT_F2H_PA_TRU_BS, /* FXO to HS broadcast (PA) through BS.*/
    CT_MAX
};

typedef struct _sip_call_info_t {
	unsigned char	call_in_used;	//call in used: 0 --no information below, 1 -- the information below is meaningful
	//unsigned char	account_id;	//this account is used by the hsId in slot call structure (slot_call_info_t) below
	unsigned char	type;		//call type
	unsigned char	state;		//call state: 0-idle, 1-ring, 2-talk, 3-hold
	unsigned char 	dir;		//call direction: 0--outgoing call, 1--incoming call, 2--this call is a handover call, the caller info below is not correct.
	//char			caller_info[43]; //should refer to dir variable above: 0--outgoing call number dialed or callee information, 1--incoming caller information
	char			num[32];	//outgoing call number dialed or incoming caller number
	char			name[16];	//incoming caller name
} sip_call_info_t;

typedef struct _slot_call_info_t {
	unsigned char	slot_in_used;	//slot in used: 0 --information below is unused, 1--information below is used or established
	unsigned char	hsId;		//this handset uses the acountId in sip call structure (sip_call_info_t) above
	//unsigned char   slot;		//current slot is in used by logical
	sip_call_info_t call[2];	//Max. 2 calls in each slot linked
} slot_call_info_t;

typedef struct _bs_call_info_t {
	slot_call_info_t slot[4];	//Max. 4 call slots established in a base
} bs_call_info_t;

typedef struct _hs_call_info_t {
	unsigned char	status;		//handset status
	sip_call_info_t call[2];	//Max. 2 calls in each slot linked
} hs_call_info_t;

typedef struct _process_table_t {
	char	name[64];			//process name
	char	description[64];	//process description
	char	restart_script[64];	//process restart script
} process_table_t;


int json_get_sys_info_rainier(ResponseEntry *rep, struct json_object *jobj);
int json_post_sys_time_config(ResponseEntry *rep, char *query_str);
int json_get_sys_time_config(ResponseEntry *rep, struct json_object *jobj);

int json_get_ethernet_rainier(ResponseEntry *rep, struct json_object *jobj);
int json_set_ethernet_rainier(ResponseEntry *rep, char *query_str);

int json_get_mgm_process_status(ResponseEntry *rep, struct json_object *jobj);
int json_set_mgm_process_restart(ResponseEntry *rep, char *query_str);

int json_get_mgm_port_status(ResponseEntry *rep, struct json_object *jobj);
int json_get_mgm_sdcard_file_list(ResponseEntry *rep, struct json_object *jobj);
int json_post_mgm_backup_database_sdcard(ResponseEntry *rep, char *query_str);
int json_post_mgm_restore_database_sdcard(ResponseEntry *rep, char *query_str);
int json_post_mgm_sdcard_remove_file(ResponseEntry *rep, char *query_str);
int json_post_mgm_backup_config_sdcard(ResponseEntry *rep, char *query_str);
int json_post_mgm_restore_config_sdcard(ResponseEntry *rep, char *query_str);
int json_post_mgm_backup_database(ResponseEntry *rep, char *query_str);
int json_get_mgm_backup_database(ResponseEntry *rep, struct json_object *jobj);
int json_post_mgm_restore_database(ResponseEntry *rep, char *query_str);
int json_post_mgm_clear_database(ResponseEntry *rep, char *query_str);
#if SUPPORT_SP938BS_OPENAPI_SERVER
/* mgm/rainier */
int json_post_mgm_rainier_pb_transfer(ResponseEntry *rep, char *query_str);
int json_post_mgm_rainier_cfg_reload(ResponseEntry *rep, char *query_str);
int json_post_mgm_rainier_post_base_reg_mode(ResponseEntry *rep, char *query_str);
int json_post_mgm_bs_fw_upgrade(ResponseEntry *rep, char *query_str);
int json_post_mgm_hs_fw_upgrade(ResponseEntry *rep, char *query_str);
int json_post_mgm_hs_fw_upgrade_abort(ResponseEntry *rep, char *query_str);
int json_post_rainier_bind_bs(ResponseEntry *rep, char *query_str);
int json_patch_rainier_hs_cfg_update(ResponseEntry *rep, char *query_str, int hs_idx);
#endif  /* #if SUPPORT_SP938BS_OPENAPI_SERVER */

int json_post_mgm_rainier_log_search_range(ResponseEntry *rep, char *query_str);
int json_post_mgm_rainier_log(ResponseEntry *rep, char *query_str);
int json_get_mgm_rainier_log(ResponseEntry *rep, struct json_object *jobj, int page_idx);
//int json_get_mgm_rainier_log_idx(ResponseEntry *rep, struct json_object *jobj, int log_idx);
//int json_get_mgm_rainier_log_of_base(ResponseEntry *rep, struct json_object *jobj, int base_idx, int page_idx);
//int json_get_mgm_rainier_log_of_base_idx(ResponseEntry *rep, struct json_object *jobj, int base_idx, int log_idx);

int json_post_mgm_rainier_reboot_base(ResponseEntry *rep, char *query_str, int base_idx);

int json_post_rainier_fxo_idx(ResponseEntry *rep, char *query_str, int fxo_idx);
int json_get_rainier_fxo(ResponseEntry *rep, struct json_object *jobj);
int json_get_rainier_fxo_idx(ResponseEntry *rep, struct json_object *jobj, int fxo_idx);

#if 0   // the base_list function is implemented by base_station_list, this api can be removed or need to modify it to be an api to get base_info
int json_get_rainier_base_list(ResponseEntry *rep, struct json_object *jobj);
int json_get_rainier_base_list_idx(ResponseEntry *rep, struct json_object *jobj, int bs_idx);
#endif

int json_post_rainier_statistic_search_range(ResponseEntry *rep, char *query_str);
int json_get_rainier_all_base_statistic(ResponseEntry *rep, struct json_object *jobj);
int json_post_rainier_base_statistic(ResponseEntry *rep, char *query_str, int index);
int json_get_rainier_base_statistic(ResponseEntry *rep, struct json_object *jobj, int index);
int json_get_rainier_traffic_statistic(ResponseEntry *rep, struct json_object *jobj);
int json_get_rainier_cdr_info(ResponseEntry *rep, struct json_object *jobj, int page_idx);

int json_get_rainier_handset_rssi_list(ResponseEntry *rep, struct json_object *jobj);
int json_post_rainier_bsRxhsRssi(ResponseEntry *rep, char *query_str);
int json_post_rainier_hsRxbsRssi(ResponseEntry *rep, char *query_str);
int json_get_rainier_bs_rssi_info(ResponseEntry *rep, struct json_object *jobj, int index);
int json_get_rainier_hs_rssi_info(ResponseEntry *rep, struct json_object *jobj, int index);
/* get/post hsid */
int json_post_hsid(ResponseEntry *rep, char *query_str);
int json_get_hsid(ResponseEntry *rep, struct json_object *jobj);
/* get ASTERISK CD data */
int json_get_cdr(ResponseEntry *rep, struct json_object *jobj, int index);

int json_post_push_phbook(ResponseEntry *rep, char *query_str);
/* get/post all phonebook */
int json_post_phbooks(ResponseEntry *rep, char *query_str);
int json_get_phbooks(ResponseEntry *rep, struct json_object *jobj);
/* get/post specific phonebook */
int json_post_phbook_idx(ResponseEntry *rep, char *query_str, int phbook_idx);
int json_get_phbook_idx(ResponseEntry *rep, struct json_object *jobj, int phbook_idx);
int json_post_add_phbook(ResponseEntry *rep, char *query_str);
int json_post_delete_phbooks(ResponseEntry *rep, char *query_str);
/* get/post all base info */
int json_post_bsids(ResponseEntry *rep, char *query_str);
int json_get_bsids(ResponseEntry *rep, struct json_object *jobj);
/* get/post specific base info */
int json_post_bsid_idx(ResponseEntry *rep, char *query_str, int base_idx);
int json_get_bsid_idx(ResponseEntry *rep, struct json_object *jobj, int base_idx);

int json_post_rainier_basic_base(ResponseEntry *rep, char *query_str);
int json_get_rainier_basic_base(ResponseEntry *rep, struct json_object *jobj);
int json_post_rainier_voip_basic(ResponseEntry *rep, char *query_str);
int json_get_rainier_voip_basic(ResponseEntry *rep, struct json_object *jobj);
int json_post_rainier_sip_acc(ResponseEntry *rep, char *query_str, int sip_acc_idx);
int json_get_rainier_sip_acc(ResponseEntry *rep, struct json_object *jobj, int sip_acc_idx);

#if 0   // unused
int json_get_rainier_sip_fxo_acc(ResponseEntry *rep, struct json_object *jobj);
int json_get_rainier_sip_group_acc(ResponseEntry *rep, struct json_object *jobj);
#endif
int json_post_rainier_sip_trunk_acc(ResponseEntry *rep, char *query_str);
int json_get_rainier_sip_trunk_acc(ResponseEntry *rep, struct json_object *jobj);
int json_post_rainier_sip_trunk_acc_idx(ResponseEntry *rep, char *query_str, int sip_acc_idx);
int json_get_rainier_sip_trunk_acc_idx(ResponseEntry *rep, struct json_object *jobj, int sip_acc_idx);
int json_post_add_rainier_sip_trunk_acc(ResponseEntry *rep, char *query_str);
int json_post_delete_rainier_sip_trunk_acc(ResponseEntry *rep, char *query_str);

int json_post_rainier_sip_acc_type_idx(ResponseEntry *rep, char *query_str, char *sip_acc_type, int sip_acc_idx);
int json_get_rainier_sip_acc_type_idx(ResponseEntry *rep, struct json_object *jobj, char *sip_acc_type, int sip_acc_idx);

int json_post_rainier_daa(ResponseEntry *rep, char *query_str);
int json_get_rainier_daa(ResponseEntry *rep, struct json_object *jobj);
int json_post_rainier_fxo_setting(ResponseEntry *rep, char *query_str);
int json_get_rainier_fxo_setting(ResponseEntry *rep, struct json_object *jobj);
int json_post_rainier_codec(ResponseEntry *rep, char *query_str);
int json_get_rainier_codec(ResponseEntry *rep, struct json_object *jobj);

int json_post_rainier_bind_bs_info(ResponseEntry *rep, char *query_str);
int json_post_rainier_remove_bs_info(ResponseEntry *rep, char *query_str);
int json_get_rainier_base_station_list(ResponseEntry *rep, struct json_object *jobj);
int json_post_rainier_base_station_list_idx(ResponseEntry *rep, char *query_str, int bs_idx);
int json_get_rainier_base_station_list_idx(ResponseEntry *rep, struct json_object *jobj, int bs_idx);
int json_post_rainier_notify_bsc(ResponseEntry *rep, char *query_str);

int json_post_rainier_hs_reg(ResponseEntry *rep, char *query_str);
int json_post_rainier_hs_remove(ResponseEntry *rep, char *query_str);

int json_get_rainier_topology(ResponseEntry *rep, struct json_object *jobj);

int json_get_rainier_handset_list(ResponseEntry *rep, struct json_object *jobj);
int json_post_rainier_handset_list_idx(ResponseEntry *rep, char *query_str, int hs_idx);
int json_patch_rainier_handset_list_idx(ResponseEntry *rep, char *query_str, int hs_idx);
int json_get_rainier_handset_list_idx(ResponseEntry *rep, struct json_object *jobj, int hs_idx);
int json_get_rainier_handset_list_of_base(ResponseEntry *rep, struct json_object *jobj, int bs_idx);
int json_post_rainier_hs_config_result(ResponseEntry *rep, char *query_str);

int json_get_rainier_base_status(ResponseEntry *rep, struct json_object *jobj);
int json_get_rainier_base_call_status(ResponseEntry *rep, struct json_object *jobj);
int json_get_rainier_handset_status(ResponseEntry *rep, struct json_object *jobj);
int json_post_rainier_call_overview_range(ResponseEntry *rep, char *query_str);
int json_get_rainier_call_overview(ResponseEntry *rep, struct json_object *jobj);

int json_post_rainier_broadcast_setting(ResponseEntry *rep, char *query_str);
int json_get_rainier_broadcast_setting(ResponseEntry *rep, struct json_object *jobj);

int json_get_rainier_base_name(ResponseEntry *rep, struct json_object *jobj);
int json_get_rainier_base_name_idx(ResponseEntry *rep, struct json_object *jobj, int bs_idx);
int json_get_rainier_handset_name(ResponseEntry *rep, struct json_object *jobj);
int json_get_rainier_handset_name_idx(ResponseEntry *rep, struct json_object *jobj, int hs_idx);

//int json_post_start_bs_fw_upgrade(ResponseEntry *rep, char *query_str, int bs_idx);
//int json_post_start_hs_fw_upgrade(ResponseEntry *rep, char *query_str, int bs_idx);
int json_post_mgm_fw_upgrade_bsc(ResponseEntry *rep, char *query_str);
int json_post_mgm_fw_upgrade_bs(ResponseEntry *rep, char *query_str);
int json_post_mgm_fw_upgrade_hs(ResponseEntry *rep, char *query_str);
int json_post_mgm_fw_upgrade_hs_abort(ResponseEntry *rep, char *query_str);
int json_post_mgm_fw_upgrade_scheduled(ResponseEntry *rep, char *query_str);
int json_get_mgm_fw_upgrade_status(ResponseEntry *rep, struct json_object *jobj);
int json_get_mgm_fw_upgrade_status_hs(ResponseEntry *rep, struct json_object *jobj);
char* get_rssi_by_time(char *time, int bsid, int hsid);
char* get_grp_in_rssi_by_time(char *time, int bsid);
#endif

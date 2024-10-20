/*******************************************************************************************
 * Copyright (c) 2018 Ryan Hsu @ SenaoNetworks - Taiwan                                    *
 *                                                                                         *
 * This code has to be used in SenaoNetworks internally                                    *
 * Unauthorized copying of this file, via any medium is strictly prohibite                 *
 * Proprietary and confidential                                                            *
 * --------------------------------------------------------------------------------------- *
 * Project:  syseye                                                                        *
 *                                                                                         *
 * Author :  Ryan Hsu (ryan_hsu@hotmail.com.tw)                                            *
 *******************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libseipc.h>
#include <sys/types.h>
#include <sys/socket.h>

#define APP_NAME "seipc"
#define SYSEYE_PATH "/tmp/syseye.unix"
#define CFG_PATH_LENGTH 512

sepkt_t cmd_type = 0;
char *cmd_args=NULL;
char *syscfg_type="CFG_RUNTIME";
int verbose=0;

#define FLAG_NONE 0
#define FLAG_SENDJSON 3
int flag=FLAG_NONE;
char *jsonfile=NULL;

static void
usage(char **argv, int exit_code)
{
    fprintf(stderr, "Usage: %s [options] \n\n", argv[0]);
    fprintf(stderr,
        "Options:\n"
	"\t-c                        Save system config to file\n"
	"\t-s <name=value>           Set a value to SYSEYE\n"
	"\t-g <name>                 Get a value from SYSEYE\n"
	"\t-n <name>                 Get number of array elements from SYSEYE\n"
	"\t-p <name>                 Remove the block under the <name> and the <name> itself\n"
	"\t-q <name>                 Remove the first member under <name>\n"
	"\t-T <name>|all             For syseye debug\n"
	"\t-r                        reconfig, put cfg_runtime to cfg_saved\n"
	"\t-t runtime|saved|ram      Designate a certain type of the system config for\n"
	"\t                          Set/Get/Prune/TreeView commands.\n"
	"\t-u <name>                 Update <name>, remove and re-add it in the tail of array\n"
	"\t-j <json>                 Send a json file to syseye.\n"
	"\t-v                        Verbose mode. Print the request and response for debug usage\n"
	"\t-a \"<action_name> <command> <parm1> <parm2> ...\"\n"
	"\t-l                        List supported actions\n"
	"\t-h                        Show this help and exit\n"
	"\n"
	);
	exit(exit_code);
}

static void
parse_args(int argc, char **argv)
{
	int opt;
	
	while((opt = getopt(argc, argv, "j:r:s:g:n:p:q:u:P:cT:db:lt:vha:")) != -1) {
		switch (opt) {
		case 'c':
			cmd_type = SEPKT_COMMIT;
			break;
		case 'r':
			cmd_type = SEPKT_RECONF;
			if (!strcmp(optarg, "all"))
				cmd_args = NULL;
			else
				cmd_args = optarg;
			break;
		case 's':
			cmd_type = SEPKT_SET;
			cmd_args = optarg;
			break;
		case 'g':
			cmd_type = SEPKT_GET;
			cmd_args = optarg;
			break;
		case 'l':
			cmd_type = SEPKT_LISTACTION;
			cmd_args = optarg;
			break;
		case 'n':
			cmd_type = SEPKT_GET_NUM_ARRAY;
			cmd_args = optarg;
			break;
		case 'p':
			cmd_type = SEPKT_PRUNE;
			if (!strcmp(optarg, "all"))
				cmd_args = NULL;
			else
				cmd_args = optarg;
			break;
		case 'q':
			cmd_type = SEPKT_PRUNE_FIRST_ELEMENT;
			cmd_args = optarg;
			break;
		case 'T':
			cmd_type = SEPKT_TREEVIEW;
			if (!strcmp(optarg, "all"))
				cmd_args = NULL;
			else
				cmd_args = optarg;
			break;
		case 't':
			syscfg_type = (!strcmp(optarg, "runtime"))?"CFG_RUNTIME":
					(!strcmp(optarg, "saved"))?"CFG_SAVED":
					(!strcmp(optarg, "ram"))?"CFG_RAM":"CFG_UNKNOWN";
			if (!strcmp(syscfg_type, "CFG_UNKNOWN")) {
				fprintf(stderr, "Wrong system config type!\n");
				exit(1);
			}
			break;
		case 'u':
			cmd_type = SEPKT_UPDATE_ELEMENT;
			cmd_args = optarg;
			break;
		case 'j':
			flag=FLAG_SENDJSON;
			cmd_type = SEPKT_NONE;
			jsonfile=optarg;
			break;
		case 'v':
            verbose=1;
            break;
        case 'a':
			cmd_type = SEPKT_ACTION;
			cmd_args = optarg;
			break;
		case 'h':
			usage(argv, 0);
		default:
			usage(argv, 1);
		}
	}
}

static int
exec_cmd_set(struct seipc_t *handle)
{
	int ret=-1;
	char *name, *value, delim='=', *p, *s, *e;
	char cfgpath[CFG_PATH_LENGTH];

    name = cmd_args;
    p = strrchr(cmd_args, delim);
    value = p+1;
    *p = '\0';

	if (!name || !value)
		return ret;

	snprintf(cfgpath, sizeof(cfgpath), "%s/%s", syscfg_type, name);
    
//	printf("SET [ %s ] = [ %s ]\n", cfgpath, value);
	if ( (s = strchr(value, '\"')) && (e = strrchr(value,'\"'))  && s!=e ){ // string
        value = s+1;
        *e='\0';
	    ret = seipc_set_str(handle, cfgpath, value);
    }
    else
	    ret = seipc_set_int(handle, cfgpath, atoi(value));
	return ret;
}

static int
exec_cmd_get(struct seipc_t *handle)
{
	int ret=-1;
	char *name;
	char cfgpath[CFG_PATH_LENGTH];
    struct seipc_t *ret_h;
	name = cmd_args;
	snprintf(cfgpath, sizeof(cfgpath), "%s/%s", syscfg_type, name);

//	printf("GET [ %s ]\n", cfgpath);
	ret_h = seipc_get_value(handle, cfgpath);

	if (!ret_h)
        return -1;
	ret = 0;
    switch(ret_h->type)
    {
        case ezJSON_NULL:
            printf("<null>\n");
            break;
        case ezJSON_BOOL:
            printf("%d\n", ret_h->result_number);
            break;
        case ezJSON_STRING:
            printf("%s\n", ret_h->result);
            break;
        case ezJSON_NUMBER:
            printf("%d\n", ret_h->result_number);
            break;
        case ezJSON_ARRAY:
        case ezJSON_OBJECT:
        default:
            break;
    }
	return ret;
}
static int
exec_cmd_get_num_array(struct seipc_t *handle)
{
	int ret=-1;
	char *name;
	char cfgpath[CFG_PATH_LENGTH];
    struct seipc_t *ret_h;
	name = cmd_args;
	snprintf(cfgpath, sizeof(cfgpath), "%s/%s", syscfg_type, name);

//	printf("GET [ %s ]\n", cfgpath);
	ret = seipc_get_num_array(handle, cfgpath);
    if (ret >= 0) printf("%d\n", ret);
	return ret;
}

static int
exec_cmd_prune(struct seipc_t *handle)
{
	int ret=-1;
	char cfgpath[CFG_PATH_LENGTH];

	if (!cmd_args)
		snprintf(cfgpath, sizeof(cfgpath), "%s", syscfg_type);
	else
		snprintf(cfgpath, sizeof(cfgpath), "%s/%s", syscfg_type, cmd_args);

//	printf("PRUNE [ %s ]\n", cfgpath);
	ret = seipc_prune(handle, cfgpath);

	return ret;
}

static int
exec_cmd_prune_first(struct seipc_t *handle)
{
	int ret=-1;
	char cfgpath[CFG_PATH_LENGTH];

	if (!cmd_args)
		snprintf(cfgpath, sizeof(cfgpath), "%s", syscfg_type);
	else
		snprintf(cfgpath, sizeof(cfgpath), "%s/%s", syscfg_type, cmd_args);

//	printf("PRUNE [ %s ]\n", cfgpath);
	ret = seipc_prune_first(handle, cfgpath);

	return ret;
}

static int
exec_cmd_update_element(struct seipc_t *handle)
{
	int ret=-1;
	char cfgpath[CFG_PATH_LENGTH];

	if (!cmd_args)
		snprintf(cfgpath, sizeof(cfgpath), "%s", syscfg_type);
	else
		snprintf(cfgpath, sizeof(cfgpath), "%s/%s", syscfg_type, cmd_args);

//	printf("PRUNE [ %s ]\n", cfgpath);
	ret = seipc_update_elem(handle, cfgpath);

	return ret;
}

static int
exec_cmd_treeview(struct seipc_t *handle)
{
	int ret=-1;
	char cfgpath[CFG_PATH_LENGTH];
	
	if (!cmd_args)
		snprintf(cfgpath, sizeof(cfgpath), "%s", syscfg_type);
	else
		snprintf(cfgpath, sizeof(cfgpath), "%s/%s", syscfg_type, cmd_args);

//	printf("TREEVIEW [ %s ]\n", cfgpath);
	ret = seipc_treeview(handle, cfgpath);
	return ret;
}
static int
exec_cmd_commit(struct seipc_t *handle)
{
	int ret=-1;
    printf("COMMIT - \n");
	ret = seipc_commit(handle, NULL);
	return ret;
}

static int
exec_cmd_reconf(struct seipc_t *handle)
{
	int ret=-1;
    printf("RECONF [ %s ]\n", (!cmd_args)?"all":cmd_args);
	ret = seipc_reconf(handle, cmd_args);
	return ret;
}

static int
exec_cmd_action(struct seipc_t *handle)
{
    int ret=-1, namelen=0;
	char name[50], command[512], *p, *result;
    JsonNode *gx;

	memset(name, 0, sizeof(name));
	memset(command, 0, sizeof(command));
	
	if (!cmd_args)
		return ret;
	else {
		p = strchr(cmd_args, ' ');
		if (!p) 
			return ret;
		namelen = p-cmd_args;
		strncpy(name, cmd_args, namelen);
		snprintf(command, sizeof(command), "%s", p+1);
//		printf("ACTION: name [ %s ] command [ %s ]\n", name, command);
		gx = seipc_action(handle, name, command);
		if (gx) {
            if (js_get_path(gx, "resultPath")!=NULL){
                printf("%d\n", js_get_path_int(gx, "resultPath"));
            }
            else{
                js_print_hr(gx);
            }

            js_free(gx);
		}
        ret = 0;
	}
	return ret;
}
static int
exec_cmd_listaction(struct seipc_t *handle)
{
	int ret=-1;

	ret = seipc_listaction(handle);
	return ret;
}

static int
send_json(struct seipc_t *handle)
{
    JsonNode *gx;
    int ret=-1;
    gx = js_parse_file(jsonfile);
    ret = seipc_send_json(handle, gx);
    js_free(gx);
    return ret;
}

int main(int argc, char **argv)
{
	int ret=0;
	struct seipc_t *handle=NULL;

	/* Parse the arguments */
	if (argc < 2)
		usage(argv, 1);
	parse_args(argc, argv);

	if (argc - optind == 0) 
		handle = seipc_create(APP_NAME, SYSEYE_PATH);

	if (!handle) {
		fprintf(stderr, "Cannot create seipc handle. Exit!\n");
		exit(1);
	}

    if (verbose)
        seipc_debug_on(stdout);

	/* Execute the command */
	switch(cmd_type) {
	case SEPKT_NONE:
		break;
	case SEPKT_SET:
		ret = exec_cmd_set(handle);
		break;
	case SEPKT_GET:
		ret = exec_cmd_get(handle);
		break;
	case SEPKT_GET_NUM_ARRAY:
		ret = exec_cmd_get_num_array(handle);
		break;
	case SEPKT_PRUNE:
		ret = exec_cmd_prune(handle);
		break;
    case SEPKT_PRUNE_FIRST_ELEMENT:
		ret = exec_cmd_prune_first(handle);
		break;
    case SEPKT_UPDATE_ELEMENT:
		ret = exec_cmd_update_element(handle);
		break;
    case SEPKT_RECONF:
		ret = exec_cmd_reconf(handle);
		break;
    case SEPKT_COMMIT:
		ret = exec_cmd_commit(handle);
		break;
	case SEPKT_TREEVIEW:
		ret = exec_cmd_treeview(handle);
		break;
    case SEPKT_ACTION:
        ret = exec_cmd_action(handle);
        break;
	case SEPKT_LISTACTION:
		ret = exec_cmd_listaction(handle);
		break;
	default:
		fprintf(stderr, "Unknown command type. Exit!\n");
		break;
	}

	if (ret < 0) {
		fprintf(stderr, "Cannot get the result!\n");
		seipc_close(handle);
		exit(1);
    }

    switch (flag) {
        case FLAG_SENDJSON:
            send_json(handle);
            break;
        default:
            break;
    }

    if (verbose)
        seipc_debug_off();

	seipc_close(handle);
	return 0;
}




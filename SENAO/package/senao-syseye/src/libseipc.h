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
#ifndef _LIBSEIPC_H_
#define _LIBSEIPC_H_
#include <ezjson.h>

typedef enum
{
    SEPKT_NONE = 0,
    SEPKT_SET = 1,
    SEPKT_GET,
    SEPKT_GET_NUM_ARRAY,
    SEPKT_PRUNE = 10,
    SEPKT_PRUNE_FIRST_ELEMENT,
    SEPKT_UPDATE_ELEMENT,
    SEPKT_RECONF = 20,
    SEPKT_COMMIT,
    SEPKT_DEFAULT,
	SEPKT_ACTION = 30,
	SEPKT_ACTIONS = 32,
	SEPKT_LISTACTION = 31,
    SEPKT_TREEVIEW = 70,
    SEPKT_RESPONSE = 99
} sepkt_t;

struct seipc_t {
    int sockfd;
    char *source;
    int type; // 0:null 1:bool 2:string 3: number 4: array 5:object    
    char *result;
    int result_number;
};
extern void seipc_debug_on(FILE *fp);
extern void seipc_debug_off(void);
extern struct seipc_t* seipc_create(char *source, char *path);
extern void seipc_close(struct seipc_t *handle);
extern int seipc_set_blk(struct seipc_t *handle, JsonNode *data);
extern int seipc_set_str(struct seipc_t *handle, char *path, char *value);
extern int seipc_set_int(struct seipc_t *handle, char *path, int value);
extern JsonNode *seipc_get_blk(struct seipc_t *handle, char *path);
extern char* seipc_get_str(struct seipc_t *handle, char *path);
extern struct seipc_t *seipc_get_value(struct seipc_t *handle, char *path);
extern int seipc_get_num_array(struct seipc_t *handle, char *path);
extern int seipc_get_int(struct seipc_t *handle, char *path);
extern int seipc_reconf(struct seipc_t *handle, char *path);
extern int seipc_commit(struct seipc_t *handle, char *path);
extern int seipc_prune(struct seipc_t *handle, char *path);
extern int seipc_prune_first(struct seipc_t *handle, char *path);
extern int seipc_update_elem(struct seipc_t *handle, char *path);
extern int seipc_treeview(struct seipc_t *handle, char *path);
extern JsonNode *seipc_action(struct seipc_t *handle, char *name, char *command);
extern int seipc_listaction(struct seipc_t *handle);
extern JsonNode *seipc_actions(struct seipc_t *handle, JsonNode *json);
extern int seipc_send_json(struct seipc_t *handle, JsonNode *json);
#endif

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
#ifndef _GLOBAL_H_
#define _GLOBAL_H_
#include <mio.h>
#include <ezjson.h>
#include "action.h"
typedef struct gdata_t {
    mio_data_t *mio_data;
    action_t *action;
    JsonNode *cfg_runtime;
    JsonNode *cfg_saved;
    JsonNode *cfg_ram;
    char storage_path[128];
}gdata_t;
#endif

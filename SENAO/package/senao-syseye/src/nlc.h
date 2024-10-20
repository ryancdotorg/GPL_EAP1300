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
#ifndef _NLC_H_
#define _NLC_H_
#include "cmd.h"
typedef enum {
    CFG_RAM = 0,
    CFG_SAVED,
    CFG_RUNTIME,
} storage_t;

typedef struct nl_cmd_t {
    char func[32];
    int line;
    cmd_type_t type;
    char data[0];
} __attribute__ ((packed, aligned(4))) nl_cmd_t;

typedef struct nl_set_data_t {
    storage_t storage;
    char path[128];
    char value[0];
} __attribute__ ((packed, aligned(4))) nl_set_data_t;

typedef struct nl_prune_data_t {
    storage_t storage;
    char path[128];
} __attribute__ ((packed, aligned(4))) nl_prune_data_t;

typedef struct nl_act_data_t {
    char name[32];
    int delay;
    char command[0];
} __attribute__ ((packed, aligned(4))) nl_act_data_t;

#endif

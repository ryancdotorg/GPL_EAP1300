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
#ifndef _CMD_H_
#define _CMD_H_
typedef enum {
    CMD_NONE = 0,
    CMD_SET = 1,
    CMD_GET,
    CMD_GET_NUM_ARRAY,
    CMD_PRUNE = 10,
    CMD_PRUNE_FIRST_ELEMENT,
    CMD_UPDATE_ELEMENT,
    CMD_RECONF = 20,
    CMD_COMMIT,
    CMD_DEFAULT,
    CMD_ACTION = 30,
    CMD_LISTACTION = 31,
    CMD_ACTIONS = 32,
    CMD_TREEVIEW = 70,
    CMD_RESPONSE = 99,
} cmd_type_t;
#endif

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
#include <unistd.h>
#include "statuscode.h"
#define check_meshap_connected_file "/var/run/map/listen_connected"
#define check_meshap_finish_file "/var/run/map/listen_finish"

int check_meshap_status()
{
    int ret;
    if  (access(check_meshap_finish_file, F_OK) != -1)
        ret = MESHAP_FINISH;
    else if(access(check_meshap_connected_file, F_OK) != -1)
        ret = MESHAP_CONNECTED;
    else
        // file doesn't exist, MAP Listening is running
        ret = STATUS_OK; 
    return ret;
}


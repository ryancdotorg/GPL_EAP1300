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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "statuscode.h"

int check_phy_status()
{
    int fd, len, ret = -1, i = 0;
    char line[32];
    int linkstatus = 0;
    if ((fd = open("/proc/LAN_STATE", O_RDONLY)) == -1)
        return PHYLINK_DATA_ERR;

    ret = read(fd, line, sizeof(line));
    if (ret <= 0){
        close(fd);
        return PHYLINK_DATA_ERR;
    }

    linkstatus = atoi(line);
    if (linkstatus > 0)
        ret = STATUS_OK;
    else
        ret = PHYLINK_DOWN;
    //printf("linkstatus: [%d]\n", linkstatus);
    close(fd);
    return ret;
}


/*******************************************************************************************
 * Copyright (c) 2018 Ryan Hsu @ SenaoNetworks - Taiwan                                    *
 *                                                                                         *
 * This code has to be used in SenaoNetworks internally                                     *
 * Unauthorized copying of this file, via any medium is strictly prohibite                 *
 * Proprietary and confidential                                                            *
 * --------------------------------------------------------------------------------------- *
 * Project:  linkmon                                                                       *
 *                                                                                         *
 * Author :  Ryan Hsu (ryan_hsu@hotmail.com.tw)                                            *
 *******************************************************************************************/
#ifndef _UTIL_COMMON_H_
#define _UTIL_COMMON_H_
#include <unistd.h>
#include <string.h>
#include <sys/syscall.h>
#include <errno.h>

int file_exist(char *fname);
#define DBGFILE "/tmp/lmdbg"
#define lmdbg(fmt, args...) do { \
        if (file_exist(DBGFILE)) { \
            char outdev[16]; \
            memset(outdev, 0, sizeof(outdev)); \
            FILE *fp = fopen(DBGFILE, "r"); \
            if (fscanf(fp, "%s", outdev)==EOF) \
                strcpy(outdev, "/dev/console"); \
            fclose(fp); \
            fp = fopen(outdev, "a"); \
            if (fp) { \
                unsigned int pid = syscall(SYS_gettid); \
                fprintf(fp, "\033[1m\033[40;%dm[%5d %-10s<%5d>%10s()]\033[0m ", 30+(pid%6)+1, pid, __FILE__, __LINE__, __func__); \
                fprintf(fp, fmt, ## args); \
                fclose(fp); \
            } \
        } \
    } while(0)

#define lmdbg_en(en, msg) \
    do { errno = en; sedbg("%s(%d):%s\n", msg, errno, strerror(errno)); } while (0)

void DumpHex(const void* data, size_t size);

#endif

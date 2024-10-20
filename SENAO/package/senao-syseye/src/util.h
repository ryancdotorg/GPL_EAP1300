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
#ifndef _UTIL_COMMON_H_
#define _UTIL_COMMON_H_
#include <unistd.h>
#include <string.h>
#include <sys/syscall.h>
#include <errno.h>

#include <fcntl.h>
#include <sys/mman.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

pid_t safe_fork_fn(void *(*fn) (void *), void *args);
pid_t safe_fork(void);
int makeargv(const char *s, const char *delimiters, char ***argvp);
void freemakeargv(char **argv);
void hexDump(char *desc, void *addr, int len);
int file_exist(char *fname);
int write_to_fname(char *fname, const char *fi, int l, const char *fn, const char *fmt, ...);
#define DBGFILE "/tmp/sedbg"

#define sedbg(fmt, args...) do { \
        if (file_exist(DBGFILE)) { \
            int fd, len, i; \
            char *tmp_ptr = NULL, *p_odev = NULL; \
            char def_odev[] = "/dev/console"; \
            char tmp[1024], fname[128]; \
            if ((fd = open(DBGFILE, O_RDONLY)) == -1) \
	        p_odev = def_odev; \
            else { \
                len = lseek(fd, 0, SEEK_END); \
                if (len == 0) p_odev = def_odev; \
                else { \
                    if ((tmp_ptr = mmap(0, len, PROT_READ, MAP_PRIVATE, fd, 0)) == MAP_FAILED) \
                        p_odev = def_odev; \
                    else { \
                        strcpy(fname, tmp_ptr); \
                        tmp_ptr = fname; \
                        i = strlen(fname) -1; \
                        while (i>=0 && strchr("\t\n\v\f\r ", tmp_ptr[i])!=NULL){ \
                            tmp_ptr[i] = '\0'; \
                            i--; \
                        } \
                        p_odev = fname; \
                    } \
                } \
            } \
            memset(tmp, 0, sizeof(tmp)); \
            snprintf(tmp, sizeof(tmp), fmt, ## args); \
            write_to_fname(p_odev, __FILE__, __LINE__, __func__, tmp); \
	    munmap(0, len); \
            close(fd); \
        } \
    } while(0)

#define sedbg_js(js) do { \
        if (file_exist(DBGFILE)) { \
            int fd, len, i; \
            char *tmp_ptr = NULL, *p_odev = NULL; \
            char def_odev[] = "/dev/console"; \
            char *tmp, fname[128]; \
            if ((fd = open(DBGFILE, O_RDONLY)) == -1) \
	        p_odev = def_odev; \
            else { \
                len = lseek(fd, 0, SEEK_END); \
                if (len == 0) p_odev = def_odev; \
                else { \
                    if ((tmp_ptr = mmap(0, len, PROT_READ, MAP_PRIVATE, fd, 0)) == MAP_FAILED) \
                        p_odev = def_odev; \
                    else { \
                        strcpy(fname, tmp_ptr); \
                        tmp_ptr = fname; \
                        i = strlen(fname) -1; \
                        while (i>=0 && strchr("\t\n\v\f\r ", tmp_ptr[i])!=NULL){ \
                            tmp_ptr[i] = '\0'; \
                            i--; \
                        } \
                        p_odev = fname; \
                    } \
                } \
            } \
            if ((tmp = js_to_str_hr(js))!=NULL) { \
                write_to_fname(p_odev, __FILE__, __LINE__, __func__, tmp); \
                free(tmp); \
            } \
	    munmap(0, len); \
            close(fd); \
        } \
    } while(0)

#define sedbg_en(en, msg) \
    do { errno = en; sedbg("%s(%d):%s\n", msg, errno, strerror(errno)); } while (0)

#endif

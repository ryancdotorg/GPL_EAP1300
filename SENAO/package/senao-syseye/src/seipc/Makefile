#/*******************************************************************************************
# * Copyright (c) 2018 Ryan Hsu @ SenaoNetworks - Taiwan                                    *
# *                                                                                         *
# * This code has to be used in SenaoNetworks internally                                    *
# * Unauthorized copying of this file, via any medium is strictly prohibite                 *
# * Proprietary and confidential                                                            *
# * --------------------------------------------------------------------------------------- *
# * Project:  syseye                                                                        *
# *                                                                                         *
# * Author :  Ryan Hsu (ryan_hsu@hotmail.com.tw)                                            *
# *******************************************************************************************/
CC ?= gcc
AR ?= ar
LN ?= ln

TEMPFILES = core core.* *.o temp.* *.out typescript* *~

# client ipc
PROG = seipc
SRCS= seipc.c
OBJS = $(SRCS:%.c=%.o)

CFLAGS_libseipc ?= -I../
CFLAGS_json ?= -I/usr/local/include
LDFLAGS_json ?= -L/usr/local/lib -lezjson
LDFLAGS_libseipc ?= -L../ -lseipc

CFLAGS = $(CFLAGS_libseipc) $(CFLAGS_json)
LDFLAGS = $(LDFLAGS_libseipc) $(LDFLAGS_json)

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(PROG) $(OBJS) $(TEMPFILES)

#/*******************************************************************************************
# * Copyright (c) 2018 Ryan Hsu @ SenaoNetworks - Taiwan                                    *
# *                                                                                         *
# * This code has to be used in SenaoNetworks internally                                     *
# * Unauthorized copying of this file, via any medium is strictly prohibite                 *
# * Proprietary and confidential                                                            *
# * --------------------------------------------------------------------------------------- *
# * Project:  syseye                                                                       *
# *                                                                                         *
# * Author :  Ryan Hsu (ryan_hsu@hotmail.com.tw)                                            *
# *******************************************************************************************/
CC ?= gcc
AR ?= ar
LN ?= ln -f

TEMPFILES = core core.* *.o temp.* *.out typescript* *~

# daemon
PROG = syseye
SRCS= main.c cmdparser.c cmdipc.c ipc_tcp.c nl_cmdparser.c ipc_nl.c thread.c action.c util_socket.c util.c
OBJS = $(SRCS:%.c=%.o)

CFLAGS_mio ?= -I/usr/local/include
CFLAGS_json ?= -I/usr/local/include

LDFLAGS_mio ?= -L/usr/local/lib -lmio
LDFLAGS_json ?= -L/usr/local/lib -lezjson

# ipc list
CFLAGS_ipc ?= -DIPC_UNIX

# thread list
LDFLAGS_thread ?= -Lthread_linkmon -llinkmon
CFLAGS_thread ?= -Ithread_linkmon -DTHREAD_LINKMON
PLUGINS_thread ?= thread_linkmon

# ACTION list
LDFLAGS_act ?= -Lact_sample -lact_sample
CFLAGS_act ?= -Iact_sample -DACT_SAMPLE
PLUGINS_act ?= act_sample

CFLAGS = $(CFLAGS_mio) $(CFLAGS_json) $(CFLAGS_ipc) $(CFLAGS_libseipc) $(CFLAGS_thread) $(CFLAGS_act)
LDFLAGS = $(LDFLAGS_mio) $(LDFLAGS_json) $(LDFLAGS_libseipc) $(LDFLAGS_thread) -lpthread

# library
LIBRARY = libseipc
LIB_SRCS=libseipc.c
LIB_STATIC=$(addsuffix .a, $(LIBRARY))
LIB_SHARED=$(addsuffix .so, $(LIBRARY))
LIB_OBJS=$(LIB_SRCS:%.c=%.o)

LIB_SHARED_LINKERNAME=$(addsuffix .so,$(LIBRARY))
LIB_SHARED_SONAME=$(addsuffix .so.1,$(LIBRARY))
LIB_SHARED_REALNAME=$(addsuffix .so.1.0.0,$(LIBRARY))

PLUGINS = $(PLUGINS_thread) $(PLUGINS_act)

all: $(LIB_SHARED) $(LIB_STATIC) plugins $(PROG)

.PHONY: plugins $(PLUGINS)

plugins: $(PLUGINS)

$(PLUGINS):
	$(MAKE) -C $@

$(PROG): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS) $(LDFLAGS_act)

$(LIB_OBJS): $(LIB_SRCS)
	$(CC) -c $(CFLAGS) $< -o $@ -fPIC

$(LIB_STATIC): $(LIB_OBJS)
	$(AR) crsv $@ $<

$(LIB_SHARED): $(LIB_OBJS)
	$(CC) -shared -Wl,-soname,$(LIB_SHARED_SONAME) -o $(LIB_SHARED_REALNAME) $<
	$(LN) -s $(LIB_SHARED_REALNAME) $(LIB_SHARED_LINKERNAME)
	$(LN) -s $(LIB_SHARED_REALNAME) $(LIB_SHARED_SONAME)

PLUGINS_CLEAN = $(PLUGINS:%=%_clean)

.PHONY: plugins_clean $(PLUGINS_CLEAN)

plugins_clean: $(PLUGINS_CLEAN)

$(PLUGINS_CLEAN):
	$(MAKE) -C $(@:_clean=) clean

clean: plugins_clean
	rm -f $(PROG) $(OBJS) $(LIB_STATIC) $(LIB_SHARED) $(LIB_OBJS) $(LIB_SHARED_LINKERNAME) $(LIB_SHARED_SONAME) $(LIB_SHARED_REALNAME) $(TEMPFILES)

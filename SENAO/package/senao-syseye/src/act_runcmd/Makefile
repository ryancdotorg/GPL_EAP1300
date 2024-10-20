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

SRCS_UTIL?= ../util.c
CFLAGS_UTIL?= -I../

# process
PROG = act_runcmd
SRCS= main.c act_runcmd.c $(SRCS_UTIL)
OBJS = $(SRCS:%.c=%.o)

CFLAGS_syseye ?= -I../
CFLAGS_mio ?= -I/usr/local/include
CFLAGS_json ?= -I/usr/local/include

LDFLAGS_mio ?= -L/usr/local/lib -lmio
LDFLAGS_json ?= -L/usr/local/lib -lezjson

CFLAGS = $(CFLAGS_syseye) $(CFLAGS_mio) $(CFLAGS_json) $(CFLAGS_UTIL)
LDFLAGS = $(LDFLAGS_mio) $(LDFLAGS_json) -lpthread

# library
LIBRARY = libact_runcmd
LIB_SRCS=act_runcmd.c
LIB_STATIC=$(addsuffix .a, $(LIBRARY))
LIB_SHARED=$(addsuffix .so, $(LIBRARY))
LIB_OBJS=$(LIB_SRCS:%.c=%.o)

LIB_SHARED_LINKERNAME=$(addsuffix .so,$(LIBRARY))
LIB_SHARED_SONAME=$(addsuffix .so.1,$(LIBRARY))
LIB_SHARED_REALNAME=$(addsuffix .so.1.0.0,$(LIBRARY))

all: $(PROG) $(LIB_STATIC)
#all: $(PROG) $(LIB_SHARED) $(LIB_STATIC)

$(PROG): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

#$(LIB_OBJS): $(LIB_SRCS)
#	$(CC) -c $(CFLAGS) $< -o $@ -fPIC

$(LIB_STATIC): $(LIB_OBJS)
	$(AR) crsv $@ $^

#$(LIB_SHARED): $(LIB_OBJS)
#	$(CC) -shared -Wl,-soname,$(LIB_SHARED_SONAME) -o $(LIB_SHARED_REALNAME) $<
#	$(LN) -s $(LIB_SHARED_REALNAME) $(LIB_SHARED_LINKERNAME)
#	$(LN) -s $(LIB_SHARED_REALNAME) $(LIB_SHARED_SONAME)

install:

clean:
	rm -f $(PROG) $(IPCCLI) $(IPCCLI_OBJS) $(LIB_STATIC) $(LIB_SHARED) $(LIB_OBJS) $(OBJS) $(LIB_SHARED_LINKERNAME) $(LIB_SHARED_SONAME) $(LIB_SHARED_REALNAME)

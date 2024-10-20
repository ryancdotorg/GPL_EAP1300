#/*******************************************************************************************
# * Copyright (c) 2018 Ryan Hsu @ SenaoNetworks - Taiwan                                    *
# *                                                                                         *
# * This code has to be used in SenaoNetworks internally                                     *
# * Unauthorized copying of this file, via any medium is strictly prohibite                 *
# * Proprietary and confidential                                                            *
# * --------------------------------------------------------------------------------------- *
# * Project:  linkmon                                                                       *
# *                                                                                         *
# * Author :  Ryan Hsu (ryan_hsu@hotmail.com.tw)                                            *
# *******************************************************************************************/
CC ?= gcc
AR ?= ar
LN ?= ln

TEMPFILES = core core.* *.o temp.* *.out typescript* *~

# daemon
PROG = linkmon
SRCS= main.c mio.c phylink.c dns.c http.c https.c arpdup.c icmp.c util.c linkmon.c ipc_udp.c cmdipc.c cmdparser.c
OBJS = $(SRCS:%.c=%.o)

# library
LIBRARY = liblmipc
LIB_SRCS=liblmipc.c
LIB_STATIC=$(addsuffix .a, $(LIBRARY))
LIB_SHARED=$(addsuffix .so, $(LIBRARY))
LIB_OBJS=$(LIB_SRCS:%.c=%.o)

LIB_SHARED_LINKERNAME=$(addsuffix .so,$(LIBRARY))
LIB_SHARED_SONAME=$(addsuffix .so.1,$(LIBRARY))
LIB_SHARED_REALNAME=$(addsuffix .so.1.0.0,$(LIBRARY))

# client ipc
IPCCLI = lmipc
IPCCLI_SRCS= lmipc.c
IPCCLI_OBJS = $(IPCCLI_SRCS:%.c=%.o)

JSON_C_DIR?="/home/.work/Test/test/jsonc_wrap/jsonc_lib"
CFLAGS += -I$(JSON_C_DIR)/include/json-c
LDFLAGS += -L$(JSON_C_DIR)/lib -ljson-c
#LDFLAGS += -L/usr/lib/x86_64-linux-gnu -lssl -lcrypto

LIBLMIPC_DIR ?= ./

all: $(PROG) $(LIB_SHARED) $(LIB_STATIC) $(IPCCLI)

$(PROG): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

$(IPCCLI): $(IPCCLI_OBJS)
	$(CC) -o $@ $^ -L$(LIBLMIPC_DIR) -llmipc $(LDFLAGS)

cmdparser.o: cmdparser.c
	$(CC) -c $(CFLAGS) $< -o $@ $(LDFLAGS)
	
ipc_udp.o: ipc_udp.c
	$(CC) -c $(CFLAGS) $< -o $@ 

$(LIB_OBJS): $(LIB_SRCS)
	$(CC) -c $(CFLAGS) $< -o $@ -fPIC

$(LIB_STATIC): $(LIB_OBJS)
	$(AR) crsv $@ $<

$(LIB_SHARED): $(LIB_OBJS)
	$(CC) -shared -Wl,-soname,$(LIB_SHARED_SONAME) -o $(LIB_SHARED_REALNAME) $<
	$(LN) -s $(LIB_SHARED_REALNAME) $(LIB_SHARED_LINKERNAME)
	$(LN) -s $(LIB_SHARED_REALNAME) $(LIB_SHARED_SONAME)

install:

clean:
	rm -f $(PROG) $(IPCCLI) $(IPCCLI_OBJS) $(LIB_STATIC) $(LIB_SHARED) $(LIB_OBJS) $(OBJS) $(LIB_SHARED_LINKERNAME) $(LIB_SHARED_SONAME) $(LIB_SHARED_REALNAME)

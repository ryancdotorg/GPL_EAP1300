#/*******************************************************************************************
# * Copyright (c) 2018 Ryan Hsu @ SenaoNetworks - Taiwan                                    *
# *                                                                                         *
# * This code has to be used in SenaoNetworks internally                                     *
# * Unauthorized copying of this file, via any medium is strictly prohibite                 *
# * Proprietary and confidential                                                            *
# * --------------------------------------------------------------------------------------- *
# * Project:  mio                                                                       *
# *                                                                                         *
# * Author :  Ryan Hsu (ryan_hsu@hotmail.com.tw)                                            *
# *******************************************************************************************/
CC ?= gcc
AR ?= ar
LN ?= ln -f

TEMPFILES = core core.* *.o temp.* *.out typescript* *~

#library
LIBRARY = libmio
LIB_SRCS=mio.c
LIB_STATIC=$(addsuffix .a, $(LIBRARY))
LIB_SHARED=$(addsuffix .so, $(LIBRARY))
LIB_OBJS=$(LIB_SRCS:%.c=%.o)

LIB_SHARED_LINKERNAME=$(addsuffix .so,$(LIBRARY))
LIB_SHARED_SONAME=$(addsuffix .so.1,$(LIBRARY))
LIB_SHARED_REALNAME=$(addsuffix .so.1.0.0,$(LIBRARY))

CFLAGS =
LDFLAGS =

all: $(LIB_SHARED) $(LIB_STATIC)

$(LIB_OBJS): $(LIB_SRCS)
	$(CC) -c $(CFLAGS) $< -o $@ -fPIC

$(LIB_STATIC): $(LIB_OBJS)
	$(AR) crsv $@ $<

$(LIB_SHARED): $(LIB_OBJS)
	$(CC) -shared -Wl,-soname,$(LIB_SHARED_SONAME) -o $(LIB_SHARED_REALNAME) $<
	$(LN) -s $(LIB_SHARED_REALNAME) $(LIB_SHARED_LINKERNAME)
	$(LN) -s $(LIB_SHARED_REALNAME) $(LIB_SHARED_SONAME)

clean:
	rm -f $(LIB_STATIC) $(LIB_SHARED) $(LIB_OBJS) $(LIB_SHARED_LINKERNAME) $(LIB_SHARED_SONAME) $(LIB_SHARED_REALNAME) $(TEMPFILES) 

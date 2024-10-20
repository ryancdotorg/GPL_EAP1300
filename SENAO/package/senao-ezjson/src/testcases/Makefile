#/*******************************************************************************************
# * Copyright (c) 2018 Ryan Hsu @ SenaoNetworks - Taiwan                                    *
# *                                                                                         *
# * This code has to be used in SenaoNetworks internally                                     *
# * Unauthorized copying of this file, via any medium is strictly prohibite                 *
# * Proprietary and confidential                                                            *
# * --------------------------------------------------------------------------------------- *
# * Project:  ezjson                                                                        *
# *                                                                                         *
# * Author :  Ryan Hsu (ryan_hsu@hotmail.com.tw)                                            *
# *******************************************************************************************/
CC ?= gcc
AR ?= ar
LN ?= ln

TEMPFILES = core core.* *.o temp.* *.out typescript* *~

# daemon
PROG = test
SRCS= main.c
OBJS = $(SRCS:%.c=%.o)

CFLAGS = -I../
LDFLAGS = -L../ -lezjson

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(PROG) $(OBJS) $(TEMPFILES)

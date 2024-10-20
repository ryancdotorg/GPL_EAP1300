.PHONY:	all clean install

TOP=$(shell pwd)/../../..
APPS_DIR=$(TOP)/MIPS32_APPS
H_DIR=$(APPS_DIR)/h
APPAGENTD_DIR=../app_agent
WEB_INCLUDE_DIR=../includes
SYSUTIL_DIR=$(APPS_DIR)/sysutil/include
CONFIG_DIR= $(COPTS)
HEADER_DIR=$(APPS_DIR)/header/h

INCLUDES =  -I./ -I$(H_DIR) -I$(WEB_INCLUDE_DIR) -I$(APPAGENTD_DIR) -I$(CONFIG_DIR)

# 20120711 Jerry: copy from web_backup directory
OBJS = uap_lib.o semaph.o apiport.o admin_cfg.o aes.o base64.o

CCFLAGS= $(EXTRA_CFLAGS)
CC = $(CROSS)gcc
AR = $(CROSS)ar
LD = $(CROSS)ld
RANLIB= $(CROSS)ranlib

all: depend appagentutil.a

appagentutil.a: $(OBJS)
	$(AR) crv $@ $(OBJS) 
	$(RANLIB) $@

%.o: %.c
	@echo 'CC $@'
	@$(CC) $(DEFINES) $(INCLUDES) $(CCFLAGS) -c $< -o $@

clean:
	$(RM) -f *.o *~ *.a .depend

depend: .depend

.depend:
	CC=$(CC) ../mkdep $(CFLAGS) $(INCLUDES) `ls *.c`

ifneq ($(wildcard .depend),)
include .depend
endif

install: all

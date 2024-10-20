CC=$(CROSS)gcc
LD = $(CROSS)ld
CFLAGS+=$(EXTRA_CFLAGS)
LDFLAGS+=$(EXTRA_LDFLAGS)
EXECS = cfdk
OBJS = cfdk.o sort.o
MYINCLUDES = -I../../configs/h -I./
WEB_CONFIG_FILE=../../configs/h/webConfig.h
ifneq ("$(wildcard $(WEB_DEF_CONFIG_FILE))","")
CFLAGS+=-DSUPPORT_WEB_DEF_CONFIG=1
MYINCLUDES+=-I$(dir $(WEB_DEF_CONFIG_FILE))
ifneq ("$(wildcard $(WEB_CONFIG_FILE))","")
CFLAGS+=-DSUPPORT_WEB_CONFIG=1
endif
endif

#ifneq ("$(WEB_LANG_ARRAY_NAME)","")
#CFLAGS+=-DWEB_LANG_ARRAY_NAME="\"$(WEB_LANG_ARRAY_NAME)\""
#endif
CFLAGS+=-DWEB_LANG_ARRAY_NAME="\"LANG\""

all: clean   $(EXECS)

%.o: %.c
	$(CC) $(CFLAGS) $(MYINCLUDES) -c -o $@ $<

$(EXECS): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

clean:
	rm -f $(EXECS) *.elf *.gdb *.o

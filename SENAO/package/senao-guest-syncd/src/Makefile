PRODUCT_CONFIG_MAKE = ../../configs/product_config.make
ifeq ($(wildcard $(PRODUCT_CONFIG_MAKE)),$(PRODUCT_CONFIG_MAKE))
include $(PRODUCT_CONFIG_MAKE)
endif

.PHONY:	all clean build-deplibs clean-deplibs rebuild

CC = $(CROSS)gcc
AR = $(CROSS)ar
LD = $(CROSS)ld
RANLIB= $(CROSS)ranlib
CFLAGS += -Wall -pipe $(EXTRA_CFLAGS)
LDFLAGS=$(EXTRA_LDFLAGS)

GUESTSYNC_CLI_OBJS = guestsync_ctrl.o guestsync_cli.o

GUESTSYNC_OBJS = common.o eloop.o ctrl_iface.o guestsyncd.o guestsyncdcmd.o guestsync.o sockIntf.o guestsynchandle.o guestsync_list.o aes.o

INCLUDES = -I./ -I./include

ifeq ($(APPS_FULL_DIR),)
APPS_FULL_DIR=../../MIPS32_APPS/
endif

all: guest_syncd guest_syncli

build-deplibs:
ifneq ($(DEPLIBS),)
	for d in $(DEPLIBS); do \
		($(MAKE) -C $(APPS_FULL_DIR)/$$d all) || exit; \
	done;
endif

clean-deplibs:
ifneq ($(DEPLIBS),)
	for d in $(DEPLIBS); do \
		($(MAKE) -C $(APPS_FULL_DIR)/$$d clean) || exit; \
	done;
endif

guest_syncd: $(GUESTSYNC_OBJS)
	@echo 'CC $@'
	@$(CC) $(CFLAGS) $(INCLUDES) -o $@ $(GUESTSYNC_OBJS) -lcrypto;

guest_syncli: $(GUESTSYNC_CLI_OBJS)
	@echo 'CC $@'
	@$(CC) $(CFLAGS) $(INCLUDES) -o $@ $(GUESTSYNC_CLI_OBJS);

%.o: %.c
	@echo 'CC $@'
	@$(CC) $(DEFINE) $(CFLAGS) $(INCLUDES) -c -o $@ $<

clean:
	rm -rf *.elf *.gdb *.o *~ *.a .depend; \
	rm -rf guest_syncli guest_syncd; \
	find ./ -name "*~" | xargs rm -rf; \
	find ./ -name "*.o" | xargs rm -rf

rebuild: clean-deplibs build-deplibs clean all

install:
	mkdir -p $(PREFIX)/sbin; \
	if [ ! -e guest_syncd ] || [ ! -e guest_syncli ] ; then \
		false; \
	else \
		install -c guest_syncd $(PREFIX)/sbin ; \
		install -c guest_syncli $(PREFIX)/sbin ; \
	fi


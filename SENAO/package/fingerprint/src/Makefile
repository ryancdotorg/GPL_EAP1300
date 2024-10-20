PRODUCT_CONFIG_MAKE = ../../configs/product_config.make
ifeq ($(wildcard $(PRODUCT_CONFIG_MAKE)),$(PRODUCT_CONFIG_MAKE))
include $(PRODUCT_CONFIG_MAKE)
endif

.PHONY:all clean rebuild

CC = $(CROSS)gcc
AR = $(CROSS)ar
LD = $(CROSS)ld
RANLIB= $(CROSS)ranlib
CFLAGS = -Wall -pipe $(EXTRA_CFLAGS)
LDFLAGS=$(EXTRA_LDFLAGS)

FINGER_CLI_OBJS = finger_ctrl.o finger_cli.o

FINGER_OBJS = common.o eloop.o ctrl_iface.o fingerd.o fingerdcmd.o finger.o sockIntf.o fingerhandle.o finger_list.o aes.o fingerprint.o

INCLUDES = -I./ -I./include


all: fingerprint finger_syncli

fingerprint: $(FINGER_OBJS)
	@echo 'CC $@'
	@$(CC) $(CFLAGS) $(INCLUDES) -o $@ $(FINGER_OBJS) -lcrypto;

finger_syncli: $(FINGER_CLI_OBJS)
	@echo 'CC $@'
	@$(CC) $(CFLAGS) $(INCLUDES) -o $@ $(FINGER_CLI_OBJS);

%.o: %.c
	@echo 'CC $@'
	@$(CC) $(DEFINE) $(CFLAGS) $(INCLUDES) -c -o $@ $<

clean:
	rm -rf *.elf *.gdb *.o *~ *.a .depend; \
	rm -rf finger_syncli fingerprint; \
	find ./ -name "*~" | xargs rm -rf; \
	find ./ -name "*.o" | xargs rm -rf

rebuild: clean-deplibs clean all

install:
	mkdir -p $(PREFIX)/sbin; \
	if [ ! -e fingerprint ] || [ ! -e finger_syncli ] ; then \
		false; \
	else \
		install -c fingerprint $(PREFIX)/sbin ; \
		install -c finger_syncli $(PREFIX)/sbin ; \
	fi

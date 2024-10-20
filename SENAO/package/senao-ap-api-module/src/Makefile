PRODUCT_CONFIG_MAKE = $(TOPDIR)/SENAO/configs/product_config.make
ifeq ($(wildcard $(PRODUCT_CONFIG_MAKE)),$(PRODUCT_CONFIG_MAKE))
include $(PRODUCT_CONFIG_MAKE)
endif

INCLUDES = -I./include \
        $(EXTRA_INCLUDE)

OBJS = sn_api.o ext_api.o main.o jwt_api.o

ifeq ($(CONFIG_PACKAGE_senao-packeteer),y)
CCFLAGS += -DHAS_SENAO_PACKETEER=1
endif

.PHONY:all clean install

all: api

api:$(OBJS)
	$(CC) -o api.fcgi $^ -ljson-c -lssl -lcrypto -lfcgi -lm -lapi -lsnjson $(INCLUDES) $(EXTRA_CFLAGS) $(EXTRA_INCLUDE) $(CCFLAGS)

%.o: %.c
	echo 'CC $@'
	$(CC) -c $< -o $@ $(CFLAGS) -I$(SN_CONFIGS_DIR) $(INCLUDES) $(EXTRA_CFLAGS) $(CCFLAGS)

clean:
	rm -rf *.o

install:


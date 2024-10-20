PRODUCT_CONFIG_MAKE = $(TOPDIR)/SENAO/configs/product_config.make
ifeq ($(wildcard $(PRODUCT_CONFIG_MAKE)),$(PRODUCT_CONFIG_MAKE))
include $(PRODUCT_CONFIG_MAKE)
endif

.PHONY:	all clean 

CCFLAGS= $(EXTRA_CFLAGS)
CC = $(CROSS)gcc
AR = $(CROSS)ar
LD = $(CROSS)ld
LDFLAGS=$(EXTRA_LDFLAGS)

APPAGENT_UTIL_DIR=../appagent_util
CONFIG_DIR= $(COPTS)

LIBS += $(APPAGENT_UTIL_DIR)/appagentutil.a \
        -L. \
	-L$(STAGING_DIR)/usr/lib/ \
	-lsysutil \
	-lapi

INCLUDES = -I./ \
	   -I$(APPAGENT_UTIL_DIR) \
	   -I../includes \
	   -I./setting/include \
	   -I$(wildcard $(BUILD_DIR)/uci-*) \
	   -I$(wildcard $(BUILD_DIR)/senao-sysutil-*/include) \
	   -I$(wildcard $(BUILD_DIR)/libapi-*/include) \
	   -I$(CONFIG_DIR)

INCLUDES += $(EXTRA_INCLUDE)

LIBS += -lpthread
LIBS += $(EXTRA_LIBS)

all: depend app_agentd 

OBJS =	main.o appagent_cfg_data.o appagent_cfg.o appagentd.o envcfg.o appagents.o \
		setting/app_agent.o setting/sitecom_setting.o setting/account_setting.o

# JSON Setting
OBJS += setting/json_setting.o

# USB Setting
ifeq ($(CONFIG_APP_AGENT_SUPPORT_USB_SETTING),y)
OBJS += setting/usb_setting.o setting/usb_json_setting.o
endif

# WAN Setting
ifeq ($(CONFIG_SUPPORT_WAN_SETTING),y)
OBJS += setting/wan_setting.o
endif

# MESH settings
ifeq ($(CONFIG_APP_AGENT_SUPPORT_MESH_JSON),y)
OBJS += setting/mesh_setting.o setting/mesh_json_setting.o
endif

# Device Setting
OBJS += setting/device_setting.o
OBJS += setting/device_json_setting.o

# LAN Setting
OBJS += setting/lan_setting.o
OBJS += setting/lan_json_setting.o

# WLAN Setting
OBJS += setting/wlan_setting.o
OBJS += setting/wlan_json_setting.o

OBJS += http_ssl.o
ifeq ($(CONFIG_USE_LIB_MATRIXSSL),y)
OBJS += matrixssl_helper.o
DEFINES += -DUSE_MATRIXSSL=1
else
DEFINES += -DUSE_OPENSSL=1
endif

# COMMAND LIST
ifeq ($(CONFIG_APP_AGENT_SUPPORT_COMMAND_LIST),y)
OBJS += command_list.o
endif

# DDNS
OBJS += setting/ddns_setting.o

# IPCAM settings
ifeq ($(CONFIG_ROUTER_SUPPORT_IPCAM),y)
OBJS += setting/ipcam_setting.o setting/ipcam_json_setting.o
endif

# Deubg
CCFLAGS += -UDEBUG -UXML_DEBUG -USUPPORT_DEBUG_TOOL

CONFIG_FUNC_LIST=\
	CONFIG_SUPPORT_WLAN_5G_SETTING \
	CONFIG_SUPPORT_IPV6_SETTING \
	CONFIG_SUPPORT_DHCP6S_SETTING \
	CONFIG_SUPPORT_WAN_SETTING \
	CONFIG_SUPPORT_IPERF_THROUGHPUT_TEST \
	CONFIG_SUPPORT_PEOPLE_FUNCTION \
	CONFIG_SUPPORT_CONFIG_HAS_SECTIONNAME \
	CONFIG_SUPPORT_SIMPLE_NVR \
	CONFIG_SUPPORT_FIREWALL_URL_CATEGORY \
	CONFIG_SUPPORT_MESH_AUTO_CHAN

CCFLAGS += $(foreach n, $(CONFIG_FUNC_LIST), $(if $(filter y m, $($(n))),-D$(patsubst CONFIG_SUPPORT_%,SUPPORT_%=1,$(n))))

ifeq ($(CONFIG_ROUTER_SUPPORT_IPCAM),y)
CCFLAGS += -DROUTER_SUPPORT_IPCAM=1
endif

ifeq ($(CONFIG_PACKAGE_senao_capwap_c1.8.x),y)
CCFLAGS += -DSUPPORT_CAPWAP_CONTROL=1
endif

ifeq ($(CONFIG_SUPPORT_AP_MESH_PROJECT),y)
CCFLAGS += -DSUPPORT_AP_MESH_PROJECT=1
endif

ifeq ($(CONFIG_APP_AGENT_SUPPORT_BATMAN_2019),y)
CCFLAGS += -DSUPPORT_BATMAN_2019=1
endif
#####################
###   Functions   ###
#####################

app_agentd: $(OBJS) $(APPAGENT_UTIL_DIR)/appagentutil.a
	$(CC) $(INCLUDES) $(CCFLAGS) $(CFLAGS) $(OBJS) $(LIBS) $(LDFLAGS) -o ../$@

%.o: %.c
	@echo 'CC $@'
	@$(CC) $(DEFINES) $(INCLUDES) $(CCFLAGS) -c $< -o $@

clean:
	rm -f ../app_agentd *.o *.core core.* .depend
	rm -f setting/*.o

depend: .depend

.depend:
	@CC=$(CC) ../mkdep $(CFLAGS) $(INCLUDES) $(patsubst %.o,%.c,$(OBJS))

ifneq ($(wildcard .depend),)
include .depend
endif

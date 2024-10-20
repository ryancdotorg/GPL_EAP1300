PRODUCT_CONFIG_MAKE = $(TOPDIR)/SENAO/configs/product_config.make
ifeq ($(wildcard $(PRODUCT_CONFIG_MAKE)),$(PRODUCT_CONFIG_MAKE))
include $(PRODUCT_CONFIG_MAKE)
endif

#-include $(TOP_DIR)/SENAO/configs/senao-frame-server/product_feature.make

prefix=/usr
DESTDIR=

COPTS=-O2
FPIC=-fPIC
CFLAGS+=$(COPTS) $(WOPTS) -std=gnu99

AR=ar
LD=ld
CC=gcc
RANLIB=ranlib
INSTALL=install
MKDIR=mkdir

ifeq ($(DEBUG),1)
  COPTS = -O0
  CFLAGS += -g3
endif
OS=$(shell uname)
ifeq ($(OS),Darwin)
  LINK=$(LD)
  SHLIB_EXT=dylib
  SHLIB_FLAGS=-dylib
else
  LINK=$(CC)
  SHLIB_EXT=so
  SHLIB_FLAGS=-shared -Wl,-soname,$(SHLIB_FILE)
endif

ifeq ($(CONFIG_PACKAGE_senao-ipcam-api-module),y)
  JSON_API_OBJ = api_json_check.c api_response.c json_common.c stack.c
  JSON_API_OBJ += $(wildcard ./ipcam/*.c)
#  JSON_API_OBJ = ${JSON_API_OBJ//json_ipcam.c/}
else
  JSON_API_OBJ = $(wildcard *.c)
endif

ifeq ($(CONFIG_CPAPWAP_SUPPORT_CAPTIVE_PORTAL_SETTING),y)
CFLAGS += -DSUPPORT_CAPTIVE_PORTAL_SETTING=1
endif

CFLAGS += -DHAS_MESH_SETTING=1
VARIABLE_API_OBJS += api_mesh.c
VARIABLE_API_INCLUDES += api_mesh.h
ifeq ($(CONFIG_SUPPORT_MESH_SETTING),y)
ifeq ($(CONFIG_APP_AGENT_SUPPORT_MESH_JSON),y)
CFLAGS += -DAPP_AGENT_SUPPORT_MESH_JSON=1
CFLAGS += -DHAS_MESH_JSON=1
endif
ifeq ($(CONFIG_PACKAGE_alfred),y)
CFLAGS += -DHAS_ALFRED=1
endif
endif

ifeq ($(CONFIG_PACKAGE_senao-packeteer),y)
CFLAGS += -DHAS_SENAO_PACKETEER=1
endif

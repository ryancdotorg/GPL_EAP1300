# *******************************************************************************************
# * Copyright (c) 2006-7 Laboratorio di Sistemi di Elaborazione e Bioingegneria Informatica *
# *                      Universita' Campus BioMedico - Italy                               *
# *                                                                                         *
# * This program is free software; you can redistribute it and/or modify it under the terms *
# * of the GNU General Public License as published by the Free Software Foundation; either  *
# * version 2 of the License, or (at your option) any later version.                        *
# *                                                                                         *
# * This program is distributed in the hope that it will be useful, but WITHOUT ANY         *
# * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A 	    *
# * PARTICULAR PURPOSE. See the GNU General Public License for more details.                *
# *                                                                                         *
# * You should have received a copy of the GNU General Public License along with this       *
# * program; if not, write to the:                                                          *
# * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,                    *
# * MA  02111-1307, USA.                                                                    *
# *                                                                                         *
# * --------------------------------------------------------------------------------------- *
# * Project:  Capwap                                                                        *
# *                                                                                         *
# * Author :  Ludovico Rossi (ludo@bluepixysw.com)                                          *
# *           Del Moro Andrea (andrea_delmoro@libero.it)                                    *
# *           Giovannini Federica (giovannini.federica@gmail.com)                           *
# *           Massimo Vellucci (m.vellucci@unicampus.it)                                    *
# *	      Donato Capitella (d.capitella@gmail.com)  	                                    *
# *******************************************************************************************
#PRODUCT_CONFIG_MAKE = $(TOPDIR)/SENAO/configs/product_config.make
#ifeq ($(wildcard $(PRODUCT_CONFIG_MAKE)),$(PRODUCT_CONFIG_MAKE))
#include $(PRODUCT_CONFIG_MAKE)
#endif

AC_LDFLAGS = $(STAGING_DIR)/usr/lib/libssl.so $(STAGING_DIR)/usr/lib/libcrypto.so -ldal -lpthread -ldl -lapi -lsysutil -L${DAL_DIR}
WTP_LDFLAGS = $(STAGING_DIR)/usr/lib/libssl.so $(STAGING_DIR)/usr/lib/libcrypto.so -lpthread -ldl -lm -lrt -lfcgi -ljson-c -lapi -lsnjson -lsysutil
WUM_LDFLAGS = -lm
LDFLAGS += ${TARGET_LDFLAGS}
CC_FLAGS = -Wall -O2 -D_REENTRANT -I$(STAGING_DIR)/usr/include/ -I${SQLITE_INC_DIR} -I${DAL_DIR} -I. -I$(SN_CONFIGS_DIR) -I$(SN_INCLUDE_DIR) -I$(STAGING_DIR)/usr/include/ -I$(STAGING_DIR)/usr/include/sysutil -I$(STAGING_DIR)/usr/include/libapi -I$(STAGING_DIR)/usr/include/api-json -I$(STAGING_DIR)/usr/include/json-c ${TARGET_CFLAGS} ${EXTRA_CFLAGS}

# Memory leak
#LDFLAGS += ../dmalloc-5.5.0/libdmallocth.a
#CFLAGS += -DDMALLOC

# Debug Flag
CC_FLAGS += -DCW_DEBUGGING
CC_FLAGS += -DCW_WTP_AP 
PRODUCT_NAME=$(shell cat $(TOPDIR)/SENAO/configs/gconfig.mk | grep "CONFIG_MODEL_NAME=" | cut -d '=' -f 2 | sed -n 's/"//gp')
CW_VERSION_TARGET:=$(shell ( grep "CW_VERSION_$${PRODUCT_NAME}" CWVersion.h || grep CW_VERSION_UNDEF_MODEL CWVersion.h ) | awk -F\  '{print $$3}')
CC_FLAGS += -DCW_VERSION_TARGET=$(CW_VERSION_TARGET)
DEF_AP_WIFI_IFACE_NUM=$(shell cat $(TOPDIR)/SENAO/configs/product_config.make | grep "CONFIG_WEB_DEF_AP_WIFI_IFACE_NUM=" | cut -d '=' -f 2 | sed -n 's/"//gp')
CC_FLAGS += -DCW_DEF_AP_WIFI_IFACE_NUM=$(DEF_AP_WIFI_IFACE_NUM)
RM = /bin/rm -f

# list of generated object files for AC. 
AC_SRCS = AC.c ACConfigFile.c ACMainLoop.c ACDiscoveryState.c ACJoinState.c ACConfigureState.c ACDataCheckState.c \
			ACRunState.c ACProtocol_User.c ACRetransmission.c CWCommon.c CWConfigFile.c CWErrorHandling.c CWList.c \
			CWLog.c ACMultiHomedSocket.c  ACProtocol.c CWSafeList.c CWNetwork.c CWProtocol.c CWRandom.c CWSecurity.c \
			CWOpenSSLBio.c CWStevens.c CWThread.c CWBinding.c CWVendorPayloadsAC.c ACBinding.c ACInterface.c \
			ACSettingsFile.c timerlib.c ACDataBase.c ACImageState.c ACCertResetState.c avl.c

# list of generated object files for API
API_SRCS = WTPBoardApiCommon.c WTPBoardApiDevice.c WTPBoardApiSystem.c WTPBoardApiNetwork.c WTPBoardApiMesh.c

# list of generated object files for WTP. 
WTP_SRCS = WTP.c WTPConfigFile.c \
			WTPProtocol.c WTPProtocol_User.c WTPDiscoveryState.c WTPJoinState.c WTPConfigureState.c \
			WTPDataCheckState.c WTPRunState.c WTPRetransmission.c WTPSulkingState.c WTPProxyMode.c \
			CWCommon.c CWConfigFile.c CWErrorHandling.c CWSafeList.c CWList.c CWLog.c CWNetwork.c CWProtocol.c \
			CWRandom.c CWSecurity.c CWOpenSSLBio.c CWStevens.c CWThread.c CWVendorPayloadsWTP.c \
			WTPSettingsFile.c timerlib.c WTPImageState.c WTPCertResetState.c WTPRunStateCheck.c  WTPExternalSignalHandling.c

ifeq (SUPPORT_CONFIG_HAS_SECTIONNAME,$(findstring SUPPORT_CONFIG_HAS_SECTIONNAME,$(EXTRA_CFLAGS)))
API_SRCS += WTPBoardApiWireless_wn.c
else
API_SRCS += WTPBoardApiWireless.c
endif

ifeq (CW_WTP_AP,$(findstring CW_WTP_AP,$(CC_FLAGS)))
WTP_SRCS += WTPDataRecordStatistics.c WTPSitesurvey.c
endif
          
ifeq (CW_WTP_SWITCH,$(findstring CW_WTP_SWITCH,$(CC_FLAGS)))
WTP_SRCS += cJSON.c
endif

ifeq (SUPPORT_CAPTIVE_PORTAL_SETTING,$(findstring SUPPORT_CAPTIVE_PORTAL_SETTING,$(EXTRA_CFLAGS)))
API_SRCS += WTPBoardApiPortal.c
endif

ifeq (SUPPORT_CONFIG_HAS_SECTIONNAME,$(findstring SUPPORT_CONFIG_HAS_SECTIONNAME,$(EXTRA_CFLAGS)))
API_SRCS += WTPBoardApiNas_wn.c
else
API_SRCS += WTPBoardApiNas.c
endif

WUA_SRCS = WUA.c

WUM_SRCS = wum/wum.c wum/wumLib.c

AC_OBJS = $(patsubst %.c,%.o,$(AC_SRCS))
WTP_OBJS = $(patsubst %.c,%.o,$(WTP_SRCS))
API_OBJS = $(patsubst %.c,setting/%.o,$(API_SRCS))
WUA_OBJS = $(patsubst %.c,%.o,$(WUA_SRCS))
WUM_OBJS = $(patsubst %.c,%.o,$(WUM_SRCS))

AC_DEPS := $(patsubst %.o,%.d,$(AC_OBJS))
WTP_DEPS := $(patsubst %.o,%.d,$(WTP_OBJS))
API_DEPS := $(patsubst %.o,%.d,$(API_OBJS))
WUA_DEPS := $(patsubst %.o,%.d,$(WUA_OBJS))

# program executables.
AC_NAME = ac.bin
WTP_NAME = wtp.bin
WUA_NAME = wua.bin
WUM_NAME = wum/wum.bin

.PHONY: clean

# top-level rule, to compile everything.
all: $(WUM_NAME) $(AC_NAME) $(WTP_NAME)

%.o : %.c
	$(CC) -c $< $(CC_FLAGS) -o $@

$(AC_NAME): $(AC_OBJS)
	$(CC) $(AC_OBJS) $(CC_FLAGS) $(LDFLAGS) $(AC_LDFLAGS) -o $@

$(WTP_NAME): $(API_OBJS) $(WTP_OBJS)
	$(CC) $(API_OBJS) $(WTP_OBJS) $(CC_FLAGS) $(LDFLAGS) $(WTP_LDFLAGS) -o $@
	

$(WUA_NAME): $(WUA_OBJS)
	$(CC) $(WUA_OBJS) $(CC_FLAGS) $(LDFLAGS) -o $@

$(WUM_NAME): $(WUM_OBJS)
	$(CC) $(WUM_OBJS) $(CC_FLAGS) $(LDFLAGS) $(WUM_LDFLAGS) -o $@

clean: 
ifeq ($(strip $(TARGET)),)
	$(RM) *.bin wum/*.bin *.o wum/*.o $(AC_DEPS) $(WTP_DEPS)
else
	$(RM) $(AC_NAME) $(WTP_NAME) $(WUA_NAME) $(WUM_NAME) $(AC_OBJS) $(WTP_OBJS) $(WUA_OBJS) $(WUM_OBJS) $(AC_DEPS) $(WTP_DEPS)
endif

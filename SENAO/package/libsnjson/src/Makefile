include Makefile.inc

LIBS= \
	-L. \
	-L$(STAGING_DIR)/usr/lib/ \
	-luci \
	-lpcre \
	-lapi \
	-lsqlite3 \
	-lsysutil

INCLUDES= \
	-I. \
	-I./include \
	-I$(STAGING_DIR)/usr/include/sysutil \
	-I$(STAGING_DIR)/usr/include/libapi \
	$(EXTRA_INCLUDE)

ifeq ($(CONFIG_PACKAGE_senao-ipcam-api-module),y)
	INCLUDES += -I./ipcam/include
endif

ifeq ($(CONFIG_PACKAGE_senao-frame-server),y)
	INCLUDES += -I../../../SENAO/package/senao-frame-server/src/systemUtilities/h \
				-I$(wildcard $(BUILD_DIR)/senao-frame-server-*/include)
endif

all: libsnjson.$(SHLIB_EXT)

libsnjson.$(SHLIB_EXT): $(JSON_API_OBJ)
	$(LINK) $(CFLAGS) $(EXTRA_CFLAGS) $(SHLIB_FLAGS) $(FPIC) -o $@ $^ $(LIBS) $(INCLUDES) $(EXTRA_LIBS)

clean:
	@rm -f *.so* *.o

install: all
	$(MKDIR) -p $(DESTDIR)$(prefix)/lib
	$(INSTALL) -m0755 $(SHLIB_FILE) $(DESTDIR)$(prefix)/lib/

FORCE: ;
.PHONY: FORCE

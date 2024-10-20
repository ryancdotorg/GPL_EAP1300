ifeq ($(CONFIG_PROCD_INIT),)
LOCAL_FILES_DIR:=./files
else
LOCAL_FILES_DIR:=./files.procd
endif

### the whole directory
define InstallFiles
	$(CP) ./files/* $(1)/
endef

define InstallOldFiles
	$(CP) ./files.old/* $(1)/
endef

define InstallLocalFiles
	$(CP) $(LOCAL_FILES_DIR)/* $(1)/
endef

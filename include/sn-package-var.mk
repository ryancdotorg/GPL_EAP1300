SnReleaseVer_script=$(shell \
  PKG_HEAD_REV=`git rev-parse $(PKG_SOURCE_URL) --short HEAD`; \
  if [ -z "$(1)" ]; then \
    echo $$PKG_HEAD_REV; \
  else \
    _PKG_TAG_REV=`git rev-parse $(PKG_SOURCE_URL) --short v$(PKG_VERSION)-rc$(1)`; \
    PKG_TAG_REV=`git log -1 $$_PKG_TAG_REV  --format=format:%h`; \
    if [ "$$PKG_TAG_REV" = "$$PKG_HEAD_REV" ]; then \
      echo $(1);\
    else \
      echo $(1)-g$$PKG_HEAD_REV; \
    fi; \
  fi; \
)

define SnReleaseVer
$(call SnReleaseVer_script,$1)
endef



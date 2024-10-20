PRODUCT_NAME = ENGENIUS

.PHONY:	all clean install 

CFLAGS= -pipe -Wno-deprecated

APPAGENT_APPS=appagent_util app_agent 

MY_CFLAGS += $(EXTRA_CFLAGS)

MY_CFLAGS += -DAPP_AGENTD_HAS_SSL=1

all: build-appagent

build-appagent:
ifneq ($(APPAGENT_APPS),)
	for d in $(APPAGENT_APPS); do \
		  (cd $$d; $(MAKE) EXTRA_CFLAGS="$(MY_CFLAGS)" CROSS=$(CROSS) -f Makefile;); \
	done
endif
	#sh ./gencert.sh $(PRODUCT_NAME) ./agentssl.cnf $(PWD)/agent.pem $(PRODUCT_NAME);

%.o: %.c
	@echo 'CC $@'
	@$(CC) $(DEFINES) $(INCLUDES) $(CFLAGS) $(COPTS) -c $< -o $@

clean:
ifneq ($(APPAGENT_APPS),)
	for d in $(APPAGENT_APPS); do \
		  $(MAKE) -C $$d -f Makefile clean; \
	done
endif

install:
	install -c app_agentd $(PREFIX)/sbin
	install -c agent.pem $(PREFIX)/etc

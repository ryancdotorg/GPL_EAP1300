INSTALL_DATADIR=\"/usr/share\"
INCLUDES += $(EXTRA_INCLUDE) -I./include

all: server

server: server.c wamp_topic.o wamp.o
	$(CC) $^ -o $@ $(LDFLAGS) $(LIBS) $(INCLUDES) $(CFLAGS) -I$(SN_CONFIGS_DIR) -DINSTALL_DATADIR=$(INSTALL_DATADIR)

%.o: %.c
	echo 'CC $@'
	$(CC) -c $< -o $@ $(LDFLAGS) $(LIBS) $(INCLUDES) $(CFLAGS) -I$(SN_CONFIGS_DIR)

clean:
	rm -f server

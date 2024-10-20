.PHONY: all clean

CC = $(CROSS)gcc
AR = $(CROSS)ar
LD = $(CROSS)ld
RANLIB = $(CROSS)ranlib
CFLAGS += -Wall -pipe $(EXTRA_CFLAGS)
LDFLAGS = $(EXTRA_CFLAGS) -I.

LIBS = -lssl -lcrypto

EXEC = app_client

OBJS=app_client.o aes.o base64.o

all: $(EXEC)

rebuild: clean all

%.o: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) $(CCFLAGS) -c -o $@ $<

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $? $(LIBS)

clean:
	-rm -f $(EXEC) *.o *~

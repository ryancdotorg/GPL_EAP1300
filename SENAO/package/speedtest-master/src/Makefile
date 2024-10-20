EXEC = speedtest-master

speedtest-master: speedtest.o
	$(CC) $(CFLAGS) $(LDFLAGS) $(GSOAP_OBJS) speedtest.o -lpthread -lm -o speedtest-master
	$(STRIP) speedtest-master

speedtest-master.o:	speedtest.c
	$(CC) $(CFLAGS) $(LDFLAGS) $(INCLUDES) -c  speedtest.c

clean:
	-rm -f $(EXEC) *.o *~; \
	find ./ -name "*~" | xargs rm -rf; \
	find ./ -name "*.o" | xargs rm -rf

#include "mio.h"
#include "icmp.h"
#include "util.h"
#include <signal.h>
#include <arpa/inet.h>
#include <string.h>
#include <netinet/ip_icmp.h>

const int DEFDATALEN = 56;
const int MAXIPLEN = 60;
const int MAXICMPLEN = 76;
void et_func1(void *mio_data, void *event_data)
{
	int fd = *((int *)event_data);

	printf("fd: %d\n", fd);
        int res = -1;
	char PDU[DEFDATALEN + MAXIPLEN + MAXICMPLEN];
	struct sockaddr_in from;
	socklen_t fromlen;
	unsigned char ipBytes[4];
	struct iphdr *iphdr;
	struct icmp *pkt;

        memset(&from, 0, sizeof(struct sockaddr_in));
	fromlen = sizeof(from);
	res= recvfrom (fd, PDU, sizeof(PDU), 0,
			(struct sockaddr *) &from, &fromlen);
	DumpHex(PDU, res);
        if(res == -1 || res<76) {
           printf("recvfrom error %d\n", res);
           //return -1;
        }

	if (res >= 76) {			/* ip + icmp */
            memcpy(&(ipBytes[0]), &from.sin_addr, 4);
	    iphdr = (struct iphdr *) PDU;
	    pkt = (struct icmp *) (PDU + (iphdr->ihl << 2));
	    if (pkt->icmp_type == ICMP_ECHOREPLY) {
           	printf("ping success\n\n");
            //	return 0;
	    }
	}
}
typedef struct icmp_data_t {
	int fd;
	unsigned char ipBytes[4];
}icmp_data_t;

void tm_func1(void *mio_data, void *data)
{
	struct icmp_data_t *icmpdata = (struct icmp_data_t *) data;
	send_icmpv4(icmpdata->fd, icmpdata->ipBytes);
}

int main(int argc, char *argv[])
{
	mio_data_t *mio_data;
	mio_event_t *e1;
	mio_data = mio_init();
	int fd = create_socket_icmpv4();
	e1 = add_event(&(mio_data->read_pool), fd, et_func1, &fd);

	icmp_data_t *icmpdata;
	icmpdata = malloc(sizeof(struct icmp_data_t));
	icmpdata->fd = fd;

	in_addr_t addr;
	addr = inet_addr("168.95.1.1");
	//addr = inet_addr("1.2.3.4");
	memcpy(icmpdata->ipBytes, (unsigned char *)&addr, 4);
	add_timer(&(mio_data->timer_pool), 5, 0, tm_func1, (void *)icmpdata, 1);

	mio_loop(mio_data);
	mio_uninit(mio_data);
}

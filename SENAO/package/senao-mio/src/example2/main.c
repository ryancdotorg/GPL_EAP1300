#include "mio.h"
#include "util.h"
#include <signal.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/un.h>
#include <unistd.h>

void et_func1(void *mio_data, void *event_data)
{
	char recvbuf[1024];
	int fd = *((int *)event_data);

	printf("fd: %d\n", fd);
        int res = -1;
	struct sockaddr_in6 from;
	socklen_t fromlen;

        memset(&from, 0, sizeof(struct sockaddr_in6));
	fromlen = sizeof(from);
	res= recvfrom (fd, recvbuf, sizeof(recvbuf), 0,
			(struct sockaddr *) &from, &fromlen);
	DumpHex(recvbuf, res);
}

int create_socket(int domain, void *access_point){
	int server_sockfd;
	int server_len;
	int port = *((int *)access_point);

	server_sockfd = socket(domain, SOCK_DGRAM, 0);
	if (domain == PF_INET6){
		struct sockaddr_in6 server_address;
		int port = *((int *)access_point);
		server_address.sin6_family = domain;
		server_address.sin6_addr = in6addr_any;
		server_address.sin6_port = htons(port);
		server_len = sizeof(server_address);
		bind(server_sockfd, (struct sockaddr *)&server_address, server_len);
		listen(server_sockfd, 5);
	}
	return server_sockfd;
}

int main(int argc, char *argv[])
{
	mio_data_t *mio_data;
	mio_event_t *e1;
	mio_data = mio_init();
	int port=547;
	port = atoi(argv[1]);
	printf("port:[%d]\n", port);
	int fd = create_socket(PF_INET6, &port);
	e1 = add_event(&(mio_data->read_pool), fd, et_func1, &fd);
	mio_loop(mio_data);
	mio_uninit(mio_data);
}

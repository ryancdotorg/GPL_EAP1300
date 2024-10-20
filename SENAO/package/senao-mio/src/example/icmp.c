#include <netinet/ip_icmp.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

static int short icmp_checksum (unsigned short *buf, int bytes)
{
    unsigned int i;
    unsigned short  *p = buf;
    unsigned short sum = 0;

    /* Sum */
    for (i=bytes; i > 1; i -= 2)
        sum += *p++;

    /* If uneven length */
    if (i > 0)
        sum += *((unsigned char *) (p));

    /* Carry */
    while ((sum & 0xFFFF0000) != 0)
        sum = (sum >> 16) + (sum & 0xFFFF);

    return ~(sum);
}

int create_socket_icmpv4()
{
	int sockfd;
	sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (sockfd < 0)
	{
		perror("socket error:");
	}
	return sockfd;
}

void send_icmpv4(int sockfd, unsigned char *ipBytes)
{
	const int DEFDATALEN = 56;
	const int MAXIPLEN = 60;
	const int MAXICMPLEN = 76;
	struct sockaddr_in destaddr;
	struct icmp *pkt;
	char sendpacket[DEFDATALEN + MAXIPLEN + MAXICMPLEN];

	memset(&destaddr, 0,sizeof(struct sockaddr_in));
	memset(sendpacket, 0, sizeof(sendpacket));

	// Set IP msg 
	destaddr.sin_family = AF_INET;
	memcpy(&destaddr.sin_addr, ipBytes, 4);

	// Set ICMP msg 
	pkt = (struct icmp*)sendpacket;
	pkt->icmp_type = ICMP_ECHO;
	pkt->icmp_cksum = icmp_checksum((unsigned short *)pkt, sizeof(struct icmp));
	if (sendto(sockfd, sendpacket, sizeof(sendpacket), 0, (struct sockaddr *)&destaddr, sizeof(destaddr)) == -1)
	{
		printf("sendto error\n");
	}

}

/*
int main(int argc, char **argv)
{
	int sockfd;
	sockfd = create_socket_icmpv4();
	in_addr_t addr;
	addr = inet_addr(argv[1]);
	unsigned char *ipBytes;
	ipBytes = (unsigned char *)&addr;
	send_icmpv4(sockfd, ipBytes);

	return 0;
}
*/

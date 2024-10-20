/*  Copyright (C) 2011-2015  P.D. Buchan (pdbuchan@yahoo.com)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// Send an IPv4 ICMP echo request packet via raw socket at the link layer (ethernet frame),
// and receive echo reply packet (i.e., ping). Includes some ICMP data.
// Need to have destination MAC address.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>           // close()
#include <string.h>           // strcpy, memset(), and memcpy()

#include <netdb.h>            // struct addrinfo
#include <sys/types.h>        // needed for socket(), uint8_t, uint16_t, uint32_t
#include <sys/socket.h>       // needed for socket()
#include <netinet/in.h>       // IPPROTO_ICMP, INET_ADDRSTRLEN
#include <netinet/ip.h>       // struct ip and IP_MAXPACKET (which is 65535)
#include <net/if_arp.h>
#include <arpa/inet.h>        // inet_pton() and inet_ntop()
#include <sys/ioctl.h>        // macro ioctl is defined
#if TOOLCHAIN_MUSL
#include <asm/ioctls.h>
#else
#include <bits/ioctls.h>      // defines values for argument "request" of ioctl.
#include <net/ethernet.h>
#endif
#include <net/if.h>           // struct ifreq
#include <linux/if_ether.h>   // ETH_P_IP = 0x0800, ETH_P_IPV6 = 0x86DD
#include <linux/if_packet.h>  // struct sockaddr_ll (see man 7 packet)
#include <sys/time.h>         // gettimeofday()

#include <errno.h>            // errno, perror()

// Define some constants.
#define ETH_HDRLEN 14  // Ethernet header length
#define ARP_HDRLEN 8
#define ARP_DATALEN (6+4+6+4)	//mac+ip+mac+ip

#define DEBUG_PRINT 0
#define USE_RECVMSG_FOR_INTERFACE_INFO	0
#define USE_CHANGE_TARGET_IP_IN_FUNCTION 1

#define SEND_PACKET_INTERFACE "br-lan"
#define DEFAULT_SUBNET_IP_NUM	255
#define DEFAULT_SUBNET_MASK	"255.255.255.0"
// Function prototypes
uint16_t checksum (uint16_t *, int);
char *allocate_strmem (int);
uint8_t *allocate_ustrmem (int);
int *allocate_intmem (int);
void sendPacketToCheckAlive(char *interface, char *targetIp, char *source);
void sendArpToLanAll(char *interface, char *targetIp, uint16_t sendIntervalms);
struct arp_data 
{
	char senderMac[6];
	char senderIP[4];
	char recverMac[6];
	char recverIP[4];
};

void usage(char *argv0)
{
	fprintf(stderr,"Usage: %s \n\
	[-I interface] [-c continuous send arp times] [-S source IP] \n\
	[-T target IP][-d ping timeout] [-m ping m_timeout] \n\
	[-A lan ip range] [-i send interval in ms(0~16383)] \n\
	[-x just send arp packet, this argument should be the last]\n", argv0);
}

int
main (int argc, char **argv)
{
  int i, status, datalen, frame_length, sendsd, recvsd, bytes, *ip_flags, trycount, trylim, done, timeout=0, m_timeout=0, c; 
  uint16_t sendInterval=10; //in ms
  uint8_t *data, *src_mac, *dst_mac, *send_ether_frame, *recv_ether_frame, sendCount=0, sendTimes=1, useEthAll=0;
  int dst_mac_str[6];
  char *interface, *target, *src_ip, *dst_ip, *rec_ip;
#if USE_RECVMSG_FOR_INTERFACE_INFO
  char *recv_interface;
#endif
  char src_inf_mac[17];
  char send_mac[18], recv_mac[18];	
  struct addrinfo hints, *res;

  struct sockaddr_in *ipv4;
  struct sockaddr_ll device;
  struct ifreq ifr;
  struct sockaddr from;
  socklen_t fromlen;
  struct timeval wait, t1, t2;
  struct timezone tz;
  char lanIpRange[12];
  struct arphdr send_arphdr, *recv_arphdr;
  double dt;
  void *tmp;
  struct in_addr  s_in, t_in;
  struct arp_data send_arpdata, *recv_arp_data;

  // Allocate memory for various arrays.
  src_mac = allocate_ustrmem (6);
  dst_mac = allocate_ustrmem (6);
  data = allocate_ustrmem (IP_MAXPACKET);
  send_ether_frame = allocate_ustrmem (IP_MAXPACKET);
  recv_ether_frame = allocate_ustrmem (IP_MAXPACKET);
  interface = allocate_strmem (40);
#if USE_RECVMSG_FOR_INTERFACE_INFO
  recv_interface = allocate_strmem (40);
#endif
  target = allocate_strmem (40);
  src_ip = allocate_strmem (INET_ADDRSTRLEN);
  dst_ip = allocate_strmem (INET_ADDRSTRLEN);
  rec_ip = allocate_strmem (INET_ADDRSTRLEN);
  ip_flags = allocate_intmem (4);



  for (;;) {
	c = getopt( argc, argv, "I:S:T:d:m:A:c:i:xa");
	if (c == EOF) break;
	switch (c) {
	case 'I':
		strcpy(interface, optarg);
		break;
	case 'S':
		strcpy(src_ip, optarg);
		break;
	case 'T':
		strcpy(target, optarg);
		break;
	case 'i': 
		sendInterval = atoi(optarg);
		break;
	case 'A':
		strcpy(lanIpRange, optarg);
		if(sendTimes >= 1)
		{
#if USE_CHANGE_TARGET_IP_IN_FUNCTION
		    sendArpToLanAll(interface, lanIpRange, sendInterval);
#else
		    for(sendCount=0; sendCount<sendTimes; sendCount++)
		    {
			for(i=1; i<=DEFAULT_SUBNET_IP_NUM-1; i++)
			{
			    sprintf(dst_ip, "%s%d", lanIpRange, i);
			    if(sendInterval)
				    usleep(sendInterval*1000); //need sleep between arp sending to avoid too high traffic
		 	    sendPacketToCheckAlive(interface, dst_ip, src_ip);
			}
		    }
#endif
		}
		goto exit_free_mem;
		return;
	case 'x':
		sendPacketToCheckAlive(interface, target, src_ip);
		goto exit_free_mem;
		return;
	case 'c':
		sendTimes = atoi(optarg);
		break;
    case 'd':
        timeout = atoi(optarg);
        break;
    case 'm':
        m_timeout = atoi(optarg);
        break;
    case 'a':
	useEthAll=1;
	break;
    default:  // show usage
		usage(argv[0]);
		goto exit_free_mem;
		exit(1);
	}
  }

  // Interface to send packet through.
  if(strlen(interface)<1)
  { 
	printf("Please enter interface name  with -I\n");
        goto exit_free_mem;
  }

  // Submit request for a socket descriptor to look up interface.
  // We'll use it to send packets as well, so we leave it open.
  if ((sendsd = socket (PF_PACKET, SOCK_RAW, htons (ETH_P_ALL))) < 0) {
    perror ("socket() failed to get socket descriptor for using ioctl() ");
    goto exit_free_mem;
  }

  // Use ioctl() to look up interface name and get its MAC address.
  memset (&ifr, 0, sizeof (ifr));
  snprintf (ifr.ifr_name, sizeof (ifr.ifr_name), "%s", interface);
  if (ioctl (sendsd, SIOCGIFHWADDR, &ifr) < 0) {
    perror ("ioctl() failed to get source MAC address ");
    goto exit_free_mem;
  }

  // Copy source MAC address.
  memcpy (src_mac, ifr.ifr_hwaddr.sa_data, 6);

  // Report source MAC address to stdout.
  //printf ("MAC address for interface %s is ", interface);
  //for (i=0; i<5; i++) {
  //  printf ("%02x:", src_mac[i]);
  //}
  //printf ("%02x\n", src_mac[5]);

  // Find interface index from interface name and store index in
  // struct sockaddr_ll device, which will be used as an argument of sendto().
  memset (&device, 0, sizeof (device));
  if ((device.sll_ifindex = if_nametoindex (interface)) == 0) {
    perror ("if_nametoindex() failed to obtain interface index ");
    goto exit_free_mem;
  }
  //printf ("Index for interface %s is %i\n", interface, device.sll_ifindex);

  // Set destination MAC address: arp is always broadcast because we dont know it
    dst_mac[0] = 0xff;
    dst_mac[1] = 0xff;
    dst_mac[2] = 0xff;
    dst_mac[3] = 0xff;
    dst_mac[4] = 0xff;
    dst_mac[5] = 0xff;

  // Source IPv4 address: you need to fill this out
  if(strlen(src_ip)<7)
  {
	printf("Please enter source ip in format X.X.X.X with -S\n");
	goto exit_free_mem;
  }

  // Destination URL or IPv4 address: you need to fill this out
  if(strlen(target)<7)
  {
	printf("Please enter source ip in format X.X.X.X with -T\n");
        goto exit_free_mem;

  }

  // Fill out hints for getaddrinfo().
  memset (&hints, 0, sizeof (struct addrinfo));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = hints.ai_flags | AI_CANONNAME;

  // Resolve target using getaddrinfo().
  if ((status = getaddrinfo (target, NULL, &hints, &res)) != 0) {
    fprintf (stderr, "getaddrinfo() failed: %s\n", gai_strerror (status));
    goto exit_free_mem;
  }
  ipv4 = (struct sockaddr_in *) res->ai_addr;
  tmp = &(ipv4->sin_addr);
  if (inet_ntop (AF_INET, tmp, dst_ip, INET_ADDRSTRLEN) == NULL) {
    status = errno;
    fprintf (stderr, "inet_ntop() failed.\nError message: %s", strerror (status));
    goto exit_free_mem;
  }
  freeaddrinfo (res);

  // Fill out sockaddr_ll.
  device.sll_family = AF_PACKET;
  memcpy (device.sll_addr, src_mac, 6);
  device.sll_halen = 6;

  //  arp header
  send_arphdr.ar_hrd = htons(ARPHRD_ETHER);
  send_arphdr.ar_pro = htons(ETH_P_IP);
  send_arphdr.ar_hln = 6;
  send_arphdr.ar_pln = 4;
  send_arphdr.ar_op = htons(ARPOP_REQUEST);

  if ((status = inet_pton (AF_INET, src_ip, &(s_in.s_addr))) != 1) {
    fprintf (stderr, "inet_pton() failed.\nError message: %s", strerror (status));
    goto exit_free_mem;
  }

  if ((status = inet_pton (AF_INET, target, &(t_in.s_addr))) != 1) {
    fprintf (stderr, "inet_pton() failed.\nError message: %s", strerror (status));
    goto exit_free_mem;
  }

  memcpy(send_arpdata.senderMac, device.sll_addr, 6);
  memcpy(send_arpdata.senderIP, &(s_in.s_addr), 4);
  memcpy(send_arpdata.recverMac, dst_mac, 6);
  /*send_arpdata.recverMac[0]=0xff;
  send_arpdata.recverMac[1]=0xff;
  send_arpdata.recverMac[2]=0xff;
  send_arpdata.recverMac[3]=0xff;
  send_arpdata.recverMac[4]=0xff;
  send_arpdata.recverMac[5]=0xff;*/
  memcpy(send_arpdata.recverIP, &(t_in.s_addr), 4);
  
  // Fill out ethernet frame header.

  // Ethernet frame length = ethernet header (MAC + MAC + ethernet type) + ethernet data (ARP header)
  frame_length = ETH_HDRLEN +  ARP_HDRLEN + ARP_DATALEN;

  // Destination and Source MAC addresses
  memcpy (send_ether_frame, dst_mac, 6);
  memcpy (send_ether_frame + 6, src_mac, 6);

  // Next is ethernet type code (ETH_P_IP for IPv4).
  // http://www.iana.org/assignments/ethernet-numbers
  send_ether_frame[12] = ETH_P_ARP / 256;
  send_ether_frame[13] = ETH_P_ARP % 256;

  // Next is ethernet frame data (ARP header + ARP data.

  // ARP header
  memcpy (send_ether_frame + ETH_HDRLEN, &send_arphdr, ARP_HDRLEN);

  // ARP data
  memcpy (send_ether_frame + ETH_HDRLEN + ARP_HDRLEN, &send_arpdata, ARP_DATALEN);

  // Submit request for a raw socket descriptor to receive packets.
  if(useEthAll==1)
  {
    if ((recvsd = socket (PF_PACKET, SOCK_RAW, htons (ETH_P_ALL))) < 0){
      perror ("socket() failed to obtain a receive socket descriptor ");
      goto exit_free_mem;
    }
  }
  else
  {
    if ((recvsd = socket (PF_PACKET, SOCK_RAW, htons (ETH_P_ARP))) < 0){
      perror ("socket() failed to obtain a receive socket descriptor ");
      goto exit_free_mem;
    }
  }

  bind(recvsd, (struct sockaddr *)&device, sizeof(device));	//need this to recv packet from interface


  // Set maximum number of tries to ping remote host before giving up.
  trylim = 3;
  trycount = 0;

  // Cast recv_arphdr as pointer to arp header within received ethernet frame.
  recv_arphdr = (struct arphdr *) (recv_ether_frame + ETH_HDRLEN);
  recv_arp_data = (struct arp_data *) (recv_ether_frame + ETH_HDRLEN + ARP_HDRLEN);

  done = 0;
  for (;;) {

    // SEND

    // Send ethernet frame to socket.
    if ((bytes = sendto (sendsd, send_ether_frame, frame_length, 0, (struct sockaddr *) &device, sizeof (device))) <= 0) {
      perror ("sendto() failed ");
      goto exit_free_mem;
    }

    // Start timer.
    (void) gettimeofday (&t1, &tz);

    // Set time for the socket to timeout and give up waiting for a reply.
    if(timeout==0 && m_timeout==0)
    {
	    timeout = 2;	//set default timeout 2 sec if input without -d
        m_timeout = 0;
    }
    wait.tv_sec  = timeout;  
    wait.tv_usec = m_timeout;
    setsockopt (recvsd, SOL_SOCKET, SO_RCVTIMEO, (char *) &wait, sizeof (struct timeval));
#if USE_RECVMSG_FOR_INTERFACE_INFO
        int on = 1;
        struct iovec iov;
	struct msghdr msg;
        char cbuf[100];

        setsockopt(recvsd, SOL_IP, IP_PKTINFO, &on, sizeof(on));
        iov.iov_base = (void*)recv_ether_frame;
        iov.iov_len = sizeof(recv_ether_frame);
        msg.msg_name = &from;
        msg.msg_namelen = fromlen;
        msg.msg_iov = &iov;
        msg.msg_iovlen = 1;
        msg.msg_control = cbuf;
        msg.msg_controllen = sizeof(cbuf);

#endif
    // Listen for incoming ethernet frame from socket recvsd.
    // We expect an ICMP ethernet frame of the form:
    //     MAC (6 bytes) + MAC (6 bytes) + ethernet type (2 bytes)
    //     + ethernet data (IPv4 header + ICMP header)
    // Keep at it for 'timeout' seconds, or until we get an ICMP reply.

    // RECEIVE LOOP
    uint receiveLoop=0;
    for (;;) {
	receiveLoop++;
      memset (recv_ether_frame, 0, IP_MAXPACKET * sizeof (uint8_t));
      memset (&from, 0, sizeof (from));
      fromlen = sizeof (from);
#if USE_RECVMSG_FOR_INTERFACE_INFO
      if (bytes = recvmsg (recvsd, &msg, 0) < 0)
#else
      if ((bytes = recvfrom (recvsd, recv_ether_frame, IP_MAXPACKET, 0, (struct sockaddr *) &from, &fromlen)) < 0) 
#endif
      {

        status = errno;

        // Deal with error conditions first.
        if (status == EAGAIN) {  // EAGAIN = 11
          //printf ("No reply within %d seconds %d useconds.\n", timeout, m_timeout);
          trycount++;
          break;  // Break out of Receive loop.
        } else if (status == EINTR) {  // EINTR = 4
		//printf("=====error EINTR=====\n");
          continue;  // Something weird happened, but let's keep listening.
        }else {
		//printf("====error else=====\n");
          perror ("recvfrom() failed ");
          goto exit_free_mem;
        }
      }  // End of error handling conditionals.
#if USE_RECVMSG_FOR_INTERFACE_INFO
	struct cmsghdr *cmsg ;
	for(cmsg = CMSG_FIRSTHDR(&msg); cmsg != NULL; cmsg = CMSG_NXTHDR(&msg, cmsg))
	{
	    if(cmsg->cmsg_level == SOL_IP && cmsg->cmsg_type == IP_PKTINFO)
	    {
        	struct in_pktinfo *i = (struct in_pktinfo *)CMSG_DATA(cmsg);
        	if(i->ipi_ifindex)
        	{
        	    if_indextoname(i->ipi_ifindex, recv_interface);
		
        	    printf("incoming interface(if: %d, name: %s)\n", i->ipi_ifindex, recv_interface);
        	}
        	break;
    	    }	
	}	
#endif
      // Check for an IP ethernet frame, carrying ICMP echo reply. If not, ignore and keep listening.
      if ((((recv_ether_frame[12] << 8) + recv_ether_frame[13]) == ETH_P_ARP) && htons(recv_arphdr->ar_op) == ARPOP_REPLY) {
#if USE_RECVMSG_FOR_INTERFACE_INFO
        printf("recv %s\n", recv_ether_frame);
#endif	
	sprintf(recv_mac, "%2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X", (unsigned char)recv_ether_frame[0],(unsigned char)recv_ether_frame[1], \
        	(unsigned char)recv_ether_frame[2],(unsigned char)recv_ether_frame[3],(unsigned char)recv_ether_frame[4],(unsigned char)recv_ether_frame[5]);

	sprintf(send_mac, "%2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X", (unsigned char)send_arpdata.senderMac[0],(unsigned char)send_arpdata.senderMac[1], \
		(unsigned char)send_arpdata.senderMac[2],(unsigned char)send_arpdata.senderMac[3],(unsigned char)send_arpdata.senderMac[4],(unsigned char)send_arpdata.senderMac[5]);

	//printf("send_mac=%s, recv_mac=%s\n", send_mac, recv_mac);
	if (inet_ntop (AF_INET, &(recv_arp_data->senderIP), dst_ip, INET_ADDRSTRLEN) == NULL) {
    		status = errno;
    		fprintf (stderr, "inet_ntop() failed.\nError message: %s", strerror (status));
    		goto exit_free_mem;
  	}

	if(strcmp(send_mac, recv_mac)==0 && strcmp(target, dst_ip)==0)
	{
		sprintf(send_mac, "%2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X", (unsigned char)recv_arp_data->senderMac[0],(unsigned char)recv_arp_data->senderMac[1], \
                (unsigned char)recv_arp_data->senderMac[2],(unsigned char)recv_arp_data->senderMac[3],(unsigned char)recv_arp_data->senderMac[4],(unsigned char)recv_arp_data->senderMac[5]);

		printf("received arp reply, %s is at %s\n", target, send_mac );
		done=1;
		break;
	}	

        // Stop timer and calculate how long it took to get a reply.
        (void) gettimeofday (&t2, &tz);
        dt = (double) (t2.tv_sec - t1.tv_sec) * 1000.0 + (double) (t2.tv_usec - t1.tv_usec) / 1000.0;
      }
      else{ //not received arp packet
	(void) gettimeofday (&t2, &tz);
        dt = (double) (t2.tv_sec - t1.tv_sec) * 1000.0 + (double) (t2.tv_usec - t1.tv_usec) / 1000.0;

	if(dt >= (double)((double)(timeout*1000.0)+(double)(m_timeout/1000.0))){
	  trycount=trylim;
	  //printf("ping timeout, dt:%f no response in %d sec %d usec\n", dt, timeout, m_timeout);
          break;	
	}
      }	
    }  // End of Receive loop.

    // The 'done' flag was set because an echo reply was received; break out of send loop.
    if (done == 1) {
      break;  // Break out of Send loop.
    }

    // We ran out of tries, so let's give up.
    if (trycount == trylim) {
      printf ("Recognized no arping replies from remote host after %i tries.\n", trylim);
      break;
    }

  }  // End of Send loop.

exit_free_mem:
  // Close socket descriptors.
  close (sendsd);
  close (recvsd);

  // Free allocated memory.
  free (src_mac);
  free (dst_mac);
  free (data);
  free (send_ether_frame);
  free (recv_ether_frame);
  free (interface);
  free (target);
  free (src_ip);
  free (dst_ip);
  free (rec_ip);
  free (ip_flags);

  return (EXIT_SUCCESS);
}

// Computing the internet checksum (RFC 1071).
// Note that the internet checksum does not preclude collisions.
uint16_t
checksum (uint16_t *addr, int len)
{
  int count = len;
  register uint32_t sum = 0;
  uint16_t answer = 0;

  // Sum up 2-byte values until none or only one byte left.
  while (count > 1) {
    sum += *(addr++);
    count -= 2;
  }

  // Add left-over byte, if any.
  if (count > 0) {
    sum += *(uint8_t *) addr;
  }

  // Fold 32-bit sum into 16 bits; we lose information by doing this,
  // increasing the chances of a collision.
  // sum = (lower 16 bits) + (upper 16 bits shifted right 16 bits)
  while (sum >> 16) {
    sum = (sum & 0xffff) + (sum >> 16);
  }

  // Checksum is one's compliment of sum.
  answer = ~sum;

  return (answer);
}

void sendArpToLanAll(char *interface, char *targetIpPrefix, uint16_t sendIntervalms)
{
        int sendsd, status, frame_length, bytes;
        uint8_t *send_ether_frame, *src_mac, *dst_mac, i;
        char *src_ip, *targetIp;
        struct sockaddr_ll device;
        struct ifreq ifr;
        struct in_addr  s_in, t_in;
        struct arphdr send_arphdr;
        struct arp_data send_arpdata;
        struct   sockaddr_in *sin;

	targetIp = allocate_strmem (INET_ADDRSTRLEN);

        send_ether_frame = allocate_ustrmem (IP_MAXPACKET);
        src_mac = allocate_ustrmem (6);
        dst_mac = allocate_ustrmem (6);
        src_ip = allocate_strmem (INET_ADDRSTRLEN);
	if((sendsd = socket (PF_PACKET, SOCK_RAW, htons (ETH_P_ALL))) < 0)
        {
                perror ("socket() failed to get socket descriptor for using ioctl() ");
                goto exit_free_sock_mem;
        }
        memset (&ifr, 0, sizeof (ifr));
        if(strlen(interface)<=1)
                strcpy(interface, SEND_PACKET_INTERFACE);
        snprintf (ifr.ifr_name, sizeof (ifr.ifr_name), "%s", interface);

        if (ioctl (sendsd, SIOCGIFHWADDR, &ifr) < 0) {
                perror ("ioctl() failed to get source MAC address ");
                goto exit_free_sock_mem;
        }
        memcpy (src_mac, ifr.ifr_hwaddr.sa_data, 6);


        if (ioctl (sendsd, SIOCGIFADDR, &ifr) < 0) {
                perror ("ioctl() failed to get source ip address ");
                goto exit_free_sock_mem;
        }

        sin = (struct sockaddr_in *)&ifr.ifr_addr;
        strcpy(src_ip,inet_ntoa(sin->sin_addr));
        memset (&device, 0, sizeof (device));
        if ((device.sll_ifindex = if_nametoindex (interface)) == 0) {
                perror ("if_nametoindex() failed to obtain interface index ");
                goto exit_free_sock_mem;
        }

        dst_mac[0] = 0xff;
        dst_mac[1] = 0xff;
        dst_mac[2] = 0xff;
        dst_mac[3] = 0xff;
        dst_mac[4] = 0xff;
        dst_mac[5] = 0xff;

        device.sll_family = AF_PACKET;
        memcpy (device.sll_addr, src_mac, 6);
        device.sll_halen = 6;

        send_arphdr.ar_hrd = htons(ARPHRD_ETHER);
        send_arphdr.ar_pro = htons(ETH_P_IP);
        send_arphdr.ar_hln = 6;
        send_arphdr.ar_pln = 4;
        send_arphdr.ar_op = htons(ARPOP_REQUEST);

        if ((status = inet_pton (AF_INET, src_ip, &(s_in.s_addr))) != 1) {
                fprintf (stderr, "inet_pton() failed.\nError message: %s", strerror (status));
                goto exit_free_sock_mem;
        }
        memcpy(send_arpdata.senderMac, device.sll_addr, 6);
        memcpy(send_arpdata.senderIP, &(s_in.s_addr), 4);
        memcpy(send_arpdata.recverMac, dst_mac, 6);
        frame_length = ETH_HDRLEN +  ARP_HDRLEN + ARP_DATALEN;
        memcpy (send_ether_frame, dst_mac, 6);
        memcpy (send_ether_frame + 6, src_mac, 6);

        send_ether_frame[12] = ETH_P_ARP / 256;
        send_ether_frame[13] = ETH_P_ARP % 256;
	memcpy (send_ether_frame + ETH_HDRLEN, &send_arphdr, ARP_HDRLEN);
	for (i=1; i<DEFAULT_SUBNET_IP_NUM; i++)	//1~254
	{
		sprintf(targetIp, "%s%d", targetIpPrefix, i);
	        if ((status = inet_pton (AF_INET, targetIp, &(t_in.s_addr))) != 1) {
	                fprintf (stderr, "inet_pton() failed.\nError message: %s", strerror (status));
	                goto exit_free_sock_mem;
	        }
		memcpy(send_arpdata.recverIP, &(t_in.s_addr), 4);
	
	        memcpy (send_ether_frame + ETH_HDRLEN, &send_arphdr, ARP_HDRLEN);
	        memcpy (send_ether_frame + ETH_HDRLEN + ARP_HDRLEN, &send_arpdata, ARP_DATALEN);
	        //printf("=====send arp packet to %s======\n", targetIp);
	        if((bytes = sendto (sendsd, send_ether_frame, frame_length, 0, (struct sockaddr *) &device, sizeof (device))) <= 0)
	        {
	                perror ("sendto() failed ");
			goto exit_free_sock_mem;
	        }
		usleep(sendIntervalms*1000);
	}
exit_free_sock_mem:
        close (sendsd);
	free (targetIp);
        free (src_ip);
        free (src_mac);
        free (dst_mac);
        free (send_ether_frame);
}
void sendPacketToCheckAlive(char *interface, char *targetIp, char *source)
{
	int sendsd, status, frame_length, bytes;
	uint8_t *send_ether_frame, *src_mac, *dst_mac;
	char *src_ip;
	struct sockaddr_ll device;
	struct ifreq ifr;
	struct in_addr  s_in, t_in;
	struct arphdr send_arphdr;
	struct arp_data send_arpdata;
	struct   sockaddr_in *sin; 

	send_ether_frame = allocate_ustrmem (IP_MAXPACKET);
	src_mac = allocate_ustrmem (6);
	dst_mac = allocate_ustrmem (6);
	src_ip = allocate_strmem (INET_ADDRSTRLEN);

	if(strlen(source)>6)
		memcpy(src_ip, source, INET_ADDRSTRLEN);

        if((sendsd = socket (PF_PACKET, SOCK_RAW, htons (ETH_P_ALL))) < 0)
        {
                perror ("socket() failed to get socket descriptor for using ioctl() ");
                goto exit_free_sock_mem;
        }
	memset (&ifr, 0, sizeof (ifr));
	if(strlen(interface)<=1)
		strcpy(interface, SEND_PACKET_INTERFACE);
	snprintf (ifr.ifr_name, sizeof (ifr.ifr_name), "%s", interface);

	if (ioctl (sendsd, SIOCGIFHWADDR, &ifr) < 0) {
		perror ("ioctl() failed to get source MAC address ");
		goto exit_free_sock_mem;
	}
	memcpy (src_mac, ifr.ifr_hwaddr.sa_data, 6);
	/*sprintf(src_mac,"%02x%02x%02x%02x%02x%02x",  
            (unsigned char)ifr.ifr_hwaddr.sa_data[0],  
            (unsigned char)ifr.ifr_hwaddr.sa_data[1],  
            (unsigned char)ifr.ifr_hwaddr.sa_data[2],  
            (unsigned char)ifr.ifr_hwaddr.sa_data[3],  
            (unsigned char)ifr.ifr_hwaddr.sa_data[4],  
            (unsigned char)ifr.ifr_hwaddr.sa_data[5]); */

	sin = (struct sockaddr_in *)&ifr.ifr_addr;

	if(strlen(src_ip)<=6){
		printf("src_ip illegal, use %s's IP in arp sender ip\n", interface);
	        if (ioctl (sendsd, SIOCGIFADDR, &ifr) < 0) {
	                printf("ERROR, device %s has no IP address\n", interface);
        	        goto exit_free_sock_mem;
        	}else
			strcpy(src_ip,inet_ntoa(sin->sin_addr));
	}

	memset (&device, 0, sizeof (device));
	if ((device.sll_ifindex = if_nametoindex (interface)) == 0) {
		perror ("if_nametoindex() failed to obtain interface index ");
		goto exit_free_sock_mem;
	}
	dst_mac[0] = 0xff;
	dst_mac[1] = 0xff;
	dst_mac[2] = 0xff;
	dst_mac[3] = 0xff;
	dst_mac[4] = 0xff;
	dst_mac[5] = 0xff;
	
	device.sll_family = AF_PACKET;
	memcpy (device.sll_addr, src_mac, 6);
	device.sll_halen = 6;

	send_arphdr.ar_hrd = htons(ARPHRD_ETHER);
	send_arphdr.ar_pro = htons(ETH_P_IP);
	send_arphdr.ar_hln = 6;
	send_arphdr.ar_pln = 4;
	send_arphdr.ar_op = htons(ARPOP_REQUEST);

	if ((status = inet_pton (AF_INET, src_ip, &(s_in.s_addr))) != 1) {
		fprintf (stderr, "inet_pton() failed. src_ip=%s\nError message: %s\n", src_ip, strerror (status));
		goto exit_free_sock_mem;
	}
	if ((status = inet_pton (AF_INET, targetIp, &(t_in.s_addr))) != 1) {
		fprintf (stderr, "inet_pton() targetIP failed. targetIp=%s\nError message: %s\n", targetIp, strerror (status));
		goto exit_free_sock_mem;
	}
	memcpy(send_arpdata.senderMac, device.sll_addr, 6);
	memcpy(send_arpdata.senderIP, &(s_in.s_addr), 4);
	memcpy(send_arpdata.recverMac, dst_mac, 6);
	memcpy(send_arpdata.recverIP, &(t_in.s_addr), 4);
	frame_length = ETH_HDRLEN +  ARP_HDRLEN + ARP_DATALEN;

        memcpy (send_ether_frame, dst_mac, 6);
        memcpy (send_ether_frame + 6, src_mac, 6);

        send_ether_frame[12] = ETH_P_ARP / 256;
        send_ether_frame[13] = ETH_P_ARP % 256;
	memcpy (send_ether_frame + ETH_HDRLEN, &send_arphdr, ARP_HDRLEN);
	memcpy (send_ether_frame + ETH_HDRLEN + ARP_HDRLEN, &send_arpdata, ARP_DATALEN);	
		//printf("=====send arp packet to %s======\n", targetIp);
	if((bytes = sendto (sendsd, send_ether_frame, frame_length, 0, (struct sockaddr *) &device, sizeof (device))) <= 0) 
	{
		perror ("sendto() failed ");
	}

exit_free_sock_mem:
	close (sendsd);
	free (src_ip);
	free (src_mac);
	free (dst_mac);
	free (send_ether_frame);
}



// Allocate memory for an array of chars.
char *
allocate_strmem (int len)
{
  void *tmp;

  if (len <= 0) {
    fprintf (stderr, "ERROR: Cannot allocate memory because len = %i in allocate_strmem().\n", len);
    exit (EXIT_FAILURE);
  }

  tmp = (char *) malloc (len * sizeof (char));
  if (tmp != NULL) {
    memset (tmp, 0, len * sizeof (char));
    return (tmp);
  } else {
    fprintf (stderr, "ERROR: Cannot allocate memory for array allocate_strmem().\n");
    exit (EXIT_FAILURE);
  }
}

// Allocate memory for an array of unsigned chars.
uint8_t *
allocate_ustrmem (int len)
{
  void *tmp;

  if (len <= 0) {
    fprintf (stderr, "ERROR: Cannot allocate memory because len = %i in allocate_ustrmem().\n", len);
    exit (EXIT_FAILURE);
  }

  tmp = (uint8_t *) malloc (len * sizeof (uint8_t));
  if (tmp != NULL) {
    memset (tmp, 0, len * sizeof (uint8_t));
    return (tmp);
  } else {
    fprintf (stderr, "ERROR: Cannot allocate memory for array allocate_ustrmem().\n");
    exit (EXIT_FAILURE);
  }
}

// Allocate memory for an array of ints.
int *
allocate_intmem (int len)
{
  void *tmp;

  if (len <= 0) {
    fprintf (stderr, "ERROR: Cannot allocate memory because len = %i in allocate_intmem().\n", len);
    exit (EXIT_FAILURE);
  }

  tmp = (int *) malloc (len * sizeof (int));
  if (tmp != NULL) {
    memset (tmp, 0, len * sizeof (int));
    return (tmp);
  } else {
    fprintf (stderr, "ERROR: Cannot allocate memory for array allocate_intmem().\n");
    exit (EXIT_FAILURE);
  }
}


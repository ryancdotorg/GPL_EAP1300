/*
 * Copyright (C) 2007-2016  B.A.T.M.A.N. contributors:
 *
 * Marek Lindner <mareklindner@neomailbox.ch>, Simon Wunderlich
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA
 *
 */

#include "icmp_helper.h"
#include "main.h"

#include <errno.h>
#include <linux/filter.h>
#include <linux/if_packet.h>
#include <linux/rtnetlink.h>
#include <net/ethernet.h>
#include <netinet/ether.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <unistd.h>

#include "debug.h"
#include "debugfs.h"
#include "functions.h"
#include "list.h"
#include "netlink.h"
#include "packet.h"

#ifndef ETH_P_BATMAN
#define ETH_P_BATMAN	0x4305
#endif /* ETH_P_BATMAN */

static LIST_HEAD(interface_list);
static size_t direct_reply_len;
static uint8_t uid;
static uint8_t primary_mac[ETH_ALEN];
static uint8_t icmp_buffer[BATADV_ICMP_MAX_PACKET_SIZE];

#define BATADV_ICMP_MIN_PACKET_SIZE sizeof(struct batadv_icmp_packet)

#define BADADV_ICMP_ETH_OFFSET(member) \
	(ETH_HLEN + offsetof(struct batadv_icmp_packet, member))

static struct icmp_interface *icmp_interface_find(const char *ifname)
{
	struct icmp_interface *found = NULL;
	struct icmp_interface *iface;

	list_for_each_entry(iface, &interface_list, list) {
		if (strcmp(iface->name, ifname) == 0) {
			found = iface;
			break;
		}
	}

	return found;
}

static bool icmp_interfaces_is_my_mac(uint8_t dst[ETH_ALEN])
{
	struct icmp_interface *iface;

	list_for_each_entry(iface, &interface_list, list) {
		if (memcmp(iface->mac, dst, ETH_ALEN) == 0)
			return true;
	}

	return false;
}

void icmp_interface_destroy(struct icmp_interface *iface)
{
	close(iface->sock);
	list_del(&iface->list);
	free(iface);
}

static int icmp_interface_filter(int sock, int uid)
{
	struct sock_fprog filter;
	struct sock_filter accept_icmp[] = {
		/* load ethernet proto */
		BPF_STMT(BPF_LD + BPF_H + BPF_ABS,
			 offsetof(struct ether_header, ether_type)),
		/* jump to ret 0 when it is != 0x4305 */
		BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K,
			 ETH_P_BATMAN,
			 0, 14),
		/* load pktlen */
		BPF_STMT(BPF_LD + BPF_W + BPF_LEN,
			 0),
		/* jump to ret 0 when it is < 34 */
		BPF_JUMP(BPF_JMP + BPF_JGE + BPF_K,
			 ETH_HLEN + BATADV_ICMP_MIN_PACKET_SIZE,
			 0, 12),
		/* jump to ret 0 when it is > 130 */
		BPF_JUMP(BPF_JMP + BPF_JGT + BPF_K,
			 ETH_HLEN + BATADV_ICMP_MAX_PACKET_SIZE,
			 11, 0),
		/* load batman-adv type */
		BPF_STMT(BPF_LD + BPF_B + BPF_ABS,
			 BADADV_ICMP_ETH_OFFSET(packet_type)),
		/* jump to ret 0 when it is != BATADV_ICMP */
		BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K,
			 BATADV_ICMP,
			 0, 9),
		/* load batman-adv version */
		BPF_STMT(BPF_LD + BPF_B + BPF_ABS,
			 BADADV_ICMP_ETH_OFFSET(version)),
		/* jump to ret 0 when it is != 15 */
		BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K,
			 BATADV_COMPAT_VERSION,
			 0, 7),
		/* load batman-adv icmp msg_type */
		BPF_STMT(BPF_LD + BPF_B + BPF_ABS,
			 BADADV_ICMP_ETH_OFFSET(msg_type)),
		/* accept BATADV_ECHO_REPLY or go to next check */
		BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K,
			 BATADV_ECHO_REPLY,
			 2, 0),
		/* accept BATADV_DESTINATION_UNREACHABLE or go to next check */
		BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K,
			 BATADV_DESTINATION_UNREACHABLE,
			 1, 0),
		/* accept BATADV_TTL_EXCEEDED or go to ret 0 */
		BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K,
			 BATADV_TTL_EXCEEDED,
			 0, 3),
		/* load batman-adv icmp uid */
		BPF_STMT(BPF_LD + BPF_B + BPF_ABS,
			 BADADV_ICMP_ETH_OFFSET(uid)),
		/* jump to ret 0 when it is not our uid */
		BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K,
			 uid,
			 0, 1),
		/* accept 130 bytes */
		BPF_STMT(BPF_RET + BPF_K,
			 ETH_HLEN + BATADV_ICMP_MAX_PACKET_SIZE),
		/* ret 0 -> reject packet */
		BPF_STMT(BPF_RET + BPF_K,
			 0),
	};

	memset(&filter, 0, sizeof(filter));
	filter.len = sizeof(accept_icmp) / sizeof(*accept_icmp);
	filter.filter = accept_icmp;

	if (setsockopt(sock, SOL_SOCKET, SO_ATTACH_FILTER, &filter,
		       sizeof(filter)))
		return -1;

	return 0;
}

static int icmp_interface_add(const char *ifname, const uint8_t mac[ETH_ALEN])
{
	struct icmp_interface *iface;
	struct sockaddr_ll sll;
	struct ifreq req;
	int ret;

	iface = malloc(sizeof(*iface));
	if (!iface)
		return -ENOMEM;

	iface->mark = 1;
	memcpy(iface->mac, mac, ETH_ALEN);

	strncpy(iface->name, ifname, IFNAMSIZ);
	iface->name[sizeof(iface->name) - 1] = '\0';

	iface->sock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (iface->sock < 0) {
		fprintf(stderr, "Error - can't create raw socket: %s\n", strerror(errno));
		ret = -errno;
		goto free_iface;
	}

	memset(&req, 0, sizeof(struct ifreq));
	strncpy(req.ifr_name, ifname, IFNAMSIZ);
	req.ifr_name[sizeof(req.ifr_name) - 1] = '\0';

	ret = ioctl(iface->sock, SIOCGIFINDEX, &req);
	if (ret < 0) {
		fprintf(stderr, "Error - can't create raw socket (SIOCGIFINDEX): %s\n", strerror(errno));
		ret = -errno;
		goto close_sock;
	}

	memset(&sll, 0, sizeof(sll));
	sll.sll_family = AF_PACKET;
	sll.sll_protocol = htons(ETH_P_ALL);
	sll.sll_pkttype = PACKET_HOST;
	sll.sll_ifindex = req.ifr_ifindex;

	ret = bind(iface->sock, (struct sockaddr *)&sll, sizeof(struct sockaddr_ll));
	if (ret < 0) {
		fprintf(stderr, "Error - can't bind raw socket: %s\n", strerror(errno));
		ret = -errno;
		goto close_sock;
	}

	ret = icmp_interface_filter(iface->sock, uid);
	if (ret < 0) {
		fprintf(stderr, "Error - can't add filter to raw socket: %s\n", strerror(-ret));
		goto close_sock;
	}

	list_add(&iface->list, &interface_list);

	return 0;

close_sock:
	close(iface->sock);
free_iface:
	free(iface);

	return ret;
}

int icmp_interfaces_init(void)
{
	get_random_bytes(&uid, 1);

	return 0;
}

static struct nla_policy link_policy[IFLA_MAX + 1] = {
	[IFLA_IFNAME] = { .type = NLA_STRING, .maxlen = IFNAMSIZ },
	[IFLA_MASTER] = { .type = NLA_U32 },
	[IFLA_ADDRESS] = { .type = NLA_UNSPEC,
			   .minlen = ETH_ALEN,
			   .maxlen = ETH_ALEN,
	},
};

struct icmp_interface_update_arg {
	int ifindex;
};

static int icmp_interface_update_parse(struct nl_msg *msg, void *arg)
{
	struct icmp_interface_update_arg *update_arg = arg;
	struct nlattr *attrs[IFLA_MAX + 1];
	struct icmp_interface *iface;
	struct ifinfomsg *ifm;
	char *ifname;
	int ret;
	int master;
	uint8_t *mac;

	ifm = nlmsg_data(nlmsg_hdr(msg));
	ret = nlmsg_parse(nlmsg_hdr(msg), sizeof(*ifm), attrs, IFLA_MAX,
			  link_policy);
	if (ret < 0)
		goto err;

	if (!attrs[IFLA_IFNAME])
		goto err;

	if (!attrs[IFLA_MASTER])
		goto err;

	if (!attrs[IFLA_ADDRESS])
		goto err;

	ifname = nla_get_string(attrs[IFLA_IFNAME]);
	master = nla_get_u32(attrs[IFLA_MASTER]);
	mac = nla_data(attrs[IFLA_ADDRESS]);

	/* required on older kernels which don't prefilter the results */
	if (master != update_arg->ifindex)
		goto err;

	/* update or add interface */
	iface = icmp_interface_find(ifname);
	if (!iface) {
		icmp_interface_add(ifname, mac);
		goto err;
	}

	/* update */
	iface->mark = 1;
	memcpy(iface->mac, mac, ETH_ALEN);

err:
	return NL_OK;
}

static int get_nexthop_debugfs(const char *mesh_iface,
			       struct ether_addr *mac,
			       uint8_t nexthop[ETH_ALEN],
			       char ifname[IF_NAMESIZE])
{
	char *tptr;
	char *temp1, *temp2;
	char *dest, *neigh, *iface;
	int lnum, tnum;
	struct ether_addr *mac_tmp;
	char path[1024];
	FILE *f = NULL;
	size_t len = 0;
	char *line = NULL;
	char *primary_info;

	debugfs_make_path(DEBUG_BATIF_PATH_FMT "/" "originators", mesh_iface, path, sizeof(path));

	f = fopen(path, "r");
	if (!f)
		return -EOPNOTSUPP;

	lnum = 0;
	while (getline(&line, &len, f) != -1) {
		lnum++;

		/* find primary mac */
		if (lnum == 1) {
			primary_info = strstr(line, "MainIF/MAC: ");
			if (!primary_info)
				continue;

			primary_info += 12;
			temp1 = strstr(primary_info, "/");
			if (!temp1)
				continue;

			temp1++;
			temp2 = strstr(line, " ");
			if (!temp2)
				continue;

			temp2[0] = '\0';
			mac_tmp = ether_aton(temp1);
			if (!mac_tmp)
				continue;

			memcpy(primary_mac, mac_tmp, ETH_ALEN);
		}

		if (lnum < 3)
			continue;

		/* find correct neighbor */
		for (tptr = line, tnum = 0;; tptr = NULL, tnum++) {
			tptr = strtok_r(tptr, "\t []()", &temp2);
			if (!tptr)
				break;
			switch (tnum) {
			case 0: dest = tptr; break;
			case 2: break;
			case 3: neigh = tptr; break;
			case 4: iface = tptr; break;
			default: break;
			}
		}
		if (tnum <= 4)
			continue;

		mac_tmp = ether_aton(dest);
		if (!mac_tmp || memcmp(mac_tmp, mac, ETH_ALEN) != 0)
			continue;

		mac_tmp = ether_aton(neigh);
		if (!mac_tmp)
			continue;

		memcpy(nexthop, mac_tmp, ETH_ALEN);
		strncpy(ifname, iface, IF_NAMESIZE);
		ifname[IF_NAMESIZE - 1] = '\0';
		break;
	}
	free(line);
	fclose(f);

	return 0;
}

static void icmp_interface_unmark(void)
{
	struct icmp_interface *iface;

	list_for_each_entry(iface, &interface_list, list)
		iface->mark = 0;
}


static void icmp_interface_sweep(void)
{
	struct icmp_interface *iface, *safe;

	list_for_each_entry_safe(iface, safe, &interface_list, list) {
		if (iface->mark)
			continue;

		icmp_interface_destroy(iface);
	}
}

static int icmp_interface_update(const char *mesh_iface)
{
	struct icmp_interface_update_arg update_arg;

	update_arg.ifindex = if_nametoindex(mesh_iface);
	if (!update_arg.ifindex)
		return -errno;

	/* unmark current interface - will be marked again by query */
	icmp_interface_unmark();

	query_rtnl_link(update_arg.ifindex, icmp_interface_update_parse,
			&update_arg);

	/* remove old interfaces */
	icmp_interface_sweep();

	get_primarymac_netlink(mesh_iface, primary_mac);

	return 0;
}

static int icmp_interface_send(struct batadv_icmp_header *icmp_packet,
			       size_t packet_len, struct icmp_interface *iface,
			       uint8_t nexthop[ETH_ALEN])
{
	struct ether_header header;
	struct iovec vector[2];

	header.ether_type = htons(ETH_P_BATMAN);
	memcpy(header.ether_shost, iface->mac, ETH_ALEN);
	memcpy(header.ether_dhost, nexthop, ETH_ALEN);

	vector[0].iov_base = &header;
	vector[0].iov_len  = sizeof(struct ether_header);
	vector[1].iov_base = icmp_packet;
	vector[1].iov_len  = packet_len;

	return (int)writev(iface->sock, vector, 2);
}

int icmp_interface_write(const char *mesh_iface,
			 struct batadv_icmp_header *icmp_packet, size_t len)
{
	struct batadv_icmp_packet_rr *icmp_packet_rr;
	struct icmp_interface *iface;
	uint8_t nexthop[ETH_ALEN];
	char ifname[IF_NAMESIZE];
	struct ether_addr mac;
	size_t packet_len;
	int ret;

	if (len < sizeof(*icmp_packet))
		return -EINVAL;

	if (len >= BATADV_ICMP_MAX_PACKET_SIZE)
		packet_len = BATADV_ICMP_MAX_PACKET_SIZE;
	else
		packet_len = len;

	if (icmp_packet->packet_type != BATADV_ICMP)
		return -EINVAL;

	if (icmp_packet->msg_type != BATADV_ECHO_REQUEST)
		return -EINVAL;

	icmp_interface_update(mesh_iface);

	if (list_empty(&interface_list))
		return -EFAULT;

	/* find best neighbor */
	memcpy(&mac, icmp_packet->dst, ETH_ALEN);

	ret = get_nexthop_netlink(mesh_iface, &mac, nexthop, ifname);
	if (ret == -EOPNOTSUPP)
		ret = get_nexthop_debugfs(mesh_iface, &mac, nexthop, ifname);
	if (ret < 0)
		goto dst_unreachable;

	iface = icmp_interface_find(ifname);
	if (!iface)
		goto dst_unreachable;

	direct_reply_len = 0;

	icmp_packet->uid = uid;
	memcpy(icmp_packet->orig, primary_mac, ETH_ALEN);

	/* start RR packet */
	icmp_packet_rr = (struct batadv_icmp_packet_rr *)icmp_packet;
	if (packet_len == sizeof(*icmp_packet_rr))
		memcpy(icmp_packet_rr->rr[0], iface->mac, ETH_ALEN);

	return icmp_interface_send(icmp_packet, packet_len, iface, nexthop);

dst_unreachable:
	memcpy(icmp_buffer, icmp_packet, packet_len);
	icmp_packet = (struct batadv_icmp_header *)icmp_buffer;
	icmp_packet->msg_type = BATADV_DESTINATION_UNREACHABLE;
	direct_reply_len = packet_len;
	return 0;
}

static int icmp_interface_preselect(fd_set *read_sockets)
{
	struct icmp_interface *iface;
	int max = 0;

	FD_ZERO(read_sockets);

	list_for_each_entry(iface, &interface_list, list) {
		FD_SET(iface->sock, read_sockets);
		if (max <= iface->sock)
			max = iface->sock + 1;
	}

	return max;
}

static ssize_t icmp_interface_get_read_sock(fd_set *read_sockets,
					    struct icmp_interface **piface)
{
	struct icmp_interface *iface;
	int sock = -1;

	list_for_each_entry(iface, &interface_list, list) {
		if (!FD_ISSET(iface->sock, read_sockets))
			continue;

		sock = iface->sock;
		*piface = iface;
		break;
	}

	return sock;
}

ssize_t icmp_interface_read(struct batadv_icmp_header *icmp_packet, size_t len,
			    struct timeval *tv)
{
	struct batadv_icmp_packet_rr *icmp_packet_rr;
	struct icmp_interface *iface;
	struct ether_header header;
	struct iovec vector[2];
	fd_set read_sockets;
	size_t packet_len;
	int max_sock;
	ssize_t read_len;
	int read_sock;
	int res;

	if (len < sizeof(*icmp_packet))
		return -EINVAL;

	if (len >= BATADV_ICMP_MAX_PACKET_SIZE)
		packet_len = BATADV_ICMP_MAX_PACKET_SIZE;
	else
		packet_len = len;

	if (direct_reply_len > 0) {
		memcpy(icmp_packet, icmp_buffer, packet_len);
		direct_reply_len = 0;
		return (ssize_t)packet_len;
	}

retry:
	max_sock = icmp_interface_preselect(&read_sockets);

	res = select(max_sock, &read_sockets, NULL, NULL, tv);
	/* timeout, or < 0 error */
	if (res <= 0)
		return res;

	read_sock = icmp_interface_get_read_sock(&read_sockets, &iface);
	if (read_sock < 0)
		return read_sock;

	vector[0].iov_base = &header;
	vector[0].iov_len  = sizeof(struct ether_header);
	vector[1].iov_base = icmp_packet;
	vector[1].iov_len  = packet_len;

	read_len = readv(read_sock, vector, 2);
	if (read_len < 0)
		return read_len;

	if (read_len < ETH_HLEN)
		goto retry;

	read_len -= ETH_HLEN;

	if (read_len < (ssize_t)sizeof(*icmp_packet))
		goto retry;

	if (!icmp_interfaces_is_my_mac(icmp_packet->dst))
		goto retry;

	/* end RR packet */
	icmp_packet_rr = (struct batadv_icmp_packet_rr *)icmp_packet;
	if (read_len == sizeof(*icmp_packet_rr) &&
	    icmp_packet_rr->rr_cur < BATADV_RR_LEN) {
		memcpy(icmp_packet_rr->rr[icmp_packet_rr->rr_cur], iface->mac,
		       ETH_ALEN);
		icmp_packet_rr->rr_cur++;
	}

	return read_len;
}

void icmp_interfaces_clean(void)
{
	struct icmp_interface *iface, *safe;

	list_for_each_entry_safe(iface, safe, &interface_list, list)
		icmp_interface_destroy(iface);
}

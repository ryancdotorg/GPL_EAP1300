// SPDX-License-Identifier: GPL-2.0
/* Copyright (C) 2009-2019  B.A.T.M.A.N. contributors:
 *
 * Marek Lindner <mareklindner@neomailbox.ch>
 *
 * License-Filename: LICENSES/preferred/GPL-2.0
 */

#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <net/if.h>
#include <linux/if_link.h>
#include <linux/rtnetlink.h>
#include <netlink/netlink.h>
#include <netlink/msg.h>
#include <netlink/attr.h>

#include "main.h"
#include "sys.h"
#include "functions.h"

#define IFACE_STATUS_LEN 256

static void interface_usage(void)
{
	fprintf(stderr, "Usage: batctl [options] interface [parameters] [add|del iface(s)]\n");
	fprintf(stderr, "       batctl [options] interface [parameters] [create|destroy]\n");
	fprintf(stderr, "parameters:\n");
	fprintf(stderr, " \t -M disable automatic creation/removal of batman-adv interface\n");
	fprintf(stderr, " \t -h print this help\n");
}

static int get_iface_status_netlink_parse(struct nl_msg *msg, void *arg)
{

	struct nlattr *attrs[NUM_BATADV_ATTR];
	struct nlmsghdr *nlh = nlmsg_hdr(msg);
	char *iface_status = arg;
	struct genlmsghdr *ghdr;

	if (!genlmsg_valid_hdr(nlh, 0))
		return NL_OK;

	ghdr = nlmsg_data(nlh);
	if (ghdr->cmd != BATADV_CMD_GET_HARDIF)
		return NL_OK;

	if (nla_parse(attrs, BATADV_ATTR_MAX, genlmsg_attrdata(ghdr, 0),
		      genlmsg_len(ghdr), batadv_netlink_policy))
		return NL_OK;

	if (attrs[BATADV_ATTR_ACTIVE])
		strncpy(iface_status, "active\n", IFACE_STATUS_LEN);
	else
		strncpy(iface_status, "inactive\n", IFACE_STATUS_LEN);

	iface_status[IFACE_STATUS_LEN - 1] = '\0';

	return NL_STOP;
}

static char *get_iface_status_netlink(unsigned int meshif, unsigned int hardif,
				      char *iface_status)
{
	char *ret_status = NULL;
	struct nl_sock *sock;
	struct nl_msg *msg;
	int batadv_family;
	struct nl_cb *cb;
	int ret;

	iface_status[0] = '\0';

	sock = nl_socket_alloc();
	if (!sock)
		return NULL;

	ret = genl_connect(sock);
	if (ret < 0)
		goto err_free_sock;

	batadv_family = genl_ctrl_resolve(sock, BATADV_NL_NAME);
	if (batadv_family < 0)
		goto err_free_sock;

	cb = nl_cb_alloc(NL_CB_DEFAULT);
	if (!cb)
		goto err_free_sock;

	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, get_iface_status_netlink_parse,
		iface_status);

	msg = nlmsg_alloc();
	if (!msg)
		goto err_free_cb;

	genlmsg_put(msg, NL_AUTO_PID, NL_AUTO_SEQ, batadv_family,
		    0, 0, BATADV_CMD_GET_HARDIF, 1);

	nla_put_u32(msg, BATADV_ATTR_MESH_IFINDEX, meshif);
	nla_put_u32(msg, BATADV_ATTR_HARD_IFINDEX, hardif);

	ret = nl_send_auto_complete(sock, msg);
	if (ret < 0)
		goto err_free_msg;

	nl_recvmsgs(sock, cb);

	if (strlen(iface_status) > 0)
		ret_status = iface_status;

err_free_msg:
	nlmsg_free(msg);
err_free_cb:
	nl_cb_put(cb);
err_free_sock:
	nl_socket_free(sock);

	return ret_status;
}

static struct nla_policy link_policy[IFLA_MAX + 1] = {
	[IFLA_IFNAME] = { .type = NLA_STRING, .maxlen = IFNAMSIZ },
	[IFLA_MASTER] = { .type = NLA_U32 },
};

struct print_interfaces_rtnl_arg {
	int ifindex;
};

static int print_interfaces_rtnl_parse(struct nl_msg *msg, void *arg)
{
	struct print_interfaces_rtnl_arg *print_arg = arg;
	char iface_status[IFACE_STATUS_LEN];
	struct nlattr *attrs[IFLA_MAX + 1];
	char path_buff[PATH_BUFF_LEN];
	struct ifinfomsg *ifm;
	char *ifname;
	int ret;
	const char *status;
	int master;

	ifm = nlmsg_data(nlmsg_hdr(msg));
	ret = nlmsg_parse(nlmsg_hdr(msg), sizeof(*ifm), attrs, IFLA_MAX,
			  link_policy);
	if (ret < 0)
		goto err;

	if (!attrs[IFLA_IFNAME])
		goto err;

	if (!attrs[IFLA_MASTER])
		goto err;

	ifname = nla_get_string(attrs[IFLA_IFNAME]);
	master = nla_get_u32(attrs[IFLA_MASTER]);

	/* required on older kernels which don't prefilter the results */
	if (master != print_arg->ifindex)
		goto err;

	status = get_iface_status_netlink(master, ifm->ifi_index, iface_status);
	if (!status) {
		snprintf(path_buff, sizeof(path_buff), SYS_IFACE_STATUS_FMT,
			 ifname);
		ret = read_file("", path_buff, USE_READ_BUFF | SILENCE_ERRORS,
				0, 0, 0);
		if (ret != EXIT_SUCCESS)
			status = "<error reading status>\n";
		else
			status = line_ptr;
	}

	printf("%s: %s", ifname, status);

	free(line_ptr);
	line_ptr = NULL;

err:
	return NL_OK;
}

static int print_interfaces(char *mesh_iface)
{
	struct print_interfaces_rtnl_arg print_arg;

	if (!file_exists(module_ver_path)) {
		fprintf(stderr, "Error - batman-adv module has not been loaded\n");
		return EXIT_FAILURE;
	}

	print_arg.ifindex = if_nametoindex(mesh_iface);
	if (!print_arg.ifindex)
		return EXIT_FAILURE;

	query_rtnl_link(print_arg.ifindex, print_interfaces_rtnl_parse,
			&print_arg);

	return EXIT_SUCCESS;
}

struct count_interfaces_rtnl_arg {
	int ifindex;
	unsigned int count;
};

static int count_interfaces_rtnl_parse(struct nl_msg *msg, void *arg)
{
	struct count_interfaces_rtnl_arg *count_arg = arg;
	struct nlattr *attrs[IFLA_MAX + 1];
	struct ifinfomsg *ifm;
	int ret;
	int master;

	ifm = nlmsg_data(nlmsg_hdr(msg));
	ret = nlmsg_parse(nlmsg_hdr(msg), sizeof(*ifm), attrs, IFLA_MAX,
			  link_policy);
	if (ret < 0)
		goto err;

	if (!attrs[IFLA_IFNAME])
		goto err;

	if (!attrs[IFLA_MASTER])
		goto err;

	master = nla_get_u32(attrs[IFLA_MASTER]);

	/* required on older kernels which don't prefilter the results */
	if (master != count_arg->ifindex)
		goto err;

	count_arg->count++;

err:
	return NL_OK;
}

static unsigned int count_interfaces(char *mesh_iface)
{
	struct count_interfaces_rtnl_arg count_arg;

	count_arg.count = 0;
	count_arg.ifindex = if_nametoindex(mesh_iface);
	if (!count_arg.ifindex)
		return 0;

	query_rtnl_link(count_arg.ifindex, count_interfaces_rtnl_parse,
			&count_arg);

	return count_arg.count;
}

static int create_interface(const char *mesh_iface)
{
	struct ifinfomsg rt_hdr = {
		.ifi_family = IFLA_UNSPEC,
	};
	struct nlattr *linkinfo;
	struct nl_msg *msg;
	int err = 0;
	int ret;

	msg = nlmsg_alloc_simple(RTM_NEWLINK,
				 NLM_F_REQUEST | NLM_F_CREATE | NLM_F_EXCL | NLM_F_ACK);
	if (!msg) {
		return -ENOMEM;
	}

	ret = nlmsg_append(msg, &rt_hdr, sizeof(rt_hdr), NLMSG_ALIGNTO);
	if (ret < 0) {
		err = -ENOMEM;
		goto err_free_msg;
	}

	ret = nla_put_string(msg, IFLA_IFNAME, mesh_iface);
	if (ret < 0) {
		err = -ENOMEM;
		goto err_free_msg;
	}

	linkinfo = nla_nest_start(msg, IFLA_LINKINFO);
	if (!linkinfo) {
		err = -ENOMEM;
		goto err_free_msg;
	}

	ret = nla_put_string(msg, IFLA_INFO_KIND, "batadv");
	if (ret < 0) {
		err = -ENOMEM;
		goto err_free_msg;
	}

	nla_nest_end(msg, linkinfo);

	err = netlink_simple_request(msg);

err_free_msg:
	nlmsg_free(msg);

	return err;
}

static int destroy_interface(const char *mesh_iface)
{
	struct ifinfomsg rt_hdr = {
		.ifi_family = IFLA_UNSPEC,
	};
	struct nl_msg *msg;
	int err = 0;
	int ret;

	msg = nlmsg_alloc_simple(RTM_DELLINK, NLM_F_REQUEST | NLM_F_ACK);
	if (!msg) {
		return -ENOMEM;
	}

	ret = nlmsg_append(msg, &rt_hdr, sizeof(rt_hdr), NLMSG_ALIGNTO);
	if (ret < 0) {
		err = -ENOMEM;
		goto err_free_msg;
	}

	ret = nla_put_string(msg, IFLA_IFNAME, mesh_iface);
	if (ret < 0) {
		err = -ENOMEM;
		goto err_free_msg;
	}

	err = netlink_simple_request(msg);

err_free_msg:
	nlmsg_free(msg);

	return err;
}

static int set_master_interface(const char *iface, unsigned int ifmaster)
{
	struct ifinfomsg rt_hdr = {
		.ifi_family = IFLA_UNSPEC,
	};
	struct nl_msg *msg;
	int err = 0;
	int ret;

	msg = nlmsg_alloc_simple(RTM_SETLINK, NLM_F_REQUEST | NLM_F_ACK);
	if (!msg) {
		return -ENOMEM;
	}

	ret = nlmsg_append(msg, &rt_hdr, sizeof(rt_hdr), NLMSG_ALIGNTO);
	if (ret < 0) {
		err = -ENOMEM;
		goto err_free_msg;
	}

	ret = nla_put_string(msg, IFLA_IFNAME, iface);
	if (ret < 0) {
		err = -ENOMEM;
		goto err_free_msg;
	}

	ret = nla_put_u32(msg, IFLA_MASTER, ifmaster);
	if (ret < 0) {
		err = -ENOMEM;
		goto err_free_msg;
	}

	err = netlink_simple_request(msg);

err_free_msg:
	nlmsg_free(msg);

	return err;
}

static int interface(struct state *state, int argc, char **argv)
{
	int i, optchar;
	int ret;
	unsigned int ifindex;
	unsigned int ifmaster;
	const char *long_op;
	unsigned int cnt;
	int rest_argc;
	char **rest_argv;
	bool manual_mode = false;

	while ((optchar = getopt(argc, argv, "hM")) != -1) {
		switch (optchar) {
		case 'h':
			interface_usage();
			return EXIT_SUCCESS;
		case 'M':
			manual_mode = true;
			break;
		default:
			interface_usage();
			return EXIT_FAILURE;
		}
	}

	rest_argc = argc - optind;
	rest_argv = &argv[optind];

	if (rest_argc == 0)
		return print_interfaces(state->mesh_iface);

	check_root_or_die("batctl interface");

	if ((strcmp(rest_argv[0], "add") != 0) && (strcmp(rest_argv[0], "a") != 0) &&
	    (strcmp(rest_argv[0], "del") != 0) && (strcmp(rest_argv[0], "d") != 0) &&
	    (strcmp(rest_argv[0], "create") != 0) && (strcmp(rest_argv[0], "c") != 0) &&
	    (strcmp(rest_argv[0], "destroy") != 0) && (strcmp(rest_argv[0], "D") != 0)) {
		fprintf(stderr, "Error - unknown argument specified: %s\n", rest_argv[0]);
		interface_usage();
		goto err;
	}

	if (strcmp(rest_argv[0], "destroy") == 0)
		rest_argv[0][0] = 'D';

	switch (rest_argv[0][0]) {
	case 'a':
	case 'd':
		if (rest_argc == 1) {
			fprintf(stderr,
				"Error - missing interface name(s) after '%s'\n",
				rest_argv[0]);
			interface_usage();
			goto err;
		}
		break;
	case 'c':
	case 'D':
		if (rest_argc != 1) {
			fprintf(stderr,
				"Error - extra parameter after '%s'\n",
				rest_argv[0]);
			interface_usage();
			goto err;
		}
		break;
	default:
		break;
	}

	switch (rest_argv[0][0]) {
	case 'c':
		ret = create_interface(state->mesh_iface);
		if (ret < 0) {
			fprintf(stderr,
				"Error - failed to add create batman-adv interface: %s\n",
				strerror(-ret));
			goto err;
		}
		return EXIT_SUCCESS;
	case 'D':
		ret = destroy_interface(state->mesh_iface);
		if (ret < 0) {
			fprintf(stderr,
				"Error - failed to destroy batman-adv interface: %s\n",
				strerror(-ret));
			goto err;
		}
		return EXIT_SUCCESS;
	default:
		break;
	}

	/* get index of batman-adv interface - or try to create it */
	ifmaster = if_nametoindex(state->mesh_iface);
	if (!manual_mode && !ifmaster && rest_argv[0][0] == 'a') {
		ret = create_interface(state->mesh_iface);
		if (ret < 0) {
			fprintf(stderr,
				"Error - failed to create batman-adv interface: %s\n",
				strerror(-ret));
			goto err;
		}

		ifmaster = if_nametoindex(state->mesh_iface);
	}

	if (!ifmaster) {
		ret = -ENODEV;
		fprintf(stderr,
			"Error - failed to find batman-adv interface: %s\n",
			strerror(-ret));
		goto err;
	}

	/* make sure that batman-adv is loaded or was loaded by create_interface */
	if (!file_exists(module_ver_path)) {
		fprintf(stderr, "Error - batman-adv module has not been loaded\n");
		goto err;
	}

	for (i = 1; i < rest_argc; i++) {
		ifindex = if_nametoindex(rest_argv[i]);

		if (!ifindex) {
			fprintf(stderr, "Error - interface does not exist: %s\n", rest_argv[i]);
			continue;
		}

		if (rest_argv[0][0] == 'a')
			ifindex = ifmaster;
		else
			ifindex = 0;

		ret = set_master_interface(rest_argv[i], ifindex);
		if (ret < 0) {
			if (rest_argv[0][0] == 'a')
				long_op = "add";
			else
				long_op = "delete";

			fprintf(stderr, "Error - failed to %s interface %s: %s\n",
				long_op, rest_argv[i], strerror(-ret));
			goto err;
		}
	}

	/* check if there is no interface left and then destroy mesh_iface */
	if (!manual_mode && rest_argv[0][0] == 'd') {
		cnt = count_interfaces(state->mesh_iface);
		if (cnt == 0)
			destroy_interface(state->mesh_iface);
	}

	return EXIT_SUCCESS;

err:
	return EXIT_FAILURE;
}

COMMAND(SUBCOMMAND_MIF, interface, "if", 0, NULL,
	"[add|del iface(s)]\tdisplay or modify the interface settings");

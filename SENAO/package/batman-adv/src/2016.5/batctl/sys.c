/*
 * Copyright (C) 2009-2016  B.A.T.M.A.N. contributors:
 *
 * Marek Lindner <mareklindner@neomailbox.ch>
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


#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <net/if.h>
#include <linux/if_link.h>
#include <netlink/netlink.h>
#include <netlink/msg.h>
#include <netlink/attr.h>

#include "main.h"
#include "sys.h"
#include "functions.h"
#include "debug.h"

const char *sysfs_param_enable[] = {
	"enable",
	"disable",
	"1",
	"0",
	NULL,
};

const char *sysfs_param_server[] = {
	"off",
	"client",
	"server",
	NULL,
};

const struct settings_data batctl_settings[BATCTL_SETTINGS_NUM] = {
	{
		.opt_long = "orig_interval",
		.opt_short = "it",
		.sysfs_name = "orig_interval",
		.params = NULL,
	},
	{
		.opt_long = "ap_isolation",
		.opt_short = "ap",
		.sysfs_name = "ap_isolation",
		.params = sysfs_param_enable,
	},
	{
		.opt_long = "bridge_loop_avoidance",
		.opt_short = "bl",
		.sysfs_name = "bridge_loop_avoidance",
		.params = sysfs_param_enable,
	},
	{
		.opt_long = "distributed_arp_table",
		.opt_short = "dat",
		.sysfs_name = "distributed_arp_table",
		.params = sysfs_param_enable,
	},
	{
		.opt_long = "aggregation",
		.opt_short = "ag",
		.sysfs_name = "aggregated_ogms",
		.params = sysfs_param_enable,
	},
	{
		.opt_long = "bonding",
		.opt_short = "b",
		.sysfs_name = "bonding",
		.params = sysfs_param_enable,
	},
	{
		.opt_long = "fragmentation",
		.opt_short = "f",
		.sysfs_name = "fragmentation",
		.params = sysfs_param_enable,
	},
	{
		.opt_long = "network_coding",
		.opt_short = "nc",
		.sysfs_name = "network_coding",
		.params = sysfs_param_enable,
	},
	{
		.opt_long = "isolation_mark",
		.opt_short = "mark",
		.sysfs_name = "isolation_mark",
		.params = NULL,
	},
	{
		.opt_long = "multicast_mode",
		.opt_short = "mm",
		.sysfs_name = "multicast_mode",
		.params = sysfs_param_enable,
	},
};

static void log_level_usage(void)
{
	fprintf(stderr, "Usage: batctl [options] loglevel [parameters] [level[ level[ level]]...]\n");
	fprintf(stderr, "parameters:\n");
	fprintf(stderr, " \t -h print this help\n");
	fprintf(stderr, "levels:\n");
	fprintf(stderr, " \t none    Debug logging is disabled\n");
	fprintf(stderr, " \t all     Print messages from all below\n");
	fprintf(stderr, " \t batman  Messages related to routing / flooding / broadcasting\n");
	fprintf(stderr, " \t routes  Messages related to route added / changed / deleted\n");
	fprintf(stderr, " \t tt      Messages related to translation table operations\n");
	fprintf(stderr, " \t bla     Messages related to bridge loop avoidance\n");
	fprintf(stderr, " \t dat     Messages related to arp snooping and distributed arp table\n");
	fprintf(stderr, " \t nc      Messages related to network coding\n");
	fprintf(stderr, " \t mcast   Messages related to multicast\n");
}

int handle_loglevel(char *mesh_iface, int argc, char **argv)
{
	int optchar, res = EXIT_FAILURE;
	int log_level = 0;
	char *path_buff;
	char str[4];
	int i;

	while ((optchar = getopt(argc, argv, "h")) != -1) {
		switch (optchar) {
		case 'h':
			log_level_usage();
			return EXIT_SUCCESS;
		default:
			log_level_usage();
			return EXIT_FAILURE;
		}
	}

	path_buff = malloc(PATH_BUFF_LEN);
	snprintf(path_buff, PATH_BUFF_LEN, SYS_BATIF_PATH_FMT, mesh_iface);

	if (argc != 1) {
		for (i = 1; i < argc; i++) {
			if (strcmp(argv[i], "none") == 0) {
				log_level = 0;
				break;
			} else if (strcmp(argv[i], "all") == 0) {
				log_level = 63;
				break;
			} else if (strcmp(argv[i], "batman") == 0)
				log_level |= BIT(0);
			else if (strcmp(argv[i], "routes") == 0)
				log_level |= BIT(1);
			else if (strcmp(argv[i], "tt") == 0)
				log_level |= BIT(2);
			else if (strcmp(argv[i], "bla") == 0)
				log_level |= BIT(3);
			else if (strcmp(argv[i], "dat") == 0)
				log_level |= BIT(4);
			else if (strcmp(argv[i], "nc") == 0)
				log_level |= BIT(5);
			else if (strcmp(argv[i], "mcast") == 0)
				log_level |= BIT(6);
			else {
				log_level_usage();
				goto out;
			}
		}

		snprintf(str, sizeof(str), "%i", log_level);
		res = write_file(path_buff, SYS_LOG_LEVEL, str, NULL);
		goto out;
	}

	res = read_file(path_buff, SYS_LOG_LEVEL, USE_READ_BUFF, 0, 0, 0);

	if (res != EXIT_SUCCESS)
		goto out;

	log_level = strtol(line_ptr, (char **) NULL, 10);

	printf("[%c] %s (%s)\n", (!log_level) ? 'x' : ' ',
	       "all debug output disabled", "none");
	printf("[%c] %s (%s)\n", (log_level & BIT(0)) ? 'x' : ' ',
	       "messages related to routing / flooding / broadcasting",
	       "batman");
	printf("[%c] %s (%s)\n", (log_level & BIT(1)) ? 'x' : ' ',
	       "messages related to route added / changed / deleted", "routes");
	printf("[%c] %s (%s)\n", (log_level & BIT(2)) ? 'x' : ' ',
	       "messages related to translation table operations", "tt");
	printf("[%c] %s (%s)\n", (log_level & BIT(3)) ? 'x' : ' ',
	       "messages related to bridge loop avoidance", "bla");
	printf("[%c] %s (%s)\n", (log_level & BIT(4)) ? 'x' : ' ',
	       "messages related to arp snooping and distributed arp table", "dat");
	printf("[%c] %s (%s)\n", (log_level & BIT(5)) ? 'x' : ' ',
	       "messages related to network coding", "nc");
	printf("[%c] %s (%s)\n", (log_level & BIT(6)) ? 'x' : ' ',
	       "messages related to multicast", "mcast");

out:
	free(path_buff);
	return res;
}

static void settings_usage(int setting)
{
	fprintf(stderr, "Usage: batctl [options] %s|%s [parameters]",
	       (char *)batctl_settings[setting].opt_long, (char *)batctl_settings[setting].opt_short);

	if (batctl_settings[setting].params == sysfs_param_enable)
		fprintf(stderr, " [0|1]\n");
	else if (batctl_settings[setting].params == sysfs_param_server)
		fprintf(stderr, " [client|server]\n");
	else
		fprintf(stderr, "\n");

	fprintf(stderr, "parameters:\n");
	fprintf(stderr, " \t -h print this help\n");
}

int handle_sys_setting(char *mesh_iface, int setting, int argc, char **argv)
{
	int vid, optchar, res = EXIT_FAILURE;
	char *path_buff, *base_dev = NULL;
	const char **ptr;

	while ((optchar = getopt(argc, argv, "h")) != -1) {
		switch (optchar) {
		case 'h':
			settings_usage(setting);
			return EXIT_SUCCESS;
		default:
			settings_usage(setting);
			return EXIT_FAILURE;
		}
	}

	/* prepare the classic path */
	path_buff = malloc(PATH_BUFF_LEN);
	snprintf(path_buff, PATH_BUFF_LEN, SYS_BATIF_PATH_FMT, mesh_iface);

	/* if the specified interface is a VLAN then change the path to point
	 * to the proper "vlan%{vid}" subfolder in the sysfs tree.
	 */
	vid = vlan_get_link(mesh_iface, &base_dev);
	if (vid >= 0)
		snprintf(path_buff, PATH_BUFF_LEN, SYS_VLAN_PATH, base_dev, vid);

	if (argc == 1) {
		res = read_file(path_buff, (char *)batctl_settings[setting].sysfs_name,
				NO_FLAGS, 0, 0, 0);
		goto out;
	}

	if (!batctl_settings[setting].params)
		goto write_file;

	ptr = batctl_settings[setting].params;
	while (*ptr) {
		if (strcmp(*ptr, argv[1]) == 0)
			goto write_file;

		ptr++;
	}

	fprintf(stderr, "Error - the supplied argument is invalid: %s\n", argv[1]);
	fprintf(stderr, "The following values are allowed:\n");

	ptr = batctl_settings[setting].params;
	while (*ptr) {
		fprintf(stderr, " * %s\n", *ptr);
		ptr++;
	}

	goto out;

write_file:
	res = write_file(path_buff, (char *)batctl_settings[setting].sysfs_name,
			 argv[1], argc > 2 ? argv[2] : NULL);

out:
	free(path_buff);
	free(base_dev);
	return res;
}

static void gw_mode_usage(void)
{
	fprintf(stderr, "Usage: batctl [options] gw_mode [mode] [sel_class|bandwidth]\n");
	fprintf(stderr, "options:\n");
	fprintf(stderr, " \t -h print this help\n");
}

int handle_gw_setting(char *mesh_iface, int argc, char **argv)
{
	int optchar, res = EXIT_FAILURE;
	char *path_buff, gw_mode;
	const char **ptr;

	while ((optchar = getopt(argc, argv, "h")) != -1) {
		switch (optchar) {
		case 'h':
			gw_mode_usage();
			return EXIT_SUCCESS;
		default:
			gw_mode_usage();
			return EXIT_FAILURE;
		}
	}

	path_buff = malloc(PATH_BUFF_LEN);
	snprintf(path_buff, PATH_BUFF_LEN, SYS_BATIF_PATH_FMT, mesh_iface);

	if (argc == 1) {
		res = read_file(path_buff, SYS_GW_MODE, USE_READ_BUFF, 0, 0, 0);

		if (res != EXIT_SUCCESS)
			goto out;

		if (line_ptr[strlen(line_ptr) - 1] == '\n')
			line_ptr[strlen(line_ptr) - 1] = '\0';

		if (strcmp(line_ptr, "client") == 0)
			gw_mode = GW_MODE_CLIENT;
		else if (strcmp(line_ptr, "server") == 0)
			gw_mode = GW_MODE_SERVER;
		else
			gw_mode = GW_MODE_OFF;

		free(line_ptr);
		line_ptr = NULL;

		switch (gw_mode) {
		case GW_MODE_CLIENT:
			res = read_file(path_buff, SYS_GW_SEL, USE_READ_BUFF, 0, 0, 0);
			break;
		case GW_MODE_SERVER:
			res = read_file(path_buff, SYS_GW_BW, USE_READ_BUFF, 0, 0, 0);
			break;
		default:
			printf("off\n");
			goto out;
		}

		if (res != EXIT_SUCCESS)
			goto out;

		if (line_ptr[strlen(line_ptr) - 1] == '\n')
			line_ptr[strlen(line_ptr) - 1] = '\0';

		switch (gw_mode) {
		case GW_MODE_CLIENT:
			printf("client (selection class: %s)\n", line_ptr);
			break;
		case GW_MODE_SERVER:
			printf("server (announced bw: %s)\n", line_ptr);
			break;
		default:
			goto out;
		}

		free(line_ptr);
		line_ptr = NULL;
		goto out;
	}

	if (strcmp(argv[1], "client") == 0)
		gw_mode = GW_MODE_CLIENT;
	else if (strcmp(argv[1], "server") == 0)
		gw_mode = GW_MODE_SERVER;
	else if (strcmp(argv[1], "off") == 0)
		gw_mode = GW_MODE_OFF;
	else
		goto opt_err;

	res = write_file(path_buff, SYS_GW_MODE, argv[1], NULL);
	if (res != EXIT_SUCCESS)
		goto out;

	if (argc == 2)
		goto out;

	switch (gw_mode) {
	case GW_MODE_CLIENT:
		res = write_file(path_buff, SYS_GW_SEL, argv[2], NULL);
		break;
	case GW_MODE_SERVER:
		res = write_file(path_buff, SYS_GW_BW, argv[2], NULL);
		break;
	}

	goto out;

opt_err:
	fprintf(stderr, "Error - the supplied argument is invalid: %s\n", argv[1]);
	fprintf(stderr, "The following values are allowed:\n");

	ptr = sysfs_param_server;
	while (*ptr) {
		fprintf(stderr, " * %s\n", *ptr);
		ptr++;
	}

out:
	free(path_buff);
	return res;
}

static void ra_mode_usage(void)
{
	fprintf(stderr, "Usage: batctl [options] routing_algo [algorithm]\n");
	fprintf(stderr, "options:\n");
	fprintf(stderr, " \t -h print this help\n");
}

int handle_ra_setting(int argc, char **argv)
{
	DIR *iface_base_dir;
	struct dirent *iface_dir;
	int optchar;
	char *path_buff;
	int res = EXIT_FAILURE;
	int first_iface = 1;

	while ((optchar = getopt(argc, argv, "h")) != -1) {
		switch (optchar) {
		case 'h':
			ra_mode_usage();
			return EXIT_SUCCESS;
		default:
			ra_mode_usage();
			return EXIT_FAILURE;
		}
	}

	if (argc == 2) {
		res = write_file(SYS_SELECTED_RA_PATH, "", argv[1], NULL);
		goto out;
	}

	path_buff = malloc(PATH_BUFF_LEN);
	if (!path_buff) {
		fprintf(stderr, "Error - could not allocate path buffer: out of memory ?\n");
		goto out;
	}

	iface_base_dir = opendir(SYS_IFACE_PATH);
	if (!iface_base_dir) {
		fprintf(stderr, "Error - the directory '%s' could not be read: %s\n",
			SYS_IFACE_PATH, strerror(errno));
		fprintf(stderr, "Is the batman-adv module loaded and sysfs mounted ?\n");
		goto free_buff;
	}

	while ((iface_dir = readdir(iface_base_dir)) != NULL) {
		snprintf(path_buff, PATH_BUFF_LEN, SYS_ROUTING_ALGO_FMT, iface_dir->d_name);
		res = read_file("", path_buff, USE_READ_BUFF | SILENCE_ERRORS, 0, 0, 0);
		if (res != EXIT_SUCCESS)
			continue;

		if (line_ptr[strlen(line_ptr) - 1] == '\n')
			line_ptr[strlen(line_ptr) - 1] = '\0';

		if (first_iface) {
			first_iface = 0;
			printf("Active routing protocol configuration:\n");
		}

		printf(" * %s: %s\n", iface_dir->d_name, line_ptr);

		free(line_ptr);
		line_ptr = NULL;
	}

	closedir(iface_base_dir);
	free(path_buff);

	if (!first_iface)
		printf("\n");

	res = read_file("", SYS_SELECTED_RA_PATH, USE_READ_BUFF, 0, 0, 0);
	if (res != EXIT_SUCCESS)
		return EXIT_FAILURE;

	printf("Selected routing algorithm (used when next batX interface is created):\n");
	printf(" => %s\n", line_ptr);
	free(line_ptr);
	line_ptr = NULL;

	print_routing_algos();
	return EXIT_SUCCESS;

free_buff:
	free(path_buff);
out:
	return res;
}

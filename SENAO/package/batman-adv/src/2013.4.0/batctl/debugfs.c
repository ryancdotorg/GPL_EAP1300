/*
 * Copyright (C) 2009 Clark Williams <williams@redhat.com>
 * Copyright (C) 2009 Xiao Guangrong <xiaoguangrong@cn.fujitsu.com>
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

#include "debugfs.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/vfs.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#ifndef DEBUGFS_MAGIC
#define DEBUGFS_MAGIC          0x64626720
#endif

static int debugfs_premounted;
static char debugfs_mountpoint[MAX_PATH+1];

static const char *debugfs_known_mountpoints[] = {
	"/sys/kernel/debug/",
	"/debug/",
	NULL,
};

/* construct a full path to a debugfs element */
int debugfs_make_path(const char *fmt, char *mesh_iface, char *buffer, int size)
{
	int len;

	if (strlen(debugfs_mountpoint) == 0) {
		buffer[0] = '\0';
		return -1;
	}

	len = strlen(debugfs_mountpoint) + strlen(fmt) + 1;
	if (len >= size)
		return len+1;

	snprintf(buffer, size-1, fmt, debugfs_mountpoint, mesh_iface);
	buffer[size - 1] = '\0';
	return 0;
}

static int debugfs_found;

/* find the path to the mounted debugfs */
const char *debugfs_find_mountpoint(void)
{
	const char **ptr;
	char type[100];
	FILE *fp;

	if (debugfs_found)
		return (const char *) debugfs_mountpoint;

	ptr = debugfs_known_mountpoints;
	while (*ptr) {
		if (debugfs_valid_mountpoint(*ptr) == 0) {
			debugfs_found = 1;
			strcpy(debugfs_mountpoint, *ptr);
			return debugfs_mountpoint;
		}
		ptr++;
	}

	/* give up and parse /proc/mounts */
	fp = fopen("/proc/mounts", "r");
	if (fp == NULL) {
		fprintf(stderr, "Error - can't open /proc/mounts for read: %s\n",
		       strerror(errno));
		return NULL;
	}

	while (fscanf(fp, "%*s %"
		      STR(MAX_PATH)
		      "s %99s %*s %*d %*d\n",
		      debugfs_mountpoint, type) == 2) {
		if (strcmp(type, "debugfs") == 0)
			break;
	}
	fclose(fp);

	if (strcmp(type, "debugfs") != 0)
		return NULL;

	debugfs_found = 1;

	return debugfs_mountpoint;
}

/* verify that a mountpoint is actually a debugfs instance */

int debugfs_valid_mountpoint(const char *debugfs)
{
	struct statfs st_fs;

	if (statfs(debugfs, &st_fs) < 0)
		return -ENOENT;
	else if (st_fs.f_type != (long) DEBUGFS_MAGIC)
		return -ENOENT;

	return 0;
}


int debugfs_valid_entry(const char *path)
{
	struct stat st;

	if (stat(path, &st))
		return -errno;

	return 0;
}

/* mount the debugfs somewhere if it's not mounted */

char *debugfs_mount(const char *mountpoint)
{
	/* see if it's already mounted */
	if (debugfs_find_mountpoint()) {
		debugfs_premounted = 1;
		return debugfs_mountpoint;
	}

	/* if not mounted and no argument */
	if (mountpoint == NULL)
		mountpoint = "/sys/kernel/debug";

	if (mount(NULL, mountpoint, "debugfs", 0, NULL) < 0)
		return NULL;

	/* save the mountpoint */
	strncpy(debugfs_mountpoint, mountpoint, sizeof(debugfs_mountpoint));
	debugfs_found = 1;

	return debugfs_mountpoint;
}

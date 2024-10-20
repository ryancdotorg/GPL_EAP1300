#!/bin/sh
#
# Copyright (c) 2014 The Linux Foundation. All rights reserved.
# Copyright (C) 2011 OpenWrt.org
#

INFINITY6_BOARD_NAME=
INFINITY6_MODEL=

infinity6_board_detect() {
	local machine
	local name

	machine=$(cat /proc/device-tree/model)

	case "$machine" in
	*"INFINITY6"*)
		name="infinity6"
		;;
	esac

	[ -z "$name" ] && name="unknown"

	[ -z "$INFINITY6_BOARD_NAME" ] && INFINITY6_BOARD_NAME="$name"
	[ -z "$INFINITY6_MODEL" ] && INFINITY6_MODEL="$machine"

	[ -e "/tmp/sysinfo/" ] || mkdir -p "/tmp/sysinfo/"

	echo "$INFINITY6_BOARD_NAME" > /tmp/sysinfo/board_name
	echo "$INFINITY6_MODEL" > /tmp/sysinfo/model
}

infinity6_board_name() {
	local name

	[ -f /tmp/sysinfo/board_name ] && name=$(cat /tmp/sysinfo/board_name)
	[ -z "$name" ] && name="unknown"

	echo "$name"
}

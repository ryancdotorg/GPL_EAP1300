#!/bin/sh
#
# Copyright (c) 2014 The Linux Foundation. All rights reserved.
# Copyright (C) 2011 OpenWrt.org
#

INFINITY5_BOARD_NAME=
INFINITY5_MODEL=

infinity5_board_detect() {
	local machine
	local name

	machine=$(cat /proc/device-tree/model)

	case "$machine" in
	*"INFINITY5"*)
		name="infinity5"
		;;
	esac

	[ -z "$name" ] && name="unknown"

	[ -z "$INFINITY5_BOARD_NAME" ] && INFINITY5_BOARD_NAME="$name"
	[ -z "$INFINITY5_MODEL" ] && INFINITY5_MODEL="$machine"

	[ -e "/tmp/sysinfo/" ] || mkdir -p "/tmp/sysinfo/"

	echo "$INFINITY5_BOARD_NAME" > /tmp/sysinfo/board_name
	echo "$INFINITY5_MODEL" > /tmp/sysinfo/model
}

infinity5_board_name() {
	local name

	[ -f /tmp/sysinfo/board_name ] && name=$(cat /tmp/sysinfo/board_name)
	[ -z "$name" ] && name="unknown"

	echo "$name"
}

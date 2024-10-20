#!/bin/sh
#
# Copyright (c) 2014 The Linux Foundation. All rights reserved.
#

do_infinity6() {
	. /lib/infinity6.sh

	infinity6_board_detect
}

boot_hook_add preinit_main do_infinity6

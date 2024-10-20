#!/bin/sh
#
# Copyright (c) 2014 The Linux Foundation. All rights reserved.
#

do_infinity5() {
	. /lib/infinity5.sh

	infinity5_board_detect
}

boot_hook_add preinit_main do_infinity5

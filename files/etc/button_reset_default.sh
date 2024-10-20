#!/bin/sh
folder="/tmp/UserConfig"
tmpfile="/tmp/UserConfig/overlay.tar.gz"
mtd_block="$(cat /proc/mtd | grep "userconfig" | cut -d ":" -f 1 | sed -e "s/^mtd/mtdblock/g")"
# mtd_block = mtdblock5

upper_folder="/overlay/upper/"

if [ -d "$upper_folder" ];
then
	rm_folder="/overlay/upper/*"
	rm_etc_folder="/overlay/upper/etc/*"
else
	rm_folder="/overlay/*"
	rm_etc_folder="/overlay/etc/*"
fi

([ -d "$folder" ] || (rm -rf "$folder" && mkdir -p "$folder")) && {
	[ -n "$mtd_block" ] && {
		rm $rm_etc_folder -rf
		mount -t jffs2 "/dev/$mtd_block" "$folder"
	}
}

if [ -f "$tmpfile" ];
then
	gunzip -t -f "$tmpfile" && \
	tar -zxvf "$tmpfile" -C/ && \
	echo "===========  config restored  ===========" > /dev/console
	umount "/dev/$mtd_block"
	if [ -f "/usr/sbin/snlogger" ];
	then
		snlogger "event.warn 1.0" "ap_sys_info_reset2default,reason='reset to default by HW button'"
		ezmcheckin
	fi
	sync
	reboot -f
else
	echo "===========  no user_config.tar.gz, restore to factory default  ===========" > /dev/console
	if [ -f "/usr/sbin/snlogger" ];
	then
		snlogger "event.warn 1.0" "ap_sys_info_reset2default,reason='reset to default by HW button'"
		ezmcheckin
	fi
	rm $rm_folder -rf
	sync
	reboot -f
fi


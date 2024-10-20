#!/bin/sh
folder="/tmp/UserConfig"
tmpfile="/tmp/UserConfig/overlay.tar.gz"
target="/overlay/upper/etc"
mtd_block="$(cat /proc/mtd | grep "userconfig" | cut -d ":" -f 1 | sed -e "s/^mtd/mtdblock/g")"
# mtd_block = mtdblock5

uci set sysProductInfo.version=vesrion
uci set sysProductInfo.version.gui=2
uci commit
sleep 1

([ -d "$folder" ] || (rm -rf "$folder" && mkdir -p "$folder")) && {
	[ -n "$mtd_block" ] && mount -t jffs2 "/dev/$mtd_block" "$folder"
}

if [ -d "$target" ];
then
	rm -rf "$folder"/*
	cd / &&\
	tar -zcvf "$tmpfile" "$target" && \
	echo "===========  config save  ===========" > /dev/console
else
	echo "===========  no /overlay/upper/etc, do nothing  ===========" > /dev/console
fi

umount "/dev/$mtd_block"

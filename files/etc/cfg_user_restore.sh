#!/bin/sh
folder="/tmp/UserConfig"
tmpfile="/tmp/UserConfig/overlay.tar.gz"
mtd_block="$(cat /proc/mtd | grep "userconfig" | cut -d ":" -f 1 | sed -e "s/^mtd/mtdblock/g")"
# mtd_block = mtdblock5
target="/overlay/upper/etc"

([ -d "$folder" ] || (rm -rf "$folder" && mkdir -p "$folder")) && {
	[ -n "$mtd_block" ] && {
		rm /overlay/upper/etc/* -rf
		mount -t jffs2 "/dev/$mtd_block" "$folder"
	}
}

if [ -f "$tmpfile" ];
then
	mkdir -p /tmp/config_check
	rm -rf /tmp/config_check/*
	gunzip -t -f "$tmpfile" && \
	tar -zxvf "$tmpfile" -C /tmp/config_check && {
		[ -d "/tmp/config_check/overlay/upper/" ] && {
			# echo "===========  EX config  ===========" > /dev/console
			# Don't restore GUI v1 config
			guiver=$(uci get sysProductInfo.version.gui -c /tmp/config_check/overlay/upper/etc/config || echo 1)
		} || {
			# echo "===========  smart config  ===========" > /dev/console
			# Don't restore GUI v1 config
			guiver=$(uci get sysProductInfo.version.gui -c /tmp/config_check/overlay/etc/config || echo 1)
		}

		rm -rf /tmp/config_check
		if [ "$guiver" -ge 2 ];
		then
			[ -d "$target" ] && rm -rf "$target"/* &&
				echo "===========  erase /overlay/upper/etc/*  ===========" > /dev/console
			tar -zxvf "$tmpfile" -C  / &&
				echo "===========  config restored  ===========" > /dev/console
			[ -d "/overlay/etc/" ] && {
				# echo "===========  smart config, move to upper  ===========" > /dev/console
				mv /overlay/etc /overlay/upper/
			}
			umount "/dev/$mtd_block"
			
			return 0
		else
			echo "===========  Don't restore v1 config, do nothing  ===========" > /dev/console
		fi
	}
else
	echo "===========  no user_config.tar.gz, do nothing  ===========" > /dev/console
fi
return 1

#
# Copyright (C) 2011 OpenWrt.org
#

# @ sstar/infinity6/base-files/

. /lib/infinity5.sh


PART_NAME=firmware
RAMFS_COPY_DATA=/lib/infinity5.sh

platform_find_partitions() {
	local first dev size erasesize name
	while read dev size erasesize name; do
		name=${name#'"'}; name=${name%'"'}
		case "$name" in
			vmlinux.bin.l7|vmlinux|kernel|linux|linux.bin|rootfs|filesystem)
				if [ -z "$first" ]; then
					first="$name"
				else
					echo "$erasesize:$first:$name"
					break
				fi
			;;
		esac
	done < /proc/mtd
}

platform_find_kernelpart() {
	local part
	for part in "${1%:*}" "${1#*:}"; do
		case "$part" in
			vmlinux.bin.l7|vmlinux|kernel|linux|linux.bin)
				echo "$part"
				break
			;;
		esac
	done
}

senao_get_vender_id() {
	get_image "$@" | dd bs=2 count=1 skip=3 2>/dev/null | hexdump -v -n 2 -e '1/1 "%02x"'
}

senao_get_product_id() {
	get_image "$@" | dd bs=2 count=1 skip=5 2>/dev/null | hexdump -v -n 2 -e '1/1 "%02x"'
}

senao_image_header_check(){
	senao_magic_long="$(get_magic_long "$1")"
	senao_vendor_id="$(senao_get_vender_id "$1")"
	senao_product_id="$(senao_get_product_id "$1")"
	eval "rm -rf /tmp/fw_id.txt"
	echo "senao_vendor_id = $senao_vendor_id" > /tmp/fw_id.txt
	echo "senao_product_id = $senao_product_id" >> /tmp/fw_id.txt
	echo "senao_magic_long = $senao_magic_long" >> /tmp/fw_id.txt
	check_senao_image_header $senao_magic_long $senao_vendor_id $senao_product_id && return 0
	return 1
}

platform_check_image() {
	local board=$(infinity6_board_name)
	local magic="$(get_magic_word "$1")"
	local magic_long="$(get_magic_long "$1")"

	[ "$#" -gt 1 ] && return 1

	case "$board" in
	infinity6)
		echo "infinity6"
		[ "$magic" != "2705" ] && {
			echo "Invalid image type."
			return 1
		}
		return 0
		;;
	*)
		senao_magic_long="$(get_magic_long "$1")"
		senao_vendor_id="$(senao_get_vender_id "$1")"
		senao_product_id="$(senao_get_product_id "$1")"
		eval "rm -rf /tmp/fw_id.txt"
		echo "senao_vendor_id = $senao_vendor_id" > /tmp/fw_id.txt
		echo "senao_product_id = $senao_product_id" >> /tmp/fw_id.txt
		echo "magic_long = $magic_long" >> /tmp/fw_id.txt
		check_senao_image_header $senao_magic_long $senao_vendor_id $senao_product_id && return 0
		return 1
		;;
	esac

	echo "Sysupgrade is not yet supported on $board."
	return 1
}

platform_pre_upgrade() {
	local board=$(infinity5_board_name)

	case "$board" in
	infinity5)
		;;
	esac
}

platform_do_upgrade() {
	local board=$(infinity5_board_name)

	case "$board" in
	cus532k)
		platform_do_upgrade_ioe "$ARGV" "$board"
		;;
	*)
		default_do_upgrade "$ARGV"
		;;
	esac
}

disable_watchdog() {
	killall watchdog
	( ps | grep -v 'grep' | grep '/dev/watchdog' ) && {
		echo 'Could not disable watchdog'
		return 1
	}
}

append sysupgrade_pre_upgrade disable_watchdog

#
# Copyright (C) 2011 OpenWrt.org
#

USE_REFRESH=1

. /lib/ipq806x.sh
. /lib/upgrade/common.sh

RAMFS_COPY_DATA=/lib/ipq806x.sh
RAMFS_COPY_BIN="/usr/bin/dumpimage /bin/mktemp /usr/sbin/mkfs.ubifs
	/usr/sbin/ubiattach /usr/sbin/ubidetach /usr/sbin/ubiformat /usr/sbin/ubimkvol
	/usr/sbin/ubiupdatevol /usr/bin/basename /bin/rm /usr/bin/find /sbin/setconfig /bin/mkdir"

get_full_section_name() {
	local img=$1
	local sec=$2

	dumpimage -l ${img} | grep "^ Image.*(${sec})" | \
		sed 's,^ Image.*(\(.*\)),\1,'
}

image_contains() {
	local img=$1
	local sec=$2
	dumpimage -l ${img} | grep -q "^ Image.*(${sec}.*)" || return 1
}

print_sections() {
	local img=$1

	dumpimage -l ${img} | awk '/^ Image.*(.*)/ { print gensub(/Image .* \((.*)\)/,"\\1", $0) }'
}

image_has_mandatory_section() {
	local img=$1
	local mandatory_sections=$2

	for sec in ${mandatory_sections}; do
		image_contains $img ${sec} || {\
			return 1
		}
	done
}

image_demux() {
	local img=$1

	for sec in $(print_sections ${img}); do
		local fullname=$(get_full_section_name ${img} ${sec})

		dumpimage -i ${img} -o /tmp/${fullname}.bin ${fullname} > /dev/null || { \
			echo "Error while extracting \"${sec}\" from ${img}"
			return 1
		}
	done
	return 0
}

image_is_FIT() {
	if ! dumpimage -l $1 > /dev/null 2>&1; then
		echo "$1 is not a valid FIT image"
		return 1
	fi
	return 0
}

switch_layout() {
	local layout=$1
	local boot_layout=`find / -name boot_layout`

	# Layout switching is only required as the  boot images (up to u-boot)
	# use 512 user data bytes per code word, whereas Linux uses 516 bytes.
	# It's only applicable for NAND flash. So let's return if we don't have
	# one.

	[ -n "$boot_layout" ] || return

	case "${layout}" in
		boot|1) echo 1 > $boot_layout;;
		linux|0) echo 0 > $boot_layout;;
		*) echo "Unknown layout \"${layout}\"";;
	esac
}

do_flash_mtd() {
	local bin=$1
	local mtdname=$2
	local append=""

	local mtdpart=$(grep "\"${mtdname}\"" /proc/mtd | awk -F: '{print $1}')
	local pgsz=$(cat /sys/class/mtd/${mtdpart}/writesize)
	[ -f "$CONF_TAR" -a "$SAVE_CONFIG" -eq 1 -a "$2" == "rootfs" ] && append="-j $CONF_TAR"

	if [ "$2" == "rootfs" ]; then
		########################################
		#             v1.0.2                                              v1.0.3
		#0x000000190000-0x000000690000 : "0:HLOS"            0x000000190000-0x000000590000 : "0:HLOS"
		#0x000000690000-0x000001f50000 : "rootfs"            0x000000590000-0x000001f50000 : "rootfs"
		#0x000001660000-0x000001f50000 : "rootfs_data"       0x000001560000-0x000001f50000 : "rootfs_data"

		#dev:    size   erasesize  name                      dev:    size   erasesize  name
		#mtd8: 00500000 00010000 "0:HLOS"                    mtd8: 00400000 00010000 "0:HLOS"
		#mtd9: 018c0000 00010000 "rootfs"                    mtd9: 019c0000 00010000 "rootfs"
		#mtd10:008f0000 00010000 "rootfs_data"              mtd10: 009f0000 00010000 "rootfs_data"

		#kernel size=0x500000=5242880 and uboot version grate than 102. Need move hlos & rootfs partition.
		if [ "$(cat /sys/class/mtd/mtd8/size)" == "5242880" -a "$newUbootVer" -gt "102" ]; then
			#output for new kernel size, 0x400000=4194304.
			dd if=/dev/mtd8 of=/tmp/kernel.bin bs=4194304 count=1
			#generate rootfs for 0x590000~0x690000=0x100000=(1048576)
			dd if=/tmp/${bin}.bin of=/tmp/rootfs1.bin bs=1048576 count=1 conv=sync
			cat /tmp/rootfs1.bin >> /tmp/kernel.bin
			# write to 0:HLOS, use mtd write xxx "0:HLOS" cause "Could not open mtd device: 0" error message.
			mtd write /tmp/kernel.bin -e "/dev/mtd8" "/dev/mtd8"

			dd if=/tmp/${bin}.bin of=/tmp/rootfs2.bin bs=1048576 skip=1
			mtd write /tmp/rootfs2.bin $append -e "rootfs" "rootfs"

			return 0
		fi
	fi

    dd if=/tmp/${bin}.bin bs=${pgsz} conv=sync | mtd $append write - -e "/dev/${mtdpart}" "/dev/${mtdpart}"
}

do_flash_emmc() {
	local bin=$1
	local emmcblock=$2

	dd if=/dev/zero of=${emmcblock}
	dd if=/tmp/${bin}.bin of=${emmcblock}
}

do_flash_partition() {
	local bin=$1
	local mtdname=$2
	local emmcblock="$(find_mmc_part "$mtdname")"

	if [ -e "$emmcblock" ]; then
		do_flash_emmc $bin $emmcblock
	else
		do_flash_mtd $bin $mtdname
	fi
}

do_flash_bootconfig() {
	local bin=$1
	local mtdname=$2

	# Fail safe upgrade
	if [ -f /proc/boot_info/getbinary_${bin} ]; then
		cat /proc/boot_info/getbinary_${bin} > /tmp/${bin}.bin
		do_flash_partition $bin $mtdname
	fi
}

do_flash_failsafe_partition() {
	local bin=$1
	local mtdname=$2
	local emmcblock

	# Fail safe upgrade
	[ -f /proc/boot_info/$mtdname/upgradepartition ] && {
		mtdname=$(cat /proc/boot_info/$mtdname/upgradepartition)
	}

	emmcblock="$(find_mmc_part "$mtdname")"

	if [ -e "$emmcblock" ]; then
		do_flash_emmc $bin $emmcblock
	else
		do_flash_mtd $bin $mtdname
	fi
}

set_active_partition() {
	active=$(setconfig -g 38)

	[ "0" = "${active}" ] && (
		setconfig -a 1
		setconfig -a 2 -s 38 -d 1
		setconfig -a 5
	)

	[ "1" = "${active}" ] && (
		setconfig -a 1
		setconfig -a 2 -s 38 -d 0
		setconfig -a 5
	)
}

do_flash_ubi_dual_img() {
	local bin=$1
	local mtdpart=$2

	ubiformat /dev/${mtdpart} -y -f /tmp/${bin}.bin && set_active_partition
}

do_flash_ubi_single_img() {
	local bin=$1
	local mtdname=$2
	local mtdpart

	mtdpart=$(grep "\"${mtdname}\"" /proc/mtd | awk -F: '{print $1}')
	ubidetach -f -p /dev/${mtdpart}

	# Fail safe upgrade
	[ -f /proc/boot_info/$mtdname/upgradepartition ] && {
		mtdname=$(cat /proc/boot_info/$mtdname/upgradepartition)
	}

	mtdpart=$(grep "\"${mtdname}\"" /proc/mtd | awk -F: '{print $1}')

	ubiformat /dev/${mtdpart} -y -f /tmp/${bin}.bin
}

# TODO: Dual Image - Fail safe upgrade
do_flash_ubi() {
	local mtdpart=$(grep "\"rootfs2\"" /proc/mtd | awk -F: '{print $1}') ## should be mtd1

	if [ -n "${mtdpart}" ]; then
		do_flash_ubi_dual_img $1 ${mtdpart}
	else
		do_flash_ubi_single_img $1 $2
	fi
}

do_flash_tz() {
	local sec=$1
	local mtdpart=$(grep "\0:QSEE\"" /proc/mtd | awk -F: '{print $1}')

	if [ -n "$mtdpart" ]; then
		do_flash_failsafe_partition ${sec} "0:QSEE"
	else
		do_flash_failsafe_partition ${sec} "0:TZ"
	fi
}

to_upper ()
{
	echo $1 | awk '{print toupper($0)}'
}

flash_section() {
	local sec=$1

	local board=$(ipq806x_board_name)
	case "${sec}" in
		hlos*) switch_layout linux; do_flash_failsafe_partition ${sec} "0:HLOS";;
		rootfs*) switch_layout linux; do_flash_failsafe_partition ${sec} "rootfs";;
		fs*) switch_layout linux; do_flash_failsafe_partition ${sec} "rootfs";;
		ubi*) switch_layout linux; do_flash_ubi ${sec} "rootfs";;
		sbl1*)
			[ $isUpgradeUboot == 1 ] && {
				switch_layout boot; do_flash_partition ${sec} "0:SBL1";} || {
				return 1;}
			;;
		sbl2*) switch_layout boot; do_flash_failsafe_partition ${sec} "0:SBL2";;
		sbl3*) switch_layout boot; do_flash_failsafe_partition ${sec} "0:SBL3";;
		mibib*)
			[ $isUpgradeUboot == 1 ] && {
				switch_layout boot; do_flash_partition ${sec} "0:MIBIB";} || {
				return 1;}
			;;
		dtb-$(to_upper $board)*) switch_layout boot; do_flash_partition ${sec} "0:DTB";;
        u-boot*)
			[ $isUpgradeUboot == 1 ] && {
				switch_layout boot; do_flash_failsafe_partition ${sec} "0:APPSBL";} || {
				return 1;}
			;;
		#ddr-$(to_upper $board)*) switch_layout boot; do_flash_partition ${sec} "0:DDRPARAMS";;
		#ddr-${board}-*) switch_layout boot; do_flash_failsafe_partition ${sec} "0:DDRCONFIG";;
		ddr*) switch_layout boot; do_flash_failsafe_partition ${sec} "0:CDT";;
		ssd*) switch_layout boot; do_flash_partition ${sec} "0:SSD";;
		tz*) switch_layout boot; do_flash_tz ${sec};;
		rpm*) switch_layout boot; do_flash_failsafe_partition ${sec} "0:RPM";;
		*) echo "Section ${sec} ignored"; return 1;;
	esac

	echo "Flashed ${sec}"
}

erase_emmc_config() {
	local emmcblock="$(find_mmc_part "rootfs_data")"
	if [ -e "$emmcblock" -a "$SAVE_CONFIG" -ne 1 ]; then
		dd if=/dev/zero of=${emmcblock}
	fi
}

senao_get_vender_id() {
	get_image "$@" | dd bs=2 count=1 skip=3 2>/dev/null | hexdump -v -n 2 -e '1/1 "%02x"'
}

senao_get_product_id() {
	get_image "$@" | dd bs=2 count=1 skip=5 2>/dev/null | hexdump -v -n 2 -e '1/1 "%02x"'
}

senao_get_version_major() {
	get_image "$@" | dd bs=1 count=2 skip=106 2>/dev/null | hexdump -v -n 2 -e '1/1 "%x"'
}

senao_get_version_minor() {
	get_image "$@" | dd bs=1 count=2 skip=110 2>/dev/null | hexdump -v -n 2 -e '1/1 "%x"'
}

senao_image_header_check(){
	senao_magic_long="$(get_magic_long "$1")"
	senao_vendor_id="$(senao_get_vender_id "$1")"
	senao_product_id="$(senao_get_product_id "$1")"
	senao_version_major=$(printf "%d" 0x"$(senao_get_version_major "$1")")
	senao_version_minor=$(printf "%d" 0x"$(senao_get_version_minor "$1")")
	eval "rm -rf /tmp/fw_id.txt"
	echo "senao_vendor_id = $senao_vendor_id" > /tmp/fw_id.txt
	echo "senao_product_id = $senao_product_id" >> /tmp/fw_id.txt
	echo "senao_magic_long = $senao_magic_long" >> /tmp/fw_id.txt
	# check version must >= 3.7.x
	[ $senao_version_major -eq 3 -a $senao_version_minor -lt 7 ] && return 1
	check_senao_image_header $senao_magic_long $senao_vendor_id $senao_product_id && return 0
	return 1
}

platform_check_image() {
	local board=$(ipq806x_board_name)

	local mandatory_nand="ubi"
	local mandatory_nor_emmc="hlos fs"
	local mandatory_nor="hlos"
	local mandatory_section_found=0
	local optional="sbl1 sbl2 mibib u-boot ddr ssd tz rpm"
	local ignored="bootconfig"

	image_is_FIT $1 || return 1

	if [ $SKIP_IMAGE_CHECK -eq 0 ]; then
		image_has_mandatory_section $1 ${mandatory_nand} && {\
			mandatory_section_found=1
		}

		image_has_mandatory_section $1 ${mandatory_nor_emmc} && {\
			mandatory_section_found=1
		}

		image_has_mandatory_section $1 ${mandatory_nor} && {\
			mandatory_section_found=1
		}
	else
		mandatory_section_found=1
	fi

	if [ $mandatory_section_found -eq 0 ]; then
		echo "Error: mandatory section(s) missing from \"$1\". Abort..."
		return 1
	fi

	for sec in ${optional}; do
		image_contains $1 ${sec} || {\
			echo "Warning: optional section \"${sec}\" missing from \"$1\". Continue..."
		}
	done

	for sec in ${ignored}; do
		image_contains $1 ${sec} && {\
			echo "Warning: section \"${sec}\" will be ignored from \"$1\". Continue..."
		}
	done

	image_demux $1 || {\
		echo "Error: \"$1\" couldn't be extracted. Abort..."
		return 1
	}

	[ -f /tmp/hlos_version ] && rm -f /tmp/*_version
	dumpimage -c $1
	return $?
}

platform_version_upgrade() {
	local version_files="appsbl_version sbl_version tz_version hlos_version rpm_version"
	local sys="/sys/devices/system/qfprom/qfprom0/"
	local tmp="/tmp/"

	for file in $version_files; do
		[ -f "${tmp}${file}" ] && {
			echo "Updating "${sys}${file}" with `cat "${tmp}${file}"`"
			echo `cat "${tmp}${file}"` > "${sys}${file}"
			rm -f "${tmp}${file}"
		}
	done
}

platform_do_upgrade() {
	local board=$(ipq806x_board_name)
	local isUpgradeUboot=0
	local newUbootVer
	local currentUbootVer=$(setconfig -g 24 | awk -F. '{print $1$2$3}')

	# verify some things exist before erasing
	if [ ! -e $1 ]; then
		echo "Error: Can't find $1 after switching to ramfs, aborting upgrade!"
		reboot
	fi

	# use for setconfig
	mkdir -p $RAM_ROOT/var

	for sec in $(print_sections $1); do
		if [ ! -e /tmp/${sec}.bin ]; then
			echo "Error: Cant' find ${sec} after switching to ramfs, aborting upgrade!"
			reboot
		fi

        if [ ${sec%-*} == "u-boot" ]; then
            newUbootVer=$(cat /tmp/${sec}.bin | grep uboot_version | cut -d " " -f 2 | tail -c 6 | awk -F. '{print $1$2$3}')

            if [ $newUbootVer -gt $currentUbootVer ]; then
                isUpgradeUboot=1;
            fi
        fi
	done

	# mkdir -p $RAM_ROOT/var
	case "$board" in
	db149 | ap148 | ap145 | ap148_1xx | db149_1xx | db149_2xx | ap145_1xx | ap160 | ap160_2xx | ap161 | ak01_1xx | ap-dk01.1-c1 | ap-dk01.1-c2 | ap-dk04.1-c1 | ap-dk04.1-c2 | ap-dk04.1-c3 | ap-dk04.1-c4 | ap-dk06.1-c1 | EAP1300)
		for sec in $(print_sections $1); do
			flash_section ${sec} $isUpgradeUboot
		done

		switch_layout linux
		# update bootconfig to register that fw upgrade has been done
		do_flash_bootconfig bootconfig "0:BOOTCONFIG"
		do_flash_bootconfig bootconfig1 "0:BOOTCONFIG1"
		platform_version_upgrade

		erase_emmc_config
		return 0;
		;;
	esac

	echo "Upgrade failed!"
	return 1;
}

platform_copy_config_single() {
	local nand_part="$(find_mtd_part "ubi_rootfs")"
	local emmcblock="$(find_mmc_part "rootfs_data")"

	if [ -e "$nand_part" ]; then
		local mtdname=rootfs
		local mtdpart

		[ -f /proc/boot_info/$mtdname/upgradepartition ] && {
			mtdname=$(cat /proc/boot_info/$mtdname/upgradepartition)
		}

		mtdpart=$(grep "\"${mtdname}\"" /proc/mtd | awk -F: '{print $1}')
		ubiattach -p /dev/${mtdpart}
		mount -t ubifs ubi0:ubi_rootfs_data /tmp/overlay
		tar zxvf /tmp/sysupgrade.tgz -C /tmp/overlay/
	elif [ -e "$emmcblock" ]; then
		mount -t ext4 "$emmcblock" /tmp/overlay
		tar zxvf /tmp/sysupgrade.tgz -C /tmp/overlay/
	fi
}

platform_copy_config_dual() {
	local nand_part="$(find_mtd_part "ubi_rootfs")"

	if [ -e "$nand_part" ]; then
		local mtdname=rootfs2
		local mtdpart

		mtdpart=$(grep "\"${mtdname}\"" /proc/mtd | awk -F: '{print $1}')
		ubiattach -p /dev/${mtdpart}
		mount -t ubifs ubi1:ubi_rootfs_data /tmp/overlay
		tar zxvf /tmp/sysupgrade.tgz -C /tmp/overlay/
	fi
}

platform_copy_config() {
	local mtdpart=$(grep "\"rootfs2\"" /proc/mtd | awk -F: '{print $1}') ## should be mtd1

	if [ -n "${mtdpart}" ]; then
		platform_copy_config_dual
	else
		platform_copy_config_single
	fi
}

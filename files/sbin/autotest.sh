#!/bin/sh

. /sbin/beauty.sh

product=`cat /etc/modelname`
kernel=`uname -a |awk -F'#' {'print $1'}`
version=`cat /etc/version | head -n 3 | tail -n 1`
becho Model: $product
becho Kernel: $kernel
becho Version: $version

cecho 41 37 "=== RUN AUTO TEST ==="
if [ "$1" == "-h" ] || [ "$1" == "" ]; then
	echo "0: Run all test"
	echo "1: nlh test"
	echo "2: cal-power test"
fi
echo "Select Test Item(Default is all) : "
read ans

if [ "$ans" = "" ] || [ "$ans" = "0" ] || [ "$ans" = "1" ]; then
	#Test 1: nlh test
	yecho "* Test 1 *"
	echo "Start nlh test:"
	nlh_check=`opkg list |grep nlh`
	nlc_check=`opkg list |grep nlc`

	if [ "$nlh_check" != "" ]; then
		echo "    Detect use senao-nlh now..."
		rm -f /tmp/allpa_nlh_test
		nlhandler -c "touch /tmp/allpa_nlh_test" 1>/dev/null 2>/dev/null
		sleep 1
		if [ -f "/tmp/allpa_nlh_test" ]; then
			gecho "nlh TEST PASS"
		else
			recho "nlh TEST FAIL"
			recho "PL should rebuild FW!"
		fi
	elif [ "$nlc_check" != "" ]; then
		echo "    Detect use senao-syseye-nlc now..."
		rm -rf /tmp/syseye_selftest
		seipc -a "act_selftest /bin/touch /tmp/syseye_selftest" 1>/dev/null 2>/dev/null
		sleep 1
		if [ -f "/tmp/syseye_selftest" ]; then
			gecho "nlh TEST PASS"
		else
			recho "nlh TEST FAIL"
			recho "PL should rebuild FW!"
		fi
	else
		recho "nlh TEST FAIL !"
		recho "PL should rebuild FW!"
	fi

	echo ""
fi

if [ "$ans" = "" ] || [ "$ans" = "0" ] || [ "$ans" = "2" ]; then
	#Test 2: cal-power test
	art_mtd=`cat /proc/mtd |grep ART | awk -F':' {'print $1'}`
	check_24G=`hexdump -C /dev/$art_mtd |grep 00001000`
	check_5G=`hexdump -C /dev/$art_mtd |grep 00005000`
	yecho "* Test 2 *"
	echo "Start cal-power test:"
	echo "    Detect cal-power data in ART partition..."
	if [ "$check_24G" != "" ] && [ "$check_5G" != "" ]; then
		gecho "cal-power check PASS !"
	else
		recho "cal-power check PASS !"
	fi
fi


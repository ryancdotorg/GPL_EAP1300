#!/bin/sh

en_screen=$(uci show functionlist|grep SUPPORT_SCREEN)

[ -x /usr/sbin/screen ] || {
	echo "Not support screen command yet..."
	echo "Not support screen command yet...">/dev/console
	exit
}

case "$1" in
	"ON"|"on")
		[ "$en_screen" == "" ] && {
			uci add_list functionlist.functionlist.SUPPORT_SCREEN=1
			uci commit
		}
		touch /var/run/utmp
		screen /dev/ttyS0 115200,cs8
	;;
	"OFF"|"off")
		echo "killall screen..."
		echo "killall screen...">/dev/console
		killall screen
		uci delete functionlist.functionlist.SUPPORT_SCREEN 2>/dev/null
		uci commit
		rm /var/run/utmp 2>/dev/null
	;;
	"status"|"st")
		[ "$en_screen" == "" ] && {
			echo "Status: functionlist.functionlist.SUPPORT_SCREEN is not set"
			echo "Status: functionlist.functionlist.SUPPORT_SCREEN is not set">/dev/console
		} || {
			echo "Status: functionlist.functionlist.SUPPORT_SCREEN is enable"
			echo "Status: functionlist.functionlist.SUPPORT_SCREEN is enable">/dev/console
		}
	;;
	*)
		# For telnet
		echo "Usage:"
		echo "	Enable)    run_screen.sh on"
		echo "	Disable)   run_screen.sh off"
		echo "	Status)    run_screen.sh st"
		echo ""
		echo "Please run script again..."
		echo ""
		# For serial port console
		echo "Usage:" >/dev/console
		echo "	Enable)    run_screen.sh on" >/dev/console
		echo "	Disable)   run_screen.sh off" >/dev/console
		echo "	Status)    run_screen.sh st" >/dev/console
		echo "" >/dev/console
		echo "Please run script again..." >/dev/console
		echo "" >/dev/console
	;;
esac


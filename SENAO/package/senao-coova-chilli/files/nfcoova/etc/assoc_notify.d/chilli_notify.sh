#!/bin/sh

#$1: device name
#$2: client mac address
#$3: join or left

. /etc/chilli/chilli-libs.sh
ssidnum=$(get_ssid_num $1)
config_name="br-ssid$ssidnum"
mac=$2

if [ "${1#ath}" != "$1" -a -f /tmp/etc/chilli/$config_name/config ];then
	if [ "$3" = "join" ];then
		# destory mac from qrfs RPS_NO_CPU = 0xffff = 65535
		echo "$2 65535 $1" > /proc/qrfs/rule
		chilli_query_client join $config_name $mac
	elif [ "$3" = "left" ];then
		chilli_query_client left $config_name $mac 1 0
	fi
fi

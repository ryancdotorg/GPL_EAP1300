for wifi_dev in $(/usr/sbin/foreach wireless wifi-device disabled 0); do
	opmode=$(uci get wireless.$wifi_dev.opmode)
	if [ "$opmode" == "ap" -o "$opmode" == "wds_ap" ]; then
		scan_vap=$(sh /sbin/getWifiFirstIF $wifi_dev)
		iwlist $scan_vap scan &
	fi
done

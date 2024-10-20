target_vif=$1
reason_str=$2

if [ "$target_vif" == "all" ]; then
	wifi_devs=$(eval "/usr/sbin/foreach wireless wifi-device disabled 0")
	target_vif="all"	#replace vif since we need to reload all the vifs under this radio
	ssid_str="all"
else
	vif_type=$(uci get wireless.$target_vif)
	if [ "$vif_type" == "wifi-device" ]; then
		wifi_devs=$target_vif
		target_vif="all"	#replace vif since we need to reload all the vifs under this radio
		ssid_str="all"
	elif [ "$vif_type" == "wifi-iface" ]; then
		wifi_devs=$(uci get wireless.$target_vif.device)
		ssid_str=$(uci get wireless.$target_vif.ssid)
	else	#should not happen
		#echo "target_vif=$target_vif, vif_type=$vif_type, should not happen" > /dev/console
		ssid_str="unknown"
	fi
fi

for radio in $wifi_devs; do

	opmode=$(uci get wireless.$radio.opmode)
	if [ "$opmode" != "ap" -a "$opmode" != "wds_ap" ]; then
		echo "$radio opmode is $opmode, no need snlog ap_reload_wifi" > /dev/console
		continue;
	fi

	if [ "$radio" == "wifi0" ]; then
                radio_str="2.4G"
        elif [ "$radio" == "wifi1" -o "$radio" == "wifi2" ]; then
                radio_str="5G"
        elif [ "$radio" == "wifi3" ]; then
                radio_str="6G"
        else
                radio_str="unknown" #should not happen
        fi

	if [ "$target_vif" == "all" ]; then
		vifs=$(eval "/usr/sbin/foreach wireless wifi-iface device $radio")
	else
		vifs=$target_vif	#set it to do single loop
	fi
	for vif in $vifs; do
		opmode=$(uci get wireless.$radio.opmode)
		modeDisplay=$(uci get wireless.$vif.mode_display)
		vifDisabled=$(uci get wireless.$vif.disabled)
		vifNetwork=$(uci get wireless.$vif.network)
		vifMode=$(uci get wireless.$vif.mode)
		
		if [ "$vifNetwork" == "lsp" -o "$vifNetwork" == "app" ]; then
			#echo "vif $vif network is $vifNetwork, no need snlog ap_reload_wifi" > /dev/console
			continue
		fi
		if [ "$vifMode" != "ap" ]; then
			#echo "vif $vif mode is not ap, no need snlog ap_reload_wifi" > /dev/console
			continue
		fi
		if [ "$modeDisplay" != "$opmode" ]; then
			#echo "vif $vif mode display=$modeDisplay, not equal to opmode $opmode, no need snlog ap_reload_wifi" > /dev/console
			continue
		fi
		if [ "$modeDisplay" != "ap" -a "$modeDisplay" != "wds_ap" ]; then
			#echo "vif $vif mode_display $modeDisplay not ap and not wds_ap, no need snlog ap_reload_wifi" > /dev/console
			continue
		fi
		if [ "$vifDisabled" != "0" ]; then
			#echo "vif $vif is disabled, no need snlog ap_reload_wifi" > /dev/console
			continue
		fi
		ssid_str=$(uci get wireless.$vif.ssid)
		snlogger "event.info 1.0" "ap_reload_wifi,radio='$radio_str', ssid_profile_name='$ssid_str', reason='$reason_str'"
	done
done

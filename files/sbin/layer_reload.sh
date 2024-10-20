. /lib/sn_functions.sh
. /lib/functions.sh
. /lib/wifi/wifi_funcs.sh
. /lib/wifi/qcawifi.sh
. /lib/wifi/hostapd.sh
. /lib/wifi/nawds.sh

default_cfg80211=0

reload_vap() {
        device=$1
        vif=$2

	local bgscan_disabled=0
	config_load wireless
        config_get ifname $vif ifname
	config_get network $vif network
	config_get disabled $vif disabled	

	[ -f "/sbin/wifi_reload_log.sh" ] && sh /sbin/wifi_reload_log.sh "$vif" "ssid reload"

	remove_hostapd_conf $ifname
	notify_ubus_to_unbridge $ifname
	bg_vif=$(get_backgroundscan_vif $device)
	if [ "$bg_vif" == "$vif" ]; then
		disable_background_scan $device
		bgscan_disabled=1
	fi

        destroy_vap $ifname
	destroy_dummy_interfaces $device

	if [ "$disabled" == "0" ]; then
	        create_wifi_vap $device $vif
	        set_vap_params $device $vif
		ifconfig $ifname up
		ubus call network.interface."$network" add_device "{ \"name\" : \"$ifname\"}"
		common_dev_setting_after_vap_up "$device"
		## bandsteer settings ##
		index="$(echo $vif |awk -F '_' '{print $3}')"
		/sbin/sn_bandsteering $index

		## dscp setting ##
		set_vap_wmm_dscp $ifname

		#WAR: password error###
		remove_hostapd_conf $ifname
		wpa_cli -g /var/run/hostapd/global raw ADD bss_config=$ifname:/var/run/hostapd-$ifname.conf
		#################	
	fi
	[ "$bgscan_disabled" == "1" ] && set_background_scan $device
	set_beacon_interval $device #no matter increase or decrease vap num, we should recheck and set the beacon interval
	set_nawds_or_mesh_mode $device
	create_dummy_interfaces "$device"
}

set_for_command_layer() {

	config_load wireless

	vif=$1
	option=$2
	value=$3
	#ifname=$(uci changes wireless.$vif | awk -F "." '{print $1"."$2}')
	#option=$(uci changes wireless.$vif | awk -F "[.=]" '{print $3}')
	#value=$(uci changes wireless.$vif | awk -F "[.=]" '{print $4}')

	if [ "$ifname" == "wifi0" -o "$ifname" == "wifi1" -o "$ifname" == "wifi2" ]; then	#radio's commands
		if [ "$option" == "channel" -o "$option" == "txpower" ]; then	#radio's command uses iwconfig to set on vap
			radioIdx=$(echo $vif | awk -F "wifi" '{print $2}')
			vap=$(sh /sbin/getWifiFirstIF $radioIdx)
			iwconfig $vap $option $value
		else	#radio's command uses iwpriv/cfg80211tool to set on radio
			device_if $ifname $option $value
		fi

	elif	[ ! -z $(echo $option | grep bands) ]; then	#bandsteer reladted command
		device=$(uci -q get wireless.$vif.device)
                echo set_sn_bandsteering_per_ssid $vif $ifname > /dev/console
		set_sn_bandsteering_per_ssid $vif $ifname
	else	#vap's commands
		echo device_if $ifname $option $value > /dev/console
		device_if $ifname $option $value
	fi
}

layer=$1
vif=$2
option=$3
value=$4

if [ "$layer" == "1" ]; then
	device=$(uci get wireless.$vif.device)
	echo "====layer=1 vap layer, vif=$vif device=$device===="
	reload_vap $device $vif
elif [ "$layer" == "2" ]; then
	echo "====layer=2 radio layer, vif=device=$vif======"
	#wifi down $vif  #since wifi up already contained wifi down, should not need this
	[ -f "/sbin/wifi_reload_log.sh" ] && sh /sbin/wifi_reload_log.sh "$vif" "radio reload"
	wifi up $vif
elif [ "$layer" == "3" ]; then
	echo "====layer=3 ap layer, should not happen===="
	#luci-reload auto network
elif [ "$layer" == "0" ]; then
	echo "====layer=0 cmd layer, should not happen===="
	#set_for_command_layer $vif $option $value
fi

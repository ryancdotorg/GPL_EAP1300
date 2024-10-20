#!/bin/sh
# Copyright (C) 2006 OpenWrt.org
. /lib/functions.sh
. /lib/functions/service.sh

wlan5g_2=$(uci get functionlist.functionlist.SUPPORT_WLAN5G_2 2>/dev/null)

sync_config_value(){
	local section=$1
	local option=$2
	local value=""
	config_get value "$section" "$option"
	source=$(echo ${section} |grep wifi0 | grep -v wds)
	if [ "$source" != "" ]; then
		target=$(echo ${section} |sed -e s/wifi0/wifi1/)
		vlanid_5g=$(uci get wireless.$target.vlan_id 2>/dev/null)
		if [ "$vlanid_5g" != "" ]; then
			if [ "$value" != "$vlanid_5g" ]; then
				uci set wireless.$target.vlan_id=$value
			fi
		fi
		if [ "$wlan5g_2" != "" ]; then
			target2=$(echo ${section} |sed -e s/wifi0/wifi2/)
			vlanid_5g_2=$(uci get wireless.$target2.vlan_id 2>/dev/null)
			if [ "$vlanid_5g_2" != "" ]; then
				if [ "$value" != "$vlanid_5g_2" ]; then
					uci set wireless.$target2.vlan_id=$value
				fi
			fi
		fi
	fi
}

config_load wireless
config_foreach sync_config_value wifi-iface vlan_id
#uci commit wireless

#!/bin/sh

local dont_sync_array="device network mode mode_display ifname index disabled"

local changes=$( uci changes | grep "wireless.wifi0_ssid_" | sed 's/wireless.wifi0_ssid_/wireless.wifi1_ssid_/g' )
for change in $changes
do
	local option=$(echo "$change"|awk -F'wireless.wifi1_ssid_' '{print $2}'|awk -F'.' '{print $2}'|awk -F'=' '{print $1}')
	local sync=1
	for dont_sync in $dont_sync_array
	do
		if [ "$option" == "$dont_sync" ]; then
        		sync=0
	        fi
	done
	if [ "$sync" == "1" ]; then
		uci set $change
	fi
done


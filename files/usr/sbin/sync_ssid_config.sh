#!/bin/sh

local dont_sync_array="device network mode mode_display ifname index disabled"

local i=1
until [ "$i" == "9" ]
do
	local options=$(uci show wireless.wifi0_ssid_$i|awk -F'wireless.wifi0_ssid_' '{print $2}'|awk -F'.' '{print $2}'|awk -F'=' '{print $1}')
	#local j=1
	#local option=$(echo $options|awk -F' ' '{print $ENVIRON["j"]}')
	for option in $options
	do
		local sync=1
		for dont_sync in $dont_sync_array
		do
			if [ "$option" == "$dont_sync" ]; then
				sync=0
			fi
		done 
		if [ "$sync" == "1" ]; then
			uci set wireless.wifi1_ssid_$i.$option="$(uci get wireless.wifi0_ssid_"$i"."$option")"
		fi
	done
	i=$(($i+1))
done


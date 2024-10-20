#!/bin/sh

date_now=$(date +%s)
modified=0
mpsk_files=$(ls /etc/ezmcloud | grep wpa_psk)

for mpsk_file in $mpsk_files
do
    touch /var/log/ezmcloud/$mpsk_file
    modified=0
    while read line
    do
        seq=$(($seq+1))
        if [ "${line:0:11}" = "expiration=" ]; then
            exp=${line%% *}
            exp=${exp#expiration=}
            if [ "${exp:-0}" != "0" ] && [ $exp -le $date_now ]; then
                modified=1
            else
                echo $line >> /var/log/ezmcloud/$mpsk_file
            fi
        else
            echo $line >> /var/log/ezmcloud/$mpsk_file
        fi
    done < /etc/ezmcloud/$mpsk_file

    if [ "$modified" = "1" -a "$(pidof ezmconfig)" = "" ]; then
        mv /var/log/ezmcloud/$mpsk_file /etc/ezmcloud/$mpsk_file
	reload_sections=$(/usr/sbin/foreach wireless wifi-iface wpa_psk_file /etc/ezmcloud/$mpsk_file)
	for section in $reload_sections
	do
		ifname=$(uci get wireless.$section.ifname)
		if [ "$ifname" != "" ]; then
			hostapd_cli -i $ifname -p /var/run/hostapd-phy0/ reload_wpa_psk
		fi
	done
    else
        rm /var/log/ezmcloud/$mpsk_file
    fi
done

sync

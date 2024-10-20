#!/bin/sh
#file_all='/tmp/ap_cli_info'
. /lib/functions.sh

file_24g_tmp="/tmp/tmp_client_list_24g"
file_24g="/tmp/client_list_24g"
file_5g_tmp="/tmp/tmp_client_list_5g"
file_5g="/tmp/client_list_5g"
file_dhcp_24g="/tmp/client_list_dhcp_24g"
file_dhcp_5g="/tmp/client_list_dhcp_5g"
interface_24g="$(iwconfig 2>/dev/null | grep -E "^ath[^ ]+.*(11b|11g|11ng)" | awk '{print $1}' | tr "\n" " ")"
interface_5g="$(iwconfig 2>/dev/null | grep -E "^ath[^ ]+.*(11a|11na|11ac)" | awk '{print $1}' | tr "\n" " ")"

filelist_24g=$(echo $file_24g_tmp $file_24g $file_dhcp_24g)
filelist_5g=$(echo $file_5g_tmp $file_5g $file_dhcp_5g)

add_fingerprint_to_client_info() {
    if [ -e "$file_tmp" ]; then
	READFILE_1="$file_tmp"
	local j=0

	while read line; do
	    j=$(($j+1))
	    mac="$(awk -F" " 'NR=='$j'{print $1}' $file_tmp)"

	    if [ -n "$mac" ]; then
		READFILE_2="$file_dhcp"
		#
		# wlanconfig athx list sta
		# IEs is "RSN WME" or "WME"
		#
		sed -i 's/RSN WME/RSNWME/' $file_tmp

		local k=0
		#local client=$(cat $file_tmp | sed -n ''$j' p' | tr -d '\r' | tr -s '/t| ' '|')
		local client=$(cat $file_tmp | sed -n ''$j' p' | sed 's/[\t]*$/|\t/g' | tr -d '\r' | tr -s '/t| ' '|')

		while read line; do
		    k=$(($k+1))
		    check=$(cat $file_dhcp | sed -n ''$k' p' | grep $mac)
		    [ -n "$check" ] && {
			local dhcp=$(awk -F "|" 'NR=='$k'{print $2"|"$3"|"$4}' $file_dhcp)
			echo "$client$dhcp" >> $file_target
			break;
		    }
		done < "$READFILE_2"
	    fi
	    if [ -n "$mac" -a -z "$check" ]; then
		echo "${client}0.0.0.0|unknown|unknown" >> $file_target
	    fi
	done < "$READFILE_1"
    fi
}

initial_wifi_ifname() {
	local ifname
	local device
	config_get ifname $1 ifname
	config_get device $1 device
	uci add_list wireless.${device}.ifname=${ifname}
}

config_load wireless
config_get ifname "wifi$1" ifname
[ -n "$ifname" ] || config_foreach initial_wifi_ifname wifi-iface

case "$1" in
	0)
	    file_tmp="$file_24g_tmp"
	    file_dhcp="$file_dhcp_24g"
	    file_target="$file_24g"
	    interface_list="$interface_24g"
	;;
	1)
	    file_tmp="$file_5g_tmp"
	    file_dhcp="$file_dhcp_5g"
	    file_target="$file_5g"
	    interface_list="$interface_5g"
	;;
	2)
	    file_tmp="$file_5g_tmp"_"$1"
	    file_dhcp="$file_dhcp_5g"_"$1"
	    file_target="$file_5g"_"$1"
	    interface_list="$interface_5g"
	;;
	*)
	    exit 0
	;;
esac

file_list=$(echo $file_tmp $file_dhcp $file_target)

for file in $file_list
do
    [ -e "$file" ] && rm "$file"
done

for i in $interface_list
do
    valid=$(echo $ifname | grep -w $i);
    if [ -n "$valid" ]; then
	wlanconfig $i list $2 | grep : >> "$file_tmp"
	if [ -e "/tmp/fingerprint_status_list_$i" ]; then
	    cat "/tmp/fingerprint_status_list_$i" >> "$file_dhcp"
	fi
    fi
done

if [ -e "$file_dhcp" ]; then
    add_fingerprint_to_client_info
else
    cat "$file_tmp" | tr -d '\r' | tr -s '/t| ' ' ' > "$file_target"
fi

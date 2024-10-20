#!/bin/sh

mac=$1

[ -z "$mac" ] && {
	echo "WARNING:[proxyarp_clear.sh] mac address is null." > /dev/console
	return
}

vaps=$(ls /tmp/fingerprint_wifi_list_* | awk -F '_' '{print $4}')

[ -z "$vaps" ] && {
        echo "WARNING:[proxyarp_clear.sh] can't get vap list." > /dev/console
        return
}

for ifname in $vaps; do
        isproxyarp=$(iwpriv $ifname get_proxyarp | awk -F ':' '{print $2}' | awk -F '' '{print $1}')
        isinvap=$(wlanconfig $ifname list | grep $mac)
        [ -z "$isinvap" ] || break
done

if [ -n "$ifname" -a -n "$isinvap" -a "$isproxyarp" == "1" ]; then
        snlog_enabled=$(uci get wifiprofile.snWifiConf.SUPPORT_SENAOLOG)
        if [ "$snlog_enabled" == "1" ]; then
                /sbin/kicksta.sh "proxyarp notification" 1 $mac $ifname 5 &
        else
                /sbin/kicksta.sh 1 $mac $ifname 5 &
        fi
fi

#!/bin/sh

intf=$1
action=$2

if [ "$action" == "up" ]; then
    vif=$(eval "/usr/sbin/foreach wireless wifi-iface ifname $intf")
    hide_ssid=$(uci -q get wireless.$vif.hidden)
    cfg80211tool $intf dynamicbeacon 0
    cfg80211tool $intf hide_ssid ${hide_ssid:-0}
elif [ "$action" == "down" ]; then
    cfg80211tool $intf hide_ssid 1
    cfg80211tool $intf dynamicbeacon 1
fi

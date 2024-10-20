#!/bin/sh

clientmac=$1
gatewayip=$2
vlanIds=$3
bridgeNames="$(uci get network.99.ifname) $(uci get network.lan.ifname)"

vifs=$(eval "/usr/sbin/foreach network | grep vlan")
for vif in $vifs; do
    ifname=$(uci get network.$vif.ifname)
    bridgeNames="$bridgeNames $ifname"
done

if [ -z "$vlanIds"]
then
    vlanIds="$(cat /etc/ezmcloud/config/hostapd-ssid_*.wpa_psk | grep 00:00:00:00:00:00 | awk -F"vlanid=" '{print $2}' | awk '{print $1}') $(cat /etc/ezmcloud/config/hostapd-ssid_*.wpa_psk | grep "$clientmac" | awk -F"vlanid=" '{print $2}' | awk '{print $1}')"
fi

for vlanId in $vlanIds; do
    for bridgeName in $bridgeNames; do
        gatewaymac=$(arp-scan --vlan="$vlanId" --interface="$bridgeName" --arpspa=0.0.0.0 "$gatewayip" | grep "$gatewayip" | awk '{printf $2}' | sed 's/://g')
        if [ -n "$gatewaymac" ]
        then
            break
        fi
    done
    if [ -n "$gatewaymac" ]
    then
        clientmac=$(echo "$clientmac" | sed 's/://g')
        finger_syncli set data "$clientmac" "$gatewaymac" add
        break
    fi
done

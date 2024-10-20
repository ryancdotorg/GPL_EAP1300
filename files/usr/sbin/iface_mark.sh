#!/bin/sh

. /lib/functions.sh

strstr()
{
    # Does $1 contain $2 ?
    [ "${1#*$2*}" = "$1" ] && return 1
    return 0
}

ipt()
{
    iptables -w $*
    ip6tables -w $*
}

get_bridge_name() {
    bridge_name=""
    if [ "${1%%-*}" = "br" ];then
        ifname=$(ls /sys/class/net/$1/brif/)
        bridge_name=$ifname
    else
        bridge_path=$(ls /sys/class/net/br-*/brif/$1/state 2>/dev/null)
        if [ "$bridge_path" != "" ];then
            bridge_path=${bridge_path#/sys/class/net/}
            bridge_name=${bridge_path%%/*}
        fi
    fi

    if [ "$bridge_name" = "" ]; then
        iface_section=$(foreach wireless wifi-iface ifname $1) # wifi0_ssid_1
        if [ "$iface_section" != "" ]; then
            network_name=$(uci get wireless.$iface_section.network)
            if [ "$network_name" != "" ]; then
                bridge_name="br-$(uci get wireless.$iface_section.network)"
            fi
        fi
    fi
    echo $bridge_name
}

get_ssid_num() {
    local input=$1
    if [ "${input%%-*}" = "br" ];then
        ssidnum=${input#br-ssid}
    elif [ "${input//bat/}" != "$input" ]; then
        ssidnum=9
    else
        ssidnum=${input#ath[0-9]}
        ssidnum=$((${ssidnum:-0}+1))
    fi
    echo ${ssidnum:-1}
}

setIfaceMark()
{
    ssid_ifname=$1 # athx or athx.xxx
    bridge_name=$(get_bridge_name $ssid_ifname)
    pure_ifname=${ssid_ifname%.*} # remove vlan tag
    ssid_idx=$(get_ssid_num $pure_ifname)

    if [ "$bridge_name" = "" ]; then
        return
    fi

    ipt -t mangle -A iface_mark -i $bridge_name -m physdev --physdev-in $ssid_ifname --physdev-is-in -j MARK --set-mark 0x${ssid_idx}00/0x000f00
}

setEthIfaceMark()
{
    local ifname
    local enable
    local ssid_profile
    config_get ifname $1 ifname
    config_get enable $1 enable
    config_get ssid_profile $1 ssid_profile
    bridge_name=$(get_bridge_name $ifname)
    portIdx=${1#*_}

    if [ "$enable" != "1" ]; then
        return
    fi

    if [ "$ssid_profile" != "" ]; then
        ipt -t mangle -A iface_mark -i $bridge_name -m physdev --physdev-in $ifname --physdev-is-in -j MARK --set-mark 0x00000${ssid_profile}00/0x00000f00
    else
        ipt -t mangle -A iface_mark -i $bridge_name -m physdev --physdev-in $ifname --physdev-is-in -j MARK --set-mark 0x00008${portIdx}00/0x00008f00
    fi
}

config_load wireless

ipt -N iface_mark -t mangle
ipt -F iface_mark -t mangle
ipt -D fwmark -t mangle -j iface_mark
ipt -I fwmark -t mangle -j iface_mark

# add mark for exist wireless interface
ath_ifaces=$(ls /sys/class/net/ | grep -E "ath|bat")

for ifname in $ath_ifaces
do
    setIfaceMark $ifname
done

# add mark for ethprofile interface
if [ "$(uci get functionlist.functionlist.SUPPORT_SSID_PROFILE 2> /dev/null)" = 1 ]; then
    config_load ethprofile
    config_foreach setEthIfaceMark profile
fi
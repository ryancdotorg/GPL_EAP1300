#!/bin/sh
. /etc/mgmt.d/mgmt_path.sh
load_ebtable() {
	local kernel_version=$(ls /lib/modules/)
	[ -d /sys/module/ebtables ] || insmod /lib/modules/${kernel_version}/ebtables.ko
	[ -d /sys/module/ebtable_broute ] || insmod /lib/modules/${kernel_version}/ebtable_broute.ko
	[ -d /sys/module/ebtable_filter ] || insmod /lib/modules/${kernel_version}/ebtable_filter.ko
	[ -d /sys/module/ebtable_nat ] || insmod /lib/modules/${kernel_version}/ebtable_nat.ko
	[ -d /sys/module/ebt_ip ] || insmod /lib/modules/${kernel_version}/ebt_ip.ko
	[ -d /sys/module/ebt_snat ] || insmod /lib/modules/${kernel_version}/ebt_snat.ko

}

unload_ebtable() {
	[ -d /sys/module/ebt_snat ] && rmmod ebt_snat
	[ -d /sys/module/ebt_ip ] && rmmod ebt_ip
	[ -d /sys/module/ebtable_nat ] && rmmod ebtable_nat
	[ -d /sys/module/ebtable_filter ] && rmmod ebtable_filter
	[ -d /sys/module/ebtable_broute ] && rmmod ebtable_broute
	[ -d /sys/module/ebtables ] && rmmod ebtables
}

disable_mgmt_ebtable()
{
    local ifname="$1"
    ebtables -D INPUT -i $ifname -j mgmt_${ifname}_chain 2>/dev/null
    ebtables -D FORWARD -i $ifname -j mgmt_${ifname}_chain 2>/dev/null
    ebtables -F mgmt_${ifname}_chain 2>/dev/null
    ebtables -X mgmt_${ifname}_chain 2>/dev/null
}

enable_mgmt_ebtable()
{
    local ifname="$1"
    local brlanGWIP="$2"

    if [ "$brlanGWIP" == "" ]
    then
        ebtables -I INPUT -j DROP
        addRuleNoDHCPServer $ifname $brlanGWIP
        ebtables -D INPUT -i $ifname -j mgmt_${ifname}_chain 2>/dev/null
        ebtables -A INPUT -i $ifname -j mgmt_${ifname}_chain
        ebtables -D INPUT -j DROP
        ebtables -D FORWARD -i $ifname -j mgmt_${ifname}_chain
    else
        ebtables -I FORWARD -j DROP
        addRuleDHCPServer $ifname $brlanGWIP
        ebtables -D FORWARD -i $ifname -j mgmt_${ifname}_chain 2>/dev/null
        ebtables -A FORWARD -i $ifname -j mgmt_${ifname}_chain
        ebtables -D FORWARD -j DROP
        ebtables -D INPUT -i $ifname -j mgmt_${ifname}_chain
    fi
}

addRuleDHCPServer() {
    local ifname="$1"
    local brlanGWIP="$2"
    local brlanGWMac=$(arping -D -I br-lan $brlanGWIP -f -w 5 | grep "from $brlanGWIP" | awk '{print $5}'  | sed s/'.*\[\|\].*'//g)
    ## for DNS query packet ##
    ebtables -N mgmt_${ifname}_chain 2>/dev/null
    ebtables -F mgmt_${ifname}_chain 2>/dev/null
    ebtables -A mgmt_${ifname}_chain -p ipv4 -i "$ifname" --ip-proto udp --ip-destination-port 53 -j ACCEPT
    ebtables -A mgmt_${ifname}_chain -p ipv4 -i "$ifname" --ip-proto udp --ip-source-port 53 -j ACCEPT
    ebtables -A mgmt_${ifname}_chain -p ipv4 -i "$ifname" --ip-proto tcp --ip-destination-port 53 -j ACCEPT
    ebtables -A mgmt_${ifname}_chain -p ipv4 -i "$ifname" --ip-proto tcp --ip-source-port 53 -j ACCEPT
    ## for dhcp packtet ##
    ebtables -A mgmt_${ifname}_chain -p ipv4 -i "$ifname" --ip-proto udp --ip-destination-port 67 -j ACCEPT
    ebtables -A mgmt_${ifname}_chain -p ipv4 -i "$ifname" --ip-proto udp --ip-source-port 67 -j ACCEPT
    ebtables -A mgmt_${ifname}_chain -p ipv4 -i "$ifname" --ip-proto udp --ip-destination-port 68 -j ACCEPT
    ebtables -A mgmt_${ifname}_chain -p ipv4 -i "$ifname" --ip-proto udp --ip-source-port 68 -j ACCEPT
    ebtables -A mgmt_${ifname}_chain -p ipv4 -i "$ifname" --ip-dst "$brlanGWIP" -j DROP
    ebtables -A mgmt_${ifname}_chain -p ipv4 -i "$ifname" --ip-dst "$brlanGWIP" -j DROP
    if [ "$brlanGWMac" != "" ]; then
        ebtables -A mgmt_${ifname}_chain -p ipv4 -i "$ifname"  -d  ${brlanGWMac} -j DROP
    fi
}

addRuleNoDHCPServer() {
    ifname="$1"
    ebtables -N mgmt_${ifname}_chain 2>/dev/null
    ebtables -F mgmt_${ifname}_chain 2>/dev/null
    ebtables -A mgmt_${ifname}_chain -p ipv4 ! -i "$ifname" --ip-proto udp --ip-source-port 68 -j DROP
}


brlanIP=$(ifconfig br-lan | grep Bcast | awk -F " " '{print $2}' | awk -F ":" '{print $2}')
brlanMask=$(ifconfig br-lan | grep Bcast | awk -F " " '{print $4}' | awk -F ":" '{print $2}')
brlanPrefix=$(ipcalc.sh $brlanIP $brlanMask | grep PREFIX | awk -F "PREFIX=" '{print $2}' | awk -F " " '{print $1}')
brguestIP=$(ifconfig br-guest | grep Bcast | awk -F " " '{print $2}' | awk -F ":" '{print $2}')
brguestMask=$(ifconfig br-guest | grep Bcast | awk -F " " '{print $4}' | awk -F ":" '{print $2}')
brguestPrefix=$(ipcalc.sh $brlanIP $brlanMask | grep PREFIX | awk -F "PREFIX=" '{print $2}' | awk -F " " '{print $1}')

brlanGWIP=$(route -n | grep -e "^0\.0\.0\.0" | awk '{print $2}')

case "$1" in
    "set_rule")
        load_ebtable
        if [ -e /etc/init.d/syskey ]; then
            /etc/init.d/syskey setValue "ebtables" "mgmt" "1"
        fi
        enable_mgmt_ebtable "$MGMT_IFACE" "$brlanGWIP"
    return
    ;;
    "load")
        load_ebtable
        if [ -e /etc/init.d/syskey ]; then
            /etc/init.d/syskey setValue "ebtables" "mgmt" "1"
        fi
    return
    ;;
    "unload")
        disable_mgmt_ebtable "$MGMT_IFACE"
        if [ -e /etc/init.d/syskey ]; then
            /etc/init.d/syskey setValue "ebtables" "mgmt" "0"
        else
            unload_ebtable
        fi
    return
    ;;
esac

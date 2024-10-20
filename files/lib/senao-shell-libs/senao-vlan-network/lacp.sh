#!/bin/sh

. /lib/sn_set_mac.sh
#LACP setting
bond0_setup() {
	local if2Bound1="eth0"
	local if2Bound2="eth1"
	local short_timeout=$(uci get network.lacp.short_timeout)
	local system_priority=$(uci get network.lacp.system_priority)
	ifconfig bond0 down
	echo 4 > /sys/class/net/bond0/bonding/mode    #LACP
	echo 1 > /sys/class/net/bond0/bonding/xmit_hash_policy #layer3+4
	echo $short_timeout > /sys/class/net/bond0/bonding/lacp_rate #0:long, 1:short
	ifconfig bond0 up
	ifenslave bond0 $if2Bound2 $if2Bound1
	echo $system_priority > /sys/class/net/bond0/bonding/ad_sys_priority #system priority(1~65535)
}

start() {
	bond0_setup
        # "ifenslave bond0 eth0 eth1" will change eth1 mac to eth0 mac.
        # need to set eth0, eth1 mac again.
        sn_set_mac
}

reload() {
	#add sleep to fix eth0 eth1 can add to bond0.
	sleep 3
	bond0_setup
        # "ifenslave bond0 eth0 eth1" will change eth1 mac to eth0 mac.
        # need to set eth0, eth1 mac again.
	sn_set_mac
}

case "$1" in
	start) start;;
	reload) reload;;
esac

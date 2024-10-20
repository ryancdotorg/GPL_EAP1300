#!/bin/sh

lan_mac_file="/tmp/lan_mac.txt"
wan_mac_file="/tmp/wan_mac.txt"
error_msg="/tmp/invalid_mac.txt"

sn_set_mac()
{

    if [ -r $lan_mac_file ]; then
        local lan_mac=$(cat $lan_mac_file)
        ifconfig `uci get /rom/etc/config/network.lan.ifname` hw ether $lan_mac 2>/dev/null
    else
        echo "/etc/init.d/network can not read $lan_mac_file" >> $error_msg

                local ifnames=$(uci get /rom/etc/config/network.lan.ifname)

                for iface in $ifnames; do
                        local if_num=$(echo ${iface} | cut -c 4)
                        local eth_mac=$(cat /tmp/lan${if_num}_mac.txt)
                        if [ -n "${eth_mac}" ]; then
                                ifconfig ${iface} hw ether ${eth_mac} 2>/dev/null
                        else
                                echo "/etc/init.d/network can not read /tmp/lan${if_num}_mac.txt" >> $error_msg
                        fi
                done

    fi

	if [ -r $wan_mac_file ]; then
		local wan_mac=$(cat $wan_mac_file)
		ifconfig `uci get /rom/etc/config/network.wan.ifname` hw ether $wan_mac 2>/dev/null
	fi
}


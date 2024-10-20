#!/bin/sh
. /etc/chilli/chilli-libs.sh

# Drop all v6 packet here if client not auth
# FIXME: must echo +00:aa:bb:cc:dd:10@fe80::01:02 > /proc/net/coova/coova-br-ssid1 to update ipv6 list in the nfcoova, need modify kcoova.c to support it
if [ $bridgemode -eq 1 ]
then
    markvalue=$(get_mark_value $CONFIG_NAME 0)
    ipt6 $iptable_name -t mangle -I $portal_prerouting_mangle_x -i $HS_WANIF -m coova ! --name $HS_KNAME -m mark --mark $markvalue -j DROP
else
    markvalue=$(get_mark_value $CONFIG_NAME 0)
    ipt6 $iptable_name -t mangle -I $portal_prerouting_mangle_x -i $DHCPIF -m coova ! --name $HS_KNAME -m mark --mark $markvalue -j DROP
fi

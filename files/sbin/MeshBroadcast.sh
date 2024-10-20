#!/bin/sh

wifiBand=$1
BroadcastStart=$2   #0 for stop, 1 for start

if [ $wifiBand -eq 0 ]; then
	ifname=$(uci get wireless.wifi0_mesh.ifname)
else
	ifname=$(uci get wireless.wifi1_mesh.ifname)
fi

if [ $BroadcastStart -eq 1 ];then
	iwpriv "$ifname" bc 1
        echo " */4 * * * *     /sbin/MeshBroadcast.sh $wifiBand 0 " >> /etc/crontabs/root
else
	iwpriv "$ifname" bc 0
        sed '/MeshBroadcast/d' /etc/crontabs/root -i
	uci set wireless.wifi0_mesh.MeshEzBroCast=0
	uci set wireless.wifi1_mesh.MeshEzBroCast=0
fi


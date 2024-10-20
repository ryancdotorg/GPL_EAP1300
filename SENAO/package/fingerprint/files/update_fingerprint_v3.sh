#!/bin/sh

#This script used for new fingerprint daemon that independence from dhcrelay
#It records all dhcp reply packet device information, we should append wifi info here

mesh_node_info="/tmp/mesh_global_node_info"
mesh_key=`uci get wireless.wifi1_mesh.aeskey`
mesh_id=`uci get wireless.wifi1_mesh.Mesh_id`

WIFI_STATION_LIST="/tmp/fingerprint_wifi_list_"

fingerprint_data="/tmp/fingerprint.txt"
fingerprint_data_tmp="/tmp/fingerprint_tmp.txt"
fpFileTmpPath="/tmp/fingerprint_v3_file_tmp"
fpFilePath="/tmp/fingerprint_v3_file"

usearping="1"	#it seems interface_arping has some bug that packet will go out through another interface, use interface_ping
CHECK_ALIVE_IN_FINGERPRINT="1"

update_all_dhcp_fingerprint()
{
    if [ "$(batctl gw | awk {'print $1'})" == "server" ]; then

	touch $fpFileTmpPath
	cp $fingerprint_data $fingerprint_data_tmp -f
        getWlanStaInfo 0
        getWlanStaInfo 1
        getWiredDevInfo
        
	cat $fpFileTmpPath > $fpFilePath
        rm $fpFileTmpPath -f
    fi
}

getWlanStaInfo()
{
    vifs=$(eval "/usr/sbin/foreach wireless wifi-iface device wifi$1")	

    for vif in $vifs; do
        dev_disabled=$(uci get wireless.$vif.disabled)

        if [ "$dev_disabled" == "1" ]; then
            continue
        fi

        dev_name=$(uci get wireless.$vif.ifname)

	if [ "$dev_name" == "ath35" ]; then	#not to find mesh device
	    continue
	fi
        wlanconfig $dev_name list sta | tail -n +2 > $WIFI_STATION_LIST$dev_name

        lineNum=$(sed -n '$=' "$WIFI_STATION_LIST$dev_name")
        if [ -z "$lineNum" ]; then
            continue
        fi

        devSSID=$(uci get wireless.$vif.ssid)

        for i in `seq 1 $lineNum`; do
	    devMAC=$(cat $WIFI_STATION_LIST$dev_name | awk '{print $1}' | head -n $i | tail -n 1 | sed 'y/ABCDEF/abcdef/')
            if [ -z "$devMAC" ]; then
                    continue
            fi
            devIP=$(cat $fingerprint_data_tmp | grep -i $devMAC | awk 'FS="|" {print $2}' | tail -n 1)
	    if [ -z "$devIP" ]; then
	            continue
	    fi	   
            devOS=$(cat $fingerprint_data_tmp | grep -i $devMAC | awk 'FS="|" {print $3}' | tail -n 1)
            devHost=$(cat $fingerprint_data_tmp | grep -i $devMAC | awk 'FS="|" {print $4}' | tail -n 1)
            devRSSI=$(cat $WIFI_STATION_LIST$dev_name | grep -i $devMAC | awk '{print $6}' | tail -n 1)
            devTxByte=$(cat $WIFI_STATION_LIST$dev_name | grep -i $devMAC | awk '{print $7}' | tail -n 1)
            devRxByte=$(cat $WIFI_STATION_LIST$dev_name | grep -i $devMAC | awk '{print $8}' | tail -n 1)
	
            sed -i '/'$devMAC'/G' $fingerprint_data_tmp #add an empty line in file, to prevent get wrong line after deleted a line
            sed -i '/'$devMAC'/d' $fingerprint_data_tmp

	    if [ "$devOS" == "LINUX" ]; then
		continue;
	    else
		echo "1	$devMAC	$devIP	$devOS	$devHost	$devSSID	$devRSSI	$devTxByte	$devRxByte" >> $fpFileTmpPath
	    fi
        done
    done
}

getWiredDevInfo()
{
    lineNum=$(sed -n '$=' "$fingerprint_data_tmp")

    if [ -z "$lineNum" ]; then
        return
    fi

    for i in `seq 1 $lineNum`; do
        devMAC=$(cat $fingerprint_data_tmp | awk 'FS="|" {print $1}' | head -n $i | tail -n 1 )
	if [ -z "$devMAC" ]; then
		continue
	fi
        devIP=$(cat $fingerprint_data_tmp | grep -i $devMAC | awk 'FS="|" {print $2}' | head -n 1 )
        devOS=$(cat $fingerprint_data_tmp | grep -i $devMAC | awk 'FS="|" {print $3}' | head -n 1 )
        devHost=$(cat $fingerprint_data_tmp | grep -i $devMAC | awk 'FS="|" {print $4}' | head -n 1 )

#check if it is mesh node
	if [ "$devOS" == "LINUX" ]; then
	    continue;
	fi

#check if it is wan device ip
        if [ -e "/tmp/wandev" ]; then
            wanDev=$(cat /tmp/wandev)
            devWanIP=$(ifconfig $wanDev |grep "inet addr"| awk '{print $2}' | awk -F: '{print $2}')
            if [ "$devWanIP" == "$devIP" ]; then                
                continue;
            fi
        fi

#check if it is br-lan ip
        devBrIP=$(ifconfig br-lan |grep "inet addr"| awk '{print $2}' | awk -F: '{print $2}')
        if [ "$devBrIP" == "$devIP" ]; then
            continue;
        fi

#check if it is from wan
        if [ -n "$wanDev" ]; then
	    if [ "$usearping" == "1" ]; then
		    devFromWan=$(interface_arping -I $wanDev -S $devBrIP -T $devIP -d 1 | grep $devIP | grep "received" | awk '{print $1}')
	    else
		    devFromWan=$(interface_ping -I $wanDev -S $devBrIP -T $devIP -M $devMAC -m 500000 | grep $devIP | grep "bytes received" | awk '{print $1}')
	    fi
            if [ -n "$devFromWan" ]; then   #dev is from wan port, not to be shown in fingerprint_file
                continue;
            fi
        fi

	echo "1	$devMAC	$devIP	$devOS	$devHost	---	---	---	---" >> $fpFileTmpPath
    done
}

update_all_dhcp_fingerprint


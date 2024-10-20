#!/bin/sh

#This script used for new fingerprint daemon that independence from dhcrelay
#It records all dhcp reply packet device information, we should append wifi info here

mesh_node_info="/tmp/mesh_global_node_info"
mesh_key=`uci get wireless.wifi1_mesh.aeskey`
mesh_id=`uci get wireless.wifi1_mesh.Mesh_id`

WIFI_STATION_LIST="/tmp/fingerprint_wifi_list_"

fingerprint_data="/tmp/fingerprint.txt"
fpFileTmpPath="/tmp/fingerprint_v2_file_tmp"
fpFilePath="/tmp/fingerprint_v2_file"

usearping="1"

update_all_dhcp_fingerprint()
{
    if [ "$(batctl gw | awk {'print $1'})" == "server" ]; then

        #2016.10.5 George, update global mesh info if there is no node info.
        if [ ! -e $mesh_node_info ]; then
            /sbin/mesh.sh get_mesh_global_node_info $mesh_id $mesh_key &
            sleep 1
        fi

        getWlanStaInfo 0
        getWlanStaInfo 1
        getWiredDevInfo
        if [ -e $fpFileTmpPath ]; then
            #2016.10.5 George, add latest dhcp relay result to fingerprint.
            cat $fpFileTmpPath > $fpFilePath
            rm $fpFileTmpPath
        fi
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

        wlanconfig $dev_name list sta | tail -n +2 > $WIFI_STATION_LIST$dev_name

        lineNum=$(sed -n '$=' "$WIFI_STATION_LIST$dev_name")

        if [ -z "$lineNum" ]; then
            continue
        fi

        devSSID=$(uci get wireless.$vif.ssid)

        for i in `seq 1 $lineNum`; do
            devMAC=$(cat $WIFI_STATION_LIST$dev_name | awk '{print $1}' | head -n $i | tail -n 1 | awk '{print toupper($0)}' )
            devIP=$(cat $fingerprint_data | grep -i $devMAC | awk 'FS="|" {print $2}' | tail -n 1)
            devOS=$(cat $fingerprint_data | grep -i $devMAC | awk 'FS="|" {print $3}' | tail -n 1)
            devHost=$(cat $fingerprint_data | grep -i $devMAC | awk 'FS="|" {print $4}' | tail -n 1)
	    devConType=$(cat $fingerprint_data | grep -i $devMAC | awk 'FS="|" {print $5}' | tail -n 1)
            devRSSI=$(cat $WIFI_STATION_LIST$dev_name | grep -i $devMAC | awk '{print $6}' | tail -n 1)
            devTxByte=$(cat $WIFI_STATION_LIST$dev_name | grep -i $devMAC | awk '{print $7}' | tail -n 1)
            devRxByte=$(cat $WIFI_STATION_LIST$dev_name | grep -i $devMAC | awk '{print $8}' | tail -n 1)

            if [ $usearping == "1" ]; then
                devAlive=$(arping -f -I br-lan $devIP -c 3 | grep "Received 0")
            else
                devAlive=$(ping $devIP -c 1 | grep "received" | grep -v "0 received"| grep -v "0 packets received")
            fi

            #2016.10.7 George, Prevent duplicate MAC to record to list.
            if [ -z "`cat $fpFileTmpPath | grep $devMAC`" ] && [ -z "$devAlive" ]; then
                if [ -z "`cat $mesh_node_info | grep $devIP`" ]; then
                    echo "1	$devMAC	$devIP	$devOS	$devHost	$devSSID	$devRSSI	$devTxByte	$devRxByte" >> $fpFileTmpPath
	            if [ -z "$devConType" ]; then
                        sed -i '/'$devMAC'/s/$/1/' $fingerprint_data	#append info to last column, 1 for wifi, 2 for ethernet
                    fi
                fi
            fi
        done
    done
}

getWiredDevInfo()
{
    lineNum=$(sed -n '$=' "$fingerprint_data")

    if [ -z "$lineNum" ]; then
        return
    fi

    for i in `seq 1 $lineNum`; do
        devMAC=$(cat $fingerprint_data | awk 'FS="|" {print $1}' | head -n $i | tail -n 1 )
        devIP=$(cat $fingerprint_data | grep -i $devMAC | awk 'FS="|" {print $2}' | head -n 1 )
        devOS=$(cat $fingerprint_data | grep -i $devMAC | awk 'FS="|" {print $3}' | head -n 1 )
        devHost=$(cat $fingerprint_data | grep -i $devMAC | awk 'FS="|" {print $4}' | head -n 1 )
	devConType=$(cat $fingerprint_data | grep -i $devMAC | awk 'FS="|" {print $5}' | head -n 1 )
	if [ "$devContype" == "1" ]; then
		continue;
	fi

#check if it is wan device ip
        if [ -e "/tmp/wandev" ]; then
            wanDev=$(cat /tmp/wandev)
            devWanIP=$(ifconfig $wanDev |grep "inet addr"| awk '{print $2}' | awk -F: '{print $2}')
            if [ "$devWanIP" == "$devIP" ]; then                
                sed -i '/'$devMAC'/ d' $fingerprint_data
                continue;
            fi
        fi

#check if it is br-lan ip
        devBrIP=$(ifconfig br-lan |grep "inet addr"| awk '{print $2}' | awk -F: '{print $2}')
        if [ "$devBrIP" == "$devIP" ]; then
            sed -i '/'$devMAC'/d' $fingerprint_data
            continue;
        fi

#check if it is from wan
        if [ -n "$wanDev" ]; then
	    devFromWan=$(interface_ping -I $wanDev -S $devBrIP -T $devIP -M $devMAC -m 500000 | grep $devIP | grep "bytes received" | awk '{print $1}')
            if [ -n "$devFromWan" ]; then   #dev is from wan port, delete this device from fpwirelist and not to be shown in fingerprint_file
                sed -i '/'$devMAC'/ d' $fingerprint_data
                continue;
            fi
        fi

#check if it is alive       
        if [ $usearping == "1" ]; then
            devAlive=$(arping -f -I br-lan $devIP -c 3 | grep "Received 0")
        else
	    devAlive=$(ping $devIP -c 1 | grep "received" | grep -v "0 received"| grep -v "0 packets received")
        fi
        if [ -n "$devAlive" ]; then
            sed -i '/'$devMAC'/d' $fingerprint_data
	    continue;                           
        fi

        #2016.10.7 George, Prevent duplicate MAC to record to list.
        if [ -z "`cat $fpFileTmpPath | grep -i $devMAC`" ]; then
	    if [ -z "`cat $mesh_node_info | grep $devIP`" ]; then
		sed -i '/'$devMAC'/s/$/2/' $fingerprint_data	#append info to last column, 2 for ethernet
		echo "1	$devMAC	$devIP	$devOS	$devHost	---	---	---	---" >> $fpFileTmpPath
            fi
        fi
    done
}

update_all_dhcp_fingerprint


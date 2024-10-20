#!/bin/sh

## global parameters ##
# role auto-detection
role_file_path="/tmp/currentrole"
currentrole="client"
[ -e "$role_file_path" ] && currentrole=$(cat $role_file_path)
ping_retry_limit=3
debugMsgEnable=0
batman_gw_role=$(batctl gw | awk '{print $1}')

# root hop count
hop_count_file="/tmp/mesh_root_hc"

# renew lan ip
gw_mac_file="/tmp/mesh_gw_mac"

## sub function ##
renewLanIp()
{
    local proto=$(uci get network.lan.proto)
    local PID

    case $proto in
        dhcp)
            PID=`pidof udhcpc`
            #1. release
            /bin/kill -SIGUSR2 $PID
            #2. renew
            /bin/kill -SIGUSR1 $PID
            ;;
        *)
            ;;
    esac
}

chkRenewLanIp()
{
    local gw_mac="$(batctl gwl | grep "=>" | awk -F" " '{ print $2 }')"
    local pre_gw_mac

    if [ -e "$gw_mac_file" ]; then
        pre_gw_mac=$(cat $gw_mac_file)
    fi

    if [ -z "$pre_gw_mac" ]; then
        echo $gw_mac > $gw_mac_file
        pre_gw_mac=$gw_mac
    fi

    if [ "$gw_mac" != "$pre_gw_mac" ]; then
        echo $gw_mac > $gw_mac_file
        renewLanIp
    fi
}

calRootCount()
{
    local hop=-1
    local gw_mac="$(batctl gwl | grep "=>" | awk -F" " '{ print $2 }')"
    if [ "$debugMsgEnable" == "1" ]; then
	echo "debug gw_mac:[$gw_mac]"
    fi
    if [ -n "$gw_mac" ]; then
        hop="$(batctl tr $gw_mac | wc -l)"
    fi
    if [ "$debugMsgEnable" == "1" ]; then	
	echo "debug hop:[$hop]"
    fi
    if [ "$hop" != "0" -a "$hop" != "-1" ]; then
        let "hop=$hop - 1"
    fi
    echo "$hop" > "$hop_count_file"
}

setMeshAP()
{
	if [ "$currentrole" == "client" -a "$batman_gw_role" == "client" ]; then
		# renew lan ip
		chkRenewLanIp
		# root hop count
		calRootCount
		exit 0
	fi
	batctl gw client
	echo client > $role_file_path

	uci set mesh.wifi.role=client
	uci commit mesh

	# renew lan ip
	chkRenewLanIp
	# root hop count
	calRootCount

	#refresh LED status
	/usr/shc/led-internet
}

setGatewayAP()
{
	echo 0 > "$hop_count_file"
	if [ "$currentrole" == "server" -a "$batman_gw_role" == "server" ]; then
		exit 0
	fi
	batctl gw server
	echo server > $role_file_path

	uci set mesh.wifi.role=server
	uci commit mesh
}

## main function ##

[ ! -e "/tmp/booting_is_done" ] && exit

[ -e "/var/run/luci-reload-status" ] && exit #exit if luci reload init scripts

[ -e "/tmp/mesh_syncing" -o -e "/tmp/mode_switching" -o -e "/tmp/mode_switching_network_reload" ] && exit  #exit if syncing or reloading, it may makes something error

mesh24GDisabled=$(uci get wireless.wifi0_mesh.disabled)
mesh5GDisabled=$(uci get wireless.wifi1_mesh.disabled)

if [ "$mesh24GDisabled" == 1 -a "$mesh5GDisabled" == 1 ]; then
	if [ "$debugMsgEnable" == "1" ]; then
		echo ===Mesh function is disabled, clean checking function in crontab=== >> /dev/console
	fi
    sed -i '/chkEth2GW/d' /etc/crontabs/root
    crontab /etc/crontabs/root
    exit 0
fi

gatewayIP=$(route -n| grep 'UG[ \t]'|awk '{ print $2 }')

if [ -z "$gatewayIP" ] || [ "$gatewayIP" == "0.0.0.0" ]; then
	if [ "$debugMsgEnable" == "1" ]; then
		echo ===error! device does not have gateway ip==== >> /dev/console
	fi
	setMeshAP
	exit 0
fi

devIP=$(ifconfig br-lan | awk '/inet addr/{print substr($2,6)}')

if [ -z "$devIP" ]; then
	if [ "$debugMsgEnable" == "1" ]; then
	        echo ===error! device does not have ip address=== >> /dev/console
	fi
	setMeshAP
    exit 0
fi


#ethInterface=$(uci get network.lan.ifname)
#emr3000 is eth0 eth1, use hardcode eth0 for now.
ethInterface=eth0

if [ -z $ethInterface ]; then
	if [ "$debugMsgEnable" == "1" ]; then
        	echo ===error! device does not have ethernet interface=== >> /dev/console
	fi
	setMeshAP
	exit 0
else
	chkEth=$(cat /sys/class/net/"$ethInterface"/carrier)
	if [ "$chkEth" != "1" ]; then
		if [ "$debugMsgEnable" == "1" ]; then
			echo ===ethernet not plugged in, set mesh ap directly=== >> /dev/console
		fi
		setMeshAP
		exit 0
	fi
fi

gatewayMac=$(cat /proc/net/arp | grep "$gatewayIP " | awk '{print $4}')

if [ -z "$gatewayMac" ]; then		#arping the gateway mac to refresh the arp table
	arping -I br-lan $gatewayIP -c 1	
	gatewayMac=$(cat /proc/net/arp | grep "$gatewayIP " | awk '{print $4}')
fi

for i in `seq 1 $ping_retry_limit`
do
	if [ -z "$gatewayMac" ]; then
		pingAns=$(interface_ping -I $ethInterface -S $devIP -T $gatewayIP -d 1 | grep "bytes received" | awk '{print $1}')
	else
		pingAns=$(interface_ping -I $ethInterface -S $devIP -T $gatewayIP -d 1 -M $gatewayMac | grep "bytes received" | awk '{print $1}')
	fi

	if [ "$pingAns" == "$gatewayIP" ]; then 
		if [ "$debugMsgEnable" == "1" ]; then 
			echo pingAns=$pingAns gatewayIP=$gatewayIP >> /dev/console
		fi
		setGatewayAP
		exit 0
	else
        if [ "$debugMsgEnable" == "1" ]; then
            echo ping failed $i times >> /dev/console
        fi
	fi
done

setMeshAP


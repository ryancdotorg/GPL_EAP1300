
MESH_NAWDS_MODE="8"
MESH_NAWDS_DEFAULT_MCS_RATE=54000
mesh_node_max=32

wds_mac_peer="/tmp/wlanwdspeer.sh"

## mac to senao IPv6 unique local ##
mac_to_ipv6_unique_local() {
    local mac=$(echo $1 | tr 'A-F' 'a-f')
    IFS=':'; set $mac; unset IFS
    printf "fc00::%x:%x:%x:%x\n" 0x"`printf %x $((0x${1}^0x02))`"${2} 0x"${3}ff" 0x"fe${4}" 0x"${5}${6}"    
}

MeshESData() {
    #MESH_ES_DATA
    DeviceSN=$(setconfig -g 0 | cut -c 4-9)
    DeviceDomain=$(setconfig -g 4)
    DeviceType=1 # 1 for Mesh Router, 4 for Mesh AP CAM
    DeviceConfig=$(uci get network.sys.mesh_configured_5g)
    echo "$DeviceConfig $DeviceSN $DeviceType $DeviceDomain" > /proc/mesh_es_data
}


set_batman() {
#set +x

DEFAULT_BAT0_DIR="/sys/class/net/bat0/mesh"

	config_get orig_interval "$1" orig_interval
	[ -n "$orig_interval" ] && batctl -m "$1" orig_interval "$orig_interval"

	config_get ap_isolation "$1" ap_isolation
	[ -n "$ap_isolation" ] && batctl -m "$1" ap_isolation "$ap_isolation"

	config_get bridge_loop_avoidance "$1" bridge_loop_avoidance
	[ -n "$bridge_loop_avoidance" ] && batctl -m "$1" bridge_loop_avoidance "$bridge_loop_avoidance"

	config_get distributed_arp_table "$1" distributed_arp_table
	[ -n "$distributed_arp_table" ] && batctl -m "$1" distributed_arp_table "$distributed_arp_table"

	config_get aggregated_ogms "$1" aggregated_ogms
	[ -n "$aggregated_ogms" ] && batctl -m "$1" aggregation "$aggregated_ogms"

	config_get bonding "$1" bonding
	[ -n "$bonding" ] && batctl -m "$1" bonding "$bonding"

	config_get fragmentation "$1" fragmentation
	[ -n "$fragmentation" ] && batctl -m "$1" fragmentation "$fragmentation"

#	config_get network_coding "$1" network_coding
#	[ -n "$network_coding" ] && batctl -m "$1" network_coding "$network_coding"

	config_get multicast_mode "$1" multicast_mode
	[ -n "$multicast_mode" ] && batctl -m "$1" multicast_mode "$multicast_mode"

	echo 15 > $DEFAULT_BAT0_DIR/hop_penalty

#	config_get gw_sel_class "$1" gw_sel_class
#	[ -n "$gw_sel_class" ] && batctl gw_sel_class "$gw_sel_class"
#set -x
}

enable_mesh_nawds() {
#set +x
	local ifname="$1"
	local vif="$2"
	local flag_aesucast
	local cap=0
#	config_get wlanwdspeer "$vif" WLANWDSPeer
	WIFI_CAP=` iwpriv $device get_txchainmask  |cut -d : -f2`
	WIFI_CAP="`echo $WIFI_CAP`"
	case "$hwmode" in
		*AC*)
			case "${htmode},${WIFI_CAP}" in
				HT20,1) cap=0x20;; #AC 1x1 HT20
				HT40,1) cap=0x40;; #AC 1x1 HT40
				HT80,1) cap=0x80;; #AC 1x1 HT80
				HT20,3) cap=0x24;; #AC 2x2 HT20
				HT40,3) cap=0x44;; #AC 2x2 HT40
				HT80,3) cap=0x84;; #AC 2x2 HT80
				HT20,7) cap=0x28;; #AC 3x3 HT20
				HT40,7) cap=0x48;; #AC 3x3 HT40
				HT80,7) cap=0x88;; #AC 3x3 HT80
				HT20,15) cap=0x22;; #AC 4x4 HT20
				HT40,15) cap=0x42;; #AC 4x4 HT40
				HT80,15) cap=0x82;; #AC 4x4 HT80
				HT20,255) cap=0x26;; #AC 8x8 HT20
				HT40,255) cap=0x46;; #AC 8x8 HT40
				HT80,255) cap=0x86;; #AC 8x8 HT80
			esac
			;;
		*N*)
			case "${htmode},${WIFI_CAP}" in
				HT20,1) cap=1;;
				HT40,1) cap=2;;
				HT20,3) cap=5;;
				HT40,3) cap=6;;
				HT20,7) cap=9;;
				HT40,7) cap=10;;
				HT20,15) cap=17;;
				HT40,15) cap=18;;
				HT20,255) cap=0x0d;;
				HT40,255) cap=0x0e;;
			esac
			;;
	esac

	batctl if add "$ifname"

	brctl stp br-lan 0

        local MeshIDEnable=$(uci get wireless.$vif.MeshConnectType)     #always use MeshID, it should not be 0 now
        if [ $MeshIDEnable -eq 1 ]; then
                MeshIDValue=$(uci get wireless.$vif.Mesh_id)
                echo $MeshIDValue > /proc/mesh_id
                iwpriv "$ifname" hide_ssid 1
        else
                echo 0 > /proc/mesh_id
        fi

	if [ -f "/proc/mesh_es_data" ]; then
	    MeshESData
	    iwpriv "$ifname" es_data 1
	fi

        #Fixed 201508 get value from config      
        local mesh_tq_maximum=-60                                
        local mesh_tq_minimun=-85  
        config_get tb_refresh_rate "$vif" MeshTbRefreshRate        
	config_get tb_disconnect_timeout "$vif" MeshConnInactTimeout
        config_get sql_limit_rssi "$vif" MeshSQTLimitRSSI
	
	batctl bl 1
	
	#batctl orig_interval 3000
        batctl orig_interval $(($tb_refresh_rate*1000))

	config_load batman-adv
	config_foreach set_batman mesh

        get_tqassist="$(wlanconfig $ifname nawds tqassist | head -n 7| awk -F: '{printf $2}')"
        mesh_tq_maximum=$(($sql_limit_rssi+10))
        mesh_tq_minimun=$(($sql_limit_rssi-15))
        set_tqassist="$(echo ${get_tqassist} | awk '{printf $1" ""'$sql_limit_rssi'"" ""'$mesh_tq_minimun'"" ""'$mesh_tq_maximum'"" "$5" "$6}')"
        if [ "$(echo ${set_tqassist} | awk '{print NF}')" -eq 6 ]; then
                wlanconfig $ifname nawds tqassist "${set_tqassist}"
        fi

	# mesh robust
	config_get mesh_ro_rssi_thd "$vif" LinkRobustThreshold
	mesh_ro_en=1
	echo "$mesh_ro_en $mesh_ro_rssi_thd $mesh_ro_rssi_thd 2 -1" > /proc/mesh_robust 	#robust thd gap 2 and tolerance -1 for -90 very low rssi case

	ifconfig $ifname mtu 2290

	ifconfig bat0 up
	ifconfig $ifname up

	# Senao : block bat0 send arp packet
	arptables -L | grep -q 'j DROP -o bat0' || arptables -A OUTPUT -o bat0 -j DROP

	# nawds prevent channel shift issue
	iwpriv $device dcs_enable 0
	
	iwpriv $ifname ap_bridge 0
	
	config_get encry "$vif" nawds_encr
	case $encry in
	*aes*) #AES encryption
		config_get passph "$vif" aeskey
		#passph=123456789  #for test
		flag_aesucast=1;
		;;
	esac
	
	iwpriv $ifname wds 1
	
	iwpriv $ifname mcast_rate $MESH_NAWDS_DEFAULT_MCS_RATE
	echo defcaps:${cap} > /dev/console
	wlanconfig "$ifname" nawds defcaps "$cap"
	
	[ -n "$flag_aesucast" ] && {
		wlanconfig "$ifname" nawds enwdsdefaeskey "$passph"
	}
        
	local MeshEzSetup=$(($(uci get wireless.wifi0_mesh.MeshEasySetup) || $(uci get wireless.wifi1_mesh.MeshEasySetup)))
        if [ $MeshEzSetup -eq 1 ]; then		
		esPassword=$(uci get wireless.$vif.aeskey)
		wlanconfig "$ifname" nawds enwdspwd $esPassword
        fi
	
	esPassword=$(uci get wireless.$vif.aeskey)
	wlanconfig $ifname nawds enwdspwd "$esPassword"
	#We do nawds mode 0 is because we have to reset all MAC address without VAP destroy for fast-reload timing
	wlanconfig "$ifname" nawds mode 0
	
	wlanconfig $ifname nawds enwdsnodenum $mesh_node_max

	#sleep at least 10 seconds to wait for nawds init done, otherwise it will not add following wds entry successfully
	sleep 10

	wlanconfig "$ifname" nawds mode $MESH_NAWDS_MODE

    local mesh_controller=$(uci get mesh.wifi.controller)

    [ -n "$mesh_controller" ] && {
        if [ "$mesh_controller" == "master" ]; then
            [ -n "$(cat /etc/crontabs/root | grep chkEth2GW.sh)" ] && {
                sed -i '/chkEth2GW/d' /etc/crontabs/root
                crontab /etc/crontabs/root
            }
            batctl gw_mode server
        else
            mesh_period_check_gw_state
        fi
    } || {
        mesh_period_check_gw_state
    }

    # set Mesh interface IPv6 global local address to br-lan for app-agentd #
    # if your vender support only single-band mesh, you don't need to delete old global addr #
    ## delete old global addr
    old_ip6addr="$(ifconfig br-lan | grep Global | grep 'inet6 addr: fc00' | awk -F " " '{printf $3}')"
    if [ -n "$old_ip6addr" ]; then
        ip -6 addr del ${old_ip6addr} dev br-lan
        sleep 1
    fi
    ## add new one
    mesh_addr="$(ifconfig ${ifname} | grep HWaddr | awk -F " " '{printf $5}')"
    mesh_global="$(mac_to_ipv6_unique_local $mesh_addr)"
    ip -6 addr add ${mesh_global}/64 dev br-lan
    # set end #

#set -x
}

get_cap()
{
	cap=0
	WIFI_CAP=` iwpriv $device get_txchainmask  |cut -d : -f2`
	WIFI_CAP="`echo $WIFI_CAP`"
	case "$1" in
		*AC*|*ac*)
			case "${htmode},${WIFI_CAP}" in
				HT20,1) cap=0x20;; #AC 1x1 HT20
				HT40,1) cap=0x40;; #AC 1x1 HT40
				HT80,1) cap=0x80;; #AC 1x1 HT80
				HT20,3) cap=0x24;; #AC 2x2 HT20
				HT40,3) cap=0x44;; #AC 2x2 HT40
				HT80,3) cap=0x84;; #AC 2x2 HT80
				HT20,7) cap=0x28;; #AC 3x3 HT20
				HT40,7) cap=0x48;; #AC 3x3 HT40
				HT80,7) cap=0x88;; #AC 3x3 HT80
				HT20,15) cap=0x22;; #AC 4x4 HT20
				HT40,15) cap=0x42;; #AC 4x4 HT40
				HT80,15) cap=0x82;; #AC 4x4 HT80
				HT20,255) cap=0x26;; #AC 8x8 HT20
				HT40,255) cap=0x46;; #AC 8x8 HT40
				HT80,255) cap=0x86;; #AC 8x8 HT80
			esac
			;;
		*N*|*n*)
			case "${htmode},${WIFI_CAP}" in
				HT20,1) cap=1;;
				HT40,1) cap=2;;
				HT20,3) cap=5;;
				HT40,3) cap=6;;
				HT20,7) cap=9;;
				HT40,7) cap=10;;
				HT20,15) cap=17;;
				HT40,15) cap=18;;
				HT20,255) cap=0x0d;;
				HT40,255) cap=0x0e;;
				*,1) cap=2;; # default HT40 1x1
				*,3) cap=6;; # default HT40 2x2
				*,7) cap=10;; # default HT40 3x3
			esac
			;;
	esac

	echo ${cap}
}

enable_wdsbridge() {
	
	local ifname="$1"
	local vif="$2"
	local cap=0
	local maclist
	config_get wlanwdspeer "$vif" WLANWDSPeer

	cap=$(get_cap "$hwmode")

	#echo "$wlanwdspeer"	
	wlanwdspeer="`echo $wlanwdspeer|tr [a-z] [A-Z]`"
	while [ -n "$wlanwdspeer" ]; do
		[ "`echo $wlanwdspeer|cut -c 13`" = "V" ] && {
	        	maclist="$maclist `echo $wlanwdspeer|cut -c 1-12`"
	        }
	   	wlanwdspeer=`echo $wlanwdspeer|cut -c 14-`
	done
	echo $maclist
	

	iwpriv "$ifname" wds 1
	#We do nawds mode 0 is because we have to reset all MAC address without VAP destroy for fast-reload timing.
	wlanconfig "$ifname" nawds mode 0
        #Set nawds mode to IEEE80211_NAWDS_LEARNING_BRIDGE --> IEEE80211_NAWDS_LEARNING_REPEATER
        # Workaround. Since wds nawds device using wlan driver v10.2.3(LSDK-10.2.r2-00013-External-4) has IOT issue with
        # other nawds device(RD3's) usng older version v10.2.85, a workaround is provided below. 
        # 1. set as IEEE80211_NAWDS_LEARNING_REPEATER mode
        # 2. enable hidden ssid on nawds repeater interface
        # 3. modify wlan driver - skip probe-req if configured as nawds repeater
        #                       - reject auth/assoc-req if configure as nawds repeater
        #                         and request sender is non-nawds device(ex: normal wlan client)

	wlanconfig "$ifname" nawds mode 3
        # give wds bridge interface ssid
	iwconfig "$ifname" essid "nawds_bridge"
	iwpriv "$ifname" hide_ssid 1

	config_get encry "$vif" nawds_encr
	case $encry in
		*ccmp*) #AES encryption
			flag_aesucast=1;
			config_get key "$vif" key
			;;
		*wep*) #WEP encryption
			config_get keypass "$vif" wlanwdswepkey
			keyno="1"
			iwconfig "$ifname" key [$keyno] $keypass
	;;
	esac

	#sleep at least 10 seconds to wait for nawds init done, otherwise it will not add following wds entry successfully
	sleep 10
	
        [ -e "$wds_mac_peer" ] && rm -rf "$wds_mac_peer"

	[ -n "$maclist" ] && {
		config_get wlanwdspeer_mode "$vif" WLANWDSPeer_mode
		wlanwdspeer_mode="`echo $wlanwdspeer_mode|tr [a-z] [A-Z]`"
		for mac in $maclist; do
			[ "$(uci -q get functionlist.functionlist.SUPPORT_WDS_PEER_CAPABILITY)" == "1" ] && {
				[ -n "$wlanwdspeer_mode" ] && {
					# wlanwdspeer_mode="$mac[12]$hwmode[1]v" $hwmode(0:11ac, 1:11n)
					sub_mac_hwmode="${wlanwdspeer_mode#*$mac}"
					[ ${#wlanwdspeer_mode} -gt ${#sub_mac_hwmode} ] && mac_hwmode=$(echo ${sub_mac_hwmode} | cut -c 1)
					# set the mac hwmode to 11n
					[ "$mac_hwmode" == "1" ] && cap=$(get_cap "N")
					[ "$mac_hwmode" == "0" ] && cap=$(get_cap "AC")
				}
			}

			mac=`echo $mac|sed -e \
				"s/\([0-9A-F]\{2\}\)\([0-9A-F]\{2\}\)\([0-9A-F]\{2\}\)\([0-9A-F]\{2\}\)\([0-9A-F]\{2\}\)\([0-9A-F]\{2\}\)/\1:\2:\3:\4:\5:\6/"`
			wlanconfig "$ifname" nawds add-repeater $mac $cap
			[ -n "$flag_aesucast" ] && {
				wlanconfig "$ifname" nawds add-aesucast $mac $key
			}
		done
	}

        wdsb_period_check_gw_state "$ifname"
}

enable_mesh_easysetup() {
	ifname=$1
	if24GName=$(uci get wireless.wifi0_mesh.ifname)
	if5GName=$(uci get wireless.wifi1_mesh.ifname)

        #local wifiBand=$1	#0 for 2.4G 1 for 5G
	if [ "$ifname" = "$if24GName" ]; then
		broadCast=$(uci get wireless.wifi0_mesh.MeshEzBroCast)  #0 for none, 1 for broadcast 2 for listen
		uci set wireless.wifi0_mesh.MeshEzBroCast=0
		wifiBand=0
	else	#elif ["$ifname" = "$if5GName"];then
		broadCast=$(uci get wireless.wifi1_mesh.MeshEzBroCast)
		uci set wireless.wifi1_mesh.MeshEzBroCast=0
		wifiBand=1
	fi

	if [ $broadCast -eq 2 ]; then  # listen
		mesh_do_listen_beacon $wifiBand
	elif [ $broadCast -eq 1 ]; then	#broadcast
		mesh_do_broadcast_beacon $wifiBand
		
	fi
}

mesh_do_listen_beacon(){
	sh /sbin/MeshListen.sh $1 &	#start mesh listen
}

mesh_do_broadcast_beacon(){
        sh /sbin/MeshBroadcast.sh $1 1&   #start mesh broadcast 
}

mesh_period_check_gw_state(){
	[ "0" == "$(cat /etc/crontabs/root | grep chkEth2GW.sh | wc -l)" ] && {
		echo "*/1 * * * * /sbin/chkEth2GW.sh" >> /etc/crontabs/root
		crontab /etc/crontabs/root
	}
}

wdsb_period_check_gw_state(){
        ifname=$1
        crontab -l |grep -v "/sbin/chkEth2WDSB.sh" | crontab -
        crontab -l | { cat; echo "*/1 * * * * /sbin/chkEth2WDSB.sh $ifname"; } | crontab -

}

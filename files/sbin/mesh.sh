#!/bin/sh
. /usr/share/libubox/jshn.sh

show_escape()
{
    escape="$1"
    escape=$(echo "$escape" | sed 's/\\/\\\\/g')
    escape=$(echo "$escape" | sed 's/\"/\\\"/g')
    escape=$(echo "$escape" | sed "s/'/'\"'\"'/g")
    echo "$escape"
}

## mac to IPv6 link-local ##
mac_to_ipv6_link_local() {
    local mac=$(echo $1 | tr 'A-F' 'a-f')
    IFS=':'; set $mac; unset IFS
    printf "fc80::%x:%x:%x:%x\n" 0x"`printf %x $((0x${1}^0x02))`"${2} 0x"${3}ff" 0x"fe${4}" 0x"${5}${6}"    
}
## mac to senao IPv6 unique local ##
mac_to_ipv6_unique_local() {
    local mac=$(echo $1 | tr 'A-F' 'a-f')
    IFS=':'; set $mac; unset IFS
    printf "fc00::%x:%x:%x:%x\n" 0x"`printf %x $((0x${1}^0x02))`"${2} 0x"${3}ff" 0x"fe${4}" 0x"${5}${6}"    
}
## IPv6 link-local to mac ##
ipv6_link_local_to_mac() {
    echo "ipv6_to_mac"
}

json_format_recovery(){
    local json_data="$1"
    local target="$2"

    json_init
    json_load "$json_data"
    json_select "$3"

    local Index="1"
    while json_get_type arrvar $Index && [ "$arrvar" = string ]; do
            json_get_var arrvar "$((Index++))"
	    echo $arrvar
            echo $arrvar >> "$target"
    done
    json_select ".."
}

check_json_result()
{
    json_load "$1"
    json_get_var var1 "$2"
    echo "$var1"
}

del_mesh(){
    deleteDevice="$1"
    ssid="$2"
    wpakey=$(show_escape "$3")
    local payload='{"MeshAdminUsername":"'$ssid'","MeshAdminPassword":"'$wpakey'","ActionType":"'0'","Broadcast":"'false'","TargetMeshIP":"'$deleteDevice'"}'

    eval "app_client -m POST -a mesh/SetMeshNetworkProfile -i '$deleteDevice' -e 1 -d 0 -p '$payload'"
    #eval app_client -m POST -a mesh/SetMeshNetworkProfile -i ${deleteDevice} -d 0 -p '{"MeshAdminUsername":"${ssid}","MeshAdminPassword":"${wpakey}","ActionType":0,"Broadcast":false,"TargetMeshIP":"${deleteDevice}"}'
}
notice_throughput(){
    myselfIP="::1"
    id="$1"
    wpakey=$(show_escape "$2")
    fromIP=$(mac_to_ipv6_unique_local "$3")
    toIP=$(mac_to_ipv6_unique_local "$4")
    local payload='{"MeshAdminUsername":"'$id'","MeshAdminPassword":"'$wpakey'","Source":"'$fromIP'","Destination":"'$toIP'"}'

    eval "app_client -m POST -i '$myselfIP' -a mesh/RunMeshThroughputTest -e 1 -d 0 -p '$payload'"
}
get_throughput(){
    myselfIP="::1"
    id="$1"
    wpakey=$(show_escape "$2")
    fromIP=$(mac_to_ipv6_unique_local "$3")
    toIP=$(mac_to_ipv6_unique_local "$4")
    local payload='{"MeshAdminUsername":"'$id'","MeshAdminPassword":"'$wpakey'","Source":"'$fromIP'","Destination":"'$toIP'"}'

    eval "app_client -m POST -i '$myselfIP' -a mesh/GetMeshThroughputTestResult -e 1 -d 0 -p '$payload'"
}
show_throughput(){
    local counter=0
    local timer="$5"
    local cmdResult
    local testResult

    cmdResult=$(notice_throughput "$1" "$2" "$3" "$4")
    testResult=$(check_json_result "$cmdResult" "RunMeshThroughputTestResult")

    if [ "$testResult" = "OK" -o "$testResult" = "ERROR_PROCESS_IS_RUNNING" ]; then
	while [ $counter -lt $timer ]
	do
	    cmdResult=$(get_throughput "$1" "$2" "$3" "$4")
	    testResult=$(check_json_result "$cmdResult" "GetMeshThroughputTestResultResult")
	    if [ "$testResult" = "OK" ]; then
		counter=$timer
	    else
		counter=$(($counter+2))
		sleep 2
	    fi
	done
    fi
    echo "$cmdResult" | sed 's/RunMeshThroughputTestResult/GetMeshThroughputTestResultResult/g'
}
notice_ping(){
    myselfIP="::1"
    id="$1"
    wpakey=$(show_escape "$2")
    fromIP=$(mac_to_ipv6_unique_local "$3")
    toIP=$(mac_to_ipv6_unique_local "$4")
    pingnum="$5"
    mesh_mac=$(get_myselfmac)
    local payload='{"MeshAdminUsername":"'$id'","MeshAdminPassword":"'$wpakey'","Source":"'$fromIP'","Destination":"'$toIP'","NumberOfPing":"'$pingnum'","MeshMAC":"'$mesh_mac'"}'

    eval "app_client -m POST -i '$myselfIP' -a mesh/RunMeshPingTest -e 1 -d 0 -p '$payload'"
}
get_ping(){
    myselfIP="::1"
    id="$1"
    wpakey=$(show_escape "$2")
    fromIP=$(mac_to_ipv6_unique_local "$3")
    mesh_mac=$(get_myselfmac)
    local payload='{"MeshAdminUsername":"'$id'","MeshAdminPassword":"'$wpakey'","Source":"'$fromIP'","MeshMAC":"'$mesh_mac'"}'

    eval "app_client -m POST -i '$myselfIP' -a mesh/GetMeshPingTestResult -e 1 -d 0 -p '$payload'"
}
show_ping(){
    local counter=0
    local timer="$6"
    local cmdResult
    local testResult

    cmdResult=$(notice_ping "$1" "$2" "$3" "$4" "$5")
    testResult=$(check_json_result "$cmdResult" "RunMeshPingTestResult")

    if [ "$testResult" = "OK" -o "$testResult" = "ERROR_PROCESS_IS_RUNNING" ]; then
	while [ $counter -lt $timer ]
	do
	    cmdResult=$(get_ping "$1" "$2" "$3")
	    testResult=$(check_json_result "$cmdResult" "GetMeshPingTestResultResult")
	    if [ "$testResult" = "OK" ]; then
		counter=$timer
	    else
		counter=$(($counter+2))
		sleep 2
	    fi
	done
    fi
    echo "$cmdResult" | sed 's/RunMeshPingTestResult/GetMeshPingTestResultResult/g'
}
notice_traceroute(){
    myselfIP="::1"
    id="$1"
    wpakey=$(show_escape "$2")
    fromIP=$(mac_to_ipv6_unique_local "$3")
    toIP=$(mac_to_ipv6_unique_local "$4")
    mesh_mac=$(get_myselfmac)
    local payload='{"MeshAdminUsername":"'$id'","MeshAdminPassword":"'$wpakey'","Source":"'$fromIP'","Destination":"'$toIP'","MeshMAC":"'$mesh_mac'"}'

    eval "app_client -m POST -i '$myselfIP' -a mesh/RunMeshTraceRoute -e 1 -d 0 -p '$payload'"
}
get_traceroute(){
    myselfIP="::1"
    id="$1"
    wpakey=$(show_escape "$2")
    fromIP=$(mac_to_ipv6_unique_local "$3")
    toIP=$(mac_to_ipv6_unique_local "$4")
    mesh_mac=$(get_myselfmac)
    local payload='{"MeshAdminUsername":"'$id'","MeshAdminPassword":"'$wpakey'","Source":"'$fromIP'","Destination":"'$toIP'","MeshMAC":"'$mesh_mac'"}'

    eval "app_client -m POST -i '$myselfIP' -a mesh/GetMeshTraceRouteResult -e 1 -d 0 -p '$payload'"
}
show_traceroute(){
    local counter=0
    local timer="$5"
    local cmdResult
    local testResult

    cmdResult=$(notice_traceroute "$1" "$2" "$3" "$4")
    testResult=$(check_json_result "$cmdResult" "RunMeshTraceRouteResult")

    if [ "$testResult" = "OK" -o "$testResult" = "ERROR_PROCESS_IS_RUNNING" ]; then
	while [ $counter -lt $timer ]
	do
	    cmdResult=$(get_traceroute "$1" "$2" "$3" "$4")
	    testResult=$(check_json_result "$cmdResult" "GetMeshTraceRouteResultResult")
	    if [ "$testResult" = "OK" ]; then
		counter=$timer
	    else
		counter=$(($counter+2))
		sleep 2
	    fi
	done
    fi
    echo "$cmdResult" | sed 's/RunMeshTraceRouteResult/GetMeshTraceRouteResultResult/g'
}
get_neighbors(){
    myselfIP="::1"
    id="$1"
    wpakey=$(show_escape "$2")
    local payload='{"MeshAdminUsername":"'$id'","MeshAdminPassword":"'$wpakey'"}'

    eval "app_client -m POST -i '$myselfIP' -a mesh/GetMeshDeviceNeighbors -e 1 -d 0 -p '$payload'"
}
sync_mesh_robust_threshold(){
    id="$1"
    wpakey=$(show_escape "$2")
    rssiThd="$3"
    local originators="$(batctl o -H | grep -v "^No" |awk -F" " '{ print $1 }')"
    local ip_str="::1"
    local payload='{"MeshAdminUsername":"'$id'","MeshAdminPassword":"'$wpakey'","LinkRobustThreshold":"'$rssiThd'"}'

    if [ -n "$originators" ]; then
        for i in $originators
        do
            ip_str="$ip_str,$(mac_to_ipv6_unique_local $i)"
        done
    fi

    if [ -n "$ip_str" ]; then
        eval "app_client -m POST -M '$ip_str' -a mesh/SyncMeshRobustThreshold -e 1 -d 0 -p '$payload'"
    fi
}
get_originator(){
    local local_originator="$(batctl o | grep B.A.T.M.A.N. | awk -F" " '{ print $5 }' | awk -F"/" '{ print $2 }')"
    local local_gw_mode="$(batctl gw | awk -F" " '{ print $1 }')"
    local originators="$(batctl o | tail -n +3 | awk -F" " '{ print $1 }' | tr "\n" " ")"
    local gateways="$(batctl gwl | tail -n +2 | awk -F" " '{ print $2 }' | tr "\n" " ")"

    [ -n "$originators" -a -n "$gateways" ] && {
	[ -f /tmp/mesh_originator_list ] && rm -rf /tmp/mesh_originator_list
	[ -n "$local_originator" -a -n "$local_gw_mode" ] && echo "$local_originator $local_gw_mode" >> /tmp/mesh_originator_list
	for omac in $originators
	do
	    for gmac in $gateways
	    do
		if [ "$omac" = "$gmac" ]; then
		    echo "$omac server" >> /tmp/mesh_originator_list
		    continue 2
		fi
	    done
	    echo "$omac client" >> /tmp/mesh_originator_list
	done
    }
}
get_listening_wds_link(){
    local ifname
    if [ "$1"="0" -o "$1"="1" ]; then
	ifname=$(uci get wireless.wifi"$1"_mesh.ifname)
	[ -n "$ifname" ] && wlanconfig $ifname nawds learning | tail -n 32 | grep -v "00:00:00:00:00:00" | awk -F" " '{ print $2" "$3" "$4 }' > /tmp/mesh_nawds_learning_list
    fi
}
get_mesh_global_node_info() {

    id="$1"
    wpakey=$(show_escape "$2")

	#20180331 Jason:Fix for BATMAN2017
	local batman_ver=`batctl o |grep 2016 1>/dev/null 2>/dev/null && echo '2016' || echo '2017up'`

    if [ "$batman_ver" = "2016" ]; then
		local originators="$(batctl o -H | grep -v "^No" |awk -F" " '{ print $1 }')"
	else
		local originators="$(batctl o -H | grep '*'| grep -v "^No"|awk -F" " '{ print $2 }')"
    fi
    local ip_str="::1"
    local payload='{"MeshAdminUsername":"'$id'","MeshAdminPassword":"'$wpakey'"}'

    if [ -n "$originators" ]; then
       for i in $originators
       do
	   ip_str="$ip_str,$(mac_to_ipv6_unique_local $i)"
       done
    fi

    if [ -n "$ip_str" ]; then
	    eval "app_client -m POST -M '$ip_str' -a mesh/GetMeshNodeInfo -e 1 -d 0 -p '$payload' >/tmp/mesh_global_node_info"
    fi
}
show_mesh_global_node_info() {
    get_mesh_global_node_info "$1" "$2"
    echo $(cat /tmp/mesh_global_node_info)
}
get_mesh_local_node_info() {
    id="$1"
    wpakey=$(show_escape "$2")
    local ip_str="::1"
    local payload='{"MeshAdminUsername":"'$id'","MeshAdminPassword":"'$wpakey'"}'

    eval "app_client -m POST -i '$ip_str' -a mesh/GetMeshNodeInfo -e 1 -d 0 -p '$payload' >/tmp/mesh_local_node_info"
}
show_mesh_local_node_info() {
    get_mesh_local_node_info "$1" "$2"
    echo $(cat /tmp/mesh_local_node_info)
}
get_login_mesh_info() {
    id="$1"
    wpakey=$(show_escape "$2")
    local ip_str="::1"
    local payload='{"MeshAdminUsername":"'$id'","MeshAdminPassword":"'$wpakey'"}'

    eval "app_client -m POST -i '$ip_str' -a mesh/GetLoginMeshNodeInfo -e 1 -d 0 -p '$payload'"
}
get_Mesh_Bundle_Setting() {
    local bundle_mac_str=$(setconfig -g 33)
    local mac_str=$(echo $bundle_mac_str | cut -d= -f2 | sed 's/://g')
    local keyIndex=$(printf %d 0x${mac_str:4:8})
    local LFSR_MASK=$(printf %d 0x98124557)
    local LFSR_MASK2=$(printf %d 0x4321a)
    local base_num=36
    local str36='23456789ABCDEFGHJKLMNPQRSTUVWXYZ38BZ'
    local str36_2='WXCDYNJU8VZABKL46PQ7RS9T2E5H3MFGPWR2'
    local keylen=12

    for i in `seq 0 $((keylen-1))`
    do
	if [ $(($keyIndex % 2)) -eq 1 ]; then		
	    keyIndex=$((((keyIndex ^ $LFSR_MASK) >> 1) | $(printf %d 0x80000000)))
	else
	    keyIndex=$(((keyIndex ^ $LFSR_MASK2) >> 1))
	fi
	eval "char_index$i=$(($keyIndex % $base_num))"
    done

    key=${str36:$char_index0:1}

    for i in `seq 1 $((keylen-1))` 
    do
	eval "preCharIndex=\$char_index$((i-1))"
	eval "charIndex=\$char_index$i"
	if [ $preCharIndex -eq $charIndex ]; then
	    key=$key${str36_2:$((($charIndex+$i)%$base_num)):1}
	else
	    key=$key${str36:$charIndex:1}
	fi
    done

    local MeshIdIndex=$(printf %d 0x${mac_str:6:6})
    local mesh_id=$(($MeshIdIndex*4+10000000))
    uci set wireless.wifi0_mesh.Mesh_id=$mesh_id	#set 2.4G 5G mesh id identical since /proc/mesh_id is only one file, maybe separate it into 2 files in the future	
    uci set wireless.wifi1_mesh.Mesh_id=$mesh_id
    uci set wireless.wifi0_mesh.aeskey=$key
    uci set wireless.wifi1_mesh.aeskey=$key
    uci set wireless.wifi0_mesh.MeshConnectType=1
    uci set wireless.wifi1_mesh.MeshConnectType=1

}
get_myselfmac()
{
    if [ "$(uci get wireless.wifi0_mesh.disabled)" -eq 0 ]; then
        mesh_if="$(uci get wireless.wifi0_mesh.ifname)"
    else
        mesh_if="$(uci get wireless.wifi1_mesh.ifname)"
    fi
	ifconfig $mesh_if | awk '/HWaddr/{print $5}'
}
case $1 in
    del_mesh)     del_mesh "$2" "$3" "$4" ;;
    notice_throughput)     notice_throughput "$2" "$3" "$4" "$5";;
    get_throughput)     get_throughput "$2" "$3" "$4" "$5";;
    show_throughput)     show_throughput "$2" "$3" "$4" "$5" "$6";;
    notice_ping)     notice_ping "$2" "$3" "$4" "$5" "$6";;
    get_ping)     get_ping "$2" "$3" "$4";;
    show_ping)	   show_ping "$2" "$3" "$4" "$5" "$6" "$7";;
    notice_traceroute)     notice_traceroute "$2" "$3" "$4" "$5";;
    get_traceroute)     get_traceroute "$2" "$3" "$4" "$5";;
    show_traceroute)     show_traceroute "$2" "$3" "$4" "$5" "$6";;
    get_neighbors)     get_neighbors "$2" "$3";;
    get_Mesh_Bundle_Setting) get_Mesh_Bundle_Setting;;
    sync_mesh_robust_threshold) sync_mesh_robust_threshold "$2" "$3" "$4";;
    get_originator) get_originator;;
    get_listening_wds_link) get_listening_wds_link "$2";;
    get_mesh_global_node_info) get_mesh_global_node_info "$2" "$3";;
    show_mesh_global_node_info) show_mesh_global_node_info "$2" "$3";;
    get_mesh_local_node_info) get_mesh_local_node_info "$2" "$3";;
    show_mesh_local_node_info) show_mesh_local_node_info "$2" "$3";;
    get_login_mesh_info) get_login_mesh_info "$2" "$3";;
    json_test) json_test;;
    get_myselfmac) get_myselfmac;;
    mac_to_ipv6_unique_local) mac_to_ipv6_unique_local "$2";;
esac

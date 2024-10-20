#!/bin/sh

dhcp_renew()
{
	PID=`pidof udhcpc`
	#1. release
	/bin/kill -SIGUSR2 $PID
	#2. renew
	/bin/kill -SIGUSR1 $PID
}

get_bridge_name() {
	if [ "${1%%-*}" = "br" ];then
		ifname=$(ls /sys/class/net/$1/brif/)
		echo $ifname
	else
		bridge_path=$(ls /sys/class/net/br-*/brif/$1/state 2>/dev/null)
		if [ "$bridge_path" != "" ];then
			bridge_path=${bridge_path#/sys/class/net/}
			echo ${bridge_path%%/*}
		else
			echo
		fi
	fi
}

# only for SSID-based
get_ssid_num() {
	local input=$1
	if [ "${input%%-*}" = "br" ];then
		# filter bridge_prefix, ssid or wds
		ssidnum=${input#br-ssid}
		ssidnum=${ssidnum#br-wds}
	elif [ "${input:0:4}" = "ssid" ];then
		ssidnum=${input#ssid}
	elif [ "${input:0:3}" = "wds" ];then
		ssidnum=${input#wds}
	elif [ "${input:0:3}" = "nat" ];then
		ssidnum=${input#nat}
	else
		ssidnum=${input#ath[0-9]}
		ssidnum=${ssidnum#enjet[0-9]}
		ssidnum=$((${ssidnum:-0}+1))
	fi
	echo ${ssidnum:-1}
}

get_iface() {
	opt=$1
	ifname=$2
	case "$opt" in
		ip)
			ifconfig $ifname | awk '/inet addr/{print substr($2,6)}'
			;;
		mask)
			ifconfig $ifname | awk '/Mask:/{print substr($4,6)}'
			;;
		mac|macnc|machypen)
			local mac=$(ifconfig $ifname | awk '/HWaddr/{print $5}')
			if [ $opt = "macnc" ]; then
				echo $mac |tr -d ":"
			elif [ $opt = "machypen" ]; then
				echo $mac | sed 's/:/-/g'
			else
				echo $mac
			fi
			;;
	esac
}

get_ipv6_ll() {
	local mac=$1
	if [ "$mac" = "" ];then
		mac=$(get_iface mac br-lan)
		mac=${mac:-`setconfig -g 6`}
		mac=$(tolower $mac)
	fi
	IFS=':'; set $mac; unset IFS
	echo "fe80::$(printf %02x $((0x$1 ^ 2)))$2:${3}ff:fe$4:$5$6"
}

tolower() {
	echo $1 | tr [A-Z] [a-z]
}

ipcalc() {
	opt=$1
	shift
	ipcalc.sh $* | awk  -F"=" '/^'$opt'=/{print $2}'
}

ifget() {
    opt=$1
    ifname=$2
    case "$opt" in
        ip)
            ifconfig $ifname | awk '/inet addr/{print substr($2,6)}'
            ;;
        mask)
            ifconfig $ifname | awk '/Mask:/{print substr($4,6)}'
            ;;
        mac|macnc)
            local mac=$(ifconfig $ifname | awk '/HWaddr/{print $5}')
            if [ $opt = "macnc" ]; then
                echo $mac |tr -d ":"
            else
                echo $mac
            fi
            ;;
    esac
}

isip() {
	local ip=$1
	local valid_check=$(echo $ip|awk -F. '$1<=255&&$2<=255&&$3<=255&&$4<=255{print "yes"}')
	if echo $ip|grep -E "^[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}$">/dev/null; then
		if [ ${valid_check:-no} == "yes" ]; then
			echo 1
		else
			echo 0
		fi
	else
		echo 0
	fi
}


# ret_type, skip_dev
is_ip_conflict() {
    local iplist
    local iplistall

    iplistall=$(ip -o -4 addr show)

    if [ "$skip_dev" = "" ]
    then
        iplist=`echo "$iplistall" | awk '{print $4}'`
        iflist=`echo "$iplistall" | awk '{print $2}'`
    else
        iplist=`echo "$iplistall" | grep -v $skip_dev | awk '{print $4}'`
        iflist=`echo "$iplistall" | grep -v $skip_dev | awk '{print $2}'`
    fi

    local input_ip=$1

    if [ "${2%.*}" != "$2" ]
    then
        local input_prefix=$(ipcalc PREFIX $input_ip $2)
    else
        local input_prefix=${1#*/}
        local input_ip=${1%%/*}
    fi

    local counter=0
    local conflict=0
    local conflict_if=""

    for item in $iplist
    do
        counter=$(($counter+1))

        local chk_prefix=${item#*/}
        local check_ip=${item%%/*}
        if [ $input_prefix -lt $chk_prefix ]
        then
            chk_prefix=$input_prefix
        fi
        network1=$(ipcalc NETWORK $input_ip/$chk_prefix)
        network2=$(ipcalc NETWORK $check_ip/$chk_prefix)
        if [ "$network1" = "$network2" ]
        then
            conflict=1
            conflict_if=$(echo "$iflist" |sed -n -e ${counter}p)
            # echo $1/$2 conflic with $item > /dev/console
            break
        fi
    done

    if [ "$ret_type" = "1" ]
    then
        if [ "${conflict_if}" != "" ]
        then
            # br-nat1:172.16.1.1/16
            echo $conflict_if:$item
        fi
    else
        echo $conflict
    fi
}

get_cpu_num() {
	local cpunum=`cat /proc/cpuinfo |grep ^processor |wc -l`
    echo ${cpunum:-1}
}

bat_cpu_adjust() {
    #Senao George, enhanced throuput about 50Mbps in 5G when bat0 use CPU3(4) and scan_1 use CPU4(8)
    BATList=`ls /sys/class/net/ | grep "^bat0"`
    
    [ $(get_cpu_num) -eq 4 ] || break

    echo ">>>>>[FIX]bat_cpu_adjust in senao-shell-libs: BATList:$BATList" > /dev/console
    
    for bat in $BATList
    do
        for rxqueue in `find /sys/class/net/$bat/queues/ -name "rx-*"`
        do
            echo 4 > $rxqueue/rps_cpus
        done
    done

}


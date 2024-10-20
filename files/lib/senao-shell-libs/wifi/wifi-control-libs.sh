#!/bin/sh

default_cfg80211=0

#green
gecho() {
    if [ "$__no_color" = "1" ]
    then
        echo "$*"
    else
        if [ "${1}" == "-n" ];then
            shift
            echo -e -n "\033[32m$*\033[0m"
        else
            echo -e "\033[32m$*\033[0m"
        fi
    fi
}

is_cfg80211() {
	local cfg80211=0
	if [ -f "/ini/global.ini" ]; then
		cfg80211=$(cat /ini/global.ini | grep cfg80211_config | awk -F "=" '{print $2}')
	fi
	echo "$cfg80211"
}

is_mac() {
    if [ "$1" != "" ]
    then
        for mac_str in $@
        do
            mac_chr=${mac_str//:/ }
            mac_chr=${mac_chr//-/ }
            mac_word=$(echo "$mac_chr" |wc -w || echo 0)
            if [ "$mac_chr" != "$mac_str" -a $mac_word -eq 6 ]
            then
                for mac_hex in $mac_chr
                do
                    mac_int=`printf %d 0x$mac_hex 2>/dev/null`
                    if [ $? -ne 0 ]
                    then
                        mac_int=-1
                    fi
                    if [ "${mac_int//[0-9]/}" = "" ]
                    then
                        if [ $mac_int -gt 255 -o $mac_int -lt 0 ]
                        then
                            echo 0
                            return
                        fi
                    else
                        echo 0
                        return
                    fi
                done
            else
                echo 0
                return
            fi
        done
        echo 1
    else
        echo 0
    fi
}

WIFI_IWPRIV(){
    local cmd="$@"
    shift
    local action=$1
    local is_get=0

    if  [ "${action:0:2}" = "g_" -o "${action:0:4}" = "get_" ]
    then
        is_get=1
    else
        is_get=0
    fi

    if [ $default_cfg80211 -eq 1 ]; then
        output=`/usr/sbin/cfg80211tool $cmd`
    else
        output=`/usr/sbin/iwpriv $cmd`
    fi

    if [ $? -ne 0 ]
    then
        return 1
    else
        if [ "$is_get" = "1" ]
        then
            # don't use "$output", just echo to remove tail spaces
            echo $output
        fi
        return 0
    fi
}


ifget() {
	local opt=$1
	local ifname=$2
	local default=$3

	case "$opt" in
		ip)
			local ipaddr=`ifconfig $ifname | awk '/inet addr/{print substr($2,6)}'`
			[ -z "${ipaddr}" ] && echo $default || echo $ipaddr
			;;
		mask)
			local mask=`ifconfig $ifname | awk '/Mask:/{print substr($4,6)}'`
			[ -z "${mask}" ] && echo $default || echo $mask
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

get_wifi_ifnames() {
    local ifnames
    local radio
    local ssidnum=$1

    local ifname
    for radio in 0 1 4
    do
        ifname=`uci -q get wireless.wifi${radio}_ssid_$ssidnum.ifname`
        if [ -d "/sys/class/net/$ifname" ]
        then
            ifnames="$ifnames${ifnames:+ }$ifname"
        fi
    done
    echo $ifnames
}

get_ifnames() {
    local input=$1
    local ifnames

    if [ "$input" != "" -a "${input/[0-9]/}" = "" ]
    then
        ifnames=$(get_wifi_ifnames $input)
    elif [ "$input" = "24g" ]
    then
        ifnames=`ls /sys/class/net/ |grep -w "^ath0\|^ath0."`
    elif [ "$input" = "5g" ]
    then
        ifnames=`ls /sys/class/net/ |grep -w "^ath1\|^ath1."`
    else
        if [ "$input" = "all" -o "$input" = "" ]
        then
            ifnames=`ls /sys/class/net/ |grep ^ath |grep -v "\."`
        else
            ifnames=$input
        fi
    fi
    echo $ifnames
}

get_ssid_num() {
	local input=$1
	if [ "${input%%-*}" = "br" ];then
		ssidnum=${input#br-ssid}
	else
		ssidnum=${input#ath[0-9]}
		ssidnum=$((${ssidnum:-0}+1))
	fi
	echo ${ssidnum:-1}
}

get_ifname_radio() {
	local ifname=$1
	local radio=${ifname#ath}
	case "${radio:0:1}" in
		0)
			echo 0
			;;
		1)
			echo 1
			;;
		4)
			echo 2
			;;
		*)
			echo 0
			;;
	esac
}

get_cpu_num()
{
	local cpunum=`cat /proc/cpuinfo |grep ^processor |wc -l`
	echo ${cpunum:-1}
}

get_hostapd_phy_path() {
	# use first phy path
	local phy_path=$(ls /var/run/hostapd-*/$1 2>/dev/null |head -1)
	if [ "$phy_path" != "" ];then
		echo $(dirname $phy_path)/
	else
		echo "/var/run/hostapd-phy0/"
	fi
}

get_hostapd_ctrl_interface() {
	local input=$1
	local ctrl_interface

	ctrl_interface=$(cat /var/run/hostapd-${input}.conf | grep ^ctrl_interface=)
	ctrl_interface=${ctrl_interface##*=}

	echo $ctrl_interface
}

check_hostapd_all_interfaces()
{
    local ifname=""
    local ctrl_ifname=""
    echo "Check hostapd all interface status"
    hostapd_conf_files=$(ls /var/run/hostapd-*.conf)
    for hostapd_conf_file in $hostapd_conf_files
    do
	ifname=${hostapd_conf_file##*-}
	ifname=${ifname%.*}
	ctrl_ifname=$(cat $hostapd_conf_file | grep ^ctrl_interface=)
	ctrl_ifname=${ctrl_ifname##*=}
	if [ "$ifname" != "" ] && [ "$ctrl_ifname" != "" ]; then
	    response=$(hostapd_cli -p $ctrl_ifname -i $ifname ping)
	    state=$(hostapd_cli -p $ctrl_ifname -i $ifname status | grep ^state=)
	    state=${state##*=}
	    if [ "$response" == "" ] || [ "$state" == "DISABLED" ]; then
		wpa_cli -g /var/run/hostapd/global raw REMOVE $ifname
                wpa_cli -g /var/run/hostapd/global raw ADD bss_config=$ifname:$hostapd_conf_file
	    fi
	fi
    done
}

check_hostapd_interface()
{
    echo "Check hostapd interface status [$ifnames]"
    for ifname in $ifnames
    do
	if [ -e /var/run/hostapd/global ]; then
	    hostapd_conf_file="/var/run/hostapd-""$ifname"".conf"
	    ctrl_ifname=$(cat $hostapd_conf_file | grep ^ctrl_interface=)
	    ctrl_ifname=${ctrl_ifname##*=}
	    if [ "$ifname" != "" ] && [ "$ctrl_ifname" != "" ]; then
		response=$(hostapd_cli -p $ctrl_ifname -i $ifname ping)
		state=$(hostapd_cli -p $ctrl_ifname -i $ifname status | grep ^state=)
		state=${state##*=}
		if [ "$response" == "" ] || [ "$state" == "DISABLED" ]; then
		    wpa_cli -g /var/run/hostapd/global raw REMOVE $ifname
		    wpa_cli -g /var/run/hostapd/global raw ADD bss_config=$ifname:/var/run/hostapd-$ifname.conf
		fi
	    fi
	fi
    done
}

control_hostapd_reload()
{
    local ifnames
    local input=$*
    local hostapd_conf_file
    local interface=""

    ifnames=$(get_ifnames $input)

    if [ "$input" = "all" -o "$input" = "" ]
    then
        hostapd_conf_files=$(ls /var/run/hostapd-*.conf)
        echo "Reload hostapd config [$hostapd_conf_files]"
        for hostapd_conf_file in $hostapd_conf_files
        do
            ifname=${hostapd_conf_file##*-}
            ifname=${ifname%.*}
            if [ "$ifname" != "" ]
            then
                wpa_cli -g /var/run/hostapd/global raw REMOVE $ifname
                wpa_cli -g /var/run/hostapd/global raw ADD bss_config=$ifname:$hostapd_conf_file
            else
                echo "error to reload $hostapd_conf_file"
            fi
        done
        #Check if the interface is successfully added to hostapd.
        check_hostapd_all_interfaces
    else
        echo "Reload hostapd config [$ifnames]"
        for ifname in $ifnames
        do
            if [ -e /var/run/hostapd/global ]
            then
                wpa_cli -g /var/run/hostapd/global raw REMOVE $ifname
                wpa_cli -g /var/run/hostapd/global raw ADD bss_config=$ifname:/var/run/hostapd-$ifname.conf
            fi
        done
        #Check if the interface is successfully added to hostapd.
        check_hostapd_interface "$ifnames"
    fi
}

control_hostapd_reload_wpa_psk()
{
    local ifnames
    local input=$*

    if [ "$input" = "all" -o "$input" = "" ]
    then
        hostapd_conf_files=$(ls /var/run/hostapd-*.conf)
        echo "hostapd config [$hostapd_conf_files]"
        for hostapd_conf_file in $hostapd_conf_files
        do
            ath_name=${hostapd_conf_file##*-}
            ath_name=${ath_name%.*}
            if [ "$ath_name" != "" ]
            then
                ifnames="$ifnames $ath_name"
            else
                echo "error to reload $hostapd_conf_file"
            fi
        done
    else
        ifnames=$(get_ifnames $input)
    fi

    # echo "hostapd [$ifnames]"
    for ifname in $ifnames
    do
        phy_path=$(get_hostapd_phy_path $ifname)
        if [ -e "${phy_path}$ifname" ]
        then
            hostapd_cli -p $phy_path -i $ifname reload_wpa_psk
        fi
    done
}


control_hostapd_acl()
{
    local ifnames
    local policy=$1
    shift
    local action=$1
    shift
    local hostapd_conf_file
    local hostapd_ctrl_interface
    local mac=""
    # to fix the client removed from accept table, hostapd not kick client
    local do_kick_client=0
    # [acl] [deny/accept] [show/add/del/clear] [Interface/SSID num(1-8)/all (all of athx)]

    case "$policy" in
        "deny")
            policy=DENY_ACL
            ;;
        "accept")
            policy=ACCEPT_ACL
            ;;
        *)
            ;;
    esac


    case "$action" in
        show)
            action=SHOW
            ;;
        add|del)
            if [ "$action" = "add" ]
            then
                action=ADD_MAC
            elif [ "$action" = "del" ]
            then
                action=DEL_MAC
                if [ "$policy" = "ACCEPT_ACL" ]
                then
                    do_kick_client=1
                fi
            else
                usage
                exit 1
            fi

            if [ "$__input_file" != "" ]
            then
                for line in `cat $__input_file`
                do
                    mac="$mac $line"
                done
            fi

            while [ "$1" != "" ]
            do
                if [ "$(is_mac $1)" = "1" ]
                then
                    mac="$mac $1"
                    shift
                else
                    break
                fi
            done
            ;;
        clear)
            action=CLEAR
            ;;
        *)
            ;;
    esac

    local input=$*

    if [ "$input" = "all" -o "$input" = "" ]
    then
        hostapd_conf_files=$(ls /var/run/hostapd-*.conf)
        echo "hostapd config [$hostapd_conf_files]"
        for hostapd_conf_file in $hostapd_conf_files
        do
            ath_name=${hostapd_conf_file##*-}
            ath_name=${ath_name%.*}
            if [ "$ath_name" != "" ]
            then
                ifnames="$ifnames $ath_name"
            else
                echo "error to reload $hostapd_conf_file"
            fi
        done
    else
        ifnames=$(get_ifnames $input)
    fi

    local reason
    [ -e "/etc/config/ezmcloud" ] && reason="Remove from whitlist"

    # echo "hostapd [$ifnames]"
    for ifname in $ifnames
    do
        hostapd_ctrl_interface=$(get_hostapd_ctrl_interface $ifname)
        if [ -e ${hostapd_ctrl_interface}/$ifname ]
        then
            if [ "$mac" = "" ]
            then
                hostapd_cli -p $hostapd_ctrl_interface -i $ifname $policy $action
            else
                for per_mac in $mac
                do
                    hostapd_cli -p $hostapd_ctrl_interface -i $ifname $policy $action $per_mac
                    if [ "$do_kick_client" = "1" ]
                    then
                        /sbin/kicksta.sh $reason 1 $per_mac $ifname 4 &
                    fi
                done
            fi
        fi
    done
}

control_hostapd_sta()
{
    local ifnames
    local action=$1
    shift

    if [ "$(is_mac $1)" = "1" ]
    then
        local input=$*
        local ifnames="all"
    else
        local ifnames=$1
        shift
        local input=$*
    fi

    ifnames=$(get_ifnames $ifnames)

    case "$action" in
        list_sta)
            for ifname in $ifnames
            do
                phy_path=$(get_hostapd_phy_path $ifname)
                if [ -e "${phy_path}$ifname" ]
                then
                    hostapd_cli -p $phy_path -i $ifname list_sta
                fi
            done
            ;;
        check_sta)
            if [ -z "$input" ]
            then
                exit 1
            fi
            for ifname in $ifnames
            do
                phy_path=$(get_hostapd_phy_path $ifname)
                if [ -e "${phy_path}$ifname" ]
                then
                    local cmd=`hostapd_cli -p $phy_path -i $ifname sta $input`
                    if [ "$cmd" != "FAIL" -a "$cmd" != "" ]
                    then
                        echo $ifname
                        # exit sucess if found
                        exit 0
                    fi
                fi
            done
            # exit error if not found
            exit 1
            ;;
    esac

}

control_beacon_handle()
{
    local ifnames
    local action=$1
    shift
    local input=$*

    default_cfg80211=$(is_cfg80211)

    ifnames=$(get_ifnames $input)

    for ifname in $ifnames
    do
        local get_hide_ssid=`WIFI_IWPRIV $ifname get_hide_ssid`
        local get_dynamicbeacon=`WIFI_IWPRIV $ifname g_dynamicbeacon`
        local hide_ssid=${get_hide_ssid#*:}
        local dynamicbeacon=${get_dynamicbeacon#*:}
        # ENABLED, DISABLED
        local phy_path=$(get_hostapd_phy_path $ifname)
        local ifstate=$(hostapd_cli -p $phy_path -i $ifname stat |awk -F "=" '/^state=/{print $2}')
        if [ -n "$dynamicbeacon" ]
        then
            local use_nobeacon=0
        else
            local get_nobeacon=`WIFI_IWPRIV $ifname get_nobeacon`
            local nobeacon=${get_nobeacon#*:}
            local use_nobeacon=1
        fi

        if [ -n "$hide_ssid" -a -n "$dynamicbeacon" ] || [ $use_nobeacon -eq 1 -a -n "$nobeacon" ]
        then
            case $action in
                "on")
                    gecho "[ssid=$(get_ssid_num $ifname), vap=$ifname, state=$ifstate]"

                    # Should ath interface enable before dynamicbeacon
                    if [ "$ifstate" != "ENABLED" ]
                    then
                        local enable_state=$(hostapd_cli -p $phy_path -i $ifname enable)
                        if [ "$enable_state" = "FAIL" ]
                        then
                            control_hostapd_reload $ifname
                        fi
                    fi
                    get_hide_ssid=`WIFI_IWPRIV $ifname get_hide_ssid`
                    hide_ssid=${get_hide_ssid#*:}

                    if [ "$use_nobeacon" = "1" ]
                    then
                        [ "$nobeacon" != "0" ] && WIFI_IWPRIV $ifname nobeacon 0
                    else
                        section=$(/usr/sbin/foreach wireless wifi-iface ifname $ifname)
                        conf_hide_ssid=$(uci -q get wireless.$section.hidden)
                        if [ "$dynamicbeacon" != "0" ]
                        then
                            [ "$hide_ssid" != "1" ] && {
                                WIFI_IWPRIV $ifname hide_ssid 1
                                hide_ssid=1
                            }
                            WIFI_IWPRIV $ifname dynamicbeacon 0
                        fi
                    fi
                    [ "$hide_ssid" != "$conf_hide_ssid" ] && WIFI_IWPRIV $ifname hide_ssid ${conf_hide_ssid:-0}
                    ;;
                "off")
                    gecho "[ssid=$(get_ssid_num $ifname), vap=$ifname, state=$ifstate]"
                    if [ "$use_nobeacon" = "1" ]
                    then
                        [ "$nobeacon" != "1" ] && WIFI_IWPRIV $ifname nobeacon 1
                    else
                        [ "$hide_ssid" != "1" ] && WIFI_IWPRIV $ifname hide_ssid 1
                        [ "$dynamicbeacon" != "1" ] && WIFI_IWPRIV $ifname dynamicbeacon 1
                    fi
                    if [ "$ifstate" != "DISABLED" ]
                    then
                        hostapd_cli -p $phy_path -i $ifname disable
                    fi
                    ;;
                "show")
                    gecho "[ssid=$(get_ssid_num $ifname), vap=$ifname, state=$ifstate]"
                    if [ "$use_nobeacon" = "1" ]
                    then
                        WIFI_IWPRIV $ifname get_nobeacon
                    else
                        WIFI_IWPRIV $ifname get_hide_ssid
                        WIFI_IWPRIV $ifname g_dynamicbeacon
                    fi
                    ;;
                "fix")
                    if [ "$dynamicbeacon" = "1" ]
                    then
                        gecho "[fix $ifname hide_ssid to 1]"
                        [ "$hide_ssid" != "1" ] && WIFI_IWPRIV $ifname hide_ssid 1
                    elif [ "$dynamicbeacon" = "0" ]
                    then
                        section=$(/usr/sbin/foreach wireless wifi-iface ifname $ifname)
                        conf_hide_ssid=$(uci -q get wireless.$section.hidden)
                        gecho "[fix $ifname hide_ssid to ${conf_hide_ssid:-0}]"
                        [ "$hide_ssid" != "$conf_hide_ssid" ] && WIFI_IWPRIV $ifname hide_ssid ${conf_hide_ssid:-0}
                    fi
                    ;;
                *)
                    exit 1
            esac
        fi
    done

    return 0
}

control_client_list()
{
    local ifnames
    local action=$1
    shift
    local input=$*

    ifnames=$(get_ifnames $input)

    for ifname in $ifnames
    do
        ssid=$(iwconfig $ifname |grep ESSID |awk -F":" '{print $2}')
        gecho "[ssid_num=$(get_ssid_num $ifname), vap=$ifname, ssid_name=$ssid]"
        wlanconfig $ifname list sta
        # phy_path=$(get_hostapd_phy_path $ifname)
        # hostapd_cli -p /var/run/hostapd-phy0/ -i $ifname list_sta
    done
}

control_client_count()
{
    local ifnames
    local action=$1
    shift
    local input=$*

    ifnames=$(get_ifnames $input)

    local all_client_count=0
    local phy_path
    local client_count

    for ifname in $ifnames
    do
        phy_path=$(get_hostapd_phy_path $ifname)
        client_count=$(hostapd_cli -p $phy_path -i $ifname list_sta |wc -l)
        all_client_count=$(($all_client_count+${client_count:-0}))
    done
    echo $all_client_count
}

show_cur_chan()
{
    local radio=$1

    ifname=$(grep -l wifi$radio /sys/class/net/ath*/parent |cut -d '/' -f 5 |head -n 1)
    iwlist $ifname chan |grep Current |awk '{printf $2}'
}

show_chan_util()
{
    local radio=$1
    local ifname=$2
    local channel=$3

    if [ "$channel" == "current" ]
    then
        WIFI_IWPRIV $ifname get_chutil | awk -F ":" '{printf $2}'
    else
        iwpriv $ifname get_sn_acsutil | awk -F ":" '{print $2}'
    fi
}

show_chan_util_nwifi()
{
    local radio=$1
    local ifname=$2
    local channel=$3
    default_cfg80211=$(is_cfg80211)

    if [ "$channel" == "current" ]
    then

	if [ $default_cfg80211 -eq 1 ]
	then
	    WIFI_IWPRIV wifi$radio g_ch_util_nwifi | awk -F ":" '{printf $2}'
	else
	    again=$(iwpriv $ifname g_sn_nwifiutil | awk -F ":" '{printf $2}')
	    if [ "$again" == "255" ]
	    then
		iwpriv $ifname g_sn_nwifiutil | awk -F ":" '{print $2}'
	    fi
	fi
    else
	iwpriv $ifname g_sn_acsnwifi | awk -F ":" '{print $2}'
    fi
}

control_client_kick()
{
    local ifnames
    shift
    local client=$1
    shift
    local reason=$2
    local phy_path

    ifnames=`ls /sys/class/net/ |grep ^ath`
    for ifname in $ifnames
    do
        phy_path=$(get_hostapd_phy_path $ifname)
        wifi_clients=$(hostapd_cli -p $phy_path -i $ifname list_sta)
        for wifi_client in $wifi_clients
        do
            if [ "$wifi_client" = "$client" ]
            then
                gecho "[ssid=$(get_ssid_num $ifname), vap=$ifname]"
                /sbin/kicksta.sh ${reason:+\"$reason\"} 1 $client $ifname 4 &
            fi
        done
    done
}


control_ssid_show()
{
    local ifnames
    local action=$1
    shift
    local input=$*

    ifnames=$(get_ifnames $input)

    for ifname in $ifnames
    do
        gecho "[ssid=$(get_ssid_num $ifname), vap=$ifname]"
        case "$opt" in
            "essid")
                local essid=`iwconfig $ifname |grep  ESSID:`
                echo ${essid#*:}
                ;;
            "freq")
                iwconfig $ifname |awk '/Frequency/{print substr($2,11)" "$3}'
                ;;
            *)
                local essid=`iwconfig $ifname |grep  ESSID:`
                echo ${essid#*:}
                ;;
        esac
    done
}


control_bssid_show()
{
    local ifnames
    local action=$1
    shift
    local input=$*

    ifnames=$(get_ifnames $input)

    local all_bssid=""
    local per_bssid

    for ifname in $ifnames
    do
        per_bssid=$(cat /sys/class/net/$ifname/address)
        if [ "$per_bssid" != "" ]
        then
            all_bssid=$per_bssid${all_bssid:+,}$all_bssid
        fi
    done
    echo $all_bssid
}

change_htmode()
{
    local ifnames
    local radio=$1
    local iface=$2
    local ht_mode=$3
    shift

    ifnames=$(grep -l wifi /sys/class/net/*/parent |cut -d '/' -f 5)
    hwmode=$(uci -q get wireless.wifi$radio.hwmode)

    for ifname in $ifnames
    do
        ifconfig $ifname down
    done
    if [ "$ht_mode" == "ht20" ]
    then
	if [ "$radio" == 0 ]
	then
	    if [ "$hwmode" == "11na" ]
	    then
		iwpriv $iface mode 11NAHT20
	    else
		iwpriv $iface mode 11NGHT20
	    fi
	else
	    iwpriv $iface mode 11ACVHT20
	    if [ "$hwmode" == "11ac" ]
	    then
		iwpriv $iface mode 11ACVHT20
	    elif [ "$hwmode" == "11axa" ]
	    then
		iwpriv $iface mode 11AHE20
	    else #axg
		iwpriv $iface mode 11GHE20
	    fi
	fi
        iwpriv $iface chwidth 0
    else
	if [ "$hwmode" == "11na" ]
	then
	    iwpriv $iface mode 11NAHT40
	    iwpriv $iface chwidth 1
	    iwpriv $iface disablecoext 1
	else
	    iwpriv $iface mode 11NGHT40
	    iwpriv $iface chwidth 1
	    iwpriv $iface disablecoext 1
	fi
    fi
    for ifname in $ifnames
    do
        ifconfig $ifname up
    done
}

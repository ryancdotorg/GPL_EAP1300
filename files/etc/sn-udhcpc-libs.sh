#!/bin/sh

support_ssid_profile=$(uci -q get functionlist.functionlist.SUPPORT_SSID_PROFILE || echo 0)
wifi_en_iface=`foreach wireless wifi-iface disabled 0`

get_wireless_option() {
    local ssid_name=$1
    local option_name=$2
    local equalvalue=$3
    local section_name

    for section_name in `echo "$wifi_en_iface"|grep $ssid_name`
    do
        ret=`uci -q get wireless.$section_name.$option_name`
        if [ "$equalvalue" != "" ]
        then
            if [ "$ret" = "$equalvalue" ]
            then
                ret=$equalvalue
            fi
        fi
        # Just got the fist one.
        break
    done
    echo "$ret"
}

get_uniq_sort_string() {
    local input="$@"
    echo $(echo -e "${input// /\\n}" | sort -u)
}

get_gnmode_profile() {
	local mode=$1
	local section_name=$(foreach wireless wifi-iface guest_network $mode)
	if [ -n "$section_name" ];then
		for name in $section_name
		do
			if [ "$(uci -q get wireless.$name.disabled)" != "1" ]
			then
				local ret=$(get_section_id $name)
				echo ssid$ret
			fi
		done
	fi
}

get_ethernet_profile() {
    local mode=$1

    if [ "$mode" = "NAT" ]
    then
        local check_ip_assign=1
        local check_portal=1
    elif [ "$mode" = "Bridge" ]
    then
        local check_ip_assign=0
        local check_portal=1
    elif [ "$mode" = "Disable" ]
    then
        local check_ip_assign=0
        local check_portal=0
    elif [ "$mode" = "NAT_only" ]
    then
        local check_ip_assign=1
        local check_portal=0
    fi

    if [ "$support_ssid_profile" = "1" ]
    then
        local section=$(foreach ethprofile profile enable 1)
        if [ -n "$section" ];then
            for name in $section
            do
                local is_portal_en=$(uci -q get portal.$name.enable)
                local is_nat_mode=$(uci -q get ethprofile.$name.client_ip_assignment)
                # NAT or Bridge
                if [ $check_portal -eq 1 -a "$is_portal_en" = "1" ]
                then
                    if [ "$check_ip_assign" = "$is_nat_mode" ]
                    then
                        local ret=$(get_section_id $name)
                        echo port$ret
                    fi
                    # NAT_only
                elif [ $check_portal -eq 0 -a "$is_nat_mode" = "1" ]
                then
                    local ret=$(get_section_id $name)
                    echo port$ret
                fi
            done
        fi
    fi
}

get_ldap_if() {
	local section_name=$(foreach wireless wifi-iface auth_server 127.0.0.1)
	if [ -n "$section_name" ];then
		for name in $section_name
		do
			if [ "$(uci -q get wireless.$name.disabled)" != "1" ]
			then
				local encryption=$(uci -q get wireless.$name.encryption)
				if [ -n "$encryption" -a "${encryption//wpa/}" != "$encryption" ]
				then
					echo $(uci -q get wireless.$name.ifname)
				fi
			fi
		done
	fi
}

get_banned_msg_if() {
#banned_msg_en 0 => disable
#              1 => enable banned message
#              2 => enable block random mac
#              3 => enable both
    local section_name=$(foreach wireless wifi-iface banned_msg_en 1)
    if [ -n "$section_name" ];then
        for name in $section_name
        do
            combined_section=${name#*_}
            if [ "$(uci -q get wireless.$name.disabled)" != "1" ] && [ -s "/etc/ezmcloud/config/l2acl-${combined_section}.maclist" ]
            then
                echo $(uci -q get wireless.$name.ifname)
            fi
        done
    fi

    local section_name=$(foreach wireless wifi-iface banned_msg_en 2)
    if [ -n "$section_name" ];then
        for name in $section_name
        do
            combined_section=${name#*_}
            if [ "$(uci -q get wireless.$name.disabled)" != "1" ]
            then
                echo $(uci -q get wireless.$name.ifname)
            fi
        done
    fi

    local section_name=$(foreach wireless wifi-iface banned_msg_en 3)
    if [ -n "$section_name" ];then
        for name in $section_name
        do
            combined_section=${name#*_}
            if [ "$(uci -q get wireless.$name.disabled)" != "1" ]
            then
                echo $(uci -q get wireless.$name.ifname)
            fi
        done
    fi
}


# 0: mvlan disabled, no vlan
# 1: mvlan disabled, vlan enabled
# 2: mvlan enabled, and no vlan
# 3: mvlan enabled, vlan enabled, and the same
# 4: mvlan enabled, vlan enabled, and different

get_vlan_mode() {
    local vlantag=${1:-0}

    local is_mgmvlanEnable=$(uci -q get network.sys.WLANVLANEnable)
    if [ "$is_mgmvlanEnable" = "1" ]
    then
        local mgmvlanID=$(uci -q get network.sys.ManagementVLANID)
    fi

    if [ "$is_mgmvlanEnable" = "1" ]
    then
        if [ "${mgmvlanID:-0}" = "$vlantag" ]
        then
            echo 3
        else
            if [ $vlantag -ne 0 ]
            then
                echo 4
            else
                echo 2
            fi
        fi
    elif [ $vlantag -ne 0 ]
    then
        echo 1
    else
        echo 0
    fi
}

get_vlan_wan_bridge() {
    local vlantag=$1
    local vlanmode=$(get_vlan_mode $vlantag)
    local bridge_name=""

    case "$vlanmode" in
        0)
            bridge_name="br-lan"
            ;;
        1)
            #vlan
            bridge_name="br-vlan$vlantag"
            ;;
        2)
            bridge_name="br-99"
            ;;
        3)
            #vlan
            bridge_name="br-lan"
            ;;
        4)
            #vlan
            bridge_name="br-vlan$vlantag"
            ;;
    esac
    echo $bridge_name

    # protect check
    if [ ! -d "/sys/class/net/$bridge_name/" ]
    then
        echo "FIXME: $bridge_name not exised!" > /dev/console
    fi
    local vlan_ifnames=$(get_vlan_wan_ifname $vlantag)
    for vlanif in $vlan_ifnames
    do
        if [ ! -d "/sys/class/net/$bridge_name/brif/$vlanif" ]
        then
            echo "FIXME: $vlanif not in $bridge_name!" > /dev/console
        fi
    done
}

strip_ethprofile_interface() {
	# appends space to let strip correctly
	local ifnames=$@" "

	if [ "$support_ssid_profile" = "1" ]; then
		for ethprofile_section in $(foreach ethprofile profile)
		do
			local ethprofile_ifnames="$(uci -q get ethprofile.$ethprofile_section.ifname)"
			ifnames=${ifnames//$ethprofile_ifnames /}
		done
	fi

	# echo without " will ignore the last space
	echo $ifnames
}

get_vlan_wan_ifname() {
	local vlantag=$1

	local lan_ifnames=$(uci -q get /rom/etc/config/network.lan.ifname)

	if [ "$support_ssid_profile" = "1" ]; then
		lan_ifnames=$(strip_ethprofile_interface $lan_ifnames)
	fi

	if [ ${vlantag:-0} -eq 0 ];then
		local eth_vlanif=`echo $lan_ifnames | awk '{print $1}'`
	else
		local eth_vlanif=`echo $lan_ifnames | awk '{print $1}'`.${vlantag}
	fi

	echo $eth_vlanif
}

add_to_udhcpc_list() {

    local vlantag=${1:-0}
    local vlanbridge=$(get_vlan_wan_bridge $vlantag)

    if [ "$vlanbridge" != "br-lan" ]
    then
        test -f /var/run/udhcpc-$vlanbridge.pid || udhcpc_list=$(get_uniq_sort_string $udhcpc_list $vlanbridge)
    fi
}

ipcalc() {
	local opt=$1
	shift
	ipcalc.sh $* | awk  -F"=" '/^'$opt'=/{print $2}'
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

get_section_id() {
	local config_name=$1
	section_id=${config_name##*_}
	section_id=${section_id//[a-z_-]/}
	echo ${section_id}
}

get_profile_index() {
	local config_name=$1
	local index=${config_name//[a-z_-]/}
	if [ "$config_name" != "${config_name/ssid/}" ]
	then
		profile_index=$index
	else
		profile_index=$(($index+8))
	fi

	echo $profile_index
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

ip_rule_del_pref() {
    local prefnum=$1
    ip route flush table $prefnum
    while ip rule del pref $prefnum 2>/dev/null;do true;done
}

is_ip_conflict() {
	local iplist

	if [ "$skip_dev" = "" ]
	then
		iplist=`ip -o -4 addr show | awk '{print $4}'`
	else
		iplist=`ip -o -4 addr show | grep -v $skip_dev | awk '{print $4}'`
	fi

	local input_ip=$1
	local ret=0

	if [ "${2%.*}" != "$2" ]
	then
		local input_prefix=$(ipcalc PREFIX $input_ip $2)
	else
		local input_prefix=${1#*/}
		local input_ip=${1%%/*}
	fi

	for item in $iplist
	do
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
			ret=1
			echo CP: $1/$2 conflic with $item > /dev/console
			break
		fi
	done

	echo $ret
}

replace_config() {
local var=$2
local new_val=$3

sed -i -e "s/^$var=.*$/$var=$new_val/" "$1"
}

check_portal_config_vlan() {
    local configfile=""

    if [ "$1" = "0" ]
    then
        local checkstring='^HS_WANIF=br-99$'
    else
        local checkstring='^HS_VLAN='$1'$'
    fi

    for i in `ls /tmp/etc/chilli/`
    do
        if [ "`cat /tmp/etc/chilli/$i/config |grep "$checkstring"`" != "" ]
        then
            configfile="$configfile /tmp/etc/chilli/$i/config"
        fi
    done
    echo $configfile
}

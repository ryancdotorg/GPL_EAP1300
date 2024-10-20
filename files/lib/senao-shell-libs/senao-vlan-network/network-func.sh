. /lib/senao-shell-libs/network.sh

remove_empty_bridges()
{
	for brname in $(ls /sys/class/net/ |grep ^br-)
	do
		local brifname=$(get_bridge_name $brname || echo "x")
		if [ -z "$brifname" ]
		then
			ifconfig $brname down
			brctl delbr $brname
		fi
	done
}

append_eth_vid()
{
	for eif in $lan_ifnames
	do
		is_find_profile_eth=0
		# profile eth no need to create ethx.xxx
		if [ "$is_support_ssid_profile" = "1" ]; then
			for ethprofile_config in $(foreach ethprofile profile); do
				if [ "$(uci -q get ethprofile.$ethprofile_config.ifname)" = "$eif" ]; then
					is_find_profile_eth=1
					break
				fi
			done
		fi
		if [ "$is_find_profile_eth" = "1" ]; then
			continue
		fi

		if [ -n "${1}" ]
		then
			eth_vlanifs="${eth_vlanifs} ${eif}.${1}"
		fi
	done
	echo ${eth_vlanifs:-$lan_ifnames}
}

clear_network_configs()
{
	# clear all of network.ifname
	local network_sections=$(echo "`uci show network|grep ifname|cut -d "." -f 2`")

	for section in $network_sections
	do
		if [ "${section//[0-9]/}" = "" ]
		then
			uci set network.${section}.ifname="na"
		fi
		if [ "$is_support_ssid_profile" = "1" ]; then
			if [ "${section/nat*}" = "" ]
			then
				uci set network.${section}.ifname="na"
			fi
			if [ "${section/port*}" = "" ]
			then
				uci set network.${section}.ifname="na"
			fi
			if [ "${section/ssid[0-9]/}" = "" ]
			then
				uci set network.${section}.ifname="na"
			fi
		fi
		if [ "${section/*vlan}" = "" -o "${section/vlan*}" = "" ]
		then
			uci delete network.$section
		fi
	done

	local dhcp_sections=$(foreach dhcp dhcp ignore 0)

	for section in $dhcp_sections
	do
		if [ "${section/nat}" != "$section" ]
		then
			uci set dhcp.$section.ignore='1'
		fi
	done
}

clear_wireless_configs()
{
	# don't set network for mesh, it will call 'ubus call network.interface.lan add_device '{ "name": "ath35" }'
	# it leads ubus into wrong status
	uci -q delete wireless.wifi0_mesh.network
	uci -q delete wireless.wifi1_mesh.network
	uci -q delete wireless.wifi2_mesh.network
	uci commit wireless
}

add_network_config()
{
	local section=$1
	local ifname=$2
	local ip=$3
	local mask=$4

	uci -q get network.$section || uci set network.$section=interface
	uci set network.$section.type='bridge'
	uci set network.$section.ifname="$ifname"
	uci set network.$section.force_link='1'
	uci set network.$section.defaultroute='0'
	if [ -n "$ip" -a -n "$mask" ]
	then
		uci set network.$section.proto='static'
		[ -z "$(uci get network.$section.ipaddr)" ] && uci set network.$section.ipaddr="$ip"
		[ -z "$(uci get network.$section.netmask)" ] && uci set network.$section.netmask="$mask"
		uci set network.$section.peerdns='0'
		uci set network.$section.stp='0'
		uci set network.$section.gateway='0.0.0.0'
		uci set network.$section.dns='0.0.0.0 0.0.0.0'

		uci -q get dhcp.$section || uci set dhcp.$1=dhcp
		uci set dhcp.$section.interface="$section"
		[ -z "$(uci get dhcp.$section.leasetime)" ] && uci set dhcp.$section.leasetime='5m'
		# default 1000
		uci set dhcp.@dnsmasq[0].dhcpleasemax='65536'

		local host_ip=$(ipcalc NETWORK $ip $mask)
		local start_ip=${host_ip%.*}.$((${host_ip##*.}+1))
		[ -z "$(uci get dhcp.$section.start)" ] && uci set dhcp.$section.start="$start_ip"

		local bcast_ip=$(ipcalc BROADCAST $ip $mask)
		local end_ip=${bcast_ip%.*}.$((${bcast_ip##*.}-1))

		local ip_prefix=$(ipcalc PREFIX $ip $mask)
		local limit=$(echo $((1<<$((32-${ip_prefix})))))

		[ -z "$(uci get dhcp.$section.limit)" ] && uci set dhcp.$section.limit="$(($limit-2))"
		uci set dhcp.$section.ignore='0'
		uci set dhcp.$section.force='1'
		uci commit dhcp
	else
		uci set network.$section.proto='none'
	fi
}

guest_network_vlan_settings()
{
	local tablename="br-vlanguest"
	local is_guestvlan_en=$(uci -q get network.sys.GuestVLANEnable)
	local guestvlantag=$(uci -q get network.sys.GuestVLANID)
	local brname="vlan$guestvlantag"

	# clear table
	ip route flush table $tablename 2>/dev/null
	while ip rule delete table $tablename 2>/dev/null;do true;done

	if [ "${is_guestvlan_en:-0}" = "1" -a "${guestvlantag:-0}" != "0" ]
	then
		local eth_vlanif=$(append_eth_vid $guestvlantag)
		add_network_config $brname "$eth_vlanif"
	fi
	uci commit network
}

get_wifi_configs()
{
	local device=$1
	local wifi_mode=$(uci -q get wireless.${device}.opmode)
	local configs
	local enabled_configs

	if [ "$wifi_mode" = "ap" -o "$wifi_mode" = "wds_ap" ]
	then
		if [ "$is_enjet_on" = "1" ]
		then
			configs="${device}_enjet"
		else
			configs=$(foreach wireless wifi-iface mode_display $wifi_mode|grep "^${device}_" |grep -v '_scan$\|_guest$\|_lsp$\|_app$\|_mgmt$\|_enjet$\|_sta$')
		fi

		for option in $configs
		do
			local disabled=$(uci -q get wireless.${option}.disabled)
			if [ "${disabled:-1}" != "1" ]
			then
				enabled_configs="${option:+$enabled_configs $option}"
			fi
		done
	fi

	echo "$enabled_configs"
}

get_bridge_prefix()
{
	local device=$1
	local prefix=ssid

	if [ "$is_enjet_on" = "1" ]
	then
		local wifi_mode="enjet"
	else
		local wifi_mode=$(uci -q get wireless.${device}.opmode)
	fi

	case $wifi_mode in
		ap)
			prefix=nat
		;;
		wds_ap)
			prefix=wds
		;;
		enjet)
			prefix=enjet
		;;
	esac
	echo $prefix
}

nat_type_check() {
	local guest_network=$1
	local iface_ssid=$2
	local option=$3
	if  [ "${guest_network}" = "NAT_only" ]
	then
		local ssid_num=$(get_ssid_num $iface_ssid)
		add_network_config $iface_ssid "na" 172.$((15+$ssid_num)).1.1 255.255.0.0
		uci set wireless.$option.network="$iface_ssid"
	elif [ "${captived:-0}" = "1" ] && [ "${portal_en:-0}" = "1" ]; then
		if [ "${guest_network}" = "NAT" ] #|| [ "${guest_network}" = "Bridge" ]
		then
			add_network_config $iface_ssid "na"
			uci set wireless.$option.network="$iface_ssid"
		else
			# Enable
			uci set wireless.$option.network="guest"
		fi
	else
		uci set wireless.$option.network="guest"
	fi
}

isolation_network_options()
{
	local device="$1"
	local mpsk_no_vlan=$(uci -q get functionlist.functionlist.MPSK_NOT_SUPPORT_VLAN)

	eval wifi_configs_$device=\"$(get_wifi_configs $device)\"
	eval "wifi_configs=$(echo \${wifi_configs_$device})"

	bridge_prefix=$(get_bridge_prefix $device)

	# It's necessary to scan all wifi configs for new settings.
	for option in $wifi_configs; do
		local isolation=$(uci -q get wireless.${option}.isolation)
		local vlanID=$(uci -q get wireless.${option}.vlan_id)
		local guest_network=$(uci -q get wireless.${option}.guest_network)
		local wireless_ifname=$(uci -q get wireless.${option}.ifname)
		local multi_group_key=$(uci -q get wireless.${option}.multi_group_key)

		# ssid_bridge_type_type (guet_network)
		# 0 : no new bridge, gn=[Disable] or gn=[Bridge]
		# 1 : new bridge, br-ssidX or br-natX, gn=[NAT]  or gn=[NAT_only]
		# 2 : multi_group_key enable gn=[Disable] and [multi_group_key] = 1
		if [ "${guest_network:-Disable}" = "Disable" ]
		then
			if [ "${multi_group_key}" = "1" -a "$mpsk_no_vlan" != "1" ]
			then
				local ssid_bridge_type=2
			else
				local ssid_bridge_type=0
			fi
		elif [ "${guest_network:-Disable}" = "Bridge" ]
		then
			local ssid_bridge_type=0
		else # NAT or NAT_only
			local ssid_bridge_type=1
		fi

		local iface_ssid=${bridge_prefix}$(get_ssid_num $wireless_ifname)
		vlan_network=vlan$vlanID

		if [ "$ssid_bridge_type" = "2" -o "${isolation:-0}" = "0" ]; then # MPSK enable or Wireless VLAN disable
			local wireless_network
			if [ "${MvlanEnable:-0}" = "1" -a "${TvlanEnable:-0}" != "1" ]; then # Managment VLAN enable && Trunk VLAN disable
				wireless_network="99"
			else                                                                # Managment VLAN disable OR ( Managment VLAN enable && Trunk VLAN enable )
				wireless_network="`uci -q get /rom/etc/config/wireless.${option}.network`"
			fi
			if [ "$ssid_bridge_type" = "1" ];then                # Guest network enable
				nat_type_check ${guest_network} ${iface_ssid} ${option}
			else                                                                # Guest network disable
				# add_network_config $vlan_network "na"
				# uci set network.$vlan_network.macaddr=$lan_mac_address
				uci set wireless.${option}.network=$wireless_network
			fi
		else                                # Wireless VLAN enable, ssid_bridge_type=0 or 1
			local match_mvlan=0
			if [ "${MvlanEnable:-0}" = "1" ]; then       # Managment enable
				if [ "$MvlanID" = "$vlanID" ]; then     # Managment VLAN = Wireless VLAN
					match_mvlan=1
					if [ "$ssid_bridge_type" = "0" ]    # Disable or Bridge
					then
						uci set wireless.${option}.network="`uci -q get /rom/etc/config/wireless.${option}.network`"
					else                                # NAT or NAT_only, do nothing.
						nat_type_check ${guest_network} ${iface_ssid} ${option}
					fi
				else  # Default Wireless VLAN Setting, continue
					# match_mvlan=0, do continue
					:
				fi
				# match_mvlan=0, do continue
			fi

			# match_mvlan=0, do continue
			if [ $match_mvlan -eq 0 ]
			then
				local vlanifnames=$(append_eth_vid ${vlanID})

				add_network_config $vlan_network "$vlanifnames"
				uci set network.$vlan_network.macaddr=$lan_mac_address

				if [ "$ssid_bridge_type" = "1" ];then            # Guest network enable (NAT or NAT_only)
					nat_type_check ${guest_network} ${iface_ssid} ${option}
				else                                                            # Guest network enable
					uci set wireless.$option.network=$vlan_network
				fi
			fi
		fi
	done

	uci commit
}

sync_wireless_network_option()
{
	local value
	local my_config
	local sync_config
	local wireless_config

	for wireless_config in wifi0 $wifi_configs_wifi0 wifi1 $wifi_configs_wifi1 wifi2 $wifi_configs_wifi2; do
		if [ "$wireless_config" = "wifi0" ]
		then
			my_config="0"
			sync_config="1 2"
			continue
		elif [ "$wireless_config" = "wifi1" ]
		then
			my_config="1"
			sync_config="0 2"
			continue
		elif [ "$wireless_config" = "wifi2" ]
		then
			my_config="2"
			sync_config="0 1"
			continue
		fi
		value=$(uci -q get wireless.$wireless_config.network)
		for i in $sync_config
		do
			uci -q set wireless.${wireless_config/wifi$my_config/wifi$i}.network=$value
		done
	done
}

isolation_network_options_eth_profile()
{
	for ethprofile_config in $(foreach ethprofile profile); do
		if [ "$(uci -q get ethprofile.$ethprofile_config.enable)" = "1" ]; then
			local match_mvlan=0
			local nat_mode=$(uci -q get ethprofile.$ethprofile_config.client_ip_assignment)
			local ethProfileIfname=$(uci -q get ethprofile.$ethprofile_config.ifname)
			local ethProfileVlanEnable=$(uci -q get ethprofile.$ethprofile_config.vlan_enable)
			local ethProfileVlanID=$(uci -q get ethprofile.$ethprofile_config.vlan_id)

			if [ "$ethProfileVlanEnable" = "1" ]; then
				# Managment enable and Managment VLAN = Wireless VLAN
				if [ "${MvlanEnable:-0}" = "1" -a "$MvlanID" = "$ethProfileVlanID" ]; then
					match_mvlan=1
				fi
			fi

			if [ "$nat_mode" = "1" ]; then
				local port_num=${ethprofile_config#port_}
				local brname=nat${port_num}p
				add_network_config $brname $ethProfileIfname 172.$((23+$port_num)).1.1 255.255.0.0
				if [ "$ethProfileVlanEnable" = "1" -a "$match_mvlan" = "0" ]; then
					vlan_network=vlan$ethProfileVlanID
					add_network_config $vlan_network "$(append_eth_vid ${ethProfileVlanID})"
				fi
			else
				if [ "$ethProfileVlanEnable" = "1" ]; then
					if [ "$match_mvlan" = "1" ]; then
						uci set network.lan.ifname="$ethProfileIfname $(append_eth_vid ${ethProfileVlanID})"
					else
						vlan_network=vlan$ethProfileVlanID
						local vlanifnames=$(append_eth_vid ${ethProfileVlanID})
						vlanifnames="$ethProfileIfname $vlanifnames"

						add_network_config $vlan_network "$vlanifnames"
					fi
				else
					if [ "${MvlanEnable:-0}" = "1" ]; then
						uci set network.99.ifname="$ethProfileIfname $lan_ifnames"
					else
						uci set network.lan.ifname="$ethProfileIfname $lan_ifnames"
					fi
				fi
			fi
		fi
	done
}

setup_avahi_reflector_bridge()
{
	local configs
	local enabled=$(uci -q get avahi-daemon.avahi.reflector_enable)

	if [ "$enabled" = "1" ]; then
		for configs in $(foreach avahi-daemon reflector-rule); do
			local from
			local vlan_pool
			#option to '31'
			#option from '32-34,38'
			local to=$(uci -q get avahi-daemon.$configs.to)
			local froms=$(uci -q get avahi-daemon.$configs.from)

			# froms='32-34,38'
			froms=$(echo ${froms//,/ })
			# froms='32-34 38'
			for from in $froms; do
				if [ "$from" != "${from/-/}" ]; then
					local start=${from%-*}
					local end=${from#*-}
					vlan_pool="$vlan_pool `seq $start $end`"
					# remove \n
					vlan_pool=`echo $vlan_pool`
				else
					vlan_pool="$vlan_pool $from"
				fi
			done

			vlan_pool="$vlan_pool $to"
		done

		local var
		for var in $vlan_pool; do
			local vlan_network=vlan$var
			local vlanifnames=$(append_eth_vid ${var})
			add_network_config $vlan_network "$vlanifnames"
		done
	fi
}

wds_network_setting(){
	local wifix
	for wifix in $(ls /sys/class/net/ |grep ^wifi)
	do
		if [ "$MvlanEnable" = "1" ];then
			if [ -n "$is_wds_ap" ]; then
				if [ "`uci -q get wireless.${wifix}.opmode`" = "wds_ap" ]; then
					uci set wireless.${wifix}_wds_0.network="99"
				else
					uci set wireless.${wifix}_wds_0.network="lan"
				fi
			elif [ -n "$is_wds_bridge" ]; then
				if [ "`uci -q get wireless.${wifix}.opmode`" = "wds_bridge" ]; then
					uci set wireless.${wifix}_wds.network="99"
				else
					uci set wireless.${wifix}_wds.network="lan"
				fi
			elif [ -n "$is_wds_sta" ]; then
				if [ "`uci -q get wireless.${wifix}.opmode`" = "wds_sta" ]; then
					uci set wireless.${wifix}_wds_sta.network="99"
				else
					uci set wireless.${wifix}_wds_sta.network="lan"
				fi
			fi
		else
			uci set wireless.${wifix}_wds_0.network="lan"
			uci set wireless.${wifix}_wds.network="lan"
			uci set wireless.${wifix}_wds_sta.network="lan"
		fi
	done
}

isolation_vlan_settings()
{
	clear_network_configs
	clear_wireless_configs
	# needn't remove empty bridge. if empty bridge exist, netifd status is wrong, call ubus to remove it.
	#remove_empty_bridges

	# Check the interfaces again for product which supports Router mode.
	support_wan_mode=$(uci -q get network.wan)
	if [ -n "$support_wan_mode" ]; then
		lan_ifnames=$(uci -q get network.lan.ifname)
	fi

	wds_network_setting
	case ${MvlanEnable:-0},${TvlanEnable:-0} in
		1,0)
			uci set network.lan.ifname="$(append_eth_vid ${MvlanID})"
			uci set network.99.ifname="$lan_ifnames"
			;;
		1,1)
			uci set network.lan.ifname="$(append_eth_vid ${MvlanID})"
			uci set network.99.ifname="na"
			;;
		0,0)
			uci set network.lan.ifname="$lan_ifnames"
			uci set network.99.ifname="na"
			;;
	esac

	isolation_network_options wifi0
	isolation_network_options wifi1
	isolation_network_options wifi2
	sync_wireless_network_option


	if [ "$is_support_ssid_profile" = "1" ]; then
		isolation_network_options_eth_profile
	fi

	setup_avahi_reflector_bridge
	guest_network_vlan_settings

}

mesh_bridge()
{
	if [ "$mesh_off" = "0" ]; then
		local uci_commands=""
		# For product which supports Router mode.
		support_wan_mode=$(uci -q get network.wan)
		if [ -n "$support_wan_mode" ]; then
			uci_commands="`uci show network | grep -v "wan" | grep -v "wan6" |grep "ifname=" |grep "eth1\|eth0"| grep -v "bat0"`"
		else
			uci_commands="`uci show network |grep "ifname=" |grep "eth1\|eth0"| grep -v "bat0"`"
		fi
		_IFS=$IFS
		IFS=$'\n'
		for cmd in $uci_commands
		do
			local ifnames="${cmd#*=}"
			ifnames=${ifnames//\'/}
			IFS=$' '
			for ifname in $ifnames
			do
				local vlanid=${ifname#*.}
				if [ "$vlanid" = "$ifname" ]
				then
					vlanid=
				fi
				local batname=bat0${vlanid:+.}$vlanid
				break
			done
			echo uci set ${cmd%=*}=\"$ifnames $batname\"
			uci set ${cmd%=*}="$ifnames $batname"
			IFS=$'\n'
		done
		IFS=$_IFS
		uci commit network
		ubus call network reload
        bat_cpu_adjust
	fi
}

lacp_start()
{
	[ "$(uci get functionlist.functionlist.SUPPORT_ETHERNET_BONDING)" -eq 1 ] && {
			echo $func_enable_bonding > /tmp/prev_bond
		[ $func_enable_bonding -eq 1 ] && /lib/senao-shell-libs/senao-vlan-network/lacp.sh start
	}
}

lacp_reload()
{
	local prev_bond=$(cat /tmp/prev_bond)
	[ "$(uci get functionlist.functionlist.SUPPORT_ETHERNET_BONDING)" -eq 1 ] && {
			if [ $prev_bond -eq 1 ]; then
				ifenslave -d bond0 eth1 eth0
			fi
			[ $func_enable_bonding -eq 1 ] && {
				# below command is for bond0 can bridge to br-lan via network reload.
			ifconfig bond0 up
			ifenslave bond0 eth1 eth0
			/lib/senao-shell-libs/senao-vlan-network/lacp.sh reload &
		}
		echo $func_enable_bonding > /tmp/prev_bond
	}
}

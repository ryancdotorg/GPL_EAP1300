ezmcloud=$(test -f /etc/config/ezmcloud && echo 1 || echo 0)
if [ $ezmcloud -eq 0 ];then
    captived=$(uci get apcontroller.capwap.enable)
else
    # cloud always captived.
    captived=1
fi
portal_en=$(uci -q get portal.general.enable)

func_enable_bonding=$(uci -q get network.lacp.lacp_enable)
if [ ${func_enable_bonding:-0} -eq 1 ]; then
	lan_ifnames="bond0"
else
	lan_ifnames=$(uci -q get /rom/etc/config/network.lan.ifname)
	# failsafe mode cannot got ifname from rom.
	if [ "$lan_ifnames" = "" ]
	then
		for ifs in $(uci -q get network.lan.ifname)
		do
			if [ "${ifs#eth}" != "$ifs" ]
			then
				lan_ifnames="${ifs%%.*}${lan_ifnames:+ }${lan_ifnames}"
			fi
		done
	fi
fi
mesh_off=$(uci -q get mesh.wifi.disabled)
is_wds_ap=$(uci show wireless | grep wifi[0-2].opmode | cut -d "=" -f 2 | xargs | grep wds_ap )
is_wds_bridge=$(uci show wireless | grep wifi[0-2].opmode | cut -d "=" -f 2 | xargs | grep wds_bridge )
is_wds_sta=$(uci show wireless | grep wifi[0-2].opmode | cut -d "=" -f 2 | xargs | grep wds_sta )

if [ -n "$is_wds_ap" ]; then
	if [ "`uci -q get wireless.wifi0.opmode`" == "wds_ap" ]; then
		wds_ifname=$(uci -q get wireless.wifi0_wds_0.ifname)
    elif [ "`uci -q get wireless.wifi1.opmode`" == "wds_ap" ]; then
        wds_ifname=$(uci -q get wireless.wifi1_wds_0.ifname)
    elif [ "`uci -q get wireless.wifi2.opmode`" == "wds_ap" ]; then
		wds_ifname=$(uci -q get wireless.wifi2_wds_0.ifname)
	fi
elif [ -n "$is_wds_bridge" ]; then
	if [ "`uci -q get wireless.wifi0.opmode`" == "wds_bridge" ]; then
		wds_ifname=$(uci -q get wireless.wifi0_wds.ifname)
    elif [ "`uci -q get wireless.wifi1.opmode`" == "wds_bridge" ]; then
        wds_ifname=$(uci -q get wireless.wifi1_wds.ifname)
    elif [ "`uci -q get wireless.wifi2.opmode`" == "wds_bridge" ]; then
		wds_ifname=$(uci -q get wireless.wifi2_wds.ifname)
	fi
fi

if [ "$mesh_off" = "0" ]; then
    echo "do mesh_bridge"
elif [ -n "$is_wds_ap" -o -n "$is_wds_bridge" ]; then
	lan_ifnames="$lan_ifnames $wds_ifname"
fi

is_support_ssid_profile=$(uci -q get functionlist.functionlist.SUPPORT_SSID_PROFILE)

# ECW115 support access/trunk port and ssid profile, need to switch eth0/eth1 or only eth1.
if [ "$is_support_ssid_profile" = "1" ]; then
	for ethprofile_config in $(foreach ethprofile profile); do
		ethprofile_ifnames="$(uci -q get ethprofile.$ethprofile_config.ifname)"

		# Remove ethprofile ifnames if support eth profile.
		lan_ifnames=$(eval "echo $lan_ifnames | sed 's/$ethprofile_ifnames//g' | xargs")
	done
fi

lan_mac_address=$(setconfig -g 6)

is_enjet_on=$(uci -q get wireless.wifi1.qboost_enable)

MvlanID=$(uci -q get network.sys.ManagementVLANID)
MvlanEnable=$(uci -q get network.sys.WLANVLANEnable)
TvlanEnable=$(uci -q get network.sys.TrunkVLANEnable)

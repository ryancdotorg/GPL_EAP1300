#!/bin/sh
. /etc/mgmt.d/mgmt_path.sh
. /lib/wifi/qcawifi.sh
. /lib/wifi/hostapd.sh

create_wifi_vaps1(){
    device=$1
    vif=$2

    local nosbeacon= wlanaddr=""
    config_get mode "$vif" mode
    local wlanmode=$mode
    config_get enc "$vif" encryption "none"

    config_get ifname "$vif" ifname
    config_get_bool nosbeacon "$device" nosbeacon
    all_ifnames=$(echo $all_ifnames $ifname)

    [ "$nosbeacon" = 1 ] || nosbeacon=""

    ifname=$(/usr/sbin/wlanconfig "$ifname" create wlandev "$device" wlanmode "$mode" ${nosbeacon:+nosbeacon})

    [ $? -ne 0 ] && {
	DebugPrint "enable_qcawifi($device): Failed to set up $mode vif $ifname"
	continue
    }
    [ ${device%[0-9]} = "wifi" ] && config_set "$device" phy "$device"
    config_set "$vif" bridge "$MGMT_BRIDGE"
}

vap_ap_mode_setting1() { #parameters for ap mode only

    device=$1
    vif=$2
    config_get ifname "$vif" ifname

    config_get_bool hidden "$vif" hidden 0
    [ -n "$hidden" ] && iwpriv "$ifname" hide_ssid "$hidden"

    # beacon interval. get bintval(default value 100ms) here, not from wireless cfg file.
    local bintval=$(get_beacon_interval "$device")
    #[ -n "$bintval" ] && iwpriv "$ifname" bintval "$bintval"

    config_get dtim_period "$vif" dtim_period
    [ -n "$dtim_period" ] && iwpriv "$ifname" dtim_period "$dtim_period"

    config_get preamble "$vif" preamble
    [ -n "$preamble" ] && iwpriv "$ifname" shpreamble "$preamble"

    config_get_bool uapsd "$vif" uapsd 1
    [ -n "$uapsd" ] && iwpriv "$ifname" uapsd "$uapsd"

    config_get mcast_rate "$vif" mcast_rate
    [ "$mcast_rate" ] && iwpriv "$ifname" mcast_rate "${mcast_rate}"

    config_get mcastenhance "$vif" mcastenhance
    [ -n "$mcastenhance" ] && iwpriv "$ifname" mcastenhance "${mcastenhance}"

    config_get me_adddeny "$vif" me_adddeny
    [ -n "$me_adddeny" ] && iwpriv "$ifname" me_adddeny ${me_adddeny}
    #iwpriv "$ifname" me_adddeny 3758096635 255 255 255 #filter Bonjour/mDNS packet 
    # emr3000 use this, but the proper usage should set these parameter in wireless config, ex: wireless.wifi0_ssid_1.me_adddeny='3758096635 255 255 255'

    # per vap setting functions

    # SENAO TDMA setting
    set_sn_tdma $device $vif
}

vap_common_setting1() {	#parameters for ap and sta mode

    device=$1
    vif=$2

    config_get opmode "$device" opmode
    config_get ifname "$vif" ifname
    config_get qboost_enable "$device" qboost_enable 0
    config_get qboost_vif "$vif" qboost_vif 0

    #set_hwmode_htmode1 "$ifname" "$device" "$opmode"

    config_get wds "$vif" wds
    case "$wds" in
	1|on|enabled) wds=1;;
	*) wds=0;;
    esac
    iwpriv "$ifname" wds "$wds"

    # TDMA
    [ "$qboost_enable" == "1" ] && [ "$qboost_vif" == "1" ] && [ "$opmode" == "wds_ap" ] && iwpriv "$ifname" wds 1

    config_get_bool shortgi "$vif" shortgi 1
    [ -n "$shortgi" ] && iwpriv "$ifname" shortgi "${shortgi}"

    # 11k fastroaming-preset 11k disable, sw spec requirement
    iwpriv "$ifname" rrm 0

    iwpriv "$ifname" wmm 1  # no matter AP/STA mode, it should be enabled

    iwpriv "$ifname" doth 1 # no matter AP/STA mode, it should be enabled

    config_get nss "$vif" nss
    [ -n "$nss" -a "$ac_mode" == "1" ] && iwpriv "$ifname" nss "$nss"

    # ap_isoloation, it seems only wrap mode no need to use this
    config_get_bool ap_isolation_enabled $device ap_isolation_enabled 0
    config_get_bool isolate "$vif" isolate 0

    if [ $ap_isolation_enabled -ne 0 ]; then
	[ "$mode" = "wrap" ] && isolate=1
    fi
    local net_cfg bridge
    net_cfg="$(find_net_config "$vif")"
    [ -z "$net_cfg" -o "$isolate" = 1 -a "$mode" = "wrap" ] || {
	[ -f /sys/class/net/${ifname}/parent ] && { \
	    bridge="$(bridge_interface "$net_cfg")"
	    config_set "$vif" bridge "$bridge"
	    start_net "$ifname" "$net_cfg"
	}
    }
    #end of ap_isolation

    config_get ssid "$vif" ssid
    [ -n "$ssid" ] && {
	iwconfig "$ifname" essid "$ssid"
	#iwconfig "$ifname" essid on
	#iwconfig "$ifname" essid ${ssid:+-- }"$ssid"
    }

    set_wifi_up "$vif" "$ifname"
    set_txpower "$device" "$vif"

    set_hostapd_wpa_supplicant "$vif"

    set_aggregation "$device" "$ifname"

    # support traffic control
    [ -e "/lib/wifi/traffic_control.sh" ] && tc_setConf "$vif"

    # fix tx/rx packet and bytes incorrect issue.
    iwpriv "$device" enable_ol_stats 1

    # support disable legacy rate (set min rate)
    set_min_rate "$device" "$vif"
}

vap_ap_mode_setting() {	#parameters for ap mode only

    device=$1
    vif=$2
    config_get ifname "$vif" ifname

    config_get_bool hidden "$vif" hidden 0
    [ -n "$hidden" ] && iwpriv "$ifname" hide_ssid "$hidden"

    # beacon interval. get bintval(default value 100ms) here, not from wireless cfg file.
    local bintval=$(get_beacon_interval "$device")
    [ -n "$bintval" ] && iwpriv "$ifname" bintval "$bintval"

    config_get dtim_period "$vif" dtim_period
    [ -n "$dtim_period" ] && iwpriv "$ifname" dtim_period "$dtim_period"

    config_get preamble "$vif" preamble
    [ -n "$preamble" ] && iwpriv "$ifname" shpreamble "$preamble"

    config_get_bool uapsd "$vif" uapsd 1
    [ -n "$uapsd" ] && iwpriv "$ifname" uapsd "$uapsd"

    config_get mcast_rate "$vif" mcast_rate
    [ "$mcast_rate" ] && iwpriv "$ifname" mcast_rate "${mcast_rate}"

    config_get mcastenhance "$vif" mcastenhance
    [ -n "$mcastenhance" ] && iwpriv "$ifname" mcastenhance "${mcastenhance}"

    config_get me_adddeny "$vif" me_adddeny
    [ -n "$me_adddeny" ] && iwpriv "$ifname" me_adddeny ${me_adddeny}
    #iwpriv "$ifname" me_adddeny 3758096635 255 255 255 #filter Bonjour/mDNS packet 
    #emr3000 use this, but the proper usage should set these parameter in wireless config, ex: wireless.wifi0_ssid_1.me_adddeny='3758096635 255 255 255'

    # per vap setting functions

    # SENAO TDMA setting
    set_sn_tdma $device $vif	
}

preinit(){
    local device="$1"
    local ifname
    local mode
    local disabled
    config_get opmode "$device" opmode
    local vifs=$(eval "/usr/sbin/foreach wireless wifi-iface device $1")

    for vif in $vifs; do
        config_get mode_display "$vif" mode_display
        [ "$opmode" != "$mode_display" ] &&  continue
        config_get mode "$vif" mode
        config_get disabled "$vif" disabled
        config_get ifname "$vif" ifname
        case "$disabled:$mode" in
            "0:sta")
                ## keep wds_sta set extap 1 to make mgmt interface active correctly ##
                ## ignore the issue about wds_sta connect to the pure ap and ping successfully ##
                #[ "$mode_display" != "wds_sta" ] && iwpriv "$ifname" extap 1
                iwpriv "$ifname" extap 1
                ;;
        esac
    done
    if  [ -e "/var/run/wifi-$MGMT_IFACE.pid" ]; then
        kill $(cat /var/run/wifi-$MGMT_IFACE.pid) &> /dev/null
    fi
}

set_mgmt_vap(){
    config_foreach preinit wifi-device
    create_wifi_vaps1 $MGMT_RADIO $MGMT_SECTION
    vap_ap_mode_setting1 $MGMT_RADIO $MGMT_SECTION
    vap_common_setting1 $MGMT_RADIO $MGMT_SECTION
    #ifconfig $MGMT_IFACE down up
    ifconfig $MGMT_IFACE down;sleep 3;ifconfig $MGMT_IFACE up;echo "[Debug] mgmt ssid up." > /dev/console
    brctl addif $MGMT_BRIDGE $MGMT_IFACE

###########wifi cmd for bring up mgmt1 in other mode(except AP)##########################
#iwpriv mgmt1 hide_ssid 0
### <NG command in wds-bridge> ##iwpriv mgmt1 bintval 100
#iwpriv mgmt1 dtim_period 2
#iwpriv mgmt1 shpreamble 0
#iwpriv mgmt1 uapsd 1
#iwpriv mgmt1 mcastenhance 2
#iwpriv mgmt1 maccmd 3
#iwpriv mgmt1 maccmd 0
#SUPPORT_BAND_STEERING_WITH_SUPPRESS_PROBE_RESP_AND_ASSOCIATION not equal to 1
#wifi1
#iwpriv mgmt1 mode 11ACVHT40
### <NG command in wds-bridge> ##iwpriv mgmt1 disablecoext 1
#iwpriv mgmt1 chwidth 1
#iwconfig mgmt1 rts off
#iwpriv mgmt1 wds 0
#iwpriv mgmt1 shortgi 1
#iwpriv mgmt1 rrm 0
#iwpriv mgmt1 wmm 1
#iwpriv mgmt1 doth 1
#iwconfig mgmt1 essid ENMGMTFF2031
#iwconfig mgmt1 txpower 0
#iwpriv mgmt1 ap_bridge wpriv mgmt1 bintval 100
#################################################

}

case "$1" in
    "create_vap")
        set_mgmt_vap
        ;;
    *)
        ;;
esac


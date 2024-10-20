#!/bin/sh
. /etc/sn-udhcpc-libs.sh
. /etc/chilli/chilli-libs.sh
if [ "$HS_LOGIN_TYPE" = "108" ]
then
    . /etc/chilli/fbwifi-libs.sh
    redir_ips_file_name=$(fbwifi_get_redir_ips_file $CONFIG_NAME)
    fbwifi_all_redir_ips="$(cat $redir_ips_file_name)"
fi

REAL_WANIF=br-lan

CONFIG_NAME=$CONFIG_NAME
iptable_name=$CONFIG_NAME

wlanconfig() {
	[ -n "${DEBUG}" ] && echo wlanconfig "$@"
	/usr/sbin/wlanconfig "$@"
}

wallgardenF() {
    local WG_LIST=$(chilli_query -s /var/run/chilli.$CONFIG_NAME.sock listgarden| sed '1,/subscriber/!d')
    local WG_NUM=$(echo "$WG_LIST" |grep "^host=" |wc -l)
    local WG_HOST=$(echo "$WG_LIST" | awk -F " " '/^host=/{print $1}' |cut -d "=" -f 2 | tr '\n' ' ')
    local WG_MASK=$(echo "$WG_LIST" | awk -F " " '/^host=/{print $2}' |cut -d "=" -f 2 | tr '\n' ' ')
    local WG_PORT=$(echo "$WG_LIST" | awk -F " " '/^host=/{print $4}' |cut -d "=" -f 2 | tr '\n' ' ')
    local section_name=$(get_section_name $CONFIG_NAME)

    if [ $bridgemode -eq 1 ]
    then
        markvalue=$(get_mark_value $CONFIG_NAME 1)
        ipt4 $iptable_name -t mangle -I $portal_postrouting_mangle_x -o $HS_WANIF -m mark --mark $markvalue -j $chilli_postrouting_mangle
        ipt4 $iptable_name -t mangle -I $chilli_postrouting_mangle -m coova --name $HS_KNAME --dest -j ACCEPT
    fi

    if [ $bridgemode -eq 1 ]
    then
        markvalue=$(get_mark_value $CONFIG_NAME 0)
        ipt4 $iptable_name -t mangle -I $portal_prerouting_mangle_x -i $HS_WANIF -m mark --mark $markvalue -j $chilli_prerouting_mangle
        ipt4 $iptable_name -t nat -I $portal_prerouting_nat_x -m mark --mark $markvalue -j $chilli_prerouting_nat
    else
        markvalue=$(get_mark_value $CONFIG_NAME 0)
        ipt4 $iptable_name -t mangle -I $portal_prerouting_mangle_x -i $DHCPIF -m mark --mark $markvalue -j $chilli_prerouting_mangle
        ipt4 $iptable_name -t nat -I $portal_prerouting_nat_x -m mark --mark $markvalue -j $chilli_prerouting_nat
    fi

    if [ $bridgemode -eq 1 ]
    then
        local lan_mac=$(echo ${HS_APMAC//-/:}| tr '[A-F]' '[a-f]')
        # xt_coova will modify dest mac to ap mac if not auth
        # NOTE: because need use chilli to allow walledgarden pool, rewrite all dest mac.
        ipt4 $iptable_name -t mangle -I $chilli_prerouting_mangle -m coova ! --name $HS_KNAME --mdmac $lan_mac -j ACCEPT
        ipt4 $iptable_name -t mangle -I $chilli_prerouting_mangle -d $HS_UAMLISTEN -m coova --name $HS_KNAME --mdmac $lan_mac -j ACCEPT

        if [ "$HS_LOGIN_TYPE" = "108" ]
        then
            for redir_ip in $fbwifi_all_redir_ips
            do
                ipt4 $iptable_name -t mangle -I $chilli_prerouting_mangle -d $redir_ip -m coova --name $HS_KNAME --mdmac $lan_mac -j ACCEPT
            done
        fi

        # ipt4 $iptable_name -t mangle -I $chilli_prerouting_mangle -m coova ! --name $HS_KNAME --mdmac $lan_mac -j ACCEPT
        # ipt4 $iptable_name -t mangle -I $chilli_prerouting_mangle -m coova ! --name $HS_KNAME --mdmac $lan_mac -m udp -p udp --dport 53 -j ACCEPT
        ipt4 $iptable_name -t mangle -I $chilli_prerouting_mangle -p udp -m udp --dport 67:68 -j ACCEPT

        # for bridge mode, let dhcp release packet to AP, do release mac to pass fbwifi cert. it can use for all login_type safty...
        if [ "$HS_LOGIN_TYPE" = "108" ]
        then
            ipt4 $iptable_name -t mangle -I $chilli_prerouting_mangle -p udp -m udp --dport 67 -d $HS_DHCPLISTEN -m coova --name $HS_KNAME --mdmac $lan_mac -j ACCEPT
            # TEE to duplicat, then change dest mac to AP
            ipt4 $iptable_name -t mangle -I $chilli_prerouting_mangle -p udp -m udp --dport 67 -d $HS_DHCPLISTEN -m coova --name $HS_KNAME --mdmac $lan_mac -j TEE --gateway $HS_DHCPLISTEN
        fi
    fi

    ipt4 $iptable_name -t mangle -I $portal_forward_mangle_x -o $TUNTAP -p tcp -m tcp --tcp-flags SYN,RST SYN -j TCPMSS --set-mss 1392

    if [ $bridgemode -eq 1 ]
    then
        markvalue=$(get_mark_value $CONFIG_NAME 0)
        ipt4 $iptable_name -t filter -I $portal_forward_filter_x -m mark --mark $markvalue -j $chilli_forward_filter
        # avoid vlan's bridge defult policy DROP.
        ipt4 $iptable_name -t filter -I $chilli_forward_filter -j ACCEPT
        ipt4 $iptable_name -t filter -I $portal_forward_filter_x -i $TUNTAP -j ACCEPT
        ipt4 $iptable_name -t filter -I $portal_forward_filter_x -o $TUNTAP -j ACCEPT
        # autotest room find windows device will receive RST packets from AP, it will cause logint page cannot shown. so DROP RST packets here to WAR
        ipt4 $iptable_name -t filter -I $portal_forward_filter_x -o $TUNTAP -p tcp -m tcp --tcp-flags SYN,RST RST -j DROP
        # if vlan enabled, need this rule to pass dhcp offer from br-ssidXvlan
        ipt4 $iptable_name -t filter -I $portal_forward_filter_x -p udp -m udp --dport 67:68 -j ACCEPT
        # ipt4 $iptable_name -t filter -I $chilli_forward_filter -i $HS_WANIF -m coova ! --name $HS_KNAME -j DROP
        # avoid auth finished, some packets forwarding to lan
        ipt4 $iptable_name -t filter -I $chilli_forward_filter -d $HS_UAMLISTEN -o $HS_WANIF -j DROP
    else
        markvalue=$(get_mark_value $CONFIG_NAME 0)
        ipt4 $iptable_name -t filter -I $portal_forward_filter_x -m mark --mark $markvalue -i $TUNTAP -o $HS_WANIF -j $chilli_forward_filter
    fi


    # if [ $HS_ACCESS_LAN -eq 0 ];then
    #     ipt4 $iptable_name -t filter -I $chilli_forward_filter -d $wanSegment -j DROP
    # fi

    # if [ -n "$HS_KNAME" ]
    # then
        # if [ $HS_ACCESS_LAN -eq 0 ]
        # then
        #     ipt4 $iptable_name -t mangle -I $chilli_prerouting_mangle -i $DHCPIF -d $wanSegment -j DROP
        # fi

        # client isolation not work when nfcoova+bridgemode enable
        # only for NAT mode
        # if [ $HS_CLIENT_ISO -eq 1 ]
        # then
        #     if [ $bridgemode -eq 0 ]
        #     then
        #         ipt4 $iptable_name -t mangle -I $chilli_prerouting_mangle -i $DHCPIF -d $wanSegment -j DROP
        #     else
        #         # NAT mode only support same radio client isolation, must use ebtable(l2_isoation) to deny different radio.
        #         echo
        #     fi
        #     for i in $(get_bridge_name $CONFIG_NAME)
        #     do
        #         iwpriv "$i" ap_bridge 0
        #     done
        # fi
    # fi

    if [ $ezmcloud -eq 0 ]
    then
        # if [ $bridgemode -eq 1 ]
        # then
        #     ipt4 $iptable_name -t filter -I $portal_input_filter_x -p tcp -d $acIP --destination-port ${ac_portal_PORT:-80} -j ACCEPT
        #     ipt4 $iptable_name -t filter -I $portal_input_filter_x -p tcp -d $acIP -m string --string login.html -j DROP --algo bm
        #     ipt4 $iptable_name -t filter -I $portal_input_filter_x -p tcp -d $acIP -m string --string jquery -j DROP --algo bm
        # fi
        # ipt4 $iptable_name -t mangle -I $chilli_prerouting_mangle -d $acIP -j DROP
        # ipt4 $iptable_name -t mangle -I $chilli_prerouting_mangle -p tcp -d $acIP --destination-port ${ac_portal_PORT:-80} -j ACCEPT
        # ipt4 $iptable_name -t mangle -I $chilli_prerouting_mangle -p tcp -d $acIP -m string --string login.html -j DROP --algo bm
        # ipt4 $iptable_name -t mangle -I $chilli_prerouting_mangle -p tcp -d $acIP -m string --string jquery -j DROP --algo bm

        ipt4 $iptable_name -t filter -I $chilli_forward_filter -d $acIP -j DROP
        ipt4 $iptable_name -t filter -I $chilli_forward_filter -p tcp -d $acIP --destination-port ${ac_portal_PORT:-80} -j ACCEPT
        ipt4 $iptable_name -t filter -I $chilli_forward_filter -p tcp -d $acIP -m string --string login.html -j DROP --algo bm
        ipt4 $iptable_name -t filter -I $chilli_forward_filter -p tcp -d $acIP -m string --string jquery -j DROP --algo bm
    fi

    # allow to access gw in bridge mode
    # if [ $bridgemode -eq 1 ]
    # then
    #     ipt4 $iptable_name -t mangle -I $chilli_prerouting_mangle -d $HS_DHCPGATEWAY -j ACCEPT
    #     ipt4 $iptable_name -t filter -I $chilli_forward_filter -d $HS_DHCPGATEWAY -j ACCEPT
    # fi

    if [ "$WG_NUM" > 1 ]; then
        idx=1;
        for maskVAR in $WG_MASK
        do
            #echo $idx $(echo "$WG_HOST" | awk -F " " '{print $'$idx'}')/$maskVAR > /dev/ttyS0
            local WG_PORT_idx=$(echo "$WG_PORT" | awk -F " " '{print $'$idx'}');
            local WG_HOST_IP=$(echo "$WG_HOST" | awk -F " " '{print $'$idx'}')
            local WG_HOST_NETWORK=$(ipcalc NETWORK $WG_HOST_IP $wanMask)
            if [ "$WG_HOST_NETWORK" = "$wanNetwork" ] && \
                [ "$WG_HOST_IP" != "$acIP" ] && \
                [ "$WG_HOST_IP" != "$HS_DHCPLISTEN" ]; then
                if [ "$maskVAR" = "255.255.255.255" ] && [ "$WG_PORT_idx" = "0" ]; then
                    ipt4 $iptable_name -t filter -I $chilli_forward_filter -d $WG_HOST_IP -j ACCEPT
                    ipt4 $iptable_name -t mangle -I $chilli_prerouting_mangle -d $WG_HOST_IP -j ACCEPT
                else
                    ipt4 $iptable_name -t filter -I $chilli_forward_filter -d $WG_HOST_IP/$maskVAR -j ACCEPT
                    ipt4 $iptable_name -t mangle -I $chilli_prerouting_mangle -d $WG_HOST_IP/$maskVAR -j ACCEPT
                fi
            fi
            idx=$((++idx))
        done
    fi

    if [ $bridgemode -eq 1 ]
    then
        ipt4 $iptable_name -t filter -I $chilli_forward_filter -p udp -m udp --dport 53 -j ACCEPT
        ipt4 $iptable_name -t filter -I $chilli_forward_filter -p udp -m udp --sport 53 -j ACCEPT
        markvalue=$(get_mark_value $CONFIG_NAME 0)
        ipt4 $iptable_name -t filter -I $chilli_forward_filter -d 255.255.255.255/32 -p udp -m udp --dport 67:68 -j ACCEPT
        ipt4 $iptable_name -t filter -I $chilli_forward_filter -p udp -m udp --dport 67:68 -j ACCEPT
    else
        ipt4 $iptable_name -t nat -I $chilli_prerouting_nat -m coova --name $HS_KNAME -p tcp --dport 80 -d $HS_UAMLISTEN -j DNAT --to $HS_UAMLISTEN:$HS_UAMPORT
        ipt4 $iptable_name -t nat -I $chilli_prerouting_nat -m coova --name $HS_KNAME -p tcp --dport 443 -d $HS_UAMLISTEN -j DNAT --to $HS_UAMLISTEN:$HS_UAMUIPORT
    fi

    if [ "$HS_LOGIN_TYPE" = "108" ]
    then
        for redir_ip in $fbwifi_all_redir_ips
        do
            # http use 3990
            ipt4 $iptable_name -t nat -I $chilli_prerouting_nat -m coova --name $HS_KNAME -p tcp --dport 80 -d $redir_ip -j DNAT --to $HS_UAMLISTEN:$HS_UAMPORT
            # https use 4990, i don't know why....
            ipt4 $iptable_name -t nat -I $chilli_prerouting_nat -m coova --name $HS_KNAME -p tcp --dport 443 -d $redir_ip -j DNAT --to $HS_UAMLISTEN:$HS_UAMUIPORT
        done
    fi
}


ezmcloud=$(test -f /etc/config/ezmcloud && echo 1 || echo 0)
section_id=$(get_section_id $CONFIG_NAME)
section_name=$(get_section_name $CONFIG_NAME)
profile_index=$(get_profile_index $CONFIG_NAME)
is_ethernet=$(is_ethernet_port $CONFIG_NAME)

acIP=$(uci get apcontroller.capwap.ac)
acIP=${acIP##*=}
acIP=${acIP%%:*}
force_acIP=$(uci get apcontroller.capwap.force_ac)
force_acIP=${force_acIP##*=}
force_acIP=${force_acIP%%:*}
acIP=${acIP:-$force_acIP}
acIP=${acIP:-192.168.0.239}

ac_portal_PORT=$(uci get portal.$section_name.port || uci get portal.general.port)

portal_vlantag=`uci get portal.$section_name.vlantag`

wanIP=$HS_WANIP
wanMask=$HS_WANMASK
wanPrefix=$(ipcalc PREFIX $wanIP $wanMask)
wanNetwork=$HS_WANNETWORK
wanSegment="$wanNetwork"/"$wanPrefix"

uamnetwork=$(ipcalc NETWORK $HS_UAMLISTEN $HS_NETMASK)

# brguestIP=$(uci get network.guest.ipaddr)
# brguestMask=$(uci get network.guest.netmask)
# brguestNetwork=$(ipcalc NETWORK $brguestIP $brguestMask)

[ "$HS_BRIDGEMODE" = "on" ] && bridgemode=1 || bridgemode=0

[ -e /etc/chilli/ipv6up.sh ] && . /etc/chilli/ipv6up.sh


remotelog_enable=$(uci -q get system.@system[0].remotelog_enable)
trafficlog_enable=$(uci -q get system.@system[0].trafficlog_enable)
if [ "$remotelog_enable" = "1" -a "$trafficlog_enable" = "1" ]
then
ipt4 $iptable_name -t raw -I SN_LOGGING -i $TUNTAP -j RETURN
fi

ipt4 $iptable_name/rule -t filter -I $chilli_input_filter_rule -i $TUNTAP -d $wanIP -j DROP

if [ $bridgemode -eq 0 ]
then
ipt4 $iptable_name/rule -t filter -I $chilli_input_filter_rule -i $DHCPIF -d $wanIP -j DROP
ipt4 $iptable_name/rule -t nat -I $chilli_postrouting_nat_rule -s $HS_UAMLISTEN/$HS_NETMASK -o $HS_WANIF -j SNAT --to-source $wanIP
else
ipt4 $iptable_name/rule -t nat -I $chilli_postrouting_nat_rule -s $uamnetwork/$HS_NETMASK -o $HS_WANIF -j NETMAP --to $wanSegment
ipt4 $iptable_name/rule -t nat -I $chilli_postrouting_nat_rule -p icmp -s $HS_UAMLISTEN/$HS_NETMASK -o $HS_WANIF -j SNAT --to-source $wanIP
fi

vlan_mode=$(get_vlan_mode ${portal_vlantag:-0})
vlanbridge=$(get_vlan_wan_bridge ${portal_vlantag:-0})

if [ "$vlanbridge" != "br-lan" ]
then
    # check vlan interface again
    eth_vlanif=$(get_vlan_wan_ifname $portal_vlantag)
    eth_brname=$(get_bridge_name $eth_vlanif)
    if [ -n "$eth_brname" ]
    then
        vlanbridge=$eth_brname
    else
        echo ipup.sh: $eth_vlanif interface ERROR!! FIXME!! > /dev/console
    fi

    tablename=br-${section_name//_/}vlan
    tablenum=$((1200+$profile_index))

    cat /etc/iproute2/rt_tables |grep "^$tablenum[ \|"$'\t'"]" || echo -e "$tablenum\t$tablename" >> /etc/iproute2/rt_tables

    ip rule del pref $tablenum from $uamnetwork/$HS_NETMASK table $tablenum
    ip rule add pref $tablenum from $uamnetwork/$HS_NETMASK table $tablenum

    ip route replace default via $HS_WANGATEWAY dev $vlanbridge proto static table $tablenum
    ip route replace $wanSegment dev $vlanbridge table $tablenum
    ip route replace $uamnetwork/$HS_NETMASK proto static dev $TUNTAP table $tablenum
else
    tablename=portal-main
    tablenum=1200
    if [ -n "$HS_KNAME" ];then
        cat /etc/iproute2/rt_tables |grep "^1200[ \|"$'\t'"]" || echo -e "$tablenum\t$tablename" >> /etc/iproute2/rt_tables
        ip rule del pref $tablenum from $HS_UAMLISTEN/32 table $tablenum
        ip rule add pref $tablenum from $HS_UAMLISTEN/32 table $tablenum
    fi
fi

wallgardenF

if [ -n "$HS_KNAME" ]
then

    if [ "$is_ethernet" = "1" ]
    then
        local eth_name=$(get_ethprofile_ifname $CONFIG_NAME)

        if [ $bridgemode -eq 0 ]
        then
            mark_bridge_name=$DHCPIF
        else
            mark_bridge_name=$HS_WANIF
        fi

        if [ $is_support_syskey_mark -eq 0 ]
        then
            markvalue=$(get_mark_value $CONFIG_NAME 0)
            ipt2 $iptable_name -t mangle -I $portal_prerouting_mangle_x -i $mark_bridge_name -m physdev --physdev-in $eth_name --physdev-is-in -j MARK --set-xmark $markvalue
        fi
        # TODO: syskey not support postrouting mark
        # if [ $is_support_syskey_mark -eq 0 ]
        # then
        markvalue=$(get_mark_value $CONFIG_NAME 1)
        # NOTE: TEST for download, --physdev-is-out/--physdev-is-in/--physdev-is-bridged are the same, use --physdev-is-bridged to avoid kernel warning
        ipt2 $iptable_name -t mangle -I $portal_postrouting_mangle_x -o $mark_bridge_name -m physdev --physdev-out $eth_name --physdev-is-bridged -j MARK --set-xmark $markvalue
        # fi

    else
        local is_traiband=`uci -q get wireless.wifi4 && echo 1 || echo`
        for rad_idx in 0 1 ${is_traiband:+4}
        do
            local ath_idx=$(($section_id - 1))
            if [ "$ath_idx" = "0" ]
            then
                ath_name=ath$rad_idx
            else
                ath_name=ath$rad_idx$ath_idx
            fi
            if [ "$rad_idx" = "4" ]
            then
                local radio_mark_idx=2
            else
                local radio_mark_idx=$rad_idx
            fi

            if [ $bridgemode -eq 0 ]
            then
                mark_bridge_name=$DHCPIF
            else
                mark_bridge_name=$HS_WANIF
            fi

            if [ $is_support_syskey_mark -eq 0 ]
            then
                markvalue=$(get_mark_value $CONFIG_NAME 0 $radio_mark_idx)
                ipt2 $iptable_name -t mangle -I $portal_prerouting_mangle_x -i $mark_bridge_name -m physdev --physdev-in $ath_name --physdev-is-in -j MARK --set-xmark $markvalue
            fi
            # TODO: syskey not support postrouting mark
            # if [ $is_support_syskey_mark -eq 0 ]
            # then
            markvalue=$(get_mark_value $CONFIG_NAME 1 $radio_mark_idx)
            # NOTE: TEST for download, --physdev-is-out/--physdev-is-in/--physdev-is-bridged are the same, use --physdev-is-bridged to avoid kernel warning
            ipt2 $iptable_name -t mangle -I $portal_postrouting_mangle_x -o $mark_bridge_name -m physdev --physdev-out $ath_name --physdev-is-bridged -j MARK --set-xmark $markvalue
            # fi
        done
    fi

    if [ $bridgemode -eq 0 ]
    then
        for auth_ip in `ip route |grep $DHCPIF |awk '{print $1}'`
        do
            echo "ip route del $auth_ip dev $DHCPIF" > /dev/console
            ip route del $auth_ip dev $DHCPIF
        done
    else
        # autotest room find windows device will receive RST packets from AP, it will cause logint page cannot shown. so DROP RST packets here to WAR
        ipt4 $iptable_name -t mangle -I $portal_postrouting_mangle_x -o $TUNTAP -p tcp -m tcp --tcp-flags SYN,RST RST -j DROP
    fi

    markvalue=$(get_mark_value $CONFIG_NAME 0)
    ip rule del pref $tablenum fwmark $markvalue table $tablenum
    ip rule add pref $tablenum fwmark $markvalue table $tablenum
fi

if [ "`uci get wireless.wifi0_guest.disabled`" != "1" ]; then
	if [ "`uci get portal.general.enable`" = "1" ]; then
		portal_iface=$(uci get wireless.wifi0_guest.ifname)
		wlanconfig "$portal_iface" list sta | grep : | awk '{print $1}' | xargs -n 1 -r iwpriv "$portal_iface" kickmac
	fi
fi
if [ "`uci get wireless.wifi1_guest.disabled`" != "1" ];then
	if [ "`uci get portal.general.enable`" = "1" ]; then
		portal_iface=$(uci get wireless.wifi1_guest.ifname)
		wlanconfig "$portal_iface" list sta | grep : | awk '{print $1}' | xargs -n 1 -r iwpriv "$portal_iface" kickmac
	fi
fi

if [ "$HS_AWS_CLOUDFRONT" = "1" ]
then
    test -x $aws_ip_check && $aws_ip_check ${CONFIG_NAME}
fi

# facebook wifi
if [ "$HS_LOGIN_TYPE" = "108" ]
then
    test -x $fbwifi_wallgarden && $fbwifi_wallgarden ${CONFIG_NAME}
fi

finish_ipup() {
    local iptable_name=$1

    for ipttable in `ls -p $chilli_ipt_restore_dir/$iptable_name/ |grep -v "/$"`
    do
        local iptpath=$chilli_ipt_restore_dir/$iptable_name/$ipttable
        ipt4 $iptable_name -t $ipttable COMMIT
        ipt4_restore_rm 5 $iptpath
    done

    for ipttable in `ls -p $chilli_ipt_restore_dir/$iptable_name/rule/ |grep -v "/$"`
    do
        local iptpath=$chilli_ipt_restore_dir/$iptable_name/rule/$ipttable
        ipt4 $iptable_name/rule -t $ipttable COMMIT
        ipt4_restore_rm 5 $iptpath
    done


    for ipttable in `ls -p $chilli_ipt6_restore_dir/$iptable_name/ |grep -v "/$"`
    do
        local iptpath=$chilli_ipt6_restore_dir/$iptable_name/$ipttable
        ipt6 $iptable_name -t $ipttable COMMIT
        ipt6_restore_rm 5 $iptpath
    done

    for ipttable in `ls -p $chilli_ipt6_restore_dir/$iptable_name/rule/ |grep -v "/$"`
    do
        local iptpath=$chilli_ipt6_restore_dir/$iptable_name/rule/$ipttable
        ipt6 $iptable_name/rule -t $ipttable COMMIT
        ipt6_restore_rm 5 $iptpath
    done


    if [ -f "/usr/sbin/dnsmasq_restart" ]
    then
        /usr/sbin/dnsmasq_restart portal &
    else
        ( sleep 2 && /etc/init.d/dnsmasq restart ) &
    fi
    /etc/init.d/guestsyncd restart &
}

# echo CP: finish_ipup $CONFIG_NAME > /dev/console
finish_ipup $iptable_name

sh /usr/sbin/guest_network

if [ "$bridgemode" = "1" -o "$HS_LOGIN_TYPE" = "108" ]
then
    echo 1 > /proc/sys/net/bridge/bridge-nf-call-iptables
    if [ $is_support_syskey -eq 1 ]; then
        /etc/init.d/syskey setValue "bridge_nf_call_iptable" "portal" "1"
    fi
fi

if [ "$bridgemode" = "1" ]
then
    if [ "$is_ethernet" = "0" ]
    then
        for rad_idx in 4 1 0
        do
            ath_idx=$(($section_id - 1))
            if [ "$ath_idx" = "0" ]
            then
                ath_name=ath$rad_idx
            else
                ath_name=ath$rad_idx$ath_idx
            fi
            # if no interface,wlanconfig athx list will show Error
            if [ -d "/sys/class/net/$ath_name" ]
            then
                wifi_clients=$(wlanconfig $ath_name list |grep -v ^ADDR|awk '{printf $1"\n"}')
                for wifi_client in $wifi_clients
                do
                    # sync /proc/net/coova/ and chilli_query list
                    chilli_query -s /var/run/chilli.$CONFIG_NAME.sock dhcp-connect $wifi_client
                    # kick all clients
                    /usr/sbin/ban_kick_client.sh $ath_name $wifi_client 4 &
                done
            fi
        done
    fi
fi

if [ -d "$chilli_lock_file" ]
then
    echo $CONFIG_NAME >> $chilli_status_file
    if [ "`cat $chilli_status_file |sort -u`" = "`cat $chilli_reload_file |sort -u`" ]
    then
        rm -rf $chilli_status_file
        rm -rf $chilli_reload_file
        rm -rf $chilli_lock_file

        echo CP: Captive portal reload finished. > /dev/console
    fi
fi


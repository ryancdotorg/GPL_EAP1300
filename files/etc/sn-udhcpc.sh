#!/bin/sh
. /etc/sn-udhcpc-libs.sh

wan_brname=$interface
if [ "$wan_brname" = "br-99" ]
then
wan_vlantag=0
else
wan_ifname=`uci get network.${wan_brname#br-}.ifname`

if [ "$support_ssid_profile" = "1" ]; then
    wan_ifname=$(strip_ethprofile_interface $wan_ifname)
fi
wan_vlantag=${wan_ifname##*.}
fi

get_all_nat_profile_index() {
    local all_profile_index=""
    ### check nat_only vlan
    local gnmode_natonly_profile=$(get_gnmode_profile NAT_only)
    if [ -n "$gnmode_natonly_profile" ];then
        local natonly_profiles=""
        for profile in $gnmode_natonly_profile;do
            if [ "$natonly_profiles" = "${natonly_profiles/$profile/}" ];then
                natonly_profiles=${natonly_profiles:+$natonly_profiles }$profile
            fi
        done
    fi

    for natonly_profile in $natonly_profiles
    do
        local natonly_ssid_index=$(get_section_id $natonly_profile)
        local is_natonly_vlan_en=$(get_wireless_option ssid_$natonly_ssid_index isolation 1)

        if [ "${is_natonly_vlan_en:-0}" = "1" ]
        then
            local natonlyvlantag=$(get_wireless_option _ssid_$natonly_ssid_index vlan_id 0)
        else
            local natonlyvlantag=0
        fi
        if [ "$(get_vlan_wan_bridge $natonlyvlantag)" = "$wan_brname" ]
        then
            all_profile_index=$(get_uniq_sort_string $all_profile_index $natonly_ssid_index)
        fi
    done
    ###

    ### ethernet profile nat_only mode vlan check
    local ethprofile=$(get_ethernet_profile NAT_only)
    for profile in $ethprofile
    do
        local ethprofile_section="port_$(get_section_id $profile)"
        local is_ethprofile_vlan_en=$(uci get ethprofile.$ethprofile_section.vlan_enable)
        if [ ${is_ethprofile_vlan_en:-0} -eq 1 ]
        then
            local ethprofile_vlan=$(uci get ethprofile.$ethprofile_section.vlan_id)
        else
            local ethprofile_vlan=0
        fi
        if [ "$(get_vlan_wan_bridge $ethprofile_vlan)" = "$wan_brname" ]
        then
            profile_index=$(get_profile_index $profile)
            all_profile_index=$(get_uniq_sort_string $all_profile_index $profile_index)
        fi
    done
    ###

    ### check ldap_if vlan
    local ldap_if=$(get_ldap_if)

    for ldap_ifname in $ldap_if
    do
        local ldap_ssid_index=$(get_ssid_num $ldap_ifname)
        local is_ldap_ssid_vlan_en=$(get_wireless_option ssid_$ldap_ssid_index isolation 1)
        if [ ${is_ldap_ssid_vlan_en:-0} -eq 1 ]
        then
            local ldapifvlantag=$(get_wireless_option _ssid_$ldap_ssid_index vlan_id 0)
        else
            local ldapifvlantag=0
        fi
        if [ "$(get_vlan_wan_bridge $ldapifvlantag)" = "$wan_brname" ]
        then
            all_profile_index=$(get_uniq_sort_string $all_profile_index $ldap_ssid_index)
        fi
    done
    ###

    # echo NAT no portal[$all_profile_index] > /dev/console
    echo "$all_profile_index"

}

# echo wan_vlantag[$wan_vlantag] > /dev/console

setup_ip_rules(){
    local lan_ip=$1
    local lan_mask=$2
    local lan_br=$3
    local wan_br=$4
    local section_id=$5
    local is_guestvlan=$6
    local is_ethernet=$7
    local lan_network=$(ipcalc NETWORK $lan_ip $lan_mask)
    local lan_prefix=$(ipcalc PREFIX $lan_ip $lan_mask)

    if [ $is_guestvlan -eq 1 ]
    then
        local tablename=br-vlanguest
        local tablenum=1119
        #FIXME: not create guestvlan nat chain in firewall-re.sh
        local natChain=vlanguest_nat_rule
    elif [ $is_ethernet -eq 1 ]
    then
        #is_ethernet
        local tablename=br-port${section_id}vlan
        local tablenum=$((1200+8+$section_id))
        local natChain=port_${section_id}_nat_rule
    else
        local tablename=br-ssid${section_id}vlan
        local tablenum=$((1200+$section_id))
        local natChain=ssid_${section_id}_nat_rule
    fi

    cat /etc/iproute2/rt_tables |grep "^$tablenum[ \|"$'\t'"]" || echo -e "$tablenum\t$tablename" >> /etc/iproute2/rt_tables
    ip_rule_del_pref $tablenum
    ip rule add pref $tablenum from $lan_network/$lan_mask table $tablenum

    # setup lan
    ip route replace $lan_network/$lan_prefix dev $lan_br
    ip route replace $lan_network/$lan_prefix proto static dev $lan_br table $tablenum

    ip route replace $wan_network/$wan_prefix dev $wan_br table $tablenum
    ip route replace default via $router dev $wan_br proto static table $tablenum

    iptables -w -t nat -F $natChain
    iptables -w -t nat -I $natChain -j SNAT --to-source $wan_ip
    #
}

guestvlan_setting() {
    local all_profile_index="$(get_all_nat_profile_index)"
    if [ -n "$all_profile_index" ]
    then
        for profile_index in $all_profile_index
        do
            if [ $profile_index -gt 8 ]
            then
                #is_ethernet
                local section_id=$(($profile_index-8))
                local lan_br=br-nat${section_id}p
                local is_ethernet=1
                local lan_ip=`uci -q get network.nat${section_id}p.ipaddr`
                local lan_mask=`uci -q get network.nat${section_id}p.netmask`
            else
                local section_id=$profile_index
                local lan_br=br-nat$section_id
                local is_ethernet=0
                local lan_ip=`uci -q get network.nat${section_id}.ipaddr`
                local lan_mask=`uci -q get network.nat${section_id}.netmask`
            fi

            if [ -n "$lan_ip" -a -n "$lan_mask" ]
            then
                # use this setting
                :
            else
                # use nat default settting
                local lan_ip="172.$((15 + $profile_index)).1.1"
                local lan_mask="255.255.0.0"
            fi
            local eth_vlanif=$(get_vlan_wan_ifname $wan_vlantag)
            local eth_brname=$(get_bridge_name $eth_vlanif)
            if [ -n "$eth_brname" ]
            then
                local wan_br=$eth_brname
            else
                echo NAT_only: VLAN interface ERROR!! FIXME!! > /dev/console
                local wan_br=$wan_brname
            fi
            # echo "setup_ip_rules $lan_ip $lan_mask $lan_br $wan_br $section_id 0 ${is_ethernet:-0}" > /dev/console
            setup_ip_rules $lan_ip $lan_mask $lan_br $wan_br $section_id 0 ${is_ethernet:-0}
        done
    else
        return
    fi

    local is_guestvlan_en=$(uci -q get network.sys.GuestVLANEnable)
    local guestvlantag=$(uci -q get network.sys.GuestVLANID)

    # TODO: need to verify, is_guestvlan_en always disabled now
    if [ "${is_guestvlan_en:-0}" = "1" -a "${guestvlantag:-0}" != "0" ]
    then
        if [ "$wan_vlantag" = "$guestvlantag" ]
        then
            local lan_ip=$(uci get network.guest.ipaddr)
            local lan_mask=$(uci get network.guest.netmask)
            local lan_br=br-guest
            local eth_vlanif=$(get_vlan_wan_ifname $wan_vlantag)
            local eth_brname=$(get_bridge_name $eth_vlanif)
            if [ -n "$eth_brname" ]
            then
                local wan_br=$eth_brname
            else
                echo NAT_only: VLAN interface ERROR!! FIXME!! > /dev/console
                local wan_br=$wan_brname
            fi
            # FIXME: no iptable chain for guest
            setup_ip_rules $lan_ip $lan_mask $lan_br $wan_br 0 1
        fi
    fi
}

sysctl_set() {
    local ifname=$1
    local new_value=$2
    local old_value=$(cat /proc/sys/net/ipv4/conf/$ifname/forwarding 2>/dev/null)

    if [ -n "$new_value" -a "$old_value" != "$new_value" ]
    then
        sysctl -w net.ipv4.conf.$ifname.forwarding=$new_value
        echo "set $ifname forwarding = $new_value" > /dev/console
    fi
}

case "$1" in
    renew)
        # protect forwarding is enabled for NAT mode
        sysctl_set $wan_brname 1
        ;;
    bound)
        wan_ip=$ip
        wan_subnet=$subnet
        wan_network=$(ipcalc NETWORK $wan_ip $wan_subnet)
        wan_prefix=$(ipcalc PREFIX $wan_ip $wan_subnet)

        echo $wan_brname got ip:$wan_ip > /dev/console
        echo $wan_ip > /var/run/udhcpc-${wan_brname}.ip
        # enable forwarding for NAT mode
        sysctl_set $wan_brname 1

        # setup_main_wan_route
        ip addr flush dev $wan_brname
        ip addr add $wan_ip/$wan_subnet brd + dev $wan_brname
        ip route replace $wan_network/$wan_prefix dev $wan_brname
        # ip route replace default via $router dev $wan_brname proto static table $wan_brname

        if [ -e "/etc/init.d/l2_block" ]; then
            iface_network=${wan_brname#*-}
            sections=$(foreach wireless wifi-iface network $iface_network)
            for section in $sections
            do
                ifname=$(uci -q get wireless.${section}.ifname)
                /etc/init.d/l2_block set_block_rule $ifname "rand"
                /etc/init.d/l2_block set_block_rule $ifname "ssid"
            done
        fi

        if [ -e "/etc/init.d/portal" ]; then
            . /etc/chilli/chilli-libs.sh

            chilli_config_file=$(check_portal_config_vlan ${wan_vlantag})

            # echo chilli_config_file:[$chilli_config_file] > /dev/console

            if [ -z "$chilli_config_file" ]
            then
                guestvlan_setting
            else
                chilli_lock_counter=0
                while true
                do
                    chilli_lock_counter=$(($chilli_lock_counter+1))
                    if [ -e "/var/run/luci-reload-status" ]
                    then
                        sleep 2
                        continue
                    elif [ ! -d "$chilli_lock_file" ]
                    then
                        mkdir -p $chilli_lock_file
                        break
                    elif [ $chilli_lock_counter -gt 300 ]
                    then
                        echo "CP: FIXME!! reach chilli lock counter" > /dev/console
                        break
                    fi
                    sleep 1
                done
                rm -rf $chilli_status_file
                rm -rf $chilli_reload_file

                # wait portal reload finished
                guestvlan_setting
                # stop all chilli and run  /etc/chilli/down.sh & ipdown.sh
                for i in $chilli_config_file
                do
                    CONFIG_FILE=$(basename $(echo $i|sed 's#/config##'))

                    echo CP: $wan_brname got ip:$wan_ip, $CONFIG_FILE chilli restart > /dev/console
                    if [ -n "$CONFIG_FILE" ]
                    then
                        CONFIG_NAME=$CONFIG_FILE /etc/init.d/chilli stop
                        chilli_start_list="$chilli_start_list $CONFIG_FILE"
                        echo $CONFIG_FILE >> $chilli_reload_file
                    fi
                done

                echo "CP: ip conflict check... [$(skip_dev=$wan_brname is_ip_conflict $wan_ip $wan_subnet)]" > /dev/console
                # wait script done
                sleep 1

                dnsserver_1=${dns% *}
                dnsserver_2=${dns#$dnsserver_1 }
                echo dns1:[$dnsserver_1] > /dev/console
                echo dns2:[$dnsserver_2] > /dev/console

                for i in $chilli_config_file
                do
                    replace_config $i HS_WANIP $wan_ip
                    replace_config $i HS_WANNETWORK $wan_network
                    replace_config $i HS_WANMASK $wan_subnet
                    replace_config $i HS_WANGATEWAY $router
                    is_bridgemode=$(get_config_value $i HS_BRIDGEMODE)
                    if [ "$is_bridgemode" = "on" ]
                    then
                        replace_config $i HS_NETWORK $wan_network
                        replace_config $i HS_NETMASK $wan_subnet
                        replace_config $i HS_DHCPGATEWAY $router
                        replace_config $i HS_DHCPLISTEN $router
                        if [ -n "$dnsserver_1" ]
                        then
                            replace_config $i HS_DNS1 $dnsserver_1
                            if [ "$dnsserver_1" != "$dnsserver_2" ]
                            then
                                replace_config $i HS_DNS2 $dnsserver_2
                            fi
                        fi
                    fi
                done

                for chilli_config in $chilli_start_list
                do
                    CONFIG_NAME=$chilli_config /etc/init.d/chilli start
                done
            fi
        else
            guestvlan_setting
        fi

        test -f /etc/init.d/lsp && /etc/init.d/lsp check_lsp_ip &
        test -f /etc/init.d/appanalysis && /etc/init.d/appanalysis ap_address_update &
        ;;
    deconfig)
        ip addr flush dev $wan_brname
        # if no ipv4 ip, disable forwarding to avoid DHCP offer forwarding
        sysctl_set $wan_brname 0
        ;;
    *)
        ;;
esac



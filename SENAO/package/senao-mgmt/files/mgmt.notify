#!/bin/sh
. /etc/mgmt.d/mgmt_path.sh
iface=$1
mac=$2
act=$3

[ -f "$MGMT_POSTINIT_DONE" ] || exit 0

mgmt_ind_iface=$(/usr/sbin/foreach wireless wifi-iface network mgmt)

case "$iface" in
    "check")
        client_status=$(wlanconfig $MGMT_IFACE list sta|grep -v 'Error received'|grep :)
        if [ "$client_status" == "" ]; then
            [ "$mgmt_ind_iface" = "" ] && {
                $MGMT_DHCPD_SCRIPT "clear_dhcpd"
            }
            $MGMT_TIMER_SCRIPT "set_timer"
        else
            [ "$mgmt_ind_iface" = "" ] && {
                $MGMT_RULE_SCRIPT "set_rule"
                $MGMT_DHCPD_SCRIPT "set_dhcpd"
            }
            $MGMT_TIMER_SCRIPT "clear_timer"
        fi
    ;;
    "$MGMT_IFACE" )
        client_status=$(wlanconfig $MGMT_IFACE list sta|grep -v 'Error received'|grep :)
        case "$act" in
            "join")
                if [ "$client_status" != "" ]; then
                    [ "$mgmt_ind_iface" = "" ] && {
                        $MGMT_RULE_SCRIPT "set_rule"
                        $MGMT_DHCPD_SCRIPT "set_dhcpd"
                    }
                    $MGMT_TIMER_SCRIPT "clear_timer"
                fi
                ;;
            "left")
                if [ "$client_status" == "" ]; then
                    [ "$mgmt_ind_iface" = "" ] && {
                        $MGMT_DHCPD_SCRIPT "clear_dhcpd"
                    }
                    $MGMT_TIMER_SCRIPT "set_timer"
                fi
                ;;
        esac
    ;;
esac

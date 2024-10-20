#!/bin/sh
. /lib/functions.sh

MGMT_RULE_SCRIPT="/etc/mgmt.d/mgmt_rule.sh"
MGMT_DHCPD_SCRIPT="/etc/mgmt.d/mgmt_dhcpd.sh"
MGMT_TIMER_SCRIPT="/etc/mgmt.d/mgmt_timer.sh"
MGMT_NOTIFY_SCRIPT="/etc/assoc_notify.d/mgmt_notify.sh"
MGMT_VAP_SCRIPT="/etc/mgmt.d/mgmt_vap.sh"
mgmt_delRule="/tmp/mgmt_delRule"
DHCPD_CONF="/etc/dhcpd.conf"
DHCPD_SCRIPT="/etc/init.d/dhcpd"
MGMT_HOSTAPD_CONF=/var/run/hostapd-mgmt*.conf
MGMT_BRIDGE="br-lan"
MGMT_IFACE=
MGMT_SECTION=
MGMT_RADIO=
MGMT_DISABLED=
MGMT_POSTINIT_DONE="/var/run/mgmt/postinit_done"
MGMT_FIRST_BOOT_FILE="/var/run/mgmt/first_boot"

findMgmtSsid() {
    local cfg="$1"
    config_get ifname "$cfg" ifname
    config_get device "$cfg" device
    config_get disabled "$cfg" disabled
    case $ifname in
        mgmt*)
            MGMT_IFACE=$ifname
            MGMT_SECTION=$cfg
            MGMT_RADIO=$device
            MGMT_DISABLED=$disabled
            return
            ;;
    esac
}
#MGMT_IFACE=$(uci show wireless | grep -e 'mgmt.*\.ifname' | awk -F '=' '{print $2}')
config_load wireless
config_foreach findMgmtSsid  wifi-iface



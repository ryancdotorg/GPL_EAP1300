#!/bin/sh
. /etc/mgmt.d/mgmt_path.sh
DHCP_START=150
DHCP_LIMIT=50
uci_lan_ip=
uci_lan_mask=
cfg_lan_network=
cfg_dhcp_start=
cfg_dhcp_end=

GEN_DHCPD_CONF () {
    echo "HELOO"
    echo > $DHCPD_CONF
        cat >> $DHCPD_CONF <<EOF
# dhcpd.conf

authoritative;

default-lease-time 3600;
max-lease-time 86400;

option domain-name-servers $uci_lan_ip;

subnet $cfg_lan_network netmask $uci_lan_mask {
  range $cfg_dhcp_start $cfg_dhcp_end;
  option routers $uci_lan_ip;
}
EOF
}

RM_DHCPD_CONF () {
 [ -f $DHCPD_CONF ] && rm -f $DHCPD_CONF
}

CHECK_DHCPD_CONF(){
    uci_lan_ip=$(uci get network.lan.ipaddr)
    echo "$uci_lan_ip"
    uci_lan_mask=$(uci get network.lan.netmask)
    echo "$uci_lan_mask"

    cfg_lan_ip=$(cat $DHCPD_CONF | grep 'domain-name-servers'| sed -e 's/\;//g' | awk '{print $3}')
    echo "$cfg_lan_ip"

    cfg_lan_mask=$(cat $DHCPD_CONF | grep netmask | sed -e 's/\;//g' | awk '{print $4}')
    echo "$cfg_lan_mask"


   if [ "$cfg_lan_mask" == "$uci_lan_mask" -a "$cfg_lan_ip" == "$uci_lan_ip" ]; then
        return
   else
        cfg_lan_ip=$uci_lan_ip
        cfg_lan_mask=$uci_lan_mask
   fi


    cfg_lan_network=$(ipcalc.sh $uci_lan_ip $uci_lan_mask | grep NETWORK | awk -F '=' '{print $2}')
    echo "$cfg_lan_network"
    cfg_dhcp_start=$(ipcalc.sh $uci_lan_ip $uci_lan_mask $DHCP_START $DHCP_LIMIT | grep START | awk -F '=' '{print $2}')
    echo "$cfg_dhcp_start"
    cfg_dhcp_end=$(ipcalc.sh $uci_lan_ip $uci_lan_mask $DHCP_START $DHCP_LIMIT | grep END | awk -F '=' '{print $2}')
    echo "$cfg_dhcp_end"
    GEN_DHCPD_CONF

}

case "$1" in
    "set_dhcpd")
        CHECK_DHCPD_CONF
        $DHCPD_SCRIPT reload
    return
    ;;
    "clear_dhcpd")
        $DHCPD_SCRIPT stop
        RM_DHCPD_CONF
    return
    ;;
esac

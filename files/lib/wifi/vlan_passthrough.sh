#!/bin/sh
#. /etc/functions.sh

##### Rule : When wds ap or wds bridge enable/disable set proc vlan_passthrough enable/disable

vlan_pass_proc="/proc/sys/net/8021q/vlan_passthrough"
wds_iface_proc="/proc/sys/net/8021q/wds_ifname"

vlanpass_wds()
{
	local ifname_list="$1"

	if [ -n "$ifname_list" ]; then
		[ -e "$vlan_pass_proc" ] && echo 1 > $vlan_pass_proc
		[ -e "$wds_iface_proc" ] && echo "$ifname_list" > $wds_iface_proc
	else
		[ -e "$vlan_pass_proc" ] && echo 0 > $vlan_pass_proc
		[ -e "$wds_iface_proc" ] && echo '' > $wds_iface_proc
	fi
}

vlanpass_mesh()
{
	# TODO
	[ -e "$vlan_pass_proc" ] && echo "1" > ${vlan_pass_proc}
	[ -e "$wds_iface_proc" ] && echo "bat0 " > ${wds_iface_proc}
}


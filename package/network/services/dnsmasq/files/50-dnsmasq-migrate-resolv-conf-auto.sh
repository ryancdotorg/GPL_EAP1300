#!/bin/sh

# this file is prepare for downgrade fw from new dnsmasq to old dnsmasq use ...
resolvfile=$(uci -c /rom/etc/config get dhcp.@dnsmasq[0].resolvfile)
[ -n "$resolvfile" -a "$(uci get dhcp.@dnsmasq[0].resolvfile)" != "$resolvfile" ] && {
	uci set dhcp.@dnsmasq[0].resolvfile="$resolvfile"
	uci commit dhcp
}

# [ "$(uci get dhcp.@dnsmasq[0].resolvfile)" = "/tmp/resolv.conf.auto" ] && {
# 	uci set dhcp.@dnsmasq[0].resolvfile="/tmp/resolv.conf.d/resolv.conf.auto"
# 	uci commit dhcp
# }

exit 0

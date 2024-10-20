#!/bin/sh

. /lib/functions.sh
. /lib/functions/service.sh

disable_wireless_guest_network() {
	local vif="$1"

	uci set wireless."$vif".guest_network="Disable"
}

disalbe_guest_network() {
        config_load wireless
	config_foreach disable_wireless_guest_network wifi-iface
}

disalbe_wireless_portal() {
	local vif="$1"

	uci set portal."$vif".enable=0
}

disalbe_portal() {
        config_load portal
	config_foreach disalbe_wireless_portal portal	
}

standalone_config() {
	disalbe_guest_network
	disalbe_portal
}

case "standalone" in
    standalone) standalone_config;;
esac

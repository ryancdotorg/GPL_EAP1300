
target_ap_macaddress=$1

if [ -z $1 ]; then

	echo conn_app_ssid_dbg.sh [TARGET AP MAC address]
	exit

fi

mac_str="$(echo $target_ap_macaddress |tr [a-z] [A-Z]|sed 's/://g')"
mac_str6="$(echo $mac_str|cut -c 7-12)"
mac_str8="$(echo $mac_str|cut -c 5-12)"
hiddenAppKey="$(echo $mac_str8$mac_str8$mac_str8$mac_str8 | tr -d '\n' | md5sum | awk {'print $1'} | tr -d '\n')"
echo ".ssid=ENMGMT$mac_str6"
echo ".key=$hiddenAppKey"

cat << EOF > /var/run/wpa_supplicant-ath2.conf 
ctrl_interface=/var/run/wpa_supplicant-ath2

pmf=

network={
        scan_ssid=1
        ssid="ENMGMT$mac_str6"
        key_mgmt=WPA-PSK WPA-PSK-SHA256
        proto=RSN
        ieee80211w=1
        psk="$hiddenAppKey"
        pairwise=CCMP 
        group=CCMP TKIP
        multi_ap_backhaul_sta=
}
EOF

wifi down
wifi detect

#cfg80211tool wifi1 bsta_fixed_idmask 255
#cfg80211tool wifi1 set_bcnburst 1
#cfg80211tool wifi1 obss_rxrssi_th 35
#cfg80211tool wifi1 sIgmpDscpOvrid 1
#cfg80211tool wifi1 sIgmpDscpTidMap 6
#cfg80211tool wifi1 enable_ol_stats 1
#cfg80211tool wifi1 txbf_snd_int 100
#cfg80211tool wifi1 obss_rssi_th 35
#cfg80211tool wifi1 discon_time 10
#cfg80211tool wifi1 reconfig_time 60
#cfg80211tool wifi1 fc_buf0_max 8192
#cfg80211tool wifi1 fc_buf1_max 8192
#cfg80211tool wifi1 fc_buf2_max 8192
#cfg80211tool wifi1 fc_buf3_max 8192
#cfg80211tool wifi1 burst 1
#cfg80211tool wifi1 dcs_enable 0
#cfg80211tool wifi1 enable_ol_stats 1
ifconfig wifi1 up

wlanconfig ath2 create wlandev wifi1 wlanmode sta -cfg80211
iw phy "$(cat /sys/class/net/wifi1/phy80211/name)" interface add ath2 type managed
iw ath2 set 4addr on >/dev/null 2>&1


wpa_cli -g /var/run/wpa_supplicantglobal interface_add ath2 /var/run/wpa_supplicant-ath2.conf nl80211 /var/run/wpa_supplicant-ath2  br-lan

cfg80211tool ath2 extap 1
cfg80211tool ath2 mode 11AHE80
cfg80211tool ath2 channel 0
cfg80211tool ath2 disablecoext 1
cfg80211tool ath2 chwidth 2
cfg80211tool ath2 wds 0
cfg80211tool ath2 shortgi 0
cfg80211tool ath2 uapsd 1
cfg80211tool ath2 backhaul 0
cfg80211tool ath2 stafwd 0
cfg80211tool ath2 he_ulofdma 0
cfg80211tool ath2 wmm 1
cfg80211tool ath2 doth 1
cfg80211tool ath2 ssid ENMGMT$mac_str6

ifconfig ath2 192.168.101.33 up



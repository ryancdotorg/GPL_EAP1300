#!/bin/sh

if [ "$1" == "-h" ] || [ "$1" == "" ];then
	echo "-r [24g|5g|5g2]"
    echo "-e [none|wpa2mix]"
    echo "-p [password]"
    echo "-s [ssid]"
    echo "Ex: cb.sh -r 24g -e none -s ssid"
    echo "Ex: cb.sh -r 5g -e wpa2mix -p 12345678 -s ssid"
    exit 1
fi

    while getopts "r:e:p:s:h" option
    do
        case "${option}" in
            r)  radio="${OPTARG}"
                ;;
            e)  enc="${OPTARG}"
                ;;
            p)  pwd="${OPTARG}"
                ;;
            s)  ssid="${OPTARG}"
                ;;                
            h)  exit
                ;;                
        esac
    done
    
    echo $radio $enc $pwd $ssid

if [ "$enc" == "none" ];then
	encryption="none"
elif [ "$enc" == "wpa2mix" ];then
	encryption="psk2+ccmp"
else
	encryption="$enc"
fi

#24G CB
if [ "$radio" == "24g" ] || [ "$1" == "" ];then
	echo "[CB DEBUG] Run 2.4G CB mode, encrypt=$enc, SSID=$ssid"	
	[ $(uci get wireless.wifi0_sta) ] && {
		echo "[CB DEBUG] wireless.wifi0_sta exist!"
		uci set wireless.wifi0.opmode=sta
		uci set wireless.wifi0_sta.disabled=0
		uci set wireless.wifi1_sta.disabled=1
		`uci set wireless.wifi0_sta.ssid=$ssid`
		`uci set wireless.wifi0_sta.encryption=$encryption`
		`uci set wireless.wifi0_sta.key=$pwd`

	} || {
		echo "[CB DEBUG] wireless.wifi0_sta not exist!"
		uci set wireless.wifi0.opmode=sta
		uci set wireless.wifi0_sta=wifi-iface
		uci set wireless.wifi0_sta.disabled=0
		uci set wireless.wifi1_sta.disabled=1
		uci set wireless.wifi0_sta.device=wifi0
		uci set wireless.wifi0_sta.network=lan
		uci set wireless.wifi0_sta.mode=sta
		`uci set wireless.wifi0_sta.ssid=$ssid`
		`uci set wireless.wifi0_sta.encryption=$encryption`
		`uci set wireless.wifi0_sta.key=$pwd`
		uci set wireless.wifi0_sta.bintval=100
		uci set wireless.wifi0_sta.protmode=0
		uci set wireless.wifi0_sta.dtim_period=2
		uci set wireless.wifi0_sta.frag=2346
		uci set wireless.wifi0_sta.preamble=0
		uci set wireless.wifi0_sta.nawds=0
		uci set wireless.wifi0_sta.wds=0
		uci set wireless.wifi0_sta.mode_display=sta
		uci set wireless.wifi0_sta.ifname=ath26
		uci set wireless.wifi0_sta.preferbssid_enable=0
	}
	luci-reload auto
fi

#5G CB
if [ "$radio" == "5g" ] || [ "$1" == "" ];then
	echo "[CB DEBUG] Run 5G CB mode, encrypt=$enc, SSID=$ssid"	

	[ $(uci get wireless.wifi1_sta) ] && {
		echo "[CB DEBUG] wireless.wifi1_sta exist!"
		uci set wireless.wifi1.opmode=sta
		uci set wireless.wifi0_sta.disabled=1
		uci set wireless.wifi1_sta.disabled=0
		`uci set wireless.wifi1_sta.ssid=$ssid`
		`uci set wireless.wifi1_sta.encryption=$encryption`
		`uci set wireless.wifi1_sta.key=$pwd`		
	} || {
		echo "[CB DEBUG] wireless.wifi1_sta not exist!"
		uci set wireless.wifi1.opmode=sta
		uci set wireless.wifi1_sta=wifi-iface
		uci set wireless.wifi0_sta.disabled=1
		uci set wireless.wifi1_sta.disabled=0
		uci set wireless.wifi1_sta.device=wifi1
		uci set wireless.wifi1_sta.network=lan
		uci set wireless.wifi1_sta.mode=sta
		`uci set wireless.wifi1_sta.ssid=$ssid`
		`uci set wireless.wifi1_sta.encryption=$encryption`
		`uci set wireless.wifi1_sta.key=$pwd`
		uci set wireless.wifi1_sta.bintval=100
		uci set wireless.wifi1_sta.protmode=0
		uci set wireless.wifi1_sta.dtim_period=2
		uci set wireless.wifi1_sta.frag=2346
		uci set wireless.wifi1_sta.preamble=0
		uci set wireless.wifi1_sta.nawds=0
		uci set wireless.wifi1_sta.wds=0
		uci set wireless.wifi1_sta.mode_display=sta
		uci set wireless.wifi1_sta.ifname=ath56
		uci set wireless.wifi1_sta.preferbssid_enable=0
	}
	luci-reload auto
fi


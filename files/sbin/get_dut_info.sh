#!/bin/sh

[ -z "$1" ] && echo get_dut_info.sh [ssid_index] && exit 
[ $1 -lt 1 -o $1 -gt 8 ] && echo ssid index 1 to 8 && exit

for radio_index in 0 1; do
uci_config="\
   opmode:wireless.wifi${radio_index}.opmode\
   radio_disable:wireless.wifi${radio_index}_ssid_$1.disabled\
   mesh_disable:wireless.wifi${radio_index}_mesh.disabled
   "
echo ================ wifi${radio_index} SSID$1 ============= 
for i in $uci_config; do
echo ${i%%:*} = $(uci get ${i#*:}) 
done
done
echo ==========================================
uci_config="\
   vlan_isolation:wireless.wifi1_ssid_$1.isolation\
   l2_isolation:wireless.wifi1_ssid_$1.l2_isolatior\
   traffic_shapping:wireless.wifi1_ssid_$1.tc_enabled\
   app_dection:ndpi.ssid_$1.enable\
   security_mode:wireless.wifi1_ssid_$1.encryption\
   accounting:wireless.wifi1_ssid_$1.acct_enabled\
   11r:wireless.wifi1_ssid_$1.fastroamingEnable  
   "
for i in $uci_config; do
echo ${i%%:*} = $(uci -q get ${i#*:}) 
done

mode=$(uci get wireless.wifi1_ssid_$1.guest_network)
if [ "$mode" == "Disable" ];then
	mode_result=Bridge
	portal_result=disable
elif [ "$mode" == "NAT_only" ];then
	mode_result=NAT
	portal_result=disable
elif [ "$mode" == "NAT" ];then
	mode_result=NAT
	portal_result=enable
elif [ "$mode" == "Bridge" ];then
	mode_result=Bridge
	portal_result=enable
fi

echo mode = $mode_result 
echo portal = $portal_result 

if [ "$portal_result" == "enable" ];then
portal=$(uci -q get portal.ssid_$1.loginType)

case "$portal" in	
#login-type
	0) cp_result="splash and go";;
	1) cp_result="ezmaster authentication";;
	2) cp_result="radius server";;
	3) cp_result="third party authentication";;
	4) cp_result="social login";;
	#login-type-cloud
	100) cp_result="click through";;
	101) cp_result="engenius radius";;
	102) cp_result="voucher service";;
	103) cp_result="custom radius";;
	104) cp_result="3rd party";;
	105) cp_result="social login";;
esac
echo portal_auth = $cp_result 
fi

bs=$(uci -q get wireless.wifi0_ssid_$1.bandsteer_en)

case "$bs" in
	0)	bs_result=disable;;
	1)	bs_result=force;;
	2)  bs_result=prefer5G;;
	3)  bs_result=bandbalance;;
esac
echo band_steering = $bs_result 
echo ==========================================


#!/bin/sh
#
# Copyright (c) 2014, 2016, The Linux Foundation. All rights reserved.
#

. /lib/sn_functions.sh
. /lib/senao-shell-libs/network.sh

funcDebug=0
default_cfg80211=0
wds_enable=0
wifi_profile="/lib/wifi/wifi_profile"
sn_wifi_profile="/lib/wifi/sn_wifi_profile"
#qca_wifi_profile="/lib/wifi/qca_wifi_profile"  #QCA configs, no need now

hostap_profile="/lib/wifi/hostapd_profile"
sn_hostap_profile="/lib/wifi/sn_hostapd_profile"

wifi_prof_config_name="wifiprofile"
hostap_prof_config_name="hostapprofile"

snConfig="snWifiConf"
snHostapConfig="snHostapConf"

wifi_prof_config="/etc/config/$wifi_prof_config_name"
hostap_prof_config="/etc/config/$hostap_prof_config_name"

PROC_SN_ROLE="/proc/sn_role"

DebugPrint(){
        [ "$funcDebug" == "1" ] && echo "$@" > /dev/console
}

wlanconfig() {
        [ "$funcDebug" == "1" ] && echo wlanconfig "$@"
        /usr/sbin/wlanconfig "$@"
}

iwconfig() {
        [ "$funcDebug" == "1" ] && echo iwconfig "$@"
        /usr/sbin/iwconfig "$@"
}

iwpriv() {
        [ "$funcDebug" == "1" ] && echo iwpriv "$@"
        /usr/sbin/iwpriv "$@"
}

device_if() {
	if [ "$default_cfg80211" == "1" ]; then
		cfg80211tool "$@"
	else
		iwpriv "$@"
	fi
}

delay_cmd() {
	[ "$funcDebug" == "1" ] && echo sleep "$@"
        delay=$1
        shift
        ( sleep $delay && $@ ) &
}

insmod() {
	[ "$funcDebug" == "1" ] && echo insmod "$@"
	/usr/sbin/insmod "$@"
}


get_sn_wifi_option() {

	if [ ! -f "$wifi_prof_config" ]; then
	{
		echo 1		#if there is no wifi profile config, we assume all the options are 1 to avoid function failed
		return
	}
	fi
	option=$1
	
	ret=$(uci -q get $wifi_prof_config_name"."$snConfig"."$option);
	
	if [ "$ret" == "1" ]; then 
		echo 1
	else
		DebugPrint "$option not equal to 1"
		echo 0
	fi
}

get_sn_hostap_option() {

        if [ ! -f "$hostap_profile" ]; then
        {
                echo 1          #if there is no hostapd profile config, we assume all the options are 1 to avoid function failed
                echo "there is not get hostap profile config from $hostap_profile." > /dev/console
                return
        }
        fi
        option=$1

        ret=$(cat $hostap_profile | grep $option)

        if [ -z "$ret" ]; then
                echo 0
                return
        fi

        ret=$(cat $hostap_profile | grep $option | grep "#")

        if [ -z "$ret" ]; then
                echo 1
        else
                DebugPrint "$option not enabled"
                echo 0
        fi
}

addOption() {
        optionName=$1
        optionValue=$2
	prof_config=$3
        printf "\toption %-40s '%s'\n" $optionName $optionValue >> $prof_config
}

wifiProfile2Config(){
	if [ ! -s "$wifi_prof_config" ]; then
        #if wifi profile config already exists, no need do it
        #this config used for reference, the value should not be altered

	        if [ -s "$wifi_profile" ]; then

	                cat $wifi_profile | sed 's/^#.*//g' | grep "SENAO" > $sn_wifi_profile     #prepare sn_wifi_profile, it only contain SENAO_XXXX 

	                #cp $wifi_profile $qca_wifi_profile     
	                #sed -i '/SENAO/d' $qca_wifi_profile    #QCA configs, no need now
	        fi

        	if [ -s "$sn_wifi_profile" ]; then

			echo "config profile $snConfig" > $wifi_prof_config

                	lineNum=$(sed -n '$=' "$sn_wifi_profile")

                	for i in `seq 1 $lineNum`; do
                	        optionName=$(cat $sn_wifi_profile | head -n $i |tail -n 1 | awk -F 'SENAO_' '{print $2}'|awk -F '=' '{print $1}')
                	        optionValue=$(cat $sn_wifi_profile | head -n $i |tail -n 1 | awk -F 'SENAO_' '{print $2}'|awk -F '=' '{print $2}')
                	        [ "$optionValue" == "1" ] && addOption $optionName $optionValue $wifi_prof_config
                	done

		fi
	fi
}

HostapdProfile2Config(){
	if [ ! -s "$hostap_prof_config" ]; then

		if [ -s "$hostapd_profile" ]; then
			cat $hostap_profile | sed 's/^#.*//g' | grep "SENAO" > $sn_hostap_profile
		fi
		
		if [ -s "$sn_hostap_profile" ]; then

			echo "config profile $snHostapConfig" > $hostap_prof_config

			lineNum=$(sed -n '$=' "$sn_hostap_profile")
			
			for i in `seq 1 $lineNum`; do
				optionName=$(cat $sn_hostap_profile | head -n $i |tail -n 1 | awk -F 'SENAO_' '{print $2}'|awk -F '=' '{print $1}')
				optionValue=$(cat $sn_hostap_profile | head -n $i |tail -n 1 | awk -F 'SENAO_' '{print $2}'|awk -F '=' '{print $2}')
				[ "$optionValue" == "y" ] && addOption $optionName "1" $hostap_prof_config
			done
		fi

	fi 		
}

get_module_parameter() {
	moduleName=$1
	paramName=$2

	value=$(cat /sys/module/$moduleName/parameters/$paramName)

	if [ -n $value ]; then
		echo $value
	else
		echo 0
	fi
}

hostapd_global_restart() {

        if  [ -e "/var/run/hostapd-global.pid" ]; then
                kill $(cat /var/run/hostapd-global.pid) &> /dev/null
                rm /var/run/hostapd-global.pid &> /dev/null
        
	        hostapd -g /var/run/hostapd/global -B -P /var/run/hostapd-global.pid
	fi
}

dfs_spoof() {
	support_dfs_spoof=$(get_sn_wifi_option SUPPORT_DFS_SPOOF)
	[ "$support_dfs_spoof" == "0" ] && return

	local device="$1"

	[ "$device" == "wifi0" ] && return

	config_get dfs_ignore "$device" dfs_ignore 0
	iwpriv "$device" dfs_ignore "$dfs_ignore"

}

green_mode() {
        local device="$1"
        local domain=$(setconfig -g 4)
        local obey=$(uci get wireless.$device.obeyregpower)
        local country=$(uci get wireless.$device.country)
        local outdoor=$(uci -q get sysProductInfo.model.outdoor || echo -n "0")
        local dfsCertified=0

        # domain 0 = FCC
        # domain 1 = ETSI/EU
        # domain 2 = INT/others
        if [ "$obey" = "undefined" ]; then
                obey=1
                case ${domain} in
                        0)
                                country=840
                        ;;
                        1)
                                country=528
                        ;;
                        2)
                        ;;
                        *)
                        ;;
                esac
                uci set wireless.${device}.obeyregpower="$obey"
                uci set wireless.${device}.country="$country"
                uci commit wireless
        fi


	[ "$device" == "wifi0" ] && return

	if [ -f "/lib/wifi/RegularDomain.sh" ]; then
	{

	        local check_wifi2=$(/usr/sbin/foreach wireless wifi-device disabled 0| grep wifi2) #jacky added, to check the platform has wifi2 or not
	        local wifi2_opmode=$(uci -q get wireless.wifi2.opmode)

        	if [ "$wifi2_opmode" != "mon" -a -n "$check_wifi2" ]; then
                	local val=$(sh /lib/wifi/RegularDomain.sh $country $domain $obey $outdoor $dfsCertified $check_wifi2)
        	else
                	local val=$(sh /lib/wifi/RegularDomain.sh $country $domain $obey $outdoor $dfsCertified)
        	fi

	        local disable_band=$(echo $val | cut -d ' ' -f 1)
	        local weather_ch=$(echo $val | cut -d ' ' -f 2) # whether support weather radar channel
	
		support_dis_weather_ch=$(get_sn_wifi_option SUPPORT_DISABLE_WEATHER_RADAR_CHANNEL)
	
	        [ "$support_dis_weather_ch" == "0" ] && iwpriv "$device" weather_ch $weather_ch
	
	        [ -n "$check_wifi2" ] && {
	                local split_band=$(uci -q get wireless.$device.split_band) #Jacky:used for tri-band platform, this variable defined by vendor using
	
        	        case ${device} in
        	                "wifi1")
        	                        [ -z "$split_band" ] && split_band=12   #12 means band3+band4=4+8=12=high band, channel100~165, default tri-band setting
        	                        disable_band=$(($disable_band & $split_band))
        	                        ;;
        	                "wifi2")
        	                        [ -z "$split_band" ] && split_band=3    #3 means band1+band2=1+2=3=low band, channel36~64, default tri-band setting
        	                        disable_band=$(($disable_band & $split_band))
        	                        ;;
        	        esac
        	}

        	[ "$disable_band" == "0" ] && {
			config_set "$device" nochannel 1
        	        uci set wireless.$device.nochannel="1"
        	        uci commit wireless
        	} || {
			config_set "$device" nochannel 0
        	        uci set wireless.$device.nochannel="0"
        	        uci commit wireless
        	        iwpriv "$device" disable_band "$disable_band"
        	}
	}
	fi
}

TEMP_NOCTL_INFO_FILE="/tmp/noCtlFiles"

searchNextBoardData(){

        numOfSearch=$(cat $TEMP_NOCTL_INFO_FILE | grep $1 -c)

        if [ $numOfSearch -eq 1 ]; then
                noCTLBoardData=$(cat $TEMP_NOCTL_INFO_FILE | grep $1)
                sed -i '/'"$noCTLBoardData"'/d' $TEMP_NOCTL_INFO_FILE
                shift
                keyWordPriority=$@
                echo $noCTLBoardData
        elif [ $numOfSearch -eq 0 ]; then
                shift
                [ -n "$1" ] && searchNextBoardData "$@"
        else    #found more than 1 file with this keyword
                noCTLBoardData=$(cat $TEMP_NOCTL_INFO_FILE | grep $1 | grep $2 -c)
                shift
                searchNextBoardData "$@"
        fi
}

check_boarddata() {
    local wifi_dev=$1
    local FIRMWARE_PATH="/lib/firmware/"
    local keyWordPriority="2G wifi0 IPQ4019 5G wifi1" #key word search priority

    ls -R $FIRMWARE_PATH | grep noCTL > $TEMP_NOCTL_INFO_FILE
                                                                           
    numOfNoCTL=$(ls -R $FIRMWARE_PATH | grep -c noCTL)
    config_get country "wifi0" country #just get wifi0 country since all radio should be in the same country
                                                                                                   
    for count in `seq 0 $(($numOfNoCTL-1))`; do                                            
                               
	noCTLBData=$(searchNextBoardData $keyWordPriority)

	[ -z "$noCTLBData" ] && continue

	CTLBData=$(echo $noCTLBData | awk -F '_noCTL' '{print $1}')".bin"
	noCTLBDataPath=$(find $FIRMWARE_PATH -name $noCTLBData)
	CTLBDataPath=$(echo $noCTLBDataPath | awk -F '_noCTL' '{print $1}')".bin"
	CountryBDataPath=$(echo $noCTLBDataPath | awk -F '_noCTL' '{print $1}')"_sn_${country}.bin"

	drv_load_data=$CTLBDataPath

	obey=$(uci get wireless.$wifi_dev.obeyregpower)
        [ "$obey" == "undefined" ] && obey=1

	if [ "$obey" = "1" ]; then
		if [ -f "$CountryBDataPath" ]; then
		    rom_boarddata="/rom${CountryBDataPath}"
		else
		    rom_boarddata="/rom${CTLBDataPath}"
		fi
	else
		rom_boarddata="/rom${noCTLBDataPath}"
	fi

        [ -n "$rom_boarddata" ] && [ -n "$drv_load_data" ] && ln -sf $rom_boarddata $drv_load_data
        DebugPrint "rom_boarddata=$rom_boarddata, drv_load_data=$drv_load_data"
    done  

    [ -f "$TEMP_NOCTL_INFO_FILE" ] && rm $TEMP_NOCTL_INFO_FILE -f
}

check_boarddata_multi_project() {
	local wifi_dev=$1
	local FIRMWARE_PATH="/lib/firmware/QCA9888/hw.2/"
	local keyWordPriority="2G wifi0 IPQ4019 5G wifi1" #key word search priority
	local modelname=$(cat /etc/modelname)

	ls $FIRMWARE_PATH | grep noCTL > $TEMP_NOCTL_INFO_FILE

	numOfNoCTL=$(ls $FIRMWARE_PATH | grep -c noCTL)
	for count in `seq 0 $(($numOfNoCTL-1))`; do
		noCTLBData=$(searchNextBoardData $keyWordPriority)
		[ -z "$noCTLBData" ] && continue

		CTLBData=$(echo $noCTLBData | awk -F '_noCTL' '{print $1}')".bin"
		CTLBSTAData=$(echo $noCTLBData | awk -F '_noCTL' '{print $1}')"_sta.bin"
		noCTLBDataPath=$(find ${FIRMWARE_PATH}${modelname} -name $noCTLBData)
		CTLBDataPath=$(echo $noCTLBDataPath | awk -F '_noCTL' '{print $1}')".bin"
		CTLBSTADataPath=$(find ${FIRMWARE_PATH}${modelname} -name $CTLBSTAData)
		drv_load_data=${FIRMWARE_PATH}${CTLBData}

		opmode=$(uci get wireless.$wifi_dev.opmode)
		obey=$(uci get wireless.$wifi_dev.obeyregpower)
		[ "$obey" == "undefined" ] && obey=1

		if [ "$obey" == "1" ]; then
			rom_boarddata="/rom${CTLBDataPath}"
			if  [ "$opmode" == "sta" -o "$opmode" == "wds_sta" ] && [ -f "$CTLBSTADataPath" ]; then
				rom_boarddata="/rom${CTLBSTADataPath}"
			fi
		else
			rom_boarddata="/rom${noCTLBDataPath}"
		fi

		[ -n "$rom_boarddata" ] && [ -n "$drv_load_data" ] && ln -sf $rom_boarddata $drv_load_data
		DebugPrint "rom_boarddata=$rom_boarddata, drv_load_data=$drv_load_data"
	done

	[ -f "$TEMP_NOCTL_INFO_FILE" ] && rm $TEMP_NOCTL_INFO_FILE -f
}

check_offload() {
    support_tdma=$(get_sn_wifi_option SUPPORT_TDMA)
    [ "$support_tdma" == "0" ] && return

    qboost=$(uci get wireless.wifi1.qboost_enable)
    enjet_disabled=$(uci get wireless.wifi1_enjet.disabled)

    [ -z "$qboost" -o -z "$enjet_disabled" ] && return

    local FIRMWARE_PATH="/lib/firmware/"
    offload="athwlan.bin"
    offload_codeswap="athwlan.codeswap.bin"
    offload_csma="athwlan_csma.bin"
    offload_codeswap_csma="athwlan.codeswap_csma.bin"
    offload_path=$(find ${FIRMWARE_PATH} -name $offload)
    offload_codeswap_path=$(find ${FIRMWARE_PATH} -name $offload_codeswap)
    offload_csma_path=$(find ${FIRMWARE_PATH} -name $offload_csma)
    offload_codeswap_csma_path=$(find ${FIRMWARE_PATH} -name $offload_codeswap_csma)

    if [ "$qboost" ==  "1" ] && [ "$enjet_disabled" == "0" ]; then
         ln -sf "/rom${offload_path}" $offload_path
         ln -sf "/rom${offload_codeswap_path}" $offload_codeswap_path
         logger "/rom${offload_path}" -t offload -p 5
         logger "/rom${offload_codeswap_path}" -t offload -p 5
         echo "offload path: /rom${offload_path}" > /dev/console
         echo "offload path: /rom${offload_codeswap_path}" > /dev/console
    else
         ln -sf "/rom${offload_csma_path}" $offload_path
         ln -sf "/rom${offload_codeswap_csma_path}" $offload_codeswap_path
         logger "/rom${offload_csma_path}" -t offload -p 5
         logger "/rom${offload_codeswap_csma_path}" -t offload -p 5
         echo "offload path: /rom${offload_path}" > /dev/console
         echo "offload path: /rom${offload_codeswap_path}" > /dev/console
    fi

}

sn_bandsteering_check() {
    # We only need to check a couple SSID at least,   #
    # even if we support multiple SSID band-steering. #
    # SSID-1 band-steering check #

    support_band_steering=$(get_sn_wifi_option SUPPORT_BAND_STEERING_WITH_SUPPRESS_PROBE_RESP_AND_ASSOCIATION)

    [ "$support_band_steering" == "0" ] && return

    local id1=1 # wifi0 vap index
    local id2=1 # wifi1 vap index

    # check vap enable #
    wifi0_ssid_disabled=$(uci get wireless.wifi0_ssid_"$id1".disabled)
    wifi1_ssid_disabled=$(uci get wireless.wifi1_ssid_"$id2".disabled)
    if [ "$wifi0_ssid_disabled" = "1" ] || [ "$wifi1_ssid_disabled" = "1" ]; then
        DebugPrint "radio disable, bandsteer 0"
        bandsteer=0
        return
    fi

    # check SSID #
    wifi0_ssid=$(uci get wireless.wifi0_ssid_"$id1".ssid)
    wifi1_ssid=$(uci get wireless.wifi1_ssid_"$id2".ssid)
    if [ "$wifi0_ssid" != "$wifi1_ssid" ]; then
        DebugPrint "SSID mismatch, bandsteer 0"
        bandsteer=0
        return
    fi

    # check encryption #
    wifi0_ssid_enc=$(uci get wireless.wifi0_ssid_"$id1".encryption)
    wifi1_ssid_enc=$(uci get wireless.wifi1_ssid_"$id2".encryption)
    if [ "$wifi0_ssid_enc" != "$wifi1_ssid_enc" ]; then
        DebugPrint "encryption mismatch, bandsteer 0"
        bandsteer=0
        return
    fi

    # check wep key #
    if [ "${wifi0_ssid_enc:0:3}" = "wep" ]; then
        wifi0_ssid_key_id=$(uci get wireless.wifi0_ssid_"$id1".key_id)
        wifi1_ssid_key_id=$(uci get wireless.wifi1_ssid_"$id1".key_id)
        if [ "$wifi0_ssid_key_id" != "$wifi1_ssid_key_id" ]; then
            DebugPrint "wep key id mismatch, bandsteer 0"
            bandsteer=0
            return
        fi

        wifi0_ssid_key=$(uci get wireless.wifi0_ssid_"$id1".key"$wifi0_ssid_key_id")
        wifi1_ssid_key=$(uci get wireless.wifi1_ssid_"$id2".key"$wifi1_ssid_key_id")
        if [ "$wifi0_ssid_key" != "$wifi1_ssid_key" ]; then
            DebugPrint "wep key mismatch, bandsteer 0"
            bandsteer=0
            return
        fi
    fi

    # check wpa-psk key #
    if [ "${wifi0_ssid_enc:0:3}" = "psk" ]; then
        wifi0_ssid_key=$(uci get wireless.wifi0_ssid_"$id1".key)
        wifi1_ssid_key=$(uci get wireless.wifi1_ssid_"$id2".key)
        if [ "$wifi0_ssid_key" != "$wifi1_ssid_key" ]; then
            DebugPrint "psk key mismatch, bandsteer 0"
            bandsteer=0
            return
        fi
    fi
}


##### wifi distance #####
distance_func()
{
	local device="$1"
	local phydevice="$2"

	DebugPrint "distance_func device:$device phydevice:$phydevice"
	config_get hwmode "$device" hwmode	
	config_get distance "$device" distance

	[ -n "$distance" ] && {
		[ -f /lib/wifi/dfs.sh ] && \
			local w5g_dfs_recovery1="echo $distance > /proc/sys/dev/$phydevice/distance"
		echo "$distance" > /proc/sys/dev/$phydevice/distance
	}
	enable_dfs_check "$device" "$hwmode"
}

enable_dfs_check() 
{
	#config_get hwmode "$device" hwmode
	local device="$1"
	local hwmode="$2"
	case "$hwmode" in
		*a|11na|11ac)
			#enable_dfs_check $device  " $w5g_dfs_recovery1"
			[ -f "$testcase" ] &&
			dfs_func "$device" "$w5g_dfs_recovery1" &
		;;
		*) DebugPrint "No define for DFS check:hwmode_dfs=$hwmode" ;;
	esac
}

distance_ack() {
        local device="$1"
        local phydevice="$2"

        config_get distance "$device" distance

	if [ $distance -lt 6000 -o $distance -gt 63000 ];then
		iwpriv "$phydevice" ack_timeout 64
	elif [ $distance -ge 6000 ];then
		local acktimeout
		acktimeout=$(($(($(($distance-6000))/150))+64)) ##((distance-6000)/300)*2+64
		iwpriv "$phydevice" ack_timeout $acktimeout
	fi
}

qcawifi_start_hostapd_cli() {
	local device=$1
	local ifidx=0
	local radioidx=${device#wifi}

	config_get vifs $device vifs

	[ -z $vifs ] && vifs=$(eval "/usr/sbin/foreach wireless wifi-iface device $device")

	for vif in $vifs; do
		local config_methods vifname

		config_get vifname "$vif" ifname

		if [ -n $vifname ]; then
			[ $ifidx -gt 0 ] && vifname="ath${radioidx}$ifidx" || vifname="ath${radioidx}"
		fi

		config_get_bool wps_pbc "$vif" wps_pbc 0
		config_get config_methods "$vif" wps_config
		[ "$wps_pbc" -gt 0 ] && append config_methods push_button

		if [ -n "$config_methods" ]; then
			pid=/var/run/hostapd_cli-$vifname.pid
			hostapd_cli -i $vifname -P $pid -a /lib/wifi/wps-hostapd-update-uci -p /var/run/hostapd-$device -B
		fi

		ifidx=$(($ifidx + 1))
	done
}

enable_fastscan() {

	wifi_dev=$1
	support_fastscan=$(get_sn_wifi_option SUPPORT_BACKGROUND_FASTSCAN)

	[ "$support_fastscan" == "0" ] && return	

	echo 1 > /tmp/notify_fastscan_enable

	triband5G_fastscan_status=0;  #0:both disabled, 1: only wifi1 enable, 2: only wifi2 , 3 both enabled
	fastscan_script_24G="/sbin/Fastscan24G.sh"
	fastscan_script_5G="/sbin/Fastscan5G.sh"
	isConfigured=$(uci -q get apcontroller.capwap.enable)
	if [ "$isConfigured" = "1" ];then
		test "$(pgrep -f Fastscan)" && kill -9 $(pgrep -f Fastscan)	
		DebugPrint "AP controlled by controller, do fastscan by capwap"
	else
		fastscan_duration=20
		if [ "$wifi_dev" == "wifi0" ]; then
			local vifs=$(eval "/usr/sbin/foreach wireless wifi-iface device $wifi_dev")
			for vif in $vifs; do
				if [ "$(uci -q get wireless.$vif.fastroamingEnable)" == "1" ]; then
					[ -e "$fastscan_script_24G" ] && sh "$fastscan_script_24G" $fastscan_duration &
					break
				fi
			done
		else
			local vifs=$(eval "/usr/sbin/foreach wireless wifi-iface device wifi1")
			for vif in $vifs; do
				if [ "$(uci -q get wireless.$vif.fastroamingEnable)" == "1" ]; then
					triband5G_fastscan_status=$(expr $triband5G_fastscan_status + 1)
					break
				fi
			done

			local vifs=$(eval "/usr/sbin/foreach wireless wifi-iface device wifi2")
			for vif in $vifs; do
				if [ "$(uci -q get wireless.$vif.fastroamingEnable)" == "1" ]; then
					triband5G_fastscan_status=$(expr $triband5G_fastscan_status + 2)
					break
				fi
			done
			if [ "$triband5G_fastscan_status" == "0" ]; then
				DebugPrint "No need to open 5G fastscan"
			else
				[ -e "$fastscan_script_5G" ] && sh "$fastscan_script_5G"  $fastscan_duration $triband5G_fastscan_status &
			fi
		fi
	fi
	
}

disable_fastscan() {

        support_fastscan=$(get_sn_wifi_option SUPPORT_BACKGROUND_FASTSCAN)

        [ "$support_fastscan" == "0" ] && return
	test "$(pgrep -f Fastscan)" && kill -9 $(pgrep -f Fastscan)
	echo 0 > /tmp/notify_fastscan_enable
}

set_dis_legacy_rate() {
	ifname=$1
	minRate=$2
	case "$minRate" in
		2)
		rateCode=0x0001
		bcn_rate=2000
		#iwpriv $ifname set_bcn_rate 2000
		;;
		5)	#should be 5.5, but for convenience, avoid use the point
		rateCode=0x0003
		bcn_rate=5500
		#iwpriv $ifname set_bcn_rate 5500
		;;
		6)
		rateCode=0x0007
		bcn_rate=6000
		#iwpriv $ifname set_bcn_rate 6000
		;;
		9)
		rateCode=0x0017
		bcn_rate=9000
		#iwpriv $ifname set_bcn_rate 9000
		;;
		11)
		rateCode=0x0037
		bcn_rate=11000
		#iwpriv $ifname set_bcn_rate 11000
		;;
		12)
		rateCode=0x003f
		bcn_rate=12000
		#iwpriv $ifname set_bcn_rate 12000
		;;
		18)
		rateCode=0x007f
		bcn_rate=18000
		#iwpriv $ifname set_bcn_rate 18000
		;;
		24)
		rateCode=0x00ff
		bcn_rate=24000
		#iwpriv $ifname set_bcn_rate 24000
		;;
	esac
	iwpriv $ifname dis_legacy $rateCode
	###Due to 5G bit rate set fail####
	iwpriv $ifname set_bcn_rate $bcn_rate
	
}

set_airtime_fairness_vip() {
	local check_atf=$(uci -q get airtime_fairness.atf)     #jacky added, to check the venfor has airtime_fairness or not
	if [ "$check_atf" == "atf" ]; then
		atf_enabled=$(uci -q get airtime_fairness.atf.enabled)
		atf_usage=$(uci -q get airtime_fairness.atf.usage)
		[ -e "/etc/crontabs/root" -a -n "$(cat /etc/crontabs/root | grep updateATFConfig)" ] && {
			sed -i '/updateATFConfig/d' /etc/crontabs/root
		}
		if [ "$atf_usage" == "vip" ]; then      ##maybe add more usage in the future
			if [ "$atf_enabled" == "1" ]; then
				[ -e "/tmp/set_vip_list" ] && rm /tmp/set_vip_list -f #remove this because wifi reload and atf setting also reset
				uci set airtime_fairness.atf.parameter_changed=1
				uci commit airtime_fairness
				echo "*/1 * * * * /bin/sh /sbin/updateATFConfig.sh &" >> /etc/crontabs/root
				crontab /etc/crontabs/root
			fi
		fi
	fi
}

set_vlan_passthrough() {

        support_vlanpass=$(get_sn_wifi_option SUPPORT_VLAN_PASSTHROUGH)

        [ "$support_vlanpass" == "0" ] && return

	local mesh_off=$(uci -q get mesh.wifi.disabled)
	if [ ! -e "/lib/wifi/vlan_passthrough.sh" ]; then
		DebugPrint "/lib/wifi/vlan_passthrough.sh not exists"
		return 0
	fi

	local ethIf=$(uci get /rom/etc/config/network.lan.ifname)
	local batmanIf="bat0"
	local vlanpassIf=""

	##
	## /proc/sys/net/8021q/wds_ifname should be "eth0 eth1 "
	## It's must have a blank in the end.
	##
	if [ "$mesh_off" == "0" ]; then
		vlanpassIf="$batmanIf $ethIf $vlanpassIf"
	elif [ "$wds_enable" == "1" ]; then
		vlanpassIf="$ethIf $vlanpassIf"
		wds_ap_vif=$(eval /usr/sbin/foreach wireless wifi-iface mode_display wds_ap)
		for vif in $wds_ap_vif; do
			if [ "$(uci get wireless.$vif.disabled)" == "0" ]; then
				wds_ap_ifname=$(uci get wireless.$vif.ifname)
				[ -n "$wds_ap_ifname" ] && {
					vlanpassIf="${wds_ap_ifname} ${vlanpassIf}"
				}
			fi
		done
	fi
	vlanpass_wds "$vlanpassIf"
}

create_dummy_interfaces() {

	local wifi_dev=$1
	local dummyList="/tmp/dummyIf_"$wifi_dev	#since some vendor use ath29 as repeater interface, use this file to indicate whether it is dummy or not

	[ -e "$dummyList" ] && rm "$dummyList" -f

	local athw=$(ifconfig 2>/dev/null|grep $wifi_dev);
	athdummy="dummy"$(echo $wifi_dev| awk -F 'wifi' '{print$2}')
        [ -z "$athw" ] && {
                wlanconfig "$athdummy" create wlandev "$wifi_dev" wlanmode "ap" nosbeacon
		echo $athdummy >> $dummyList 
        }
}

destroy_dummy_interfaces() {

        local wifi_dev=$1
        athdummy=$(uci get wireless."$wifi_dev"_dummy.ifname)
        [ -z "$athdummy" ] && { #default dummy interface of wifix
            athdummy="dummy"$(echo $wifi_dev| awk -F 'wifi' '{print$2}')
        }
        destroy_vap $athdummy
}

set_min_rate() {
	#  SENAO support disable legacy rate (set min rate)
	local wifi_dev=$1
	local vif=$2

	config_get ifname $vif ifname
	config_get minLegacyRate "$wifi_dev" min_rate 0
        [ $minLegacyRate -gt 1 ] && [ $minLegacyRate -le 24 ] && {
                if [ $minLegacyRate -le 6 ] && [[ "$wifi_dev" == "wifi1" || "$wifi_dev" == "wifi2" ]]; then
                        DebugPrint "5G device $device min data rate is 6Mbps, cannot disable lower rates"
                else
                        set_dis_legacy_rate $ifname $minLegacyRate
                        if [ "$wifi_dev" == "wifi0" ] && [ $minLegacyRate -gt 11 ]; then
                        	DebugPrint "disabled all b mode basic rate, set pureg=1 for basic rate setting"
                                iwpriv $ifname pureg 1
                        fi
                fi
        }
}

set_client_limit() {
	#sENAO support client limit function

        support_connlimit=$(get_sn_wifi_option SUPPORT_CONNECTION_LIMIT)

        [ "$support_connlimit" == "0" ] && return

	local wifi_dev=$1
	config_get clientlimits_enable "$wifi_dev" clientlimits_enable
        config_get clientlimits_number "$wifi_dev" clientlimits_number

	local conn_limit_max=$(uci -q get functionlist.vendorlist.CONNECTION_LIMIT_NUM)

	[ -z $conn_limit_max ] && conn_limit_max=127
        
        [ -n $clientlimits_enable ] &&
            [ $clientlimits_enable -eq 0 ] && clientlimits_number=$conn_limit_max

	if [ $clientlimits_number -gt $conn_limit_max ];then
		clientlimits_number=$conn_limit_max
	fi

        # fix kernel panic when clientlimits_number < nawds_mac_number in wds_ap
        config_get opmode "$wifi_dev" opmode
        config_get nawds_mac_list "${wifi_dev}_wds_0" WLANWDSPeer ""
        local nawds_mac_number=0
        [ $clientlimits_enable -eq 1 -a "$opmode" == "wds_ap" ] && {
            [ -n "$nawds_mac_list" ] && {
                nawds_mac_number=$(echo "$nawds_mac_list" | grep -o "v" | wc -l)
                clientlimits_number=$(($clientlimits_number+$nawds_mac_number))
            }
        }

        # TDMA
        config_get qboost_enable "$wifi_dev" qboost_enable 0
        [ "$qboost_enable" == "1" ] && clientlimits_number=8

        [ -n $clientlimits_number ] && {
		# cause mesh 5g batman connection failed.
		#device_if $wifi_dev max_radio_sta $clientlimits_number
		if [ "$support_connlimit" == "1" ]; then
		case "$wifi_dev" in
                    wifi0)
                    echo "$clientlimits_number" > /proc/connection_limit
                    ;;
                    wifi1)
                    echo "$clientlimits_number" > /proc/connection_limit_5g
                    ;;
                    wifi2)
                    echo "$clientlimits_number" > /proc/connection_limit_5g_2
                    ;;
                esac
		fi
        }
}

get_beacon_interval() {
	# beacon interval. get bintval(default value 100ms) here, not from wireless cfg file
	#just preset the beacon interval, the iwpriv command will be done later
	local wifi_dev=$1
	config_get vifs "$wifi_dev" vifs
	config_get opmode "$wifi_dev" opmode
	config_get qboost_enable "$device" qboost_enable 0
	local bintval=100  # set default 100ms
	local working_vap_num=0

	[ -z $vifs ] && vifs=$(eval "/usr/sbin/foreach wireless wifi-iface device $wifi_dev")

	for vif in $vifs; do
		config_get opmode "$device" opmode
		config_get mode "$vif" mode
		config_get mode_display "$vif" mode_display
		config_get mode_display_2 "$vif" mode_display_2
		config_get mesh "$vif" mesh 0
		local mesh_disabled=$(uci -q get mesh.wifi.disabled)
		config_get qboost_vif "$vif" qboost_vif 0
		config_get mgmt_vif "$vif" mgmt_vif 0
		config_get disabled "$vif" disabled 1

		[ "$disabled" == "1" ] && continue;

		# check mode_display/mode_display_2 without mesh / mgmt intf
		[ $mesh -eq 0 -a $mgmt_vif -eq 0 ] && [ "$opmode" != "$mode_display" ] && [ "$opmode" != "$mode_display_2" ] && continue;

		[ "$mesh" == "1" ] && [ "$(uci get functionlist.functionlist.CONTROLLER_MODE_SUPPORT_MESH)" == "1" ] && [ "$(uci get apcontroller.capwap.enable)" != "1" ] && continue;
		[ "$mesh" == "1" ] && [ "$(uci get functionlist.functionlist.SUPPORT_MESH_SETTING)" != "1" ] && continue;

		# Not create mesh intf when mesh_disabled = 1
		[ $mesh -eq 1 ] && [ ${mesh_disabled:-0} -eq 1 ] && continue;

		# Not create mgmt interface in controller mode.
		[ $mgmt_vif -eq 1 ] && [ $(uci get apcontroller.capwap.enable) -eq 1 ] && continue;

		# TDMA
		[ "$opmode" == "ap" -o "$opmode" == "wds_ap" ] && {
			[ "$qboost_enable" == "0" ] && [ "$qboost_vif" == "1" ] && continue;
			[ "$qboost_enable" == "1" ] && [ "$qboost_vif" == "0" -a "$mgmt_vif" == "0" ] && continue;
		}

		#monitor mode interface will not send beacon, not to count in working vap
		[ "$mode" == "monitor" ] && continue

		working_vap_num=$(expr $working_vap_num + 1)
	done
	# if vap number of ap,mesh,and guest network in total are more than 8, set beacon interval  to 200ms
	if [ "$working_vap_num" -gt  "8" ]; then
		bintval=200
	fi
	echo $bintval
}

set_beacon_interval() {

        wifi_dev=$1
        beacon_int=$(get_beacon_interval "$wifi_dev")

        if [ -n "$lastApIf" ]; then
                device_if $lastApIf bintval $beacon_int
        else
                radioIdx=$(echo $wifi_dev | awk -F "wifi" '{print $2}')
                ifname=$(/sbin/getWifiFirstIF "$radioIdx")
                device_if $ifname bintval $beacon_int
        fi
}

set_sn_bandsteering() {

    local wifi_dev=$1
    local ifname=$2
    config_get bandsteer "$wifi_dev" bandsteer

    [ -n "$bandsteer" ] && echo "$bandsteer" > /proc/bandsteer

    if [ -e "/sbin/sn_bandsteering" ]; then
	/sbin/sn_bandsteering           #new flow, create a bandsteer checking script
    elif [ "$bandsteer" != "0" ]; then

	local wifi_dev=$1
	local ifname=$2
	local firstApIfname="$(uci get wireless.$wifidev_ssid_1.ifname)"
	local check_wifi2=$(/usr/sbin/foreach wireless wifi-device disabled 0| grep wifi2) #jacky added, to check the platform has wifi2 or not
	config_get bandsteer "$wifi_dev" bandsteer
	config_get bandsteerrssi "$wifi_dev" bandsteerrssi
	config_get bandsteerpersent "$wifi_dev" bandsteerpersent
	config_get bandsteerHBrssi "$wifi_dev" bandsteerHBrssi
	config_get bandsteerHBpersent "$wifi_dev" bandsteerHBpersent

	sn_bandsteering_check      

	config_set bandsteer "$wifi_dev" bandsteer

	[ -n "$bandsteer" ] && echo "$bandsteer" > /proc/bandsteer

	[ -n "$bandsteer" -a "$bandsteer" != "0" ] && {
	    [ "$ifname" == $firstApIfname ] && {
		iwpriv "$ifname" bs_en 1
	    }
	}
	[ -n "$bandsteer" -a "$bandsteer" = "2" ] && {
	    [ "$ifname" == $firstApIfname ] && {
		iwpriv "$ifname" bsmin5grssi "$bandsteerrssi"
		[ -n "$check_wifi2" ] && {
		    iwpriv "$ifname" bs5ghbrssi  "$bandsteerHBrssi"
		}
	    }
	}
	[ -n "$bandsteer" -a "$bandsteer" = "3" ] && {
	    [ "$ifname" == $firstApIfname ] && {
		iwpriv "$ifname" bsmin5grssi "$bandsteerrssi"
		iwpriv "$ifname" bsloadratio "$bandsteerpersent"
		[ -n "$check_wifi2" ] && {
		    iwpriv "$ifname" bs5ghbrssi  "$bandsteerHBrssi"
		    iwpriv "$ifname" bshbratio "$bandsteerHBpersent"
		}
	    }
	}
    fi
}

set_sn_bandsteering_per_ssid() {

    local vif=$1
    local ifname=$2
    local bandsteer=1;
    file="/tmp/indexfile"

    #get the enabled ssid index
    touch $file
    for vif in $vifs; do
	case "$vif" in
            *ssid*)
		index="$(echo $vif |awk -F '_' '{print $3}')"
		getindex=$(cat $file |grep $index)
		if [ -z $(cat $file |grep $index) ]; then
                    echo $index >>$file
		fi
		;;
	esac
    done

    [ -n "$bandsteer" ] && echo "$bandsteer" > /proc/bandsteer
    if [ "$device" = "wifi0" ]; then
	if [ -e "/sbin/sn_bandsteering" ]; then
	    exec < $file
	    while read line
	    do
		/sbin/sn_bandsteering $line
	    done
	    rm $file
	fi
    fi
}

set_sn_fasthandover() {

        support_fasthandover=$(get_sn_wifi_option SUPPORT_FAST_HANDOVER_INDEPENDENT)

        [ "$support_fasthandover" == "0" ] && return
	
	local wifi_dev=$1
	config_get opmode "$wifi_dev" opmode
        [ "$opmode" != "ap" ] &&  fasthandover_status="0" || config_get fasthandover_status "$wifi_dev" fasthandover_status
        config_get fasthandover_rssi "$wifi_dev"   fasthandover_rssi

        if [ "$fasthandover_rssi" -lt -95 ]; then
                fasthandover_rssi=-95
        fi
        case "$wifi_dev" in
                wifi0)
                echo "$fasthandover_status" "$fasthandover_rssi" > /proc/ap_roaming
                ;;
                wifi1)
                echo "$fasthandover_status" "$fasthandover_rssi" > /proc/ap_roaming_5g
                ;;
                wifi2)
                echo "$fasthandover_status" "$fasthandover_rssi" > /proc/ap_roaming_5g_2
                ;;
        esac
}

set_macfilter() {
	
	vif=$1
	ifname=$2
	config_get macfilter "$vif" macfilter
	case "$macfilter" in
	allow)
		config_get allowmaclist "$vif" allowmaclist
		[ -n "$allowmaclist" ] && {
			iwpriv "$ifname" maccmd 3 # flush MAC list
			while [ -n "$allowmaclist" ]; do
				mactmp=`echo $allowmaclist | cut -d ' ' -f 1`
				iwpriv "$ifname" addmac "$mactmp"
				allowmaclist=`echo $allowmaclist|cut -c 18-`
			done
		}
		iwpriv "$ifname" maccmd 1
	;;
	deny)
		config_get denymaclist "$vif" denymaclist
		[ -n "$denymaclist" ] && {
			iwpriv "$ifname" maccmd 3 # flush MAC list
			while [ -n "$denymaclist" ]; do
				mactmp=`echo $denymaclist | cut -d ' ' -f 1`
				iwpriv "$ifname" addmac "$mactmp"
				denymaclist=`echo $denymaclist|cut -c 18-`
			done
		}
		iwpriv "$ifname" maccmd 2
	;;
	*)
	# default deny policy if mac list exists
	#[ -n "$denymaclist" ] && iwpriv "$ifname" maccmd 2
	# default disable and flush MAC list
	iwpriv "$ifname" maccmd 3
	iwpriv "$ifname" maccmd 0
	;;
	esac
}

set_macfilter_via_hostapd() {
	vif=$1
	ifname=$2

	local hostapdL2ACLPath="/etc/l2acl/"
	local ssid_index=${vif#*_ssid_}

	[ -f ${hostapdL2ACLPath}l2acl-ssid_${ssid_index}.maclist ] && rm -f ${hostapdL2ACLPath}l2acl-ssid_${ssid_index}.maclist

	config_get macfilter "$vif" macfilter
	case "$macfilter" in
	allow)
		[ ! -d "$hostapdL2ACLPath" ] && mkdir -p "$hostapdL2ACLPath"
		config_get allowmaclist "$vif" allowmaclist
		[ -n "$allowmaclist" ] && {
			while [ -n "$allowmaclist" ]; do
				mactmp=`echo $allowmaclist | cut -d ' ' -f 1`
				echo "$mactmp" >> ${hostapdL2ACLPath}l2acl-ssid_${ssid_index}.maclist
				allowmaclist=`echo $allowmaclist|cut -c 18-`
			done
		}
	;;
	deny)
		[ ! -d "$hostapdL2ACLPath" ] && mkdir -p "$hostapdL2ACLPath"
		config_get denymaclist "$vif" denymaclist
		[ -n "$denymaclist" ] && {
			while [ -n "$denymaclist" ]; do
				mactmp=`echo $denymaclist | cut -d ' ' -f 1`
				echo "$mactmp" >> ${hostapdL2ACLPath}l2acl-ssid_${ssid_index}.maclist
				denymaclist=`echo $denymaclist|cut -c 18-`
			done
		}
	;;
	esac
}

set_sn_tdma(){
	device=$1
	vif=$2

	config_get opmode "$device" opmode
	config_get qboost_enable "$device" qboost_enable
	config_get ifname "$vif" ifname
	config_get qboost_vif "$vif" qboost_vif 0

	case "$opmode" in
		ap|wds_ap)
			if [ $qboost_enable -eq 1 ]; then
				iwpriv "$device" qboost_enable 1
				iwpriv "$device" tdma_peer_pri 0
				config_get aptimeslot "$device" aptimeslot

				if [ $aptimeslot -eq 0 ]; then
					iwpriv "$device" tdma_txop 4
				else
					iwpriv "$device" tdma_txop "$aptimeslot"
				fi

				if [ "$qboost_vif" == "1" ]; then
					iwpriv "$ifname" tdma_vap 1
				fi
			else
				iwpriv "$device" qboost_enable 0
				iwpriv "$device" tdma_peer_pri 0
				iwpriv "$device" tdma_txop 0
			fi
		;;
		sta|wds_sta)
			iwpriv "$device" qboost_enable 0
			iwpriv "$device" tdma_txop 0
			config_get stationpriority "$device" stationpriority
			[ -n "$stationpriority" ] && iwpriv "$device" tdma_peer_pri "$stationpriority"
		;;
	esac

}

create_wifi_vap(){

	device=$1
	vif=$2
        
	config_get opmode "$device" opmode
        config_get qboost_enable "$device" qboost_enable 0

        local nosbeacon= wlanaddr=""
        config_get mode "$vif" mode
        local wlanmode=$mode
        config_get enc "$vif" encryption "none"
        config_get mode_display "$vif" mode_display
        config_get mode_display_2 "$vif" mode_display_2
        config_get mesh "$vif" mesh 0
        local mesh_disabled=$(uci -q get mesh.wifi.disabled)
        config_get qboost_vif "$vif" qboost_vif 0
        config_get mgmt_vif "$vif" mgmt_vif 0

	# check mode_display/mode_display_2 without mesh / mgmt intf
	[ $mesh -eq 0 -a $mgmt_vif -eq 0 ] && [ "$opmode" != "$mode_display" ] && [ "$opmode" != "$mode_display_2" ] && return;

	[ "$mesh" == "1" ] && [ "$(uci get functionlist.functionlist.CONTROLLER_MODE_SUPPORT_MESH)" == "1" ] && [ "$(uci get apcontroller.capwap.enable)" != "1" ] && return;
	[ "$mesh" == "1" ] && [ "$(uci get functionlist.functionlist.SUPPORT_MESH_SETTING)" != "1" ] && return;

	# Not create mesh intf when mesh_disabled = 1
	[ $mesh -eq 1 ] && [ ${mesh_disabled:-0} -eq 1 ] && return;

	# Not create mgmt interface in controller mode.
	[ $mgmt_vif -eq 1 ] && [ $(uci get apcontroller.capwap.enable) -eq 1 ] && return;

        # TDMA
	[ "$opmode" == "ap" -o "$opmode" == "wds_ap" ] && {
		[ "$qboost_enable" == "0" ] && [ "$qboost_vif" == "1" ] && return;
		[ "$qboost_enable" == "1" ] && [ "$qboost_vif" == "0" -a "$mgmt_vif" == "0" ] && return;
	}

        config_get ifname "$vif" ifname
        config_get_bool nosbeacon "$device" nosbeacon
        config_get qwrap_enable "$device" qwrap_enable 0
        all_ifnames=$(echo $all_ifnames $ifname)

        #[ "$wlanmode" = "ap_monitor" ] && wlanmode="specialvap"        #non-using modes, disable now, keep for the future
        #[ "$wlanmode" = "ap_smart_monitor" ] && wlanmode="smart_monitor"
        #[ "$wlanmode" = "ap_lp_iot" ] && wlanmode="lp_iot_mode"

        [ "$nosbeacon" = 1 ] || nosbeacon=""

	if [ "$mode" = "sta" -a "$qwrap_enable" -gt 0 ]; then
                wlanaddr="00:00:00:00:00:00"
                DebugPrint "wlanconfig $ifname create wlandev $device wlanmode $wlanmode ${wlanaddr:+wlanaddr $wlanaddr} ${nosbeacon:+nosbeacon}"
                ifname=$(/usr/sbin/wlanconfig "$ifname" create wlandev "$device" wlanmode "$wlanmode" ${wlanaddr:+wlanaddr "$wlanaddr"} ${nosbeacon:+nosbeacon})
        else
                DebugPrint "wlanconfig $ifname create wlandev $device wlanmode $mode ${nosbeacon:+nosbeacon}"
                ifname=$(/usr/sbin/wlanconfig "$ifname" create wlandev "$device" wlanmode "$mode" ${nosbeacon:+nosbeacon})
        fi

        [ $? -ne 0 ] && {
                DebugPrint "enable_qcawifi($device): Failed to set up $mode vif $ifname"
                return
        }
        config_set "$vif" ifname "$ifname"
}

create_wifi_vaps(){
        device=$1

	config_get vifs "$device" vifs

        for vif in $vifs; do
                create_wifi_vap $device $vif
        done
}

destroy_vap() {
	local ifname="$1"
	ifconfig $ifname down
	wlanconfig $ifname destroy
}

set_vap_params() {

	device=$1
	vif=$2
	
	config_get opmode "$device" opmode
	config_get qboost_enable "$device" qboost_enable 0
                
	local start_hostapd= start_wpa_supplicant= nosbeacon= wlanaddr=""
        config_get mode "$vif" mode
        local wlanmode=$mode
        config_get enc "$vif" encryption "none"
        config_get mode_display "$vif" mode_display
        config_get mode_display_2 "$vif" mode_display_2
        config_get mesh "$vif" mesh 0
	local mesh_disabled=$(uci -q get mesh.wifi.disabled)
	config_get qboost_vif "$vif" qboost_vif 0
	config_get mgmt_vif "$vif" mgmt_vif 0
	config_get_bool hs20 "$vif" hs20 0

	# check mode_display/mode_display_2 without mesh / mgmt intf
	[ $mesh -eq 0 -a $mgmt_vif -eq 0 ] && [ "$opmode" != "$mode_display" ] && [ "$opmode" != "$mode_display_2" ] && return;
	
	[ "$mesh" == "1" ] && [ "$(uci get functionlist.functionlist.CONTROLLER_MODE_SUPPORT_MESH)" == "1" ] && [ "$(uci get apcontroller.capwap.enable)" != "1" ] && return;
	[ "$mesh" == "1" ] && [ "$(uci get functionlist.functionlist.SUPPORT_MESH_SETTING)" != "1" ] && return;

	# Not create mesh intf when mesh_disabled = 1
	[ $mesh -eq 1 ] && [ ${mesh_disabled:-0} -eq 1 ] && return;

	# Not create mgmt interface in controller mode.
	[ $mgmt_vif -eq 1 ] && [ $(uci get apcontroller.capwap.enable) -eq 1 ] && return;

	# TDMA
	[ "$opmode" == "ap" -o "$opmode" == "wds_ap" ] && {
		[ "$qboost_enable" == "0" ] && [ "$qboost_vif" == "1" ] && return;
		[ "$qboost_enable" == "1" ] && [ "$qboost_vif" == "0" -a "$mgmt_vif" == "0" ] && return;
	}

	if [ "$mode" == "ap" ]; then
		vap_ap_mode_setting "$device" "$vif"	
	else
		vap_non_ap_mode_setting "$device" "$vif"
	fi

	vap_common_setting "$device" "$vif"
	[ "$hs20" -gt 0 ] && vap_hotspot_setting "$device" "$vif"
	#vap_non_using_setting "$device" "$vif"		#non using parameters
}

set_wifi_param_per_vap() {

        device=$1

        config_get vifs "$device" vifs
        for vif in $vifs; do
                set_vap_params $device $vif
        done    #end of device vifs loop

}

remove_hostapd_conf() {
	dev=$1
	[ -f /sys/class/net/${dev}/parent ] && { \
		local parent=$(cat /sys/class/net/${dev}/parent)
		[ -n "$parent" -a "$parent" = "$device" ] && { \
			#new flow, to kill wifi related processes which use wpa_cli and global
			[ -f "/var/run/hostapd-${dev}.lock" ] && { \
				wpa_cli -g /var/run/hostapd/global raw REMOVE ${dev}
				rm /var/run/hostapd-${dev}.lock
			}
		}
	}
}

set_aggregation() {

	wifi_dev=$1
	ifname=$2

	config_get aggregation_enable "$wifi_dev" aggregation_enable

	if [ -n "$aggregation_enable" ] && [ "$wifi_dev" == "wifi0" ]; then
		if [ "$aggregation_enable" == "1" ]; then
			config_get aggregation_frame "$wifi_dev" aggregation_frame
			[ -n "$aggregation_frame" ] && iwpriv "$ifname" ampdu "$aggregation_frame"
			local power=0;
			config_get aggregation_byte "$wifi_dev" aggregation_byte
			[ -n "$aggregation_byte" ] && {
				if [ "$aggregation_byte" -gt  "8191" ] && [ "$aggregation_byte" -le  "16383" ]; then
					power=1;
				elif [ "$aggregation_byte" -gt  "16383" ] && [ "$aggregation_byte" -le  "32767" ]; then
					power=2;
				elif [ "$aggregation_byte" -gt  "32767" ] && [ "$aggregation_byte" -le  "65535" ]; then
					power=3;
				else
					power=0;
				fi
				iwpriv "$ifname" maxampdu "$power"
			}
		else
			iwpriv "$ifname" ampdu "64"
			iwpriv "$ifname" maxampdu "3"
		fi
	fi
}

vap_common_setting() {	#parameters for ap and sta mode

		device=$1
		vif=$2

		config_get opmode "$device" opmode
		config_get ifname "$vif" ifname
		config_get qboost_enable "$device" qboost_enable 0
		config_get qboost_vif "$vif" qboost_vif 0

                set_hwmode_htmode "$ifname" "$device" "$opmode"

                config_get wds "$vif" wds
                case "$wds" in
                        1|on|enabled) wds=1;;
                        *) wds=0;;
                esac
                iwpriv "$ifname" wds "$wds"

		# TDMA
		[ "$qboost_enable" == "1" ] && [ "$qboost_vif" == "1" ] && [ "$opmode" == "wds_ap" ] && iwpriv "$ifname" wds 1

		config_get_bool shortgi "$vif" shortgi 1
	        [ -n "$shortgi" ] && iwpriv "$ifname" shortgi "${shortgi}"    

            ### SENAO ### 11k fastroaming-- preset 11k disable, sw spec requirement
                #iwpriv "$ifname" rrm 0
                if [ "$mode_display" == "ap" -o "$mode_display" == "wds_ap" ]; then
                        iwpriv "$ifname" apchanrpt 1	## SENAO: always enable ap chan report ##
                        iwpriv "$ifname" rrm 1	## SENAO: always enable 802.11k ##
                fi

                iwpriv "$ifname" wmm 1  #no matter AP/STA mode, it should be enabled

                iwpriv "$ifname" doth 1 #no matter AP/STA mode, it should be enabled

                config_get nss "$vif" nss
                [ -n "$nss" -a "$ac_mode" == "1" ] && iwpriv "$ifname" nss "$nss"

                #ap_isoloation, it seems only wrap mode no need to use this
                config_get_bool ap_isolation_enabled $device ap_isolation_enabled 0
                config_get_bool isolate "$vif" isolate 0

                if [ $ap_isolation_enabled -ne 0 ]; then
                        [ "$mode" = "wrap" ] && isolate=1
                fi
                local net_cfg bridge
                net_cfg="$(find_net_config "$vif")"
                [ -z "$net_cfg" -o "$isolate" = 1 -a "$mode" = "wrap" ] || {
                        [ -f /sys/class/net/${ifname}/parent ] && { \
                                        bridge="$(bridge_interface "$net_cfg")"
                                config_set "$vif" bridge "$bridge"
                                start_net "$ifname" "$net_cfg"
                        }
                }
		#end of ap_isolation

                config_get ssid "$vif" ssid
                [ -n "$ssid" ] && {
                        iwconfig "$ifname" essid "$ssid"
                        #iwconfig "$ifname" essid on
                        #iwconfig "$ifname" essid ${ssid:+-- }"$ssid"
                }

                set_wifi_up "$vif" "$ifname"
                set_txpower "$device" "$vif"

                set_hostapd_wpa_supplicant "$vif"

                set_aggregation "$device" "$ifname"

                #  support traffic control
                [ -e "/lib/wifi/traffic_control.sh" ] && tc_setConf "$vif"

                #Fix tx/rx packet and bytes incorrect issue.
                iwpriv "$device" enable_ol_stats 1

		# support disable legacy rate (set min rate)
		config_get nawds_vif "$vif" nawds
		if [ "$nawds_vif" != "1" ]; then
			set_min_rate "$device" "$vif"
		fi

		# Fixed packet loss issue (small packet)
		# set 2, no issue like dropped calls during VOIP calls, and the throughput is best.
		iwpriv "$ifname" amsdu 2
}

vap_ap_mode_setting() {	#parameters for ap mode only

	device=$1
	vif=$2
	config_get ifname "$vif" ifname

	config_get_bool hidden "$vif" hidden 0
	[ -n "$hidden" ] && iwpriv "$ifname" hide_ssid "$hidden"

	config_get_bool dynamicbeacon "$vif" dynamicbeacon 0
	[ $hidden -eq 1 ] && iwpriv "$ifname" dynamicbeacon "$dynamicbeacon"

	#jacky: not to set beacon interval here, move to set_beacon_interval and just set in lastApIf or getWifiFirstIF
	# beacon interval. get bintval(default value 100ms) here, not from wireless cfg file.
	#local bintval=$(get_beacon_interval "$device")
	#[ -n "$bintval" ] && {
        #iwpriv "$ifname" bintval "$bintval"
        #config_set "$vif" bintval "$bintval"
	#}

	config_get dtim_period "$vif" dtim_period
	[ -n "$dtim_period" ] && iwpriv "$ifname" dtim_period "$dtim_period"

	config_get preamble "$vif" preamble
	[ -n "$preamble" ] && iwpriv "$ifname" shpreamble "$preamble"

	config_get_bool uapsd "$vif" uapsd 1
	[ -n "$uapsd" ] && iwpriv "$ifname" uapsd "$uapsd"

	config_get athnewind "$vif" athnewind
	[ -n "$athnewind" ] && iwpriv "$ifname" athnewind "$athnewind"

	config_get mcast_rate "$vif" mcast_rate
	[ "$mcast_rate" ] && iwpriv "$ifname" mcast_rate "${mcast_rate}"

	config_get mcastenhance "$vif" mcastenhance
	[ -n "$mcastenhance" ] && iwpriv "$ifname" mcastenhance "${mcastenhance}"

        config_get bcmcdrop "$vif" bcmcdrop
        [ -n "$bcmcdrop" ] && iwpriv "$ifname" bcmcdrop "${bcmcdrop}"

	config_get me_adddeny "$vif" me_adddeny
	[ -n "$me_adddeny" ] && iwpriv "$ifname" me_adddeny ${me_adddeny}

	config_get_bool proxyarp "$vif" proxyarp
	[ -n "$proxyarp" ] && device_if "$ifname" proxyarp "$proxyarp"

	#iwpriv "$ifname" me_adddeny 3758096635 255 255 255 #filter Bonjour/mDNS packet 
	#emr3000 use this, but the proper usage should set these parameter in wireless config, ex: wireless.wifi0_ssid_1.me_adddeny='3758096635 255 255 255'

	#per vap setting functions

	# black/white list setting
	if [ "$(uci -q get functionlist.functionlist.SUPPORT_HOSTAPD_MAC_FILTER)" = "1" ]; then
		set_macfilter_via_hostapd $vif $ifname
	else
		set_macfilter $vif $ifname
	fi
	#SENAO bandsteering setting
        support_band_steering=$(get_sn_wifi_option SUPPORT_BAND_STEERING_WITH_SUPPRESS_PROBE_RESP_AND_ASSOCIATION)
        if [ "$support_band_steering" != "0" ]; then
            band_steering_per_ssid=$(get_sn_wifi_option SUPPORT_BAND_STEERING_PER_SSID)
            if [ "$band_steering_per_ssid" == "1" ]; then
                set_sn_bandsteering_per_ssid $vif $ifname
            else
                set_sn_bandsteering $device $ifname	    
            fi
        fi

	#SENAO TDMA setting
    support_tdma=$(get_sn_wifi_option SUPPORT_TDMA)
    [ "$support_tdma" != "0" ] && set_sn_tdma $device $vif	
}

vap_non_ap_mode_setting() { #parameters for sta mode only

	device=$1
	vif=$2

	config_get ifname "$vif" ifname
	local extap=0
	local supp_mgmt=$(uci -q get functionlist.functionlist.SUPPORT_MANAGEMENT_SSID)

	# wds_sta extap set 1 when support mgmt ssid, it fix mgmt ssid can't up.
	[ "$mode" = "sta" ] && [ "$mode_display" != "wds_sta" -o ${supp_mgmt:-0} -eq 1 ] && [ "$mode_display" != "sta_ap" ] && extap=1

	config_set "$vif" extap "$extap"

	[ -n "$extap" ] && iwpriv "$ifname" extap "$extap"

        #SENAO TDMA setting
        support_tdma=$(get_sn_wifi_option SUPPORT_TDMA)
        [ "$support_tdma" != "0" ] && set_sn_tdma $device $vif

}

vap_hotspot_setting() {	#parameters for ap mode

	device=$1
	vif=$2

	config_get ifname "$vif" ifname

	config_get_bool l2tif "$vif" l2tif
	[ -n "$l2tif" ] && iwpriv "$ifname" l2tif "$l2tif"

	config_get_bool qbssload "$vif" qbssload
	[ -n "$qbssload" ] && iwpriv "$ifname" qbssload "$qbssload"

	config_get_bool proxyarp "$vif" proxyarp
	[ -n "$proxyarp" ] && iwpriv "$ifname" proxyarp "$proxyarp"

	config_get_bool dgaf_disable "$vif" dgaf_disable
	[ -n "$dgaf_disable" ] && iwpriv "$ifname" dgaf_disable "$dgaf_disable"

	config_get osen "$vif" osen
	[ -n "$osen" ] && iwpriv "$ifname" osen "$osen"

	config_get vhtmubfer "$vif" vhtmubfer
	[ -n "$vhtmubfer" ] && iwpriv "$ifname" vhtmubfer "$vhtmubfer"

	config_get hcbssload "$vif" hcbssload
	[ -n "$hcbssload" ] && iwpriv "$ifname" hcbssload "$hcbssload"

}

#parameters but not using now since it does not exist in our etc/config/wireless
#NOTE: if you find any parameter want to use in this function, please move to another function ex: vap_ap_mode_setting
vap_non_using_setting() { 

	device=$1	
	vif=$2
	
	#config_get vhtmcs "$vif" vhtmcs        #set this in hwmode_htmode setting
	#[ -n "$vhtmcs" ] && iwpriv "$ifname" vhtmcs "$vhtmcs"

	#config_get chwidth "$vif" chwidth      #set this in hwmode_htmode setting
	#[ -n "$chwidth" ] && iwpriv "$ifname" chwidth "$chwidth"

	config_get ifname "$vif" ifname

	config_get ldpc "$vif" ldpc
	[ -n "$ldpc" ] && iwpriv "$ifname" ldpc "$ldpc"

	config_get rx_stbc "$vif" rx_stbc
	[ -n "$rx_stbc" ] && iwpriv "$ifname" rx_stbc "$rx_stbc"

	config_get tx_stbc "$vif" tx_stbc
	[ -n "$tx_stbc" ] && iwpriv "$ifname" tx_stbc "$tx_stbc"

	config_get cca_thresh "$vif" cca_thresh
	[ -n "$cca_thresh" ] && iwpriv "$ifname" cca_thresh "$cca_thresh"

	config_get set11NRetries "$vif" set11NRetries
	[ -n "$set11NRetries" ] && iwpriv "$ifname" set11NRetries "$set11NRetries"

	config_get chanbw "$vif" chanbw
	[ -n "$chanbw" ] && iwpriv "$ifname" chanbw "$chanbw"

	config_get maxsta "$vif" maxsta
	[ -n "$maxsta" ] && iwpriv "$ifname" maxsta "$maxsta"

	config_get sko_max_xretries "$vif" sko_max_xretries
	[ -n "$sko_max_xretries" ] && iwpriv "$ifname" sko "$sko_max_xretries"

	config_get extprotmode "$vif" extprotmode
	[ -n "$extprotmode" ] && iwpriv "$ifname" extprotmode "$extprotmode"

	config_get extprotspac "$vif" extprotspac
	[ -n "$extprotspac" ] && iwpriv "$ifname" extprotspac "$extprotspac"

	config_get_bool cwmenable "$vif" cwmenable
	[ -n "$cwmenable" ] && iwpriv "$ifname" cwmenable "$cwmenable"

	config_get_bool protmode "$vif" protmode
	[ -n "$protmode" ] && iwpriv "$ifname" protmode "$protmode"

	config_get enablertscts "$vif" enablertscts
	[ -n "$enablertscts" ] && iwpriv "$ifname" enablertscts "$enablertscts"

	config_get txcorrection "$vif" txcorrection
	[ -n "$txcorrection" ] && iwpriv "$ifname" txcorrection "$txcorrection"

	config_get rxcorrection "$vif" rxcorrection
	[ -n "$rxcorrection" ] && iwpriv "$ifname" rxcorrection "$rxcorrection"
	
}

set_hostapd_wpa_supplicant() {

		vif=$1

		config_get ifname "$vif" ifname
		config_get mode "$vif" mode
		config_get enc "$vif" encryption "none"
		config_get nawds "$vif" nawds

                func_enable_radius_acc=$(uci -q get functionlist.functionlist.SUPPORT_ALL_ENCRYPTION_HAS_RADIUS_SETTING)

                if [ "$func_enable_radius_acc" == "1" ]; then
                        config_get acct_enabled "$vif" acct_enabled
                        if [ "$acct_enabled" == "1" ]; then
                                # Radius Accounting enable for Disabled/WEP/PSK*
                                start_hostapd=1
                        fi
                fi

                [ -f /var/run/hostapd-$ifname.conf ] && rm -f /var/run/hostapd-$ifname.conf

		#SENAO WPA3's function
		support_wpa3=$(get_sn_wifi_option SUPPORT_QCA_WPA3)

		if [ "$support_wpa3" == "1" ]; then 
			config_get own_ie_override "$vif" own_ie_override
			[ -n "$own_ie_override" ] && iwpriv "$ifname" rsn_override 1

			config_get_bool sae "$vif" sae 
			config_get_bool owe "$vif" owe 

			config_get suite_b "$vif" suite_b 
			[ -n $suite_b ] && {
				if [ $suite_b -ne 192 ]
				then
					echo "$suite_b bit security level is not supported for SUITEB" > /dev/console
					destroy_vap $ifname
					return
				fi
			}
		fi

                case "$enc" in
                        none)
                                # If we're in open mode and want to use WPS, we
                                # must start hostapd
                                config_get_bool wps_pbc "$vif" wps_pbc 0
                                config_get config_methods "$vif" wps_config
                                [ "$wps_pbc" -gt 0 ] && append config_methods push_button
                                start_wpa_supplicant=1 # Dakota need wpa_supplicant at none encrypt
                                start_hostapd=1
                                # We start hostapd in open mode also
                        ;;

                        wep*)
                                config_get key "$vif" key_id
                                config_get wep_key "$vif" key$key
                                [ "$mode" = "sta" ] || {
                                        iwconfig "$ifname" enc "[$key]" "${wep_key:-off}"
                                        case "$enc" in
                                                *open)
                                                        authmode=1;authmode2="open"
                                                        ;;
                                                *shared)
                                                        authmode=2;authmode2="restricted"
                                                        ;;
                                        esac
                                        iwpriv "$ifname" authmode $authmode
                                        iwconfig "$ifname" key $authmode2 # restricted,open
                                }
                                start_hostapd=1
				
				if [ "$support_wpa3" == "1" ]; then 
					if [ $sae -eq 1 ] || [ $owe -eq 1 ]
					then
						echo "With SAE/OWE enabled, wep enc is not supported" > /dev/console
						destroy_vap $ifname
						return
					fi
				fi
                        ;;
			wpa*|8021x)
                                start_hostapd=1
                                start_wpa_supplicant=1
                                config_get key "$vif" key

                                ### SENAO ### 11k fastroaming
				#config_get fastroaming_en "$vif" fastroamingEnable
                                #if [ "$fastroaming_en" == "1" ]; then
                                #        iwpriv "$ifname" rrm 1
                                #else   #no need to set zero since it already preset before
                                #       iwpriv "$ifname" rrm 0
                                #fi
				
				if [ "$support_wpa3" == "1" ]; then
					start_hostapd=1
				fi
			;;
			mixed*|psk*)
                                start_hostapd=1
                                start_wpa_supplicant=1
                                config_get key "$vif" key

                                ### SENAO ### 11k fastroaming
				#config_get fastroaming_en "$vif" fastroamingEnable
                                #if [ "$fastroaming_en" == "1" ]; then
                                #        iwpriv "$ifname" rrm 1
                                #else   #no need to set zero since it already preset before
                                #       iwpriv "$ifname" rrm 0
                                #fi

				if [ "$support_wpa3" == "1" ]; then
					config_get wpa_psk_file  "$vif" wpa_psk_file
					if [ -z $key ] && [ -z $wpa_psk_file ]
					then
						echo "Key is NULL" > /dev/console
						destroy_vap $ifname
						return
					fi
	
					case "$enc" in
						*tkip*)
							if [ $sae -eq 1 ] || [ $owe -eq 1 ]
							then
								echo "With SAE/OWE enabled, tkip enc is not supported" > /dev/console
								destroy_vap $ifname
								return
							fi
						;;
					esac
				fi
			;;
			tkip*)
				if [ "$support_wpa3" == "1" ]; then
					if [ $sae -eq 1 ] || [ $owe -eq 1 ]
					then
						echo "With SAE/OWE enabled, tkip enc is not supported" > /dev/console
						destroy_vap $ifname
						return
					fi
				fi
                        ;;
			#Needed ccmp*|gcmp* check for SAE OWE auth types
			ccmp*|gcmp*)
				if [ "$support_wpa3" == "1" ]; then
					flag=0
					start_hostapd=1
					config_get key "$vif" key
					config_get wpa_psk_file  "$vif" wpa_psk_file
					config_get sae_password "$vif" sae_password
					if [ $sae -eq 1 ]
					then
						if [ -z "$sae_password" ] && [ -z "$key" ] && [ -z $wpa_psk_file ]
						then
							echo "key/sae_password are NULL" > /dev/console
							destroy_vap $ifname
							return
						fi
					fi
	
					if [ $owe -eq 1 ]
					then
						check_owe_groups() {
							local owe_groups=$(echo $1 | tr "," " ")
							for owe_group_value in $owe_groups
							do
								if [ $owe_group_value -ne 19 ] && [$owe_group_value -ne 20 ] && [ $owe_group_value -ne 21 ]
								then
									echo "Invalid owe_group: $owe_group_value" > /dev/console
									destroy_vap $ifname
									flag=1
									return
								fi
							done
						}
	
						config_list_foreach "$vif" owe_groups check_owe_groups
	
						if [ $flag -eq 1 ]
						then
							return
						fi
					 fi
				fi
			;;
                esac

                case "$mode" in
                        ap|wrap|ap_monitor|ap_smart_monitor|mesh|ap_lp_iot)
                                iwpriv "$ifname" ap_bridge "$((isolate^1))"
                                if [ -n "$start_hostapd" -a "$nawds" != "1" ] && eval "type hostapd_setup_vif" 2>/dev/null >/dev/null; then
                                        hostapd_setup_vif "$vif" atheros no_nconfig || {
                                                DebugPrint "enable_qcawifi($device): Failed to set up hostapd for interface $ifname"
                                                # make sure this wifi interface won't accidentally stay open without encryption
                                                ifconfig "$ifname" down
                                                wlanconfig "$ifname" destroy
                                                return
                                        }
                                fi
                        ;;
                        wds|sta)
                                # Client Bridge + WPA* = call wpa_supplicant_setup_vif
                                # Client Bridge + WEP = don't call wpa_supplicant_setup_vif (not sure at Dakota)
                                # Client Bridge + None + Dakota= call wpa_supplicant_setup_vif
                                # Client Bridge + None + Scorpion= don't wpa_supplicant_setup_vif

                                # Repeater + WPA*/WEP/None = call wpa_supplicant_setup_vif
                                if [ "$mode_display" != "sta" -o "$start_wpa_supplicant" = "1" ] && eval "type wpa_supplicant_setup_vif" 2>/dev/null >/dev/null; then
                                        wpa_supplicant_setup_vif "$vif" athr || {
                                                DebugPrint "enable_qcawifi($device): Failed to set up wpa_supplicant for interface $ifname"
                                                ifconfig "$ifname" down
                                                wlanconfig "$ifname" destroy
                                                return
                                        }
                                fi
                        ;;
                        adhoc)
                                if eval "type wpa_supplicant_setup_vif" 2>/dev/null >/dev/null; then
                                        wpa_supplicant_setup_vif "$vif" athr || {
                                                DebugPrint "enable_qcawifi($device): Failed to set up wpa"
                                                ifconfig "$ifname" down
                                                wlanconfig "$ifname" destroy
                                                return
                                        }
                                fi
                        ;;
                esac

}

set_hwmode_htmode() {
		local ifname=$1
		local device=$2
		local opmode=$3
                config_get hwmode "$device" hwmode auto
                config_get htmode "$device" htmode auto
                local pureg=0
                local puren=0
                local ac_mode=0
                local disablecoext=0 # we can use this parameter to set fixed40Mhz
                local chwidth=0
                local allow_frag=0

                # if it is client bridge, give hwmode and htmode with 11ng and HT40 for BGN mode, and 11ac and HT80 for 11ACVHT80 mode.
                # let sta connect to ap in 20MHz or 40MHz bandwidth which depend on AP's setting.
                if [ "$opmode" == "sta" -o "$opmode" == "wds_sta" -o "$opmode" == "sta_ap" ]; then
                        channel=0
                        config_set "$device" aggregation_enable "0"
                        case "$device" in
                                *0)
                                        hwmode=11ng; htmode=HT20_40
                                        config_set "$device" rate "0x0"
                                        ;;
                                *1|*2)
                                        hwmode=11ac; htmode=HT80                                        
					config_set "$device" rate ""
                                ;;
                        esac
                fi

                [ -f /tmp/opmode ] || touch /tmp/opmode
                uci get opmode.@$device[0] -c /tmp -q || {
                        uci add opmode $device -c /tmp -q
                        uci set opmode.@$device[0].opmode=$opmode -c /tmp -q
                }

                # if not ap mode or opmode change, renew dhcp
                [ "$renew" = "0" -a "$opmode" != "ap" -o "$opmode" != "$(uci get opmode.@$device[0].opmode -c /tmp)" ] && {
                        proto=$(uci get network.lan.proto)
                        ${proto}_renew
                        renew=1
                        unset proto
                }

                # record opmode
                uci set opmode.@$device[0].opmode=$opmode -c /tmp -q
                uci commit opmode -c /tmp -q

                case "$hwmode:$htmode" in
                # The parsing stops at the first match so we need to make sure
                # these are in the right orders (most generic at the end)
                        *ng:HT20) hwmode=11NGHT20; disablecoext=1; chwidth=0;;
                        *ng:HT20_40) hwmode=11NGHT40; chwidth=1;;
                        *ng:HT40) hwmode=11NGHT40; disablecoext=1; chwidth=1;;
                        *ng:*) hwmode=11NGHT20; chwidth=0;;
                        *na:HT20) hwmode=11NAHT20; chwidth=0;;
                        *na:HT20_40) hwmode=11NAHT40; chwidth=1;;
                        *na:HT40) hwmode=11NAHT40; disablecoext=1; chwidth=1;;
                        *na:*) hwmode=11NAHT40; chwidth=1;;
                        *ac:HT20) hwmode=11ACVHT20; ac_mode=1; chwidth=0;;
                        *ac:HT40+) hwmode=11ACVHT40PLUS; ac_mode=1; chwidth=1;;
                        *ac:HT40-) hwmode=11ACVHT40MINUS ac_mode=1; chwidth=1;;
                        *ac:HT20_40) hwmode=11ACVHT40; ac_mode=1; chwidth=1;;
                        *ac:HT40) hwmode=11ACVHT40; ac_mode=1; disablecoext=1; chwidth=1;;
                        *ac:HT80) hwmode=11ACVHT80; ac_mode=1; chwidth=2;;
                        *ac:HT160) hwmode=11ACVHT160;;
                        *ac:HT80_80) hwmode=11ACVHT80_80; chwidth=2;;
                        *ac:*) hwmode=11ACVHT80; ac_mode=1; chwidth=2;;
                        *n:*) puren=1;;
                        *b:*) hwmode=11B; allow_frag=1;;
                        *bg:*) hwmode=11G; allow_frag=1;;
                        *g:*) hwmode=11G; pureg=1; allow_frag=1;;
                        *a:*) hwmode=11A; allow_frag=1;;
                        *) hwmode=AUTO;;
                esac

                #handle puren 
                [ $puren -gt 0 ] && {
                        case "$device:$htmode" in
                                *0:HT20) hwmode=11NGHT20; disablecoext=1; chwidth=0;;
                                *0:HT20_40) hwmode=11NGHT40; chwidth=1;;                                
                                *0:HT40) hwmode=11NGHT40; disablecoext=1; chwidth=1;;
                                *:HT20) hwmode=11NAHT20; chwidth=0;;
                                *:HT20_40) hwmode=11NAHT40; chwidth=1;;
                                *:HT40) hwmode=11NAHT40; disablecoext=1; chwidth=1;;
                                *) hwmode=AUTO;;
                        esac
                }
                iwpriv "$ifname" mode "$hwmode"
                [ $pureg -gt 0 ] && iwpriv "$ifname" pureg "$pureg"
                [ $puren -gt 0 ] && iwpriv "$ifname" puren "$puren"
                [ $channel -gt 0 ] && iwconfig "$ifname" channel "$channel"
                iwpriv "$ifname" disablecoext "${disablecoext}"
                # repeater mode don't set chwidth.
                [ "$opmode" != "sta_ap" ] && iwpriv "$ifname" chwidth "${chwidth}"

                config_get frag "$vif" frag

                config_get rts "$device" rts
                if [ -n "$rts" ] && [ "$ac_mode" != "1" ]; then
                        [ ${rts} -ge 2346 ] && unset rts
                        [ -n "$rts" ] && iwconfig "$ifname" rts "${rts%%.*}"
                fi
                [ -n "$rts" -a "$ac_mode" == "1" ] && iwconfig "$ifname" rts off

}

set_nawds_or_mesh_mode() {
	local wifi_dev=$1
	config_get vifs "$wifi_dev" vifs
	config_get qboost_enable "$wifi_dev" qboost_enable 0

	[ -z $vifs ] && vifs=$(eval "/usr/sbin/foreach wireless wifi-iface device $wifi_dev")

	for vif in $vifs; do
                config_get ifname "$vif" ifname
                config_get nawds "$vif" nawds
                config_get disabled "$vif" disabled
                config_get opmode "$wifi_dev" opmode
                config_get mode_display "$vif" mode_display
                config_get mesh "$vif" mesh 0
		config_get qboost_vif "$vif" qboost_vif 0
		config_get mgmt_vif "$vif" mgmt_vif 0

		[ "$mesh" != "1" ] && [ "$opmode" != "$mode_display" ] && {
			continue
		}

		local mesh_disabled=$(uci -q get mesh.wifi.disabled)

		[ "$mesh" == "1" ] && [ "$(uci get functionlist.functionlist.CONTROLLER_MODE_SUPPORT_MESH)" == "1" ] && [ "$(uci get apcontroller.capwap.enable)" != "1" ] && continue;
		[ "$mesh" == "1" ] && [ "$(uci get functionlist.functionlist.SUPPORT_MESH_SETTING)" != "1" ] && continue;
		[ "$mesh" == "1" ] && [ "$mesh_disabled" == "1" ] && continue;

		# TDMA
		[ "$opmode" == "ap" -o "$opmode" == "wds_ap" ] && {
			[ "$qboost_enable" == "0" ] && [ "$qboost_vif" == "1" ] && continue;
			[ "$qboost_enable" == "1" ] && [ "$qboost_vif" == "0" -a "$mgmt_vif" == "0" ] && continue;
		}

                case "$nawds:$disabled:$mesh" in
                1:0:1)
		        support_mesh=$(get_sn_wifi_option SUPPORT_NAWDS_MESH)

        		[ "$support_mesh" == "1" ] && {
                        
				enable_mesh_nawds   "$ifname" "$vif"
				### mesh easy-setup v1 ###
                        	local MeshEzSetup=$(uci -q get wireless.$vif.MeshEasySetup)

				support_mesh_ezsetup=$(get_sn_wifi_option SUPPORT_MESH_EASYSETUP)

	                        if [ "$MeshEzSetup" == "1" -a "$support_mesh_ezsetup" == "1" ]; then
        	                        enable_mesh_easysetup "$ifname" "$vif"
                	        fi
			}
			### end ###
                ;;
                1:0:0)
                        enable_wdsbridge   "$ifname" "$vif"
			wds_enable=1
                ;;
                esac
		### mesh easy-setup v2 ###
		local t_C_24g=$(uci -q get network.sys.mesh_configured)
		local t_C_5g=$(uci -q get network.sys.mesh_configured_5g)

		[ -z "$t_C_24g" ] && t_C_24g=0
		[ -z "$t_C_5g" ] && t_C_5g=0

		local t_Configured=$(expr $t_C_24g + $t_C_5g)
		local t_EzSetup=$(uci -q get wireless.$vif.MeshEasySetup)

		if [ "$t_EzSetup" = "2" ] && [ "$t_Configured" = "0" ]; then
			enable_mesh_easysetup_cfg "$vif"
		fi
		### end ###
	done
}

sn_fix_rate_issue_func() {
	local wifi_dev=$1
        local rate_ioctl
      	local ac_mode=$(uci -q get wireless.$wifi_dev.hwmode | grep ac)
	
	[ -n "$ac_mode" ] && ac_mode=1

	config_get rate "$wifi_dev" rate
        [ -n "$rate" ] && [ "$ac_mode" == "1" ] && rate_ioctl=vhtmcs || rate_ioctl=set11NRates

        for ifn in $all_ifnames; do
                ifconfig $ifn down
                ifconfig $ifn up
                DebugPrint "enable interface $ifn and use iwconfig to check interface down or up"
                # Fix "Interface doesn't accept private ioctl, set11NRates (8BE0): Invalid argument"
                [ -n "$rate" ] && delay_cmd 5 iwpriv "$ifn" $rate_ioctl "$rate"
                if [ -n "$rate" ] && [ "$rate" != "0x0" ] && [ "$rate_ioctl" == "set11NRates" ] && [[ "$wifi_dev" == "wifi1" || "$wifi_dev" == "wifi2" ]]; then
                        iwpriv $ifn ldpc 0x0                    #disable ldpc for IOT issue, only for 5G 11N fixed data rate
                else
                        if [ -n "$ldpc" ]; then
                                iwpriv "$ifn" ldpc "$ldpc"
                        else
                                iwpriv "$ifn" ldpc 0x3                  #default setting for ldpc
                        fi
                fi
        done

}

sn_fix_HT20_40_issue_func() {
        # Fix: if HT20_40 cfg, let 2.4g choose proper bandwidth itself when locates in noisy wlan env(mostly it choose 20MHz)
	local wifi_dev=$1
        config_get htmode "$device" htmode auto
        case "$device:$htmode" in
                *0:HT20_40)
                        sleep 3
                        for ifn in $all_ifnames; do
                                (ifconfig $ifn down && sleep 2 && ifconfig $ifn up && DebugPrint "$ifn:down-and-up" )&
                        done
                        ;;
        esac
}

sn_set_channel_final() {
        [ $setchannel -eq 0 -a "$opmode" != "sta" -a "$opmode" != "wds_sta" -a "$opmode" != "sta_ap" ] && {
		#comment out: avoid doing twice ACS when channel config is auto.
                #delay_cmd 5 iwconfig "$ifname" channel "$channel"
                setchannel=1;
        }
}

set_channel_config() {

    support_channelconfig=$(get_sn_wifi_option SUPPORT_CHANNEL_CONFIG)
    [ "$support_channelconfig" == "0" ] && return
    
    /sbin/setChannelConfig.sh $1

}

disable_scan_radio(){

	wifi_dev=$1

	config_get opmode "$device" opmode

	if [ ! -e "/sbin/ScanRadio.sh" -o "$opmode" != "mon" ]; then
		return 0
	fi

	test "$(pgrep -f ScanRadio)" && kill -9 $(pgrep -f ScanRadio)	
	test "$(pgrep -f airodump-ng)" && kill -9 $(pgrep -f airodump-ng)	#maybe no need kill, this process terminated when interface down

	rm /tmp/ScanRadio/ -rf	#remove ap sta scan results
	[ -e "/sys/class/leds/scan_led/brightness" ] && echo 0 > /sys/class/leds/scan_led/brightness	#add protection if no scan_led
}

set_scan_radio(){
	
	wifi_dev=$1

        config_get opmode "$device" opmode
        config_get spectrumMode "$device" spectrumMode 0

        [ "$wifi_dev" == "wifi0" -o "$wifi_dev" == "wifi1" ] && [ "$spectrumMode" == "0" ] && return

        [ "$opmode" == "ap" -a "$spectrumMode" == "1" ] && {
                config_get vifs "$device" vifs

		[ -z $vifs ] && vifs=$(eval "/usr/sbin/foreach wireless wifi-iface device $device")

                for vif in $vifs; do
                        config_get disabled "$vif" disabled
                        config_get mode_display "$vif" mode_display
                        [ "$opmode" == "$mode_display" -a "$disabled" == "0" ] && {
                                scanRadioFolder="/tmp/ScanRadio/"
                                [ ! -d "$scanRadioFolder" ] && mkdir -p "$scanRadioFolder"

                               if [ -e "/sbin/ScanRadio.sh" ]; then
                                       sh /sbin/ScanRadio.sh $vif 10 $wifi_dev &
                               else
                                       DebugPrint "/sbin/ScanRadio.sh not exists"
                               fi
                        }
                done
        }

        [ "$opmode" == "mon" ] && {
                config_get vifs "$device" vifs

		[ -z $vifs ] && vifs=$(eval "/usr/sbin/foreach wireless wifi-iface device $device")

                for vif in $vifs; do
                        config_get ifname "$vif" ifname
                        config_get mode_display "$vif" mode_display
                        [ "$opmode" == "$mode_display" ] && {

				scanRadioFolder="/tmp/ScanRadio/"
				[ ! -d "$scanRadioFolder" ] && mkdir -p "$scanRadioFolder"
				
				ifconfig $ifname up	#not regular flow, just up it before works
				[ -e "/sys/class/leds/scan_led/brightness" ] && echo 1 > /sys/class/leds/scan_led/brightness

				config_get scanProgress "$vif" scanProgress #script to switch channel
				if [ "$scanProgress" == "1" ]; then
					if [ -e "/sbin/ScanRadio.sh" ]; then
						config_get scanInterval "$vif" scanInterval #if value is null, it sets default by ScanRadio.sh.
						sh /sbin/ScanRadio.sh $vif $scanInterval $wifi_dev &
					else
						DebugPrint "/sbin/ScanRadio.sh not exists"
					fi
				fi

				config_get airodumpEnable "$vif" airodumpEnable	#to use airodump for ap sta statistic function
				if [ "$airodumpEnable" == 1 ]; then
					ap_sta_file_prefix="ap_sta_list_"
					ap_sta_list="$scanRadioFolder""$ap_sta_file_prefix""$ifname"
					chkAirodump=$(type airodump-ng | grep is)
					if [ -n "$chkAirodump" ]; then	#airodump tool exists
						[ -z "$airodAgeTime" ] && airodAgeTime=$DefaultAgeOutTime
						screen -d -m airodump-ng $ifname -w $ap_sta_list -K 1 --output-format csv -s 1
					else
						DebugPrint "airodump-ng not exists"
					fi
				fi				
                        }
                done
        }
}

check_scan_radio_enable() {

	isScanRadioEnable=0
	wifi_devs=$(uci show |grep opmode=\'mon\' | awk -F '.' '{print $2}')

	for wifi_dev in $wifi_devs; do
		[ "$isScanRadioEnable" == "1" ] && break

		config_get disabled_dev "$wifi_dev" disabled
		[ "$disabled_dev" == "1" ] && continue

		vifs=$(eval "/usr/sbin/foreach wireless wifi-iface device $wifi_dev")

		for vif in $vifs; do
			config_get mode "$vif" mode
			config_get disabled "$vif" disabled

			if [  "$mode" == "monitor" -a "$disabled" == "0" ]; then
				config_get scanProgress "$vif" scanProgress
				config_get airodumpEnable "$vif" airodumpEnable

				[ "$scanProgress" == "1" -a "$airodumpEnable" == "1" ] && {
					isScanRadioEnable=1
					break
				}
			fi
		done
	done

	echo "$isScanRadioEnable"
}

get_backgroundscan_vif(){

	wifi_dev=$1
	bg_vif=$(ps -w | grep BackgroundScan.sh | grep $wifi_dev | awk '{print $8}')
	echo "$bg_vif"
}

disable_background_scan() {

	wifi_dev=$1

	[ ! -e "/sbin/BackgroundScan.sh" ] && return

	scan_if=$(uci -q get wireless.$wifi_dev"_scan".ifname)

	PID_BGScan=$(ps -w | grep BackgroundScan.sh | grep $wifi_dev | awk '{print $1}')
	PID_Airo=$(ps -w | grep airodump-ng | grep $scan_if | awk '{print $1}')

	kill -9 $PID_BGScan $PID_Airo
	rm /tmp/Background_Scanning/$wifi_dev* -f  #remove with determined wifi_dev and scan_if
	rm /tmp/Background_Scanning/$scan_if* -f

	config_get apsteer "$wifi_dev" apsteer 0
	config_get backgroundscanEnable "$wifi_dev" backgroundscanEnable 0
	config_get vifs "$device" vifs

	for vif in $vifs; do
		config_get mode "$vif" mode
		[ "$mode" == "monitor" ] && {
			[ "$apsteer" -eq "0" -a "$backgroundscanEnable" -eq "0" ] && {
				config_get disabled "$vif" disabled
				[ "$disabled" -eq "0" ] && {
					uci -q set wireless.$vif.disabled="1"
					config_set "$vif" disabled 1
					uci commit wireless
				}
			}
		}
	done
}

set_background_scan() {

	local wifi_dev=$1
	local isFastroaming=0 # if 1, use exttool to switch channel
	local backgroundscan=1
	local fastroamingOnly=2
	local backgroundscan_fastroaming=3
	local apsteerOnly=4
	local backgroundscan_apsteer=5
	local bgmode=0
	local bg_vif
	local ap_vif

	config_get opmode "$wifi_dev" opmode

	if [ "$opmode" == "ap" -o "$opmode" == "wds_ap" ]; then {
		config_get backgroundscanEnable "$wifi_dev" backgroundscanEnable
		config_get vifs "$wifi_dev" vifs
		config_get apsteer_enable "$wifi_dev" apsteer 0

		# senao WAR : mesh enable -> SCAN SSID disable
		[ "$(uci get wifiprofile.snWifiConf.SUPPORT_WAR_MESH_DISABLE_SCAN_SSID)" == "1" -a "$(uci get mesh.wifi.disabled)" == "0" ] && {
			backgroundscanEnable=0
			apsteer_enable=0
		}


		if [ "$apsteer_enable" == 1 ]; then	#wireless.wifix.apsteer and wifisyncd.wifisyncd.enabled should be enabled to start apsteer function
			apsteer_enable=$(uci get wifisyncd.wifisyncd.enabled)
		fi

		[ -z $vifs ] && vifs=$(eval "/usr/sbin/foreach wireless wifi-iface device $wifi_dev")

# Make 11r(fastroaming) independent from background scan. Not to check 11r to decide whether background scan turns on or not.
#		for vif in $vifs; do
#			config_get ifname "$vif" ifname
#			config_get disabled "$vif" disabled
#			config_get fastroamingEnable "$vif" fastroamingEnable
#
#			if [ "$disabled" == "0" ]; then {
#				config_get mode "$vif" mode
#				[ "$mode" == "ap" ] && [ -z $ap_vif ] && ap_vif=$vif
#				[ "$fastroamingEnable" == "1" ] && {
#					isFastroaming=1
#					break
#				}
#			}
#			fi
#		done

		haveScanRadio=$(check_scan_radio_enable)

		[ "$backgroundscanEnable" == "1" -o "$apsteer_enable" == "1" ] && [ "$haveScanRadio" != "1" ] && { # vif must be a monitor mode to enable airodump-ng
	        	for vif in $vifs; do
				config_get mode "$vif" mode
                		config_get disabled "$vif" disabled
				[ "$mode" == "monitor" -a "$disabled" == "0" ] && {
					bg_vif=$vif
					break
				}
			done
		}

		if [ -e "/sbin/BackgroundScan.sh" ]; then
			if [ "$backgroundscanEnable" == "1" -a "$apsteer_enable" == "1" ]; then
				bgmode=$backgroundscan_apsteer
			elif [ "$backgroundscanEnable" == "1" -a "$isFastroaming" == "0" ]; then
				bgmode=$backgroundscan
			elif [ "$backgroundscanEnable" == "1" -a "$isFastroaming" == "1" ]; then
				bgmode=$backgroundscan_fastroaming
			elif [ "$backgroundscanEnable" == "0" -a "$apsteer_enable" == "0" -a "$isFastroaming" == "1" ]; then
				bgmode=$fastroamingOnly
			elif [ "$backgroundscanEnable" == "0" -a "$apsteer_enable" == "1" ]; then
				bgmode=$apsteerOnly
			else
				DebugPrint "No backgroundscan mode"
				DebugPrint "backgroundscanEnable=$backgroundscanEnable, isFastroaming=$isFastroaming, apsteer_enable=$apsteer_enable"
			fi

			if [ "$bgmode" != "0" ]; then
				capwapEnabled=$(uci -q get wifiprofile.snWifiConf.SUPPORT_CAPWAP_IWLIST_SCAN)	#for EWS series WTP, not to use monitor mode to scan, just exttool change channel and displayscan
				if [ "$capwapEnabled" == "1" ]; then
					bgmode=$fastroamingOnly	 #we can just use this mode since EWS doesn't have apsteering, rogue ap detection use iwlist displayscan
					for vif in $vifs; do
						config_get mode "$vif" mode
						config_get disabled "$vif" disabled
						[ "$mode" == "ap" -a "$disabled" == "0" -a -z $bg_vif ] && {
							bg_vif=$vif
							break
						}
					done
				fi

				if [ -n "$bg_vif" ]; then
					sh /sbin/BackgroundScan.sh $wifi_dev $bg_vif $bgmode $haveScanRadio &
				else
					sh /sbin/BackgroundScan.sh $wifi_dev $vif $bgmode $haveScanRadio &
				fi
			else
				DebugPrint "do nothing for background scanning"
			fi
		else
			DebugPrint "/sbin/BackgroundScan.sh not exists"
		fi
	}
	fi
}

set_bridge_mac_to_proc() {
	support_senaolog=$(get_sn_wifi_option SUPPORT_SENAOLOG)

	[ "$support_senaolog" == "0" ] && return

	bridge_mac=$(uci get network.lan.macaddr)

	[ -z "$bridge_mac" ] && bridge_mac=$(ifconfig br-lan | grep HWaddr | awk -F " " '{print $5}')

	echo $bridge_mac > /proc/bridge_mac
}

notify_ubus_to_unbridge()
{
	local interface=$1
	local tmp=$(get_bridge_name $interface)
	local network=${tmp#br-}
	local counter=0
	while [ -z "$network" -a $counter -lt 5 ]
	do
		#echo -------------- > /dev/console
		#brctl show  > /dev/console
		tmp=$(get_bridge_name $1)
		network=${tmp#br-}
		sleep 1
		counter=$(($counter + 1))
	done

        #echo ==========interface=$interface network=$network =========== > /dev/console
	if [ -n "$network" ]; then
		ubus call network.interface."$network" remove_device "{ \"name\" : \"$interface\"}"
	else
		DebugPrint "$interface not in bridge, no need to notify ubus to unbridge"
	fi

	return

}

set_legacy_deny()
{
    local ignore_legacy=0
    local wifi_dev=$1

    config_get opmode "$wifi_dev" opmode
    config_get legacy_hwmode_deny "$wifi_dev" legacy_hwmode_deny

    if [ "$legacy_hwmode_deny" != "0" -a "$legacy_hwmode_deny" != "1" ]; then
        return
    fi

    if [ "$opmode" == "ap" -o "$opmode" == "wds_ap" ]; then
        config_get vifs "$wifi_dev" vifs

	[ -z $vifs ] && vifs=$(eval "/usr/sbin/foreach wireless wifi-iface device $wifi_dev")

        for vif in $vifs; do
            config_get ifname "$vif" ifname
            config_get disabled "$vif" disabled
            config_get mode_display "$vif" mode_display

            [ "$disabled" == "0" -a "$mode_display" == "ap" ] && {
                iwpriv "$ifname" puren "$legacy_hwmode_deny"
                #echo "interface:[$vif] legacy_hwmode_deny:[$legacy_hwmode_deny]" > /dev/console
            }
        done
    fi
}

mesh_RP_config()
{
	if [ "$(uci get mesh.wifi.disabled)" == "0" ]; then
		[ -e "/tmp/myNextHopMAC" ] && rm /tmp/myNextHopMAC
		mac_str=$(cat /tmp/wifi0_mac.txt | sed 's/://g')
		uci set wireless.wifi0_rp_ap.ssid="MESH_RP_AP${mac_str:6:6}"
		mac_str=$(cat /tmp/wifi1_mac.txt | sed 's/://g')
		uci set wireless.wifi1_rp_ap.ssid="MESH_RP_AP${mac_str:6:6}"
		if [ "$(uci -q get mesh.wifi.support5g2)" == "1" ]; then
			mac_str=$(cat /tmp/wifi2_mac.txt | sed 's/://g')
			uci set wireless.wifi2_rp_ap.ssid="MESH_RP_AP${mac_str:6:6}"
		fi
		mesh_key=$(uci get wireless.wifi0_mesh.aeskey)
		uci set wireless.wifi0_rp_ap.key=$mesh_key
		uci set wireless.wifi0_rp_sta.key=$mesh_key
		uci set wireless.wifi1_rp_ap.key=$mesh_key
		uci set wireless.wifi1_rp_sta.key=$mesh_key
		if [ "$(uci -q get mesh.wifi.support5g2)" == "1" ]; then
			uci set wireless.wifi2_rp_ap.key=$mesh_key
			uci set wireless.wifi2_rp_sta.key=$mesh_key
		fi

		if [ "$(uci -q get wireless.wifi0_mesh.disabled)" == "0" ]; then
			uci set wireless.wifi0_rp_ap.disabled=0
			uci set wireless.wifi0_rp_sta.disabled=0
			uci set wireless.wifi1_rp_ap.disabled=1
			uci set wireless.wifi1_rp_sta.disabled=1
		elif [ "$(uci -q get wireless.wifi1_mesh.disabled)" == "0" ]; then
			uci set wireless.wifi0_rp_ap.disabled=1
			uci set wireless.wifi0_rp_sta.disabled=1
			uci set wireless.wifi1_rp_ap.disabled=0
			uci set wireless.wifi1_rp_sta.disabled=0
			if [ "$(uci -q get mesh.wifi.support5g2)" == "1" ]; then
				uci set wireless.wifi2_rp_ap.disabled=0
				uci set wireless.wifi2_rp_sta.disabled=0
			fi
		elif [ "$(uci -q get wireless.wifi2_mesh.disabled)" == "0" ]; then
			if [ "$(uci -q get mesh.wifi.support5g2)" == "1" ]; then
				uci set wireless.wifi0_rp_ap.disabled=1
				uci set wireless.wifi0_rp_sta.disabled=1
				uci set wireless.wifi1_rp_ap.disabled=0
				uci set wireless.wifi1_rp_sta.disabled=0
				uci set wireless.wifi2_rp_ap.disabled=0
				uci set wireless.wifi2_rp_sta.disabled=0
			fi
		fi
	else
		uci set wireless.wifi0_rp_ap.disabled=1
		uci set wireless.wifi0_rp_sta.disabled=1
		uci set wireless.wifi1_rp_ap.disabled=1
		uci set wireless.wifi1_rp_sta.disabled=1
		if [ "$(uci -q get mesh.wifi.support5g2)" == "1" ]; then
			uci set wireless.wifi2_rp_ap.disabled=1
			uci set wireless.wifi2_rp_sta.disabled=1
		fi
	fi
}

set_vap_wmm_dscp()
{
	ifname=$1
	device_if $ifname set_dscp_ovride 1
	device_if $ifname s_dscp_tid_map 0xe0 0 #56
	device_if $ifname s_dscp_tid_map 0xc0 0 #48
	device_if $ifname s_dscp_tid_map 0xb8 6 #46
	device_if $ifname s_dscp_tid_map 0xa0 4 #40
	device_if $ifname s_dscp_tid_map 0x98 4 #38
	device_if $ifname s_dscp_tid_map 0x90 4 #36
	device_if $ifname s_dscp_tid_map 0x88 4 #34
	device_if $ifname s_dscp_tid_map 0x80 5 #32
	device_if $ifname s_dscp_tid_map 0x78 4 #30
	device_if $ifname s_dscp_tid_map 0x70 4 #28
	device_if $ifname s_dscp_tid_map 0x68 4 #26
	device_if $ifname s_dscp_tid_map 0x60 4 #24
	device_if $ifname s_dscp_tid_map 0x58 3 #22
	device_if $ifname s_dscp_tid_map 0x50 3 #20
	device_if $ifname s_dscp_tid_map 0x48 3 #18
	device_if $ifname s_dscp_tid_map 0x40 0 #16
	device_if $ifname s_dscp_tid_map 0x38 2 #14
	device_if $ifname s_dscp_tid_map 0x30 2 #12
	device_if $ifname s_dscp_tid_map 0x28 2 #10
	device_if $ifname s_dscp_tid_map 0x20 1 #8
	device_if $ifname s_dscp_tid_map 0x0  0 #0
}

set_wmm_dscp()
{
#
# WMM         802.1p    DSCP
# Voice        7
#              6         46
# Video        5         32
#              4         24, 26, 28, 30, 34, 36, 38, 40
# Best Effort  3         18, 20, 22
#              0         0, 16,48,56
# Background   2         10, 12, 14
#              1         8

	# There is a issue if cpu is MIPS architecture that wmm dscp modify cause 2.4g wifi encryption fail.
	[ -n "$(cat /proc/cpuinfo | grep cpu | grep MIPS)" ] && return

	for vap in $(iwconfig 2>/dev/null | grep ^ath | awk {'print $1'}); do
		for iface in $(foreach wireless wifi-iface); do
			if [ "$vap" == "$(uci get wireless.$iface.ifname)" ]; then
				if [ "$(uci get wireless.$iface.mode)" == "ap" ]; then
					set_vap_wmm_dscp $vap
				fi
			fi
		done
	done
}

fix_fastroam()
{
#2020.11.26 Senao Sam(written by Senao George), fix fast-roaming issue in IPQ40xx.
#The client will be not connect to AP after several times of roaming.

	for vap in $(iwconfig 2>/dev/null | grep ^ath | awk {'print $1'}); do
		for iface in $(foreach wireless wifi-iface); do
			if [ "$vap" == "$(uci get wireless.$iface.ifname)" ]; then
				if [ "$(uci get wireless.$iface.mode)" == "ap" ] && [ "`cat /tmp/sysinfo/model | grep IPQ40xx`" ] ; then
					wifitool $vap beeliner_fw_test 174 0
				fi
			fi
		done
	done
}

chk_wds_ap()
{
    #chk wds ap is exist or not
    [ -f "$PROC_SN_ROLE" ] || return

    local sn_reconn_enable=0
    local sn_reconn_role=0 # wds-root:1, wds-node:2, else:0

    for vap in $(iwconfig 2>/dev/null | grep ^ath | awk {'print $1'}); do
	for iface in $(foreach wireless wifi-iface); do
            if [ "$vap" == "$(uci get wireless.$iface.ifname)" ]; then
                if [ "$(uci get wireless.$iface.mode_display)" == "wds_ap" ]; then
                    #echo ">>>>>chk_wds_ap:$vap" > /dev/console
                    sn_reconn_role=1
                    sn_reconn_enable=0
                fi
                if [ "$(uci get wireless.$iface.mode_display)" == "wds_bridge" ]; then
                    #echo ">>>>>chk_wds_bridge:$vap" > /dev/console
                    sn_reconn_role=2
                    sn_reconn_enable=1
                fi
            fi
	done
    done
    
    #echo -e "\033[33m sn_reconn_role:[$sn_reconn_role] sn_reconn_enable:[$sn_reconn_enable]\033[0m" > /dev/console
    echo "$sn_reconn_role" > "$PROC_SN_ROLE"
    uci set sn_reconn.global.enable=$sn_reconn_enable
    uci commit sn_reconn
}

reset_mesh_for_RP()
{
	if [ "$(uci get mesh.wifi.disabled)" == "0" ]; then
		mesh_if=""
		if [ "$(uci -q get wireless.wifi0_mesh.disabled)" == "0" ]; then
			mesh_if="$(uci get wireless.wifi0_mesh.ifname)"
		elif [ "$(uci -q get wireless.wifi1_mesh.disabled)" == "0" ]; then
			mesh_if="$(uci get wireless.wifi1_mesh.ifname)"
		elif [ "$(uci -q get wireless.wifi2_mesh.disabled)" == "0" ]; then
			mesh_if="$(uci get wireless.wifi2_mesh.ifname)"
		fi
		if [ -n "$mesh_if" ]; then
			echo "[wifi_funs/mesh_RP_config] reset mesh interface [$mesh_if] for RP connection" > /dev/console
			ifconfig $mesh_if down up
		fi
	fi
}

set_11k_prescan() {     #imediately scan and set scan timer

    wifi_dev=$1
    scan_vap=$(sh /sbin/getWifiFirstIF $wifi_dev)
    config_get channel $wifi_dev channel
    config_get opmode $wifi_dev opmode

    if [ "$(uci -q get wifiprofile.snWifiConf.SUPPORT_80211K_RRM)" = "0" ]; then
    DebugPrint "not support 11k prescan on the wifi profile features"
    return
    fi

    if [ "$opmode" != "ap" -a "$opmode" != "wds_ap" ]; then
    DebugPrint "no need 11k prescan on $wifi_dev since opmode is $opmode"
    return
    fi

    [ "$channel" != "0" -a "$channel" != "auto" ] && iwlist $scan_vap scan &

    [ -e "/etc/crontabs/root" -a -n "$(cat /etc/crontabs/root | grep 11kPreScan.sh)" ] && {
    sed -i '/11kPreScan/d' /etc/crontabs/root
    }
    echo "59 23 * * * /bin/sh /sbin/11kPreScan.sh &" >> /etc/crontabs/root
}


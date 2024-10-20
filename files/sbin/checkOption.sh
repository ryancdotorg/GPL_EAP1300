#!/bin/sh
#Author:Senao, Jacky Chou

#2020.05.04 Senao, George Lin
#Using checkOption.sh in luci-reload (/SENAO/host/base-files/sbin/luci-reload)
#to boost wifi reload process.

funcDebug=0
default_cfg80211=0

if [ ! -e "/sbin/layer_reload.sh" ]; then
	return
fi

wchange_file="/tmp/wireless_changes"
command_file="/tmp/reload_cmds.sh"
uid=""
ds=""
dd=""
check_option_log_file=""
check_option_log_need="$(uci get functionlist.functionlist.SUPPORT_LUCI_RELOAD_LOG 2>&1)"
check_option_log_do=0

DebugPrint(){
	[ "$funcDebug" == "1" ] && echo "$@" > /dev/console
}

remove_empty_lines(){
	sed -i '/^$/d' $wchange_file
}

remove_ignore_options(){
	while read f1
	do
		sed -i '/\.'$f1'=/d' $wchange_file	#the . must be ".", for some case like id will misjudge to affect ssid
	done < /lib/wifi/ignoreOption.csv
}
remove_radio_below_changes(){
	device=$1
	vifs=$(eval "/usr/sbin/foreach wireless wifi-iface device $device")
	DebugPrint "remove_radio_below_changes, device=$device, vifs=$vifs $wchange_file"
	sed -i '/wireless.'$device'./G' $wchange_file
	sed -i '/wireless.'$device'./d' $wchange_file
	for vif in $vifs; do
		sed -i '/wireless.'$vif'./G' $wchange_file
		sed -i '/wireless.'$vif'./d' $wchange_file
	done
}

remove_vap_below_changes(){
	vif=$1
	DebugPrint "remove_vap_below_changes, vif=$vif $wchange_file"
	sed -i '/wireless.'$vif'./G' $wchange_file
	sed -i '/wireless.'$vif'./d' $wchange_file
}

remove_current_cmd_change(){
	vif=$1
	option=$2
	DebugPrint "remove_vap_below_changes, vif=$vif, option=$option $wchange_file"
	sed -i '/wireless.'$vif'.'$option'=/G' $wchange_file
	sed -i '/wireless.'$vif'.'$option'=/d' $wchange_file
}

remove_related_bandsteer_changes(){
	index=$1
	sed -i '/wireless.wifi0_ssid_'$index'.bandsteer/G' $wchange_file
	sed -i '/wireless.wifi1_ssid_'$index'.bandsteer/G' $wchange_file
	sed -i '/wireless.wifi0_ssid_'$index'.bandsteer/d' $wchange_file	#temp using
	sed -i '/wireless.wifi1_ssid_'$index'.bandsteer/d' $wchange_file
}

remove_related_fasthandover_changes(){
        vif=$1
	sed -i '/wireless.'$vif'.fasthandover_/G' $wchange_file
        sed -i '/wireless.'$vif'.fasthandover_/d' $wchange_file       #temp using
}

log_start() {
	[ -f "/tmp/luci_reload_uid" ] && uid=$(cat "/tmp/luci_reload_uid")
	[ -n "$uid" ] && {
	    check_option_log_file="/tmp/check_option.log.$uid"
	    ds=$(date +%s)
	}
}

log_end() {
	    [ -n "$check_option_log_file" ] && {
		pid=$$
		dd=$(date +%s)
		[ -f "$check_option_log_file" ] && rm -rf "$check_option_log_file"
		echo "============================== check option log ==============================" >> "$check_option_log_file"
		echo "pid:$pid" >> "$check_option_log_file"
		echo "duration:$((dd-ds))" >> "$check_option_log_file"
		echo "layer:$layer" >> "$check_option_log_file"
	    }
}

quit() {
	### end log
	[ "$check_option_log_do" == "1" ] && log_end
	exit
}

get_wifisyncd_status() {

	pid=$(pgrep wifi_syncd)
	if [ -z $pid ]; then
		echo 0		#not activate
	else
		echo 1		#running
	fi
}

check_need_wifisyncd() {
	wifi0_apsteer_enable=$(($(uci -q get wireless.wifi0.disabled)^1 && $(uci get wireless.wifi0.apsteer)))
        wifi1_apsteer_enable=$(($(uci -q get wireless.wifi1.disabled)^1 && $(uci get wireless.wifi1.apsteer)))
	wifisyncd_bansteer_enable=$(uci -q get wifisyncd.wifisyncd.bandsteer_en)
	if [ "$wifi0_apsteer_enable" == "1" -o "$wifi1_apsteer_enable" == "1" -o "$wifisyncd_bansteer_enable" == "1" ]; then
		echo 1
	else
		echo 0
	fi
}

### start log
[ -n "$check_option_log_need" ] && [ "$check_option_log_need" == "1" ] && check_option_log_do=1
[ "$check_option_log_do" == "1" ] && log_start

if [ "$default_cfg80211" == "1" ]; then
	device_if="cfg80211tool"
else
	device_if="iwpriv"
fi

top_layer=0
ap_bridge_changed=0

rm $command_file -f

optionNum=$(uci changes wireless | wc -l) #wireless option num

#totalOptionNum=$(uci changes | wc -l)

#if [ $totalOptionNum -gt $optionNum ]; then
#	DebugPrint "total uci changes num=$totalOptionNum > wireless option num $optionNum, not to do wifi boost reload"
#	layer=3 #ap layer
#	quit #quit script
#fi

uci changes wireless > $wchange_file

remove_ignore_options

optionNum=$(cat $wchange_file | wc -l)

for i in `seq 1 $optionNum`; do
	option=$(cat $wchange_file | head -n $i | tail -n 1 | awk -F "[.=]" '{print $3}')
	#section=$(cat $wchange_file | head -n $i | tail -n 1 | awk -F "[.=]" '{print $2}')
	#value=$(cat $wchange_file | head -n $i | tail -n 1 | awk -F "[.=]" '{print $4}')
	while read f1
	do
		if [ "$f1" == "$option" ]; then
			DebugPrint "found option $option, got to reload network"
			layer=3	#ap layer
			break
		fi
	done < /lib/wifi/apLOption.csv
done

if [ "$layer" == "3" ]; then	#found ap layer option, got to reload wireless in old flow, skip speed up flow
    quit #quit script
fi

optionNum=$(cat $wchange_file | wc -l)

radioReloadNum=0	#if radio reload number >= 2, we want to to reload network

if [ $(get_wifisyncd_status) != $(check_need_wifisyncd) ]; then		#wifisyncd is the package got to be reloaded before wifi
	echo "luci-reload auto wifisyncd" >> $command_file
fi

for i in `seq 1 $optionNum`; do
	option=$(cat $wchange_file | head -n $i | tail -n 1 | awk -F "[.=]" '{print $3}')
	section=$(cat $wchange_file | head -n $i | tail -n 1 | awk -F "[.=]" '{print $2}')
	value=$(cat $wchange_file | head -n $i | tail -n 1 | awk -F "[.=]" '{print $4}')
	isRadioSection=0

	#duplicated option: wifix.disabled, wifix_ssid_x.disabled
	[ "$section" == "wifi0" -o  "$section" == "wifi1" -o "$section" == "wifi2" ] && isRadioSection=1

	if [ "$option" == "disabled" -a "$isRadioSection" == "1" ]; then
		layer=2
		DebugPrint "found $section $option set $value, reload radio"
		echo "sh /sbin/layer_reload.sh $layer $section" >> $command_file
		remove_radio_below_changes $section
		[ "$value" != "1" ] && radioReloadNum=$(($radioReloadNum+1))	#if disable radio, this reload only disable_qcawifi, no need uplevel to network
		continue
	else
		while read f1
		do
			if [ "$f1" == "$option" ]; then
				layer=2 #radio layer, in this layer, the section equals to radio device
				if [ -n "`uci -q get wireless.$section.device`" ]; then
					section=$(uci -q get wireless.$section.device)
				fi
				DebugPrint "found radio option $option, reload radio $section"
				echo "sh /sbin/layer_reload.sh $layer $section" >> $command_file
				remove_radio_below_changes $section
				radio_disabled=$(uci -q get wireless.$section.disabled)
				[ "$radio_disabled" != "1" ] && radioReloadNum=$(($radioReloadNum+1))	#if radio is disabled, no need to uplevel to network
				break
			fi
		done < /lib/wifi/radioLOption.csv
	fi
done

[ $radioReloadNum -ge 2 ] && quit	#return before remove empty lines to make boost_reload.sh not to boost

remove_empty_lines

[ $layer -gt $top_layer ] && top_layer=$layer

optionNum=$(cat $wchange_file | wc -l)

vapReloadNum=0

for i in `seq 1 $optionNum`; do
	option=$(cat $wchange_file | head -n $i | tail -n 1 | awk -F "[.=]" '{print $3}')
	vif=$(cat $wchange_file | head -n $i | tail -n 1 | awk -F "[.=]" '{print $2}')
	value=$(cat $wchange_file | head -n $i | tail -n 1 | awk -F "[.=]" '{print $4}')
	while read f1
        do
                if [ "$f1" == "$option" ]; then
			wifi_dev=$(uci -q get wireless.$vif.device)
			radio_disabled=$(uci -q get wireless.$wifi_dev.disabled)
			DebugPrint "found vap option $option, reload vap $vif"
			remove_vap_below_changes $vif
			if [ "$radio_disabled" != "1" ]; then	#no need to reload vap if the parent radio is disabled
				layer=1 #vap layer
				echo "sh /sbin/layer_reload.sh $layer $vif" >> $command_file
				vapReloadNum=$(($vapReloadNum+1))
			else
				DebugPrint "no need to reload vap $vif since the parent wifi device $wifi_dev is disabled"
			fi
			break
                fi
        done < /lib/wifi/vapLOption.csv
done

vapReloadNum=$(($radioReloadNum*6+$vapReloadNum))

[ $vapReloadNum -ge 10 ] && quit	#if reload too many vaps or vaps + radio, we can just reload network

remove_empty_lines

[ $layer -gt $top_layer ] && top_layer=$layer

optionNum=$(cat $wchange_file | wc -l)
for i in `seq 1 $optionNum`; do
        option=$(cat $wchange_file | head -n $i | tail -n 1 | awk -F "[.=]" '{print $3}')
        vif=$(cat $wchange_file | head -n $i | tail -n 1 | awk -F "[.=]" '{print $2}')
        value=$(cat $wchange_file | head -n $i | tail -n 1 | awk -F "[.=]" '{print $4}' | awk -F "\'" '{print $2}')
        while read f1
        do
                if [ "$f1" == "$option" ]; then
			layer=0
			vif_disabled=$(uci -q get wireless.$vif.disabled)
			if [ "$vif" == "wifi0" -o  "$vif" == "wifi1" -o "$vif" == "wifi2" ]; then	#option in radio level
				if [ "$option" == "txpower" -o "$option" == "channel" ]; then
					if [ "$value" == "auto" ]; then #no matter channel or txpower, the command uses value 0 stands for auto
						value=0
					fi
					radioIdx=$(echo $vif | awk -F "wifi" '{print $2}')
					ifname=$(sh /sbin/getWifiFirstIF $radioIdx)
					DebugPrint "found special command option $option"
					DebugPrint "use command: iwconfig $ifname $option $value for this change"
					[ "$option" == "channel" ] && echo "/sbin/setChannelConfig.sh $vif" >> $command_file
					echo "iwconfig $ifname $option $value" >> $command_file
					remove_current_cmd_change $vif $option
				elif [ ! -z $(echo $option | grep fasthandover) ]; then
					opmode=$(uci -q get wireless.$vif.opmode)
					[ "$opmode" != "ap" ] &&  fasthandover_status="0" || fasthandover_status=$(uci -q get wireless."$vif".fasthandover_status)
					fasthandover_rssi=$(uci -q get wireless."$vif".fasthandover_rssi)
					if [ "$fasthandover_rssi" -lt -95 ]; then
						fasthandover_rssi=-95
					fi
					case "$vif" in
					wifi0)
						echo ""$fasthandover_status" "$fasthandover_rssi" > /proc/ap_roaming" >> $command_file
					;;
					wifi1)
						echo ""$fasthandover_status" "$fasthandover_rssi" > /proc/ap_roaming_5g" >> $command_file
					;;
					wifi2)
						echo ""$fasthandover_status" "$fasthandover_rssi" > /proc/ap_roaming_5g_2" >> $command_file
					;;
					esac
					remove_related_fasthandover_changes $vif
				fi
			else	#option in vap level
				if    [ ! -z $(echo $option | grep bands) ]; then     #bandsteer reladted command
					fastroaming=$(uci -q get wireless.$vif.fastroamingEnable)
					if [ $fastroaming == "1" -a $option == "bandsteer_en" -a $value == "1" ]; then
						DebugPrint "$option set to $value and the $vif fastroaming=$fastroaming, do network reload"
						quit #quit script
					else
						DebugPrint "found cmd option $option, exec /sbin/sn_bandsteering"
						index="$(echo $vif |awk -F '_' '{print $3}')"
						echo "/sbin/sn_bandsteering $index" >> $command_file
						remove_current_cmd_change $vif $option
						remove_related_bandsteer_changes $index
					fi
				elif	[ "$option" == "isolate" ]; then
					ifname=$(uci -q get wireless.$vif.ifname)
					l2_isolatior=$(uci -q get wireless.$vif.l2_isolatior)
					DebugPrint "option=$option, device_if "$ifname" ap_bridge "$((value^1))""
					if [ "$vif_disabled" != "1" ]; then
						if [ "$l2_isolatior" != "1" ]; then
							echo "$device_if $ifname ap_bridge "$((value^1))"" >> $command_file
							ap_bridge_changed=1
						fi
					fi
					remove_current_cmd_change $vif $option
				else	#no need to exec layer_reload.sh
					ifname=$(uci -q get wireless.$vif.ifname)
		                        DebugPrint "found regular cmd option $option, $device_if $ifname $option $value"
					[ "$vif_disabled" != "1" ] && echo "$device_if $ifname $option $value" >> $command_file
					remove_current_cmd_change $vif $option
				fi
			fi
			break
                fi
	done < /lib/wifi/cmdLOption.csv
done

remove_empty_lines

if [ $top_layer -gt 0 ]; then
	echo "/usr/sbin/iface_mark.sh &" >> $command_file
	echo "luci-reload auto appanalysis" >> $command_file
	echo "luci-reload auto ndpi" >> $command_file
	echo "luci-reload auto sntcd" >> $command_file
	echo "luci-reload auto fingerprint" >> $command_file
	echo "luci-reload auto wifi_schedule" >> $command_file
	echo "luci-reload auto led" >> $command_file
fi

if [ "$ap_bridge_changed" == "1" -o $top_layer -gt 0 ]; then
	echo "luci-reload auto l2_isolation" >> $command_file
fi

### end log
[ "$check_option_log_do" == "1" ] && log_end


#!/sbin/bash

atf_enabled=$(uci get airtime_fairness.atf.enabled)
atf_changed=$(uci get airtime_fairness.atf.parameter_changed)
atf_vip=$(uci get airtime_fairness.atf.vip_list)

if [ "$atf_enabled" != "1" ]; then
	exit 0
fi

SET_ATF_BY_SSID=0  #maybe use config to set it later, for potential risk, set it off now
SET_ATF_BY_MAC=1   #maybe use config to set it later

wifi_radios="wifi0 wifi1" #wifi2

#file path prefix
sta_list=/tmp/stalist_
old_sta_list=/tmp/old_stalist_
online_vip_list=/tmp/online_vip_list_
old_online_vip_list=/tmp/old_online_vip_list_
online_nvip_list=/tmp/online_nvip_list_
old_online_nvip_list=/tmp/old_online_nvip_list_
set_vip_list=/tmp/set_vip_list

#parameters to fine tune atf function
reserved_atf_percent=10		#used for new connected station
vip_nvip_ratio=2		#the vip:non-vip airtime ratio, for sta using
ap_coefficient=3
gn_coefficient=2

debugEnable=0

delNonVipAtfSetting(){
	local vifName=$1

	if [ -e "$online_nvip_list$vifName" ]; then             #non-vip number
		nonVipNum=$(sed -n '$=' "$online_nvip_list$vifName")

		for i in `seq 1 $nonVipNum`; do
			local nonVipMac=$(cat $online_nvip_list$vifName | head -n $i | tail -n 1| tr -d ":")
			if [ -e "$set_vip_list" -a -n "$nonVipMac" ]; then
				checkVipSet=$(cat $set_vip_list | grep $nonVipMac)
				if [ -n "$checkVipSet" ]; then
					[ $debugEnable -eq 1 ] && echo "we have set $nonVipMac on atf vip before, now delete it since vip list changed" >> /dev/console
					wlanconfig $vifName delsta $nonVipMac
					sed -i '/'$vip'/d' $set_vip_list
				fi
			fi
		done
	fi
}

setStaAtf(){	#generate per vap station list and sepated to vip list and non-vip list, calculate vip percent and then set vip airtime to each vip mac

	local wifidev=$1
        local vifs=$(eval "/usr/sbin/foreach wireless wifi-iface device $wifidev")

	for vif in $vifs; do

                local modeDisplay=$(uci get wireless.$vif.mode_display)
                local vifDisabled=$(uci get wireless.$vif.disabled)
		local vifName=$(uci get wireless.$vif.ifname)

                if [ "$modeDisplay" == "ap" ] && [ "$vifDisabled" == "0" ]; then
                        rm $sta_list$vifName -f
                        rm $online_vip_list$vifName -f
                        rm $online_nvip_list$vifName -f

			wlanconfig "$vifName" list | sed -n '2,$p' | awk '{print $1}' >> $sta_list$vifName

			if [ ! -e $old_sta_list$vifName ]; then			#check the station list changed or not
				listChanged=1
			else	#check the stalist and old stalist by md5sum
				local md5StaList=$(md5sum $sta_list$vifName | awk '{print $1}')
				local md5OldStaList=$(md5sum $old_sta_list$vifName | awk '{print $1}')
				if [ "$md5StaList" != "$md5OldStaList" ]; then
					listChanged=1
				else
					listChanged=0
				fi
			fi
			if [ $listChanged -eq 1 -o "$atf_changed" != "0" ]; then #only reconfig the airtime if sta list or vip list changed
				cp $sta_list$vifName $old_sta_list$vifName
				cp $sta_list$vifName $online_nvip_list$vifName	#sta_list = online_vip_list + online_nvip_list

				local vips=$(uci get airtime_fairness.atf.vip_list)
				if [ "$vips" != "0" ]; then 
					for vip in $vips; do		#separe sta list to vip list and non-vip list
						cat $sta_list$vifName | grep "$vip" >> $online_vip_list$vifName
        		                	sed -i '/'$vip'/G' $online_nvip_list$vifName
        	        	        	sed -i '/'$vip'/d' $online_nvip_list$vifName
					done
				fi
				if [ ! -e "$online_vip_list$vifName" ]; then
					touch "$online_vip_list$vifName"	#if vip not exist, create an empty file
				fi	
				if [ ! -e "$old_online_vip_list$vifName" ];then
					oldNonVipNum=0
					vipListChanged=1
				else
					local md5VipList=$(md5sum $sta_list$vifName | awk '{print $1}')
					local md5OldVipList=$(md5sum $old_sta_list$vifName | awk '{print $1}')
					if [ "$md5VipList" != "$md5OldVipList" ]; then
						vipListChanged=1
					else
						vipListChanged=0
					fi
					if [ -e "$old_online_nvip_list$vifName" ]; then	
						oldNonVipNum=$(sed -n '$=' "$old_online_nvip_list$vifName")
						if [ -z "$oldNonVipNum" ]; then
							oldNonVipNum=0
						fi
					else
						oldNonVipNum=0
					fi
				fi
				sed -i '/^$/d' 	$online_vip_list$vifName		#remove empty lines
				sed -i '/^$/d'  $online_nvip_list$vifName

				if [ -e "$online_vip_list$vifName" ]; then		#vip number
					vipNum=$(sed -n '$=' "$online_vip_list$vifName")
					if [ -z "$vipNum" ]; then
						vipNum=0
					fi
				else
					vipNum=0
				fi
				if [ -e "$online_nvip_list$vifName" ]; then		#non-vip number
					nonVipNum=$(sed -n '$=' "$online_nvip_list$vifName")
					if [ -z "$nonVipNum" ]; then
						nonVipNum=0
					fi
				else
					nonVipNum=0
				fi

				cp $online_nvip_list$vifName $old_online_nvip_list$vifName
				cp $online_vip_list$vifName $old_online_vip_list$vifName

				if [ "$oldNonVipNum" == "$nonVipNum" ] && [ "$vipListChanged" == "0" ]; then	#check if vip list changed or non vip number changed
					[ $debugEnable -eq 1 ] && echo "vip list and non-vip number not changed, no need to update $vifName ATF" >> /dev/console
					continue
				fi

				if [ "$vipNum" == "0" ]; then
					[ $debugEnable -eq 1 ] && echo "no vip connected, got to delete vip settings" >> /dev/console
					#wlanconfig $vifName flushatftable  #should not use this command, it makes all sta disconnected
					delNonVipAtfSetting $vifName
					iwpriv $vifName commitatf 1
					continue
				fi
				vipPercent=$(( $vip_nvip_ratio*(100-$reserved_atf_percent)/(($vipNum*($vip_nvip_ratio))+$nonVipNum)))

				[ $debugEnable -eq 1 ] && echo "$vipNum vip and $nonVipNum normal sta on $vifName, each vip uses $vipPercent"%" airtime of vap" >> /dev/console

				delNonVipAtfSetting $vifName

				for i in `seq 1 $vipNum`; do
					local vipMac=$(cat $online_vip_list$vifName | head -n $i | tail -n 1| tr -d ":")
					[ $debugEnable -eq 1 ] && echo "set $vifName $vipMac $vipPercent % airtime" >> /dev/console
					wlanconfig $vifName addsta $vipMac $vipPercent
					echo $vipMac >> $set_vip_list
				done
				iwpriv $vifName commitatf 1
			else
				[ $debugEnable -eq 1 ] && echo "$vifName do nothing since station list changed=$listChanged parameter_changed=$atf_changed" >> /dev/console
			fi
		fi
	done
}

setATFbySsid(){

	local wifidev=$1
	local vifs=$(eval "/usr/sbin/foreach wireless wifi-iface device $wifidev")
	local apNum=0
	local gnNum=0
	local meshPercent=0
	local gnPercent=0
	for vif in $vifs; do	#get enabled ap number, they will use residual airtime

                local modeDisplay=$(uci get wireless.$vif.mode_display)
                local vifDisabled=$(uci get wireless.$vif.disabled)

                if [ "$modeDisplay" == "ap" ] && [ "$vifDisabled" == "0" ]; then
			local networkType=$(uci get wireless.$vif.network)
			if [ "$networkType" == "guest" ]; then
				gnNum=$(($gnNum+1))
			else
				apNum=$(($apNum+1))
			fi
		elif [ "$modeDisplay" == "mesh" ] && [ "$vifDisabled" == "0" ]; then
			meshPercent=$(uci get wireless.$vif.atfpercent)
		fi
	done

	[ $debugEnable -eq 1 ] && echo "$wifidev apNum=$apNum, gnNum=$gnNum, meshPercent=$meshPercent" >> /dev/console

	if [ $gnNum -ge 1 ]; then 
		gnPercent=$((((100-$meshPercent)/($apNum*$ap_coefficient+$gnNum*$gn_coefficient))*$gn_coefficient))
		[ $debugEnable -eq 1 ] && echo "Guest network uses $gnPercent airtime" >> /dev/console
	fi
	
	for vif in $vifs; do

		local modeDisplay=$(uci get wireless.$vif.mode_display)
		local vifDisabled=$(uci get wireless.$vif.disabled)

		if [ "$modeDisplay" == "ap" -o "$modeDisplay" == "mesh" ] && [ "$vifDisabled" == "0" ]; then	#only mesh and ap need to set atf
			local vifName=$(uci get wireless.$vif.ifname)
			local vifSsid=$(uci get wireless.$vif.ssid)
			local networkType=$(uci get wireless.$vif.network)
			if [ "$networkType" == "guest" ]; then
				if [ $gnPercent -gt 0 ]; then
					[ $debugEnable -eq 1 ] && "echo set guset network $vifName airtime $gnPercent %" >> /dev/console
					wlanconfig "$vifName" addssid "$vifSsid" "$gnPercent"
					iwpriv "$vifName" commitatf 1
				fi
			elif [ "$modeDisplay" == "mesh" ]; then
				if [ "$meshPercent" != "0" ]; then
					echo set mesh vif $vifName airtime $meshPercent % 
					wlanconfig "$vifName" addssid "$vifSsid" "$meshPercent"
					iwpriv "$vifName" commitatf 1
				fi
			else
				[ $debugEnable -eq 1 ] && echo "pure ap $vifName uses residual airtime(others in atf table), no need to set" >> /dev/console
			fi
		fi
	done
}

for wifidev in $wifi_radios; do

        local radioDisabled=$(uci get wireless.$wifidev.disabled)
        local radioOpMode=$(uci get wireless.$wifidev.opmode)
        if [ "$radioDisabled" == "0" ] && [ "$radioOpMode" == "ap" ]; then
		if [ "$atf_changed" != "0" ] && [ $SET_ATF_BY_SSID -eq 1 ]; then
                        setATFbySsid $wifidev
                fi
		if [ $SET_ATF_BY_MAC -eq 1 ]; then
			setStaAtf $wifidev
		fi
        fi
done

if [ "$atf_changed" != "0" ]; then
	[ $debugEnable -eq 1 ] && echo "uci set airtime_fairness.atf.parameter_changed=0" >> /dev/console
	uci set airtime_fairness.atf.parameter_changed=0
fi

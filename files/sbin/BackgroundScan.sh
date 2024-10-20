DebugEnable=0
requireProcessList=0
ageOutTime=120 #time in second for ap sta list entry age out
apsteer=0
default_cfg80211=0
apsteer_boost_scan=0
airodump_recovery=0

device=$1
vif=$2
val=$3
haveScanRadio=$4

BackgroundScanFilePath="/tmp/Background_Scanning/"
apStaFilter="/usr/sbin/scanRadio"
ap_list_filename=$BackgroundScanFilePath$device"_ap_list.txt"
sta_list_filename=$BackgroundScanFilePath$device"_sta_list.txt"
tmp_ap_list_filename=$BackgroundScanFilePath$device"_ap_list.txt.tmp"
tmp_sta_list_filename=$BackgroundScanFilePath$device"_sta_list.txt.tmp"
BGscanChannelList=$BackgroundScanFilePath$device"_ChannelList"
ApsteerChannelList=$BackgroundScanFilePath$device"_ApsteerChannelList"
displayScanResult=$BackgroundScanFilePath$device"_displayscanResult"
displayScanAdvanceResult=$BackgroundScanFilePath$device"_displayscanAdvanceResult"

DebugPrint() {
        [ "$DebugEnable" == "1" ] && echo "$@" > /dev/console
}

processApStaList(){

        vif=$1

	ap_sta_file=$BackgroundScanFilePath$ifname"_ap_sta_list-01.csv"

        if [ -f "$apStaFilter" ]; then {
		$apStaFilter -i $ap_sta_file -a $ap_list_filename -s $sta_list_filename -e $ageOutTime
        } else {
		#separate AP_STA_LIST to ap list and sta list
		cat $ap_sta_file  | tr -d '\r' | tail -n +3 | sed '/Station MAC/,$d' > $tmp_ap_list_filename
        	cat $ap_sta_file  | tr -d '\r' | sed -n '/Station/,$p'| tail -n +2 > $tmp_sta_list_filename

		#filter entry with channel/rate -1
		echo "$(awk -F ',' '($4 != -1 && $5 != -1){print $0}' $tmp_ap_list_filename)" > $tmp_ap_list_filename
		echo "$(awk -F ',' '($4 != -1){print $0}' $tmp_sta_list_filename)" > $tmp_sta_list_filename

	        filterApListByTime $vif
        	filterStaListByTime $vif

	        mv $tmp_ap_list_filename $ap_list_filename
        	mv $tmp_sta_list_filename $sta_list_filename
	}
	fi
}

filterApListByTime(){                                                                                            

        vif=$1                                                                                         
                              
	lineNum=$(cat $tmp_ap_list_filename | grep -v '^\s*$' | wc -l)                                       
        WORKING_FILE=$BackgroundScanFilePath$vif"working_list.tmp"
                                                          
	[ -d "$WORKING_FILE" ] && rm -f $WORKING_FILE                                                                                                           
 
        nowTimeStamp=$(date +%s)                                                                            
        ageOut=$(uci -q get wireless.$vif.ageOutTime)                                                            
        [ -n "$ageOut" ] || ageOut=$DefaultAgeOutTime                                                            
                                                                                                                 
        ageOutTimeStamp=$(( $nowTimeStamp - $ageOut))                                                  
                                                    
        for i in `seq 1 $lineNum`; do   
                                                              
                time=$(awk -v line=$i -F '[ ,]+' 'NR == line { print $5; exit }' $tmp_ap_list_filename)
		
		[ -n "$time" ] && timeStamp=$(date +%s -d $time)              
                                                            
                if [ $timeStamp -ge $ageOutTimeStamp ]; then
                        head -n $i $tmp_ap_list_filename | tail -n 1 >> $WORKING_FILE
                fi                                   
        done         
        mv $WORKING_FILE $tmp_ap_list_filename       
}                                                   

filterStaListByTime(){                                                                                      

        vif=$1                                                                                             
                                                     
        lineNum=$(cat $tmp_sta_list_filename | grep -v '^\s*$' | wc -l)                                                            
                                                                                                       
        WORKING_FILE=$BackgroundScanFilePath$vif"working_list.tmp"

	[ -d "$WORKING_FILE" ] && rm -f $WORKING_FILE                                  
                                                              
        nowTimeStamp=$(date +%s)                              
        ageOut=$(uci -q get wireless.$vif.ageOutTime)                                                       
        [ -n "$ageOut" ] || ageOut=$DefaultAgeOutTime                                                       
                                                                                                            
        ageOutTimeStamp=$(( $nowTimeStamp - $ageOut))                                                            
                                                                                                                 
        for i in `seq 1 $lineNum`; do                                                                            
                                                                                                       
                time=$(awk -v line=$i -F '[ ,]+' 'NR == line { print $5; exit }' $tmp_sta_list_filename)

                [ -n "$time" ] && timeStamp=$(date +%s -d $time)
 
                if [ $nowTimeStamp -ge $ageOutTimeStamp ]; then                                        
                        head -n $i $tmp_sta_list_filename | tail -n 1 >> $WORKING_FILE                  
                fi                                                                                      
        done                                                                         
        mv $WORKING_FILE $tmp_sta_list_filename                                      
}                                                                                     

check_airodump() {

	vif=$1
	ifname=$(uci get wireless.$vif.ifname)

	[ -z "$ifname" ] && {
		DebugPrint "Background scanning - airodump-ng fail, no ifname:[$ifname] to enable airodump-ng."
		return
	}

	PID_Airo=$(ps -w | grep airodump-ng | grep $ifname | awk '{print $1}')

	if [ -z "$PID_Airo" ]; then
		ap_sta_name="_ap_sta_list"
		ap_sta_file=$BackgroundScanFilePath$ifname$ap_sta_name
		rm $ap_sta_file* -rf
		if [ "$apsteer" == "1" ]; then
			screen -d -m airodump-ng $ifname -w $ap_sta_file --output-format csv -s 1 -G -B $ageOutTime -L $ap_list_filename -l $sta_list_filename
		else
			screen -d -m airodump-ng $ifname -w $ap_sta_file --output-format csv -s 1 -B $ageOutTime -L $ap_list_filename -l $sta_list_filename
		fi
	fi
}

airodumpEnable() {

	vif=$1	
	
	ifname=$(uci get wireless.$vif.ifname)

        if [ -z "$ifname" ]; then {

                DebugPrint "Background scanning - airodump-ng fail, no ifname:[$ifname] to enable airodump-ng."
                return

	} else {
		check_wifi2=$(/usr/sbin/foreach wireless wifi-device disabled 0| grep wifi2)
		wifi2_mode=$(uci get wireless.wifi2.opmode)
		ap_sta_name="_ap_sta_list"
		ap_sta_file=$BackgroundScanFilePath$ifname$ap_sta_name

		# avoid tri-band processed too long to cause airodump failed/
		[ -n "$check_wifi2" -a "$wifi2_mode" == "ap" ] && sleep 5
		airodump_recovery=1
		if [ "$apsteer" == "1" ]; then
			screen -d -m airodump-ng $ifname -w $ap_sta_file --output-format csv -s 1 -G -B $ageOutTime -L $ap_list_filename -l $sta_list_filename
		else
	                screen -d -m airodump-ng $ifname -w $ap_sta_file --output-format csv -s 1 -B $ageOutTime -L $ap_list_filename -l $sta_list_filename
		fi
	}
	fi
}

saveDisplayChan() {
	local wifi_dev=$1
	local ifname=$2
	local channel=$3

	iwlist $ifname displayscan | awk -F"\t" '$5 == "'${channel}'" {print $0}' > /tmp/Background_Scanning/${wifi_dev}_displayscan_normal_${channel}
	iwlist $ifname displayscan advance | tail -n +3 > /tmp/Background_Scanning/${wifi_dev}_displayscan_advance_${channel}

	[ -e "${displayScanResult}.tmp" ] && rm -f "${displayScanResult}.tmp"
	[ -e "${displayScanAdvanceResult}.tmp" ] && rm -f "${displayScanAdvanceResult}.tmp"

	for i in $(ls ${BackgroundScanFilePath}${wifi_dev}_displayscan_normal_*); do
		[ -e "$i" ] && cat "$i" >> "$displayScanResult.tmp"
	done

	for i in $(ls ${BackgroundScanFilePath}${wifi_dev}_displayscan_advance_*); do
		[ -e "$i" ] && cat "$i" >> "$displayScanAdvanceResult.tmp"
	done

	cp "${displayScanResult}.tmp" "$displayScanResult"
	awk '!seen[$0]++' "${displayScanAdvanceResult}.tmp" > "$displayScanAdvanceResult" # remove duplicate rows
	rm "${displayScanResult}.tmp"
	rm "${displayScanAdvanceResult}.tmp"
}

updateChanUtil() {	#used for background scan swithed channel

	local wifi_dev=$1
	local ifname=$2
	local channel=$3

	if [ -z $channel ]; then
                return
        fi

	chanUtilList="$BackgroundScanFilePath""$wifi_dev""_chanUtil.txt"
	chanUtilList_tmp="$BackgroundScanFilePath""$wifi_dev""_chanUtil_tmp"

	if [ "$default_cfg80211" == "1" ]; then
		chutil=$(cfg80211tool $ifname get_sn_acsutil | awk -F ':' '{print $2}')
	else
		chutil=$(iwpriv $ifname get_sn_acsutil | awk -F ':' '{print $2}')
	fi
	rm $chanUtilList_tmp -f
	[ -e "$chanUtilList" ] && cp -f $chanUtilList $chanUtilList_tmp
	[ -e "$chanUtilList" ] && sed -i '/'"channel:$channel utilization"'/d' $chanUtilList_tmp

	DebugPrint "updateApChanUtil channel:$channel utilization:$chutil"
	echo channel:$channel utilization:$chutil >> $chanUtilList_tmp
	cp -f $chanUtilList_tmp $chanUtilList
}

updateApChanUtil() {	#used for primary channel
	
	local wifi_dev=$1
	local apChannel=$2

	if [ ! -e "/sbin/getWifiFirstIF" ]; then 
		DebugPrint "no /sbin/getWifiFirstIF, cannot update channel utilization on AP channel"
		return
	fi

	if [ $wifi_dev == "wifi0" ]; then                                                                  
                apIfName=$(/sbin/getWifiFirstIF 0)                
        elif [ $wifi_dev == "wifi1" ]; then
                apIfName=$(/sbin/getWifiFirstIF 1)               
        else                                                                          
                DebugPrint "wifi dev $wifi_dev not support"
                return                                      
        fi

	chanUtilList="$BackgroundScanFilePath""$wifi_dev""_chanUtil.txt"
        chanUtilList_tmp="$BackgroundScanFilePath""$wifi_dev""_chanUtil_tmp"

	if [ "$default_cfg80211" == "1" ]; then
                chutil=$(cfg80211tool $apIfName get_chutil | awk -F ':' '{print $2}')
        else
                chutil=$(iwpriv $apIfName get_chutil | awk -F ':' '{print $2}')
        fi
	
	rm $chanUtilList_tmp -f
        [ -e "$chanUtilList" ] && cp -f $chanUtilList $chanUtilList_tmp
        [ -e "$chanUtilList" ] && sed -i '/'"channel:$apChannel utilization"'/d' $chanUtilList_tmp

	DebugPrint "updateApChanUtil channel:$apChannel utilization=$chutil"
        echo channel:$apChannel utilization=$chutil >> $chanUtilList_tmp
        cp -f $chanUtilList_tmp $chanUtilList
}

switchChannel() {
	
	wifi_dev=$1
	vif=$2
	restart=$3

	count=0
	list_index=0	#0 for apsteer channel list, 1 for full channel bgscan
	chIndex=1
	chIndex_2=1
	defaultswitchChanInterval=20
	mindwell_limit=51
	maxdwell_limit=70
	defaultmindwell=51
	defaultmaxdwell=100
	defaultresttime=1
	defaultmaxscantime=200
	defaultidletime=51
	apStaUpdateCount=0
	apStaUpdateLimit=5

	ifname=$(uci get wireless.$vif.ifname)
	switchChanInterval=$(uci get wireless.$wifi_dev.switchChanInterval)
	mindwell=$(uci get wireless.$wifi_dev.mindwell)
	maxdwell=$(uci get wireless.$wifi_dev.maxdwell)
	resttime=$(uci get wireless.$wifi_dev.resttime)
	maxscantime=$(uci get wireless.$wifi_dev.maxscantime)
	idletime=$(uci get wireless.$wifi_dev.idletime)

	[ -z "$switchChanInterval" ] && {
		switchChanInterval=$defaultswitchChanInterval
		DebugPrint "Background scanning, no switchChanInterval, use default:[$defaultswitchChanInterval] sec."
	}

        [ -z "$mindwell" ] && {
                mindwell=$defaultmindwell
                DebugPrint "Background scanning, no mindwell, use default:[$defaultmindwell] ms."
        }

        [ -z "$maxdwell" ] && {
                maxdwell=$defaultmaxdwell
                DebugPrint "Background scanning, no maxdwell, use default:[$defaultmaxdwell] ms."
        }

        [ -z "$resttime" ] && {
                resttime=$defaultresttime
                DebugPrint "Background scanning, no resttime, use default:[$defaultresttime] ms."
        }

        [ -z "$maxscantime" ] && {
                maxscantime=$defaultmaxscantime
                DebugPrint "Background scanning, no maxscantime, use default:[$defaultmaxscantime] ms."
        }

        [ -z "$idletime" ] && {
                idletime=$defaultidletime
                DebugPrint "Background scanning, no idletime, use default:[$defaultidletime] ms."
        }

	if [ -z "$ifname" ]; then {

		DebugPrint "Background scanning - switch channel fail, no ifname:[$ifname] to get channel list."
		return

	} else {
		ChannelList=$BGscanChannelList

		iwlist $ifname channel | grep "Channel" | awk -F" " '{ print $2 }' > $ChannelList

		[ "$restart" == 1 ] && {
			sleep 10
			#Not to do interface down up since it makes airodump shut down
			#ifconfig $ifname down up #avoid auto channel to cause airodump failed.
		}

		setChannel=$(iwlist $ifname channel | grep Current | awk -F " " '{print $2}')   #the primary channel that always works on

		if [ "$apsteer" == "1" -o "$apsteer_boost_scan" == "1" ]; then
			ChannelList=$ApsteerChannelList
			echo $setChannel > $ChannelList
			apStaUpdateLimit=1
		fi

		while [ 1 ]                                                       
		do                                                                                                               
			[ -f "$ChannelList" ] || {
				DebugPrint "Background scanning - switch channel fail, channel list file is not exist."
				return
			}

			[ "$airodump_recovery" == "1" ] && check_airodump $vif

			if [ "$apsteer_boost_scan" == "1" ]; then
				channelNum=$(sed -n '$=' "$ChannelList")
				channelNum_2=$(sed -n '$=' "$BGscanChannelList")
				list_index=$(( $count % 2 ))
				if [ "$list_index" == "1" ]; then	#0 for apsteer channel list, 1 for full channel bgscan
					chIndex=$(( ($count / 2 ) % $channelNum_2  + 1 ))
					channel_2=$(cat $BGscanChannelList | head -n $chIndex | tail -n 1 | sed s'/^0//')

					if [ "$channel_2" == "$setChannel" -o "$channel_2" == "$channel" ]; then {      #if the scan channel is the primary channel
						count=$(( $count + 1 ))
						continue
					}
					fi
					channel=$channel_2
				else
					if [ "$channelNum" == "1" ]; then
						count=$(( $count + 1 ))
						continue
					fi
					chIndex=$(( ($count / 2 ) % $channelNum + 2 )) #+2 since the first channeel always be the primary channel, we skip it
		                        channel_1=$(cat $ChannelList | head -n $chIndex | tail -n 1 | sed s'/^0//')       #the channel to scan

					if [ "$channel_1" == "$setChannel" -o "$channel_1" == "$channel" ]; then {      #if the scan channel is the primary channel
						count=$(( $count + 1 ))
						continue
					}
					fi
					channel=$channel_1
				fi
			else
				channelNum=$(sed -n '$=' "$ChannelList")
				chIndex=$(( ($count % $channelNum ) + 1 ))
				channel=$(cat $ChannelList | head -n $chIndex | tail -n 1 | sed s'/^0//')       #the channel to scan

				if [ "$channel" == "$setChannel" ]; then {	#if the scan channel is the primary channel
					count=$(( $count + 1 ))
					[ $channelNum == "1" ] && sleep $switchChanInterval
					#processApStaList $vif
					continue
				}
				fi
			fi

			switchChanInterval=$(uci -q get wireless.$$wifi_dev.switchChanInterval)
			[ -n "$switchChanInterval" ] || switchChanInterval=$defaultswitchChanInterval

			if [ "$apsteer_boost_scan" == "1" ]; then
				switchChanInterval=$(( $switchChanInterval / 2 ))
			fi

			mindwell=$(uci -q get wireless.$wifi_dev.mindwell)
			[ -n "$mindwell" ] || mindwell=$defaultmindwell
			maxdwell=$(uci -q get wireless.$wifi_dev.maxdwell)
			[ -n "$mindwell" ] || maxdwell=$defaultmaxdwell

			disableSwitchChan=$(uci -q get wireless.$wifi_dev.disableSwitchChan)
			if [ "$disableSwitchChan" == "1" ]; then
				channel=$setChannel
			fi

			echo "$channel" > $BackgroundScanFilePath$vif"_next_channel"
			sleep $switchChanInterval
			updateApChanUtil $wifi_dev $setChannel


			#Set mindwell and maxdwell limit that prevent ap list is empty or not enough to display
			if [ $mindwell -lt $mindwell_limit ]; then
				mindwell=$mindwell_limit
			fi
			if [ $maxdwell -le $maxdwell_limit ]; then
				maxdwell=$maxdwell_limit
			fi

			[ "$channel" != "$setChannel" ] && exttool --scan --interface $wifi_dev --mindwell $mindwell --maxdwell $maxdwell --resttime $resttime \
				--maxscantime $maxscantime --idletime $idletime --scanmode 1 --chcount 1 $channel
			[ "$channel" != "$setChannel" ] && updateChanUtil $wifi_dev $ifname $channel
			if [ "$(uci -q get wifiprofile.snWifiConf.SUPPORT_CAPWAP_IWLIST_SCAN)" == "1" ]; then
				# add sleep to fix get info incomplete.
				sleep 5
				saveDisplayChan $wifi_dev $ifname $channel
				saveDisplayChan $wifi_dev $ifname $setChannel
			fi

			count=$(( $count + 1 ))

			apStaUpdateCount=$(( $apStaUpdateCount + 1))

			if [ $apStaUpdateCount -ge $apStaUpdateLimit -a "$requireProcessList" == "1" ]; then {
				ageOutTime=$(( $channelNum * $switchChanInterval ))
				if [ "$apsteer_boost_scan" == "1" ]; then
					ageOutTime=$(( ($channelNum_2+$channelNum) * $switchChanInterval * 2 ))
				fi
				[ $ageOutTime -lt 120 ] && ageOutTime=120
				#processApStaList $vif
				apStaUpdateCount=0;
			}
			fi	
		done
	}
	fi
}

[ -z "$device" ] && {                                                          
        DebugPrint "Background scanning fail, the device:[$device] is NULL."
	return
}                                                 
                                                                  
[ -z "$vif" ] && {                                                
	DebugPrint "Background scanning fail, the vif:[$vif] is NULL."
	return
}

[ -z "$haveScanRadio" ] && {
        DebugPrint "Background scanning fail, the haveScanRadio:[$haveScanRadio] is NULL."
        return
}

[ -d "$BackgroundScanFilePath" ] || mkdir $BackgroundScanFilePath

[ "$val" == "1" -o "$val" == "3" -o "$val" == "4" -o "$val" == "5" ] && requireProcessList=1

case "$val" in
	1) # backgroundscan
		[ "$haveScanRadio" == "0" ] && {
			airodumpEnable $vif
			switchChannel $device $vif 1
		}
		;;
	2) # fastroamingOnly
		switchChannel $device $vif 0
		;;
	3) # backgroundscan_fastroaming
		[ "$haveScanRadio" == "0" ] && {
			airodumpEnable $vif
		}
		switchChannel $device $vif 1
		;;
	4) # apsteering only, apsteer priority is higher than fastroaming, this case no matter fastroaming enable or not
		[ "$haveScanRadio" == "0" ] && {
			apsteer=1
			airodumpEnable $vif
			switchChannel $device $vif 1
		}
		;;
	5) # backgroundscan_apsteer, the channel list will be all channel but the apsteer channel will scan more frequently
		[ "$haveScanRadio" == "0" ] && {
			apsteer_boost_scan=1
			apsteer=1
			airodumpEnable $vif
			switchChannel $device $vif 1
		}
		;;
	*)
		DebugPrint "Background scanning, no background scanning mode to do."
		return
		;;
esac

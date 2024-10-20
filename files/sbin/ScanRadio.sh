DebugEnable=0
scanRadioFolder="/tmp/ScanRadio/"
DefaultVif="wifi2_scan"
DefaultAgeOutTime="120"	#time in second for ap sta list entry age out
vif=$1
intervalUnit="ms"	#changes the using of this script, the arg2 unit is ms
interval=$2
device=$3
if [ "$intervalUnit" == "ms" ]; then
DefaultInterval=300
else
DefaultInterval=3
fi
count=1
chIndex=1
athssdEnable=0	#part of spectrum, it takes almost 20 sec to process, disable it now
apStaUpdateInterval=10	#in sec
apStaUpdateCount=0
apStaFilter="/usr/sbin/scanRadio"
preUtilSleepMs=30	#due to the offload counter update every 20 msec, we should wait more than 20 after changed channel to get pure channel counters
ageOutTime=120 #time in second for ap sta list entry age out

DebugPrint(){
        [ "$DebugEnable" == "1" ] && echo "$@" > /dev/console
}

MSLEEP(){
	sleepms=$(( $1 * 1000))
	usleep $sleepms
}

AIRODUMP_AP_STA_LIST_FILENAME="/tmp/ScanRadio/ap_sta_list_scan0-01.csv"
AIRODUMP_AP_STA_LIST_FILENAME2="/tmp/ScanRadio/ap_sta_list_scan_2-01.csv"
TMP_AP_LIST_FILENAME="/tmp/ScanRadio/ap_list.txt.tmp"
TMP_STA_LIST_FILENAME="/tmp/ScanRadio/sta_list.txt.tmp"
AP_LIST_FILENAME="/tmp/ScanRadio/ap_list.txt"
AP_LIST_FILENAME_="/tmp/ScanRadio/"$device"_ap_list.txt"
STA_LIST_FILENAME="/tmp/ScanRadio/sta_list.txt"
STA_LIST_FILENAME_="/tmp/ScanRadio/"$device"_sta_list.txt"

processApStaList(){

	ifname=$1
    
	ap_sta_file=$scanRadioFolder"ap_sta_list_"$ifname"-01.csv"
	#echo "==ap_sta_file:$ap_sta_file======" >/dev/console
	if [ -f "$apStaFilter" ]; then
		if [ "$ifname" == "scan_2" ]; then
			$apStaFilter -i $AIRODUMP_AP_STA_LIST_FILENAME2
		elif [ "$ifname" == "scan_0" ] || [ "$ifname" == "scan_1" ]; then
			$apStaFilter -i $ap_sta_file -a $AP_LIST_FILENAME_ -s $STA_LIST_FILENAME_ -e $ageOutTime
		else
			$apStaFilter -i $AIRODUMP_AP_STA_LIST_FILENAME
		fi
	else
	#separate AP_STA_LIST to ap list and sta list	
		cat $AIRODUMP_AP_STA_LIST_FILENAME  | tr -d '\r' | tail -n +3 | sed '/Station MAC/,$d' > $TMP_AP_LIST_FILENAME
		cat $AIRODUMP_AP_STA_LIST_FILENAME | tr -d '\r' | sed -n '/Station/,$p'| tail -n +2 > $TMP_STA_LIST_FILENAME

	#filter entry with channel/rate -1
		cat $TMP_AP_LIST_FILENAME | awk -F ',' '($4 != -1 && $5 != -1){print $0}' > $TMP_AP_LIST_FILENAME
		cat $TMP_STA_LIST_FILENAME | awk -F ',' '($4 != -1){print $0}' > $TMP_STA_LIST_FILENAME

		filterApListByTime
		filterStaListByTime

		mv $TMP_AP_LIST_FILENAME $AP_LIST_FILENAME
		mv $TMP_STA_LIST_FILENAME $STA_LIST_FILENAME
	fi
}

filterApListByTime(){

	lineNum=$(sed -n '$=' $TMP_AP_LIST_FILENAME)

	WORKING_FILE="/tmp/ScanRadio/working_list.tmp"

	rm -f $WORKING_FILE

	nowTimeStamp=$(date +%s)
	ageOut=$(uci -q get wireless.$vif.ageOutTime)
	[ -n "$ageOut" ] || ageOut=$DefaultAgeOutTime

	ageOutTimeStamp=$(( $nowTimeStamp - $ageOut))

	for i in `seq 1 $lineNum`; do
		
		time=$(awk -v line=$i -F '[ ,]+' 'NR == line { print $5; exit }' $TMP_AP_LIST_FILENAME)
		timeStamp=$(date +%s -d $time)

		if [ $timeStamp -ge $ageOutTimeStamp ]; then
			head -n $i $TMP_AP_LIST_FILENAME | tail -n 1 >> $WORKING_FILE
		fi
	done
	mv $WORKING_FILE $TMP_AP_LIST_FILENAME
}

filterStaListByTime(){

	lineNum=$(sed -n '$=' $TMP_STA_LIST_FILENAME)

	WORKING_FILE="/tmp/ScanRadio/working_list.tmp"

	rm -f $WORKING_FILE

	nowTimeStamp=$(date +%s)
	ageOut=$(uci -q get wireless.$vif.ageOutTime)
	[ -n "$ageOut" ] || ageOut=$DefaultAgeOutTime

	ageOutTimeStamp=$(( $nowTimeStamp - $ageOut))
        
	for i in `seq 1 $lineNum`; do
                
		time=$(awk -v line=$i -F '[ ,]+' 'NR == line { print $5; exit }' $TMP_STA_LIST_FILENAME)
		timeStamp=$(date +%s -d $time)

		if [ $nowTimeStamp -ge $ageOutTimeStamp ]; then
			head -n $i $TMP_STA_LIST_FILENAME | tail -n 1 >> $WORKING_FILE
		fi
	done
	mv $WORKING_FILE $TMP_STA_LIST_FILENAME
}

check_airodump() {
	vif=$1
	ifname=$(uci -q get wireless.$vif.ifname)

	[ -z "$ifname" ] && {
		DebugPrint "Scan Radio - airodump-ng fail, no ifname:[$ifname] to enable airodump-ng."
		return
	}

	PID_Airo=$(ps -w | grep airodump-ng | grep $ifname | awk '{print $1}')

	if [ -z "$PID_Airo" ]; then
		ap_sta_file_prefix="ap_sta_list_"
		ap_sta_list="$scanRadioFolder""$ap_sta_file_prefix""$ifname"
		rm $ap_sta_list* -rf
		screen -d -m airodump-ng $ifname -w $ap_sta_list -K 1 --output-format csv -s 1 -G -B $ageOutTime -L $AP_LIST_FILENAME_ -l $STA_LIST_FILENAME_
	fi
}

startChanUtil(){	#trigger get_sn_chutil to update cycle_count and rx_clear_count for new channel statistic
	MSLEEP $preUtilSleepMs
	mainSleepInterval=$(( $interval - $preUtilSleepMs ))
	ifname=$1
	iwpriv $ifname get_sn_chutil > /dev/null
}

saveChanUtil(){

	ifname=$1
	channel=$2
	value=$3

	chUtilList_name="chanUtil.txt"
	chUtilList_name_tmp=$chUtilList_name"_tmp"
	chUtilList=$scanRadioFolder$chUtilList_name
	chUtilList_tmp=$scanRadioFolder$chUtilList_name_tmp

	if [ -z $value ] || [ "$value" == "0" ]; then
		chUtil=$(iwpriv $ifname get_sn_chutil | awk -F ":" '{print $2}')
		DebugPrint "iwpriv $ifname get_sn_chutil $chUtil"
	else
		chUtil=$value
		DebugPrint "set chUtil to value $value"
	fi

	if [ "$chUtil" != "255" ]; then 
		[ -e "$chUtilList" ] && cp $chUtilList $chUtilList_tmp		#write in tmp file for synchnization issue
		[ -e "$chUtilList" ] && sed -i '/'channel:$channel'/d' $chUtilList_tmp
		echo channel:$channel	utilization:$chUtil >> $chUtilList_tmp
		DebugPrint "channel:$channel   utilization:$chUtil"
		mv $chUtilList_tmp $chUtilList
	fi
}

chkSpectrumTools(){

	chkSpectraltool=$(type spectraltool | grep is)
	if [ -z "$chkSpectraltool" ]; then
                DebugPrint "no spectraltool, please check your package setting"
		echo 0
		return
	fi
	
	[ "$athssdEnable" == "1" ] && {
		chkAthssd=$(type athssd | grep is)
	        if [ -z "$chkAthssd" ]; then
        	        DebugPrint "no athssd, please check your package setting"
        	        echo 0
        	        return		
        	fi
	}
	echo 1
}

SpectrumAlone(){
	wifi_dev=$1
	channel=$2
	spectrum_default="outFile"
	spectrum_file_name="spectrum_channel_"$channel".txt"
	spectrum_file=$scanRadioFolder$spectrum_file_name
	output=$scanRadioFolder$spectrum_default
	number=$(uci get wireless.$wifi_dev.samples_number)
	spectrum_cmd=""

	mkdir -p $scanRadioFolder
	spectraltool -i $wifi_dev startscan
	spectraltool -i $wifi_dev pwr_format 1
	spectraltool -i $wifi_dev fft_size 5

	[ -n "$number" ] && spectrum_cmd=$spectrum_cmd" get_samples $number" || spectrum_cmd=$spectrum_cmd" get_samples 25"
	[ -n "output" ] && spectrum_cmd=$spectrum_cmd" -o $output"
	spectraltool -i $wifi_dev $spectrum_cmd
	[ -s "$output" ] && cp -rf $output $spectrum_file

	spectraltool -i $wifi_dev stopscan
}

StartSpectrum(){	#start spectrum scan
	vif=$1
	channel=$2
	wifi_dev=$(uci -q get wireless.$vif.device)
	ifname=$(uci -q get wireless.$vif.ifname)

	DebugPrint "StartSpectrum, spectraltool -i $wifi_dev startscan" 
	spectraltool -i $wifi_dev startscan &

	spectraltool -i $wifi_dev pwr_format 1
	
	[ "$athssdEnable" == "1" ] && {
		if [ $channel -ge 36 ]; then #5G channels
			DebugPrint "athssd -i $wifi_dev -j $ifname -c 5 -a"
			athssd -i $wifi_dev -j $ifname -c 5 -a &
		else
			DebugPrint "athssd -i $wifi_dev -j $ifname -c 5"
			athssd -i $wifi_dev -j $ifname -c 5 &
		fi
	}
}

saveSpectrumRawData(){
	vif=$1
	wifi_dev=$(uci -q get wireless.$vif.device)

	DebugPrint "saveSpectrum, spectraltool -i $wifi_dev raw_data &"
        spectraltool -i $wifi_dev raw_data &
}

saveSpectrum(){
	vif=$1
	channel=$2

	spectrum_default_file="outFile"
	wifi_dev=$(uci -q get wireless.$vif.device)
	
        spectrum_file_name="spectrum_channel_"$channel".txt"
        spectrum_file=$scanRadioFolder$spectrum_file_name

	DebugPrint "sleep $spectrumDelay for spectraltool raw_data finide"
	sleep $spectrumDelay

	DebugPrint "saveSpectrum, spectraltool -i $wifi_dev stopscan &"
	spectraltool -i $wifi_dev stopscan &

	[ -s "$spectrum_default_file" ] && DebugPrint "mv $spectrum_default_file $spectrum_file" #use -s to check if the file size is not zero
	[ -s "$spectrum_default_file" ] && mv $spectrum_default_file $spectrum_file

	[ "$athssdEnable" == "1" ] && {
		test "$(pgrep -f athssd)" && DebugPrint "saveSpectrum, kill -9 $(pgrep -f athssd)"
		test "$(pgrep -f athssd)" && kill -9 $(pgrep -f athssd)		#kill athssd
	}
}

[ -z "$vif" ] && {
	vif=$DefaultVif
	DebugPrint "NO arg1, use default vif $vif"
}

[ -z "$interval" ] && {
	interval=$(uci -q get wireless.$vif.scanInterval)
	if [ -z "$interval" ]; then
		interval=$DefaultInterval
		DebugPrint "NO arg2, no wireless.$vif.scanInterval, use default interval $interval"
	else
		DebugPrint "NO arg2, use wireless.$vif.scanInterval $interval"
	fi
}

ifname=$(uci -q get wireless.$vif.ifname)

wifi_dev=$(uci get wireless.$vif.device)
[ -z "$ifname" ] && {
	DebugPrint "NO interface, terminate script. Please check your wireless.$vif.ifname"
	return
}

spectrumMode=$(uci get wireless.$wifi_dev.spectrumMode)
chkIfUp=$(iwconfig $ifname | grep Monitor)

[ -z "$chkIfUp" -a "$spectrumMode" != "1" ] && {
	DebugPrint "$ifname did not up, terminate script. Please check your monitor mode function in qcawifi.sh"
	return
}

channelListName="channelList_"$ifname

channelList=$scanRadioFolder$channelListName

if [ ! -d $scanRadioFolder ]; then
	mkdir -p $scanRadioFolder
fi 

scanChan=$(uci -q get wireless.$vif.scanChannel)

if [ -n "$scanChan" ]; then	#use user defined channel list
	chan=0
	chCount=1
	while [ ! -z "$chan" ]; do
		chan=$(uci -q get wireless.$vif.scanChannel | awk '{print $'$chCount'}')
		[ -n "$chan" ] && echo "$chan" >> $channelList
		chCount=$(($chCount + 1))        
	done
else
	scanchan_mode=$(uci -q get wireless.$vif.scanchan_mode) # 0:both 2.4G and 5G, 1:only 2.4G, 2:only 5G
	#use radio suppoted channel list
	if [ "$scanchan_mode" == "0" ]; then
		iwlist $ifname channel | grep "Channel" | awk -F" " '{ print $2 }' > $channelList

		chan_2G=$(cat $channelList | awk -F" " '{if ($1 >= 1 && $1 <= 14) print $1 }')
		chan_5G=$(cat $channelList | awk -F" " '{if ($1 >= 36) print $1 }')
		[ -z "$chan_2G" ] && echo "Scan Radio Warning: The hardware does not support 2.4G, but scanchan_mode selects to scan both 2.4G and 5G radio." > /dev/console
		[ -z "$chan_5G" ] && echo "Scan Radio Warning: The hardware does not support 5G, but scanchan_mode selects to scan both 2.4G and 5G radio." > /dev/console
	elif [ "$scanchan_mode" == "1" ]; then
		iwlist $ifname channel | grep "Channel" | awk -F" " '{if ($2 >= 1 && $2 <= 14) print $2 }' > $channelList
	elif [ "$scanchan_mode" == "2" ]; then
		iwlist $ifname channel | grep "Channel" | awk -F" " '{if ($2 >= 36) print $2 }' > $channelList
	else
		iwlist $ifname channel | grep "Channel" | awk -F" " '{ print $2 }' > $channelList
	fi

	chanStr=$(cat $channelList)

	[ -z "$chanStr" ] && {
		echo "Scan Radio ERROR: The channel list has no channel to switch." > /dev/console
		return
	}
fi
channelNum=$(sed -n '$=' "$channelList")

updateInterval=$(uci -q get wireless.$vif.updateApStaInterval)

[ -n $updateInterval -a $updateInterval -ge 1 ] && apStaUpdateInterval=$updateInterval

[ "$intervalUnit" == "ms" ] && apStaUpdateInterval=$(( $apStaUpdateInterval * 1000))

apStaUpdateLimit=$(( $apStaUpdateInterval / interval ))

spctrumTool=$(chkSpectrumTools)

# avoid the interface not yet enabled before scan spectrum, causing vap cannot work.
[ "$spectrumMode" == "1" ] && sleep 5

while :
do
	spectrumMode=$(uci get wireless.$wifi_dev.spectrumMode)
	scanProgress=$(uci -q get wireless.$vif.scanProgress)	#0 STOP, 1 SCAN, 2 PAUSE

	if [ "$spectrumMode" != "1" ]; then
		if [ -z "$scanProgress" ]; then
			DebugPrint "no scanProgress option in wireless.$vif, terminate scanRadio script"
			break;
		elif [ "$scanProgress" == "2" ]; then
			DebugPrint "scanProgress=$scanProgress, PAUSE, sleep $interval"
			sleep $interval
			continue;
		elif [ "$scanProgress" == "0" ]; then
			DebugPrint "scanProgress=$scanProgress, Force terminate scanRadio script"
			break;
		fi

		chanUtilEnable=$(uci -q get wireless.$vif.chanUtilEnable)
		spectrumEnable=$(uci -q get wireless.$vif.spectrumEnable)

		channel=$(cat $channelList | head -n $chIndex | tail -n 1)

		DebugPrint "set $ifname channel to $channel, chIndex=$chIndex count=$count"

#check airodump if exist
		check_airodump $vif

#switch channel
		[ "$channel" != "$setChannel" ] && iwconfig $ifname channel $channel

#sleep scan interval
		DebugPrint "Sleep $interval"

		mainSleepInterval=$interval

		setChannel=$channel

#start scan here
		[ "$chanUtilEnable" == "1" ] && sumUtil=0
		[ "$spectrumEnable" == "1" ] && [ "$spctrumTool" == "1" ] && StartSpectrum "$vif" "$channel"
		[ "$spectrumEnable" == "0" ] && spectrumDelay=0

		[ "$chanUtilEnable" == "1" ] && startChanUtil $ifname

		if [ "$intervalUnit" != "ms" ]; then
			sleepCount=1
			sleepTime=$(( $interval - $spectrumDelay ))
		fi
#end

#process ap_sta_list, entry filter
		apStaUpdateCount=$(( $apStaUpdateCount + 1))
		if [ $apStaUpdateCount -ge $apStaUpdateLimit ]; then
			#processApStaList $ifname
			apStaUpdateCount=0;
		fi

		if [ "$intervalUnit" == "ms" ]; then
			MSLEEP $mainSleepInterval
		else
			while [ $sleepCount -le $sleepTime ]; do	#sleep 1 sec in every loop

				[ "$chanUtilEnable" == "1" ] && {
					util=$(iwpriv $ifname get_chutil | awk -F ":" '{print $2}')
					sumUtil=$(( $sumUtil + $util ))
					DebugPrint "sleepCount=$sleepCount, util=$util, sumUtil=$sumUtil"
				}
		
				sleep 1
				sleepCount=$(( $sleepCount + 1 ))
		
		#do spectraltool raw_data at the n-1 sec, reserve the last one sec for preparing data
		[ $sleepCount -eq $sleepTime ] && [ "$spectrumEnable" == "1" ] && [ "$spctrumTool" == "1" ] && saveSpectrumRawData "$vif"
			done
		fi
#end sleep

#end of routine on this channel
		[ "$chanUtilEnable" == "1" ] && {
			if [ "$intervalUnit" == "ms" ]; then
				saveChanUtil $ifname $channel
			else
				avgUtil=0
				[ $sumUtil -gt 0 ] && avgUtil=$(( $sumUtil / $sleepTime ))  #not to use sleepCount, use sleepTime because the while loop condition is -le
				saveChanUtil $ifname $channel $avgUtil
			fi
		}
	
		[ "$spectrumEnable" == "1" ] && [ "$spctrumTool" == "1" ] && saveSpectrum "$vif" "$channel"
#end
	else
		channelNum=$(sed -n '$=' "$channelList")
		chIndex=$(( ($count % $channelNum ) + 1 ))
		channel=$(cat $channelList | head -n $chIndex | tail -n 1)
		[ "$channel" != "$setChannel" ] && iwconfig $ifname channel $channel
		setChannel=$channel
		SpectrumAlone "$wifi_dev" "$channel"
	fi

	count=$(( $count + 1 ))
	chIndex=$(( ($count % $channelNum ) + 1 ))
done 

DebugPrint "End of ScanRadio.sh"

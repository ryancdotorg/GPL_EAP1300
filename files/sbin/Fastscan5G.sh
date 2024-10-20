#/bin/bash
scan_duration=$1
triband_status=$2
counter=0
startFastscan=0


if [ -z "$triband_status" ]; then
	triband_status=1	#for old qca_wifi.sh without argument 2
fi

sleep 10 #avoid to do scan with 2.4G at the same time

wait_for_wifiDevDisabled()
{
	if [ "$triband_status" == "1" ] || [ "$triband_status" == "3" ]; then
		athw=$(ifconfig 2>/dev/null|grep wifi1)
		vapDev=$(sh /sbin/getWifiFirstIF 1)
		if [ -n "$vapDev" ]; then
	        	while [ -z "$athw" ]
			do
				sleep 20
				athw=$(ifconfig 2>/dev/null|grep wifi1)
			done
		fi
	fi

        if [ "$triband_status" == "2" ] || [ "$triband_status" == "3" ]; then
                athw=$(ifconfig 2>/dev/null|grep wifi2)
		vapDev=$(sh /sbin/getWifiFirstIF 2)
		if [ -n "$vapDev" ]; then 
	                while [ -z "$athw" ]
	                do
	                        sleep 20
	                        athw=$(ifconfig 2>/dev/null|grep wifi1)
	                done
		fi
        fi	
}

wait_for_wifiDevUp()
{
	if [ "$triband_status" == "1" ] || [ "$triband_status" == "3" ]; then
		wifiDev=$(sh /sbin/getWifiFirstIF 1)
		if [ -n "$wifiDev" ]; then
			wifiMacAddr=$(iwconfig $wifiDev | awk '/Access Point/ {print $6}')
			wifi_scheduled_status=1
			while [ "$wifiMacAddr" = "Not-Associated" ]
			do
		        	counter=$(expr $counter + 1)
		        	sleep 5
					wifiDev=$(sh /sbin/getWifiFirstIF 1)
					if [ -z $wifiDev ]; then
						continue
					fi
	        		wifiMacAddr=$(iwconfig $wifiDev | awk '/Access Point/ {print $6}')
				wifi_schedule=$(uci get wifi_schedule.wireless.ScheduleEnable)
        			if [ $counter -gt 3 ]; then    #add protection for worst case any unknown cause makes wifi abnormal
					if [ $wifi_schedule -eq 1 ]; then
						if [ -f /tmp/schedule_$wifi_Dev ]; then		#wifi schedule makes wifi enabled but dev down, cannot bring up wifi dev now
							wifi_scheduled_status=$(cat /tmp/schedule_$wifi_Dev)
						fi
					fi
					if [ $wifi_scheduled_status -eq 1 ]; then
	        			        ifconfig $wifiDev down
        				        sleep 3
        				        ifconfig $wifiDev up
        				        counter=0
					fi
        			fi
			done
		fi
	fi
        if [ "$triband_status" == "2" ] || [ "$triband_status" == "3" ]; then
                wifiDev=$(sh /sbin/getWifiFirstIF 2)
		if [ -n "$wifiDev" ]; then
	                wifiMacAddr=$(iwconfig $wifiDev | awk '/Access Point/ {print $6}')
	                wifi_scheduled_status=1
	                while [ "$wifiMacAddr" = "Not-Associated" ]
	                do
	                        counter=$(expr $counter + 1)
	                        sleep 5
	                                wifiDev=$(sh /sbin/getWifiFirstIF 2)
	                                if [ -z $wifiDev ]; then
	                                        continue
	                                fi
	                        wifiMacAddr=$(iwconfig $wifiDev | awk '/Access Point/ {print $6}')                              
	                        wifi_schedule=$(uci get wifi_schedule.wireless.ScheduleEnable)
	                        if [ $counter -gt 3 ]; then    #add protection for worst case any unknown cause makes wifi abnormal
	                                if [ $wifi_schedule -eq 1 ]; then
	                                        if [ -f /tmp/schedule_$wifi_Dev ]; then         #wifi schedule makes wifi enabled but dev down, cannot bring up wifi dev now
	                                                wifi_scheduled_status=$(cat /tmp/schedule_$wifi_Dev)
        	                                fi
        	                        fi
        	                        if [ $wifi_scheduled_status -eq 1 ]; then
        	                                ifconfig $wifiDev down
        	                                sleep 3
        	                                ifconfig $wifiDev up
        	                                counter=0
                	                fi
                	        fi
                	done
		fi
        fi
}

wait_for_startFastscan()
{	
	if [ -f /tmp/notify_fastscan_enable ]; then
		startFastscan=$(cat /tmp/notify_fastscan_enable)
	fi
	
	while [ $startFastscan -eq 0 ];
		do
		    	sleep 5
	        if [ -f /tmp/notify_fastscan_enable ]; then
        	 	startFastscan=$(cat /tmp/notify_fastscan_enable)
        	fi
    	done	
}

scan_counter=0

while :
do
	sleep $scan_duration

        wait_for_wifiDevDisabled

        wait_for_wifiDevUp

        wait_for_startFastscan

	if [ "$triband_status" == "1" ] || [ "$triband_status" == "3" ]; then
	        wifi1Dev=$(sh /sbin/getWifiFirstIF 1)
		if [ -n "$wifi1Dev" ]; then 
			wifi1ChNum=$(iwlist $wifi1Dev frequency | tail -n +2 | grep -c Channel)
		else
			wifi1ChNum=0
		fi
	else
		wifi1ChNum=0
	fi
        if [ "$triband_status" == "2" ] || [ "$triband_status" == "3" ]; then
                wifi2Dev=$(sh /sbin/getWifiFirstIF 2)
		if [ -n "$wifi2Dev" ]; then
			wifi2ChNum=$(iwlist $wifi2Dev frequency | tail -n +2 | grep -c Channel)
		else
			wifi2ChNum=0
		fi
	else
		wifi2ChNum=0
        fi
	totalChNum=$(expr $wifi1ChNum + $wifi2ChNum)
	if [ $totalChNum -eq 0 ]; then
		echo error, wifi1 channel number + wifi2 channel number=0 > /dev/console
		exit 0
	fi
	
	scan_counter=$(expr $scan_counter % $totalChNum)

	if [ $scan_counter -lt $wifi1ChNum ]; then
		wifiDev=$wifi1Dev
	elif [ $scan_counter -lt $(expr $wifi1ChNum + $wifi2ChNum) ]; then
		wifiDev=$wifi2Dev
	else
		echo error, scan_counter=$scan_counter, wifi1ChNum=$wifi1ChNum, wifi2ChNum=$wifi2ChNum > /dev/console
	fi
	
	if [ -z $wifiDev ]; then
		continue
	fi
        
	iwlist $wifiDev scanning active_quiet
	scan_counter=$(expr $scan_counter + 1)
done


#/bin/bash
scan_duration=$1
counter=0
startFastscan=0

wait_for_wifiDevDisabled()
{
	athw=$(ifconfig 2>/dev/null|grep wifi0)
    while [ -z "$athw" ]
	do
		sleep 20
		athw=$(ifconfig 2>/dev/null|grep wifi0)
	done
}

wait_for_wifiDevUp()
{
	wifiDev=$(sh /sbin/getWifiFirstIF 0)
	wifiMacAddr=$(iwconfig $wifiDev | awk '/Access Point/ {print $6}')
	wifi_scheduled_status=1
	while [ $wifiMacAddr = Not-Associated ]
	do
        	counter=$(expr $counter + 1)
        	sleep 5
		wifiDev=$(sh /sbin/getWifiFirstIF 0)
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

while :
do
        wait_for_wifiDevDisabled

        wait_for_wifiDevUp

        wait_for_startFastscan

        wifiDev=$(sh /sbin/getWifiFirstIF 0)
	if [ -z $wifiDev ]; then
		continue
	fi
        iwlist $wifiDev scanning active_quiet
        sleep $scan_duration
done


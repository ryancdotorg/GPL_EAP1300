#!/bin/sh

server_ip=$1
checkin_data="/var/log/ezmcloud/ezm_checkin_data"

if [ "$server_ip" != "" ]
then
	while true
	do
		if [ -f "$checkin_data" ]
		then
			new_file_name="/tmp/checkin_$(date '+%Y-%m-%d_%H-%M')_`setconfig -g 6 |tr -d ':\n '`"
			mv $checkin_data $new_file_name
			curl -F "file=@${new_file_name}" --url http://$server_ip/
			rm $new_file_name
		fi
		sleep 10
	done
else
	echo "no server ip" > /dev/console
fi

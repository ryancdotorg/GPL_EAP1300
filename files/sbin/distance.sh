#!/bin/sh
. /etc/functions.sh

for device in wifi0 wifi1;
do

	distance_env=$(uci get wireless."$device".distance_env)
	echo $distance_env > /proc/sys/dev/"$device"/distance_env
	             
	distance=$(uci get wireless."$device".distance)
	echo $distance > /proc/sys/dev/"$device"/distance
	
	
done

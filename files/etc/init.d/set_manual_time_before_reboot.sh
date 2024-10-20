#!/bin/sh /etc/rc.common
STOP=99
stop(){
	year=`date +"%Y"`
	month=`date +"%m"`
	day=`date +"%d"`
	hour=`date +"%H"`
	min=`date +"%M"`
	min2=$(($min+2))

	#echo $year $month $day $hour $min

	uci set systime.systime.year=$year
	uci set systime.systime.month=$month
	uci set systime.systime.day=$day
	uci set systime.systime.hour=$hour
	uci set systime.systime.minute=$min2
	uci commit
}

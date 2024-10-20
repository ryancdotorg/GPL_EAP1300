#!/bin/sh

case $1 in
	0)
		ath="$(ls /sys/class/net/ | grep -E "^ath0|^ath2|^enjet0|^mgmt0|^ath32" | head -1)"
		ath="${ath:-dummy0}"
	;;
	1)
		ath="$(ls /sys/class/net/ | grep -E "^ath1|^ath5|^enjet1|^mgmt1|^ath35" | head -1)"
		ath="${ath:-dummy1}"
	;;
	2)
		ath="$(ls /sys/class/net/ | grep -E "^ath4|^ath6|^ath37" | head -1)"
		ath="${ath:-dummy2}"
	;;
esac

chanc=$'\n'"$(wlanconfig ${ath} list chan)"
chanc=${chanc//- /-}
echo "$chanc"|awk '{FS="Channel"} {printf "Channel%s\n", $2}'|grep Mhz >> /tmp/${ath}_channel
echo "$chanc"|awk '{FS="Channel"} {printf "Channel%s\n", $3}'|grep Mhz >> /tmp/${ath}_channel 
first=0
while read line
do
        [ $first != 0 ] && printf ",\n" || first=1
#	echo ----
	channel_freq=${line% C *}
#	echo $channel_freq
	channel=${channel_freq%: *}
	channel_num=$( echo $channel | sed s/"Channel"/""/g | sed s/" "/""/g )
#	echo $channel_num
	freq=$( echo ${channel_freq#$channel: } | awk '{FS=" "} {printf "%s", $1}')
	freq=$(awk 'BEGIN{printf "%.3f\n",('$freq'/1000)}')
#	echo freq=$freq
	option=${line#* Mhz }
	ht20=""
	ht40_24g=""
	ht40_5g="0"
	v40="0"
	v80="0"
	for i in $option
	do
#		echo $i
		case "$i" in
			C)
				ht20="C"
			;;
			CU)
				ht40_24g="U"
				ht40_5g="1"
			;;
			CL)
				[ -n "$ht40_24g" ] && ht40_24g="${ht40_24g} L" || ht40_24g="L"
				ht40_5g="1"
			;;
			VU|VL)
				v40="1"
			;;
			V80*)
				v80="1"
			;;
			*)
			;;
		esac
	done
	case $1 in
		0)
			printf "{\"Channel\": \"Ch $channel_num : $freq GHz\", \"Value\": \"$channel_num\", \"ht20\":\"$ht20\", \"ht40\":\"$ht40_24g\"}"
		;;
		1|2)
			printf "{\"Channel\": \"Ch $channel_num : $freq GHz\", \"Value\": \"$channel_num\", \"ht40\":\"$ht40_5g\", \"v40\":\"$v40\", \"v80\":\"$v80\"}"
		;;
	esac
done < /tmp/${ath}_channel

rm /tmp/${ath}_channel

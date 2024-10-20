#!/bin/sh

default_cfg80211=0
funcDebug=0
aid=0

cfg80211tool() {
        [ "$funcDebug" == "1" ] && echo cfg80211tool "$@" > /dev/console
        /usr/sbin/cfg80211tool "$@"
}

iwpriv() {
        [ "$funcDebug" == "1" ] && echo iwpriv "$@" > /dev/console
        /usr/sbin/iwpriv "$@"
}

device_if() {
        if [ "$default_cfg80211" == "1" ]; then
                cfg80211tool "$@"
        else
                iwpriv "$@"
        fi
}

is_mac() {
    if [ "$1" != "" ]
    then
        for mac_str in $@
        do
            mac_chr=${mac_str//:/ }
            mac_chr=${mac_chr//-/ }
            mac_word=$(echo "$mac_chr" |wc -w || echo 0)
            if [ "$mac_chr" != "$mac_str" -a $mac_word -eq 6 ]
            then
                for mac_hex in $mac_chr
                do
                    mac_int=`printf %d 0x$mac_hex 2>/dev/null`
                    if [ $? -ne 0 ]
                    then
                        mac_int=-1
                    fi
                    if [ "${mac_int//[0-9]/}" = "" ]
                    then
                        if [ $mac_int -gt 255 -o $mac_int -lt 0 ]
                        then
                            echo 0
                            return
                        fi
                    else
                        echo 0
                        return
                    fi
                done
            else
                echo 0
                return
            fi
        done
        echo 1
    else
        echo 0
    fi
}

add_blacklist_and_kick() {
	sta_mac=$1
	ifname=$2
	
	device_if $ifname addmac_sec $sta_mac
        device_if $ifname kickmac $sta_mac
        aid=$(device_if $ifname get_snassocid | awk -F ':' '{print $2}')
        device_if $ifname kickmac $sta_mac
        device_if $ifname kickmac $sta_mac
        device_if $ifname kickmac $sta_mac
}

del_blacklist() {
	sta_mac=$1	
	ifname=$2

	device_if $ifname delmac_sec $sta_mac
}

preset_blacklist(){
	sta_mac=$1
	ifname=$2
	
	is_maccmd_sec_en=$(device_if $ifname get_maccmd_sec)
	is_maccmd_sec_en=${is_maccmd_sec_en#*:}
	is_maccmd_sec_en=${is_maccmd_sec_en%% *}
	
	if [ "$is_maccmd_sec_en" != "2" ]; then
	# enable acl for session/idle timeout use
		device_if $ifname maccmd_sec 2
	fi
}

postset_blacklist(){
	ifname=$1

	is_maccmd_sec_en=$(device_if $ifname get_maccmd_sec)
	is_maccmd_sec_en=${is_maccmd_sec_en#*:}
	is_maccmd_sec_en=${is_maccmd_sec_en%% *}

	if [ "$is_maccmd_sec_en" != "0" ]; then
	#reset the acl policy
		device_if $ifname maccmd_sec 0
	fi
}

reason=$1

if [ "$reason" != "1" -a "$reason" != "2" ]; then
	use_snlog=1
	mode=$2
	target=$3
	ifname=$4
	delay_sec=$5
else
	#old usage, without snlog
	use_snlog=0
	mode=$1 #1: kick one spcified station, 2: read station list to kick 3:cloud kick sta(kick button)
	target=$2
	ifname=$3
	delay_sec=$4
fi

[ "$reason" = "fasthandover" ] && delay_sec=5
[ "$reason" = "bandsteering" ] && delay_sec=5

if [ $mode == "1" -o $mode == "3" ]; then
	if [ -z $ifname ]; then
		echo ERROR! ifname is empty > /dev/console
		exit
	fi
	if [ "$(is_mac $target)" = "1" ]; then	
		preset_blacklist $target $ifname
		add_blacklist_and_kick $target $ifname

		if [ $use_snlog == "1" ] && [ ! -z $aid ] ;then
			if [ $mode == "3" ]; then
				snlogger "event.info 1.0" "ap_cloud_kick_station,reason='$reason', client_mac='$target', ifname='$ifname', aid='$aid'"
			elif [ "$reason" == "apsteer notification" ]; then
				#snlogger "event.info 1.0" "ap_clear_station,reason='$reason', client_mac='$target', ifname='$ifname', aid='$aid'"
				echo execute kicksta.sh without senaolog since it shows in apsteer > /dev/console
			else
				snlogger "event.info 1.0" "ap_kick_station,reason='$reason', client_mac='$target', ifname='$ifname', aid='$aid'"
			fi
		fi

		sleep ${delay_sec:-1}
		del_blacklist $target $ifname
		postset_blacklist $ifname
	fi

elif [ $mode == "2" ]; then

	if [ ! -e $target"_tmp" ]; then
		cp $target $target"_tmp"
	else
		echo ===$target"_tmp" exitsted, not to do so close. exit=== > /dev/console
		exit
	fi

	staNum=$(sed -n '$=' $target"_tmp")

	for i in `seq 1 $staNum`; do
		client=$(cat $target"_tmp" | head -n $i | tail -n 1 | awk '{print $1}')
		ifname=$(cat $target"_tmp" | head -n $i | tail -n 1 | awk '{print $2}')
		[ -z "$client" -o -z "$ifname" ] && continue
		[ "$(is_mac $client)" != "1" ] && continue
		preset_blacklist $client $ifname
		add_blacklist_and_kick $client $ifname
		[ $use_snlog == "1" ] && [ ! -z $aid ] && snlogger "event.info 1.0" "ap_kick_station,reason='$reason', client_mac='$client', ifname='$ifname', aid='$aid'"
		kicked=1
	done

	[ "$kicked" == "1" ] && sleep ${delay_sec:-1}

	for i in `seq 1 $staNum`; do
		client=$(cat $target"_tmp" | head -n $i | tail -n 1 | awk '{print $1}')
		ifname=$(cat $target"_tmp" | head -n $i | tail -n 1 | awk '{print $2}')
		[ -z "$client" -o -z "$ifname" ] && continue
		[ "$(is_mac $client)" != "1" ] && continue
		del_blacklist $client $ifname
		postset_blacklist $ifname
	done

	rm $target"_tmp" -f
else
	echo ERROR! not supported mode:$mode >> /dev/console
fi

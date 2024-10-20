#!/bin/sh

log="/tmp/log/auto_test"
debug_log=0
debug_log_file="/tmp/log/auto_test_debug.log"
pid=""

debug_log() {
        [ "$debug_log" == "1" ] && echo "\[$pid\] $@" >> "$debug_log_file"
}

luci_reload_log() {
    local uid="$1"
    local total_reload_duration=0
    
    [ "$uid" == "" ] && [ -f "$log/last_uid" ] && uid=$(cat "$log/last_uid")

    debug_log "UID=$uid"

    [ "$uid" == "" ] && {
	echo "-1"
	return
    }

    total_reload_log="$log/total_reload_log.$uid"

    [ -f "$total_reload_log" ] && {
    	echo "0"
    	return
    }

    check_option_log="/tmp/check_option.log.$uid"
    boost_reload_log="/tmp/boost_reload.log.$uid"
    luci_reload_log="/tmp/luci_reload.log.$uid"

    [ ! -f "$check_option_log" ] && [ ! -f "boost_reload_log" ]  && [ ! -f "$luci_reload_log" ] && {
    	echo "-1"
    	return
    }

    [ -f "$check_option_log" ] && {
	cat "$check_option_log" >> "$total_reload_log"
	check_option_duration=$(cat "$check_option_log" | grep "duration:" | cut -d':' -f 2)
	[ -n $check_option_duration ] && total_reload_duration=$((total_reload_duration+check_option_duration))
	rm -rf "$check_option_log"
    }
    debug_log "check_option_duration=$check_option_duration"

    [ -f "$boost_reload_log" ] && {
	cat "$boost_reload_log" >> "$total_reload_log"
	boost_reload_duration=$(cat "$boost_reload_log" | grep "duration:" | cut -d':' -f 2)
	[ -n $boost_reload_duration ] && total_reload_duration=$((total_reload_duration+boost_reload_duration))
	rm -rf "$boost_reload_log"
    }
    debug_log "boost_reload_duration=$boost_reload_duration"

    [ -f "$luci_reload_log" ] && {
	cat "$luci_reload_log" >> "$total_reload_log"
	luci_reload_duration=$(cat "$luci_reload_log" | grep "duration:" | cut -d':' -f 2)
	[ -n $luci_reload_duration ] && total_reload_duration=$((total_reload_duration+luci_reload_duration))
	rm -rf "$luci_reload_log"
    }
    debug_log "luci_reload_duration=$luci_reload_duration"

    echo "date:$(date)" >> "$total_reload_log"

    echo "$total_reload_duration" > "$log/total_reload_duration.$uid"
    debug_log "total_reload_duration=$total_reload_duration"

    echo "0"
}

luci_reload_time() {
    local uid="$1"
    local duration=""
    local path=""

    [ "$uid" == "" ] && [ -f "$log/last_uid" ] && uid=$(cat "$log/last_uid")

    [ "$uid" == "" ] && {
	echo "-1"
	return
    }

    luci_reload_log "$uid" > /dev/null 2>&1
   
    path="$log/total_reload_duration.$uid"
    [ -f "$path" ] && duration=$(cat $path)
    [ "$duration" != "" ] && echo "$duration" || echo "-1"
}

print_layer_name() {
    local id="$1"
    local name=""

    case $id in
    0) name="CMD";;
    1) name="VAP";;
    2) name="RADIO";;
    3) name="AP";;
    *) name="UNKNOWN";;
    esac
    echo "$name"
}

luci_reload_show() {
    local uid="$1"
    local duration=""
    local layer_id=""
    local layer_name=""
    local history=""

    [ "$uid" == "" ] && [ -f "$log/last_uid" ] && uid=$(cat "$log/last_uid")

    [ "$uid" == "" ] && {
        uid=$(luci_reload_uid)
	echo "UID is empty, so log cannot be generated."
	echo "New UID=$uid"
	return
    }

    [ -f "$log/total_reload_duration.$uid" ] && mv "$log/total_reload_duration.$uid" "$log/total_reload_duration.temp"
    [ -f "$log/total_reload_log.$uid" ] && mv "$log/total_reload_log.$uid" "$log/total_reload_log.temp"

    luci_reload_log "$uid" > /dev/null 2>&1

    if [ -f "$log/total_reload_duration.$uid" ]; then
        echo "============================== Total Reload Time =============================="
        duration=$(cat "$log/total_reload_duration.$uid")
        [ -n "$duration" ] && echo "duration:$duration" || echo "duration:-1"
    elif [ -f "$log/total_reload_duration.temp" ]; then
        echo "============================== Total Reload Time =============================="
        duration=$(cat "$log/total_reload_duration.temp")
        [ -n "$duration" ] && echo "duration:$duration" || echo "duration:-1"
    fi

    if [ -f "$log/total_reload_log.$uid" ]; then
        echo "============================== Top Layer =============================="
        layer_id=$(cat "$log/total_reload_log.$uid" | grep "layer:" | cut -c7-8)
        [ -n "$layer_id" ] && layer_name=$(print_layer_name "$layer_id")
        [ -n "$layer_name" ] && echo "layer:$layer_name" || echo "layer:NULL"
        history=$(cat "$log/total_reload_log.$uid")
        [ -n "$history" ] && echo "$history"
    elif [ -f "$log/total_reload_log.temp" ]; then
        echo "============================== Top Layer =============================="
        layer_id=$(cat "$log/total_reload_log.temp" | grep "layer:" | cut -c7-8)
        [ -n "$layer_id" ] && layer_name=$(print_layer_name "$layer_id")
        [ -n "$layer_name" ] && echo "layer:$layer_name" || echo "layer:NULL"
        history=$(cat "$log/total_reload_log.temp")
        [ -n "$history" ] && echo "$history"
    fi
}

luci_reload_uid() {
    local uid=""
    local max_num=0
    local list_num=0

    if [ -d "$log" ]; then
	[ -f "$log/list_uid" ] && list_num=$(cat "$log/list_uid" | wc -l)
        [ -f "/tmp/luci_reload_num" ] && max_num=$(cat "/tmp/luci_reload_num")

	[ "$max_num" == "0" ] && max_num=10

	[ "$list_num" -ge "$max_num" ] && rm -rf "$log/"*

	[ -f "$log/last_uid" ]  && {
	    uid=$(cat "$log/last_uid")
	    [ "$uid" != "" ] && echo "$uid" >> "$log/list_uid"
	}
    else
	mkdir "$log"
    fi

    [ -f "/tmp/luci_reload_uid" ]  && rm -rf "/tmp/luci_reload_uid"
    uid=$(openssl rand -hex 8)
    [ "$uid" != "" ] && {
	echo "$uid"
	echo "$uid" > "$log/last_uid"
	echo "$uid" > "/tmp/luci_reload_uid"
    }
}

luci_reload_num() {
    num="$1"
    
    [ "$num" != "" ] && echo "$num" > "/tmp/luci_reload_num"
    echo "$num"
}

luci_reload_backup() {
    uid="$1"
    backup=""

    date > "$log/backup_date"
    if [ "$uid" == "" ]; then
	[ -f "/tmp/backup_auto_test.tar.gz" ] && rm -rf "/tmp/backup_auto_test.tar.gz"
	tar -zcvf "/tmp/backup_auto_test.tar.gz" "/tmp/log/auto_test" > /dev/null 2>&1
	[ -f "/tmp/backup_auto_test.tar.gz" ] && backup="/tmp/backup_auto_test.tar.gz"
    else
        [ -f "/tmp/backup_auto_test.$uid.tar.gz" ] && rm -rf "/tmp/backup_auto_test.$uid.tar.gz"
        [ -f "$log/total_reload_log.$uid" ] && [ -f "$log/total_reload_duration.$uid" ] && [ -f "$log/backup_date" ] && {
	    tar -zcvf "/tmp/backup_auto_test.$uid.tar.gz" "$log/total_reload_log.$uid" "$log/total_reload_duration.$uid" "$log/backup_date" > /dev/null 2>&1
	    [ -f "/tmp/backup_auto_test.$uid.tar.gz" ] && backup="/tmp/backup_auto_test.$uid.tar.gz"
	}
    fi
    echo "$backup"
}

luci_reload_clear() {
    rm -rf /tmp/backup_auto_test.*.tar.gz > /dev/null 2>&1
    rm -rf /tmp/backup_auto_test.tar.gz > /dev/null 2>&1
    rm -rf /tmp/check_option.log.* > /dev/null 2>&1
    rm -rf /tmp/boost_reload.log.* > /dev/null 2>&1
    rm -rf /tmp/luci_reload.log.* > /dev/null 2>&1
    rm -rf /tmp/luci_reload_uid > /dev/null 2>&1
    rm -rf /tmp/luci_reload_num > /dev/null 2>&1
    [ -d "$log" ] && rm -rf "$log"
    [ -d "$log" ] && echo "-1" || echo "0"
}

reload_usage() {
    echo "usage: sh auto_test.sh $command \"$option\""
    echo "option list:"
    echo " num: setup max number of luci reload log, default:10"
    echo " uid: create uid of luci reload"
    echo " log: create log of luci reload <uid>"
    echo " time: show time of luci relaod <uid>"
    echo " backup: backup log of luci relaod <uid>"
    echo " clear: clear log of luci relaod"
    echo " show: show log of luci relaod"
}

luci_reload() {
    case "$1" in
    num) luci_reload_num $2;;
    uid) luci_reload_uid;;
    log) luci_reload_log $2;;
    time) luci_reload_time $2;;
    backup) luci_reload_backup $2;;
    clear) luci_reload_clear;;
    show) luci_reload_show;;
    *) usage;;
    esac
}

usage() {
    echo "usage: sh auto_test.sh \"$command\""
    echo "command list:"
    echo " reload: luci reload function"
}

[ "$debug_log" == "1" ] && {
    [ -d  "$log" ] || mkdir "$log"
    pid=$$
}

case "$1" in
    reload) luci_reload $2 $3;;
    *) usage;;
esac

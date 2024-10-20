
lock "/var/run/boost-reload"

changes_file="/tmp/wireless_changes"
boost_reload_log_need="$(uci get functionlist.functionlist.SUPPORT_LUCI_RELOAD_LOG 2>&1)"
boost_reload_log_file=""
boost_reload_log_do=0
boost_reload_changes=""
boost_reload_command=""
pid=$$
uid=""
ds=""
dd=""

wirte_log() {
        [ "$boost_reload_log_need" == "1" ] && [ -n "$boost_reload_log_file" ] && echo "$@" >> "$boost_reload_log_file"
}

log_start() {
    [ -f "/tmp/luci_reload_uid" ] && uid=$(cat "/tmp/luci_reload_uid")
    [ -n "$uid" ] && {
	boost_reload_log_file="/tmp/boost_reload.log.$uid"
	boost_reload_log_do=1
	ds=$(date +%s)
    }
}

log_end() {
    [ "$boost_reload_log_file" != "" ] && [ -n "$boost_reload_command" ] && {
	pid=$$
	dd=$(date +%s)
	[ -f "$boost_reload_log_file" ] && rm -rf "$boost_reload_log_file"
	wirte_log "============================== boost reload log =============================="
	wirte_log "pid:$pid"
	wirte_log "duration:$((dd-ds))"
	wirte_log "list_reload_commad:"
	wirte_log "$boost_reload_command"
	wirte_log "list_uci_changes:"
	wirte_log "$boost_reload_changes"
    }
}

### start log
[ -n "$boost_reload_log_need" ] && [ "$boost_reload_log_need" == "1" ] && boost_reload_log_do=1
[ "$boost_reload_log_do" == "1" ] && log_start

if [ -e $changes_file ]; then
	optionNum=$(cat $changes_file | wc -l)
	command_file="/tmp/reload_cmds.sh"

	if [ "$optionNum" != "0" ]; then
        	echo "====optionNum=$optionNum, not to do boost reload====" > /dev/console
		cp $changes_file $changes_file"_tmp" -f	#save the wireless change for option layer improving
		[ -e "$command_file" ] && mv $command_file $command_file"_tmp" -f
		[ -e $command_file"_tmp" ] && rm $command_file"_tmp" -f
	else
		echo "====enter boost reload flow=====" > /dev/console
		echo "wifi_boost_reload" > "/var/run/luci-reload-status"
		[ "$boost_reload_log_do" == "1" ] && boost_reload_changes="$(uci changes)"
        	uci commit wireless
        	if [ -e "$command_file" ]; then
			cp $command_file $command_file"_tmp" -f
			[ "$boost_reload_log_do" == "1" ] && boost_reload_command="$(cat $command_file)"
                	sh $command_file
	                rm $command_file -f
        	fi
		echo "====boost reload finished=====" > /dev/console
		[ -e $changes_file"_tmp" ] && rm $changes_file"_tmp" -f
	fi
	rm $changes_file -f
fi

### end log
[ "$boost_reload_log_do" == "1" ] && log_end

lock -u "/var/run/boost-reload"

#!/bin/sh
 . /etc/mgmt.d/mgmt_path.sh

MGMT_DEFAULT_PW="12345678"
MGMT_DEFAULT_IDLE_TIME=30
MGMT_TIMER_HANDLER="/tmp/mgmt_timer_handler"
MGMT_TIMER_HANDLER_PID_FILE="/tmp/mgmt_timer_handler_pid"

#default_cfg80211=0

#is_cfg80211() {
#	local cfg80211=0
#	if [ -f "/ini/global.ini" ]; then
#		cfg80211=$(cat /ini/global.ini | grep cfg80211_config | awk -F "=" '{print $2}')
#	fi
#	echo "$cfg80211"
#}

mgmt_idle_feature() {
    config_get livetime $MGMT_SECTION livetime
    [ "$livetime" = "0" ] && echo "0" || echo "1"
}

is_mgmt_setting_initialzed(){
    #config_get key $MGMT_SECTION key
    #[ "$key" = "$MGMT_DEFAULT_PW" ] && echo "0" || echo "1"
    [ -f "$MGMT_FIRST_BOOT_FILE" ] && echo "1" || echo "0"
}


if [ $(is_mgmt_setting_initialzed) = 1 ]; then
    config_get livetime $MGMT_SECTION livetime
    mgmt_timer_min=$livetime
else
    mgmt_timer_min=$MGMT_DEFAULT_IDLE_TIME
fi


#gen_timer_handler() {
#cat <<END > $MGMT_TIMER_HANDLER
##!/bin/sh
#echo "\$\$" > $MGMT_TIMER_HANDLER_PID_FILE
#sleep ${mgmt_timer_min}m && iwpriv $MGMT_IFACE hide_ssid 1;iwpriv $MGMT_IFACE dynamicbeacon 1;iwpriv $MGMT_IFACE nobeacon 1
#$MGMT_RULE_SCRIPT "unload"
#
#END
#
#chmod 755 $MGMT_TIMER_HANDLER
#}
#
#
#gen_timer_handler_cfg80211() {
#cat <<END > $MGMT_TIMER_HANDLER
##!/bin/sh
#echo "\$\$" > $MGMT_TIMER_HANDLER_PID_FILE
#sleep ${mgmt_timer_min}m && cfg80211tool $MGMT_IFACE hide_ssid 1;cfg80211tool $MGMT_IFACE dynamicbeacon 1
#$MGMT_RULE_SCRIPT "unload"
#
#END
#
#chmod 755 $MGMT_TIMER_HANDLER
#}

gen_timer_handler_wifi_control() {
cat <<END > $MGMT_TIMER_HANDLER
#!/bin/sh
echo "\$\$" > $MGMT_TIMER_HANDLER_PID_FILE
sleep ${mgmt_timer_min}m && wifi-control.sh beacon off $MGMT_IFACE
$MGMT_RULE_SCRIPT "unload"

END

chmod 755 $MGMT_TIMER_HANDLER
}

set_timer() {
    #default_cfg80211=$(is_cfg80211)
    clear_timer
    #if [ $default_cfg80211 -eq 1 ]; then
    #    gen_timer_handler_cfg80211
    #else
    #    gen_timer_handler
    #fi
    gen_timer_handler_wifi_control
    $MGMT_TIMER_HANDLER &
}

clear_timer() {
    [ -f $MGMT_TIMER_HANDLER ] && rm -f $MGMT_TIMER_HANDLER
    if [ -f $MGMT_TIMER_HANDLER_PID_FILE ]; then
        local mgmt_timer_handler_pid=$(cat $MGMT_TIMER_HANDLER_PID_FILE)
        local mgmt_timer_handler_sleep_pid=$(pgrep -P $mgmt_timer_handler_pid)
        kill -9 $mgmt_timer_handler_sleep_pid && echo "sleep  $mgmt_timer_handler_sleep_pid killled!"
        kill -9 $mgmt_timer_handler_pid && rm -f $MGMT_TIMER_HANDLER_PID_FILE
    fi
}


case "$(mgmt_idle_feature):$1" in
    1:set_timer)
        set_timer
        ;;
    1:clear_timer)
        clear_timer
        ;;
    0:*)
        clear_timer
        ;;
esac

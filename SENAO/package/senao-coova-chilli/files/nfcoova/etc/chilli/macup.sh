#!/bin/sh
#DEBUG=y
[ -n "${DEBUG}" ] && echo -e "\r\\033[34m[$(cat /proc/$PPID/status|grep ^Name:|cut -f 2)]$0\\033[0m" > /dev/console
[ -n "${DEBUG}" ] && echo CONFIG_NAME:$CONFIG_NAME > /dev/console
[ -n "${DEBUG}" ] && echo guest mac:$CALLING_STATION_ID > /dev/console
[ -n "${DEBUG}" ] && echo RUNSCRIPT:$RUNSCRIPT > /dev/console
[ -n "${DEBUG}" ] && echo SESSION_TIME:$SESSION_TIME > /dev/console
[ -n "${DEBUG}" ] && echo SESSION_TIMEOUT:$SESSION_TIMEOUT > /dev/console
[ -n "${DEBUG}" ] && echo IDLE_TIME:$IDLE_TIME > /dev/console
[ -n "${DEBUG}" ] && echo IDLE_TIMEOUT:$IDLE_TIMEOUT > /dev/console
[ -n "${DEBUG}" ] && echo AUTHENTICATED:$AUTHENTICATED > /dev/console
[ -n "${DEBUG}" ] && echo USER_NAME:$USER_NAME > /dev/console
[ -n "${DEBUG}" ] && echo GONE_TIME:$GONE_TIME > /dev/console
[ -n "${DEBUG}" ] && echo SYNC_FLAG:$SYNC_FLAG > /dev/console

if [ "${CONFIG_NAME/port/}" != "$CONFIG_NAME" ]
then
    is_ethernet=1
else
    is_ethernet=0
fi

if [ "$is_ethernet" = "1" ]
then
    return 0
else
    ssid=${CONFIG_NAME#br-ssid}
    ssid=${ssid#br-nat}
fi

# check client here again
if [ "$SYNC_FLAG" = "1" -a "`wifi-control.sh hostapd check_sta $ssid ${CALLING_STATION_ID//-/:}`" = "" ]
then
    . /etc/chilli/chilli-libs.sh
    chilli_query_client left $CONFIG_NAME $CALLING_STATION_ID $(($GONE_TIME+1)) ${IDLE_TIME:-0}
else
    if [ "$CONFIG_NAME" != "" ];then
        if [ "$AUTHENTICATED" == "1" ]
        then
            session_time=${SESSION_TIME:-0}
            session_timeout=${SESSION_TIMEOUT:-0}
            idle_time=${IDLE_TIME:-0}
            idle_timeout=${IDLE_TIMEOUT:-0}
            session_time_remain=$(($session_timeout-$session_time))
            idle_time_remain=$(($idle_timeout-$idle_time))

            if [ $session_timeout -ne 0 ]
            then
                if [ $session_time_remain -lt 20 ]
                then
                    return 0
                fi
            fi

            session_time=$(($session_time+2))

            if [ $idle_timeout -ne 0 ]
            then
                if [ $idle_time_remain -lt 20 ]
                then
                    return 0
                fi
            fi

            idle_time=$(($idle_time+1))

            . /tmp/etc/chilli/$CONFIG_NAME/config

            if [ "${HS_GUEST_SYNC}" = "on" ]; then
                if [ "$GUESTSYNCD_FORCESEND" = "1" ]
                then
                    sendcmd=forcesend
                else
                    sendcmd=send
                fi
                # TODO: only bridge mode do sync ip
                if [ "$HS_BRIDGEMODE" != "on" ]
                then
                    FRAMED_IP_ADDRESS=0.0.0.0
                fi
                cmd="guest_syncli $sendcmd update ${CALLING_STATION_ID//[:|-]/} $CONFIG_NAME ${FRAMED_IP_ADDRESS:-0.0.0.0} $session_time $session_timeout $idle_time $idle_timeout $(($GONE_TIME+1)) $INPUT_PACKETS $OUTPUT_PACKETS $INPUT_OCTETS $OUTPUT_OCTETS $ACCT_INTERIM_INTERVAL $ACCT_SESSION_ID '${USER_NAME}' 1>/dev/null 2>&1 &"
                eval $cmd
            fi
        fi
    fi
fi
return 0

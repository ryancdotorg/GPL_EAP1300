#!/bin/sh
#DEBUG=y
[ -n "${DEBUG}" ] && echo -e "\r\\033[34m[$(cat /proc/$PPID/status|grep ^Name:|cut -f 2)]$0\\033[0m" > /dev/console
[ -n "${DEBUG}" ] && echo guest mac:$CALLING_STATION_ID > /dev/console
[ -n "${DEBUG}" ] && echo terminate_cause:$TERMINATE_CAUSE > /dev/console

. /tmp/etc/chilli/$CONFIG_NAME/config
DHCPIF=$HS_LANIF
if [ "$HS_BRIDGEMODE" = "on" ]
then
    # do nothing
    :
else
    if [ -n "$FRAMED_IP_ADDRESS" -a "$FRAMED_IP_ADDRESS" != "0.0.0.0" ]
    then
        ip route del $FRAMED_IP_ADDRESS dev $DHCPIF
        if [ "$HS_WANIF" != "br-lan" ]
        then
            ip route del $FRAMED_IP_ADDRESS dev $DHCPIF table ${CONFIG_NAME}vlan
        else
            ip route del $FRAMED_IP_ADDRESS dev $DHCPIF table portal-main
        fi
    fi
fi

# 1:RADIUS_TERMINATE_CAUSE_USER_REQUEST, 6:RADIUS_TERMINATE_CAUSE_ADMIN_RESET
if [ "$TERMINATE_CAUSE" = "6" -o "$TERMINATE_CAUSE" = "1" ]
then
    . /etc/chilli/chilli-libs.sh
    chilli_query_client logout $CONFIG_NAME $CALLING_STATION_ID
fi

#!/bin/sh
#DEBUG=y
[ -n "${DEBUG}" ] && echo -e "\r\\033[34m[$(cat /proc/$PPID/status|grep ^Name:|cut -f 2)]$0\\033[0m" > /dev/console
[ -n "${DEBUG}" ] && echo CONFIG_NAME:$CONFIG_NAME > /dev/console
[ -n "${DEBUG}" ] && echo guest mac:$CALLING_STATION_ID > /dev/console


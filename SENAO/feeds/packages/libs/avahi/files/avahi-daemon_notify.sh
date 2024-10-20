#!/bin/sh

iface=$1
act=$3

case "$act" in
    "join"|"left")
        if [ "${iface:0:4}" == "mgmt" ]; then
		/etc/init.d/avahi-daemon restart &
        fi
        ;;
esac

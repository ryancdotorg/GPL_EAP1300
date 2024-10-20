#!/bin/sh

# WAR : advoid wtp mem too high

while [ 1 ]; do
    sleep 300
    memlist=$(ps | grep wtp | grep capwap | grep -v grep | awk -F " " '{print $3}')
    for mem in $memlist; do
        if [ $mem -gt 40000 ];then
            killall wtp
        fi
    done
done

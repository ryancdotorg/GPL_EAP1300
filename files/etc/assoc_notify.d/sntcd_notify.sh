#!/bin/sh

iface=$1
# mac=$2
# act=$3

touch /var/run/sntcd/stamp/$iface.sync

killall -SIGUSR1 sntcd

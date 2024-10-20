#!/bin/sh
#must use iputils-ping, busybox is /bin/ping
pingtool=/usr/bin/ping

check_mtu()
{
    local size=$1
    local timeout=1
    local failure=0
    local success=0

    while true; do
        if [ $size -gt 1472 -o $size -lt 1384 ]
        then
            # mtu 1500
            size=1472
            break
        elif [ $failure -gt 1 -a $success -gt 1 ]
        then
            size=$(($size-4))
            break
        fi

        $pingtool -s $size -c 1 -W $timeout -w $timeout -M do $target 1>/dev/null 2>&1
        if [ $? -eq 0 ];then
            success=$(($success+1))
            size=$(($size+4))
            if [ $failure -gt 0 -a $success -gt 0 ]
            then
                timeout=2
            fi
        else
            failure=$(($failure+1))
            size=$(($size-4))
        fi
    done
    echo $size
}

#TODO: use fping
for target in 8.8.8.8 208.67.220.220 208.67.220.222
do
	$pingtool -c 1 -w 2 $target 1>/dev/null 2>&1
	ret=$?
	[ $ret -eq 0 ] && break
done

if [ $ret -ne 0 ]
then
    # if proxy enabled, always set 1500
    proxy_use_profile=`uci -q get redsocks2.ethernet.proxy_use_profile`
    for profile in $proxy_use_profile; do
        if [ "$profile" != "none" ]; then
            redsocks_enable=1
            echo 1500
            exit 0
        fi
    done
fi

newsize=$(check_mtu 1472)

echo $(($newsize+28))

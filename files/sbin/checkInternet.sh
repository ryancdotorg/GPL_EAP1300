#!/bin/sh
# use http method first. if no internet, try to ping.

online=1
online_dns=1
online_server=1
ipv6_online=1
server="google.com ipinfo.io/ip"
PING="ping -w1"
    DNS="8.8.8.8"
[ -e "/usr/bin/fping" ] && {
	PING="fping -t500 -c1"
	DNS="8.8.8.8 208.67.220.222"
}

# google dns + opendns
for t in $(seq 1 3); do
    for s in $DNS; do
        $PING $s > /dev/null 2>&1
        online_dns=$?
        [ $online_dns -eq 0 ] && break
    done
    [ $online_dns -eq 0 ] && break
done

for i in $server; do
    curl -m 3 -s $i > /dev/null 2>&1
    online_server=$?
    [ $online_server -eq 0 ] && break
done

if [ $online_dns -eq 0 -a $online_server -eq 0 ]; then
    online=0;
fi

if [ $online -ne 0 ]; then
	if [ -x /bin/ping6 ]; then
		ping6 -w1 2001:4860:4860::8888 > /dev/null 2>&1
		ipv6_online=$?
	fi
fi

if [ $online -eq 0 -o $ipv6_online -eq 0 ]; then
	echo -n "Online"
else
	echo -n "Offline"
fi

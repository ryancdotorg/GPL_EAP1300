#!/bin/sh
add_garden()
{
    local config_name=$1
    local gardenip=$2
    echo  "chilli_query -s /var/run/chilli.$config_name.sock remgarden data $gardenip" >> /var/run/chilli.garden.$config_name.sh
    chilli_query -s /var/run/chilli.$config_name.sock addgarden data $gardenip
}

rem_garden()
{
    local config_name=$1
    [ -e "/var/run/chilli.garden.$config_name.sh" ] && sh /var/run/chilli.garden.$config_name.sh 2>/dev/null
    rm -f /var/run/chilli.garden.$config_name.sh 2>/dev/null
}

AWS_IP_RANGE_FILE=/tmp/aws-ip-ranges.json
AWS_SYNC_TOKEN_FILE=/tmp/aws-sync-token.txt
[ "$1" != "" ] && force_check=1 || force_check=0
# br-ssid1, br-ssid2
chilli_dhcp_if=${1:-`ls /tmp/etc/chilli/`}
curl --connect-timeout 3 -m 3 -s --insecure https://ip-ranges.amazonaws.com/ip-ranges.json -o $AWS_IP_RANGE_FILE
new_syncToken=`jq -r '.syncToken' < $AWS_IP_RANGE_FILE`
old_syncToken=`test -f $AWS_SYNC_TOKEN_FILE && cat $AWS_SYNC_TOKEN_FILE`
if [ $force_check -eq 1 ] || [ "$new_syncToken" != "$old_syncToken" ]
then
    echo $new_syncToken > $AWS_SYNC_TOKEN_FILE
    cloudfront_ip=`jq -r '.prefixes[] | select(.service=="CLOUDFRONT") | .ip_prefix' < $AWS_IP_RANGE_FILE`
    for cif in $chilli_dhcp_if
    do
        rem_garden $cif
        for ip in $cloudfront_ip
        do
            add_garden $cif $ip
        done
    done
fi

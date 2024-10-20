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

. /etc/chilli/fbwifi-libs.sh
# br-ssid1, br-ssid2
#6,31.13.24.0/21,443
#6,31.13.64.0/18,443
#6,45.64.40.0/22,443
#6,66.220.144.0/20,443
#6,69.63.176.0/20,443
#6,69.171.224.0/19,443
#6,74.119.76.0/22,443
#6,102.132.96.0/20,443
#6,103.4.96.0/22,443
#6,129.134.0.0/17,443
#6,157.240.0.0/19,443
#6,157.240.192.0/18,443
#6,173.252.64.0/18,443
#6,179.60.192.0/22,443
#6,185.60.216.0/22,443
#6,185.89.216.0/22,443

chilli_dhcp_if=${1:-`ls /tmp/etc/chilli/`}
garden_list=`cat $(fbwifi_get_config_file $chilli_dhcp_if) |jq -rcM '.config.traffic_allowlist[] | "\(.protocol),\(.ip),\(.port)"'`
for cif in $chilli_dhcp_if
do
    rem_garden $cif
    for list in $garden_list
    do
        protocol=${list%%,*}
        case "$protocol" in
            17)
                proto=udp
                ;;
            1)
                proto=icmp
                ;;
            *)
                # 6
                proto=tcp
                ;;
        esac
        _ip=${list#*,}
        ip=${_ip%,*}
        port=${list##*,}

        add_garden $cif $proto:$ip:$port
    done
done

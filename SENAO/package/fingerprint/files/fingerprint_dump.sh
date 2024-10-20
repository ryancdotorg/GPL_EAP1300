#!/bin/sh
. /etc/functions.sh

output_dir=$1

# this dir created by ezmclientinfo.
fingerprint_ready_dir="/var/run/fingerprint/ready"
dhcp_leases_file="/tmp/dhcp.leases"

if [ -n "$output_dir" ]
then
    mkdir -p $output_dir
    WIFI_STATION_LIST_PREFIX="/$output_dir/fingerprint_wifi_list_"
    WIFI_STATION_FP_STAUS_PREFIX="/$output_dir/fingerprint_status_list_"
elif [ -d "$fingerprint_ready_dir" -a -n "$(pidof ezmclientinfo)" ]
then
    rm -f /tmp/fingerprint_wifi_*
    rm -f /tmp/fingerprint_status_*
    cp $fingerprint_ready_dir/* /tmp/
    exit 0
else
    WIFI_STATION_LIST_PREFIX="/tmp/fingerprint_wifi_list_"
    WIFI_STATION_FP_STAUS_PREFIX="/tmp/fingerprint_status_list_"
fi
fingerprint_data="/tmp/fingerprint.txt"
fingerprint_db_data="/tmp/fingerprint_db.txt"
fingerprint_tmp_dir="/tmp/fingerprint_tmp"
[ -f $fingerprint_tmp_dir ] && rm -f $fingerprint_tmp_dir
mkdir -p $fingerprint_tmp_dir
fingerprint_data_tmp=`mktemp -u -p $fingerprint_tmp_dir 2>/dev/null` || fingerprint_data_tmp="$fingerprint_tmp_dir/fingerprint_tmp.txt"
fingerprint_db_data_tmp=`mktemp -u -p $fingerprint_tmp_dir 2>/dev/null` || fingerprint_db_data_tmp="$fingerprint_tmp_dir/fingerprint_db_tmp.txt"
MESH_IFACE="ath35"

config_load wireless

dumpFPInfo()
{
    test -f $fingerprint_data && cp $fingerprint_data $fingerprint_data_tmp -f || touch $fingerprint_data_tmp
	test -f $fingerprint_db_data && cp $fingerprint_db_data $fingerprint_db_data_tmp -f || touch $fingerprint_db_data_tmp
	config_foreach dumpWlanStaFPInfo wifi-device
	rm -f $fingerprint_data_tmp
	rm -f $fingerprint_db_data_tmp
}

get_fingerprint_ip()
{
    local mac=$1
    shift
    local input=$@
    local _tmp=${input#$mac|}
    local ip=${_tmp%%|*}
    echo $ip
}

dumpWlanStaFPInfo()
{
    vifs=$(eval "/usr/sbin/foreach wireless wifi-iface device $1")

    for vif in $vifs; do
        config_get ifname "$vif" ifname
        config_get disabled "$vif" disabled
        config_get ssid "$vif" ssid
        config_get mode_display "$vif" mode_display
        config_get guest_network "$vif" guest_network

        WIFI_STATION_FP_STAUS=$WIFI_STATION_FP_STAUS_PREFIX$ifname
        WIFI_STATION_LIST=$WIFI_STATION_LIST_PREFIX$ifname
        case "$disabled:$ifname:$mode_display" in
            1:*)
                    [ -f $WIFI_STATION_FP_STAUS ] && rm $WIFI_STATION_FP_STAUS -f
                    continue
                ;;
            *:$MESH_IFACE)
                    continue
                ;;
            0:ath*:ap | 0:ath*:wdsap | 0:ath*:sta_ap)
                    : > $WIFI_STATION_FP_STAUS
                    wlanconfig $ifname list sta | tail -n +2 > $WIFI_STATION_LIST
                    lineNum=$(sed -n '$=' "$WIFI_STATION_LIST")
                    [ -z "$lineNum" ] && continue

                    for i in `seq 1 $lineNum`; do
			local devFP=""
                        devMAC=$(cat $WIFI_STATION_LIST | awk '{print $1}' | head -n $i | tail -n 1 | sed 'y/ABCDEF/abcdef/')
                        revise_client_ip=""
                        [ -z "$devMAC" ] && continue

                        case "$guest_network" in
                            NAT_only)
                                if [ -f "$dhcp_leases_file" ]
                                then
                                    _revise_client_ip=`cat $dhcp_leases_file |awk '/'$devMAC'/{print $2" "$3}' |grep ^$devMAC |tail -1`
                                    revise_client_ip=${_revise_client_ip#* }
                                fi
                                ;;
                            Bridge|NAT)
                                nfcoova_proc_path="/proc/net/coova/coova-br-ssid${vif##*_}"
                                if [ -f $nfcoova_proc_path ]
                                then
                                    _revise_client_ip=`cat $nfcoova_proc_path |grep -i ^mac=${devMAC//:/-}`
                                    if [ `echo "$_revise_client_ip" |wc -l` -ne 1 ]
                                    then
                                        _revise_auth_client=`echo "$_revise_client_ip" |grep 'state=1'`
                                        if [ `echo "$_revise_auth_client" |wc -l` -ne 1 ]
                                        then
                                            # FIXME check subnet is best...
                                            revise_client_ip=`chilli_query -s /var/run/chilli.br-ssid${vif##*_}.sock list mac $devMAC |awk '{printf $2}'`
                                        else
                                            revise_client_ip=`echo "$_revise_auth_client" |awk -F "=| " {'printf $4'}`
                                        fi
                                    else
                                        revise_client_ip=`echo "$_revise_client_ip" |awk -F "=| " {'printf $4'}`
                                    fi
                                fi
                                ;;
                            *)
                                ;;
                        esac
                        if [ -n "$output_dir" -a -f "$fingerprint_db_data_tmp" ]
                        then
                            devFP=$(cat $fingerprint_db_data_tmp | grep -i $devMAC | tail -1)
                        fi
                        if [ -z "$devFP" -a -f "$fingerprint_db_data_tmp" ]
                        then
                            devFP=$(cat $fingerprint_db_data_tmp | grep -i $devMAC | tail -1)
                        fi
                        if [ -z "$devFP" -a -f "$fingerprint_data_tmp" ]
                        then
                            devFP=$(cat $fingerprint_data_tmp | grep -i $devMAC | tail -1)
                        fi

                        if [ -z "$devFP" ]
                        then
                            devFP="$devMAC|${revise_client_ip:-0.0.0.0}|LINUX|user-${devMAC//:/-}"
                        fi

                        local client_ip=$(get_fingerprint_ip $devMAC $devFP)
                        if [ "$revise_client_ip" != "" -a "$revise_client_ip" != "$client_ip" -a "$client_ip" != "0.0.0.0" -a "$revise_client_ip" != "0.0.0.0" ]
                        then
                            # replace by revise client ip
                            devFP=`echo "$devFP" |sed "s/|$client_ip|/|$revise_client_ip|/"`
                        else
                            # TODO: revise by fingerprint_sync or senao-client-sniffer
                            echo
                        fi

                        # always echo fp status to file
                        echo $devFP >> $WIFI_STATION_FP_STAUS
                    done
                    ;;
        esac
    done
}


dumpFPInfo

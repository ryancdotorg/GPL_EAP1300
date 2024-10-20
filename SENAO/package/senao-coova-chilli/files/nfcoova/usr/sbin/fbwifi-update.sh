#!/bin/sh

. /etc/chilli/chilli-libs.sh
. /etc/chilli/fbwifi-libs.sh

if [ -n "$CONFIG_NAME" ]
then
    config_name=$CONFIG_NAME
else
    config_name=$1
fi

if [ -z "$config_name" ]
then
    for config in $(fbwifi_get_all_config_name)
    do
        fbwifi-update.sh $config
    done
    return 0
fi

get_value(){
    local list=$1
    local token=$2
    local key=$3
    echo "$list" |jq -r -M  ".sessions[]| select(.session.userName==\"$token\" and .dhcpState==\"pass\")" | jq -r -M ".$key"
}

section_name=$(fbwifi_get_section_name $config_name)
gateway_id=$(fbwifi_get_gateway_id $config_name)
gateway_token=$(fbwifi_get_gateway_token $config_name)
tokens_validation_api="https://api.fbwifi.com/v2.0/tokens?access_token=$gateway_token"

config_version=$(fbwifi_get_config_version $config_name)


if [ -z "$config_version" ]
then
    echo "FBWIFI: no config version, reload $config_name to got new config" > /dev/console
    chilli_retry_crontab add $config_name
    return 0
fi

#ssid_num=$(get_ssid_num $config_name)
userlist=`chilli_query -s /var/run/chilli.$config_name.sock -json list`
#clientlist=`wifi-control.sh hostapd list_sta $ssid_num`
tokens=`echo "$userlist" |jq -r -M ".sessions[]| select(.dhcpState==\"pass\").session.userName"`
token_string=""
fingerprint_dir="/var/run/fingerprint/ready/"
traffic_type="total"

profile_name=$(fbwifi_get_profile_name $config_name)
fbwifi_tokens_dir=$fbwifi_tmp_dir/$config_name/$fbwifi_tokens_path

if [ ! -d "$fingerprint_dir" ]
then
    fingerprint_dir="/tmp/"
fi
if [ "$tokens" = "null" ]
then
    #only update config_version
    :
else
    add_comma=""
    _tokens=$tokens
    tokens=${tokens//-/}
    if [ "$_tokens" != "$tokesn" ]
    then
        # it's weird user... release it.
        weird_macs=$(get_value "$userlist" "-" "macAddress")
        for wmac in $weird_macs
        do
            [ -n "${FBWIFI_DEBUG}" ] && echo "FBWIFI: release username - : $wmac" > /dev/console
            chilli_query -s /var/run/chilli.$config_name.sock dhcp-release $wmac &
            release_mac_upper=$(echo $wmac | tr '[a-z]' '[A-Z]')
            release_mac_file=${release_mac_upper//:/-}
            rm -f $fbwifi_tokens_dir/$release_mac_file
        done
    fi

    for token in $tokens
    do
        per_mac=$(get_value "$userlist" "$token" "macAddress")
        per_mac=${per_mac//-/:}
        per_mac=$(echo $per_mac | tr '[A-Z]' '[a-z]')
        [ -n "${FBWIFI_DEBUG}" ] && echo "FBWIFI: token: $token" > /dev/console
        [ -n "${FBWIFI_DEBUG}" ] && echo "FBWIFI: mac: $per_mac" > /dev/console
        per_incoming=$(get_value "$userlist" "$token" "accounting.inputOctets")
        per_outgoing=$(get_value "$userlist" "$token" "accounting.outputOctets")
        per_sessiontime=$(get_value "$userlist" "$token" "accounting.sessionTime")
        per_idletime=$(get_value "$userlist" "$token" "accounting.idleTime")
        per_ishere=$(get_value "$userlist" "$token" "accounting.present")
        per_noaccounting=$(get_value "$userlist" "$token" "accounting.noAccounting")
        count=1

        for each_mac in $per_mac
        do
            per_signal=`cat $fingerprint_dir/fingerprint_wifi_list_* |grep -i -m 1 "^${each_mac}" |head -n 1 |awk '{print $6}'`
            per_tput=`cat $fingerprint_dir/fingerprint_wifi_list_* |grep -i -m 1 "^${each_mac}" |head -n 1 |awk '{print $5}'`
            per_tput=${per_tput//[a-zA-Z]/}

            per_token=""

            per_ishere=`echo $per_ishere |cut -d' ' -f$count`
            per_noaccounting=`echo $per_noaccounting |cut -d' ' -f$count`
            # per_isconn=$(echo "$clientlist" |grep -q -w "$each_mac" && echo 1 || echo 0)

            #
            # present  per_noaccounting    is_connected
            #    1        0                  true
            #    0        0                  false
            #    1        1                  true  # never happen
            #    0        1                   x    # user connect to other ap
            #
            # echo $each_mac $per_ishere $per_noaccounting > /dev/console

            if [ "$per_ishere" = "0" -a "$per_noaccounting" = "0" ]
            then
                     per_token="$add_comma'$token':{
 'incoming':null,
 'outgoing':null,
 'connected_time_sec':null,
 'inactive_time_sec':null,
 'signal_rssi_dbm':null,
 'expected_tput_mbps':null,
 'is_connected':false
}"
            elif [ "$per_ishere" = 1 ]
            then
                    per_incoming=`echo $per_incoming |cut -d' ' -f$count`
                    per_outgoing=`echo $per_outgoing |cut -d' ' -f$count`
                    per_sessiontime=`echo $per_sessiontime |cut -d' ' -f$count`
                    per_idletime=`echo $per_idletime |cut -d' ' -f$count`

                    per_token="$add_comma'$token':{
 'incoming':${per_incoming:-0},
 'outgoing':${per_outgoing:-0},
 'connected_time_sec':${per_sessiontime:-0},
 'inactive_time_sec':${per_idletime:-0},
 'signal_rssi_dbm':${per_signal:-88},
 'expected_tput_mbps':${per_tput:-300},
 'is_connected':true
}"
            fi

            if [ -n "$per_token" ]
            then
                [ -z "$add_comma" ] && add_comma=",
"
                token_string="$token_string$per_token"
            fi

            count=$(($count+1))
        done
    done

    if [ -n "$token_string" ]
    then
            token_string="tokens={
${token_string}
}"
    fi

# echo "$token_string" > /dev/console
fi

retval=`$curl_command -X POST "$tokens_validation_api" ${token_string:+--data "$token_string"} --data "traffic_type=$traffic_type" --data "config_version=$config_version"`
if [ $? -ne 0 ]
then
    echo "FBWIFI: curl token validation error" > /dev/console
    return 1
fi

[ -n "${FBWIFI_DEBUG}" ] && echo "$retval" > /dev/console

error_counter_file=$fbwifi_tmp_dir/$config_name/counter.error
error_msg=`echo "$retval" | jq -r -M ".error"`
if [ "$error_msg" != "null" ]
then
    # never happen...
    ezmreport -c 1057004992 -t fbwifi "[tokens] $error_msg" &
    error_times=`cat $error_counter_file 2>/dev/null`
    error_times=$((${error_times:-0}+1))
    echo $error_times > $error_counter_file
    if [ $error_times -gt 10 ]
    then
        echo "FBWIFI: error $error_times times, add crontab to reload portal to hotfix error" > /dev/console
        chilli_retry_crontab add $config_name
    fi
    return 1
else
    [ -f "$error_counter_file" ] && rm $error_counter_file
fi

# release invalid tokens
invalid_tokens=`echo "$retval" |jq -r -M ".tokens | select(.[].valid==false)" | jq -r -M 'keys[]'`

invalid_macs=""
for release_token in $invalid_tokens
do
invalid_per_mac=$(get_value "$userlist" "$release_token" "macAddress")
invalid_macs="$invalid_macs $invalid_per_mac"
done

for release_mac in $invalid_macs
do
    [ -n "${FBWIFI_DEBUG}" ] && echo "FBWIFI: release: $release_mac" > /dev/console
    chilli_query -s /var/run/chilli.$config_name.sock dhcp-release $release_mac &
    release_mac_upper=$(echo $release_mac | tr '[a-z]' '[A-Z]')
    release_mac_file=${release_mac_upper//:/-}
    rm -f $fbwifi_tokens_dir/$release_mac_file
done


# check is config valid?
is_config_valid=`echo "$retval" |jq -r -M ".config_valid"`

if [ "$is_config_valid" = "false" ]
then
    echo "FBWIFI: config expired, reload $config_name to got new config" > /dev/console
    chilli_retry_crontab add $config_name
    return 0
fi



#!/bin/sh

. /etc/chilli/chilli-libs.sh
. /etc/chilli/fbwifi-vars.sh

fbwifi_get_profile_name() {
    #section_name/config_name/profile_name
    local name=$1
    name=${name#br-}
    name=${name//[-_]/}
    echo $name
}

fbwifi_get_section_name() {
    #section_name/config_name/profile_name
    local name=$1
    local section_id=${name##*_}
    section_id=${section_id//[a-z_-]/}

    if [ "$name" != "${name/port/}" ]
    then
        echo port_$section_id
    else
        echo ssid_$section_id
    fi
}

fbwifi_get_gateway_id() {
    local config_name=$1
    local section_name=$(fbwifi_get_section_name $config_name)
    local gateway_id=$(uci -q get portal.$section_name.gateway_id)

    echo "$gateway_id"
}

fbwifi_get_gateway_token() {
    local config_name=$1
    local section_name=$(fbwifi_get_section_name $config_name)
    local vendor_id=$(uci -q get portal.$section_name.vendor_id)
    local gateway_id=$(fbwifi_get_gateway_id $config_name)
    local gateway_secret=$(uci -q get portal.$section_name.gateway_secret)

    echo "FBWIFI:GATEWAY|$vendor_id|$gateway_id|$gateway_secret"
}

fbwifi_get_login_page() {
    local config_name=$1
    local gateway_id=$(fbwifi_get_gateway_id $config_name)
    echo "https://www.facebook.com/wifiauth/login/?gw_id=$gateway_id"
}

fbwifi_get_landing_page() {
    local config_name=$1
    local gateway_id=$(fbwifi_get_gateway_id $config_name)
    echo "https://www.facebook.com/wifiauth/portal/?gw_id=$gateway_id"
}

fbwifi_get_venue_page() {
    local config_name=$1
    local gateway_id=$(fbwifi_get_gateway_id $config_name)
    echo "https://www.facebook.com/wifiauth/portal/?gw_id=$gateway_id&source=capport"
}

fbwifi_include_urls_config() {
    local config_name=$1
    local urls_file=$(fbwifi_get_urls_file $config_name)
    if [ -f "$urls_file" ]
    then
        . $urls_file
    fi
}

fbwifi_info_update() {
    local config_name=$1
    local gateway_token=$(fbwifi_get_gateway_token $config_name)
    local gateway_api="https://api.fbwifi.com/v2.0/gateway?access_token=$gateway_token"
    local section_name=$(fbwifi_get_section_name $config_name)

    local lan_mac=`cat /sys/class/net/${REAL_WANIF:-br-lan}/address | awk -F : '{printf $3$4$5}'`
    local vendor_name="`uci -q get sysProductInfo.model.venderName || echo EnGenius`"

    # gateway info
    local name="$vendor_name `cat /etc/modelname 2>/dev/null`-$lan_mac"
    local sw_version="`cat /etc/version |head -1|awk '{printf $1}'`"
    local hw_version="`uci -q get sysProductInfo.model.HWID || echo unknown`"
    local bssids=""
    local ssids=""

    for i in 0 1 4
    do
        local wireless_section="wireless.wifi${i}_${section_name}"
        local is_wireless_disabled=`uci -q get $wireless_section.disabled`
        if [ "${is_wireless_disabled:-1}" != "1" ]
        then
            local per_ifname="`uci -q get $wireless_section.ifname`"
            # spec. 4.4 bssid should be in lower-case and separated by colons (:)
            local per_bssid="`cat /sys/class/net/$per_ifname/address | tr '[A-Z]' '[a-z]'`"
            local per_ssid="`uci -q get $wireless_section.ssid | openssl enc -base64 -e -A`"
            if [ -n "$per_bssid" -a -n "$per_ssid" ]
            then
                bssids="${bssids}${bssids:+,}'$per_bssid'"
                ssids="${ssids}${ssids:+,}'$per_ssid'"
            fi
        fi
    done

    # spec 4.4, all ap of network shared one gateway_id, keep the update name, sw_version and hw_version
    # local retval=`$curl_command -X POST "$gateway_api" --data "name=$name" --data "sw_version='$sw_version'" --data "hw_version='$hw_version'" --data "bssids=[$bssids]" --data "ssids=[$ssids]"`
    local retval=`$curl_command -X POST "$gateway_api" --data "bssids=[$bssids]" --data "ssids=[$ssids]"`

    if [ $? -eq 0 ]
    then
        local is_success=`echo "$retval" | jq -r -M .success`
        if [ "$is_success" != "true" ]
        then
            echo "FBWIFI: update gateway info error" > /dev/console
            # ezmreport -c 1057004992 -t fbwifi "[info] $retval" &
            return 1
        else
            return 0
        fi
    else
        echo "FBWIFI: network error, cannot update gateway info error" > /dev/console
        return 1
    fi
}

fbwifi_init() {
    local config_name=$1
    local profile_name=$(fbwifi_get_profile_name $config_name)

    local fbwifi_conf_dir=$fbwifi_tmp_dir/$config_name/$fbwifi_conf_path
    local fbwifi_tokens_dir=$fbwifi_tmp_dir/$config_name/$fbwifi_tokens_path
    local fbwifi_ssl_dir=$fbwifi_tmp_dir/$config_name/$fbwifi_ssl_path

    mkdir -p $fbwifi_conf_dir
    mkdir -p $fbwifi_tokens_dir
    mkdir -p $fbwifi_ssl_dir

    local gateway_token=$(fbwifi_get_gateway_token $config_name)

    fbwifi_info_update $config_name

    local config=`$curl_command -X GET "https://api.fbwifi.com/v2.0/gateway?fields=config,config_version&access_token=$gateway_token"`
    if [ $? -eq 0 -a -n "$config" ]
    then
        local is_error=`echo "$config" |jq -r -M "keys[]"`
        if [ "$is_error" = "error" ]
        then
            echo "FBWIFI: got error message, add crontab" > /dev/console
            # ezmreport -c 1057004992 -t fbwifi "[config] $config" &
            chilli_retry_crontab add $config_name
            return 1
        else
            chilli_retry_crontab rem $config_name
            # write config to file
            echo "$config" > $(fbwifi_get_config_file $config_name)

            fbwifi_polling_interval=`echo "$config" |jq -r -M ".config.polling_interval"`
            if [ -z "$fbwifi_polling_interval" ]
            then
                fbwifi_polling_interval=300
            else
                if [ $fbwifi_polling_interval -gt 600 ]
                then
                    fbwifi_polling_interval=600
                elif [ $fbwifi_polling_interval -lt 10 ]
                then
                    fbwifi_polling_interval=10
                fi
            fi

            fbwifi_host_redirect_ips=`echo "$config" |jq -r -M ".config.host_redirect_ips[]"`
            local redir_ips_file_name=$(fbwifi_get_redir_ips_file $config_name)
            echo "$fbwifi_host_redirect_ips" > $redir_ips_file_name

            # NOTE: ipup.sh will call fbwifi-garden.sh to add dynamic wallgarden.
            # fbwifi_wallgarden=`echo "$config" |jq -r -M ".config.traffic_allowlist|.[].ip"`
            fbwifi_ssl_cert=`echo "$config" |jq -r -M ".config.https_server_cert"`
            echo "$fbwifi_ssl_cert" > $fbwifi_ssl_dir/fbwifi-$profile_name-cert.pem

            fbwifi_ssl_key=`echo "$config" |jq -r -M ".config.https_server_key"`
            echo "$fbwifi_ssl_key" > $fbwifi_ssl_dir/fbwifi-$profile_name-key.pem

            fbwifi_config_version=`echo "$config" |jq -r -M ".config_version"`
            echo "$fbwifi_config_version" > $(fbwifi_get_config_version_file $config_name)

            fbwifi_id=`echo "$config" |jq -r -M ".id"`
            fbwifi_cross_origin_allowlist=`echo "$config" |jq -r -M ".config.cross_origin_allowlist[]"`
            echo "$fbwifi_cross_origin_allowlist" > $(fbwifi_get_cross_origin_allowlist_file $config_name)

            fbwifi_captive_portal_url=`echo "$config" |jq -r -M ".config.urls.captive_portal_url"`
            fbwifi_landing_page_url=`echo "$config" |jq -r -M ".config.urls.landing_page_url"`
            fbwifi_captive_portal_config_url=`echo "$config" |jq -r -M ".config.urls.captive_portal_config_url"`
            fbwifi_capport_api_url=`echo "$config" |jq -r -M ".config.urls.capport_api_url"`
            fbwifi_capport_venue_info_url=`echo "$config" |jq -r -M ".config.urls.capport_venue_info_url"`
            local url_file_name=$(fbwifi_get_urls_file $config_name)
            echo "fbwifi_captive_portal_url=\"$fbwifi_captive_portal_url\"" > $url_file_name
            echo "fbwifi_landing_page_url=\"$fbwifi_landing_page_url\"" >> $url_file_name
            echo "fbwifi_captive_portal_config_url=\"$fbwifi_captive_portal_config_url\"" >> $url_file_name
            echo "fbwifi_capport_api_url=\"$fbwifi_capport_api_url\"" >> $url_file_name
            echo "fbwifi_capport_venue_info_url=\"$fbwifi_capport_venue_info_url\"" >> $url_file_name

            return 0
        fi
    else
        echo "FBWIFI: curl init error, add crontab" > /dev/console
        chilli_retry_crontab add $config_name
        return 1
    fi
}

fbwifi_get_config_file() {
    local config_name=$1
    local profile_name=$(fbwifi_get_profile_name $config_name)
    echo "$fbwifi_tmp_dir/$config_name/$fbwifi_conf_path/$profile_name.config"
}

fbwifi_get_config_version_file() {
    local config_name=$1
    local profile_name=$(fbwifi_get_profile_name $config_name)
    echo "$fbwifi_tmp_dir/$config_name/$fbwifi_conf_path/$profile_name-config-version"
}

fbwifi_get_cross_origin_allowlist_file() {
    local config_name=$1
    local profile_name=$(fbwifi_get_profile_name $config_name)
    echo "$fbwifi_tmp_dir/$config_name/$fbwifi_conf_path/$profile_name-cross-origin"
}

fbwifi_get_urls_file() {
    local config_name=$1
    local profile_name=$(fbwifi_get_profile_name $config_name)
    echo "$fbwifi_tmp_dir/$config_name/$fbwifi_conf_path/$profile_name-urls"
}

fbwifi_get_config_version() {
    local config_name=$1
    local config_version=`cat $(fbwifi_get_config_version_file $config_name) 2>/dev/null`
    echo $config_version
}

fbwifi_get_redir_ips_file() {
    local config_name=$1
    local profile_name=$(fbwifi_get_profile_name $config_name)
    echo "$fbwifi_tmp_dir/$config_name/$fbwifi_conf_path/$profile_name-redir-ips"
}

fbwifi_get_cross_origin_allowlist() {
    # https://www.facebook.com, https://m.facebook.com, https://mtouch.facebook.com, https://mbasic.facebook.com, https://www.instagram.com
    local allow_list=""
    for allow_line in `cat $(fbwifi_get_cross_origin_allowlist_file $1)`; do
        allow_list="$allow_list${allow_list:+, }${allow_line}"
    done
    echo "${allow_list:-*}"
}

fbwifi_header_add_allow_origin() {
    local config_name="$1"
    local origin_path="${2// /}"
    origin_path="${origin_path//,/ }"

    if [ -n "$config_name" -a -n "$origin_path" ]
    then
        local match_origin=$(cat $(fbwifi_get_cross_origin_allowlist_file $1) 2>/dev/null |grep -w "$origin_path")
        if [ -n "$match_origin" ]
        then
            printf "Access-Control-Allow-Origin: $match_origin\r\n"
            printf "Vary: Origin\r\n"
        fi
    fi
}

fbwifi_get_all_config_name() {
    local checkstring='^HS_LOGIN_TYPE=108$'
    local config_name=""

    for c in `ls /tmp/etc/chilli/`
    do
        if [ "`cat /tmp/etc/chilli/$c/config |grep "$checkstring"`" != "" ]
        then
            config_name="$config_name $c"
        fi
    done
    echo $config_name
}


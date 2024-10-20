#
# Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
#
# All Rights Reserved.
# Confidential and Proprietary - Qualcomm Technologies, Inc.
#

#
# Copyright (c) 2015, The Linux Foundation. All rights reserved.
#

wps_possible=
config_methods=

### senao add ###
## senao band-steering 11v+11r ##
get_ssid_num_2() {
    local input=$1
    local ssidnum=${input#ath[0-9]}
    echo ${ssidnum}
}

get_ifname_radio_2() {
    local ifname=$1
    local radio=${ifname#ath}
    case "${radio:0:1}" in
        0)
            echo 0
            ;;
        1)
            echo 1
            ;;
        4)
            echo 2
            ;;
        *)
            echo 0
            ;;
    esac
} 

hostapd_get_ap2_macaddr()
{
    local radio_num=$(get_ifname_radio_2 $1)
    local ssid_num=$(get_ssid_num_2 $1)

    # revert radio
    if [ "$radio_num" -lt 2 ]; then
        radio_num=$((radio_num^1))
    else
        radio_num=0
    fi

    local ap2_mac=$(cat /tmp/wifi_all_mac.txt | grep "ath$radio_num$ssid_num " | awk -F ' ' '{print $2}')
    #echo -e "\033[45;37m [$1] --> [ath$radio_num$ssid_num]:[$ap2_mac]\033[0m" >/dev/console
    echo ${ap2_mac}
}

hostapd_get_ap3_macaddr()
{
    local radio_num=$(get_ifname_radio_2 $1)
    local ssid_num=$(get_ssid_num_2 $1)

    # revert radio
    if [ "$radio_num" -lt 2 ]; then
        radio_num=4
    else
        radio_num=1
    fi

    local ap3_mac=$(cat /tmp/wifi_all_mac.txt | grep "ath$radio_num$ssid_num " | awk -F ' ' '{print $2}')
    #echo -e "\033[45;37m [$1] --> [ath$radio_num$ssid_num]:[$ap3_mac]\033[0m" >/dev/console
    echo ${ap3_mac}
}
### senao add end ###

hostapd_set_extra_cred() {
	local var="$1"
	local vif="$2"
	local ifname="$3"
	local temp
	local enc enc_list

	config_get ssid "$vif" ssid
	config_get enc "$vif" encryption "none"

	#wps_build_cred_network_idx
	append "$var" "1026"
	append "$var" "0001"
	append "$var" "01"

	temp=`expr length "$ssid"`
	temp=` printf "%04X" $temp`

	#wps_build_cred_ssid
	append "$var" "1045"
	append "$var"   "$temp"
	temp=`echo -n "$ssid" | hexdump -v -e '/1 "%02X "'`
	append "$var" "$temp"

	#wps_build_cred_auth_type
	append "$var" "1003"
	append "$var" "0002"

	case "$enc" in
		none)
			append "$var" "0001"
			;;
		# Need ccmp*|gcmp* check for SAE and OWE auth type
		wpa2*|*psk2*|ccmp*|gcmp*)
			append "$var" "0020"
			;;
		*)
			# TKIP alone is now prohibited by WFA so the only
			# combination left must be CCMP+TKIP (wpa=3)
			append "$var" "0022"
			;;
	esac

	#wps_build_cred_encr_type
	append "$var" "100f"
	append "$var" "0002"
	crypto=

	enc_list=`echo "$enc" | sed "s/+/ /g"`

	case "$enc_list" in
		*tkip*)
			append "$var" "0004"
			;;
		*aes* | *ccmp*)
			append "$var" "0008"
			;;
		*mixed*)
			append "$var" "000c"
			;;
	esac


	#Key Index
	append "$var" "1028"
	append "$var" "0001"
	append "$var" "01"

	#wps_build_cred_network_key
	config_get psk "$vif" key
	append "$var" "1027"

	temp=`expr length "$psk"`
	temp=` printf "%04X" $temp`

	append "$var" "$temp"
	temp=`echo -n  $psk | hexdump -v -e '/1 "%02X "'`
	append "$var" "$temp"

	#wps_build_mac_addr
	macaddr=$(cat /sys/class/net/${ifname}/address)
        macaddr="00:00:00:00:00:00"
	append "$var" "1020"
	append "$var" "0006"
	append "$var" "$macaddr"
}
hostapd_common_add_device_config() {
	config_add_array basic_rate

	config_add_string country
	config_add_boolean country_ie doth
	config_add_int beacon_int
}


hostapd_prepare_device_config() {
        local config="$1"
        local driver="$2"

        local base="${config%%.conf}"
        local base_cfg=

        json_get_vars country country_ie beacon_int doth

        hostapd_set_log_options base_cfg

        set_default country_ie 1
        set_default doth 1

        [ -n "$country" ] && {
                append base_cfg "country_code=$country" "$N"

                [ "$country_ie" -gt 0 ] && append base_cfg "ieee80211d=1" "$N"
                [ "$hwmode" = "a" -a "$doth" -gt 0 ] && append base_cfg "ieee80211h=1" "$N"
        }
        [ -n "$hwmode" ] && append base_cfg "hw_mode=$hwmode" "$N"

        local brlist= br
        json_get_values basic_rate_list basic_rate
        for br in $basic_rate_list; do
                hostapd_add_basic_rate brlist "$br"
        done
        [ -n "$brlist" ] && append base_cfg "basic_rates=$brlist" "$N"
        [ -n "$beacon_int" ] && append base_cfg "beacon_int=$beacon_int" "$N"

        cat > "$config" <<EOF
driver=$driver
$base_cfg
EOF
}

hostapd_eap_config_parameters() {
	local var="$1"
	local vif="$2"
	local ssidprofile_id
	local ifname
	local iface_name
	local ssid_index
	local is_ezm_radius
	local sn_model_name

	config_get auth_server "$vif" auth_server
	config_get auth_port "$vif" auth_port
	[ -z "$auth_port" ] && auth_port=${auth_port:-1812}
	config_get auth_secret "$vif" auth_secret

	[ -n "$auth_server" -a -n "$auth_port" -a -n "$auth_secret" ] && {
		append "$var" "auth_server_addr=$auth_server" "$N"
		append "$var" "auth_server_port=$auth_port" "$N"
		append "$var" "auth_server_shared_secret=$auth_secret" "$N"
	}

	config_get acct_enabled "$vif" acct_enabled
	if [ "$acct_enabled" == "1" ]; then
	{
		# Radius Accounting Server
		config_get acct_server "$vif" acct_server
		[ $(isip ${acct_server}) == 0 ] && acct_server=""
		# Radius Accounting Port
		config_get acct_port "$vif" acct_port
		[ -z "$acct_port" ] && acct_port=${acct_port:-1813}
		# Radius Accounting Secret
		config_get acct_secret "$vif" acct_secret
		# Interim Accounting Interval
		config_get acct_interval "$vif" acct_interval
		[ -z "$acct_interval" ] && acct_interval=600

		[ -n "$acct_server" -a -n "$acct_port" -a -n "$acct_secret" -a -n "$acct_interval" ] && {
			append "$var" "acct_server_addr=$acct_server" "$N"
			append "$var" "acct_server_port=$acct_port" "$N"
			append "$var" "acct_server_shared_secret=$acct_secret" "$N"
			append "$var" "radius_acct_interim_interval=$acct_interval" "$N"
		} || echo "Debug message: Accounting setting failed." > /dev/console
	}
	fi

	config_get is_ezm_radius "$vif" is_ezm_radius
	if [ "$is_ezm_radius" == "1" ]
	then
		config_get device "$vif" device
		config_get network_id "$device" networkId
		append "$var" "network_id=$network_id" "$N"
		config_get ifname "$vif" ifname
		iface_name=$(foreach wireless wifi-iface ifname $ifname 2>/dev/null)
		ssid_index=$(echo $iface_name | awk -F '_ssid_' '{print $2}')
		ssidprofile_id=$(foreach cloud_mapping ap dut_config_index $ssid_index 2>/dev/null)
		append "$var" "ssidprofile_id=$ssidprofile_id" "$N"
		sn_model_name=$(uci get sysProductInfo.model.modelName)
		append "$var" "sn_model_name=$sn_model_name" "$N"
	else
		# fallback RADIUS servers. For now support secondary RADIUS server and Accounting server
		config_get secondary_auth_server "$vif" secondary_auth_server
		config_get secondary_auth_port "$vif" secondary_auth_port
		[ -z "$secondary_auth_port" ] && secondary_auth_port=${secondary_auth_port:-1812}
		config_get secondary_auth_secret "$vif" secondary_auth_secret

		[ -n "$secondary_auth_server" -a -n "$secondary_auth_port" -a -n "$secondary_auth_secret" ] && {
			append "$var" "auth_server_addr=$secondary_auth_server" "$N"
			append "$var" "auth_server_port=$secondary_auth_port" "$N"
			append "$var" "auth_server_shared_secret=$secondary_auth_secret" "$N"
		}

		[  -n "$secondary_auth_server" -a -n "$secondary_auth_secret" ] && {
			local radius_retry_primary_interval=600
			[ -n "$radius_retry_primary_interval" ] && append "$var" "radius_retry_primary_interval=$radius_retry_primary_interval" "$N"
		}
	fi

	config_get acct_enabled "$vif" acct_enabled
	if [ "$acct_enabled" == "1" ]; then
	{
		if [ "$is_ezm_radius" != "1" ]; then
		{
			# fallback Radius Accounting Server
			config_get secondary_acct_server "$vif" secondary_acct_server
			[ $(isip ${secondary_acct_server}) == 0 ] && secondary_acct_server=""
			# fallback Radius Accounting Port
			config_get secondary_acct_port "$vif" secondary_acct_port
			[ -z "$secondary_acct_port" ] && secondary_acct_port=${secondary_acct_port:-1813}
			# fallback Radius Accounting Secret
			config_get secondary_acct_secret "$vif" secondary_acct_secret

			[ -n "$secondary_acct_server" -a -n "$secondary_acct_port" -a -n "$secondary_acct_secret" ] && {
				append "$var" "acct_server_addr=$secondary_acct_server" "$N"
				append "$var" "acct_server_port=$secondary_acct_port" "$N"
				append "$var" "acct_server_shared_secret=$secondary_acct_secret" "$N"
			} || echo "Debug message: Secondary accounting setting failed." > /dev/console
		}
		fi
	}
	fi

	config_get eap_reauth_period "$vif" eap_reauth_period
	[ -n "$eap_reauth_period" ] && append "$var" "eap_reauth_period=$eap_reauth_period" "$N"
	config_get wep_key_len_broadcast "$vif" wep_key_len_broadcast
	config_get wep_key_len_unicast "$vif" wep_key_len_unicast
	append "$var" "eapol_key_index_workaround=1" "$N"
	append "$var" "ieee8021x=1" "$N"
	config_get identity_request_retry_interval "$vif" identity_request_retry_interval
	[ -n "$identity_request_retry_interval" ] && append "$var" "identity_request_retry_interval=$identity_request_retry_interval" "$N"
	config_get radius_server_retries "$vif" radius_server_retries
	[ -n "$radius_server_retries" ] && append "$var" "radius_server_retries=$radius_server_retries" "$N"
	config_get radius_max_retry_wait "$vif" radius_max_retry_wait
	[ -n "$radius_max_retry_wait" ] && append "$var" "radius_max_retry_wait=$radius_max_retry_wait" "$N"
	[ -n "$wpa_group_rekey"  ] && append "$var" "wpa_group_rekey=$wpa_group_rekey" "$N"
	[ -n "$wpa_strict_rekey"  ] && append "$var" "wpa_strict_rekey=$wpa_strict_rekey" "$N"
	[ -n "$wpa_pair_rekey"   ] && append "$var" "wpa_ptk_rekey=$wpa_pair_rekey"    "$N"
	[ -n "$wpa_master_rekey" ] && append "$var" "wpa_gmk_rekey=$wpa_master_rekey"  "$N"
	[ -n "$wep_key_len_broadcast" ] && append "$var" "wep_key_len_broadcast=$wep_key_len_broadcast" "$N"
	[ -n "$wep_key_len_unicast" ] && append "$var" "wep_key_len_unicast=$wep_key_len_unicast" "$N"
	[ -n "$wep_rekey" ] && append "$var" "wep_rekey_period=$wep_rekey" "$N"

	config_get wpa_group_update_count "$vif" wpa_group_update_count
	[ -n "$wpa_group_update_count" ] && append "$var" "wpa_group_update_count=$wpa_group_update_count" "$N"

	config_get wpa_pairwise_update_count "$vif" wpa_pairwise_update_count
	[ -n "$wpa_pairwise_update_count" ] && append "$var" "wpa_pairwise_update_count=$wpa_pairwise_update_count" "$N"

	config_get wpa_disable_eapol_key_retries "$vif" wpa_disable_eapol_key_retries
	[ -n "$wpa_disable_eapol_key_retries" ] && append "$var" "wpa_disable_eapol_key_retries=$wpa_disable_eapol_key_retries" "$N"
}

hostapd_set_bss_options() {
	local var="$1"
	local vif="$2"
	local hidden
	local auth_algs
	local enc wep_rekey wpa_group_rekey wpa_strict_rekey wpa_pair_rekey wpa_master_rekey pid sae owe suite_b
	local add_sha256_str ieee80211r_str enc_list ieee80211ai_sha256_str ieee80211ai_sha384_str sae_str owe_str suite_b_str
	local owe_transition_bssid owe_transition_ssid owe_transition_ifname owe_groups
	local sae_reflection_attack sae_commit_override sae_password sae_anti_clogging_threshold sae_groups sae_sync sae_require_mfp
	local iface_name
	local ssid_index

	[ -z "$vif" ] && hostapd_get_vif_name

	config_load wireless

	config_get enc "$vif" encryption "none"
	config_get wep_rekey        "$vif" wep_rekey        # 300
	config_get wpa_group_rekey  "$vif" wpa_group_rekey  # 300
	config_get wpa_strict_rekey  "$vif" wpa_strict_rekey  # 300
	config_get wpa_pair_rekey   "$vif" wpa_pair_rekey   # 300
	config_get wpa_master_rekey "$vif" wpa_master_rekey # 640
	config_get_bool ap_isolate "$vif" isolate 0
	#config_get_bool ieee80211r "$vif" ieee80211r 0
	config_get_bool ieee80211ai "$vif" ieee80211ai 0
	config_get_bool hidden "$vif" hidden 0
	config_get kh_key_hex "$vif" kh_key_hex "000102030405060708090a0b0c0d0e0f"
	config_get_bool sae "$vif" sae 0
	config_get_bool owe "$vif" owe 0
	config_get suite_b "$vif" suite_b 0
	
	### SENAO ### read fastroaming
	config_get ieee80211r "$vif" fastroamingEnable

	config_get  sae_reflection_attack  "$vif" sae_reflection_attack
	config_get  sae_commit_override  "$vif" sae_commit_override
	config_get  sae_password  "$vif" sae_password
	config_get  sae_anti_clogging_threshold   "$vif" sae_anti_clogging_threshold

	config_get  owe_transition_bssid   "$vif" owe_transition_bssid
	[ -n "$owe_transition_bssid" ] && append "$var" "owe_transition_bssid=$owe_transition_bssid" "$N"
	config_get  owe_transition_ssid   "$vif" owe_transition_ssid
	[ -n "$owe_transition_ssid" ] && append "$var" "owe_transition_ssid=\"$owe_transition_ssid\"" "$N"
	config_get  owe_transition_ifname "$vif" owe_transition_ifname
	[ -n "$owe_transition_ifname" ] && append "$var" "owe_transition_ifname=$owe_transition_ifname" "$N"

	config_get own_ie_override "$vif" own_ie_override
	[ -n "$own_ie_override" ] && append "$var" "own_ie_override=$own_ie_override" "$N"

	config_get device "$vif" device
	config_get hwmode "$device" hwmode
	config_get phy "$device" phy

	[ -f /var/run/hostapd-$phy/$ifname ] && rm /var/run/hostapd-$phy/$ifname
	ctrl_interface=/var/run/hostapd-$phy

	append "$var" "ctrl_interface=$ctrl_interface" "$N"

	if [ "$ap_isolate" -gt 0 ]; then
		append "$var" "ap_isolate=$ap_isolate" "$N"
	fi

	# SENAO add nobeaconup feature
	if [ "$driver" != "nl80211" ]; then
		if [ $ignore_broadcast_ssid -gt 0 ]
		then
			append "$var" "ignore_broadcast_ssid=$ignore_broadcast_ssid" "$N"
		fi
	fi

	append "$var" "send_probe_response=0" "$N"

	# Examples:
	# psk-mixed/tkip 	=> WPA1+2 PSK, TKIP
	# wpa-psk2/tkip+aes	=> WPA2 PSK, CCMP+TKIP
	# wpa2/tkip+aes 	=> WPA2 RADIUS, CCMP+TKIP
	# ...

	# TODO: move this parsing function somewhere generic, so that
	# later it can be reused by drivers that don't use hostapd

	# crypto defaults: WPA2 vs WPA1

	# If suite_b is set then hard code
	# wpa as 2
	# set ieee80211w as 2
	# set group_mgmt_cipher as BIP-GMAC-256
	# set pairwise as GCMP-256

	if [ "${suite_b}" -eq 192 ]
	then
		wpa=2
		config_set "$vif" ieee80211w 2
		config_set "$vif" group_mgmt_cipher "BIP-GMAC-256"
		hostapd_eap_config_parameters "$var" "$vif"
		append "$var" "wpa_pairwise=GCMP-256" "$N"
	else
		case "$enc" in
			none)
				wpa=0
			;;
			# Need ccmp*|gcmp* check for SAE and OWE auth type
			wpa2*|*psk2*|ccmp*|gcmp*)
				wpa=2
			;;
			*)
				# TKIP alone is now prohibited by WFA so the only
				# combination left must be CCMP+TKIP (wpa=3)
				wpa=3
			;;
		esac

		crypto=
		enc_list=`echo "$enc" | sed "s/+/ /g"`

		for enc_var in $enc_list; do
			case "$enc_var" in
				*tkip)
					crypto="TKIP $crypto"
				;;
				#This case is added only to act as a testbed AP for 11ax WFA PF
				*tkip-pure)
					crypto="PURETKIP $crypto"
				;;
				*aes)
					crypto="CCMP $crypto"
				;;
				*ccmp)
					crypto="CCMP $crypto"
				;;
				*ccmp-256)
					crypto="CCMP-256 $crypto"
				;;
				*gcmp)
					crypto="GCMP $crypto"
				;;
				*gcmp-256)
					crypto="GCMP-256 $crypto"
			esac
		done

		case "$enc_list" in
			psk | wpa)
				crypto="TKIP"
			;;
			psk2 | wpa2)
				crypto="CCMP"
			;;
			*mixed*)
				[ -z "$crypto" ] && crypto="CCMP TKIP"
			;;
		esac

		# WPA TKIP alone is no longer allowed for certification
		case "$hwmode:$crypto" in
			*:TKIP*) crypto="CCMP TKIP";;
		esac

		# This case is added only to act as a testbed AP for 11ax WFA PF
		# 11ax WFA PF requires WPA TKIP alone for testbed AP
		case "$hwmode:$crypto" in
			*:PURETKIP*) crypto="TKIP";;
		esac

		# radius Mac-based authentication
		config_get radius_macauth_enable "$vif" radius_macauth_enable 0

		# use crypto/auth settings for building the hostapd config
		case "$enc" in
			none)
				wps_possible=1
				# Here we make the assumption that if we're in open mode
				# with WPS enabled, we got to be in unconfigured state.
				wps_configured_state=1

				config_get acct_enabled "$vif" acct_enabled
				if [ "$acct_enabled" == "1" ]; then
				{
					# Radius Accounting Server
					config_get acct_server "$vif" acct_server
					[ $(isip ${acct_server}) == 0 ] && acct_server=""
					# Radius Accounting Port
					config_get acct_port "$vif" acct_port
					[ -z "$acct_port" ] && acct_port=${acct_port:-1813}
					# Radius Accounting Secret
					config_get acct_secret "$vif" acct_secret
					# Interim Accounting Interval
					config_get acct_interval "$vif" acct_interval
					[ -z "$acct_interval" ] && acct_interval=600

					[ -n "$acct_server" -a -n "$acct_port" -a -n "$acct_secret" -a -n "$acct_interval" ] && {
						append "$var" "acct_server_addr=$acct_server" "$N"
						append "$var" "acct_server_port=$acct_port" "$N"
						append "$var" "acct_server_shared_secret=$acct_secret" "$N"
						append "$var" "radius_acct_interim_interval=$acct_interval" "$N"
					} || echo "Debug message: Accounting setting failed." > /dev/console
				}
				fi
			;;
			# Need ccmp*|gcmp* check for SAE and OWE auth type
			*psk*|ccmp*|gcmp*)
				# radius Mac-based authentication supports iPSK
				if [ "$radius_macauth_enable" == "0" ]; then
				{
					config_get psk "$vif" key
					config_get wpa_psk_file "$vif" wpa_psk_file
					
					#2020.6.17 Senao George
					#mpsk vlan on/off switch
					config_get multi_group_key "$vif" multi_group_key
					
					if [ ${#psk} -eq 64 ]; then
						append "$var" "wpa_psk=$psk" "$N"
					else
						#2020.11.23 Senao George, psk personel and psk file should not both exist
						[ -n "$psk" ] && [ "$multi_group_key" != "1" ] && append "$var" "wpa_passphrase=$psk" "$N"
						#[ -n "$wpa_psk_file" ] && append "$var" "wpa_psk_file=$wpa_psk_file"  "$N"
						
						#2020.06.17 Senao George, for mpsk vlan implementation
						#2020.11.23 Senao George, psk personel and psk file should not both exist
						#echo ">>>>>[debug][ifname:$ifname] multi_group_key:$multi_group_key wpa_psk_file:$wpa_psk_file " > /dev/console
						if [ "$multi_group_key" == "1" ] && [ -n "$wpa_psk_file" ]; then
						{
							#ex:hostapd-ath0.wpa_psk
							append "$var" "wpa_psk_file=/var/run/hostapd-$ifname.wpa_psk"  "$N"
							[ -f "/var/run/hostapd-$ifname.wpa_psk" ] && rm -f /var/run/hostapd-$ifname.wpa_psk
							ln -sf $wpa_psk_file /var/run/hostapd-$ifname.wpa_psk
						}
						else
							[ -f "/var/run/hostapd-$ifname.wpa_psk" ] && rm -f /var/run/hostapd-$ifname.wpa_psk
						fi
					fi
				}
				fi

				wps_possible=1
				# By default we assume we are in configured state,
				# while the user has the provision to override this.
				wps_configured_state=2
				[ -n "$wpa_group_rekey"  ] && append "$var" "wpa_group_rekey=$wpa_group_rekey" "$N"
				[ -n "$wpa_strict_rekey"  ] && append "$var" "wpa_strict_rekey=$wpa_strict_rekey" "$N"
				[ -n "$wpa_pair_rekey"   ] && append "$var" "wpa_ptk_rekey=$wpa_pair_rekey"    "$N"
				[ -n "$wpa_master_rekey" ] && append "$var" "wpa_gmk_rekey=$wpa_master_rekey"  "$N"

				config_get wpa_group_update_count "$vif" wpa_group_update_count
				[ -n "$wpa_group_update_count" ] && append "$var" "wpa_group_update_count=$wpa_group_update_count" "$N"

				config_get wpa_pairwise_update_count "$vif" wpa_pairwise_update_count
				[ -n "$wpa_pairwise_update_count" ] && append "$var" "wpa_pairwise_update_count=$wpa_pairwise_update_count" "$N"

				config_get wpa_disable_eapol_key_retries "$vif" wpa_disable_eapol_key_retries
				[ -n "$wpa_disable_eapol_key_retries" ] && append "$var" "wpa_disable_eapol_key_retries=$wpa_disable_eapol_key_retries" "$N"

				config_get acct_enabled "$vif" acct_enabled
				if [ "$acct_enabled" == "1" ]; then
				{
					# Radius Accounting Server
					config_get acct_server "$vif" acct_server
					[ $(isip ${acct_server}) == 0 ] && acct_server=""
					# Radius Accounting Port
					config_get acct_port "$vif" acct_port
					[ -z "$acct_port" ] && acct_port=${acct_port:-1813}
					# Radius Accounting Secret
					config_get acct_secret "$vif" acct_secret
					# Interim Accounting Interval
					config_get acct_interval "$vif" acct_interval
					[ -z "$acct_interval" ] && acct_interval=600

					[ -n "$acct_server" -a -n "$acct_port" -a -n "$acct_secret" -a -n "$acct_interval" ] && {
						append "$var" "acct_server_addr=$acct_server" "$N"
						append "$var" "acct_server_port=$acct_port" "$N"
						append "$var" "acct_server_shared_secret=$acct_secret" "$N"
						append "$var" "radius_acct_interim_interval=$acct_interval" "$N"
					} || echo "Debug message: Accounting setting failed." > /dev/console
				}
				fi

				# radius Mac-based authentication supports iPSK
				if [ "$radius_macauth_enable" == "1" ]; then
				{
					append "$var" "wpa_psk_radius=2" "$N"
				}
				fi
			;;
			*wpa*)
				hostapd_eap_config_parameters "$var" "$vif"
			;;
			*wep*)
				config_get key "$vif" key_id
				key="${key:-1}"
				case "$key" in
					[1234])
						for idx in 1 2 3 4; do
							local zidx
							zidx=$(($idx - 1))
							config_get ckey "$vif" "key${idx}"
							[ -n "$ckey" ] && \
								append "$var" "wep_key${zidx}=$(prepare_key_wep "$ckey")" "$N"
						done
						append "$var" "wep_default_key=$((key - 1))"  "$N"
					;;
					*)
						append "$var" "wep_key0=$(prepare_key_wep "$key")" "$N"
						append "$var" "wep_default_key=0" "$N"
						[ -n "$wep_rekey" ] && append "$var" "wep_rekey_period=$wep_rekey" "$N"
					;;
				esac

				config_get acct_enabled "$vif" acct_enabled
				if [ "$acct_enabled" == "1" ]; then
				{
					# Radius Accounting Server
					config_get acct_server "$vif" acct_server
					[ $(isip ${acct_server}) == 0 ] && acct_server=""
					# Radius Accounting Port
					config_get acct_port "$vif" acct_port
					[ -z "$acct_port" ] && acct_port=${acct_port:-1813}
					# Radius Accounting Secret
					config_get acct_secret "$vif" acct_secret
					# Interim Accounting Interval
					config_get acct_interval "$vif" acct_interval
					[ -z "$acct_interval" ] && acct_interval=600

					[ -n "$acct_server" -a -n "$acct_port" -a -n "$acct_secret" -a -n "$acct_interval" ] && {
						append "$var" "acct_server_addr=$acct_server" "$N"
						append "$var" "acct_server_port=$acct_port" "$N"
						append "$var" "acct_server_shared_secret=$acct_secret" "$N"
						append "$var" "radius_acct_interim_interval=$acct_interval" "$N"
					} || echo "Debug message: Accounting setting failed." > /dev/console
				}
				fi

				case "$enc" in
					*shared*)
						auth_algs=2
					;;
					*mixed*)
						auth_algs=3
					;;
				esac
				wpa=0
				crypto=
			;;
			8021x)
				# For Dynamic WEP 802.1x,maybe need more fields
				config_get auth_server "$vif" auth_server
				[ -z "$auth_server" ] && config_get auth_server "$vif" server
				append "$var" "auth_server_addr=$auth_server" "$N"
				config_get auth_port "$vif" auth_port
				[ -z "$auth_port" ] && config_get auth_port "$vif" port
				auth_port=${auth_port:-1812}
				append "$var" "auth_server_port=$auth_port" "$N"
				config_get auth_secret "$vif" auth_secret
				[ -z "$auth_secret" ] && config_get auth_secret "$vif" key
				config_get eap_reauth_period "$vif" eap_reauth_period
				[ -n "$eap_reauth_period" ] && append "$var" "eap_reauth_period=$eap_reauth_period" "$N"
				config_get wep_rekey "$vif" wep_rekey 300

				append "$var" "ieee8021x=1" "$N"
				append "$var" "auth_server_shared_secret=$auth_secret" "$N"
				append "$var" "wep_rekey_period=$wep_rekey" "$N"
				append "$var" "eap_server=0" "$N"
				append "$var" "eapol_version=2" "$N"
				append "$var" "eapol_key_index_workaround=0" "$N"
				append "$var" "wep_key_len_broadcast=13" "$N"
				append "$var" "wep_key_len_unicast=13" "$N"
				auth_algs=1
				wpa=0
				crypto=
			;;
			*)
				wpa=0
				crypto=
			;;
		esac
	#termination of suite_b enable or not check
	fi

	# Mac-based authentication using external radius server
	if [ "$radius_macauth_enable" == "1" ]; then
	{
		append "$var" "macaddr_acl=2" "$N"

		case "$enc" in
		*wpa*);;
		*8021x*);;
		*)
			config_get auth_server "$vif" auth_server
			[ -z "$auth_server" ] && config_get auth_server "$vif" server
			append "$var" "auth_server_addr=$auth_server" "$N"
			config_get auth_port "$vif" auth_port
			[ -z "$auth_port" ] && config_get auth_port "$vif" port
			auth_port=${auth_port:-1812}
			append "$var" "auth_server_port=$auth_port" "$N"
			config_get auth_secret "$vif" auth_secret
			[ -z "$auth_secret" ] && config_get auth_secret "$vif" key
			append "$var" "auth_server_shared_secret=$auth_secret" "$N"
		esac
	}
	fi

	append "$var" "auth_algs=${auth_algs:-1}" "$N"
	append "$var" "wpa=$wpa" "$N"

	if [ "${suite_b}" -ne 192 ]
	then
		[ -n "$crypto" ] && append "$var" "wpa_pairwise=$crypto" "$N"
	fi

	[ -n "$wpa_group_rekey" ] && append "$var" "wpa_group_rekey=$wpa_group_rekey" "$N"
	[ -n "$wpa_strict_rekey" ] && append "$var" "wpa_strict_rekey=$wpa_strict_rekey" "$N"

	config_get nasid_enable "$vif" nasid_enable
	config_get nasport_enable "$vif" nasport_enable
	config_get nasip_enable "$vif" nasip_enable

	if [ "$nasid_enable" == "1" ]; then
		config_get nasid "$vif" nasid
		[ -n "$nasid" ] && append "$var" "nas_identifier=$nasid" "$N"
	fi

	if [ "$nasport_enable" == "1" ]; then
		config_get nasport "$vif" nasport
		[ -n "$nasport" ] && append "$var" "nas_port=$nasport" "$N"
	fi

	if [ "$nasip_enable" == "1" ]; then
		config_get nasip "$vif" nasip
		[ -n "$nasip" ] && append "$var" "own_ip_addr=$nasip" "$N"
	fi

	config_get ssid "$vif" ssid
	config_get bridge "$vif" bridge
	config_get ieee80211d "$vif" ieee80211d
	config_get iapp_interface "$vif" iapp_interface

	config_get_bool wps_pbc "$vif" wps_pbc 0
	config_get_bool wps_label "$vif" wps_label 0

	config_get config_methods "$vif" wps_config
	[ "$wps_pbc" -gt 0 ] && append config_methods push_button

	# WPS 2.0 test case 4.1.7:
	# if we're configured to enable WPS and we hide our SSID, then
	# we have to require an "explicit user operation to continue"
	config_get_bool hidden "$vif" hidden 0
	[ -n "$wps_possible" -a -n "$config_methods" -a "$hidden" -gt 0 ] && {
		echo "Hidden SSID is enabled on \"$ifname\", WPS will be automatically disabled"
		echo "Please press any key to continue."
		#read -s -n 1
		wps_possible=
	}

	[ -n "$wps_possible" -a -n "$config_methods" ] && {
		config_get device_type "$vif" wps_device_type "6-0050F204-1"
		config_get device_name "$vif" wps_device_name "OpenWrt AP"
		config_get manufacturer "$vif" wps_manufacturer "openwrt.org"
		config_get model_name "$vif" model_name "WAP"
		config_get model_number "$vif" model_number "123"
		config_get serial_number "$vif" serial_number "12345"
		config_get wps_pin "$vif" wps_pin
		config_get wps_state "$vif" wps_state $wps_configured_state
		config_get_bool wps_independent "$vif" wps_independent 1

		# SENAO Protect device_name size 32
		device_name=$(echo $device_name | cut -c -32)

		config_get pbc_in_m1 "$vif" pbc_in_m1
		[ -n "$pbc_in_m1" ] && append "$var" "pbc_in_m1=$pbc_in_m1" "$N"

		config_get_bool ext_registrar "$vif" ext_registrar 0
		[ "$ext_registrar" -gt 0 -a -n "$bridge" ] && append "$var" "upnp_iface=$bridge" "$N"

		append "$var" "eap_server=1" "$N"
		append "$var" "ap_pin=$wps_pin" "$N"
		append "$var" "wps_state=$wps_state" "$N"
		append "$var" "ap_setup_locked=0" "$N"
		append "$var" "device_type=$device_type" "$N"
		append "$var" "device_name=$device_name" "$N"
		append "$var" "manufacturer=$manufacturer" "$N"
		append "$var" "model_name=$model_name" "$N"
		append "$var" "model_number=$model_number" "$N"
		append "$var" "serial_number=$serial_number" "$N"
		append "$var" "config_methods=$config_methods" "$N"
		append "$var" "wps_independent=$wps_independent" "$N"

		# fix the overlap session of WPS PBC for dual band AP
		local macaddr=$(cat /sys/class/net/${bridge}/address)
		uuid=$(echo "$macaddr" | sed 's/://g')
		[ -n "$uuid" ] && {
			append "$var" "uuid=87654321-9abc-def0-1234-$uuid" "$N"
		}

	}

	append "$var" "ssid=$ssid" "$N"
	[ -n "$bridge" ] && append "$var" "bridge=$bridge" "$N"
	[ -n "$ieee80211d" ] && append "$var" "ieee80211d=$ieee80211d" "$N"
	[ -n "$iapp_interface" ] && append "$var" iapp_interface=$(uci_get_state network "$iapp_interface" ifname "$iapp_interface") "$N"

	if [ "$wpa" -ge "2" ]
	then
		# RSN -> allow preauthentication
		config_get rsn_preauth "$vif" rsn_preauth
		if [ -n "$bridge" -a "$rsn_preauth" = 1 ]
		then
			append "$var" "rsn_preauth=1" "$N"
			append "$var" "rsn_preauth_interfaces=$bridge" "$N"
		fi

		# RSN -> allow management frame protection

		config_get ieee80211w "$vif" ieee80211w 0

		# Allow SHA256
		case "$enc" in
			*wpa*)  keymgmt=EAP
				key_mgmt_str="WPA-EAP"
			;;
			*psk*)  keymgmt=PSK
				key_mgmt_str="WPA-PSK"
			;;
		esac
		config_get_bool add_sha256 "$vif" add_sha256 0
		config_get_bool add_sha384 "$vif" add_sha384 0

		if [ "${ieee80211w}" -eq 2 ]
		then
			add_sha256=1
		fi

		if [ "${ieee80211r}" -gt 0 -a -n "${keymgmt}" ]
		then
			ieee80211r_str="FT-${keymgmt}"
		fi

		if [ "${sae}" -eq 1 ]
		then
			config_get sae_require_mfp  "$vif" sae_require_mfp

			[ -n "$sae_reflection_attack" ] && append "$var" "sae_reflection_attack=$sae_reflection_attack" "$N"
			[ -n "$sae_commit_override" ] && append "$var" "sae_commit_override=$sae_commit_override" "$N"
			[ -n "$sae_password" ] && append "$var" "sae_password=$sae_password" "$N"
			[ -n "$sae_anti_clogging_threshold" ] && append "$var" "sae_anti_clogging_threshold=$sae_anti_clogging_threshold" "$N"

			local sae_groups="19 20 21"
			[ -n "$sae_groups" ] && append "$var" "sae_groups=$sae_groups" "$N"

			add_sae_groups() {
				local sae_groups=$(echo $1 | tr "," " ")
				[ -n "$sae_groups" ] && append "$var" "sae_groups=$sae_groups" "$N"
			}
			config_list_foreach  "$vif" sae_groups add_sae_groups

			if [ "${ieee80211r}" -gt 0 ]
			then
				sae_str="FT-SAE"
			else
				sae_str="SAE"
			fi
			config_get sae_sync  "$vif" sae_sync
			[ -n "$sae_sync" ] && append "$var" "sae_sync=$sae_sync" "$N"

			case "$enc" in
				*wpa*);;
				*psk*)
					if [ "${ieee80211w}" -eq 0 ]
					then
						ieee80211w=1
						sae_require_mfp=1
					elif [ "${ieee80211w}" -eq 1 ]
					then
						sae_require_mfp=1
					fi
				;;
				*)
					ieee80211w=2
					add_sha256=0
			esac

			[ -n "$sae_require_mfp" ] && append "$var" "sae_require_mfp=$sae_require_mfp" "$N"

		fi

		if [ "${owe}" -eq 1 ]
		then
			owe_str="OWE"

			add_owe_groups() {
				local owe_groups=$(echo $1 | tr "," " ")
				[ -n "$owe_groups" ] && append "$var" "owe_groups=$owe_groups" "$N"
			}
			config_list_foreach  "$vif" owe_groups add_owe_groups

			case "$enc" in
				*wpa*);;
				*psk*);;
				*)
					ieee80211w=2
					add_sha256=0
			esac
		fi

		if [ "${suite_b}" -eq 192 ]
		then
			suite_b_str="WPA-EAP-SUITE-B-192"
		fi

		append "$var" "ieee80211w=$ieee80211w" "$N"
		[ "$ieee80211w" -gt "0" ] && {
			config_get ieee80211w_max_timeout "$vif" ieee80211w_max_timeout
			config_get ieee80211w_retry_timeout "$vif" ieee80211w_retry_timeout
			config_get group_mgmt_cipher "$vif" group_mgmt_cipher
			[ -n "$ieee80211w_max_timeout" ] && \
				append "$var" "assoc_sa_query_max_timeout=$ieee80211w_max_timeout" "$N"
			[ -n "$ieee80211w_retry_timeout" ] && \
				append "$var" "assoc_sa_query_retry_timeout=$ieee80211w_retry_timeout" "$N"
			[ -n "$group_mgmt_cipher" ] && \
				append "$var" "group_mgmt_cipher=$group_mgmt_cipher" "$N"
		}

		[ "${add_sha256}" -gt 0 ] && add_sha256_str="${key_mgmt_str}-SHA256"

	        if [ "${ieee80211ai}" -gt 0 ]
		then
			if [ "${ieee80211r}" -gt 0 ]
			then
				[ "${add_sha256}" -gt 0 ] && ieee80211ai_sha256_str="FT-FILS-SHA256"
				[ "${add_sha384}" -gt 0 ] && ieee80211ai_sha384_str="FT-FILS-SHA384"
			else
				[ "${add_sha256}" -gt 0 ] && ieee80211ai_sha256_str="FILS-SHA256"
				[ "${add_sha384}" -gt 0 ] && ieee80211ai_sha384_str="FILS-SHA384"
			fi
			config_get erp_send_reauth_start "$vif" erp_send_reauth_start
			[ -n "$erp_send_reauth_start" ] && append "$var" "erp_send_reauth_start=$erp_send_reauth_start" "$N"
			config_get erp_domain "$vif" erp_domain
			[ -n "$erp_domain" ] && append "$var" "erp_domain=$erp_domain" "$N"
			config_get fils_realm "$vif" fils_realm
			[ -n "$fils_realm" ] && append "$var" "fils_realm=$fils_realm" "$N"
			config_get fils_cache_id "$vif" fils_cache_id
			[ -n "$fils_cache_id" ] && append "$var" "fils_cache_id=$fils_cache_id" "$N"
			config_get disable_pmksa_caching "$vif" disable_pmksa_caching
			[ -n "$disable_pmksa_caching" ] && append "$var" "disable_pmksa_caching=$disable_pmksa_caching" "$N"
			config_get own_ip_addr "$vif" own_ip_addr
			[ -n "$own_ip_addr" ] && append "$var" "own_ip_addr=$own_ip_addr" "$N"
			config_get dhcp_server "$vif" dhcp_server
			[ -n "$dhcp_server" ] && append "$var" "dhcp_server=$dhcp_server" "$N"
			config_get fils_hlp_wait_time "$vif" fils_hlp_wait_time
			[ -n "$fils_hlp_wait_time" ] && append "$var" "fils_hlp_wait_time=$fils_hlp_wait_time" "$N"
			config_get dhcp_rapid_commit_proxy "$vif" dhcp_rapid_commit_proxy
			[ -n "$dhcp_rapid_commit_proxy" ] && append "$var" "dhcp_rapid_commit_proxy=$dhcp_rapid_commit_proxy" "$N"
        	fi

		case "$ieee80211w" in
			[01]) append "$var" "wpa_key_mgmt=${key_mgmt_str} ${add_sha256_str} ${ieee80211r_str} ${ieee80211ai_sha256_str} ${ieee80211ai_sha384_str} ${sae_str} ${owe_str}" "$N";;
			2)
				if [ "${suite_b}" -eq 192 ]
				then
					append "$var" "wpa_key_mgmt=${suite_b_str}" "$N"
				else
					append "$var" "wpa_key_mgmt=${add_sha256_str} ${ieee80211r_str} ${ieee80211ai_sha256_str} ${ieee80211ai_sha384_str} ${sae_str} ${owe_str}" "$N"
				fi
			;;
		esac
	fi
	config_get macfilter "$vif" macfilter
	iface_name=$(foreach wireless wifi-iface ifname $ifname 2>/dev/null)
	ssid_index=${iface_name#*_ssid_}
	ezmcloud=$(test -f /etc/config/ezmcloud && echo 1 || echo 0)
	if [ "$ezmcloud" = "1" ]; then
		local hostapdL2ACLPath="/etc/ezmcloud/config/"
	else
		local hostapdL2ACLPath="/etc/l2acl/"
	fi

	[ -n "$macfilter" ] && [ "$ezmcloud" = "1" -o "$(uci -q get functionlist.functionlist.SUPPORT_HOSTAPD_MAC_FILTER)" = "1" ] && {
		case "$macfilter" in
			hostapd_deny|deny)
				[ "$radius_macauth_enable" != "1" ] && append "$var" "macaddr_acl=0" "$N"
				policy="deny"
				;;
			hostapd_allow|allow)
				[ "$radius_macauth_enable" != "1" ] && append "$var" "macaddr_acl=1" "$N"
				policy="accept"
				;;
			*)
				policy="not_support"
				;;
		esac
		[ "$policy" != "not_support" -a -f "${hostapdL2ACLPath}l2acl-ssid_${ssid_index}.maclist" ] && {
			append "$var" "${policy}_mac_file=${hostapdL2ACLPath}l2acl-ssid_${ssid_index}.maclist" "$N"
		}
	}
	#cloud banned message. When turn on, bypass acl check and let send snlog "disobey acl policy"
	support_banned_msg=$(get_sn_hostap_option SENAO_CLOUD_BANNED_MSG)
	config_get banned_msg_en "$vif" banned_msg_en 0
	if [ "$banned_msg_en" -gt 0 -a "$support_banned_msg" == "1" ]; then
		append "$var" "banned_msg_en=1" "$N"
	fi

	config_get multi_cred "$vif" multi_cred 0

	if [ "$multi_cred" -gt 0 ]; then
		append "$var" "skip_cred_build=1" "$N"
		append "$var" "extra_cred=/var/run/hostapd_cred_${device}.bin" "$N"
	fi

	config_get bss_load_update_period "$vif" bss_load_update_period
	[ -n "$bss_load_update_period" ] && append "$var" "bss_load_update_period=$bss_load_update_period" "$N"

	config_get_bool hs20 "$vif" hs20 0
	if [ "$hs20" -gt 0 ]
	then
		append "$var" "hs20=1" "$N"
		config_get disable_dgaf "$vif" disable_dgaf
		[ -n "$disable_dgaf" ] && append "$var" "disable_dgaf=$disable_dgaf" "$N"

		add_hs20_oper_friendly_name() {
			append "$var" "hs20_oper_friendly_name=${1}" "$N"
		}

		config_list_foreach "$vif" hs20_oper_friendly_name add_hs20_oper_friendly_name

		add_hs20_conn_capab() {
			append "$var" "hs20_conn_capab=${1}" "$N"
		}

		config_list_foreach "$vif" hs20_conn_capab add_hs20_conn_capab

		config_get hs20_wan_metrics "$vif" hs20_wan_metrics
		[ -n "$hs20_wan_metrics" ] && append "$var" "hs20_wan_metrics=$hs20_wan_metrics" "$N"
		config_get hs20_operating_class "$vif" hs20_operating_class
		[ -n "$hs20_operating_class" ] && append "$var" "hs20_operating_class=$hs20_operating_class" "$N"

		append "$var" "interworking=1" "$N"
		append "$var" "manage_p2p=1" "$N"
		append "$var" "tdls_prohibit=1" "$N"
		config_get hessid "$vif" hessid
		[ -n "$hessid" ] && append "$var" "hessid=$hessid" "$N"
		config_get access_network_type "$vif" access_network_type
		[ -n "$access_network_type" ] && append "$var" "access_network_type=$access_network_type" "$N"
		config_get internet "$vif" internet
		[ -n "$internet" ] && append "$var" "internet=$internet" "$N"
		config_get asra "$vif" asra
		[ -n "$asra" ] && append "$var" "asra=$asra" "$N"
		config_get esr "$vif" esr
		[ -n "$esr" ] && append "$var" "esr=$esr" "$N"
		config_get uesa "$vif" uesa
		[ -n "$uesa" ] && append "$var" "uesa=$uesa" "$N"
		config_get venue_group "$vif" venue_group
		[ -n "$venue_group" ] && append "$var" "venue_group=$venue_group" "$N"
		config_get venue_type "$vif" venue_type
		[ -n "$venue_type" ] && append "$var" "venue_type=$venue_type" "$N"
		add_roaming_consortium() {
			append "$var" "roaming_consortium=${1}" "$N"
		}
		config_list_foreach "$vif" roaming_consortium add_roaming_consortium

		add_venue_name() {
			append "$var" "venue_name=${1}" "$N"
		}
		config_list_foreach "$vif" venue_name add_venue_name

		#add_network_auth_type() {
		#	append "$var" "network_auth_type=${1}" "$N"
		#}
		config_get network_auth_type "$vif" network_auth_type
		[ -n "$network_auth_type" ] && append "$var" "network_auth_type=$network_auth_type" "$N"
		#config_list_foreach "$vif" network_auth_type add_network_auth_type
		config_get ipaddr_type_availability "$vif" ipaddr_type_availability
		[ -n "$ipaddr_type_availability" ] && append "$var" "ipaddr_type_availability=$ipaddr_type_availability" "$N"


		add_domain_name() {
			append "$var" "domain_name=${1}" "$N"
		}

		config_list_foreach "$vif" domain_name add_domain_name

		config_get anqp_3gpp_cell_net "$vif" anqp_3gpp_cell_net
		[ -n "$anqp_3gpp_cell_net" ] && append "$var" "anqp_3gpp_cell_net=$anqp_3gpp_cell_net" "$N"

		config_get qos_map_set "$vif" qos_map_set
		[ -n "$qos_map_set" ] && append "$var" "qos_map_set=$qos_map_set" "$N"
		config_get gas_frag_limit "$vif" gas_frag_limit
		[ -n "$gas_frag_limit" ] && append "$var" "gas_frag_limit=$gas_frag_limit" "$N"
		config_get hs20_deauth_req_timeout "$vif" hs20_deauth_req_timeout
		[ -n "$hs20_deauth_req_timeout" ] && append "$var" "hs20_deauth_req_timeout=$hs20_deauth_req_timeout" "$N"

		add_nai_realm() {
			append "$var" "nai_realm=${1}" "$N"
		}
		config_list_foreach "$vif" nai_realm add_nai_realm

		add_hs20_icon() {
			append "$var" "hs20_icon=${1}" "$N"
		}
		config_list_foreach "$vif" hs20_icon add_hs20_icon

		config_get osu_ssid "$vif" osu_ssid
		[ -n "$osu_ssid" ] && append "$var" "osu_ssid=\"$osu_ssid\"" "$N"

		add_osu_server_uri() {
			[ -n "${1}" ] && append "$var" "osu_server_uri=${1}" "$N"
		}
		config_list_foreach "$vif" osu_server_uri add_osu_server_uri

		add_osu_friendly_name() {
			append "$var" "osu_friendly_name=${1}" "$N"
		}
		config_list_foreach "$vif" osu_friendly_name add_osu_friendly_name

		config_get osu_nai "$vif" osu_nai
		[ -n "$osu_nai" ] && append "$var" "osu_nai=$osu_nai" "$N"

		config_get osu_method_list "$vif" osu_method_list
		[ -n "$osu_method_list" ] && append "$var" "osu_method_list=$osu_method_list" "$N"

		add_osu_icon() {
			append "$var" "osu_icon=${1}" "$N"
		}
		config_list_foreach "$vif" osu_icon add_osu_icon

		add_osu_service_desc() {
			append "$var" "osu_service_desc=${1}" "$N"
		}
		config_list_foreach "$vif" osu_service_desc add_osu_service_desc

	else
		config_get interworking "$vif" interworking
		[ -n "$interworking" ] && append "$var" "interworking=$interworking" "$N"
	fi

	add_anqp_elem() {
		append "$var" "anqp_elem=${1}" "$N"
	}
	config_list_foreach "$vif" anqp_elem add_anqp_elem

	config_get mbo_cell_data_conn_pref "$vif" mbo_cell_data_conn_pref
	[ -n "$mbo_cell_data_conn_pref" ] && append "$var" "mbo_cell_data_conn_pref=$mbo_cell_data_conn_pref" "$N"

	config_get osen "$vif" osen
	[ -n "$osen" ] && append "$var" "osen=$osen" "$N"

	config_get gas_comeback_delay "$vif" gas_comeback_delay
	[ -n "$gas_comeback_delay" ] && append "$var" "gas_comeback_delay=$gas_comeback_delay" "$N"

	if [ "$ieee80211r" -gt 0 ]
	then

		### SENAO ### read fastroaming
		if [ -z  "$nasid" ]; then
		    nasid="radius.senao.com"
		fi
		local AP_MAC=`ifconfig $ifname |awk '{ if( $4 == "HWaddr" )print $5 }'`
		local ROAMING_KEY="0f0e0d0c0b0a09080706050403020100"
		local R0KH="$AP_MAC $nasid $ROAMING_KEY"
		local R1KH="$AP_MAC $AP_MAC $ROAMING_KEY"
		local R1_KEY_HOLDER=`echo $AP_MAC |awk 'BEGIN {FS=":"}; {print $1$2$3$4$5$6}'`

		config_get mobility_domain "$vif" mobility_domain a1b2
		[ -n "$mobility_domain" ] && append "$var" "mobility_domain=$mobility_domain" "$N"
		config_get r0_key_lifetime "$vif" r0_key_lifetime 10000
		append "$var" "r0_key_lifetime=$r0_key_lifetime" "$N"
		config_get r1_key_holder "$vif" r1_key_holder $R1_KEY_HOLDER
		[ -n "$r1_key_holder" ] && append "$var" "r1_key_holder=$r1_key_holder" "$N"
		config_get reassociation_deadline "$vif" reassociation_deadline 1000
		append "$var" "reassociation_deadline=$reassociation_deadline" "$N"
		config_get pmk_r1_push "$vif" pmk_r1_push 1
		append "$var" "pmk_r1_push=$pmk_r1_push" "$N"
		config_get ft_over_ds "$vif" ft_over_ds 0
		[ -n "$ft_over_ds" ] && append "$var" "ft_over_ds=$ft_over_ds" "$N"

		#config_get nasid2 "$vif" nasid2
		#config_get ap_macaddr "$vif" ap_macaddr
		#config_get ap2_macaddr "$vif" ap2_macaddr
		#config_get ap2_r1_key_holder "$vif" ap2_r1_key_holder

		#append "$var" "r0kh=$ap_macaddr $nasid $kh_key_hex" "$N"
		#append "$var" "r0kh=$ap2_macaddr $nasid2 $kh_key_hex" "$N"
		#append "$var" "r1kh=$ap2_macaddr $ap2_r1_key_holder $kh_key_hex" "$N"

		## senao band-steering 11v+11r ##
                config_get bandsteer_en "$vif" bandsteer_en 0
		if [ "$bandsteer_en" -gt 0 ] && [ -e "/tmp/wifi_all_mac.txt" ]; then
		    config_get ifname "$vif" ifname
		    ap2_macaddr=$(hostapd_get_ap2_macaddr $ifname)
		    if [ -n "$ap2_macaddr" ]; then
			append "$var" "r0kh=$ap2_macaddr $nasid $ROAMING_KEY" "$N"
			append "$var" "r1kh=$ap2_macaddr $ap2_macaddr $ROAMING_KEY" "$N"
		    fi

		    ap3_macaddr=$(hostapd_get_ap3_macaddr $ifname)
		    if [ -n "$ap3_macaddr" ]; then
			append "$var" "r0kh=$ap3_macaddr $nasid $ROAMING_KEY" "$N"
			append "$var" "r1kh=$ap3_macaddr $ap3_macaddr $ROAMING_KEY" "$N"
		    fi
		fi

		## senao ##
		append "$var" "nas_identifier=$nasid" "$N"
		append "$var" "r0kh=$R0KH" "$N"
		append "$var" "r1kh=$R1KH" "$N"
		append "$var" "rsn_preauth=1" "$N"
		append "$var" "rsn_preauth_interfaces=br-lan" "$N"


		### Senao OKC fastroaming, turn senao_okc off to pass wpa3 certification.
		### The senao okc is not necessary feature.
		#echo "WPA-EAP OKC enable\n" > /dev/ttyS0
		#append "$var" "sokc_enable=1" "$N"
		#append "$var" "sokc_check_mac=1" "$N"
		#if [ $wifi_dev == "wifi0" ]; then
		#	echo "OKC 2.4G gid=1" > /dev/ttyS0
		#	append "$var" "sokc_group_index=1" "$N"
		#elif [ $wifi_dev == "wifi1" ]; then
		#	echo "OKC 5G gid=2" > /dev/ttyS0
		#	append "$var" "sokc_group_index=2" "$N"
		#fi

	fi

	config_get_bool wnm_sleep_mode "$vif" wnm_sleep_mode
	[ -n "$wnm_sleep_mode" ] && append "$var" "wnm_sleep_mode=$wnm_sleep_mode" "$N"

	config_get_bool wnm_sleep_mode_no_keys "$vif" wnm_sleep_mode_no_keys
	[ -n "$wnm_sleep_mode_no_keys" ] && append "$var" "wnm_sleep_mode_no_keys=$wnm_sleep_mode_no_keys" "$N"

	config_get_bool bss_transition "$vif" bss_transition
	[ -n "$bss_transition" ] && append "$var" "bss_transition=$bss_transition" "$N"
	return 0
}

hostapd_get_vif_name () {
	[ -e /lib/functions.sh ] && . /lib/functions.sh
	DEVICES=
	config_cb() {
		local type="$1"
		local section="$2"
		local index="$(cat /sys/class/ieee80211/$phy/index)"

		# section start
		case "$type" in
			wifi-device)
				append DEVICES "$section"
				config_set "$section" vifs ""
				config_set "$section" ht_capab ""
			;;
		esac

		# section end
		config_get TYPE "$CONFIG_SECTION" TYPE
		case "$TYPE" in
			wifi-iface)
				config_get device "$CONFIG_SECTION" device
				config_get vifs "$device" vifs
				append vifs "$CONFIG_SECTION"
				config_set "$device" vifs "$vifs"
				for vif_interface in $vifs; do
					[ "$device" == "radio$index" ] && {
						config_set "$device" phy "$phy"
						vif=$vif_interface
					}
				done
			;;
		esac
	}
}

hostapd_set_log_options() {
	local var="$1"
	local cfg="$2"
	local log_level log_80211 log_8021x log_radius log_wpa log_driver log_iapp log_mlme

	config_get log_level "$cfg" log_level 2

	config_get_bool log_80211  "$cfg" log_80211  1
	config_get_bool log_8021x  "$cfg" log_8021x  1
	config_get_bool log_radius "$cfg" log_radius 1
	config_get_bool log_wpa    "$cfg" log_wpa    1
	config_get_bool log_driver "$cfg" log_driver 1
	config_get_bool log_iapp   "$cfg" log_iapp   1
	config_get_bool log_mlme   "$cfg" log_mlme   1

	[ -z "$cfg" ] && {
		set_default log_level 2
		set_default log_80211  1
		set_default log_8021x  1
		set_default log_radius 1
		set_default log_wpa    1
		set_default log_driver 1
		set_default log_iapp   1
		set_default log_mlme   1
	}

	local log_mask=$((       \
		($log_80211  << 0) | \
		($log_8021x  << 1) | \
		($log_radius << 2) | \
		($log_wpa    << 3) | \
		($log_driver << 4) | \
		($log_iapp   << 5) | \
		($log_mlme   << 6)   \
	))

	append "$var" "logger_syslog=$log_mask" "$N"
	append "$var" "logger_syslog_level=$log_level" "$N"
	append "$var" "logger_stdout=$log_mask" "$N"
	append "$var" "logger_stdout_level=$log_level" "$N"
}

hostapd_config_multi_cred() {
	local vif="$1" && shift
	local ifname device
	local cred_config temp
	extra_cred=

	config_get ifname "$vif" ifname
	config_get device "$vif" device

	hostapd_set_extra_cred extra_cred "$vif" "$ifname"


	extra_cred=$(echo $extra_cred | tr -d ' ')
	extra_cred=$(echo $extra_cred | tr -d ':')

	temp=`expr length "$extra_cred" / 2 `
	temp=` printf "%04X" $temp`

	#ATTR_CRED
	cred_config="100e$temp$extra_cred"

	cat > /var/run/hostapd_cred_tmp.conf <<EOF
$cred_config
EOF

	sed 's/\([0-9A-F]\{2\}\)/\\\\\\x\1/gI' /var/run/hostapd_cred_tmp.conf | xargs printf >> /var/run/hostapd_cred_$device.bin

}


hostapd_setup_vif() {
	local vif="$1" && shift
	local driver="$1" && shift
	local no_nconfig
	local ifname device channel hwmode
	local fst_disabled
	local fst_iface1
	local fst_iface2
	local fst_group_id
	local fst_priority1
	local fst_priority2

	#Rest wps_possible value, for 2~8 ssid wps funtion.
	wps_possible=

	hostapd_cfg=

	# These are flags that may or may not be used when calling
	# "hostapd_setup_vif()". These are not mandatory and may be called in
	# any order
	while [ $# -ne 0 ]; do
		local tmparg="$1" && shift
		case "$tmparg" in
		no_nconfig)
			no_nconfig=1
			;;
		esac
	done

	config_get ifname "$vif" ifname
	config_get device "$vif" device
	config_get bintval "$vif" bintval
	config_get_bool dpp "$vif" dpp 0
	config_get channel "$device" channel
	config_get hwmode "$device" hwmode
	config_get htmode "$device" htmode
	config_get_bool shortgi "$vif" shortgi 1
	config_get multi_group_key "$vif" multi_group_key

	support_snlog=$(get_sn_hostap_option SENAO_LOG)
	[ "$support_snlog" == 1 ] && {
		config_get ssid_id "$vif" id
		[ -n "$ssid_id" ] && append hostapd_cfg "ssid_id=$ssid_id" "$N"
	}

	hostapd_set_log_options hostapd_cfg "$device"

	config_load fst && {
		config_get fst_disabled config disabled
		config_get fst_iface1 config interface1
		config_get fst_iface2 config interface2
		config_get fst_group_id config mux_interface
		config_get fst_priority1 config interface1_priority
		config_get fst_priority2 config interface2_priority

		if [ $fst_disabled -eq 0 ]; then
			if [ "$ifname" == $fst_iface1 ] ; then
				append hostapd_cfg "fst_group_id=$fst_group_id" "$N"
				append hostapd_cfg "fst_priority=$fst_priority1" "$N"
			elif [ "$ifname" == $fst_iface2 ] ; then
				append hostapd_cfg "fst_group_id=$fst_group_id" "$N"
				append hostapd_cfg "fst_priority=$fst_priority2" "$N"
			fi
		fi
	}

	case "$hwmode" in
		*bg|*gdt|*gst|*fh) hwmode=g;;
		*adt|*ast) hwmode=a;;
	esac

	# SENAO add nobeaconup feature
	if [ -x /usr/sbin/wifi-control.sh ]
	then
		rom_section=$(/usr/sbin/foreach /rom/etc/config/wireless wifi-iface ifname $ifname)
		nobeaconup=$(uci -q -c /rom/etc/config/ get wireless.$rom_section.nobeaconup || echo 0)
	else
		nobeaconup=0
	fi

	config_get_bool hidden "$vif" hidden 0
	if [ "$hidden" -gt 0 ]; then
		ignore_broadcast_ssid=$hidden
	else
		if [ $nobeaconup -eq 1 ]; then
			ignore_broadcast_ssid=1
		else
			ignore_broadcast_ssid=0
		fi
	fi

	if [ "$driver" == "nl80211" ]; then
                # senao mod: do not set channel directly by hostapd
		append hostapd_cfg "#${channel:+channel=$channel}" "$N"
		ht_capab=$(cat /sys/class/net/${ifname}/cfg80211_htcaps)
		vht_capab=$(cat /sys/class/net/${ifname}/cfg80211_vhtcaps)
		nl_hwmode=
		idx="$channel"
		# if UCI configuration is 11ac and htmode is not configured
		# then get htmode form syctl file /sys/class/net/$device/5g_maxchwidth
		if [ "$hwmode" == "11ac" ] && [ -z $htmode ]; then
			if [ -f /sys/class/net/$device/5g_maxchwidth ]; then
				maxchwidth="$(cat /sys/class/net/$device/5g_maxchwidth)"
				[ -n "$maxchwidth" ] && htmode=HT$maxchwidth
			else
				htmode=HT80
			fi
		fi

		case "$hwmode:$htmode" in
		# The parsing stops at the first match so we need to make sure
		# these are in the right orders (most generic at the end)
			*ng:HT20)
				nl_hwmode=g
				append ht_capab "[$htmode]"
				[ "$shortgi" != "0" ] &&  append ht_capab "[SHORT-GI-20]"
				append hostapd_cfg "ieee80211n=1" "$N"
				;;
			*ng:HT40-)
				nl_hwmode=g
				append ht_capab "[$htmode]"
				[ "$shortgi" != "0" ] &&  append ht_capab "[SHORT-GI-40]"
				append hostapd_cfg "ieee80211n=1" "$N"
				;;
			*ng:HT40+)
				nl_hwmode=g
				append ht_capab "[$htmode]"
				[ "$shortgi" != "0" ] &&  append ht_capab "[SHORT-GI-40]"
				append hostapd_cfg "ieee80211n=1" "$N"
				;;
			*ng:HT40)
				nl_hwmode=g
			case "$channel" in
				1|2|3|4|5|6|7) append ht_capab "[HT40+]";;
				8|9|10|11|12|13) append ht_capab "[HT40-]";;
			esac
				[ "$shortgi" != "0" ] &&  append ht_capab "[SHORT-GI-40]"
				append hostapd_cfg "ieee80211n=1" "$N"
				;;
			*ng:*)
				nl_hwmode=g
				append ht_capab "[HT20]"
				[ "$shortgi" != "0" ] &&  append ht_capab "[SHORT-GI-20]"
				append hostapd_cfg "ieee80211n=1" "$N"
				;;

			*na:HT20)
				nl_hwmode=a
				append ht_capab "[$htmode]"
				[ "$shortgi" != "0" ] &&  append ht_capab "[SHORT-GI-20]"
				append hostapd_cfg "ieee80211n=1" "$N"
				;;
			*na:HT40-)
				nl_hwmode=a
				append ht_capab "[$htmode]"
				[ "$shortgi" != "0" ] &&  append ht_capab "[SHORT-GI-40]"
				append hostapd_cfg "ieee80211n=1" "$N"
				;;
			*na:HT40+)
				nl_hwmode=a
				append ht_capab "[$htmode]"
				[ "$shortgi" != "0" ] &&  append ht_capab "[SHORT-GI-40]"
				append hostapd_cfg "ieee80211n=1" "$N"
				;;
			*na:HT40)
				nl_hwmode=a
			case "$channel" in
				36|44|52|60|100|108|116|124|132|140|149|157) append ht_capab "[HT40+]";;
				40|48|56|64|104|112|120|128|136|144|153|161) append ht_capab "[HT40-]";;
				esac
				[ "$shortgi" != "0" ] &&  append ht_capab "[SHORT-GI-40]"
				append hostapd_cfg "ieee80211n=1" "$N"
				;;
			*na:*)
				nl_hwmode=a
			case "$channel" in
				36|44|52|60|100|108|116|124|132|140|149|157) append ht_capab "[HT40+]";;
				40|48|56|64|104|112|120|128|136|144|153|161) append ht_capab "[HT40-]";;
				esac
				[ "$shortgi" != "0" ] &&  append ht_capab "[SHORT-GI-40]"
				append hostapd_cfg "ieee80211n=1" "$N"
				;;
			*ac:HT20|*ac:HT40+|*ac:HT40-|*ac:HT40)
				nl_hwmode=a
				append hostapd_cfg "ieee80211ac=1" "$N"
				append hostapd_cfg "ieee80211n=1" "$N"
				case "$channel" in
					36|44|52|60|100|108|116|124|132|140|149|157) append ht_capab "[HT40+]";;
					40|48|56|64|104|112|120|128|136|144|153|161) append ht_capab "[HT40-]";;
				esac
				[ "$shortgi" != "0" ] &&  append ht_capab "[SHORT-GI-40]"
				case "$channel" in
					36|40) idx=38;;
					44|48) idx=46;;
					52|56) idx=54;;
					60|64) idx=62;;
					100|104) idx=102;;
					108|112) idx=110;;
					116|120) idx=118;;
					124|128) idx=126;;
					132|136) idx=134;;
					140|144) idx=142;;
					149|153) idx=151;;
					157|161) idx=159;;
				esac
				append hostapd_cfg "vht_oper_chwidth=0" "$N"
				append hostapd_cfg "vht_oper_centr_freq_seg0_idx=$idx" "$N"
				;;
			*ac:HT80)
				nl_hwmode=a
				append hostapd_cfg "ieee80211ac=1" "$N"
				append hostapd_cfg "ieee80211n=1" "$N"
				case "$channel" in
					36|44|52|60|100|108|116|124|132|140|149|157) append ht_capab "[HT40+]";;
					40|48|56|64|104|112|120|128|136|144|153|161) append ht_capab "[HT40-]";;
				esac
				[ "$shortgi" != "0" ] &&  append ht_capab "[SHORT-GI-40]"
 				case "$channel" in
					36|40|44|48) idx=42;;
					52|56|60|64) idx=58;;
					100|104|108|112) idx=106;;
					116|120|124|128) idx=122;;
					132|136|140|144) idx=138;;
					149|153|157|161) idx=155;;
				esac
				append hostapd_cfg "vht_oper_chwidth=1" "$N"
				append hostapd_cfg "vht_oper_centr_freq_seg0_idx=$idx" "$N"
				;;
			*ac:HT160)
				nl_hwmode=a
				append hostapd_cfg "ieee80211ac=1" "$N"
				append hostapd_cfg "ieee80211n=1" "$N"
				case "$channel" in
					36|44|52|60|100|108|116|124|132|140|149|157) append ht_capab "[HT40+]";;
					40|48|56|64|104|112|120|128|136|144|153|161) append ht_capab "[HT40-]";;
				esac
				[ "$shortgi" != "0" ] &&  append ht_capab "[SHORT-GI-40]"
				case "$channel" in
					36|40|44|48|52|56|60|64) idx=50;;
					100|104|108|112|116|120|124|128) idx=114;;
				esac
				append hostapd_cfg "vht_oper_chwidth=2" "$N"
				append hostapd_cfg "vht_oper_centr_freq_seg0_idx=$idx" "$N"
				;;
			*axg:*) nl_hwmode=g;;
			*axa:*) nl_hwmode=a;;
			*ac:HT80_80) nl_hwmode=a;;
			*bg:*)
				nl_hwmode=g
				;;
			*g:*)
				nl_hwmode=g
				;;
			*a:*)
				nl_hwmode=a
				;;
			*b:*)
				nl_hwmode=g
				;;
			*)
				;;
		esac

		[ -n "$ht_capab" ] && append hostapd_cfg "ht_capab=$ht_capab" "$N"
		[ -n "$vht_capab" ] && append hostapd_cfg "vht_capab=$vht_capab" "$N"

		case "$nl_hwmode" in
			*g) hwmode=g;;
			*a) hwmode=a;;
		esac

		append hostapd_cfg "hw_mode=$hwmode" "$N"
		append hostapd_cfg "wmm_enabled=1" "$N"

		[ -n "$bintval" ] && append hostapd_cfg "beacon_int=$bintval" "$N"
		config_get dtim_period "$vif" dtim_period 1
		append hostapd_cfg "dtim_period=$dtim_period" "$N"

		# config_get_bool hidden "$vif" hidden 0
		append hostapd_cfg "ignore_broadcast_ssid=$ignore_broadcast_ssid" "$N"

	fi  #end of nl80211

	hostapd_set_bss_options hostapd_cfg "$vif" 

	# SENAO add nobeaconup feature
	if [ $nobeaconup -eq 1 ]
	then
		# beacon on when wifi-schedule start
		wifi-control.sh -a beacon off $ifname
	fi

	[ "$channel" = auto ] && channel=
	[ -n "$channel" -a -z "$hwmode" ] && wifi_fixup_hwmode "$device"
	rm -f /var/run/hostapd-$ifname.conf
	cat > /var/run/hostapd-$ifname.conf <<EOF
driver=$driver
interface=$ifname
#${channel:+channel=$channel}
$hostapd_cfg
EOF
	[ -z "${no_nconfig}" ] &&
		echo ${hwmode:+hw_mode=${hwmode#11}} >> /var/run/hostapd-$ifname.conf

	entropy_file=/var/run/entropy-$ifname.bin

	# Run a single hostapd instance for all the radio's
	# Enables WPS VAP TIE feature
	config_get_bool wps_vap_tie_dbdc qcawifi wps_vap_tie_dbdc 0

	if [ $wps_vap_tie_dbdc -ne 0 ]; then
		echo -e "/var/run/hostapd-$ifname.conf \c\h" >> /tmp/hostapd_conf_filename
	else
		[ -f "/var/run/hostapd-$ifname.lock" ] &&
			rm /var/run/hostapd-$ifname.lock
		wpa_cli -g /var/run/hostapd/global raw ADD bss_config=$ifname:/var/run/hostapd-$ifname.conf

		# SENAO add nobeaconup feature, restore ignore_broadcast_ssid value
		if [ $nobeaconup -eq 1 ]; then
			config_get_bool hidden "$vif" hidden 0
			if [ "$hidden" != "$ignore_broadcast_ssid" ]; then
				echo "ignore_broadcast_ssid=$hidden" >> /var/run/hostapd-$ifname.conf
			fi
		fi

		touch /var/run/hostapd-$ifname.lock

		if [ -n "$wps_possible" -a -n "$config_methods" ]; then
			echo " Warring - avoid wps enable to cause vap cannot enable, mask failed part and solve it in the future." > /dev/console
#			pid=/var/run/hostapd_cli-$ifname.pid
#			hostapd_cli -i $ifname  -P $pid -a /lib/wifi/wps-hostapd-update-uci -p /var/run/hostapd-$device -B
		elif [ "${dpp}" -eq 1 ]
		then
			config_get dpp_type "$vif" dpp_type "qrcode"
			config_get dpp_curve "$vif" dpp_curve "P-256"
			config_get dpp_key "$vif" dpp_key

			pid=/var/run/hostapd_cli-$ifname.pid
			hostapd_cli -i $ifname  -P $pid -a /lib/wifi/dpp-hostapd-update-uci -p /var/run/hostapd-$device -B
			hostapd_cli -i $ifname  -p /var/run/hostapd-$device DPP_BOOTSTRAP_GEN type=$dpp_type curve=$dpp_curve chan=81/$channel mac=$(cat /sys/class/net/$ifname/address | sed 's/://g') key=$dpp_key
		fi
	fi

	if [ "$multi_group_key" = "1" -a "$(crontab -l | grep "wpa-psk-check.sh" | wc -l)" = "0" ]; then
		crontab -l | { cat; echo "*/1 * * * * /usr/sbin/wpa-psk-check.sh"; } | crontab -
	fi
}


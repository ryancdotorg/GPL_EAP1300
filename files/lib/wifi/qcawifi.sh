#!/bin/sh
#
# Copyright (c) 2014, 2016, The Linux Foundation. All rights reserved.
#
. /lib/sn_functions.sh
. /lib/wifi/wifi_funcs.sh

append DRIVERS "qcawifi"

wifi0_mac_file="/tmp/wifi0_mac.txt"
wifi1_mac_file="/tmp/wifi1_mac.txt"
wifi2_mac_file="/tmp/wifi2_mac.txt"
error_msg="/tmp/invalid_mac.txt"

ar71xx_sh_file="/lib/ar71xx.sh"
ipq40xx_sh_file="/lib/firmware/IPQ4019"
qca98xx_sh_file="/lib/firmware/QCA9888"
ipq806x_sh_file="/lib/ipq806x.sh"

platform_name="unknown"

if [ -e $ar71xx_sh_file ]; then
	platform_name="ar71xx"
elif [ -e $ipq40xx_sh_file ]; then
	platform_name="ipq40xx"
elif [ -e $ipq806x_sh_file ]; then
	platform_name="ipq806x"
fi 

RELOAD_WIFI_WITHOUT_KMODS=1	#Jacky: Follow qsdk qcawifi.sh not to rmmod and insmod when wifi reload unless insmod parameter changed

monitor_mode() {	#non-using function now, monitor mode runs normal flow to create, keep it for reference
	local device="$1"

	config_get opmode "$device" opmode
	[ "$opmode" == "mon" ] && {
		config_get vifs "$device" vifs
		config_get phy "$device" phy
		for vif in $vifs; do
			config_get ifname "$vif" ifname
			config_get mode_display "$vif" mode_display
			[ "$opmode" == "$mode_display" ] && {
				DebugPrint "Create monitor vap: wlanconfig $ifname create wlandev $phy wlanmode $opmode"
				ifname=$(/usr/sbin/wlanconfig "$ifname" create wlandev "$phy" wlanmode "$opmode")
				[ $? -ne 0 ] && DebugPrint "monitor_mode($device): Failed to set up $mode vif $ifname" || ifconfig "$ifname" up
				return 0
			}
		done
	}
	return 1
}

find_qcawifi_phy() {
	local device="$1"

	local macaddr="$(config_get "$device" macaddr | tr 'A-Z' 'a-z')"

	#senao jacky: we should set phy to wifi0, wifi1 or wifi2, not use the old config phy0

	phy=$device
	#config_get phy "$device" phy
	#end of senao patch
	[ -z "$phy" -a -n "$macaddr" ] && {
		cd /sys/class/net
		for phy in $(ls -d wifi* 2>&-); do
			[ "$macaddr" = "$(cat /sys/class/net/${phy}/address)" ] || continue
			config_set "$device" phy "$phy"
			break
		done
		config_get phy "$device" phy
	}
	[ -n "$phy" -a -d "/sys/class/net/$phy" ] || {
		DebugPrint "phy for wifi device $1 not found"
		return 1
	}
	[ -z "$macaddr" ] && {
		config_set "$device" macaddr "$(cat /sys/class/net/${phy}/address)"
	}
	return 0
}

scan_qcawifi() {
	local device="$1"
	local wds
	local adhoc sta ap disabled

	[ ${device%[0-9]} = "wifi" ] && config_set "$device" phy "$device"

	local ifidx=0
	local radioidx=${device#wifi}
    local led_on=0

    config_get disabled "$device" disabled 0
    [ $disabled = 0 ] && led_on=1

    if [ "$device" = "wifi1" ]; then
        [ -e "/sys/class/leds/wlan_5g_led/brightness" ] && echo "${led_on}" > /sys/class/leds/wlan_5g_led/brightness
    fi

    if [ "$device" = "wifi0" ]; then
        [ -e "/sys/class/leds/wlan_2g_led/brightness" ] && echo "${led_on}" > /sys/class/leds/wlan_2g_led/brightness
    fi

	config_get vifs "$device" vifs
	config_get backgroundscanEnable "$device" backgroundscanEnable
	config_get apsteer "$device" apsteer 0
	config_get spectrumMode "$device" spectrumMode 0
	config_get opmode "$device" opmode
	haveScanRadio=$(check_scan_radio_enable)

	[ "$spectrumMode" == "1" ] && isCreat=0

	for vif in $vifs; do
		config_get mode "$vif" mode

		[ "$spectrumMode" == "1" -a "$opmode" == "ap" -a "$haveScanRadio" == "0" ] && {
			config_get mode_display "$vif" mode_display
			[ "$mode_display" == "ap" ] && {
				[ "$isCreat" == "0" ] && {
					config_set "$vif" disabled 0
					uci set wireless.$vif.disabled="0"
					isCreat=1
				} || {
					config_set "$vif" disabled 1
					uci set wireless.$vif.disabled="1"
				}
				uci commit wireless
			}
		}

		[ "$haveScanRadio" == "0" -a "$spectrumMode" == "0" ] && [ "$mode" == "monitor" ] && {
			config_get disabled "$vif" disabled
			if [ "$backgroundscanEnable" == "1" -o "$apsteer" == "1" ]; then
				if [ "$(uci -q get wifiprofile.snWifiConf.SUPPORT_CAPWAP_IWLIST_SCAN)" == "1" ]; then
					[ "$disabled" == "0" ] && {
						config_set "$vif" disabled 1
						uci set wireless.$vif.disabled="1"
						uci commit wireless
						DebugPrint "force to disable monitor mode because background scanning replace to iwlist displayscan."
					}
				else
					[ "$disabled" == "1" ] && {
						config_set "$vif" disabled 0
						uci set wireless.$vif.disabled="0"
						uci commit wireless
						DebugPrint "force to enable monitor mode because background scanning need airodump-ng."
					}
				fi
			else
				[ "$disabled" == "0" ] && {
					config_set "$vif" disabled 1
					uci set wireless.$vif.disabled="1"
					uci commit wireless
					DebugPrint "force to disable monitor mode because the background scanning has been terminated."
				}
			fi
		}
	
		config_get_bool disabled "$vif" disabled 0

		# senao WAR : mesh enable -> SCAN SSID disable
		[ "$(uci get wifiprofile.snWifiConf.SUPPORT_WAR_MESH_DISABLE_SCAN_SSID)" == "1" -a "$mode" == "monitor" -a "$(uci get mesh.wifi.disabled)" == "0" ] && disabled=1

		[ $disabled = 0 ] || continue

	#	local vifname
	#	[ $ifidx -gt 0 ] && vifname="ath${radioidx}$ifidx" || vifname="ath${radioidx}"

		config_get ifname "$vif" ifname
	#	config_set "$vif" ifname "${ifname:-$vifname}"

		case "$mode" in
			adhoc|sta|ap|monitor|wrap)
				append $mode "$vif"
			;;
			wds)
				config_get ssid "$vif" ssid
				[ -z "$ssid" ] && continue

				config_set "$vif" wds 1
				config_set "$vif" mode sta
				mode="sta"
				addr="$ssid"
				${addr:+append $mode "$vif"}
			;;
			*) DebugPrint "$device($vif): Invalid mode, ignored."; continue;;
		esac

		ifidx=$(($ifidx + 1))
	done

	case "${adhoc:+1}:${sta:+1}:${ap:+1}" in
		# valid mode combinations
		1::) wds="";;
		1::1);;
		:1:1)config_set "$device" nosbeacon 1;; # AP+STA, can't use beacon timers for STA
		:1:);;
		::1);;
		::);;
		*) DebugPrint "$device: Invalid mode combination in config"; return 1;;
	esac

	config_set "$device" vifs "${sta:+$sta }${ap:+$ap }${wrap:+$wrap }${adhoc:+$adhoc }${wds:+$wds }${monitor:+$monitor}"
}

# The country ID is set at the radio level. When the driver attaches the radio,
# it sets the default country ID to 840 (US STA). This is because the desired
# VAP modes are not known at radio attach time, and STA functionality is the
# common unit of 802.11 operation.
# If the user desires any of the VAPs to be in AP mode, then we set a new
# default of 843 (US AP with TDWR) from this script. Even if any of the other
# VAPs are in non-AP modes like STA or Monitor, the stricter default of 843
# will apply.
# No action is required here if none of the VAPs are in AP mode.
set_default_country() {
	local device="$1"
	local mode

	config_get phy "$device" phy

        # If the country parameter is number (either hex or decimal), we
        # assume it's a regulatory domain - i.e. we use iwpriv setCountryID.
        # Else we assume it's a country code - i.e. we use iwpriv setCountry.
        country=$(uci get wireless.$device.country)
        if [ `expr "$country" : '[0-9].*'` -ne 0 ]; then
                if [ "$country" = "00" -a "$domain" = "2" ]; then
                    iwpriv "$phy" setCountryID 528
                else
                    iwpriv "$phy" setCountryID "$country"
                fi
        elif [ -n "$country" ]; then
                iwpriv "$phy" setCountry "$country"
        else
	# The country ID is set at the radio level. When the driver attaches the radio,
	# it sets the default country ID to 840 (US STA). This is because the desired
	# VAP modes are not known at radio attach time, and STA functionality is the
	# common unit of 802.11 operation.
	# If the user desires any of the VAPs to be in AP mode, then we set a new
	# default of 843 (US AP with TDWR) from this script. Even if any of the other
	# VAPs are in non-AP modes like STA or Monitor, the stricter default of 843
	# will apply.
	# No action is required here if none of the VAPs are in AP mode.
		config_get vifs "$device" vifs
		for vif in $vifs; do
			config_get_bool disabled "$vif" disabled 0
			[ $disabled = 0 ] || continue

			config_get mode "$vif" mode
			case "$mode" in
				ap|wrap|ap_monitor|ap_smart_monitor|ap_lp_iot)
					iwpriv "$phy" setCountryID 843
					return 0;
				;;
			*) ;;
			esac
		done
	fi
	return 0
}

config_low_targ_clkspeed() { #non-using function now
        local board_name
        [ -f /tmp/sysinfo/board_name ] && {
                board_name=$(cat /tmp/sysinfo/board_name)
        }

        case "$board_name" in
                ap147 | ap151)
                   echo "true"
                ;;
                *) echo "false"
                ;;
        esac
}

# configure tx queue fc_buf_max
config_tx_fc_buf() {	#non-using function now
	local phy="$1"
	local board_name
	[ -f /tmp/sysinfo/board_name ] && {
		board_name=$(cat /tmp/sysinfo/board_name)
	}
	memtotal=$(grep MemTotal /proc/meminfo | awk '{print $2}')

	case "$board_name" in
		ap-dk*)
			if [ $memtotal -le 131072 ]; then
				# 4MB tx queue max buffer size
				iwpriv "$phy" fc_buf_max 4096
				iwpriv "$phy" fc_q_max 512
				iwpriv "$phy" fc_q_min 32
			elif [ $memtotal -le 256000 ]; then
				# 8MB tx queue max buffer size
				iwpriv "$phy" fc_buf_max 8192
				iwpriv "$phy" fc_q_max 1024
				iwpriv "$phy" fc_q_min 64
			fi
				# default value from code memsize > 256MB
		;;

		*)
		;;
	esac
}

preinit_boarddata(){
	local PART="$(grep "\"art\"" /proc/mtd | awk -F: '{print $1}')"
        if [ -z "$PART" ]; then 
		DebugPrint "==PART is empty, search ART instead of art=="
                PART="$(grep "\"ART\"" /proc/mtd | awk -F: '{print $1}')"
        fi
	[ ! -e "/tmp/wifi0.caldata" ] && {
		dd if=/dev/$PART of=/tmp/wifi0.caldata bs=4 count=272 skip=1024
	}
	[ ! -e "/tmp/wifi1.caldata" ] && {
		dd if=/dev/$PART of=/tmp/wifi1.caldata bs=4 count=529 skip=5120
	}
	local check_wifi2=$(/usr/sbin/foreach wireless wifi-device disabled 0| grep wifi2)

	[ -n "$check_wifi2" ] && [ ! -e "/tmp/wifi2.caldata" ] && {
		AR9887_PCI_DEVID="168c0050"
		QCA9888_PCI_DEVID="168c0056"
		PCI_DEVICE_ID_PATH="/proc/bus/pci/devices"

		if [ -n $(grep $AR9887_PCI_DEVID $PCI_DEVICE_ID_PATH | awk '{print $2}') ]; then    #ar9887, ex: EWS955AP scan radio
			dd if=/dev/$PART of=/tmp/wifi2.caldata bs=4 count=529 skip=9216
		elif [ -n $(grep $QCA9888_PCI_DEVID $PCI_DEVICE_ID_PATH | awk '{print $2}') ]; then #QCA9888, ex: EAP2200 triband AP
			dd if=/dev/$PART of=/tmp/wifi2.caldata bs=32 count=377 skip=1152
		else        #default use tri-band AP case
			dd if=/dev/$PART of=/tmp/wifi2.caldata bs=32 count=377 skip=1152
		fi
        }
}

confirm_boarddata(){
	local dev=$1
	local multi_project=$(grep -rn "modelName" /rom/etc/config/sysProductInfo | wc -l)

	[ -e "$ipq40xx_sh_file" ] && check_boarddata "$dev"

	[ -e "$qca98xx_sh_file" ] && {
		if [ $multi_project -gt 1 ]; then
			[ "$dev" == "wifi1" ] && check_boarddata_multi_project "$dev"
		else
			check_boarddata "$dev"
		fi
	}
}

load_qcawifi() {

	local qca_da_needed=0
	local qca_ol_needed=0
	local reload=0

	wifiModParamFile="/tmp/wifi_mod_param.txt"
	[ -e "$wifiModParamFile" ] && rm "$wifiModParamFile" -rf

	obey=$(/usr/sbin/foreach wireless wifi-device obeyregpower 1)
	if [ -z "$obey" ]; then
		obey=$(/usr/sbin/foreach wireless wifi-device obeyregpower undefined)
		if [ -z "$obey" ]; then
			obey=0
		else
			obey=1
		fi
	else
		obey=1
	fi
	
	support_high_power=$(get_sn_wifi_option SUPPORT_HIGH_POWER)
	support_green_mode=$(get_sn_wifi_option SUPPORT_GREEN_MODE)
	support_outdoor=$(get_sn_wifi_option SUPPORT_OUTDOOR_SLOTTIME_ACK_CTS_TIMEOUT)
	
	[ -n "$obey" ] && [ "$support_high_power" == "1" ] && {
		append ol_args "ac_obey_reg_power=$obey"
		param=$(get_module_parameter "qca_ol" "ac_obey_reg_power")
		[ $param != "$obey" ] && reload=1
	}
	[ -n "$obey" ] && [ "$support_green_mode" == "1" ] && {
		append hal_args "ah_obey_reg_power=$obey"
		param=$(get_module_parameter "ath_hal" "ah_obey_reg_power")
		[ $param != "$obey" ] && reload=1
	}

	outdoor=$(uci -q get sysProductInfo.model.outdoor)
	[ -n "$outdoor" ] && [ "$support_outdoor" == "1" ] && {
		append ol_args "outdoor_device_setting=$outdoor"
		param=$(get_module_parameter "qca_ol" "outdoor_device_setting")
		[ $param != "$outdoor" ] && reload=1
	}

	atfmode=$(uci -q get airtime_fairness.atf.enabled)
	[ -n "$atfmode" ] && {
		append umac_args "atf_mode=$atfmode"
		param=$(get_module_parameter "umac" "atf_mode")
                [ $param != "$atfmode" ] && reload=1
	}
	[ $RELOAD_WIFI_WITHOUT_KMODS == "1" ] && [ $reload == "1" ] && unload_qcawifi	#jacky: need to unload wifi modules if those parameter changed

	for mod in $(cat /etc/modules.d/33-qca-wifi*); do
		case ${mod} in
			umac) [ -d /sys/module/${mod} ] || { \

				insmod ${mod} ${umac_args} || { \
					lock -u /var/run/wifilock
					unload_qcawifi
					return 1
				}
			};;

			qdf) [ -d /sys/module/${mod} ] || { \
				insmod ${mod} ${qdf_args} || { \
					lock -u /var/run/wifilock
					unload_qcawifi
					return 1
				}
			};;

			qca_ol) [ -f /tmp/no_qca_ol ] || { \
					[ -d /sys/module/${mod} ] || { \
					insmod ${mod} ${ol_args} || { \
						lock -u /var/run/wifilock
						unload_qcawifi
						return 1
					}
				}
			};;

			ath_dev) [ -d /sys/module/${mod} ] || { \
				insmod ${mod} ${ath_dev_args} || { \
					lock -u /var/run/wifilock
					unload_qcawifi
					return 1
				}
			};;

			ath_hal) [ -d /sys/module/${mod} ] || { \
				insmod ${mod} ${hal_args} || { \
					lock -u /var/run/wifilock
					unload_qcawifi
					return 1
                                }
                        };;

			qca_da) [ -f /tmp/no_qca_da ] || { \
				[ -d /sys/module/${mod} ] || { \
					insmod ${mod} || { \
						lock -u /var/run/wifilock
						unload_qcawifi
						return 1
					}
				}
			};;

			ath_pktlog) [ $enable_pktlog_support -eq 0 ] || { \
				[ -d /sys/module/${mod} ] || { \
					insmod ${mod} || { \
						lock -u /var/run/wifilock
						unload_qcawifi
						return 1
					}
				}
			};;

			*) [ -d /sys/module/${mod} ] || { \
				insmod ${mod} || { \
					lock -u /var/run/wifilock
					unload_qcawifi
					return 1
				}
			};;

		esac

		echo ${mod} ${args} >> "$wifiModParamFile"
	done

       # Remove DA/OL modules, if no DA/OL chipset found
	for dev in $(ls -d /sys/class/net/wifi* 2>&-); do
		[[ -f $dev/is_offload ]] || {
			qca_da_needed=1
		}
		[[ -f $dev/is_offload ]] && {
			qca_ol_needed=1
		}
	done

	if [ $qca_ol_needed -eq 0 ]; then
		if [ ! -f /tmp/no_qca_ol ]; then
			echo "No offload chipsets found." >/dev/console
			rmmod qca_ol > /dev/null 2> /dev/null
			cat "1" > /tmp/no_qca_ol
		fi
	fi

	if [ $qca_da_needed -eq 0 ]; then
		if [ ! -f /tmp/no_qca_da ]; then
			echo "No Direct-Attach chipsets found." >/dev/console
			rmmod qca_da > /dev/null 2> /dev/null
			cat "1" > /tmp/no_qca_da
		fi
	fi


	[ -f /etc/modules.d/50-batman-adv* ] && {
		for mod in $(cat /etc/modules.d/50-batman-adv* | sed 's/-/_/'); do
			[ -d /sys/module/${mod} ] || insmod $(mod)
		done
	}
}

unload_qcawifi() {
	for mod in $(cat /etc/modules.d/33-qca-wifi* | sed '1!G;h;$!d'); do
		[ -d /sys/module/${mod} ] && rmmod ${mod}
	done

	[ -e "/etc/modules.d/50-batman-adv*" ] && {
		for mod in $(cat /etc/modules.d/50-batman-adv* | sed 's/-/_/'); do      
			[ -d /sys/module/${mod} ] || rmmod ${mod}
		done
	}
}

destroy_vap() {
	#SENAO WPA3's function
	support_wpa3=$(get_sn_wifi_option SUPPORT_QCA_WPA3)
	
	if [ "$support_wpa3" == "0" ]; then
		return
	fi

	local ifname="$1"
        ifconfig $ifname down
        wlanconfig $ifname destroy
}

disable_qcawifi() {
	local device="$1"
	local parent

	local led_on wifi_led_config

	if [ "$device" = "wifi1" ]; then
		wifi_led_config=$(uci -q get system.wifi1_led.default || echo 1)
		led_on=$((1 ^ $wifi_led_config))
		[ -e "/sys/class/leds/wlan_5g_led/brightness" ] && echo "${led_on}" > /sys/class/leds/wlan_5g_led/brightness
	fi

	if [ "$device" = "wifi0" ]; then
		wifi_led_config=$(uci -q get system.wifi0_led.default || echo 1)
		led_on=$((1 ^ $wifi_led_config))
		[ -e "/sys/class/leds/wlan_2g_led/brightness" ] && echo "${led_on}" > /sys/class/leds/wlan_2g_led/brightness
	fi

	disable_fastscan
	disable_scan_radio "$device"
	disable_background_scan "$device"

	find_qcawifi_phy "$device" || return 0
	config_get phy "$device" phy

	set_wifi_down "$device"

	include /lib/network
	cd /sys/class/net
	for dev in *; do
		[ -f /sys/class/net/${dev}/parent ] && { \
			local parent=$(cat /sys/class/net/${dev}/parent)
			[ -n "$parent" -a "$parent" = "$device" ] && { \
				#new flow, to kill wifi related processes which use wpa_cli and global
				[ -f "/var/run/hostapd-${dev}.lock" ] && { \
					wpa_cli -g /var/run/hostapd/global raw REMOVE ${dev}
					rm /var/run/hostapd-${dev}.lock
				}
				[ -f "/var/run/wpa_supplicant-${dev}.lock" ] && { \
					wpa_cli -g /var/run/wpa_supplicantglobal  interface_remove  ${dev}
					rm /var/run/wpa_supplicant-${dev}.lock
				}
				[ -f "/var/run/wapid-${dev}.conf" ] && { \
					kill "$(cat "/var/run/wifi-${dev}.pid")"
				}
				#end

				#traditional flow to kill wifi related processes
				[ -f "/var/run/wifi-${dev}.pid" ] && { \
					kill "$(cat "/var/run/wifi-${dev}.pid")"
				}
				#end
				
				#unbridge "$dev" #jacky: it's empty in config.sh, do unbridge in notify_ubus_to_unbridge before interface down
				notify_ubus_to_unbridge "$dev"

				#remove mesh interface from batman-adv before destroy it
				if [ "$dev" == "ath32" -o "$dev" == "ath35" -o "$dev" == "ath37" ]; then
					batctl if del "$dev"
				fi

				ifconfig "$dev" down
				wlanconfig "$dev" destroy
			}
			
			[ -f /var/run/hostapd_cred_${device}.bin ] && { \
				rm /var/run/hostapd_cred_${device}.bin
			}
		}
	done

	nrvaps=$(cat /sys/class/net/*/parent | grep "wifi" |wc -l)
	[ ${nrvaps} -gt 0 ] || {
		[ "$RELOAD_WIFI_WITHOUT_KMODS" != '1' ] && unload_qcawifi
# for traditional flow, remove it.
		hostapd_global_restart                  #Jacky: for hostapd connection refused issue, close hostapd global when wifi down
	}

	return 0
}


#
# Read ethaddr at /tmp/
# wifi0_mac_file="/tmp/wifi0_mac.txt"
# wifi1_mac_file="/tmp/wifi1_mac.txt"
# wifi2_mac_file="/tmp/wifi2_mac.txt"
#
set_mac()
{
	local wifi_dev=$1
	macaddr_file="/tmp/"$wifi_dev"_mac.txt"

	if [ -r $macaddr_file ]; then
		local macaddr=$(cat $macaddr_file)
		iwpriv $wifi_dev setHwaddr $macaddr 2>/dev/null
	else
		echo "qcawifi.sh can not read $macaddr_file" >> $error_msg
	fi
}

set_txpower()
{
# TXPower settings only work if device is up already
# while atheros hardware theoretically is capable of per-vif (even per-packet) txpower
# adjustment it does not work with the current atheros hal/madwifi driver
	local device=$1
	local vif=$2
	config_get txpower $device txpower
	config_get obey $device obeyregpower
	config_get phy $device phy

	if [ -z "$obey" ]; then
		# If project doesn't support green_mode,
		# just need to set txpower.
		config_get vif_txpower "$vif" txpower
		txpower="${txpower:-$vif_txpower}"
		[ -z "$txpower" ] || iwconfig "$ifname" txpower "${txpower%%.*}"
		echo "$device no green mode" >> /tmp/noGreenMode.txt
		return 0
	fi

	# Check Config come from ezmcloud
	if [ ! -f /etc/config/ezmcloud ]; then
		# Not come from ezmcloud will run orignal method
		if [ "$txpower" == "0" ]; then
			# We always set maximum power 31(no value will be bigger than this value)
			# and driver will check ctl table to handle this value into correct power.
			[ "$obey" == "0" ] && txpower=31
		else
			[ "$obey" == "1" ] && txpower=0
		fi
	fi

	iwconfig "$ifname" txpower "${txpower%%.*}"
}

cpu_irq_setup(){

        local board_name
        local wifi0_irq
        local wifi1_irq
        local wifi2_irq

        [ -f /tmp/sysinfo/board_name ] && {
                board_name=$(cat /tmp/sysinfo/board_name)
        }

        case "$board_name" in
                ap-dk*)
                        wifi0_irq=$(cat /proc/interrupts | grep wifi0 | awk 'gsub(":","",$1) {print $1}')
                        wifi1_irq=$(cat /proc/interrupts | grep wifi1 | awk 'gsub(":","",$1) {print $1}')
                        wifi2_irq=$(cat /proc/interrupts | grep wifi2 | awk 'gsub(":","",$1) {print $1}')
                        if [ -n "$wifi0_irq" ]; then
                                if [ -f /proc/irq/${wifi0_irq}/smp_affinity ]; then
                                        echo 2 > /proc/irq/${wifi0_irq}/smp_affinity
                                fi
                        fi
                        if [ -n "$wifi1_irq" ]; then
                                if [ -f /proc/irq/${wifi1_irq}/smp_affinity ]; then
                                        echo 4 > /proc/irq/${wifi1_irq}/smp_affinity
                                fi
                        fi
                        if [ -n "$wifi2_irq" ]; then
                                if [ -f /proc/irq/${wifi2_irq}/smp_affinity ]; then
                                        echo 8 > /proc/irq/${wifi2_irq}/smp_affinity
                                fi
                	fi
                ;;
                ap148*)
                        if [ -f /proc/irq/141/smp_affinity ]; then
                                echo 2 > /proc/irq/141/smp_affinity
                        fi
		;;
                *)
                ;;
        esac
}

enable_qcawifi() {
	local device="$1"

	local led_on wifi_led_config

	if [ "$device" = "wifi1" ]; then
		wifi_led_config=$(uci -q get system.wifi1_led.default || echo 1)
		led_on=$((1 ^ $wifi_led_config))
		[ -e "/sys/class/leds/wlan_5g_led/brightness" ] && echo "${led_on}" > /sys/class/leds/wlan_5g_led/brightness
	fi

	if [ "$device" = "wifi0" ]; then
		wifi_led_config=$(uci -q get system.wifi0_led.default || echo 1)
		led_on=$((1 ^ $wifi_led_config))
		[ -e "/sys/class/leds/wlan_2g_led/brightness" ] && echo "${led_on}" > /sys/class/leds/wlan_2g_led/brightness
	fi

	preinit_boarddata	#to prevent etc/preinit not prepared caldata
	confirm_boarddata "$device"
	check_offload
	load_qcawifi
	cpu_irq_setup
	local setchannel=0;
	local domain=$(setconfig -g 4)
	local ac_mode=0;

	enable_debug_wlan_script "$device"

	dfs_spoof "$device"

	green_mode "$device"

	set_mac "$device"

    # SENAO support generate vap mac before interface up
    [ -e "/usr/sbin/bsGetVapMac.sh" ] && bsGetVapMac.sh

	find_qcawifi_phy "$device" || return 0
	config_get phy "$device" phy

	# support SENAO outdoor distance parameter
        local outdoor=$(uci get sysProductInfo.model.outdoor)
        config_get distance "$device" distance
	[ -n "$distance" -a "$outdoor" = "1" ] && distance_func "$device" "$phy"
	[ -n "$distance" ] && distance_ack "$device" "$phy"

	#SENAO set default country, wireless.wifix.country may be 00 as default, need set it here by regdomain
	set_default_country "$device"

	config_get channel "$device" channel
	config_get vifs "$device" vifs

	[ "auto" == "$channel" ] && channel=0

	#set wifi params by radio, ex: iwpriv wifi1 xxxxx
	set_wifi_param_per_radio "$device"

	config_get opmode "$device" opmode
	
	case "$opmode" in
		ap|wds_ap|wds_bridge|sta_ap)
	
		#SENAO support channel config setting
		set_channel_config "$device"

		#SENAO support client limit setting
		set_client_limit "$device"	

		#SENAO support fasthandover
		set_sn_fasthandover "$device"

		config_get nochannel "$device" nochannel 0 # Is radio no channel supported

		[ "$nochannel" != "1" ] && {
			create_wifi_vaps "$device"

			#set wifi params by vap, ex: iwpriv athx xxxx
			set_wifi_param_per_vap "$device"

			#SENAO fix rate related issues
			sn_fix_rate_issue_func "$device"

			#SENAO set channel after all vap set
			sn_set_channel_final

			set_beacon_interval "$device"

			#set nawds mode or mesh
			set_nawds_or_mesh_mode "$device"

			chk_wds_ap

			#SENAO fix HT20_40 issue
			sn_fix_HT20_40_issue_func "$device"

			#SENAO enable fastscan feature
			enable_fastscan "$device"

			#SENAO set vlan passthrough
			set_vlan_passthrough

			#SENAO set airtime fairness
			set_airtime_fairness_vip

			#SENAO set legacy deny
			set_legacy_deny "$device"

			set_background_scan "$device"

			set_scan_radio "$device"
		}

		;;

		sta|wds_sta)
			set_channel_config "$device"
			create_wifi_vaps "$device"
			set_wifi_param_per_vap "$device"
		;;
		mon)
			#SENAO set scan radio
			create_wifi_vaps "$device"
                        set_scan_radio "$device"
		;;
	esac
	
	#SENAO if no vap on the radio, create dummy vap
        create_dummy_interfaces "$device"
	echo 1 #without this makes _wifi_updown in /sbin/wifi shows "enable failed"

	#senao log get bridge mac address
	set_bridge_mac_to_proc

	set_11k_prescan "$device"
}

pre_qcawifi() {
	local action=${1}

	config_load wireless

	case "${action}" in
		disable)
			config_get_bool wps_vap_tie_dbdc qcawifi wps_vap_tie_dbdc 0

			if [ $wps_vap_tie_dbdc -ne 0 ]; then
				kill "$(cat "/var/run/hostapd.pid")"
				[ -f "/tmp/hostapd_conf_filename" ] &&
					rm /tmp/hostapd_conf_filename
			fi

			eval "type qwrap_teardown" >/dev/null 2>&1 && qwrap_teardown
			eval "type icm_teardown" >/dev/null 2>&1 && icm_teardown
			eval "type wpc_teardown" >/dev/null 2>&1 && wpc_teardown
			eval "type lowi_teardown" >/dev/null 2>&1 && lowi_teardown
			[ ! -f /etc/init.d/lbd ] || /etc/init.d/lbd stop
			[ ! -f /etc/init.d/hyd ] || /etc/init.d/hyd stop
			[ ! -f /etc/init.d/ssid_steering ] || /etc/init.d/ssid_steering stop
			[ ! -f /etc/init.d/mcsd ] || /etc/init.d/mcsd stop
			[ ! -f /etc/init.d/wsplcd ] || /etc/init.d/wsplcd stop

			rm -f /var/run/wifi-wps-enhc-extn.conf
			[ -r /var/run/wifi-wps-enhc-extn.pid ] && kill "$(cat "/var/run/wifi-wps-enhc-extn.pid")"
		;;
	esac

	wifiProfile2Config

	[ "$(uci -q get mesh.wifi.supportRP)" == "1" ] && mesh_RP_config
}

post_qcawifi() {
	local action=${1}

	case "${action}" in
		enable)
			local icm_enable qwrap_enable lowi_enable

			# Run a single hostapd instance for all the radio's
			# Enables WPS VAP TIE feature

			config_get_bool wps_vap_tie_dbdc qcawifi wps_vap_tie_dbdc 0

			if [ $wps_vap_tie_dbdc -ne 0 ]; then
				hostapd_conf_file=$(cat "/tmp/hostapd_conf_filename")
				hostapd -P /var/run/hostapd.pid $hostapd_conf_file -B
				config_foreach qcawifi_start_hostapd_cli wifi-device
			fi

			config_get_bool icm_enable icm enable 0
			[ ${icm_enable} -gt 0 ] && \
					eval "type icm_setup" >/dev/null 2>&1 && {
				icm_setup
			}

			config_get_bool wpc_enable wpc enable 0
			[ ${wpc_enable} -gt 0 ] && \
					eval "type wpc_setup" >/dev/null 2>&1 && {
				wpc_setup
			}

			config_get_bool lowi_enable lowi enable 0
			[ ${lowi_enable} -gt 0 ] && \
				eval "type lowi_setup" >/dev/null 2>&1 && {
				lowi_setup
			}

			eval "type qwrap_setup" >/dev/null 2>&1 && qwrap_setup && disable_qcawifi

			# These init scripts are assumed to check whether the feature is
			# actually enabled and do nothing if it is not.
			[ ! -f /etc/init.d/lbd ] || /etc/init.d/lbd start
			[ ! -f /etc/init.d/ssid_steering ] || /etc/init.d/ssid_steering start
			[ ! -f /etc/init.d/wsplcd ] || /etc/init.d/wsplcd start

			config_get_bool wps_pbc_extender_enhance qcawifi wps_pbc_extender_enhance 0
			[ ${wps_pbc_extender_enhance} -ne 0 ] && { \
				rm -f /var/run/wifi-wps-enhc-extn.conf
				setup_wps_enhc
			}
		;;
	esac

	set_wmm_dscp
	[ "$(uci -q get mesh.wifi.supportRP)" == "1" ] && reset_mesh_for_RP
    fix_fastroam
}

check_qcawifi_device() {
	[ ${1%[0-9]} = "wifi" ] && config_set "$1" phy "$1"
	config_get phy "$1" phy
	[ -z "$phy" ] && {
		find_qcawifi_phy "$1" >/dev/null || return 0
		config_get phy "$1" phy
	}
	[ "$phy" = "$dev" ] && found=1
}

detect_qcawifi() {
	devidx=0
	load_qcawifi
	config_load wireless
	while :; do
		config_get type "radio$devidx" type
		[ -n "$type" ] || break
		devidx=$(($devidx + 1))
	done
	cd /sys/class/net
	[ -d wifi0 ] || return
	for dev in $(ls -d wifi* 2>&-); do
		found=0
		config_foreach check_qcawifi_device wifi-device
		[ "$found" -gt 0 ] && continue

		hwcaps=$(cat ${dev}/hwcaps)
		case "${hwcaps}" in
			*11bgn) mode_11=ng;;
			*11abgn) mode_11=ng;;
			*11an) mode_11=na;;
			*11an/ac) mode_11=ac;;
			*11abgn/ac) mode_11=ac;;
		esac

		cat <<EOF
config wifi-device  wifi$devidx
	option type	qcawifi
	option channel	auto
	option macaddr	$(cat /sys/class/net/${dev}/address)
	option hwmode	11${mode_11}
	# REMOVE THIS LINE TO ENABLE WIFI:
	option disabled 1

config wifi-iface
	option device	wifi$devidx
	option network	lan
	option mode	ap
	option ssid	OpenWrt
	option encryption none

EOF
	devidx=$(($devidx + 1))
	done
}

enable_debug_wlan_script() {
        #debug wlan-script mode
	local wifi_dev=$1
        local dbglvl=$(setconfig -g 10)
        if [ "$dbglvl" -eq 2 ]; then
                case "$wifi_dev" in
                        wifi0)
                                dl.sh -d 2
                                sh /tmp/athwlan0.sh
                        ;;
                        wifi1)
                                dl.sh -d 3
                                sh /tmp/athwlan1.sh
                        ;;
                        wifi2)
                                dl.sh -d 3
                                sh /tmp/athwlan2.sh
                        ;;
                esac
                return;
        fi
}

set_wifi_param_per_radio() {

	local wifi_dev=$1
	config_get phy "$wifi_dev" phy
	# Advanced QCA wifi per-radio parameters configuration
	config_get txchainmask "$wifi_dev" txchainmask
	[ -n "$txchainmask" ] && iwpriv "$phy" txchainmask "$txchainmask"

	config_get rxchainmask "$wifi_dev" rxchainmask
	[ -n "$rxchainmask" ] && iwpriv "$phy" rxchainmask "$rxchainmask"

        config_get_bool bcnburst "$wifi_dev" bcnburst 0
        [ "$bcnburst" -gt 0 ] && iwpriv "$phy" set_bcnburst "$bcnburst"

	config_get opmode "$device" opmode
	config_get qboost_enable "$device" qboost_enable 0
	#  INTEL IOT issue. It makes tdma function abnormal, so we will not to set it when sta mode or TDMA enabled
	[ "$opmode" == "sta" ] || [ "$qboost_enable" == "1" ] || iwpriv "$phy" burst 1
        
	iwpriv "$phy" dcs_enable 0 # adaptive issue
	if [ "$(uci -q get wifiprofile.snWifiConf.SUPPORT_ACS_ENHANCE_RANDOMIZE_SELECTION)" == "1" ]; then
		iwpriv "$phy" acs_2g_allch 1
	fi

}

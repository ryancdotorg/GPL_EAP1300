#!/bin/sh

. /lib/functions.sh

DEBUG=

##### Config without section name ######
local wifi_configs=$(uci show | grep @wifi-iface\\[0]=wifi-iface)
echo "check wifi section name." > /dev/console

[ -n "$wifi_configs" ] && {
    echo "start rename wifi section name." > /dev/console

    uci rename wireless.@wifi-iface[0]=wifi0_ssid_1
    uci rename wireless.@wifi-iface[1]=wifi0_ssid_2
    uci rename wireless.@wifi-iface[2]=wifi0_ssid_3
    uci rename wireless.@wifi-iface[3]=wifi0_ssid_4
    uci rename wireless.@wifi-iface[4]=wifi0_ssid_5
    uci rename wireless.@wifi-iface[5]=wifi0_ssid_6
    uci rename wireless.@wifi-iface[6]=wifi0_ssid_7
    uci rename wireless.@wifi-iface[7]=wifi0_ssid_8
    uci rename wireless.@wifi-iface[8]=wifi1_ssid_1
    uci rename wireless.@wifi-iface[9]=wifi1_ssid_2
    uci rename wireless.@wifi-iface[10]=wifi1_ssid_3
    uci rename wireless.@wifi-iface[11]=wifi1_ssid_4
    uci rename wireless.@wifi-iface[12]=wifi1_ssid_5
    uci rename wireless.@wifi-iface[13]=wifi1_ssid_6
    uci rename wireless.@wifi-iface[14]=wifi1_ssid_7
    uci rename wireless.@wifi-iface[15]=wifi1_ssid_8

    uci set wireless.wifi1.hwmode=11ac
    uci set wireless.wifi0.hwmode=11ng

    uci set wireless.wifi1.rts=2346
    uci set wireless.wifi0.rts=2346

    uci commit
}
##############################################

##### Correct the fastroaming config #####
[ -n "$(uci get wireless.wifi0.fastroamingEnable)" ] && {
    [ "$(uci get wireless.wifi0.fastroamingEnable)" == "1" ] && {
        uci set wireless.wifi0_ssid_1.fastroamingEnable=1
    }
    uci delete wireless.wifi0.fastroamingEnable
    [ "$(uci get wireless.wifi1.fastroamingEnable)" == "1" ] && {
        uci set wireless.wifi1_ssid_1.fastroamingEnable=1
    }
    uci delete wireless.wifi1.fastroamingEnable

    uci commit
}
##############################################

if [ "$(uci get functionlist.functionlist.SUPPORT_WLAN5G_2 2> /dev/null)" == "1" ]; then
    devices="wifi0 wifi1 wifi2"
else
    devices="wifi0 wifi1"
fi

sed -i 's/psk-mixed/psk2/g' /etc/config/wireless
sed -i 's/wpa-mixed/wpa2/g' /etc/config/wireless
sed -i 's/tkip+ccmp/ccmp/g' /etc/config/wireless

get_section() {
lua - <<LUA_END
#!/usr/bin/lua
    local uci = require("luci.model.uci").cursor()
    local ntm = require "luci.model.network".init()
    local devices = ntm:get_wifidevs()
    local opmode = {}
    local choice = {}
    local top_tbl = {}
    local top_gn_tbl = {}
    local wds_count = 0

    for _, dev in ipairs(devices) do
        opmode[dev:name()] = uci:get("wireless", dev:name(), "opmode")
        if opmode[dev:name()] == "wds_ap" or opmode[dev:name()] == "wds_bridge" then
            wds_count = wds_count + 1
        end
        if wds_count > 1 then
            uci:set("wireless", dev:name(), "opmode", "ap")
            uci:save("wireless")
            opmode[dev:name()] = "ap"
        end
    end

    function check_ssid(section, ssid)
        local set = {}
        local idx
        local wifi
        local chksum = 0
        local band = 0
        if uci:get("functionlist", "functionlist", "SUPPORT_WLAN5G_2") == '1' then
            band = 2
        else
            band = 1
        end
        
        if section:match("guest") then
            for i=0,band do
                if uci:get("wireless", "wifi"..i.."_guest", "disabled") == '0' and ssid == uci:get("wireless", "wifi"..i.."_guest", "ssid") and uci:get("wireless", "wifi"..i, "opmode") == 'ap' then
                    set[i] = 1
                    chksum = chksum + 1
                else
                    set[i] = 0
                end
            end
        else
            wifi, idx = section:match("wifi(%S+)_ssid_(%S+)")
            for i=0,band do
                if uci:get("wireless", "wifi"..i.."_ssid_"..idx, "disabled") == '0' and ssid == uci:get("wireless", "wifi"..i.."_ssid_"..idx, "ssid") and uci:get("wireless", "wifi"..i, "opmode") == 'ap' then 
                    set[i] = 1
                    chksum = chksum + 1
                else
                    set[i] = 0
                end
            end
        end

        return set, chksum
    end
    function check_encrType(encr)
        if not (string.match(encr, "wep") or string.match(encr, "psk[+-]") or string.match(encr, "wpa[+-]") or string.match(encr, "tkip")) then
            return true
        end
        return false
    end
    function check_encryption(section, encr, key)
        if section:match("wifi0") then
            if encr == uci:get("wireless", section:gsub("%wifi0", "wifi1"), "encryption") then
                if key == uci:get("wireless", section:gsub("%wifi0", "wifi1"), "key") then
                    return true
                end
            end
        else
            if encr == uci:get("wireless", section:gsub("%wifi1", "wifi0"), "encryption") then 
                if key == uci:get("wireless", section:gsub("%wifi1", "wifi0"), "key") then
                    return true
                end
            end
        end
        return false
    end

    uci:foreach("wireless", "wifi-iface", function(s)
        local same, ignore = 0, 0
        local syncssid = {}
        local chksum = 0

        --os.execute("echo ==============s.ssid:"..s.ssid.."========s.disabled:"..s.disabled.."=======opmode[s.device]:"..opmode[s.device].."============= >/dev/console")
        
        if s.disabled == '0' and s.mode_display == 'ap' and opmode[s.device] == 'ap' and check_encrType(s.encryption) then

            syncssid, chksum = check_ssid( s[".name"], s.ssid)    -- check SSID
            --luci.util.exec("echo "..s[".name"].." ssid same >/dev/console")
           
            --for i=0, 2 do
            --    os.execute("echo ==============syncssid["..i.."]:"..syncssid[i].."================ >/dev/console") 
            --end

            if (chksum > 1) then
                if s.device == 'wifi1' and syncssid[0] == 1 then
                    ignore = 1
                else
                    same = 1
                end
                if s.device == 'wifi2' and ( syncssid[0] == 1 or syncssid[1] == 1 ) then
                    ignore = 1
                else
                    same = 1
                end
            end

            if ignore == 0 then
                if string.match(s[".name"], "guest") then 
                    if same == 1 then
                        if uci:get("functionlist", "functionlist", "SUPPORT_WLAN5G_2") == '1' then
                            table.insert(top_gn_tbl, "s:"..s[".name"]..",1:"..syncssid[0]..",2:"..syncssid[1]..",3:"..syncssid[2])
                        else
                            table.insert(top_gn_tbl, "s:"..s[".name"]..",1:"..syncssid[0]..",2:"..syncssid[1])
                        end
                    else
                        table.insert(top_gn_tbl, s[".name"])
                    end
                elseif same == 1 then  
                    if uci:get("functionlist", "functionlist", "SUPPORT_WLAN5G_2") == '1' then
                        table.insert(top_tbl, "s:"..s[".name"]..",1:"..syncssid[0]..",2:"..syncssid[1]..",3:"..syncssid[2])
                    else
                        table.insert(top_tbl, "s:"..s[".name"]..",1:"..syncssid[0]..",2:"..syncssid[1])
                    end
                else
                    table.insert(choice, s[".name"])
                end
            end
        end
    end)

    if next(top_tbl) ~= nil then
        for n, m in ipairs(top_tbl) do
            table.insert(choice, 1, top_tbl[#top_tbl - n + 1])
        end
    end

    if next(top_gn_tbl) ~= nil then
        for o, p in ipairs(top_gn_tbl) do
            table.insert(choice, 1, top_gn_tbl[#top_gn_tbl - o + 1])
        end
    end

    for i, sec in ipairs(choice) do
        if tonumber(i) <= 8 then 
           os.execute("echo "..sec.."") 
        end
    end
LUA_END
}
sync_sec=$(get_section)
[ -n "${DEBUG}" ] && echo "---------debug--------sync_sec:$sync_sec------"

sync_option() {
    local o_sec=$1
    local n_sec=$2
    local opt=$3
    config_get val $o_sec $opt
    if [ -n "$val" ] || [ -n "$4" ]; then 
        [ -n "${DEBUG}" ] && echo "---------debug--------o_sec:$o_sec---n_sec:$n_sec---opt:$opt---val:$val----4:$4---"
        uci set wireless.$n_sec.$opt="${4:-$val}"
        case "${opt}" in
            "nasid_enable")
                [ "$val" != "0" ] && sync_option $o_sec $n_sec nasid
                ;;
            "nasport_enable")
                [ "$val" != "0" ] && sync_option $o_sec $n_sec nasport
                ;;
            "nasip_enable")
                [ "$val" != "0" ] && sync_option $o_sec $n_sec nasip
                ;;
            "acct_enabled")
                local subopts="acct_server acct_port acct_secret acct_interval"
                [ "$val" != "0" ] && { 
                    for subopt in $subopts; do
                        sync_option $o_sec $n_sec $subopt 
                    done
                }
                ;;
            "macfilter")
                if [ "$val" == "allow" ]; then
                    config_get list $o_sec allowmaclist
                    uci set wireless.$n_sec.allowmaclist="${list}"
                elif [ "$val" == "deny" ]; then 
                    config_get list $o_sec denymaclist
                    uci set wireless.$n_sec.denymaclist="${list}"
                fi
                ;;
            "tc_enabled")
                if [ "$val" != "0" ]; then
                    for i in "down" "up"; do
                        config_get peruser $o_sec "tc_${i}peruser"
                        config_get limit $o_sec "tc_${i}limit"
                        if [ "$peruser" == "1" ]; then
                            uci set wireless.$n_sec."tc_${i}limit"="$limit"
                            uci set wireless.$n_sec."tc_${i}maxlimit"=0
                        else
                            uci set wireless.$n_sec."tc_${i}limit"=0
                            uci set wireless.$n_sec."tc_${i}maxlimit"="$limit"
                        fi
                    done
                fi
                ;;
        esac
    fi
}

local opts="ssid encryption key wpa_group_rekey auth_server auth_port auth_secret hidden isolate isolation l2_isolatior vlan_id acct_enabled nasid_enable nasip_enable nasport_enable fastroamingEnable macfilter tc_enabled mcastenhance"
sync_section() {
    local idx=$1
    local sec=$2
    local same=0
    local chk1 chk2 chk3
    local i=2
    ## ----------check if both 2.4G and 5G (5G-1, 5G-2) enable-----------------------
    [ "$(uci get functionlist.functionlist.SUPPORT_WLAN5G_2 2> /dev/null)" == "1" ] && i=3

    [ -z "${sec##*s:*}" ] && {
        same=1
        while [ "${i}" != "0" ];
        do
            eval chk$i=${sec##*$i:}
            [ -n "${DEBUG}" ] && eval echo "==============chk$i:\$chk$i" >/dev/console
            sec=${sec%%,$i*}
            i=$(($i-1))
        done
        sec=${sec##s:} 
    }

    wifix=$(echo $sec | awk -F '_' '{printf $1}')
    config_get bandsteer wifi0 bandsteer
    config_get bandsteerrssi wifi0 bandsteerrssi
    config_get bandsteerpersent wifi0 bandsteerpersent

    for device in $devices; do
        if [ "$same" == "1" ]; then
            if [ "$device" == "wifi0" -a "$chk1" == "1" ] || [ "$device" == "wifi1" -a "$chk2" == "1" ] || [ "$device" == "wifi2" -a "$chk3" == "1" ]; then
                uci set wireless."${device}_ssid_$idx".disabled=0
            else
                uci set wireless."${device}_ssid_$idx".disabled=1
            fi
            if [ "$bandsteer" != "0" ]; then
                uci set wireless."${device}_ssid_$idx".bandsteer_en="$bandsteer"
                uci set wireless."${device}_ssid_$idx".bandsteerrssi="$bandsteerrssi"
                uci set wireless."${device}_ssid_$idx".bandsteerpersent="$bandsteerpersent"
            fi
        else
            if [ "$wifix" == "$device" ]; then 
                uci set wireless."${wifix}_ssid_$idx".disabled=0
            else
                uci set wireless."${device}_ssid_$idx".disabled=1
            fi
            uci set wireless."${device}_ssid_$idx".bandsteer_en=0
        fi
        for opt in $opts; do
            sync_option $sec "${device}_ssid_$idx" $opt 
        done
        ## ----------check if guest network-----------------------
        if [ -z "${sec##*guest*}" ];then
            [ -n "${DEBUG}" ] && echo "---------debug------Guest Network ${device}_ssid_$idx-----"
            uci set dhcp.guest.ignore=0
            sync_option $sec "${device}_ssid_$idx" guest_network "Enable"
            sync_option $sec "${device}_ssid_$idx" acct_enabled "0"
        fi
    done
}

set_opt(){
    for i in $1; do
        [ -n "${DEBUG}" ] && echo "---------debug------uci set $i-----"
        uci set $i 2> /dev/null
    done
}

del_opt(){
    local section=$1
    local config=$2
    for opt in $config; do
        [ -n "${DEBUG}" ] && echo "---------debug------uci delete wireless.$section.$opt-----"
        uci delete wireless.$section.$opt 2> /dev/null
    done
}

config_load wireless
## ----------------Init-----------------
uci set dhcp.guest.ignore=1

macfilteropts="macfilter denymaclist allowmaclist"
tcopts="tc_dwonperuser tc_upperuser tc_uplimit tc_downlimit nasport"
acctopts="nasport acct_server acct_port acct_secret acct_interval"
wdsbr="WLANWDSPeer wlanwdswepkey"

for dev in $devices; do
    # -------- AP interface reset --------
    for idx in $(seq 1 8); do
        cmd=`eval uci -c /rom/etc/config show wireless.${dev}_ssid_${idx}`
        eval ${dev}_ssid_${idx}_conf=\$cmd
        set_opt "$(eval echo \$${dev}_ssid_${idx}_conf)"
        # -------- Delete options which not exist in default config --------
        del_opt "${dev}_ssid_${idx}" "$macfilteropts $tcopts $acctopts"
    done
    # -------- Guest network reset --------
    cmd_gn=`eval uci -c /rom/etc/config show wireless.${dev}_guest`
    eval ${dev}_gn_conf=\$cmd_gn
    set_opt "$(eval echo \$${dev}_gn_conf)"
    # -------- Delete options which not exist in default config --------
    del_opt "${dev}_guest" "$macfilteropts $acctopts"
done

# -------- Non-AP mode reset config if setting not support encryption --------
# Check for WDS Bridge, Repeater mode.
for section in `uci show wireless | grep encr | grep 'wep\|wpa+\|wpa-\|psk+\|psk-\|tkip' |  cut -d "." -f2 | grep -v 'ssid' | grep -v 'guest'`;do
    # skip CB and WDS STA mode.
    [ "$(uci get wireless.${section}.mode)" == "sta" ] && [ "$(uci get wireless.${section}.mode_display)" != "sta_ap" ] && [ "$(uci show wireless | grep ${section} | grep encr | grep 'wep\|psk+\|tkip')" != "" ] || {
        # skip WDS AP mode.
        [ "$(uci get wireless.${section}.mode_display)" != "wds_ap" ] && {
            cmd_reset=`eval uci -c /rom/etc/config show wireless.${section}`
            eval ${section}_conf=\$cmd_reset
            set_opt "$(eval echo \$${section}_conf)"
            # -------- Delete options which not exist in default config --------
            del_opt "${section}" "$macfilteropts $tcopts $acctopts $wdsbr"
        }
    }
done

# Check WDS AP mode.
local valid_count=0
local wdsap_opts
eval "wdsap_opts=\"${opts} disabled\""

for dev in $devices; do
    valid_count=0
    for idx in $(seq 1 4); do
        if [ -z "$(uci get wireless.${dev}_wds_${idx}.encryption | grep 'wep\|wpa+\|wpa-\|psk+\|psk-\|tkip')" ]; then
            valid_count=$(($valid_count+1))

            for opt in $wdsap_opts; do
                sync_option "${dev}_wds_$idx" "${dev}_wds_$valid_count" $opt
            done
        fi

        if [ $valid_count != $idx ]; then
            section="${dev}_wds_${idx}"
            cmd_reset=`eval uci -c /rom/etc/config/ show wireless.${section}`
            eval ${section}_conf=\$cmd_reset
            set_opt "$(eval echo \$${section}_conf)"
            # -------- Delete options which not exist in default config --------
            del_opt "${section}" "$macfilteropts $tcopts $acctopts $wdsbr"
        fi
    done
done

cp /rom/etc/config/wifi_schedule /etc/config/wifi_schedule

## ----------------Main------------------
index=0
for sec in $sync_sec; do
    index=$(($index+1))
    sync_section $index $sec 
done

[ "$sync_sec" == "" ] && {
    uci set wireless.wifi0_ssid_1.disabled=1
    uci set wireless.wifi1_ssid_1.disabled=1
    [ "$(uci get functionlist.functionlist.SUPPORT_WLAN5G_2 2> /dev/null)" == "1" ] && uci set wireless.wifi2_ssid_1.disabled=1
}

## ---------------------------------
uci set wireless.wifi0_guest.disabled=1
uci set wireless.wifi1_guest.disabled=1
[ "$(uci get functionlist.functionlist.SUPPORT_WLAN5G_2 2> /dev/null)" == "1" ] && uci set wireless.wifi2_guest.disabled=1
uci set system.factory.init=2
[ -f "/sbin/vlan_id_sync.sh" ] && sh /sbin/vlan_id_sync.sh
uci commit
rm /overlay/etc/lighttpd -rf

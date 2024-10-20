#!/bin/sh
#Author:Senao

support_channelconfig=$(uci -q get wifiprofile.snWifiConf.SUPPORT_CHANNEL_CONFIG)
[ "$support_channelconfig" != "1" ] && return

wifi_dev=$1

channel_config_enable=$(uci -q get wireless.$wifi_dev.channel_config_enable)
channel_config_list=$(uci -q get wireless.$wifi_dev.channel_config_list)
country=$(uci -q get wireless.$wifi_dev.country)
htmode=$(uci -q get wireless.$wifi_dev.htmode)
channel=$(uci -q get wireless.$wifi_dev.channel)
realEtsiDfsCertified=$(uci -q get sysProductInfo.model.realEtsiDfsCertified || echo -n "0")

[ "auto" == "$channel" ] && channel=0

#[ $channel_config_enable -ne 0 ] && {
#    [ -n "$channel_config_list" ] && {
        case "$wifi_dev" in
            wifi0)
                if [ -f /etc/config/ezmcloud -a "$htmode" == "HT40" -a "$channel" == "0" ]; then
                    echo "1 1,6,11" > /proc/channel_config
                else
                    echo "$channel_config_enable $channel_config_list" > /proc/channel_config
                fi
                ;;
            wifi1)
                if [ -f /etc/config/ezmcloud -a $channel_config_enable -eq 0 -a "$channel" == "0" ]; then
                    outdoor=$(uci -q get sysProductInfo.model.outdoor || echo -n "0")
                    obey=$(uci -q get wireless.$wifi_dev.obeyregpower)
                    domain=$(setconfig -g 4)
                    dfscertified=0
                    # RegularDomain.sh $country $domain $obey $outdoor $dfsCertified tri_band real_5g

                    case "$country" in
                            8|40|56|100|191|203|208|233|246|250|276|300|348|352|372|380|398|428|440|442|492|528|578|616|620|642|643|703|705|724|752|756|804|807|826) #ETSI Country ID
                                    dfscertified=$realEtsiDfsCertified
                                    ;;
                    esac

                    val=$(sh /lib/wifi/RegularDomain.sh $country $domain $obey $outdoor $dfscertified 0 1)
                    disable_band=$(echo $val | cut -d ' ' -f 1)
                    weather_ch=$(echo $val | cut -d ' ' -f 2)

                    if [ $country -eq 158 ]; then
                        # Cloud series will not select band1 for auto channel when country is Taiwan.
                        # [ "$htmode" = "HT20" ] && echo 1 52,56,60,64,100,104,108,112,116,120,124,128,132,136,140,149,153,157,161,165 > /proc/channel_config_5g;
                        disable_band=$(($disable_band & 14))
                    fi

                    # Real RF channel
                    #	Band1		Band2		Band3					Band4
                    #20	36,40,44,48	52,56,60,64	100,104,108,112,116,132,136,140		149,153,157,161,165
                    #40	40,48		56,64		104,112,136				153,161
                    #80	40		56		104					153

                    channel_config_5g_str=
                    if [ "$htmode" = "HT20" ]; then
                        if [ $(($disable_band & 1)) -gt 0 ]; then
                                channel_config_5g_str="36,40,44,48,"
                        fi
                        if [ $(($disable_band & 2)) -gt 0 ]; then
                                channel_config_5g_str=$channel_config_5g_str"52,56,60,64,"
                        fi
                        if [ $(($disable_band & 4)) -gt 0 ]; then
                                channel_config_5g_str=$channel_config_5g_str"100,104,108,112,116,132,136,140,"
                        fi
                        if [ $(($disable_band & 8)) -gt 0 ]; then
                                channel_config_5g_str=$channel_config_5g_str"149,153,157,161,165,"
                        fi
                    elif [ "$htmode" = "HT40" ]; then
                        if [ $(($disable_band & 1)) -gt 0 ]; then
                                channel_config_5g_str="40,48,"
                        fi
                        if [ $(($disable_band & 2)) -gt 0 ]; then
                                channel_config_5g_str=$channel_config_5g_str"56,64,"
                        fi
                        if [ $(($disable_band & 4)) -gt 0 ]; then
                                channel_config_5g_str=$channel_config_5g_str"104,112,136,"
                        fi
                        if [ $(($disable_band & 8)) -gt 0 ]; then
                                channel_config_5g_str=$channel_config_5g_str"153,161,"
                        fi
                    elif [ "$htmode" = "HT80" ]; then
                        if [ $(($disable_band & 1)) -gt 0 ]; then
                                channel_config_5g_str="40,"
                        fi
                        if [ $(($disable_band & 2)) -gt 0 ]; then
                                channel_config_5g_str=$channel_config_5g_str"56,"
                        fi
                        if [ $(($disable_band & 4)) -gt 0 ]; then
                                channel_config_5g_str=$channel_config_5g_str"104,"
                        fi
                        if [ $(($disable_band & 8)) -gt 0 ]; then
                                channel_config_5g_str=$channel_config_5g_str"153,"
                        fi
                    fi

                    channel_config_list=$(echo $channel_config_5g_str | sed 's/.$//')

                    echo "1 $channel_config_list" > /proc/channel_config_5g
                else
                    echo "$channel_config_enable $channel_config_list" > /proc/channel_config_5g
                fi
                ;;
            wifi2)
                echo "$channel_config_enable $channel_config_list" > /proc/channel_config_5g_2
                ;;
        esac
#    }
#}


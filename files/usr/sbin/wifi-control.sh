#!/bin/sh

. /lib/senao-shell-libs/wifi/wifi-control-libs.sh

### Global variable ###
__json_format=0
__act_name=""
__opt_number=0
__no_color=0
__input_file=""
####

#===  FUNCTION  ================================================================
#         NAME:  usage
#  DESCRIPTION:  Display usage information.
#===============================================================================
usage()
{
    echo "Usage :  $0 [options] [--]

    Options:
    -h  Display this message
    -f  Load file
    -a  Action to Control (default option, '-a' is not necessary)
            beacon [on/off/show] [Interface/SSID num(1-8)/all (all of athx)]
            hostapd [reload] [Interface/SSID num(1-8)/all (all of athx)]
                    [reconfig] send SIGHUP to hostapd
                    [acl] [deny/accept] [show/clear] [Interface/SSID num(1-8)/all (all of athx)]
                                        [add/del] [00:aa:bb:cc:dd:10] [Interface/SSID num(1-8)/all (all of athx)]
                    [list_sta] [Interface/SSID num(1-8)/all (all of athx)]
                    [check_sta] [Interface/SSID num(1-8)/all (all of athx)] [00:aa:bb:cc:dd:10]
                    [reload_wpa_psk] [Interface/SSID num(1-8)/all (all of athx)]
            client [show/count] [Interface/SSID num(1-8)/24g/5g/all (all of athx)]
                   [kick] [00:aa:bb:cc:dd:10] (reason)
            bssid  [show] [Interface/SSID num(1-8)/24g/5g/all (all of athx)]
            ssid   [show] [essid/freq]
            chan_util [util/util_nwifi/both] [24g/5g] [current(default)/channel number] [ht20/ht40]
            chan [show] [24g/5g]
    -j  Show with Json format (TODO)
    -n  Without color
    -v  Display script version"

}    # ----------  end of function usage  ----------
MSLEEP(){
        sleepms=$(( $1 * 1000))
        usleep $sleepms
}
act_ssid_handle()
{
    local action=$ssid_opt_1
    local opt=""
    for i in `seq 1 ${__opt_number}`
    do
        eval "VAL=$(echo \${ssid_opt_$i})"
        opt="$opt${opt:+ }$VAL"
    done

    case $action in
        "show")
            control_ssid_show $opt
            ;;
        *)
            usage
            exit 1
            ;;
    esac
}

act_client_handle()
{
    local action=$client_opt_1
    local opt=""
    for i in `seq 1 ${__opt_number}`
    do
        eval "VAL=$(echo \${client_opt_$i})"
        opt="$opt${opt:+ }$VAL"
    done

    case $action in
        "kick")
            control_client_kick $opt
            ;;
        "list"|"show")
            control_client_list $opt
            ;;
        "count")
            control_client_count $opt
            ;;
        *)
            usage
            exit 1
            ;;
    esac
}

act_bssid_handle()
{
    local action=$bssid_opt_1
    local opt=""
    for i in `seq 1 ${__opt_number}`
    do
        eval "VAL=$(echo \${bssid_opt_$i})"
        opt="$opt${opt:+ }$VAL"
    done

    case $action in
        "show")
            control_bssid_show $opt
            ;;
        *)
            usage
            exit 1
            ;;
    esac
}
act_recover_handle()
{
    local ifnames
    local action=$recover_opt_1
    local radio=$recover_opt_2
    local iface=$recover_opt_3
    local ht_mode=$recover_opt_4
    shift
    support_wideband=$(exttool -h |grep wideband)
    if [ "$support_wideband" == "" ]
    then
	ifnames=$(grep -l wifi /sys/class/net/*/parent |cut -d '/' -f 5)

	for ifname in $ifnames
	do
	    ifconfig $ifname down
	done

	if [ "$radio" == 0 ]
	then
	    cur_mode=$(echo $ht_mode | grep 20)

	    if [ "$cur_mode" != "" ]
	    then
		iwpriv $iface mode $ht_mode
		iwpriv $iface chwidth 0
	    else
		iwpriv $iface mode $ht_mode
		iwpriv $iface chwidth 1
		iwpriv $iface disablecoext 1
	    fi
	else
	    cur_mode=$(echo $ht_mode | grep 20)

	    if [ "$cur_mode" != "" ] #ht20
	    then
		iwpriv $iface mode $ht_mode
		iwpriv $iface chwidth 0
	    else
		cur_mode=$(echo $ht_mode | grep 40)
		if [ "$cur_mode" != "" ] #ht40
		then
		    iwpriv $iface mode $ht_mode
		    iwpriv $iface chwidth 1
		    iwpriv $iface disablecoext 1
		else #ht80
		    iwpriv $iface mode $ht_mode
		    iwpriv $iface chwidth 2
		    iwpriv $iface disablecoext 1
		fi
	    fi
	fi

	for ifname in $ifnames
	do
	    ifconfig $ifname up
	done
    fi
}

act_chan_util_handle()
{
    local action=$chan_util_opt_1
    local wifix=$chan_util_opt_2
    local channel=$chan_util_opt_3
    local ht_mode=$chan_util_opt_4

    if [ "$wifix" == "24g" ]
    then
        radio=0
    else
        radio=1
    fi
    if [ "$ht_mode" == "ht20" ]
    then
        ht_mode_idx=0
    else
        ht_mode_idx=1
    fi

    iface=$(grep -l wifi$radio /sys/class/net/ath*/parent |cut -d '/' -f 5 |head -n 1)
    cur_mode=$(iwpriv $iface get_mode |grep 20)

    case $action in
        "util")
            show_chan_util $radio $iface $channel
            ;;
        "util_nwifi")
            show_chan_util_nwifi $radio $iface $channel
            ;;
        "both")
            if [ "$channel" != "current" ]
            then
                iwpriv wifi$radio acs_dbgtrace 0x14
                if [ "$cur_mode" == "" -a "$ht_mode_idx" == "0" ] || [ "$cur_mode" != "" -a "$ht_mode_idx" == "1" ] #current != parse ht mode
                then
                    support_wideband=$(exttool -h |grep wideband)
                    if [ "$support_wideband" != "" ]
                    then
                        exttool --scan --interface wifi$radio --wideband $ht_mode_idx --mindwell 51 --maxdwell 100 --resttime 1 --maxscantime 200 --idletime 51 --scanmode 1 --chcount 1 $channel
                    else
                        change_htmode $radio $iface $ht_mode
                        exttool --scan --interface wifi$radio --mindwell 51 --maxdwell 100 --resttime 1 --maxscantime 200 --idletime 51 --scanmode 1 --chcount 1 $channel
                    fi
                else
                    exttool --scan --interface wifi$radio --mindwell 51 --maxdwell 100 --resttime 1 --maxscantime 200 --idletime 51 --scanmode 1 --chcount 1 $channel
                fi
                MSLEEP 300
            fi
            show_chan_util $radio $iface $channel
            show_chan_util_nwifi $radio $iface $channel
            ;;
        *)
            usage
            exit 1
            ;;
    esac
}

act_chan_handle()
{
    local action=$chan_opt_1
    local wifix=$chan_opt_2

    if [ "$wifix" == "24g" ]
    then
        radio=0
    else
        radio=1
    fi

    show_cur_chan $radio
}

act_hostapd_handle()
{
    local action=$hostapd_opt_1
    local opt=""
    for i in `seq 2 ${__opt_number}`
    do
        eval "VAL=$(echo \${hostapd_opt_$i})"
        opt="$opt${opt:+ }$VAL"
    done

    case $action in
        "reload")
            control_hostapd_reload $opt
            ;;
        "reconfig")
            killall -SIGHUP hostapd
            ;;
        "acl")
            control_hostapd_acl $opt
            ;;
        "list_sta")
            control_hostapd_sta $action $opt
            ;;
        "check_sta")
            control_hostapd_sta $action $opt
            ;;
        "reload_wpa_psk")
            control_hostapd_reload_wpa_psk $opt
            ;;
        *)
            usage
            exit 1
            ;;
    esac
}

act_beacon_handle()
{
    local action=$beacon_opt_1
    local opt=""
    for i in `seq 1 ${__opt_number}`
    do
        eval "VAL=$(echo \${beacon_opt_$i})"
        opt="$opt${opt:+ }$VAL"
    done

    case $action in
        "on")
            control_beacon_handle $opt
            if [ "$(uci -q get snvpn.general.depend_beacon)" = "1" ]; then
                /usr/sbin/vpn-control.sh conn up ${opt#*on}
            fi
            ;;
        "off")
            control_beacon_handle $opt
            if [ "$(uci -q get snvpn.general.depend_beacon)" = "1" ]; then
                /usr/sbin/vpn-control.sh conn down ${opt#*off}
            fi
            ;;
        "show")
            control_beacon_handle $opt
            ;;
        "fix")
            control_beacon_handle $opt
            ;;
        *)
            usage
            exit 1
            ;;
    esac
}

action_handle()
{
    __opt_number=0
    while [ "$1" != "" ]
    do
        __opt_number=$(($__opt_number+1))
        eval ${__act_name}_opt_${__opt_number}=$1
        shift
    done
    eval act_${__act_name}_handle
    if [ $? -ne 0 ]
    then
        usage
        exit 1
    else
        exit 0
    fi
}

#-----------------------------------------------------------------------
#  Handle command line arguments
#-----------------------------------------------------------------------

while getopts ":hva:jnf:V:" opt
do
  case $opt in

    h)  usage; exit 0   ;;
    V)
        version=`opkg list zzSENAO_COMMON | awk '{print $3}'`
        input_version=$OPTARG
        major_ver=${version%%.*}
        minor_ver=${version#$major_ver.}
        minor_ver=${minor_ver%%-*}
        input_major_ver=${input_version%.*}
        input_minor_ver=${input_version#*.}
        if [ ${major_ver:-0} -ge ${input_major_ver:-0} ]
        then
            if [ ${minor_ver:-0} -ge ${input_minor_ver:-0} ]
            then
                echo 1
                exit 0
            else
                echo 0
                exit 1
            fi
        else
            echo 0
            exit 1
        fi
        ;;
    v)  opkg list zzSENAO_COMMON | awk '{print $3}'; exit 0   ;;
    j)  __json_format=1;;
    n)  __no_color=1;;
    f)
        __input_file="$OPTARG"
        [ -f "$__input_file" ] || { echo file error!!; exit 1; }
        ;;
    a)
        case "$OPTARG" in
            *)
                __act_name=$OPTARG
                shift $(($OPTIND-1))
                action_handle "$@"
                ;;
        esac
        ;;

    * )  echo -e "\n  Option does not exist : $OPTARG\n"
          usage; exit 1   ;;

  esac    # --- end of case ---
done
shift $(($OPTIND-1))

__act_name=$1
shift
action_handle "$@"

exit 0

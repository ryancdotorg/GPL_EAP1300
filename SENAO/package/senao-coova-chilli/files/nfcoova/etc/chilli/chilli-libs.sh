#!/bin/sh
# senao portal functions

aws_ip_check=/usr/sbin/aws-ip-check.sh
chilli_ipt_restore_dir=/var/run/chilli/iptables/
chilli_ipt6_restore_dir=/var/run/chilli/ip6tables/
chilli_ipt_restore_default_dir=/etc/chilli/iptables/
chilli_ipt6_restore_default_dir=/etc/chilli/ip6tables/
chilli_lock_file=/var/run/chilli/chilli-lock
chilli_status_file=/var/run/chilli/chilli-status
chilli_reload_file=/var/run/chilli/chilli-reload

support_ssid_profile=$(uci -q get functionlist.functionlist.SUPPORT_SSID_PROFILE || echo 0)

default_cfg80211=0

addstatus() {
	local config_name=$1
	shift
	[ -n "$*" ] && cat <<EOF >> /tmp/etc/chilli/$config_name/chilli_status
$*
EOF
}

strip_ethprofile_interface() {
	# appends space to let strip correctly
	local ifnames=$@" "

	if [ "$support_ssid_profile" = "1" ]; then
		for ethprofile_section in $(foreach ethprofile profile)
		do
			ethprofile_ifnames="$(uci -q get ethprofile.$ethprofile_section.ifname)"
			ifnames=${ifnames//$ethprofile_ifnames /}
		done
	fi

	# echo without " will ignore the last space
	echo $ifnames
}
lan_ifnames=$(uci -q get /rom/etc/config/network.lan.ifname)

if [ -e /etc/init.d/syskey ]; then
    is_support_syskey=1
    is_support_syskey_mark=1
else
    is_support_syskey=0
    is_support_syskey_mark=0
fi

is_cfg80211() {
	local cfg80211=0
	if [ -f "/ini/global.ini" ]; then
		cfg80211=$(cat /ini/global.ini | grep cfg80211_config | awk -F "=" '{print $2}')
	fi
	echo "$cfg80211"
}

WIFI_IWPRIV(){
	if [ $default_cfg80211 -eq 1 ]; then
		/usr/sbin/cfg80211tool "$@"
	else
		/usr/sbin/iwpriv "$@"
	fi
}

isip() {
	local ip=$1
	local valid_check=$(echo $ip|awk -F. '$1<=255&&$2<=255&&$3<=255&&$4<=255{print "yes"}')
	if echo $ip|grep -E "^[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}$">/dev/null; then
		if [ ${valid_check:-no} == "yes" ]; then
			echo 1
		else
			echo 0
		fi
	else
		echo 0
	fi
}

ifget() {
	local opt=$1
	local ifname=$2
	local default=$3

	case "$opt" in
		ip)
			local ipaddr=`ifconfig $ifname | awk '/inet addr/{print substr($2,6)}'`
			[ -z "${ipaddr}" ] && echo $default || echo $ipaddr
			;;
		mask)
			local mask=`ifconfig $ifname | awk '/Mask:/{print substr($4,6)}'`
			[ -z "${mask}" ] && echo $default || echo $mask
			;;
		mac|macnc|machypen)
			local mac=$(ifconfig $ifname | awk '/HWaddr/{print $5}')
			if [ $opt = "macnc" ]; then
				echo $mac |tr -d ":"
			elif [ $opt = "machypen" ]; then
				echo $mac | sed 's/:/-/g'
			else
				echo $mac
			fi
			;;
	esac
}

get_config_value() {
	local file=$1
	local name=$2
	local name_len=$(echo "$name"|wc -c)
	local offset=$(($name_len+1))

	test -f $file && cat $file |awk "/^$name=/{print substr(\$1,$offset)}"
}

get_ssid_num() {
	local input=$1
	if [ "${input%%-*}" = "br" ];then
		ssidnum=${input#br-ssid}
	else
		ssidnum=${input#ath[0-9]}
		ssidnum=$((${ssidnum:-0}+1))
	fi
	echo ${ssidnum:-1}
}

get_section_id() {
	local config_name=$1
	section_id=${config_name##*_}
	section_id=${section_id//[a-z_-]/}
	echo ${section_id}
}

get_section_name() {
	local config_name=$1
	local section_id=$(get_section_id $config_name)

	if [ "$config_name" != "${config_name/port/}" ]
	then
		echo port_$section_id
	else
		echo ssid_$section_id
	fi
}

get_cpu_num()
{
	local cpunum=`cat /proc/cpuinfo |grep ^processor |wc -l`
	echo ${cpunum:-1}
}

get_cpu_mask()
{
	local cpunum=$(get_cpu_num)
	local cpumask=0
	# skip cpu0 for OS use
	while [ $cpunum -gt 1 ]
	do
		cpunum=$(($cpunum-1))
		cpumask=$(($cpumask+(1 << $cpunum)))
	done
	printf %x $cpumask
}

is_ethernet_port() {
	local config_name=$1
	if [ "$config_name" != "${config_name/port/}" ]
	then
		echo 1
	else
		echo 0
	fi
}

# get_all_ethprofile_ssid() {
# 	if [ "$support_ssid_profile" = "1" ]
# 	then
# 		local ethprofile_section=$(foreach ethprofile profile enable 1)
# 		for i in $ethprofile_section
# 		do
# 			echo `uci get ethprofile.$i.ssid_profile`
# 		done
# 	fi
# }

get_ethprofile_ifname() {
	local config_name=$1

	if [ "$support_ssid_profile" = "1" ]
	then
		local section=$(get_section_name $config_name)
		local is_ethprofile_en=$(uci get ethprofile.$section.enable)
		if [ "$is_ethprofile_en" = "1" ]
		then
			echo $(uci -q get ethprofile.$section.ifname || echo eth0)
		fi
	fi
}

get_mark_value()
{
    local config_name=$1
    # 0:upload 1:download 2:both
    local updown=$2
    # 0:2.4G 1:5G 2:5G-2 3:6G
    local radio=$3

    local index=$(get_section_id $config_name)

    # not use now, always 0
    local is_ethernet=$(is_ethernet_port $config_name)

    if [ "$#" = "3" ]
    then
        if [ "$updown" = "2" ]
        then
            echo `printf 0x%x $(((${index:-1}<<8) + ($radio << 13) + ($is_ethernet << 15)))`/0xef00
        else
            echo `printf 0x%x $(((${index:-1}<<8) + ($updown << 12) + ($radio << 13) + ($is_ethernet << 15)))`/0xff00
        fi
    elif [ "$#" = "2" ]
    then
        if [ "$updown" = "2" ]
        then
            # same as only one arg
            echo `printf 0x%x $((${index:-1}<<8 + ($is_ethernet << 15)))`/0x8f00
        else
            echo `printf 0x%x $(((${index:-1}<<8) + ($updown << 12) + ($is_ethernet << 15)))`/0x9f00
        fi
    else
        echo `printf 0x%x $(((${index:-1}<<8) + ($is_ethernet << 15)))`/0x8f00
    fi
}

ipt2()
{
    ipt4 "$@"
    ipt6 "$@"
}

ipt4()
{
    local dirname=$1
    local ipt_restore_dir=$chilli_ipt_restore_dir/$dirname

    mkdir -p $ipt_restore_dir

    shift
    if [ "$1" = "-t" ]
    then
        shift
        local table=$1
        local ipt_restore_file=$ipt_restore_dir/$table
        shift
        if [ "$1" = "INIT" ]
        then
            echo "*$table" > $ipt_restore_file
        else
            # echo "$@" to "$ipt_restore_file" > /dev/console
            echo "$@" >> $ipt_restore_file
        fi
    else
        echo "ERROR ipt4" > /dev/console
    fi
}

ipt6()
{
    local dirname=$1
    local ipt6_restore_dir=$chilli_ipt6_restore_dir/$dirname

    mkdir -p $ipt6_restore_dir

    shift
    if [ "$1" = "-t" ]
    then
        shift
        local table=$1
        local ipt6_restore_file=$ipt6_restore_dir/$table
        shift
        if [ "$1" = "INIT" ]
        then
            echo "*$table" > $ipt6_restore_file
        else
            # echo "$@" to "$ipt6_restore_file" > /dev/console
            echo "$@" >> $ipt6_restore_file
        fi
    else
        echo "ERROR ipt6" > /dev/console
    fi
}


ipt4_restore_rm()
{
    ipt4_restore "$@"
    rm -f $2
}

ipt6_restore_rm()
{
    ipt6_restore "$@"
    rm -f $2
}

ipt4_restore()
{
    local count=$1
    local filename=$2

    if [ "$#" != "2" ]
    then
        return
    fi

    if [ ! -f "$filename" ]
    then
        return
    fi

    for i in `seq 1 $count`
    do
        iptables-restore --noflush $filename && {
            break
        } || {
            sleep 1
        }
    done
}

ipt6_restore()
{
    local count=$1
    local filename=$2

    if [ "$#" != "2" ]
    then
        return
    fi

    if [ ! -f "$filename" ]
    then
        return
    fi

    # support ipv6 default drop
    local base_filename=`basename $2`
    if [ "$base_filename" != "mangle" ]
    then
        return
    fi

    for i in `seq 1 $count`
    do
        ip6tables-restore --noflush $filename && {
            break
        } || {
            sleep 1
        }
    done
}


chilli_query_client()
{
    local action=$1
    local config_name=$2
    local client_mac=$3
    local gone_time=$4
    local idle_time=$5

    . /tmp/etc/chilli/$config_name/config

    case "$action" in
        logout)
            if [ "$(is_ethernet_port $config_name)" = "1" ]
            then
                :
            else
                # got coa
                guest_syncli forcesend logout ${client_mac//[:|-]/} $config_name 1>/dev/null 2>&1 &
            fi
            ;;
        left)
            # even disconnect here, it will present again, do check client in /etc/chilli/macup.sh again.
            if [ "${HS_DISCONNECT_LOGOUT}" = "on" ]; then
                chilli_query -s /var/run/chilli.$config_name.sock dhcp-release $client_mac
            else
                chilli_query -s /var/run/chilli.$config_name.sock dhcp-disconnect $client_mac &
                if [ "${HS_GUEST_SYNC}" = "on" ]; then
                    if [ "`uci -q get guestsyncd.guestsyncd.enabled`" = "1" ]
                    then
                        # avoid gonetime = 0
                        guest_syncli send gone ${client_mac//[:|-]/} $config_name ${gone_time:-1} ${idle_time:-0} 1>/dev/null 2>&1 &
                    fi
                fi
            fi
            ;;
        join)
            if [ "$HS_BRIDGEMODE" = "on" ]
            then
                chilli_query dhcp-update mac $client_mac data release_nologin
            fi
            # it will auto added by kcoova, do once here
            chilli_query -s /var/run/chilli.$config_name.sock dhcp-connect $client_mac &
            ;;
    esac
}

replace_config() {
local var=$2
local new_val=$3

sed -i -e "s/^$var=.*$/$var=$new_val/" "$1"
}

check_portal_config_vlan() {
    local configfile=""

    if [ "$1" = "0" ]
    then
        local checkstring='^HS_WANIF=br-99$'
    else
        local checkstring='^HS_VLAN='$1'$'
    fi

    for i in `ls /tmp/etc/chilli/`
    do
        if [ "`cat /tmp/etc/chilli/$i/config |grep "$checkstring"`" != "" ]
        then
            configfile="$configfile /tmp/etc/chilli/$i/config"
        fi
    done
    echo $configfile
}

get_uam_type()
{
    # 0: no ezmaster no cloud
    # 1: ezmaster
    # 2: cloud

    local portal_type=${1:-0}
    local uam_type=2

    # 0~99    : ezmaster
    # 100~199 : engenius cloud
    # 300~399 : thirdparty
    if [ $portal_type -lt 100 ]
    then
        uam_type=1
    elif [ $portal_type -ge 100 -a $portal_type -lt 200 ]
    then
        uam_type=2
    elif [ $portal_type -ge 300 -a $portal_type -lt 400 ]
    then
        uam_type=3
    fi

    echo $uam_type
}

chilli_retry_crontab()
{
    local action=$1
    local config_name=$2

    if [ "$action" = "add" ]
    then
        if [ -n "$config_name" ]
        then
            chilli_retry_crontab rem $config_name
            echo "CP crontab add retry boost_restart $config_name" > /dev/console
            crontab -l | { cat; echo "*/1 * * * * /etc/init.d/portal boost_restart $config_name"; } | crontab -
        fi
    elif [ "$action" = "rem" ]
    then
        if [ -z "$config_name" ]
        then
            echo "CP: remove all portal boost_restart of crontab"
        fi
        crontab -l |grep "/etc/init.d/portal boost_restart $config_name" > /dev/null
        if [ $? -eq 0 ]
        then
            crontab -l |grep -v "/etc/init.d/portal boost_restart $config_name" | crontab -
        fi
    fi
}

dochilli() {
	local action=$1
	local config_name=$2

	# when ipup.sh finish, chilli is ready.
	case "$action" in
		stop)
			if [ -n "$config_name" ]
			then
				echo "CP TODO dochilli $action br-ssidx" > /dev/console
				return 0
			fi
			rm -rf $chilli_lock_file
			rm -rf $chilli_status_file
			rm -rf $chilli_reload_file
			/etc/init.d/chilli $action
			;;
		start)
			if [ -n "$config_name" ]
			then
				echo "CP TODO dochilli $action br-ssidx" > /dev/console
				return 0
			fi
			mkdir -p $chilli_lock_file
			rm -rf $chilli_ipt_restore_dir
			rm -rf $chilli_ipt6_restore_dir
			/etc/init.d/chilli $action
			;;
		restart)
			# only for /etc/init.d/portal boost_restart br-ssidx use!!!!
			if [ -z "$config_name" ]
			then
				echo "CP restart_chilli must specific the br-ssidx" > /dev/console
				return 0
			fi

			if [ -f "/var/run/luci-reload-status" ]
			then
				chilli_retry_crontab add $config_name
				return 0
			else
				chilli_retry_crontab rem $restart_config
			fi

			# becaue ip chanage.. boost_reload cannot change ip, do chilli restart....
			local ssid_num=${config_name#br-ssid}
			if [ "$config_name" != "$ssid_num" ]
			then
				local is_ssid=1
			else
				local is_ssid=0
			fi

			if [ "$is_ssid" = "1" ]
			then
				wifi-control.sh beacon off $ssid_num
			fi

			CONFIG_NAME=$config_name /etc/init.d/chilli restart

			if [ "$is_ssid" = "1" ]
			then
				if [ -f "/usr/sbin/dnsmasq_restart" ]
				then
					/usr/sbin/dnsmasq_restart portal &
				else
					/etc/init.d/dnsmasq restart
				fi
				# restore beacon
				luci-reload auto wifi_schedule
			fi
			;;
		esac
}

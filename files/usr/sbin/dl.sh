#!/bin/sh

qcawifi_load()
{
    qcawifi_file=$(ls -l /etc/modules.d/ | grep qca-wifi | awk -F ' ' '{print $9}')

    for mod in $(cat /etc/modules.d/$qcawifi_file); do
        echo "dl.sh ${mod}.ko"
        dl.sh "${mod}.ko"
    done
}

qcawifi_load_spf9x()
{
    qcawifi_file=$(ls -l /lib/wifi/ | grep qca-wifi | awk -F ' ' '{print $9}')

    for mod in $(cat /lib/wifi/$qcawifi_file); do
        echo "dl.sh ${mod}.ko"
        dl.sh "${mod}.ko"
    done
}

useful_cmd_func()
{
	case "$1" in
		0)
			echo "[WiFiDebug]: iwpriv ath0 dbgLVL 0x00c00000"
			echo "[WiFi-List]: wlanconfig ath0 list sta"
			echo "[WiFiDebug]: radartool -i wifi1 bangradar  //Fake radar"
			echo "[WiFiDebug]: radartool -i wifi1 usenol 0  //Not jump"
			echo "[WiFiCommand]: iwconfig ath4 essid 11ac channel 149 rate auto"
			;;
		1)
			echo "tcpdump -w file -i br-lan ether host 14:7D:DA:7A:8D:A3"
			echo "curl -u xxx:xxx ftp://swos.synology.me/web/ -T file"
			echo "curl -u xxx:xxx -O ftp://swos.synology.me/web/file"
			;;
		2)
			if [ -d /overlay/upper/ ]; then
				echo "rm -rf /overlay/upper/*"
				rm -rf /overlay/upper/*
			else
				echo "rm -rf /overlay/*"
				rm -rf /overlay/*
			fi
			;;
		3)
			echo "Clean lua cache: rm -rf /tmp/luci-modulecache/*"
			rm -rf /tmp/luci-modulecache/*
			;;
		4)
			echo "[WEB]:    /usr/lib/lua/luci/"
			echo "[CONFIG]: /etc/config/"
			echo "[VERSION]:/etc/version"
			echo "[INIT]:   /etc/init.d/"
			;;
		5)
			echo "uci set functionlist.functionlist.SUPPORT_MYID=1"
			echo "uci commit functionlist"
			uci set functionlist.functionlist.SUPPORT_MYID=1
			uci commit functionlist
			;;
		6)
			mesh_intf=""
			[ $(uci get wireless.wifi0_mesh.disabled) -eq 0 ] && mesh_intf=$(uci get wireless.wifi0_mesh.ifname)
			[ $(uci get wireless.wifi1_mesh.disabled) -eq 0 ] && mesh_intf=$(uci get wireless.wifi1_mesh.ifname)
			[ "$mesh_intf" != "" ] && {
				wlanconfig $mesh_intf nawds list | grep -v 00:00;
				wlanconfig $mesh_intf nawds learning | grep -v 00:00;
				batctl o
			}
			;;
	esac

	exit 0
}

ezmcloud_debug_func()
{
	if [ "$1" = "1" ]; then
		cat /etc/ezmcloud/ezm_config_previous |openssl enc -base64 -d -A |jq .
	elif [ "$1" = "2" ]; then
		cat /tmp/log/ezmcloud/ezm_config_set_pre |openssl enc -base64 -d -A |jq .
	elif [ "$1" = "3" ]; then
		/etc/init.d/ezmcloud debug_start;/etc/init.d/ezmcloud restart
	elif [ "$1" = "4" ]; then
		killall -SIGUSR1 api.fcgi
	elif [ "$1" = "5" ]; then
		cd /tmp/log/ezmcloud/;dl.sh 1.json;openssl base64 -in 1.json -A -out ezm_config_set;/usr/bin/ezmconfig -dl &
	elif [ "$1" = "6" ]; then
		ubus call network.interface.lan status
	elif [ "$1" = "7" ]; then
		ubus call service list |grep -A 10 hostapd
	elif [ "$1" = "8" ]; then
		wifi-control.sh client show
	elif [ "$1" = "9" ]; then
		uci show wifiprofile.snWifiConf
	elif [ "$1" = "a" ]; then
		local ephemeral_file="/var/log/ezmcloud/ezm_checkin_data"

		local client_json=$(cat $ephemeral_file | jq '.clients[].ssids[]')
		local client_ul=$(echo $client_json | jq '.traffic[].up' | awk '{s+=$1} END {print s}')
		local client_dl=$(echo $client_json | jq '.traffic[].down' | awk '{s+=$1} END {print s}')

		local app_json=$(cat $ephemeral_file | jq '.app_traffic[]')
		local app_ul=$(echo $app_json | jq '.[].traffic[].up' |awk '{s+=$1} END {print s}')
		local app_dl=$(echo $app_json | jq '.[].traffic[].down' |awk '{s+=$1} END {print s}')

		echo [app_json]
		echo "$app_json"
		echo [client_json]
		echo "$client_json"

		echo [application]
		echo uload: $app_ul
		echo dload: $app_dl

		echo [client]
		echo uload: $client_ul
		echo dload: $client_dl
	elif [ "$1" = "b" ]; then
		cat /var/run/luci-reload-status
        elif [ "$1" = "c" ]; then
		cat /tmp/log/ezmcloud/ezm_checkin_data  | jq . | grep ephem | head -n 1 | awk -F ':' '{printf $2}'  | cut -d '"' -f 2  |openssl enc -base64 -d -A |jq .
	fi
}

if [ "$1" == "-h" ] || [ "$1" == "" ];then
    echo "-r [IP4] [-b size] [filename]    -- Download file, IP4 default 200"
    echo "-r [IP4] -p [filename] -- Upload file, IP4 default 200, please touch filename in /tftpboot and chmod 777 first."
    echo "-d [1:qcawifi(generic)/2:qcawifi(spf9x)/3:batman-adv&batctl] -- Debug"
    echo "-s [LAN_MAC] -- For modify lan mac by setconfig"
    echo "-c [interface]         -- Get interface channel"
    echo "-n                     -- Get Model Info"
    echo "-e                     -- ezmcloud debug"
    echo "-m                     -- Useful command"
    exit 1
fi;

if [ "$1" == "-e" ] ; then
	if [ "$2" != "" ]; then
		ezmcloud_debug_func $2
		exit 0;
	fi
	echo "1:(Get now config) cat /etc/ezmcloud/ezm_config_previous |openssl enc -base64 -d -A |jq ."
	echo "2:(Get last time)  cat /tmp/log/ezmcloud/ezm_config_set_pre |openssl enc -base64 -d -A |jq ."
	echo "3: /etc/init.d/ezmcloud debug_start;/etc/init.d/ezmcloud restart"
	echo "4: killall -SIGUSR1 api.fcgi  //Debug senao-openapi-server"
	echo "5: cd /tmp/log/ezmcloud/;dl.sh 1.json;openssl base64 -in 1.json -A -out ezm_config_set;/usr/bin/ezmconfig -dl &"
	echo "6: ubus call network.interface.lan status"
	echo "7: ubus call service list |grep -A 10 hostapd"
	echo "8: wifi-control.sh client show"
	echo "9: Get wifi profile: uci show wifiprofile.snWifiConf"
	echo "a: App-analysis debug"
	echo "b: cat /var/run/luci-reload-status //Check reload status"
	echo "c: get preview checkin data from /tmp/log/ezmcloud/ezm_checkin_data"
	read -p "Select:" ans1

	ezmcloud_debug_func  $ans1

    exit 0;
fi

# for get model info
if [ "$1" == "-n" ] ; then
    cat /tmp/sysinfo/model

    [ -f /sys/firmware/devicetree/base/soc_version_major ] && {
        soc_ver_maj=$(hexdump -n 1 -e '"%1d"' /sys/firmware/devicetree/base/soc_version_major)
        soc_ver_min=$(hexdump -n 1 -e '"%1d"' /sys/firmware/devicetree/base/soc_version_minor)
        [ "$soc_ver_min" = "*" ] && soc_ver_min=0

        echo "VER : $soc_ver_maj.$soc_ver_min"
    }

    exit 0;
fi

# for get wireless channel
if [ "$1" == "-c" ] ; then
    freq="1:2.412 2:2.417 3:2.422 4:2.427 5:2.432 6:2.437 7:2.442 8:2.447 9:2.452 10:2.457 11:2.462 12:2.467 13:2.472 14:2.484 \
            36:5.18 40:5.2 44:5.22 48:5.24 52:5.26 56:5.28 60:5.3 64:5.32 149:5.745 153:5.765 157:5.785 161:5.805 165:5.825 \
            167:5.835 169:5.845 171:5.855 173:5.865 100:5.5 104:5.52 108:5.54 112:5.56 116:5.58 120:5.6 124:5.62 128:5.64 132:5.66 \
            136:5.68 140:5.7 34:5.17 38:5.19 42:5.21 46:5.23 184:4.92 188:4.94 192:4.96 196:4.98 208:5.04 212:5.06  216:5.08"

    tmp=`iwconfig $2  | grep 'Frequency:' | cut -d: -f3 | awk '{print $1}'`
    for i in $freq
    do
        if [ "$tmp" == "${i#*:}" ]; then
            echo "$2 channel is ${i%:*}"
            echo "iwconfig data:"
            iwconfig $2  | grep 'Frequency:'
            echo "iwlist data:"
            iwlist $2 chan
        fi
    done
    exit 0;
fi

# Useful command
if [ "$1" == "-m" ] ; then
	if [ "$2" != "" ]; then
		useful_cmd_func $2
		exit 0;
	fi
	echo "0: List WIFI DEBUG command"
	echo "1: tcpdump/curl_server "
	echo "2: resetToDefaule      "
	echo "3: rm_luci_cache       "
	echo "4: List useful Path    "
	echo "5: AutoTest->set MYID  "
	echo "6: Show Mesh Info      "
	echo ""
	read -p "Select:" ans1

	useful_cmd_func $ans1

	exit 0;
fi

# for modify LAN mac
if [ "$1" == "-s" ] ; then
    setconfig -a 1
    if [ "$1" != "null" ] ; then
        old_lanmac=`setconfig -g 6`
        echo "old lan mac: $old_lanmac"
        setconfig -a 2 -s 6 -d $2
        if [ $? -eq 0 ] ; then #set ok
            modlan=1
        fi
    fi
    echo "====="
    setconfig -a 5
    if [ $modlan ] ; then
        new_lanmac=`setconfig -g 6`
        echo "new lan mac: $new_lanmac"
    fi
    exit 0;
fi

# for senao debug
if [ "$1" == "-d" ];then
    shift
    ans1=$1
    echo "cd /tmp....."
    cd /tmp;
    if [ "$ans1" == "" ];then
    echo "1: qcawifi(generic) debug." > /dev/ttyS0
    echo "2: batman-adv && batctl debug." > /dev/ttyS0
    read ans1
    fi
    if [ "$ans1" = "1" ]; then
	qcawifi_load
    elif [ "$ans1" = "2" ]; then
	qcawifi_load_spf9x
    elif [ "$ans1" = "3" ]; then
        echo "dl.sh batman-adv.ko"
        dl.sh batman-adv.ko
        echo "dl.sh batctl"
        dl.sh batctl
    fi

    if [ $? -ne 0 ];then
        echo "fail"
        exit 1;
    else
        echo "file downloaded to /tmp"
    fi
    exit 0;
fi;

# easy to download file
mode=`setconfig -g 5`
ic=`ifconfig br-lan 2>/dev/null|grep "inet addr"`
if [ "$ic" == "" ];then
	if [ $mode == "3" ];then
    i1=192;i2=168;i3=99;i4=8;
	else
	i1=192;i2=168;i3=1;i4=8;
	fi;
else
    tp=${ic%\.* Bcast*};
    i3=${tp#*:*\.*\.};
    tp=${tp%\.*};
    i2=${tp#*:*\.};
    tp=${tp%\.*};
    i1=${tp#*:};
    i4=8;
fi;

while [ "$1" != "" ];
do
    # get IP if input '-r'
    if [ "$1" == "-r" ];then
        shift;
        #tp=${1/./ }
        #tp=${tp/./ }
        #tp=${tp/./ }
        input4=${1##*\.}
        tp=${1%$input4}
        if [ ! "$tp" == "" ];then
            tp=${tp%.}
            input3=${tp##*\.}
            tp=${1%$input3\.$input4}
            if [ ! "$tp" == "" ];then
                tp=${tp%.}
                input2=${tp##*\.}
                tp=${1%$input2\.$input3\.$input4}
                if [ ! "$tp" == "" ];then
                    input1=${tp%.}
                fi
            fi
        fi

        if [ ! "$input1" == "" ];then
            i1=$input1
        fi
        if [ ! "$input2" == "" ];then
            i2=$input2
        fi
        if [ ! "$input3" == "" ];then
            i3=$input3
        fi
        i4=$input4

        shift;
    fi;

    # put file
    if [ "$1" == "-p" ];then
        shift;
        tftp -p -r $1 $i1.$i2.$i3.$i4;
        exit 0
    fi;

    # block size
    block_size=20000
    if [ "$1" == "-b" ];then
        shift;
        block_size=$1;
        shift;
    fi;

    # download file
    echo "rm -rf $1"
    rm -rf $1;
    echo "tftp -g -r $1 $i1.$i2.$i3.$i4"
    tftp -g -r $1 $i1.$i2.$i3.$i4 -b $block_size;
    echo "chmod 777 $1"
    chmod 777 $1;
    shift;
done;
exit 0



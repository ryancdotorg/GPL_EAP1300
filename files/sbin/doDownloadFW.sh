#!/bin/sh

NEW_FW_INFO="/tmp/new_fw_info"
fwFile1="/tmp/firmware.img"
fwFile2="/www/firmware.img"

download() {
    id="$1"
    fw_url="$2"
        #local id=$(cat /tmp/new_fw_info | awk -F ':' '{print $4}' |awk -F ',' '{print $1}'|cut -c 3-5)

    echo "id=[$id]" > /dev/console
        if [ "$id" != "" ] && [ "$id" != "0" ]; then
                url=http://$fw_url/ews/download.php?p=$id
                curl --connect-timeout 6 --speed-limit 1 --speed-time 5 -o $fwFile1 $url
                ln -sf $fwFile1 $fwFile2
        fi
}

download "$1" "$2"

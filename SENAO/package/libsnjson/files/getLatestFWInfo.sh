#!/bin/sh
. /usr/share/libubox/jshn.sh

NEW_FW_INFO=/tmp/new_fw_info

[ -f $NEW_FW_INFO ] && rm -rf $NEW_FW_INFO
[ -f "/tmp/newfwmd5sum" ] && rm -rf "/tmp/newfwmd5sum"
[ -f "/tmp/new_fw_info_ori" ] && rm -rf "/tmp/new_fw_info_ori"

version=$(cat /etc/version | grep Firmware | awk 'BEGIN{FS= " "} {print $4}'|sed 's/\(\.\)[^.]*$/\1/'|cut -d '.' -f 1-3)
#cap_ver=$(cat /etc/version_capwap)
#model_name=ENS500-ACv2
model_name=$(uci get sysProductInfo.model.modelName)
#check_enable=$(uci get system.firmware.version_check)
regular_domain=$(setconfig -g 4)
sku="FCC"
script_fw_check=/sbin/chkNewFW.sh

## get sku
if [ "$regular_domain" ==  "0" ]; then
    sku="FCC"
elif [ "$regular_domain" ==  "1" ]; then
    sku="ETSI"
elif [ "$regular_domain" ==  "2" ]; then
    sku="INT"
elif [ "$regular_domain" ==  "3" ]; then
    sku="JP"
else
    sku="TW"
fi

check_FW (){
    fw_url="$1"

    echo "sku=[$sku]" > /dev/console
	curl -o $NEW_FW_INFO --connect-timeout 3 -m 10 -g "http://$fw_url/ews/lastfwlist?device[0][type]=ap&device[0][sku]=$sku&device[0][model]=$model_name"

	fw_info=$(cat $NEW_FW_INFO)
    cp /tmp/new_fw_info /tmp/new_fw_info_ori

	if [ "$fw_info" != "" ] && [ "$fw_info" != "0" ]; then
		info=`cat $NEW_FW_INFO |cut -d '[' -f2 |cut -d ']' -f1`
		# parsing json data
		json_load "$info"
		json_get_var model			model
		json_get_var ver			ver
		json_get_var id				id
		json_get_var file_size		file_size
		json_get_var release_date	release_date
		json_get_var change_log		change_log
		json_get_var md5sum			md5sum
		json_get_var sku			sku
		json_get_var comment		comment
		json_get_var file_name		file_name
		json_get_var type		    type

		# generating new json data
		json_init
		json_add_string model			"$model"
		json_add_string version			"$ver"
		json_add_string id				"$id"
		json_add_string file_size		"$file_size"
		json_add_string release_date	"$release_date"
		json_add_string change_log		"$change_log"
		json_add_string md5sum			"$md5sum"
		json_add_string sku			    "$sku"
		json_add_string comment			"$comment"
		json_add_string file_name		"$file_name"
		json_add_string type			"$type"

		MSG=`json_dump`
		echo "$MSG" > "$NEW_FW_INFO"
		echo "$md5sum" > "/tmp/newfwmd5sum"
	fi

}


check_FW "$1"



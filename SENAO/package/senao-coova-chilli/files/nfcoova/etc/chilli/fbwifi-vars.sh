#!/bin/sh

fbwifi_tmp_dir=/var/run/fbwifi
fbwifi_conf_path=conf
fbwifi_tokens_path=tokens
fbwifi_ssl_path=ssl
fbwifi_wallgarden=/usr/sbin/fbwifi-garden.sh
curl_command="curl -s --connect-timeout 10 --speed-limit 1 --speed-time 10 -k"

#FBWIFI_DEBUG=1

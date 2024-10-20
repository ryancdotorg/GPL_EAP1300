#!/bin/sh

ifname=$1
client=$2
blocksec=$3

if [ -e "/sbin/kicksta.sh" ]
then

/sbin/kicksta.sh "portal" 1 $client $ifname ${blocksec:-1} &

else

. /etc/chilli/chilli-libs.sh

default_cfg80211=$(is_cfg80211)

is_maccmd_sec_en=$(WIFI_IWPRIV $ifname get_maccmd_sec)
is_maccmd_sec_en=${is_maccmd_sec_en#*:}
is_maccmd_sec_en=${is_maccmd_sec_en%% *}

if [ "$is_maccmd_sec_en" != "2" ]
then
    # enable acl for session/idle timeout use
    WIFI_IWPRIV $ifname maccmd_sec 2
fi

WIFI_IWPRIV $ifname addmac_sec $client
WIFI_IWPRIV $ifname kickmac $client
WIFI_IWPRIV $ifname kickmac $client
WIFI_IWPRIV $ifname kickmac $client
WIFI_IWPRIV $ifname kickmac $client
sleep ${blocksec:-1}
WIFI_IWPRIV $ifname delmac_sec $client

fi

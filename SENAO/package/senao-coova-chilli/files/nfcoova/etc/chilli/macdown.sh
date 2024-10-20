#!/bin/sh
#DEBUG=y
[ -n "${DEBUG}" ] && echo -e "\r\\033[34m[$(cat /proc/$PPID/status|grep ^Name:|cut -f 2)]$0\\033[0m" > /dev/console
[ -n "${DEBUG}" ] && echo CONFIG_NAME:$CONFIG_NAME > /dev/console
[ -n "${DEBUG}" ] && echo guest mac:$CALLING_STATION_ID > /dev/console


. /tmp/etc/chilli/$CONFIG_NAME/config
if [ "$HS_LOGIN_TYPE" = "108" ]
then
    . /etc/chilli/fbwifi-vars.sh
    if [ -n "$CALLING_STATION_ID" ]
    then
        fbwifi_token_file=$fbwifi_tmp_dir/${CONFIG_NAME}/$fbwifi_tokens_path/${CALLING_STATION_ID}
        rm -f $fbwifi_token_file
        rm -f $fbwifi_token_file-auth
        rm -f $fbwifi_token_file-done
    fi
fi


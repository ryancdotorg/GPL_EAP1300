#!/bin/sh

. /lib/functions.sh

config_name="senao-syskey"

handle_set_value() {
    config_value="$1"
    config_daemon=$(echo $config_value | awk -F ':' '{print $1}')
    section="$2"
    daemon="$3"
    set_value="$4"

    if [ "$config_daemon" = "$daemon" ]
    then
        uci del_list ${config_name}.${section}.set="$config_value"
        uci add_list ${config_name}.${section}.set="${daemon}:${set_value}"
        echo 1
    else
        echo 0
    fi
}

set_sys_val() {
    config_load "$config_name"
    section="$1"
    daemon="$2"
    set_value="$3"

    modified=$(config_list_foreach "$section" 'set' handle_set_value "$section" "$daemon" "$set_value")

    if [ "$(echo "$modified" | grep '1' | wc -l)" = "0" ]
    then
        uci add_list ${config_name}.${section}.set="${daemon}:${set_value}"
    fi
}

#!/bin/sh
. /lib/functions.sh

config="$1"
section="$2"
option="$3"
value="$4"

initial_option() {
    uci set "$config"."$1"."$option"="$value"
}

config_load "$config"
config_foreach initial_option "$section"

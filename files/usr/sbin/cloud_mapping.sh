#!/bin/sh
. /lib/functions.sh

#config ap 1
#    option cloud_config_index '1'
#    option dut_config_index '1'
#    option is_update '0'

CLOUD_MAPPING_CONFIG="/etc/config/cloud_mapping"

save_cloud_mapping_config() {
    uci commit cloud_mapping
}

update_dut_config_index() {
    local uid_pattern=$1
    local index=$2
    uci set cloud_mapping.$uid_pattern.dut_config_index=${index}
    uci set cloud_mapping.$uid_pattern.is_update=1
}

update_cloud_config_index() {
    local uid_pattern=$1
    local index=$2
    uci set cloud_mapping.$uid_pattern.cloud_config_index=${index}
    uci set cloud_mapping.$uid_pattern.is_update=1
}

new_cloud_mapping_section() {
    local uid_pattern=$1
    local opmode=$2
    uci set cloud_mapping.$uid_pattern=${opmode}
}

is_cloud_mapping_section_exist() {
    local uid_pattern=$1
    local opmode=$2
    result=$(uci get cloud_mapping.$uid_pattern)
    [ "$result" = "$opmode" ] && echo 1 || echo 0
}

is_dut_config_index_exist() {
    local uid_pattern=$1
    result=$(uci get cloud_mapping.$uid_pattern.dut_config_index)
    [ -n "$result" ] && echo 1 || echo 0
}

reinit() {
    local uid_pattern=$1
    uci set cloud_mapping.$uid_pattern.is_update=0
}

delete_unset_section() {
    local uid_pattern=$1
    local is_update=$(uci get cloud_mapping.$uid_pattern.is_update)
    [ $is_update = 0 ] && uci delete cloud_mapping.$uid_pattern
}

get_cloud_config_index() {
    local uid_pattern=$1
    result=$(uci get cloud_mapping.$uid_pattern.cloud_config_index)
    [ -n "$result" ] && echo $result || echo -1
}

get_dut_config_index() {
    local uid_pattern=$1
    result=$(uci get cloud_mapping.$uid_pattern.dut_config_index)
    [ -n "$result" ] && echo $result || echo -1
}

find_dut_config_useable_index() {
    local opmode=$1
    local max_size=$2
    local dut_config_used_array
    dut_config_used_array=$(config_foreach gen_dut_config_used_array ${opmode})
    for i in $(seq $max_size);
    do
        case "$dut_config_used_array" in
        *\[$i\]*)
            ;;
        *)
            echo $i
            break
            ;;
        esac
    done
}

gen_dut_config_used_array() {
   local uid_pattern=$1
   local dut_config_index=$(uci get cloud_mapping.$uid_pattern.dut_config_index)
   [ -n "$dut_config_index" ] && echo "[$dut_config_index]"
}

get_config_num_of_opmode() {
    local opmode=$1
    local num=0
    case $opmode in
        ap)
            num=8
            ;;
    esac
    echo "$num"
}

update_cloud_mapping() {
   local opmode=$1
   local update_uid_str=$2
   local update_uid_array=$(echo "$update_uid_str" | sed s/','/' '/g)
   local update_uid_array_length=$(expr $(echo "$update_uid_array" | wc -w))
   local ITER=1
   local MAX_SIZE=0
   local dut_config_index

   #### 0. preinit check ####
   MAX_SIZE=$(expr $(get_config_num_of_opmode ${opmode}))
   [ $MAX_SIZE -lt $update_uid_array_length ] && continue

   #### 1.create new section ####
   for uid_pattern in $update_uid_array
   do
        if [ "$(echo -n $uid_pattern | wc -m)" = "24" ]
        then
            [ $(is_cloud_mapping_section_exist ${uid_pattern} ${opmode}) = 0 ] && new_cloud_mapping_section ${uid_pattern} ${opmode}
            update_cloud_config_index ${uid_pattern} ${ITER}
            update_dut_config_index ${uid_pattern} ${ITER}
        fi
        ITER=$(expr $ITER + 1)
   done

   #### 2.delete old section ####
   config_load cloud_mapping && config_foreach delete_unset_section ${opmode}

   ##### 3.update new section ####
   #for uid_pattern in $update_uid_array
   #do
   #    if [ $(is_dut_config_index_exist $uid_pattern) = 0 ]; then
   #        dut_config_index=$(find_dut_config_useable_index ${opmode} ${MAX_SIZE})
   #        [ -n "$dut_config_index" ] && update_dut_config_index ${uid_pattern} ${dut_config_index}
   #    fi
   #done

   #### 4.initial all section ####
   config_load cloud_mapping && config_foreach reinit ${opmode}

   save_cloud_mapping_config
}

[ ! -f $CLOUD_MAPPING_CONFIG ] && touch $CLOUD_MAPPING_CONFIG >/dev/null 2>&1

case  "$1" in
    "update_cloud_mapping")
        update_cloud_mapping $2 $3
        ;;
    "get_dut_config_index")
        get_dut_config_index $2
        ;;
    "get_cloud_config_index")
        get_cloud_config_index $2
        ;;
esac

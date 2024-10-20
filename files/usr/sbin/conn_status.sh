#!/bin/sh

json_str=`wsm.sh status -j 2>/dev/null`
state=`echo $json_str |awk -F"[,:}]" '{for(i=1;i<=NF;i++){if($i~/'wtp_state'\042/){print $(i+1)}}}' | tr -d '"'`
if [ "$state" == "0" ] || [ "$state" == "1" ] || [ "$(pidof wtp)" == "" ]; then
    echo "Disconnect"
else
    ac_addr=`echo $json_str |awk -F"[,:}]" '{for(i=1;i<=NF;i++){if($i~/'ac_addr'\042/){print $(i+1)}}}' | tr -d '"'`
    ac_cid=`echo $json_str |awk -F"[,:}]" '{for(i=1;i<=NF;i++){if($i~/'ac_cid'\042/){print $(i+1)}}}' | tr -d '"'`
    if [ "$ac_cid" == "0" ]; then
        ac_cid_str=""
    else
        ac_cid_str="#$ac_cid"
    fi
    echo "Connect to $ac_addr$ac_cid_str"
fi


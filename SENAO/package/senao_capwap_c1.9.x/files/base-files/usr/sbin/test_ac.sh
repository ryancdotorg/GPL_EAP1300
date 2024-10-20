#!/bin/sh

if [ -z $1 ]; then
    # Check current connection with wtp
    json_str=`wsm.sh status -j 2>/dev/null`
    ac_addr=`echo $json_str |awk -F"[,:}]" '{for(i=1;i<=NF;i++){if($i~/'ac_addr'\042/){print $(i+1)}}}' | tr -d '"'`
    ac_port=`echo $json_str |awk -F"[,:}]" '{for(i=1;i<=NF;i++){if($i~/'ac_port'\042/){print $(i+1)}}}' | tr -d '"'`
    ac_cid=`echo $json_str |awk -F"[,:}]" '{for(i=1;i<=NF;i++){if($i~/'ac_cid'\042/){print $(i+1)}}}' | tr -d '"'`
    if [ "$ac_addr" != "0.0.0.0" ]; then
        echo "The device is managed by $ac_addr"
        result=`findAc --ip $ac_addr --port $ac_port --cid $ac_cid --if br-lan`
        echo $result
        exit
    fi

    # Check by querying EzReg server
    ezr_srv=`cat /tmp/EzrUrl`
    ezCom -i br-lan -p tcp -sa $ezr_srv -sp 80 -t 2 > /tmp/ez_res
    get_addr=`cat /tmp/ez_res | grep -e'[0-9].[0-9].[0-9].[0-9]' -c`
    ac_not_found=`cat /tmp/ez_res | grep 'AC Not Found' -c`
    if [ "$get_addr" == "0" ] && [ "$ac_not_found" == "0" ]; then
        ezCom -i br-lan -p udp -sa $ezr_srv -sp 53 -t 2 > /tmp/ez_res
        get_addr=`cat /tmp/ez_res | grep -e '[0-9].[0-9].[0-9].[0-9]' -c`
        ac_not_found=`cat /tmp/ez_res | grep 'AC Not Found' -c`
    fi
    if [ "$get_addr" == "0" ] && [ "$ac_not_found" == "0" ]; then
        echo "The device is unable to access ezReg server"
    fi
    if [ "$get_addr" == "1" ]; then
        ac_addr=`cat /tmp/ez_res | awk -F: '{printf $1}'`
        ac_port=`cat /tmp/ez_res | awk -F: '{printf $2}'`
        ac_lan_addr=`cat /tmp/ez_res | awk -F: '{printf $3}'`
        ac_cid=`cat /tmp/ez_res | awk -F: '{printf $5}'`
        if [ "$ac_cid" == "" ] ; then
            ac_cid="0"
        fi
        echo "The device is registered by $ac_addr"
        findAc --ip $ac_addr --port $ac_port --cid $ac_cid --if br-lan > /tmp/fa_res
        success=`cat /tmp/fa_res | grep 'Success' -c`
        if [ "$success" == "0" ] && [ "$ac_addr" != "$ac_lan_addr" ]; then
            findAc --ip $ac_lan_addr --port $ac_port --if br-lan > /tmp/fa_res_lan
            success=`cat /tmp/fa_res_lan | grep 'Success' -c`
            if [ "$success" == "1" ]; then
                result=`cat /tmp/fa_res_lan`
            else
                result=`cat /tmp/fa_res`
            fi
            rm /tmp/fa_res_lan
        else
            result=`cat /tmp/fa_res`
        fi
        rm /tmp/fa_res
        echo $result
    else
        echo "No controller detected"
    fi
    rm /tmp/ez_res
else
    ac_port=`echo $1 | awk -F: '{printf $2}' | awk -F# '{printf $1}'`
    ac_cid=`echo $1 | awk -F# '{printf $2}'`
    if [ "$ac_port" != "" ]; then
        ac_arg="--port $ac_port"
    fi
    if [ "$ac_cid" != "" ]; then
        ac_arg="$ac_arg --cid $ac_cid" 
    fi
    ac_addr=`echo $1 | awk -F: '{printf $1}' | awk -F# '{printf $1}'`
    result=`findAc --ip $ac_addr $ac_arg --if br-lan 2> /dev/null`
    [ "$result" == "" ] && echo "Failed: No response from $ac_addr" || echo $result
fi

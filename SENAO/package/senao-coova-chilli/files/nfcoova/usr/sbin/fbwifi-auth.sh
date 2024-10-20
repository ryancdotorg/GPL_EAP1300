#!/bin/sh

. /etc/chilli/fbwifi-vars.sh

config_name=$1
mac=${2//:/-}
mac=$(echo $mac |tr 'a-z' 'A-Z')


fbwifi_tokens_dir=$fbwifi_tmp_dir/$config_name/$fbwifi_tokens_path
token_file=$fbwifi_tokens_dir/$mac
auth_file=$token_file-auth
done_file=$token_file-done

if [ -f "$auth_file" ]
then
    cp $auth_file $token_file
    sleep 1
    touch $done_file
fi

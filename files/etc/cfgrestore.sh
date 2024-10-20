#!/bin/sh
tmpfile="/tmp/restore.gz"
#gunzip -t -f "$tmpfile" && \
#        tar -zf "$tmpfile" -xC/ && \

gunzip -t -f "$tmpfile" && {
        mkdir /tmp/restore
        tar -zf "$tmpfile" -xC /tmp/restore
        hostname=$(uci get system.@system[0].hostname)
        config_hostname=$(uci get system.@system[0].hostname -c /tmp/restore/etc/config)
        # Don't restore GUI v1 config
        guiver=$(uci get functionlist.vendorlist.WEB_GUI_VER -c /tmp/restore/etc/config || echo 1)

        if [ "$hostname" == "$config_hostname" ] && [ "$guiver" -ge 2 ];
        then
                tar -zf "$tmpfile" -xC /
                return 0
        else
                return 1
        fi
} && \
        echo "config restored" > /dev/console


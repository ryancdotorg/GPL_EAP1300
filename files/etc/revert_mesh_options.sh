#!/bin/sh

revert_items=$(uci show /rom/etc/config/wireless.${1})

if [ -n "$revert_items" ]; then
	for cfg in $revert_items; do
		uci set $cfg
	done
   sh /sbin/mesh.sh get_Mesh_Bundle_Setting
   uci commit
fi

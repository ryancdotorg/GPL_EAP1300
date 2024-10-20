#!/bin/sh
#*****************************************************************************
#
#   (C) Unpublished Work of Senao Networks, Inc.  All Rights Reserved.
#
#       THIS WORK IS AN UNPUBLISHED WORK AND CONTAINS CONFIDENTIAL,
#       PROPRIETARY AND TRADESECRET INFORMATION OF SENAO INCORPORATED.
#       ACCESS TO THIS WORK IS RESTRICTED TO (I) SENAO EMPLOYEES WHO HAVE A
#       NEED TO KNOW TO PERFORM TASKS WITHIN THE SCOPE OF THEIR ASSIGNMENTS
#       AND (II) ENTITIES OTHER THAN SENAO WHO HAVE ENTERED INTO APPROPRIATE
#       LICENSE AGREEMENTS.  NO PART OF THIS WORK MAY BE USED, PRACTICED,
#       PERFORMED, COPIED, DISTRIBUTED, REVISED, MODIFIED, TRANSLATED,
#       ABBRIDGED, CONDENSED, EXPANDED, COLLECTED, COMPILED, LINKED, RECAST,
#       TRANSFORMED OR ADAPTED WITHOUT THE PRIOR WRITTEN CONSENT OF SENAO.
#       ANY USE OR EXPLOITATION OF THIS WORK WITHOUT AUTHORIZATION COULD
#       SUBJECT THE PERPERTRATOR TO CRIMINAL AND CIVIL LIABILITY.
#
#
#*****************************************************************************/

device=$1            # ath1
client_mac=$2        # 2c:33:61:2a:42:cc
client_activity=$3   # left/join
interface_id=$4      # 2.4G:0 5G:1
rx_bytes=$5          # rxbytes
tx_bytes=$6          # txbytes

SCRIPT_DIR="/etc/assoc_notify.d"

#echo "device: $device client: $client_mac activity: $client_activity $interface_id $rx_bytes $tx_bytes" > /dev/console

for script in $(ls ${SCRIPT_DIR})
do
	if [ "${script/_notify.sh}" != "${script}" ]; then
		eval ${SCRIPT_DIR}/$script $device $client_mac $client_activity $interface_id $rx_bytes $tx_bytes
	fi
done

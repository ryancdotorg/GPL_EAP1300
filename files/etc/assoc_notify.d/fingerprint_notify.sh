#!/bin/sh

clientmac=$(echo "$2" | sed 's/://g')

finger_syncli send request "$clientmac" renew &

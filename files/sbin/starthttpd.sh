#!/bin/sh
sed -i -e 's/'startzz'/'start'/g' /etc/init.d/lighttpd
echo "[DEBUG] Start Web server. port:8001"
/etc/init.d/lighttpd start

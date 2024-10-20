#!/bin/sh
echo $1 $2 $2 >/proc/mesh_robust
sleep 1
crontab -l | grep -v "Mesh_robust" | crontab -

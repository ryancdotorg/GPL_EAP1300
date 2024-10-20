seed=`ifconfig br-lan | grep HWaddr | sed "s/://g"| awk '{print $5}'`OANES
echo "$(echo $seed | md5sum | cut -c1-8)"

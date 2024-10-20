#!/bin/sh
convert_month () {
    local months="Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec"
    local i
    local mnumber=1
    for i in $months
    do
        if [ "${1:0:3}" = "$i" ]
        then
            break
        fi
        mnumber=$(($mnumber+1))
    done
    if [ $mnumber -eq 13 ]
    then
        echo $(date +%m)
    else
        echo $mnumber
    fi
}

#googlepage=$(http_proxy=http://192.168.10.25:80/ curl -sD - www.google.com)
googlepage=$(curl -sD - www.google.com)
if [ "`echo "$googlepage"|grep Warning`" != "" ]
then
	echo "httpsynctime.sh got Warning" > /dev/console
    exit
else
	googletime=`echo "$googlepage"|grep '^Date:' | cut -d' ' -f3-7`
fi

if [ "$googletime" = "" ]
then
    exit
fi

# busybox not support this format '15 Apr 2019 11:27:22 GMT'
settimevar=$(echo $googletime | awk -F ':| ' '{ print "gday="$1"\ngmonth="$2"\ngyear="$3"\nghour="$4"\ngminute="$5"\ngsecond="$6"\ngtz="$7}')
eval $settimevar

nmonth=$(convert_month $gmonth)
#echo $gyear-$nmonth-$gday $ghour:$gminute:$gsecond
date -u -s "$gyear-$nmonth-$gday $ghour:$gminute:$gsecond"

#!/bin/sh
#
#!/bin/sh
if [ "$1" == "-h" ];then
    echo "$0 [domain] [filename]  "
    echo "domain - 0/1/2  "
    echo "filename - output file  "
    exit 1
fi;
domain=$1
output=$2
fccDfsCertified=$(uci get sysProductInfo.model.fccDfsCertified 2>/dev/null)
etsiDfsCertified=$(uci get sysProductInfo.model.etsiDfsCertified 2>/dev/null)
intDfsCertified=$(uci get sysProductInfo.model.intDfsCertified 2>/dev/null)
outdoor=$(uci get sysProductInfo.model.outdoor 2>/dev/null)
ctryNameList=$(sort /lib/wifi/RegularDomain.csv | grep -v 00 | awk -F " " '{print $1}')
#ctryIDlist[]=$(cat /lib/wifi/RegularDomain.csv | awk -F " " '{print $2}')
greenmode=$(uci get wireless.wifi1.obeyregpower 2>/dev/null)

[ -z $domain ] && domain=$(setconfig -g 4)
[ -z $output ] && output=/dev/console

isFccCtry() {
	for ctry in 32 76 124 152 170 188 214 218 320 340 458 484 591 604 630 840 858 862; do
		if [ $1 == $ctry ]; then
			return 0
		fi
	done
	return 1
} 

isEtsiCtry() {
	for ctry in 8 40 56 100 191 203 208 233 246 250 276 300 348 352 372 380 442 492 528 578 616 620 642 643 703 705 724 752 756 826 829 830; do
		if [ $1 == $ctry ]; then
			return 0
		fi
	done
	return 1
} 

for ctryName in $ctryNameList; do
	ctryID=$(grep "${ctryName} " /lib/wifi/RegularDomain.csv | grep -v 00 | awk -F " " '{print $2}')

	if [ "$domain" == "0" ]; then
		dfsCertified=${fccDfsCertified:-0}
		isFccCtry $ctryID || continue
	elif [ "$domain" == "1" ]; then
		dfsCertified=${etsiDfsCertified:-0}
		isEtsiCtry $ctryID || continue
	else
		dfsCertified=${intDfsCertified:-0}
		isFccCtry $ctryID && dfsCertified=${fccDfsCertified:-0}
		isEtsiCtry $ctryID && dfsCertified=${etsiDfsCertified:-0}
	fi

	#/lib/wifi/RegularDomain.sh "..countrycode.." "..domain.." "..greenmode.." "..outdoor.." "..dfsCertified..""))
	regd=$(sh /lib/wifi/RegularDomain.sh ${ctryID} ${domain} ${greenmode} ${outdoor} ${dfsCertified})

	disable_band=$(echo $regd | awk -F " " '{print $1}')
	weather_ch=$(echo $regd | awk -F " " '{print $2}')

	echo === $ctryName $ctryID ===  >> $output

	iwpriv wifi1 weather_ch $weather_ch
	iwpriv wifi1 disable_band $disable_band
	iwpriv wifi1 setCountryID $ctryID

	ath5="$(iwconfig 2>/dev/null |grep -E '^ath([1|5]|[1|5][0-9]|[3][5])+.*(11a|11na|11ac)'|head -n 1|awk {'printf $1'})"
	chan5c="$(wlanconfig ${ath5} list chan)";
	chan5=$(echo "$chan5c"|cut -c -49 > /tmp/chan5list;echo "$chan5c"|cut -c 50-|sed -e "s/^  *//g"|grep ^Chan >> /tmp/chan5list);

	chan_bw20=$(grep ' C\| V' /tmp/chan5list | awk -F " " '{print $2}')
	chan_bw40=$(grep ' CU\| CL\| VU\| VL'  /tmp/chan5list | awk -F " " '{print $2}')
	chan_bw80=$(grep V8  /tmp/chan5list | awk -F " " '{print $2}')

	echo BW20 : $chan_bw20 >> $output
	echo BW40 : $chan_bw40 >> $output
	echo BW80 : $chan_bw80 >> $output

done


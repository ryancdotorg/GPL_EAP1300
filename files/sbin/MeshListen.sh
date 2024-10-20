#!/bin/sh
	
MeshGetInfoTotalTime=240	# set in listen state 4 min 
MeshGetInfoIntervel=4	# do it every 4 sec
i=1	# counter
ifname=$1
vif=$2
isMeshInfoGot=0
if [ $vif = "wifi0_mesh" ]; then
	wifiDev="wifi0"
else
	wifiDev="wifi1"
fi

nawds_esinfo=/tmp/nawds_esinfo_$wifiDev

MeshListenBeacon(){
while [ $i -le $((MeshGetInfoTotalTime/MeshGetInfoIntervel)) ]
do
	wlanconfig "$ifname"  nawds esinfo > $nawds_esinfo

        local esStat=$(sed -n '2p' $nawds_esinfo | sed 's/ //g' | awk -F: '{print($2)}')    #2nd line
        local meshSsid=$(uci get wireless.$vif.ssid)
	local ConnectByMeshID=$(uci get wireless.$vif.MeshConnectType)
	#for debug
        #echo esStat=$esStat mesh24Gssid=$mesh24Gssid
	local checkMeshStatus=$(uci get wireless.$vif.MeshEzBroCast)
	if [ $checkMeshStatus -eq 0 ]; then
		break;	#to for concurrent dual band listen, if the other band got esinfo, should stop this band
	fi
        if [ "$esStat" = "yes" ]; then
		if [ $ConnectByMeshID -eq 1 ]; then
			local esID=$(sed -n '3p' $nawds_esinfo| sed 's/ //g' | awk -F: '{print($2)}')    #3rd line
		else
	        	local esSSID=$(sed -n '3p' $nawds_esinfo| sed 's/ //g' | awk -F: '{print($2)}')    #3rd line
		fi
        	local esFreq=$(sed -n '4p' $nawds_esinfo| sed 's/ //g' | awk -F: '{print($2)}')
        	local esPassword=$(sed -n '5p' $nawds_esinfo| sed 's/ //g' | awk -F: '{print($2)}')
        	local esEncrytype=$(sed -n '6p' $nawds_esinfo| sed 's/ //g' | awk -F: '{print($2)}')
        	local esWepKeyId=$(sed -n '7p' $nawds_esinfo| sed 's/ //g' | awk -F: '{print($2)}')
        	local esWepKeyLen=$(sed -n '8p' $nawds_esinfo| sed 's/ //g' | awk -F: '{print($2)}')
		if [ "$wifiDev" = "wifi0" ]; then
			local esChan=$((($esFreq-2407)/5))
		else
			local esChan=$((($esFreq-5000)/5))
		fi

        	#for debug
        	echo esStat=$esStat esSSID=$esSSID esID=$esID esChan=$esChan esPassword=$esPassword esEncrytype=$esEncrytype esWepKeyId=$esWepKeyId esWepKeyLen=$esWepKeyLen

	        if [ $ConnectByMeshID -eq 1 ]; then
			#uci set wireless.$vif.Mesh_id=$esID	#2.4G and 5G uses one /proc/mesh_id now, separate this file in the future
			uci set wireless.wifi0_mesh.Mesh_id=$esID
			uci set wireless.wifi1_mesh.Mesh_id=$esID
		else
			uci set wireless.$vif.ssid=$esSSID
        	fi
		uci set wireless.$wifiDev.channel_config_enable=1
		uci set	wireless.$wifiDev.channel=$esChan
		uci set wireless.$vif.nawds_encr=$esEncrytype
		if [ $esEncrytype="aes" ]; then
			uci set wireless.$vif.aeskey=$esPassword
		else										
			echo ERROR encrypType=$esEncrytype    WEP NOT SUPPPORT NOW
		fi
		i=$(( $MeshGetInfoTotalTime/$MeshGetInfoIntervel))
		isMeshInfoGot=1
		uci set network.sys.mesh_configured=1
		uci set network.sys.mesh_configured_5g=1
	else
		iwlist "$ifname" scanning > /dev/NULL &
        fi

	i=$(($i+1))
	sleep $MeshGetInfoIntervel
done
#uci set wireless.$vif.MeshEzBroCast=0
rm $nawds_esinfo -rf
uci set wireless.wifi0_mesh.MeshEzBroCast=0	
uci set wireless.wifi1_mesh.MeshEzBroCast=0	#both band mesh ezbrocast should be set zero

if [ $isMeshInfoGot -eq 0 ]; then
	uci set wireless.$vif.disabled=1
	uci set alfred."$wifiDev"_alfred.disabled=1
else
        uci set wireless.$vif.disabled=0
        uci set alfred."$wifiDev"_alfred.disabled=1
fi
uci commit
echo Finish $wifiDev Mesh Listen
}

echo 0 > /tmp/notify_fastscan_enable
	
	iwpriv "$ifname" dc 1
	MeshListenBeacon
	iwpriv "$ifname" dc 0

echo 1 > /tmp/notify_fastscan_enable

if [ $isMeshInfoGot -eq 1 ]; then
	#if [ $wifiDev="wifi1" ]; then
	#	psToKill="wifi0_mesh"
	#else
	#	psToKill="wifi1_mesh"
	#fi
	#kill -9 $(ps | grep MeshListen.sh | grep $psToKill | awk  '{print $1}') #no need to kill, wait it over
	luci-reload auto network
fi


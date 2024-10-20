#!/bin/sh

bandsteer_en=$(uci get wireless.wifi0.bandsteer)
bandsteerrssi=$(uci get wireless.wifi0.bandsteerrssi)
bandsteerpersent=$(uci get wireless.wifi0.bandsteerpersent)

local i=1

until [ "$i" == "9" ]
do
	if [ "$(uci get wireless.wifi0_ssid_$i.bandsteer_en)" != "0" ]; then
		uci set wireless.wifi0_ssid_$i.bandsteer_en=$bandsteer_en
		uci set wireless.wifi1_ssid_$i.bandsteer_en=$bandsteer_en
	
		uci set wireless.wifi0_ssid_$i.bandsteerrssi=$bandsteerrssi
		uci set wireless.wifi1_ssid_$i.bandsteerrssi=$bandsteerrssi

		uci set wireless.wifi0_ssid_$i.bandsteerpersent=$bandsteerpersent
		uci set wireless.wifi1_ssid_$i.bandsteerpersent=$bandsteerpersent
	fi

	i=$(($i+1))
done


echo "Setting wireless (24G) CB..."
ifconfig wifi0 hw ether 00:03:07:12:34:80
iwpriv wifi0 setCountryID 4015
wlanconfig ath0 create wlandev wifi0 wlanmode sta
iwconfig ath0 channel 1
iwpriv ath0 mode auto
iwpriv ath0 wds 1
iwconfig ath0 essid "24g"
iwconfig ath0 rts 2347

ifconfig eth1 0.0.0.0
brctl addbr br-lan
brctl addif br-lan eth1
brctl addif br-lan ath0
ifconfig ath0 0.0.0.0 up
ifconfig br-lan 192.168.1.2 up

sleep 2
iwpriv ath0 chwidth 2
iwpriv ath0 nss 2
iwpriv ath0 shortgi 1
#iwpriv ath0 ldpc 1
iwpriv ath0 ampdu 64
iwpriv ath0 amsdu 3
iwpriv ath0 extap 1

echo "Setting wireless (5G) CB ..."
ifconfig wifi1 hw ether 00:03:07:12:34:60
wlanconfig ath2 create wlandev wifi1 wlanmode sta
iwconfig ath2 channel 36
iwpriv ath2 mode auto
iwpriv ath2 wds 1
iwconfig ath2 essid "11ac"
iwconfig ath2 rts 2347

sleep 2
#iwpriv ath2 chwidth 2
iwpriv ath2 nss 2
iwpriv ath2 shortgi 1
#iwpriv ath2 ldpc 1
iwpriv ath2 ampdu 64
iwpriv ath2 amsdu 3
#iwpriv ath2 vhtmcs 9
#iwconfig ath0 txpower 0
#iwconfig ath2 txpower 20
iwpriv ath2 extap 1
ifconfig eth1 0.0.0.0
brctl addbr br-lan
brctl addif br-lan eth1
brctl addif br-lan ath2
ifconfig ath2 0.0.0.0 up
ifconfig br-lan 192.168.1.2 up

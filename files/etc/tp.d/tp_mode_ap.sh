echo 0 > /tmp/notify_fastscan_enable
echo 0 > /proc/bandsteer
ifconfig eth1 down
brctl delif br-lan eth1
ifconfig br-lan down
brctl delbr br-lan
brctl addbr br-lan
ifconfig br-lan 192.168.1.1 up
brctl addif br-lan eth1
ifconfig eth1 up

ifconfig wifi0 down
ifconfig wifi0 hw ether 00:11:22:88:E4:B0
wlanconfig ath0 create wlandev wifi0 wlanmode ap
iwpriv ath0 mode 11nght40
iwpriv ath0 cwmenable 1
iwpriv ath0 chwidth 2
iwpriv ath0 disablecoext 1
iwpriv ath0 shortgi 1
sleep 1
iwconfig ath0 essid "24g" channel 6 rate auto
iwconfig ath0 key off
ifconfig ath0 up
brctl addif br-lan ath0
ifconfig wifi1 down
ifconfig wifi1 hw ether 00:11:22:88:E4:B1
wlanconfig ath2 create wlandev wifi1 wlanmode ap
iwpriv ath2 mode 11acvht80
iwpriv ath2 wds 1
iwpriv ath2 chwidth 2
iwpriv ath2 shortgi 1
#iwpriv ath2 ldpc 1
iwpriv ath2 ampdu 64
iwpriv ath2 amsdu 3
iwpriv ath2 disablecoext 1
iwconfig ath2 essid "11ac" channel 149 rate auto 
iwconfig ath2 key off
ifconfig ath2 up
brctl addif br-lan ath2

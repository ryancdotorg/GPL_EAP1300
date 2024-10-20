#!/bin/sh
. /etc/chilli/chilli-libs.sh

addstatus $CONFIG_NAME "status_tuntap=$TUNTAP"

REAL_WANIF=br-lan

iptable_name=$CONFIG_NAME

chilli_postrouting_mangle=chilli_post_mangle_$iptable_name
chilli_prerouting_mangle=chilli_pre_mangle_$iptable_name
chilli_forward_filter=chilli_fw_filter_$iptable_name
chilli_prerouting_nat=chilli_pre_nat_$iptable_name

portal_prerouting_nat_x=portal_pre_nat_$iptable_name
portal_prerouting_mangle_x=portal_pre_mangle_$iptable_name
portal_postrouting_nat_x=portal_post_nat_$iptable_name
portal_postrouting_mangle_x=portal_post_mangle_$iptable_name
portal_input_filter_x=portal_in_filter_$iptable_name
portal_forward_filter_x=portal_fw_filter_$iptable_name
portal_forward_mangle_x=portal_fw_mangle_$iptable_name
portal_snlog_raw_x=portal_snlog_raw_$iptable_name

chilli_input_filter_rule=chilli_in_filter_$iptable_name
chilli_postrouting_nat_rule=chilli_post_nat_$iptable_name

# move ipdown.sh to here
ipt4 $iptable_name/delete -t raw INIT
ipt4 $iptable_name/delete -t raw -D portal_snlog_raw -j $portal_snlog_raw_x

ipt2 $iptable_name/delete -t mangle INIT
ipt2 $iptable_name/delete -t mangle -D portal_pre_mangle -j $portal_prerouting_mangle_x
ipt2 $iptable_name/delete -t mangle -D portal_post_mangle -j $portal_postrouting_mangle_x
ipt4 $iptable_name/delete -t mangle -D portal_fw_mangle -j $portal_forward_mangle_x

ipt4 $iptable_name/delete -t nat INIT
ipt4 $iptable_name/delete -t nat -D portal_pre_nat -j $portal_prerouting_nat_x
ipt4 $iptable_name/delete -t nat -D portal_post_nat -j $portal_postrouting_nat_x


ipt4 $iptable_name/delete -t filter INIT
ipt4 $iptable_name/delete -t filter -D portal_in_filter -j $portal_input_filter_x
ipt4 $iptable_name/delete -t filter -D portal_fw_filter -j $portal_forward_filter_x

for ipttable in `ls -p $chilli_ipt_restore_dir/$iptable_name/delete/ |grep -v "/$"`
do
    local iptpath=$chilli_ipt_restore_dir/$iptable_name/delete/$ipttable
    ipt4 $iptable_name/delete -t $ipttable COMMIT
    ipt4_restore_rm 1 $iptpath 2>/dev/null
done

for ipttable in `ls -p $chilli_ipt6_restore_dir/$iptable_name/delete/ |grep -v "/$"`
do
    local iptpath=$chilli_ipt6_restore_dir/$iptable_name/delete/$ipttable
    ipt6 $iptable_name/delete -t $ipttable COMMIT
    ipt6_restore_rm 1 $iptpath 2>/dev/null
done

# NOTE: iptable chain name must be under 29 chars !!!
ipt4 $iptable_name -t raw INIT
ipt4 $iptable_name -t raw ":$portal_snlog_raw_x - [0:0]"
ipt4 $iptable_name -t raw -I portal_snlog_raw -j $portal_snlog_raw_x
ipt4 $iptable_name -t raw -I $portal_snlog_raw_x -i $TUNTAP -j RETURN

ipt2 $iptable_name -t mangle INIT
ipt2 $iptable_name -t mangle ":$chilli_postrouting_mangle - [0:0]"
ipt2 $iptable_name -t mangle ":$chilli_prerouting_mangle - [0:0]"
ipt2 $iptable_name -t mangle ":$portal_prerouting_mangle_x - [0:0]"
ipt2 $iptable_name -t mangle ":$portal_postrouting_mangle_x - [0:0]"
ipt4 $iptable_name -t mangle ":$portal_forward_mangle_x - [0:0]"
ipt2 $iptable_name -t mangle -F $chilli_postrouting_mangle
ipt2 $iptable_name -t mangle -F $chilli_prerouting_mangle
ipt2 $iptable_name -t mangle -F $portal_prerouting_mangle_x
ipt2 $iptable_name -t mangle -F $portal_postrouting_mangle_x
ipt4 $iptable_name -t mangle -F $portal_forward_mangle_x
ipt2 $iptable_name -t mangle -I portal_pre_mangle -j $portal_prerouting_mangle_x
ipt2 $iptable_name -t mangle -I portal_post_mangle -j $portal_postrouting_mangle_x
ipt4 $iptable_name -t mangle -I portal_fw_mangle -j $portal_forward_mangle_x

ipt4 $iptable_name -t nat INIT
ipt4 $iptable_name -t nat ":$chilli_prerouting_nat - [0:0]"
ipt4 $iptable_name -t nat ":$portal_prerouting_nat_x - [0:0]"
ipt4 $iptable_name -t nat ":$portal_postrouting_nat_x - [0:0]"
ipt4 $iptable_name -t nat -F $chilli_prerouting_nat
ipt4 $iptable_name -t nat -F $portal_prerouting_nat_x
ipt4 $iptable_name -t nat -F $portal_postrouting_nat_x
ipt4 $iptable_name -t nat -I portal_pre_nat -j $portal_prerouting_nat_x
ipt4 $iptable_name -t nat -I portal_post_nat -j $portal_postrouting_nat_x

ipt4 $iptable_name -t filter INIT
ipt4 $iptable_name -t filter ":$chilli_forward_filter - [0:0]"
ipt4 $iptable_name -t filter ":$portal_input_filter_x - [0:0]"
ipt4 $iptable_name -t filter ":$portal_forward_filter_x - [0:0]"
ipt4 $iptable_name -t filter -F $chilli_forward_filter
ipt4 $iptable_name -t filter -F $portal_input_filter_x
ipt4 $iptable_name -t filter -F $portal_forward_filter_x
ipt4 $iptable_name -t filter -I portal_in_filter -j $portal_input_filter_x
ipt4 $iptable_name -t filter -I portal_fw_filter -j $portal_forward_filter_x


ipt4 $iptable_name/rule -t nat INIT
ipt4 $iptable_name/rule -t nat ":$chilli_postrouting_nat_rule - [0:0]"
ipt4 $iptable_name/rule -t nat -F $chilli_postrouting_nat_rule
ipt4 $iptable_name/rule -t nat -I $portal_postrouting_nat_x -j $chilli_postrouting_nat_rule


ipt4 $iptable_name/rule -t filter INIT
ipt4 $iptable_name/rule -t filter ":$chilli_input_filter_rule - [0:0]"
ipt4 $iptable_name/rule -t filter -F $chilli_input_filter_rule
ipt4 $iptable_name/rule -t filter -I $portal_input_filter_x -j $chilli_input_filter_rule


if [ -n "$TUNTAP" -a "$HS_BRIDGEMODE" != "on" ]
then
	if [ "$LAYER3" != "1" ]
	then
	    [ -n "$UAMPORT" -a "$UAMPORT" != "0" ] && \
		ipt4 $iptable_name -t filter -I $portal_input_filter_x -p tcp -m tcp --dport $UAMPORT --dst $ADDR -j ACCEPT
	    
	    [ -n "$UAMUIPORT" -a "$UAMUIPORT" != "0" ] && \
		ipt4 $iptable_name -t filter -I $portal_input_filter_x -p tcp -m tcp --dport $UAMUIPORT --dst $ADDR -j ACCEPT
	    
	    [ -n "$HS_TCP_PORTS" ] && {
		for port in $HS_TCP_PORTS; do
		    ipt4 $iptable_name -t filter -I $portal_input_filter_x -p tcp -m tcp --dport $port --dst $ADDR -j ACCEPT
		done
	    }
	    
	    ipt4 $iptable_name -t filter -I $portal_input_filter_x -p udp -d 255.255.255.255 --destination-port 67:68 -j ACCEPT
	    ipt4 $iptable_name -t filter -I $portal_input_filter_x -p udp -d $ADDR --destination-port 67:68 -j ACCEPT
	    ipt4 $iptable_name -t filter -I $portal_input_filter_x -p udp --dst $ADDR --dport 53 -j ACCEPT
	    ipt4 $iptable_name -t filter -I $portal_input_filter_x -p icmp --dst $ADDR -j ACCEPT
	    
	    ipt4 $iptable_name -t filter -A $portal_input_filter_x -i $TUNTAP --dst $ADDR -j DROP
	    
	    if [ "$ONLY8021Q" != "1" ]
	    then
		ipt4 $iptable_name -t filter -I $portal_input_filter_x -i $DHCPIF -j DROP
	    fi
	fi
	
	if [ "$ONLY8021Q" != "1" ]
	then
	    ipt4 $iptable_name -t filter -I $portal_forward_filter_x -i $DHCPIF -j DROP
	    ipt4 $iptable_name -t filter -I $portal_forward_filter_x -o $DHCPIF -j DROP
	fi
	
	ipt4 $iptable_name -t filter -I $portal_forward_filter_x -i $TUNTAP -j ACCEPT
	ipt4 $iptable_name -t filter -I $portal_forward_filter_x -o $TUNTAP -j ACCEPT
	
        # Help out conntrack to not get confused
        # (stops masquerading from working)
        #ipt -I PREROUTING -t raw -j NOTRACK -i $DHCPIF
        #ipt -I OUTPUT -t raw -j NOTRACK -o $DHCPIF
	
        # Help out MTU issues with PPPoE or Mesh
	ipt4 $iptable_name -t filter -I $portal_forward_filter_x -p tcp -m tcp --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu
	ipt4 $iptable_name -t mangle -I $portal_forward_mangle_x -p tcp -m tcp --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu
	
	if [ "$HS_LAN_ACCESS" != "on" -a "$HS_LAN_ACCESS" != "allow" ]
	then
		ipt4 $iptable_name -t filter -I $portal_forward_filter_x -i $TUNTAP \! -o $HS_WANIF -j DROP
	fi

	ipt4 $iptable_name -t filter -I $portal_forward_filter_x -i $TUNTAP -o $HS_WANIF -j ACCEPT
	
	[ "$HS_LOCAL_DNS" = "on" ] && \
	    ipt4 $iptable_name -t nat -I $portal_prerouting_nat_x -i $TUNTAP -p udp --dport 53 -j DNAT --to-destination $ADDR
	# ifconfig $TUNTAP mtu $MTU
	if [ "$KNAME" != "" ]
	then
		ipt4 $iptable_name -t filter -I $portal_forward_filter_x -i $DHCPIF -m coova --name $KNAME -j ACCEPT
		ipt4 $iptable_name -t filter -I $portal_forward_filter_x -o $DHCPIF -m coova --name $KNAME --dest -j ACCEPT
		# always drop bridge between bridge in NAT mode
		ipt4 $iptable_name -t filter -I $portal_forward_filter_x -i $DHCPIF \! -o $HS_WANIF -j DROP
		ipt4 $iptable_name -t filter -I $portal_input_filter_x -i $DHCPIF -p tcp -m tcp --dport $UAMPORT --dst $ADDR -j ACCEPT
		ipt4 $iptable_name -t filter -I $portal_input_filter_x -i $DHCPIF -p tcp -m tcp --dport $UAMUIPORT --dst $ADDR -j ACCEPT
		ipt4 $iptable_name -t filter -I $portal_input_filter_x -i $DHCPIF -m coova ! --name $KNAME -j DROP
		ipt4 $iptable_name -t filter -I $portal_input_filter_x -i $DHCPIF -p udp --dport 53 -j ACCEPT
		#[ -n "$DHCPLISTEN" ] && [ "$HS_BRIDGEMODE" != "on" ] && ifconfig $DHCPIF $DHCPLISTEN netmask $HS_NETMASK
    fi
fi



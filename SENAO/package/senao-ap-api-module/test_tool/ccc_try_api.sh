#!/bin/sh
ip=172.27.0.77
port=4430
token=$(curl -v -k -X POST "https://${ip}:${port}/api/sys/login" -H "accept: */*" -H "Content-Type: application/json" -H "Authorization: Bearer " -d "{\"username\":\"admin\",\"passwd\":\"admin\"}" | grep -Po '"token": "\K[^"]*')

#echo ${token}

#curl -v -k -X GET "https://${ip}:${port}/api/mgm/local_upgrade_image" -H "accept: application/json" -H "Authorization: Bearer ${token}" | grep code



bs_data={\"enable\":\"true\",\"steering_type\":\"force_5g\",\"5g_rssi_threshold\":\"-75\",\"5g_client_percent\":\"45\"}

ts_data={\"enable\":\"true\",\"download_limit\":\"16\",\"upload_limit\":\"87\",\"perclient_download_limit\":\"22\",\"perclient_upload_limit\":\"99\"}

rser_data={\"retries\":\"0\",\"interval\":\"100\",\"server1Ip\":\"192.168.8.87\",\"server1Port\":\"77\",\"server1Secret\":\"123454321\",\"server2Ip\":\"192.168.8.87\",\"server2Port\":\"87\",\"server2Secret\":\"123454321\"}

aser_data={\"enable\":\"false\",\"nasId\":\"10\",\"nasIpAddr\":\"192.168.7.7\",\"server1Ip\":\"192.168.8.8\",\"server1Port\":\"88\",\"server1Secret\":\"123454321\",\"server2Ip\":\"192.168.8.8\",\"server2Port\":\"8\",\"server2Secret\":\"123454321\"}

wpa_data={\"key_interval\":\"300\",\"passphrase\":\"112323120\"}
security_data={\"encryption\":\"WPA2-PSK\",\"auth_type\":\"TKIP\",\"wpa\":$wpa_data,\"radius_server\":$rser_data,\"accounting_server\":$aser_data}

cp_data={\"enable\":true,\"opmode\":\"nat\",\"auth_type\":\"engenius_radius\",\"external_splash_url\":\"abccc\",\"after_splash_redirect_url\":\"defff\",\"enable_redirect_client_track\":true,\"session_timeout\":50,\"idle_timeout\":90,\"walled_garden\":\"ghkkkkkkk\"}
scheduling_data={\"enable\":\"true\",\"days\":{\"su\":{\"available\":\"true\",\"start\":\"00:00\",\"end\":\"23:59\"},\"mo\":{\"available\":\"true\",\"start\":\"00:00\",\"end\":\"23:59\"},\"tu\":{\"available\":\"true\",\"start\":\"00:00\",\"end\":\"23:59\"},\"we\":{\"available\":\"true\",\"start\":\"00:00\",\"end\":\"23:59\"},\"th\":{\"available\":\"true\",\"start\":\"00:00\",\"end\":\"23:59\"},\"fr\":{\"available\":\"true\",\"start\":\"00:00\",\"end\":\"23:59\"},\"sa\":{\"available\":\"true\",\"start\":\"00:00\",\"end\":\"23:59\"}}}

maclist="00:00:00:00:00:22 00:00:00:00:24:22"
l2_acl_data={\"enable\":\"false\",\"client_mac_list\":\"$maclist\",\"policy\":\"DENY\"}

ssid_data={\"enable\":\"true\",\"enable_bands\":\"2_4G\",\"ssid_name\":\"test123\",\"hidden_ssid\":\"false\",\"client_isolation\":\"true\",\"l2_isolation\":\"true\",\"vlan_isolation\":\"true\",\"vlan_id\":\"111\",\"fast_roaming\":\"false\",\"band_steering\":$bs_data,\"traffic_shaping\":$ts_data,\"wireless_security\":$security_data,\"captive_portal\":$cp_data,\"scheduling\":$scheduling_data,\"l2_acl\":$l2_acl_data}

ping_data={\"dst_ip_addr\":\"fe80::8adc:96ff:fe6a:76e9\",\"packet_size\":\"64\",\"number_of_ping\":\"100\"}

#traceroute_data={\"dst_ip_addr\":\"2001:0db8:85a3:0000:0000:8a2e:0370:7334\"}
traceroute_data={\"dst_ip_addr\":\"fe80::8adc:96ff:fe6a:76e9\"}

nslookup_data={\"dst_ip_addr\":\"8.8.8.8\"}

case "$1" in
    bs)
    path="wifi/ssid/$ssid_idx/band_steering"
    data=$bs_data
    ;;
    ts)
    path="wifi/ssid/$ssid_idx/traffic_shaping"
    data=$ts_data
    ;;
    rser)
    path="wifi/ssid/$ssid_idx/radius_server"
    data=$rser_data
    ;;
    aser)
    path="wifi/ssid/$ssid_idx/accounting_server"
    data=$aser_data
    ;;
    security)
    path="wifi/ssid/$ssid_idx/security"
    data=$security_data
    ;;
    cp)
    path="wifi/ssid/$ssid_idx/captive_portal"
    data=$cp_data
    ;;
    sch)
    path="wifi/ssid/$ssid_idx/scheduling"
    data=$scheduling_data
    ;;
    l2_acl)
    path="wifi/ssid/$ssid_idx/l2_acl"
    data=$l2_acl_data
    ;;
    ssid)
    path="wifi/ssid/$ssid_idx"
    data=$ssid_data
    ;;
    ping)
    path="mgm/tools/ping"
    data=$ping_data
    ;;
    tr)
    path="mgm/tools/traceroute"
    data=$traceroute_data
    ;;
    nl)
    path="mgm/tools/nslookup"
    data=$nslookup_data
    ;;
    *)
        echo "*************COMMAND*************************"
        echo "bs = band_steering"
        echo "ts = traffic_shaping"
        echo "rser = radius_server"
        echo "aser = accounting_server"
        echo "security = security"
        echo "cp = captive_portal"
        echo "sch = scheduling"
        echo "l2_acl = l2_acl"
        echo "ssid = ssid"
        echo "ping = ping"
        echo "tr = traceroute"
        echo "nl = nslookup"
        echo "**************************************"
        echo ""
        echo "sh test_json.sh bs 1"
        echo "send post data to /wifi/ssid/1/band_steering"
        exit 0
    ;;
esac

echo "send to $path"

## use --htp1.0 for server.reject-expect-100-with-417 issue
#    curl -v -k -X POST "https://192.168.1.1:4430/api/$path" -H "accept: */*" -H "Content-Type: application/json" -H     "Authorization: Basic YWRtaW46YWRtaW4=" -d"$data" --http1.0


case "$2" in
	g)
	curl -v -k -X GET "https://${ip}:${port}/api/$path" -H "accept: application/json" -H "Authorization: Bearer ${token}" | grep code
	;;
	p)
	echo "data = $data"
	curl -v -k -X POST "https://${ip}:${port}/api/$path" -H "accept: application/json" -H "Authorization: Bearer ${token}" -d"$data" | grep code
	;;
esac

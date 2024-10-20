#!/bin/sh

if [ "$2" == "" ];then
    ssid_idx=1
else
    ssid_idx=$2 
fi
echo "ssid_idx=$ssid_idx"


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
        echo "**************************************"
        echo ""
        echo "sh test_json.sh bs 1"
        echo "send post data to /wifi/ssid/1/band_steering"
        exit 0
    ;;
esac

echo "send to $path"
echo "data = $data"

## use --htp1.0 for server.reject-expect-100-with-417 issue
    curl -v -k -X POST "https://192.168.1.1:4430/api/$path" -H "accept: */*" -H "Content-Type: application/json" -H     "Authorization: Basic YWRtaW46YWRtaW4=" -d"$data" --http1.0

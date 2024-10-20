#!/usr/bin/python

# First :
# sudo pip install websocket-client

import datetime
from websocket import create_connection
ws = create_connection("ws://172.27.0.35:8002", subprotocols=["wamp-protocol"])
print("Connec websocket")
print("Send Hello")
ws.send("[1, \"senao\", {\"roles\": { \"subscriber\": {} } }]")
result =  ws.recv()
print("Received '%s'" % result)

print("Sent subscribe")
ws.send("[32, 11111111, { \"interval\" : 1, \"dst_ip_addr\" : \"172.27.0.13\", \"packet_size\" : 1000, \"number_of_ping\" : 15}, \"getPingResult\"]")
# ws.send("[32, 22222222, { \"interval\" : 5, \"radio\" : 0, \"ssid\" : 0}, \"getStationList\"]")
# ws.send("[32, 33333333, { \"interval\" : 1, \"dst_wlan_mac\" : \"00:03:07:12:34:60\", \"dst_lan_mac\" : \"88:DC:96:74:47:2F\", \"radio\" : 1, \"ssid\" : 1}, \"getAntennaResul\"]")
result =  ws.recv()
print("Received '%s'" % result)


while True:
    result =  ws.recv()
    print(datetime.datetime.now().time())
    print("Received '%s'" % result)

ws.close()

LuaQ @   @dist/usr/lib/lua/luci/model/cbi/admin_network/proto_static.lua           �   %  � @܀  �C� E�  ��  � E � ܃   �	�A��BAD �� �C �C� E� � � E � ܃  @�I�A�˃�A� �C�˃�A �C�˃�AD �C���AD �� �C �C� E�  �� � � � ܃  ����A��BAD �� �C �C� E�  � � E � ܃  ����A���AD �� �C �C� E� �� �  � ܃   �	�@�	�ƌ�BAD �� �C ��F܃ �C   ��C� ED � � � ܄  A�  ܃  � � �CH�Ń ������C�܃ �   ��C� E� ��	 � �	 ܄  A
  ܃  ���D
 � �
 � D  ����
 D��K�D� ��  �D  A� � E �� \ �  A �D �� `� �KB��	   
\E _��KD� ń  � E � \ \�  ����L�K���	 E
 \D KD� ń  E E �� \ \�   �	�L�KB��	 E
 \D E� KD�ń   E �E \� � �� � \�  I�L����	 AE
 �D  � ;      get_interface    option    Value    ipaddr 
   translate    IPv4 address 	   datatype    ip4addr    depends    proto    static 
   ListValue    netmask    IPv4 netmask    value    255.255.255.0    255.255.0.0 
   255.0.0.0    gateway    IPv4 gateway 
   broadcast    IPv4 broadcast    DynamicList    dns    Use custom DNS servers    cast    string    is_virtual    stp    Flag 7   Enable <abbr title="Spanning Tree Protocol">STP</abbr> 2   Enables the Spanning Tree Protocol on this bridge    rmempty    luci    model    network 	   has_ipv6 
   ip6assign    IPv6 assignment length L   Assign a part of given length of every public IPv6-prefix to this interface     	   disabled    64    max(64)    ip6hint    IPv6 assignment hint L   Assign prefix parts using this hexadecimal subprefix ID for this interface. 	!   	@   	      ip6addr    IPv6 address    ip6gw    IPv6 gateway    s 
   ip6prefix    IPv6 routed prefix A   Public prefix routed to this device for distribution to clients.     �                                                                                                                            #   #   #   #   #   #   #   #   $   %   %   %   %   (   (   (   (   (   (   (   (   )   *   *   *   *   -   -   -   .   .   .   -   .   0   1   2   2   2   2   4   4   4   4   5   5   5   5   5   5   6   6   6   5   6   7   7   :   :   :   :   :   :   :   <   <   <   <   <   <   =   =   =   <   >   >   >   >   >   >   ?   ?   ?   @   B   B   B   B   B   B   C   C   C   B   D   D   D   D   D   D   D   D   D   F   F   F   F   F   F   F   F   G   H   H   H   H   K   K   K   K   K   K   K   K   L   M   M   M   M   P   P   P   P   P   P   P   Q   Q   Q   P   R   S   S   S   S   U         map    �      section    �      net    �      ifc    �      ipaddr    �      netmask    �      gateway    �   
   broadcast    �      dns    �   
   accept_ra    �      send_rs    �      ip6addr    �      ip6gw    �      mtu    �      metric    �   
   ip6assign q   �      ip6hint �   �      (for index) �   �      (for limit) �   �      (for step) �   �      i �   �   
   ip6prefix �   �       
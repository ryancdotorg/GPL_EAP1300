#ifndef __CAPWAP_CWCfg_HEADER__
#define __CAPWAP_CWCfg_HEADER__

#define CW_BG_SCAN_KEEP_CYCLE (3)
#define CW_WIRELESS_RADIO_2G_CHANNEL_NUM                     14
#define CW_WIRELESS_RADIO_5G_CHANNEL_NUM                     26
#define CW_WIRELESS_RADIO_2G_UTILIZATION_FIELD_SIZE          (CW_WIRELESS_RADIO_2G_CHANNEL_NUM * sizeof(int))
#define CW_WIRELESS_RADIO_5G_UTILIZATION_FIELD_SIZE          (CW_WIRELESS_RADIO_5G_CHANNEL_NUM * sizeof(int))

#define MAX_HOST_NAME_SIZE 256
typedef unsigned char CWMacAddress[6];
typedef char CWHostName[MAX_HOST_NAME_SIZE];
typedef struct
{
    CWHostName hostName;
    unsigned short port;
    int controllerId;
} CWAcAddress;

#define WTP_CFG_NONE  0
#define WTP_CFG_START 1

typedef enum
{
    WTP_CFG_AP_NAME = WTP_CFG_START, /* 1 */
    WTP_CFG_AP_LOCATION, /* 2 */
    WTP_CFG_AP_IPV4, /* 3 */
    WTP_CFG_AP_RADIO_NUM, /* 4 */
    WTP_CFG_AP_RADIO_OPERATION_MODE, /* 5 */
    WTP_CFG_AP_RADIO_WIRELESS_MODE, /* 6 */
    WTP_CFG_AP_RADIO_COUNTRY_CODE, /* 7 */
    WTP_CFG_AP_RADIO_CHANNEL_HT_MODE, /* 8 */
    WTP_CFG_AP_RADIO_CHANNEL_EXT, /* 9 */
    WTP_CFG_AP_RADIO_CHANNEL, /* 10 */
    WTP_CFG_AP_RADIO_LIMITED_CLIENTS_ENABLE, /* 11 */
    WTP_CFG_AP_RADIO_LIMITED_CLIENTS, /* 12 */
    WTP_CFG_AP_RADIO_TX_POWER, /* 13 */
    WTP_CFG_AP_RADIO_AGGRE_ENABLE, /* 14 */
    WTP_CFG_AP_RADIO_AGGRE_FRAMES, /* 15 */
    WTP_CFG_AP_RADIO_AGGRE_MAXBYTES, /* 16 */
    WTP_CFG_AP_RADIO_WLAN_NUM, /* 17 */
    WTP_CFG_AP_WLAN_ENABLE, /* 18 */
    WTP_CFG_AP_WLAN_SSID, /* 19 */
    WTP_CFG_AP_WLAN_SUPPRESSED_SSID, /* 20 */
    WTP_CFG_AP_WLAN_STA_SEPARATION, /*  21 */
    WTP_CFG_AP_WLAN_ISOLATION, /* 22 */
    WTP_CFG_AP_WLAN_VLAN, /* 23 */
    WTP_CFG_AP_WLAN_SECURITY_MODE, /* 24 */
    WTP_CFG_AP_WLAN_WEP_AUTH_TYPE, /* 25 */
    WTP_CFG_AP_WLAN_WEP_KEY_LENGTH, /* 26 */
    WTP_CFG_AP_WLAN_WEP_DEF_KEY_ID, /* 27 */
    WTP_CFG_AP_WLAN_WEP_KEY, /* 28 */
    WTP_CFG_AP_WLAN_WPA_ENCRYPT_MODE, /* 29 */
    WTP_CFG_AP_WLAN_WPA_PASSPHRASE, /* 30 */
    WTP_CFG_AP_WLAN_WPA_GROUP_KEY_INT, /* 31 */
    WTP_CFG_AP_WLAN_RADIUS_ADDR, /* 32 */
    WTP_CFG_AP_WLAN_RADIUS_PORT, /* 33 */
    WTP_CFG_AP_WLAN_RADIUS_SECRET, /* 34 */
    WTP_CFG_AP_WLAN_RADIUS_ACC_ENABLE, /* 35 */
    WTP_CFG_AP_WLAN_RADIUS_ACC_ADDR, /* 36 */
    WTP_CFG_AP_WLAN_RADIUS_ACC_PORT, /* 37 */
    WTP_CFG_AP_WLAN_RADIUS_ACC_SECRET, /* 38 */
    WTP_CFG_AP_WLAN_ACL_MODE, /* 39 */
    WTP_CFG_AP_WLAN_ACL_MAC_LIST, /* 40 */
    WTP_CFG_AP_DNS1, /* 41 */
    WTP_CFG_AP_DNS2, /* 42 */
    WTP_CFG_AP_ADMIN, /* 43 */
    WTP_CFG_AP_PASSWORD_MD5, /* 44 */
    WTP_CFG_AP_RADIO_DATA_RATE, /* 45 */
    WTP_CFG_AP_RADIO_RTSCTS_THRESHOLD, /* 46 */
    WTP_CFG_AP_WLAN_RADIUS_ACC_INTERIM_INT, /* 47 */
    WTP_CFG_AP_RADIO_OBEY_REGULATORY_POWER, /* 48 */
    WTP_CFG_AP_WLAN_WEP_INPUT_METHOD, /* 49 */
    WTP_CFG_AP_BAND_STREERING, /* 50 */
    WTP_CFG_AP_FAST_HANDOVER_STATUS, /* 51 */
    WTP_CFG_AP_FAST_HANDOVER_RSSI, /* 52 */
    WTP_CFG_AP_WLAN_DOWNLOAD_LIMIT, /* 53 */
    WTP_CFG_AP_WLAN_UPLOAD_LIMIT, /* 54 */
    WTP_CFG_AP_WLAN_ROAMING_ENABLE, /* 55 */
    WTP_CFG_AP_WLAN_ROAMING_ADV_SEARCH, /* 56 */
    WTP_CFG_AP_GUEST_NETWORK_IP, /* 57 */
    WTP_CFG_AP_GUEST_NETWORK_MASK, /* 58 */
    WTP_CFG_AP_GUEST_NETWORK_DHCP_START, /* 59 */
    WTP_CFG_AP_GUEST_NETWORK_DHCP_END, /* 60 */
    WTP_CFG_AP_GUEST_NETWORK_WINS_SERVER, /* 61 */
    WTP_CFG_AP_RADIO_DISTANCE, /* 62 */
    WTP_CFG_AP_LED_POWER, /* 63 */
    WTP_CFG_AP_LED_LAN, /* 64 */
    WTP_CFG_AP_LED_WLAN0, /* 65 */
    WTP_CFG_AP_LED_WLAN1, /* 66 */
    WTP_CFG_AP_LED_MESH, /* 67 */
    WTP_CFG_AP_WLAN_LAYER2_ISOLATION, /* 68 */ /*Darren : avoid error id*/
    WTP_CFG_AP_RADIO_PORTAL_ENABLE, /* 69 */
    WTP_CFG_AP_RADIO_PORTAL_LOGIN_TYPE, /* 70 */
    WTP_CFG_AP_RADIO_PORTAL_RADIUS_SERVER, /* 71 */
    WTP_CFG_AP_RADIO_PORTAL_REDIRECT_BEHAVIOR, /* 72 */
    WTP_CFG_AP_RADIO_PORTAL_WALLEDGARDEN, /* 73 */
    WTP_CFG_AP_RADIO_PORTAL_WALLEDGARDEN_PAGE, /* 74 */
    WTP_CFG_AP_RADIO_PORTAL_RADIUS_PORT, /* 75 */
    WTP_CFG_AP_RADIO_PORTAL_RADIUS_SECRET, /* 76 */
    WTP_CFG_AP_RADIO_PORTAL_SESSION_TIMEOUT, /* 77 */
    WTP_CFG_AP_RADIO_PORTAL_SETIMEOUT_ENABLE, /* 78 */
    WTP_CFG_AP_RADIO_PORTAL_IDLE_TIMEOUT, /* 79 */
    WTP_CFG_AP_RADIO_PORTAL_IDTIMEOUT_ENABLE, /* 80 */
    WTP_CFG_AP_RADIO_PORTAL_ACCOUNTING_ENABLE, /* 81 */
    WTP_CFG_AP_RADIO_PORTAL_ACCOUNTING_TIME, /* 82 */
    WTP_CFG_AP_RADIO_PORTAL_AUTH_TYPE, /* 83 */
    WTP_CFG_AP_RADIO_PORTAL_EXTERNAL_SERVER, /* 84 */
    WTP_CFG_AP_RADIO_PORTAL_EXTERNAL_SECRET, /* 85 */
    WTP_CFG_AP_RADIO_PORTAL_ACCOUNTING_SERVER, /* 86 */
    WTP_CFG_AP_RADIO_PORTAL_ACCOUNTING_PORT, /* 87 */
    WTP_CFG_AP_RADIO_PORTAL_ACCOUNTING_SECRET, /* 88 */
    WTP_CFG_AP_MANAGEMENT_VLAN_ID, /* 89 */
    WTP_CFG_AP_AC_LIST, /* 90 */
    WTP_CFG_AP_RADIO_PORTAL_UAMFORMAT, /* 91 */
    WTP_CFG_AP_RADIO_PORTAL_LOCAL_AUTH, /* 92 */
    WTP_CFG_AP_LAN_PORT_NUM, /* 93 */
    WTP_CFG_AP_LAN_PORT_ENABLE, /* 94 */
    WTP_CFG_AP_LAN_PORT_VLAN_ID, /* 95 */
    WTP_CFG_AP_RADIO_PORTAL_PORT, /* 96 */
    WTP_CFG_AP_RADIO_PORTAL_HTTPS_ENABLE, /* 97 */
    WTP_CFG_AP_RADIO_PORTAL_RADIUS_SECRET2, /* 98*/
    WTP_CFG_AP_RADIO_PORTAL_RADIUS_SERVER2, /* 99 */
    WTP_CFG_AP_BANDSTREERING_MODE, /* 100 */
    WTP_CFG_AP_BANDSTREERING_PERCENT_ENABLE, /* 101 */
    WTP_CFG_AP_BANDSTREERING_RSSI_ENABLE, /* 102 */
    WTP_CFG_AP_BANDSTREERING_RSSI, /* 103 */
    WTP_CFG_AP_BANDSTREERING_PERCENT, /* 104 */
    WTP_CFG_AP_WLAN_NAS_ID_ENABLE, /* 105 */
    WTP_CFG_AP_WLAN_NAS_ID, /* 106 */
    WTP_CFG_AP_WLAN_NAS_PORT_ENABLE, /* 107 */
    WTP_CFG_AP_WLAN_NAS_PORT, /* 108 */
    WTP_CFG_AP_LAN_PORT_VLAN_MODE, /* 109 */
    WTP_CFG_AP_MESH_ENABLE_TOTAL, /* 110 */
    WTP_CFG_AP_MESH_ENABLE, /* 111 */
    WTP_CFG_AP_MESH_ID, /* 112 */
    WTP_CFG_AP_MESH_WPA_KEY, /* 113 */
    WTP_CFG_AP_MESH_LINK_ROBUST_THRESHOLD, /* 114 */
    WTP_CFG_AP_WLAN_NAS_IP_ENABLE,/* 115 */
    WTP_CFG_AP_WLAN_NAS_IP, /* 116 */
    WTP_CFG_AP_WLAN_WPA_PMF_ENABLE, /* 117 */
    WTP_CFG_AP_WLAN_DOWNLOAD_PERUSER_ENABLE, /* 118 */
    WTP_CFG_AP_WLAN_UPLOAD_PERUSER_ENABLE, /* 119 */
    WTP_CFG_AP_RADIO_FAST_HANDOVER_STATUS, /* 120 */
    WTP_CFG_AP_RADIO_FAST_HANDOVER_RSSI, /* 121 */
    WTP_CFG_AP_WLAN_PORTAL_NETRORK_MODE, /* 122 */
    WTP_CFG_AP_WLAN_PORTAL_ENABLE, /* 123 */
    WTP_CFG_AP_WLAN_PORTAL_LOGIN_TYPE, /* 124 */
    WTP_CFG_AP_WLAN_PORTAL_RADIUS_SERVER, /* 125 */
    WTP_CFG_AP_WLAN_PORTAL_REDIRECT_BEHAVIOR, /* 126 */
    WTP_CFG_AP_WLAN_PORTAL_WALLEDGARDEN, /* 127*/
    WTP_CFG_AP_WLAN_PORTAL_WALLEDGARDEN_PAGE, /* 128 */
    WTP_CFG_AP_WLAN_PORTAL_RADIUS_PORT, /* 129 */
    WTP_CFG_AP_WLAN_PORTAL_RADIUS_SECRET, /* 130 */
    WTP_CFG_AP_WLAN_PORTAL_SESSION_TIMEOUT, /* 131 */
    WTP_CFG_AP_WLAN_PORTAL_SETIMEOUT_ENABLE, /* 132 */
    WTP_CFG_AP_WLAN_PORTAL_IDLE_TIMEOUT, /* 133 */
    WTP_CFG_AP_WLAN_PORTAL_IDTIMEOUT_ENABLE, /* 134 */
    WTP_CFG_AP_WLAN_PORTAL_ACCOUNTING_ENABLE, /* 135 */
    WTP_CFG_AP_WLAN_PORTAL_ACCOUNTING_TIME, /* 136 */
    WTP_CFG_AP_WLAN_PORTAL_AUTH_TYPE, /* 137 */
    WTP_CFG_AP_WLAN_PORTAL_EXTERNAL_SERVER, /* 138*/
    WTP_CFG_AP_WLAN_PORTAL_EXTERNAL_SECRET, /* 139 */
    WTP_CFG_AP_WLAN_PORTAL_ACCOUNTING_SERVER, /* 140 */
    WTP_CFG_AP_WLAN_PORTAL_ACCOUNTING_PORT, /* 141 */
    WTP_CFG_AP_WLAN_PORTAL_ACCOUNTING_SECRET, /* 142 */    
    WTP_CFG_AP_WLAN_PORTAL_PORT, /* 143 */
    WTP_CFG_AP_WLAN_PORTAL_HTTPS_ENABLE, /* 144*/
    WTP_CFG_AP_WLAN_PORTAL_RADIUS_SECRET2, /* 145*/
    WTP_CFG_AP_WLAN_PORTAL_RADIUS_SERVER2, /* 146 */
    WTP_CFG_AP_WLAN_PORTAL_UAMFORMAT, /* 147 */
    WTP_CFG_AP_WLAN_PORTAL_LOCAL_AUTH, /* 148 */
    WTP_CFG_AP_WLAN_BANDSTREERING_MODE, /* 149 */
    WTP_CFG_AP_WLAN_BANDSTREERING_PERCENT_ENABLE, /* 150 */
    WTP_CFG_AP_WLAN_BANDSTREERING_RSSI_ENABLE, /* 151 */
    WTP_CFG_AP_WLAN_BANDSTREERING_RSSI, /* 152 */
    WTP_CFG_AP_WLAN_BANDSTREERING_PERCENT, /* 153 */
    WTP_CFG_AP_LED_WLAN2, /* 154 */
    WTP_CFG_AP_LOG_REMOTE_SERVER_ENABLE,/*155*/
    WTP_CFG_AP_LOG_REMOTE_SERVER_CONFIG,/*156*/
    WTP_CFG_AP_TIME_ZONE,/*157*/
    WTP_CFG_AP_LOG_TRAFFIC_ENABLE,/*158*/
    WTP_CFG_AP_WLAN_DOWNLOAD_LIMIT_MODE,/*159*/
    WTP_CFG_AP_WLAN_UPLOAD_LIMIT_MODE,/*160*/
    WTP_CFG_AP_WLAN_L2_ISOLATION_WHITE_MAC_LIST,/*161*/
    WTP_CFG_AP_RADIO_BIT_RATE, /* 162 */
    WTP_CFG_AP_RADIO_AX_ENABLE, /* 163 */
	WTP_CFG_AP_WLAN_HOTSPOT20_JSON,/*164*/
    WTP_CFG_AP_WLAN_SUITEB_ENABLE,/*165*/
    /* TO BE ADDED */
    WTP_CFG_AP_END,
} CWWtpCfgMsgType;

typedef enum
{
    WTP_CFG_SW_NAME = WTP_CFG_START, /* 1 */
    WTP_CFG_SW_LOCATION, /* 2 */
    WTP_CFG_SW_IPV4, /* 3 */
    WTP_CFG_SW_DNS1, /* 4 */
    WTP_CFG_SW_DNS2, /* 5 */
    WTP_CFG_SW_ADMIN, /* 6*/
    WTP_CFG_SW_PASSWORD, /* 7*/
    WTP_CFG_SW_PORT_NUM,/*8*/
    WTP_CFG_SW_PORT_NO,/*9*/
    WTP_CFG_SW_PORT_SPEED_MODE,/*10*/
    WTP_CFG_SW_PORT_FLOWCTL,/*11*/
    WTP_CFG_SW_POE_POWER_BUDGET,/*12*/
    WTP_CFG_SW_POE_PORT_NUM,/*13*/
    WTP_CFG_SW_POE_PORT_NO,/*14*/
    WTP_CFG_SW_POE_PORT_ENABLE,/*15*/
    WTP_CFG_SW_POE_PORT_PRIORITY,/*16*/
    WTP_CFG_SW_POE_PORT_POWER_LIMIT_TYPE,/*17*/
    WTP_CFG_SW_POE_PORT_POWER_LIMIT,/*18*/
    WTP_CFG_SW_AC_LIST, /* 19 */
    WTP_CFG_SW_REDUNDANT_MANAGED_MAC_LIST, /* 20 */
    WTP_CFG_SW_REDUDNANCY, /* 21 */
    WTP_CFG_SW_PORT_DESCRIPTION,/*22*/
    /* TO BE ADDED */
    WTP_CFG_SW_END,
} CWWtpCfgMsgSwitchType;

#if WTP_CFG_AP_END >= WTP_CFG_SW_END
#define WTP_CFG_MAX_NUM WTP_CFG_AP_END
#else
#define WTP_CFG_MAX_NUM WTP_CFG_SW_END
#endif
#define WTP_CFG_AP_DEFAULT_SAVED_CFG_ID (WTP_CFG_AP_GUEST_NETWORK_WINS_SERVER)
#define WTP_CFG_AP_CURRENT_SAVED_CFG_ID (WTP_CFG_AP_END - 1)

/* A bitmask array to indicate which cfg type is supported */
typedef unsigned char CWWtpCfgCap[((WTP_CFG_MAX_NUM - 1) + 7) / 8];

#define CW_WTP_CFG_CAP_ON(_cap,_cfgType) \
	do{_cap[(_cfgType - 1) >> 3] |= 1 << ((_cfgType - 1) & 0x7);} while(0)

#define CW_WTP_CFG_CAP_OFF(_cap,_cfgType) \
	do{_cap[(_cfgType - 1) >> 3] &= ~(1 << ((_cfgType - 1) & 0x7));} while(0)

#define CW_WTP_CFG_CAP_CHECK(_cap,_cfgType) \
	(_cap[(_cfgType - 1) >> 3] & (1 << ((_cfgType - 1) & 0x7)))

#define CW_WTP_CFG_CAP_CLEAR(_cap) \
	do{CW_ZERO_MEMORY(_cap, sizeof(CWWtpCfgCap));} while(0)

typedef struct
{
    int ipNum;
    CWHostName *address;
} CWAcListCfg;

typedef struct
{
    int macNum;
    CWMacAddress *mac;
} CWMacList;

typedef struct
{
    int count;
    CWWtpCfgCap *cfgCap;
} CWWtpCfgCapInfo;

typedef enum
{
    CW_FALSE = 0,
    CW_TRUE = 1
} CWBool;

typedef struct
{
    unsigned int address; /* 0.0.0.0 means dhcp */
    unsigned int mask;
    unsigned int gateway;
} CWIPv4Cfg;

typedef struct
{
    unsigned int address; /* 0.0.0.0 means dhcp */
    unsigned int mask;
    unsigned int gateway;
    unsigned int dns1;
    unsigned int dns2;
} CWJoinIPv4Cfg;

typedef struct
{
    unsigned int address;
    unsigned int mask;
    unsigned int dhcpStart;
    unsigned int dhcpEnd;
    unsigned int winsServer;
} CWGuestNetworkCfg;

typedef enum
{
    CW_WEP_AUTH_TYPE_OPEN_SYSTEM = 0,
    CW_WEP_AUTH_TYPE_SHARED_KEY,
} CWWEPAuthType;

typedef enum
{
    CW_WEP_INPUT_METHOD_HEX = 0,
    CW_WEP_INPUT_METHOD_ASCII,
} CWWEPInputMethod;

#define CW_WTP_CFG_MAX_WEPKEY 4

typedef struct
{
    CWWEPAuthType authType;
    CWWEPInputMethod inputMethod;
    int keyLength;
    int defaultKey;
    char *keys[CW_WTP_CFG_MAX_WEPKEY];
} CWWEPCfg;

typedef struct
{
    unsigned int addr;
    unsigned short port;
    char *secret;

} CWRadiusCfg;

typedef struct
{
    CWBool enable;
    unsigned int addr;
    unsigned short port;
    char *secret;
    int intermiInterval;
} CWAccounting;
enum
{
    CW_WPA_GROUP_KEY_UPDATE_INTVAL_MIN = 30,
    CW_WPA_GROUP_KEY_UPDATE_INTVAL_MAX = 3600
};

typedef enum
{
    CW_WPA_ENCRPTION_TKIP_AES = 0,
    CW_WPA_ENCRPTION_TKIP,
    CW_WPA_ENCRPTION_AES,
} CWWPAEncrytion;

typedef struct
{
    int id_enable;
    char *id;
    int port_enable;
    unsigned short port;
    int ip_enable;
    unsigned int addr;
} CWNASCfg;

typedef struct
{
    CWWPAEncrytion encryption;
    char *passphrase;
    int groupKeyUpdateInterval;
    CWRadiusCfg radius;
    int pmf_enable;
} CWWPACfg;

typedef struct
{
    int enableTotal;
} CWMeshGlobalCfg;

typedef struct
{
    int enable;
    char *id;
    char *wpaKey;
    short linkRobustThreshold;
} CWMeshCfg;

typedef struct
{
    int enable;
    char *conf;
} CWApLogRemoteCfg;

typedef enum
{
    CW_WLAN_SECURITY_MODE_NONE = 0,
    CW_WLAN_SECURITY_MODE_WEP,
    CW_WLAN_SECURITY_MODE_WPA_PSK,
    CW_WLAN_SECURITY_MODE_WPA2_PSK,
    CW_WLAN_SECURITY_MODE_WPA_PSK_MIXED,
    CW_WLAN_SECURITY_MODE_WPA,
    CW_WLAN_SECURITY_MODE_WPA2,
    CW_WLAN_SECURITY_MODE_WPA_MIXED,
    CW_WLAN_SECURITY_MODE_OWE,
    CW_WLAN_SECURITY_MODE_WPA3_PERSONAL,
    CW_WLAN_SECURITY_MODE_WPA2_WPA3_PERSONAL_MIXED,
    CW_WLAN_SECURITY_MODE_WPA3,
    CW_WLAN_SECURITY_MODE_WPA3_MIXED,
} CWWLANSecurityMode;

typedef enum
{
    CW_WLAN_ACL_MODE_NONE = 0,
    CW_WLAN_ACL_MODE_ALLOW,
    CW_WLAN_ACL_MODE_DENY,
} CWWlanAclMode;

typedef struct
{
    CWWlanAclMode mode;
    int macCount;
    CWMacAddress *macs;
} CWWlanAcl;

typedef enum
{
    CW_RATE_MBPS = 0,
    CW_RATE_KBPS,
    
} CWRateMode;

typedef struct
{    
    int macCount;
    CWMacAddress *macs;
} CWWlanL2IsolatWhiteList;


typedef struct
{
    int downloadlimit;
    int uploadlimit;
    int downperUser;
    int uploadperUser;
    CWRateMode downloadMode;
    CWRateMode uploadMode;
} CWWlanTrafficShaping;

typedef struct
{
    int enable;
    int advSearch;
} CWWlanRoaming;


typedef enum
{
    CW_RADIO_COUNTRY_CODE_NONE = 0,
    CW_RADIO_COUNTRY_CODE_JAPAN = 392
} CWRadioCountryCode;

typedef enum
{
    CW_RADIO_OPERATION_MODE_DISABLED = 0,
    CW_RADIO_OPERATION_MODE_AP,
    CW_RADIO_OPERATION_MODE_AD_HOC,
} CWRadioOperationMode;

typedef enum
{
    CW_RADIO_WIRELESS_MODE_A = 0x1,
    CW_RADIO_WIRELESS_MODE_B = 0x2,
    CW_RADIO_WIRELESS_MODE_G = 0x4,
    CW_RADIO_WIRELESS_MODE_N_2G = 0x8,
    CW_RADIO_WIRELESS_MODE_N_5G = 0x10,
    CW_RADIO_WIRELESS_MODE_BGN = (CW_RADIO_WIRELESS_MODE_B | CW_RADIO_WIRELESS_MODE_G | CW_RADIO_WIRELESS_MODE_N_2G),
    CW_RADIO_WIRELESS_MODE_BG = (CW_RADIO_WIRELESS_MODE_B | CW_RADIO_WIRELESS_MODE_G),
    CW_RADIO_WIRELESS_MODE_AN = (CW_RADIO_WIRELESS_MODE_A | CW_RADIO_WIRELESS_MODE_N_5G),
    CW_RADIO_WIRELESS_MODE_ACN_5G = 0x20,
    CW_RADIO_WIRELESS_MODE_AX = 0x40
} CWRadioWirelessMode;

typedef enum
{
    CW_RADIO_CHANNEL_HT_20_40_MHZ = 0,
    CW_RADIO_CHANNEL_HT_20_MHZ,
    CW_RADIO_CHANNEL_HT_40_MHZ,
    CW_RADIO_CHANNEL_HT_80_MHZ
} CWRadioChannelHTMode;

typedef enum
{
    CW_RADIO_EXTENSION_CHANNEL_UPPER = 0,
    CW_RADIO_EXTENSION_CHANNEL_LOWER,
} CWRadioExtensionChannel;

typedef enum
{
    CW_RADIO_CHANNEL_AUTO = 0,
} CWRadioChannel;

typedef enum
{
    CW_RADIO_DATA_RATE_AUTO = 0,
    CW_RADIO_DATA_RATE_MCS_MIN = 1,
    CW_RADIO_DATA_RATE_MCS_MAX = 32,
    CW_RADIO_DATA_RATE_MCS_MIN_AC = 101,
    CW_RADIO_DATA_RATE_MCS_MAX_AC = 110
} CWRadioDataRate;

enum
{
    CW_RADIO_TX_POWER_AUTO = 0,
};

enum
{
    CW_RADIO_RTS_CTS_THRESHOLD_MIN = 1,
    CW_RADIO_RTS_CTS_THRESHOLD_MAX = 2346
};

enum
{
    CW_RADIO_LIMITED_CLIENT_MIN = 1,
    CW_RADIO_LIMITED_CLIENT_MAX = 127
};

enum
{
    CW_RADIO_AGGREGATION_FRAMES_MIN = 1,
    CW_RADIO_AGGREGATION_FRAMES_MAX = 32,
    CW_RADIO_AGGREGATION_BYTES_MIN = 2304,
    CW_RADIO_AGGREGATION_BYTES_MAX = 65535,
};

typedef struct
{
    CWRadioChannelHTMode HTMode;
    CWRadioExtensionChannel extension;
    CWRadioChannel channel;
} CWRadioChannelCfg;

typedef struct
{
    CWBool enable;
    int frames;
    int maxBytes;
} CWRadioAggregationCfg;

typedef struct
{
    CWBool enable;
    int maxClients;
} CWRadioLimitedClientsCfg;

typedef struct
{
    CWBool enable;
    char *address;
} CWPortalWallGarden;

typedef enum
{
    CW_PORTAL_LOGIN_TYPE_SPLASH = 0,
    CW_PORTAL_LOGIN_TYPE_LOCAL,
    CW_PORTAL_LOGIN_TYPE_RADIUS,
    CW_PORTAL_LOGIN_TYPE_CLOUD4WI,
} CWPortalLoginType;

typedef enum
{
    CW_PORTAL_AUTH_CHAP = 0,
    CW_PORTAL_AUTH_PAP,
} CWPortalAuthType;

typedef enum
{
    CW_PORTAL_NETWORK_DISALBED=0,
    CW_PORTAL_NETWORK_ENABLED,
    CW_PORTAL_NETWORK_BRIDAGE,
    CW_PORTAL_NETWORK_NAT    
} CWPortalNetorkType;


#define CW_DEFAULT_PORTAL_UAMFORMAT  "sn.captivePortal.login"
#define CW_DEFAULT_PORTAL_LOCAL_AUTH "sn.captivePortal.auth"

typedef struct
{
    CWBool enable;
    CWPortalLoginType type;
    char *redirBehv;
    CWPortalWallGarden garden;
    int sessionTimeout;
    CWBool sessionTimeoutEnable;
    int idleTimeout;
    CWBool idleTimeoutEnable;
    CWPortalAuthType authType;
    char *exterServer;
    char *extersecret;
    char *uamFormat;
    char *localAuth;
    CWRadiusCfg radius;
    CWAccounting accounting;
    char *termofuse;
    int termofuseEnable;
    unsigned short port;
    char *logoPath;
    CWBool httpsEnable;
    char *radius_secret2;
    unsigned int radius_addr2;
    CWPortalNetorkType network_type;
    char *capmsg;
} CWPortalCfg;

typedef struct
{
    int status;
    int rssi;
} CWFastHandoverCfg;

typedef struct
{
    int mode;
    int percent_enable;
    int rssi_enable;
    int rssi;
    int percent;
} CWWTPBandsteerCfg;

typedef struct
{
    CWBool enable;
    char *ssid;
    CWBool suppressedSSID;
    CWBool stationSeparation;
    CWBool isolation;
    int vlan;
    CWWlanAcl acl;
    CWWlanRoaming roaming;
    CWWlanTrafficShaping traffic;
    CWWLANSecurityMode securityMode;
    CWWEPCfg wep;
    CWWPACfg wpa;
    CWAccounting accounting;
    CWBool layer2Isolation;
    CWWlanL2IsolatWhiteList l2IsoWlist;
    CWNASCfg nas;
    CWPortalCfg portal;
    CWWTPBandsteerCfg bandsteering;
    char *hotspot20Json;
} CWWlanCfg;
typedef struct
{
    CWRadioCountryCode countryCode;
    CWRadioOperationMode operMode;
    //	CWBool meshEnable;
    CWRadioWirelessMode wirelessMode;
    CWRadioChannelCfg channel;
    int txPower;
    CWBool obeyRegulatoryPower;
    int dataRate;
    int bitRate;
    int ax_enable;
    int rtsCtsThreshold;
    CWRadioLimitedClientsCfg limitedClients;
    CWRadioAggregationCfg aggregation;
    int distance; /* for out-door AP */
    int wlanCount;
    CWWlanCfg *wlans;
    CWPortalCfg portal;    
    CWMeshCfg mesh;
    CWFastHandoverCfg fasthandover;
} CWRadioCfg;

typedef struct
{
    int enable;
    int vid;
} CWManageVlanCfg;

typedef struct
{
    int power;
    int lan;
    int wlan0;
    int wlan1;
    int wlan2;
    int mesh;
} CWLedCfg;

typedef struct
{
    int enable;/*0:disable 1:enable*/
    int vlanId;
    int vlanMode;
} CWWTPApLanPortCfg;

typedef struct
{
    int port;
    int mode;
    int flowControl;/*0:disable 1:enable*/
    char *descp;
} CWWTPSwPortCfg;

typedef struct
{
    int port;
    int enable;
    int priority;
    int powerType;/*auto/user define*/
    int powerLimit;
} CWWTPSwPoePortCfg;


typedef struct
{
    CWIPv4Cfg ipv4;
    unsigned int dns1;
    unsigned int dns2;
    char *name;
    char *location;
    char *admin;
    int timeZone;
    /******** AP ***********/
    unsigned char passwordMD5[16];
    int bandsteer;
    CWWTPBandsteerCfg bandsteering;
    CWFastHandoverCfg fasthandover;
    CWGuestNetworkCfg guestNetwork;
    CWManageVlanCfg	manageVlan;
    int radioCount;
    CWRadioCfg *radios;
    CWLedCfg led;
    int lanPortNum;
    CWWTPApLanPortCfg *lanPortCfg;
    CWMeshGlobalCfg mesh;

    /********* SW *************/
    char *password;
    int portNum;
    CWWTPSwPortCfg *portCfg;
    int poePowerBuget;
    int poePortNum;
    CWWTPSwPoePortCfg *poePortCfg;
    CWAcListCfg aclist;
    CWMacList manageList;
    int redundancyMode;
    int remoteLogSrvEn;
    int remoteTrafficEn;
    char *remoteLogSrvConf;
} CWWtpCfg;

typedef struct
{
    CWMacAddress bssid;
    char ssid[33]; // A SSID for a wireless network has a maximum length of 32 characters
    unsigned char ssidLen;
    CWRadioOperationMode mode;
    CWRadioChannel chan;
    int signal;
    CWWLANSecurityMode enc;
    CWRadioWirelessMode type;
    CWRadioChannelHTMode htmode;
    CWRadioExtensionChannel extch;
} CWWTPSitesurvey;

typedef enum
{
    CW_RADIOFREQTYPE_2G = 0,
    CW_RADIOFREQTYPE_5G,
    CW_RADIOFREQTYPE_5G_1,
    CW_RADIOFREQTYPE_NONE
} CWRadioFreqType;

typedef struct
{
    CWMacAddress mac;
    CWRadioFreqType radio;
    int curChannel;
    int curTxPower;
    unsigned char version;
    int infoCount;
    CWWTPSitesurvey *info;
    unsigned char *chanUtil;
} CWWTPSitesurveyInfo;

typedef struct
{
    CWRadioFreqType radioType;
    int oldChannel;
    int newChannel;
} CWWTPAutoChannelInfo;

typedef struct
{
    int radioIdx;
    int wlanIdx;
    int online; // 0:offline 1:online
    CWMacAddress mac;
    unsigned int ipAddr;
    char *hostName;
} CWWTPClientStateChange;

typedef struct
{
    int clientNum;
    CWWTPClientStateChange *info;
} CWWTPClientStateChangeInfo;

typedef struct
{
    CWMacAddress mac;
} KickmacClient;

typedef struct
{
    unsigned char radio;
    unsigned char wlan;
    unsigned short clientNum;
    KickmacClient *client;
} CWWTPKickmacInfo;

#endif

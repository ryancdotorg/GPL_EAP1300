/*******************************************************************************************
 * Copyright (c) 2006-9 Laboratorio di Sistemi di Elaborazione e Bioingegneria Informatica *
 *                      Universita' Campus BioMedico - Italy                               *
 *                                                                                         *
 * This program is free software; you can redistribute it and/or modify it under the terms *
 * of the GNU General Public License as published by the Free Software Foundation; either  *
 * version 2 of the License, or (at your option) any later version.                        *
 *                                                                                         *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY         *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A 	       *
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.                *
 *                                                                                         *
 * You should have received a copy of the GNU General Public License along with this       *
 * program; if not, write to the:                                                          *
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,                    *
 * MA  02111-1307, USA.                                                                    *
 *                                                                                         *
 * --------------------------------------------------------------------------------------- *
 * Project:  Capwap                                                                        *
 *                                                                                         *
 * Author :  Ludovico Rossi (ludo@bluepixysw.com)                                          *
 *           Del Moro Andrea (andrea_delmoro@libero.it)                                    *
 *           Giovannini Federica (giovannini.federica@gmail.com)                           *
 *           Massimo Vellucci (m.vellucci@unicampus.it)                                    *
 *           Mauro Bisson (mauro.bis@gmail.com)                                            *
 *           Antonio Davoli (antonio.davoli@gmail.com)                                     *
 *******************************************************************************************/


#ifndef __CAPWAP_CWCommon_HEADER__
#define __CAPWAP_CWCommon_HEADER__


#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>
#include <ctype.h>
#include "CWCfg.h"
#ifdef __GLIBC__
# include <net/ethernet.h>
#ifdef MACOSX
#include <netinet/if_ether.h>
#else
#include <linux/if_ether.h>
#endif
#endif

#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <inttypes.h>
#include <gconfig.h>
// #include "wireless_copy.h"

// make sure the types really have the right sizes
#define CW_COMPILE_TIME_ASSERT(name, x) typedef int CWDummy_ ## name[(x) * 2 - 1]

#define _STR(x)                             #x
#define STR(s)                              _STR(s)
#define CW_VERSION                          STR(CW_VERSION_MAJOR)"."STR(CW_VERSION_MINOR)"."STR(CW_VERSION_TARGET)

// if you get a compile error, change types (NOT VALUES!) according to your system
CW_COMPILE_TIME_ASSERT(int_size, sizeof(int) == 4);
/*begin,20150702,Andy Hu,modified for 64 bit platform*/
//CW_COMPILE_TIME_ASSERT(long_size, sizeof(long) == 4);  //marked
CW_COMPILE_TIME_ASSERT(short_size, sizeof(short) == 2);
CW_COMPILE_TIME_ASSERT(char_size, sizeof(char) == 1);

#define CW_PACKET_BUFFER_SIZE               1580
#define CW_BUFFER_SIZE                      65536
#define MAX_PACKET_SIZE                     CW_BUFFER_SIZE

#define CW_ZERO_MEMORY(dst, len)            bzero(dst, len)
#define CW_COPY_MEMORY(dst, src, len)       bcopy(src, dst, len)
#define CW_REPEAT_FOREVER                   while(1)
#define CWWaitSec(s)                        sleep(s)
#define CWWaitMicroSec(us)                  usleep(us)
#define DEFAULT_LOG_SIZE                    1000000
#define CW_EXIT_RESTART                     3
#define CW_DEFAULT_IMAGE_DATA_DIR           "/var/tmp/"
#define CW_AC_PID_FILE                      "/var/run/ac.pid"
#define CW_DEF_MEM_USAGE_LOG_THRESHOLD      90  /* percentage */

/* Require version 0.19.5+ to upgrade firmware */
#define CW_MIN_UPRGADABLE_VERSION_MAJOR      0
#define CW_MIN_UPRGADABLE_VERSION_MINOR      19
#define CW_MIN_UPRGADABLE_VERSION_TARGET     5

/* Require version 1.8.0+ to support 2-steps upgrade firmware */
#define CW_MIN_2STEPS_UPGRADABLE_VERSION_MAJOR      1
#define CW_MIN_2STEPS_UPRGADABLE_VERSION_MINOR      8
#define CW_MIN_2STEPS_UPRGADABLE_VERSION_TARGET     0

/* Require version 1.8.0+ to upgrade switch firmware */
#define CW_MIN_SWITCH_UPRGADABLE_VERSION_MAJOR      1
#define CW_MIN_SWITCH_UPRGADABLE_VERSION_MINOR      8
#define CW_MIN_SWITCH_UPRGADABLE_VERSION_TARGET     0

#define CW_VERION_ABOVE(_major1, _minor1, _target1, _major2, _minor2, _target2) \
    ((_major1 > _major2) || \
     (_major1 == _major2 && _minor1 > _minor2) || \
     (_major1 == _major2 && _minor1 == _minor2 && _target1 >= _target2))

/*switch info*/
#define SW_MAX_MODE_NAME 	64

/* AP Info */
#define CW_AUTO_TX_POWER_INTERVAL(_chNum, _sitesurveyInt)    ((_sitesurveyInt * _chNum + 20) < 600 ? 600 : (_sitesurveyInt * _chNum + 20));

#define CW_IPV4_PRINT_LIST(_addr) ((unsigned char*)&(_addr))[0],\
							((unsigned char*)&(_addr))[1],\
							((unsigned char*)&(_addr))[2],\
							((unsigned char*)&(_addr))[3]

#define CW_MAC_PRINT_LIST(_mac) (_mac)[0],\
							(_mac)[1],\
							(_mac)[2],\
							(_mac)[3],\
							(_mac)[4],\
							(_mac)[5]

#ifdef CW_DEBUGGING
#define CW_TRACE        \
	do{printf("%u %s %u\n",(unsigned int)time(NULL),__FUNCTION__,__LINE__);}while(0)
#define CW_ASSERT(x)	\
	do{if(!(x)){CWLog("ASSERT failed at %s line %u: "#x,__FILE__,__LINE__);exit(EXIT_FAILURE);}}while(0)
#else
#define CW_TRACE
#define CW_ASSERT(x)
#endif

typedef enum
{
    CW_WTP_OTHER_TYPE    = 0,
    CW_WTP_REPEATER_TYPE = 1,
    CW_WTP_SWITCH_TYPE   = 2,
    CW_WTP_AP_TYPE       = 3,
    CW_WTP_ROUTER_TYPE   = 4,
    CW_WTP_IPCAM_TYPE    = 8
} CWWTPType;

typedef enum
{
    CW_UPG_NONE = 0,
    CW_UPG_SUCCESS,
    CW_UPG_FAILED
} CWUpgResult;

#include "CWErrorHandling.h"

typedef enum
{
    WTP_CFG_ERROR_HANDLE_NONE = 0,
    WTP_CFG_ERROR_HANDLE_ROLLBACK,
    WTP_CFG_ERROR_HANDLE_RESYNC,
    WTP_CFG_ERROR_HANDLE_UNMANAGED,
} CWWtpCfgErrorHandle;

typedef struct wtp_cfg_msg_node
{
    int type;
    int keyLen;
    void *keyPtr;
    int valLen;
    void *valPtr;
    struct wtp_cfg_msg_node *next;
} CWWtpCfgMsgNode;

typedef struct
{
    int msgSize;
    CWWtpCfgMsgNode *head;
    CWWtpCfgMsgNode *tail;
} CWWtpCfgMsgList;

typedef struct
{
    CWWtpCfgMsgList cfgUpdateList;
    CWBool waitApply;
} CWWtpCfgInfo;

typedef struct
{
    CWBool apply; /* Indicate whether the config is to be applied */
    CWWtpCfgErrorHandle handle; /* unused now, How to handle if error occurs */
    CWBool rejoin; /* Indicate wtp will rejoin after applying the config (usually when network changed) */
    int waitSec; /* waiting seconds after applying the config */
    char message[512];
} CWWtpCfgResult;

typedef struct
{
    CWBool cancel;
    int waitTime; /* how many seconds to wait before applying */
} CWWTPApplyConfigInfo;

typedef enum
{
    CW_ENTER_SULKING,
    CW_ENTER_DISCOVERY,
    CW_ENTER_JOIN,
    CW_ENTER_CONFIGURE,
    CW_ENTER_DATA_CHECK,
    CW_ENTER_RUN,
    CW_ENTER_RESET,
    CW_ENTER_IMAGE,
    CW_ENTER_CERT_CHECK,
    CW_ENTER_AC,
    //CW_QUIT /* unused now */
} CWStateTransition;

typedef enum
{
    CW_UPG_ONE_STEP = 0,
    CW_UPG_TWO_STEP,
    CW_UPG_RUN,
} CWUpgFlow;

typedef enum
{
    CW_ADD = 1,
    CW_DEL
} CWCaluFlag;

typedef enum
{
    POE_NONE = 0,
    POE_ACTIVE
} CWPoeType;

typedef enum
{
    CW_SUB_STATE_IDLE = 0,
    CW_SUB_STATE_DTLS_HANDSHAKE,
    CW_SUB_STATE_WAITING_REQUEST,
    CW_SUB_STATE_RECEIVING_FIRMWARE,
    CW_SUB_STATE_BURNING_FIRMWARE,
    CW_SUB_STATE_APPLY_CONFIGURE,
    CW_SUB_STATE_WAITING_DTLS_HANDSHAKE,
    CW_SUB_STATE_UNUSED1, /* unused currently */
    CW_SUB_STATE_UNUSED2, /* unused currently */
    CW_SUB_STATE_WAITING_BURNING_FIRMWARE,
    CW_SUB_STATE_WAITING_APPLYING_CONFIG
} CWWtpSubState;

typedef struct
{
    unsigned int ip;
    unsigned int mask;
    unsigned int gateway;
    unsigned int dns1;
    unsigned int dns2;
} CWIPCfgInfo;

typedef struct
{
    CWMacAddress mac;
    unsigned int ip;
    unsigned short port;
} CWProxyRespWTPInfo;

typedef struct
{
    CWMacAddress mac;
    unsigned int txKB;
    unsigned int rxKB;
    union
    {
        int rssi;      /* for payload type = CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CURRENT_STATION_INFO */
        int interval;  /* for payload type = CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_STATION_STATISTIC */
    };
    unsigned char osTypeLen;
    unsigned char hostNameLen;
    char *osType;
    char *hostName;
    unsigned int address;
} CWStation;

typedef struct
{
    int radioIdx;
    int wlanIdx;
    int stationCount;
    CWStation *station;
} CWWTPStation;

typedef struct
{
    int role;
    CWMacAddress bssid;
} CWRadioMeshInfo;

typedef struct
{
    int infoCount;
    CWRadioMeshInfo *info;
} CWWTPMeshInfo;

typedef struct
{
    int port;
    int state;
    int class;
    int priority;
    int powerLimitType;
    int powerLimit;
    int status;
    int outputVoltage;
    int outputCurrent;
    int outputPower;
    int temperature;
} CWWTPSwitchPoePortInfo;

typedef struct
{
    int powerBudget;
    int allocPower;
    int infoCount;
    CWWTPSwitchPoePortInfo *info;
} CWWTPSwitchPoeInfo;

typedef struct
{
    int localPort;
    CWMacAddress mac;
    int remotePortLen;
    char *remotePort;
} CWWTPSwitchDevLink;

typedef struct
{
    int type;
    CWMacAddress mac;
    unsigned int ip;
    char *name;
    int nameLen;
    char *desc;
    int descLen;
    unsigned int updateTime;
    unsigned int sysTime;
    unsigned int linkInfoLastTime;
    unsigned int noLinkInfoFirstTime;
    int linkCount;
    CWWTPSwitchDevLink *linkList;
} CWWTPSwitchDev;

typedef struct
{
    int devCount;
    CWWTPSwitchDev *devInfo;
} CWWTPSwitchTopologyInfo;

typedef enum
{
    BIT_START = 0,
    FIBER_BIT = BIT_START,
    POE_BIT,
    COPPER_BIT,
    BIT_END
} CWPortBitType;

typedef struct
{
    int bitPos;
    char *name;
} CWPortbitName;

typedef struct
{
    int portNo;
    int portType;
} CWWTPSwitchHwPortInfo;

typedef struct
{
    int version;
    int numPort;
    int numTrunk;
    int numVlan;
    int numManageAp;
    int numMirror;
    int numMultiAddr;
    int maxPowerBudge;
    int minPowerBudge;
    int numStaitcMac;
    int numTrunkMember;
    int devsuptype;
    CWWTPSwitchHwPortInfo *info;
} CWWTPSwitchHwInfo;

typedef struct
{
    int port;
    int status; /*1:enable/0:disable*/
    int linkStatus; /*1:up /0:down*/
    int speed;
    int duplex; /*0:half 1:full 2:auto*/
    int flowControl; /*0:disable 1:enable*/
    int autoNeg;
} CWWTPSwitchPortStatus;

typedef struct
{
    int infoCount;
    CWWTPSwitchPortStatus *info;
} CWWTPSwitchPortInfo;

typedef struct
{
    int id;
    int mode;
    char *activePort;
    char *memberPort;
    int activePort_len;
    int memberPort_len;
} CWWTPSwitchTrunkStatus;

typedef struct
{
    int infoCount;
    CWWTPSwitchTrunkStatus *info;
} CWWTPSwitchTrunkInfo;

typedef struct
{
    int infoCount;
    CWWTPStation *info;
} CWWTPStationInfo;

typedef struct
{
    unsigned int txPkts;
    unsigned int txErrPkts;
    unsigned int txDrpPkts;
    unsigned int txBytes;
    unsigned int rxPkts;
    unsigned int rxErrPkts;
    unsigned int rxDrpPkts;
    unsigned int rxBytes;
} CWWlanStatistics;

typedef struct
{
    int radioIdx;
    int wlanIdx;
    CWWlanStatistics statistics;
} CWWlanStatisticsInfo;

typedef struct
{
    int infoCount;
    CWWlanStatisticsInfo *info;
} CWWTPStatisticsInfo;

typedef struct
{
    CWMacAddress bssid;
} CWWTPCurrentWlanCfgInfo;

typedef struct
{
    int channel;
    int txPower;
    int wlanCount;
    CWWTPCurrentWlanCfgInfo *wlan;
    unsigned char channelCount;
    unsigned char *channelList;
} CWWTPCurrentRadioCfgInfo;

typedef struct
{
    int version;
    CWIPv4Cfg ipv4;
    unsigned int dns1;
    unsigned int dns2;
    int radioCount;
    CWWTPCurrentRadioCfgInfo *radio;
} CWWTPCurrentCfgInfo;

typedef struct
{
    int dtlsSetup;
    int joinState;
    int imageState;
    int imageData;
    int configState;
    int changeState;
    int peerDead;
    int retransmitInterval;
    int retransmitCount;
    int fragment;
} CWTimer;

typedef struct
{
    CWRadioFreqType radioType;
    CWBool bEnable;
    unsigned int uint32Interval;
} CWBackgroundSitesurveyValues;

typedef struct
{
    CWRadioFreqType radioType;
    int strength; //%
} CWAutoTxPowerHealingValues;

typedef struct
{
    int len;
    char *ptr;
} CWStringValue;

typedef struct
{
    unsigned int total;
    unsigned int used;
} CWMemoryInfo;

extern const char *CW_CONFIG_FILE;
extern int gCWForceMTU;
extern int gCWRetransmitTimer;
extern int gCWNeighborDeadInterval;
extern int gCWMaxRetransmit;
extern int gMaxLogFileSize;
extern int gEnabledLog;
extern int gEnabledDebugLog;

#if defined(CW_DEBUGGING_MEMORY) && !defined(BUILD_WUM)
void *CWMemAllocTrace(size_t size, const char *function, unsigned int line);
void *CWMemZeroAllocTrace(size_t size, const char *function, unsigned int line);
void CWMemFreeTrace(void *ptr, const char *function, unsigned int line);
void CWMemTracePrint(char *file);

#define CWMemAlloc(_size) CWMemAllocTrace(_size,__FUNCTION__,__LINE__)
#define CWMemZeroAlloc(_size) CWMemZeroAllocTrace(_size,__FUNCTION__,__LINE__)
#define CWMemFree(_ptr) CWMemFreeTrace(_ptr,__FUNCTION__,__LINE__)
#else
#define CWMemAlloc(_size) malloc(_size)
#define CWMemZeroAlloc(_size) calloc(1,_size)
#define CWMemFree free
#endif

#define	CW_FREE_OBJECT(obj_name)		{if(obj_name){CWMemFree((obj_name)); (obj_name) = NULL;}}
#define	CW_FREE_OBJECTS_ARRAY(ar_name, ar_size)	{int _i = 0; for(_i = ((ar_size)-1); _i >= 0; _i--) {if(((ar_name)[_i]) != NULL){ CWMemFree((ar_name)[_i]);}} CWMemFree(ar_name); (ar_name) = NULL; }
#define	CW_PRINT_STRING_ARRAY(ar_name, ar_size)	{int i = 0; for(i = 0; i < (ar_size); i++) printf("[%d]: **%s**\n", i, ar_name[i]);}

// custom error
#define	CW_CREATE_OBJECT_ERR(obj_name, obj_type, on_err)	{obj_name = (obj_type*) (CWMemAlloc(sizeof(obj_type))); if(!(obj_name)) {on_err}}
#define	CW_CREATE_ZERO_OBJECT_ERR(obj_name, obj_type, on_err)	{obj_name = (obj_type*) (CWMemZeroAlloc(sizeof(obj_type))); if(!(obj_name)) {on_err}}
#define	CW_CREATE_OBJECT_SIZE_ERR(obj_name, obj_size,on_err)	{obj_name = (CWMemAlloc(obj_size)); if(!(obj_name)) {on_err}}
#define	CW_CREATE_ZERO_OBJECT_SIZE_ERR(obj_name, obj_size,on_err)	{obj_name = (CWMemZeroAlloc(obj_size)); if(!(obj_name)) {on_err}}
#define	CW_CREATE_ARRAY_ERR(ar_name, ar_size, ar_type, on_err)	{ar_name = (ar_type*) (CWMemAlloc(sizeof(ar_type) * (ar_size))); if(!(ar_name)) {on_err}}
#define	CW_CREATE_ZERO_ARRAY_ERR(ar_name, ar_size, ar_type, on_err)	{ar_name = (ar_type*) (CWMemZeroAlloc((ar_size)*sizeof(ar_type))); if(!(ar_name)) {on_err}}
#define	CW_CREATE_STRING_ERR(str_name, str_length, on_err)	{str_name = (char*) (CWMemAlloc(sizeof(char) * ((str_length)+1) ) ); if(!(str_name)) {on_err}}
#define	CW_CREATE_STRING_FROM_STRING_ERR(str_name, str, on_err)	{CW_CREATE_STRING_ERR(str_name, strlen(str), on_err); if(str_name){strcpy((str_name), str);}}
#define	CW_CREATE_STRING_FROM_STRING_LEN_ERR(str_name, str, len, on_err) {str_name = (char*) (CWMemAlloc(sizeof(char) * ((len)+1) ) ); if(!(str_name)) {on_err} else{CW_COPY_MEMORY(str_name, str, len); str_name[len]='\0';}}

#if (!defined(BUILD_WUM))
#include "CWStevens.h"
#include "config.h"
#include "CWLog.h"
#include "CWErrorHandling.h"

#include "CWRandom.h"
//#include "CWTimer.h"
#include "timerlib.h"
#include "CWThread.h"
#include "CWNetwork.h"
#include "CWList.h"
#include "CWSafeList.h"

#include "CWProtocol.h"
#include "CWSecurity.h"
#include "CWConfigFile.h"
#include "sn_syslog_desc.h"
#endif

#define CW_SET_PORT_BIT(list,bit)   do{list=((list)|(1<<(bit)));}while(0)
#define CW_RESET_PORT_BIT(list,bit) do{list=((list)&(~(1<<(bit))));}while(0)
#define CW_CHECK_PORT_BIT(list,bit) ((list) & (1<<(bit)))

static const char b64_table[] = {
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
  'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
  'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
  'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
  'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
  'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
  'w', 'x', 'y', 'z', '0', '1', '2', '3',
  '4', '5', '6', '7', '8', '9', '+', '/'
};


#define b64_malloc(ptr) CWMemAlloc(ptr)
#define b64_realloc(ptr, size) realloc(ptr, size)



int CWTimevalSubtract(struct timeval *res, const struct timeval *x, const struct timeval *y);
CWBool CWParseSettingsFile(void);
CWBool CWErrorHandlingInitLib(void);
CWBool CWForkShellProcess();
CWBool CWCloseShellProcess();
int _CWSystem(CWBool dbg, const char *fmt, ...); /* This function must be used after calling CWForkShellProcess */
#define CWSystem(_fmt, ...) _CWSystem(CW_FALSE, _fmt, ##__VA_ARGS__)
#define CWSystemDbg(_fmt, ...) _CWSystem(CW_TRUE, _fmt, ##__VA_ARGS__)
int CWSystemUnsafe(const char *fmt, ...); /* This function should not be used in multi-threads environment */
/*****************************************************/
CWBool CWWtpCfgMsgListAdd(CWWtpCfgMsgList *list, int type, int keyLen, void *keyPtr, int valLen, void *valPtr);
CWBool CWWtpCfgMsgListRemove(CWWtpCfgMsgList *list, int type, int keyLen, void *keyPtr);
void CWWtpCfgMsgListFree(CWWtpCfgMsgList *list);
void CWWtpCfgFree(CWWtpCfg *cfg);
void CWWtpStationInfoFree(CWWTPStationInfo *staInfo);
void CWWtpCurrentCfgInfoFree(CWWTPCurrentCfgInfo *cfgInfo);
void CWWtpSwitchTopologyInfoFree(CWWTPSwitchTopologyInfo *info);
void CWWtpSwitchTrunkInfoFree(CWWTPSwitchTrunkInfo *trunkinfo);
void CWWtpSwitchPortInfoFree(CWWTPSwitchPortInfo*portInfo);


CWBool CWParseAcAddrString(const char *str, CWAcAddress *addr);
void CWSetAcAddr(unsigned int ip, unsigned short port, int controllerId, CWAcAddress *addr);
CWBool CWCreateAcAddrString(const CWAcAddress *addr, char **pstr);
CWBool CWParseMacString(const char *str, CWMacAddress mac);
char *CWMacToString(CWMacAddress mac, char *str);
CWBool CWCreateStringByFile(const char *file, char **pstr);
CWBool CWSaveStringToFile(const char *file, const char *pstr);

char *CWCreateString(const char *fmt, ...);
char *_CWCreateStringByCmd(int type, const char *fmt, ...);
#define CWCreateStringByCmd(_fmt, ...) _CWCreateStringByCmd(0, _fmt, ##__VA_ARGS__)
#define CWCreateStringByCmdStdout(_fmt, ...) _CWCreateStringByCmd(1, _fmt, ##__VA_ARGS__)
#define CWCreateStringByCmdStderr(_fmt, ...) _CWCreateStringByCmd(2, _fmt, ##__VA_ARGS__)
//#define CWCreateStringByCmdStdOut(_fmt, ...) _CWCreateStringByCmd(1, _fmt, ##__VA_ARGS__)
CWBool CWGetFileSize(char *file, int *size);
CWBool CWGetFileMD5(char *file, unsigned char chksum[]);
CWBool CWRespawn(void);
CWBool CWCheckFileEqual(char *file1, char *file2);
int CWGetUptime(void);
CWBool CWGetMemoryInfo(CWMemoryInfo *memInfo);
CWBool CWGetTopMemoryProcess(char **processName, unsigned int *memUsed);
CWBool CWGetRadioIndex(CWRadioFreqType radioType, unsigned int capCode, int *radioIndex);
CWBool CWGetRadioType(CWRadioFreqType *radioType, unsigned int capCode, int radioIndex);
CWBool CWGetRadioName(CWRadioFreqType radioType,int radioCount,char *radioName);

int CWParseUnimac(const char *str, CWMacAddress mac);
CWBool CWParseMacList(const char *str, CWMacAddress **macList, int *macNum);
CWBool CWParseAcList(const char *str, CWHostName **acList, int *acNum);
char *CWStrtokSingle(char *str, char const *delims);
void CWWtpMeshInfoFree(CWWTPMeshInfo *meshInfo);
int CWConvertRadioIdx(int radioIdx);

/**
 * Encode `unsigned char *' source with `size_t' size.
 * Returns a `char *' base64 encoded string.
 */

char *b64_encode (const unsigned char *, size_t);

/**
 * Dencode `char *' source with `size_t' size.
 * Returns a `unsigned char *' base64 decoded string.
 */
unsigned char *b64_decode (const char *, size_t);

/**
 * Dencode `char *' source with `size_t' size.
 * Returns a `unsigned char *' base64 decoded string + size of decoded string.
 */
unsigned char *b64_decode_ex (const char *, size_t, size_t *);

#endif

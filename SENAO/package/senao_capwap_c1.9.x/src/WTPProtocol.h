/*******************************************************************************************
 * Copyright (c) 2006-7 Laboratorio di Sistemi di Elaborazione e Bioingegneria Informatica *
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
 *******************************************************************************************/


#ifndef __CAPWAP_WTPProtocol_HEADER__
#define __CAPWAP_WTPProtocol_HEADER__

#include <setting/WTPBoardApiCommon.h>
#include <setting/WTPBoardApiDevice.h>
#include <setting/WTPBoardApiSystem.h>
#include <setting/WTPBoardApiNetwork.h>
#if SUPPORT_CONFIG_HAS_SECTIONNAME
#include <setting/WTPBoardApiWireless_wn.h>
#elif SUPPORT_WLAN_5G_2_SETTING
#include <setting/WTPBoardApiWireless_triband.h>
#else
#include <setting/WTPBoardApiWireless.h>
#endif
#include <setting/WTPBoardApiPortal.h>
#include <setting/WTPBoardApiNas.h>
#include <setting/WTPBoardApiMesh.h>

/*_____________________________________________________*/
/*  *******************___TYPES___*******************  */
typedef struct
{
    int ACIPv4ListCount;
    int *ACIPv4List;
} ACIPv4ListValues;

typedef struct
{
    int ACIPv6ListCount;
    struct in6_addr *ACIPv6List;
} ACIPv6ListValues;

typedef struct
{
    int stations;
    int limit;
    int activeWTPs;
    int maxWTPs;
    CWAuthSecurity security;
    int RMACField;
    //	int WirelessField;
    int DTLSPolicy;
    CWACVendorInfos vendorInfos;
    char *name;
    CWProtocolIPv4NetworkInterface *IPv4Addresses;
    int IPv4AddressesCount;
    CWProtocolIPv6NetworkInterface *IPv6Addresses;
    int IPv6AddressesCount;
    ACIPv4ListValues ACIPv4ListInfo;
    ACIPv6ListValues ACIPv6ListInfo;
    CWNetworkLev4Address preferredAddress;
    CWNetworkLev4Address incomingAddress;
    CWIPCfgInfo *ipCfgInfo; /* not NULL if received ip change request */
    CWTimer *timer;
    CWBool debugLogState;
    CWNetworkLev4Address *proxyAddress;
    int controllerId;
    unsigned int mtu;
    int wtpPacketInterval;
    int utc_time;
    CWWtpCfgCapInfo *cfgCapInfo;
} CWACInfoValues;

typedef struct
{
    CWACInfoValues ACInfoPtr;
    CWProtocolResultCode code;
    ACIPv4ListValues ACIPv4ListInfo;
    ACIPv6ListValues ACIPv6ListInfo;
    CWImageIdentifier imageId;
    int connId;
} CWProtocolJoinResponseValues;

typedef struct
{
    ACIPv4ListValues ACIPv4ListInfo;
    ACIPv6ListValues ACIPv6ListInfo;
    int discoveryTimer;
    int echoRequestTimer;
    int radioOperationalInfoCount;
    CWRadioOperationalInfoValues *radioOperationalInfo;
    WTPDecryptErrorReport radiosDecryptErrorPeriod;
    int idleTimeout;
    int fallback;
    CWIPCfgInfo *staticIPInfo;
    CWProtocolVendorSpecificValues vendorValuesWtpCfg;
    int statsUploadInterval;
    int statsPollInterval;
    int statsMaxClients;
    int memLogThreshold;
    CWBool clientStateChgEventEnable;
    CWBackgroundSitesurveyValues bgStSvy2gVal;
    CWBackgroundSitesurveyValues bgStSvy5gVal;
    CWBackgroundSitesurveyValues bgStSvy5gOneVal;
} CWProtocolConfigureResponseValues;

typedef struct
{
    int statsMaxClients;
    int debugLog;
    int utc_time;
} CWProtocolEchoResponseValues;

typedef struct
{
    CWProtocolVendorSpecificValues vendorValuesCfg;
    CWBool waitApply;
    CWImageIdentifier imageId;
} CWProtocolConfigurationUpdateRequestValues;

typedef enum
{
    CW_MATCH_CFG_TYPE = 0,
    CW_MATCH_CFG_RADIO_TYPE = 1,
    CW_MATCH_CFG_RADIO_WLAN_TYPE = 2,
    CW_MATCH_CFG_RADIO_WLAN_WEPKEY_TYPE = 3,
} CWChangedCfgMtachKeyType;

typedef enum
{
    CW_PORT_AUTO = 0,
    CW_PORT_1000_FULL,
    CW_PORT_100_FULL,
    CW_PORT_100_HALF,
    CW_PORT_10_FULL,
    CW_PORT_10_HALF,
    CW_PORT_DISABLED
} CWSwitchPortMode;

typedef struct
{
    int type;
    CWChangedCfgMtachKeyType keyType;
} CWChangedCfg;

/*__________________________________________________________*/
/*  *******************___PROTOTYPES___*******************  */
CWBool CWAssembleMsgElemACName(CWProtocolMessage *msgPtr);				// 4
CWBool CWAssembleMsgElemACNameWithPriority(CWProtocolMessage *msgPtr);			// 5
CWBool CWAssembleMsgElemDataTransferData(CWProtocolMessage *msgPtr, int data_type);	//13
CWBool CWAssembleMsgElemDiscoveryType(CWProtocolMessage *msgPtr, CWDiscoveryType type);			//20
//CWBool CWAssembleMsgElemSimpleDiscoveryType(CWProtocolMessage *msgPtr);                 //20 ac to ac
CWBool CWAssembleMsgElemDuplicateIPv4Address(CWProtocolMessage *msgPtr);		//21
CWBool CWAssembleMsgElemLocationData(CWProtocolMessage *msgPtr);			//27
CWBool CWAssembleMsgElemStatisticsTimer(CWProtocolMessage *msgPtr);			//33
CWBool CWAssembleMsgElemWTPBoardData(CWProtocolMessage *msgPtr);			//35
CWBool CWAssembleMsgElemWTPDescriptor(CWProtocolMessage *msgPtr);			//36
CWBool CWAssembleMsgElemWTPFrameTunnelMode(CWProtocolMessage *msgPtr);			//38
CWBool CWAssembleMsgElemWTPIPv4Address(CWProtocolMessage *msgPtr);			//39
CWBool CWAssembleMsgElemWTPMACType(CWProtocolMessage *msgPtr);				//40
CWBool CWAssembleMsgElemWTPName(CWProtocolMessage *msgPtr);				//41
//CWBool CWAssembleMsgElemWTPOperationalStatistics(CWProtocolMessage *msgPtr,int radio);	//42
CWBool CWAssembleMsgElemWTPRadioStatistics(CWProtocolMessage *msgPtr, int radio);	//43
CWBool CWAssembleMsgElemWTPRebootStatistics(CWProtocolMessage *msgPtr);			//44
//CWBool CWAssembleMsgElemWTPStaticIPInfo(CWProtocolMessage *msgPtr);			//45
/* AP */
CWBool CWAssembleMsgElemWTPRadioInformation(CWProtocolMessage *msgPtr);
CWBool CWWTPAssembleMsgElemSitesurveyInfo(CWProtocolMessage *msgPtr, CWWTPSitesurveyInfo *sitesurveyInfo);
CWBool CWWTPAssembleMsgElemAutoChannelChangeInfo(CWProtocolMessage *msgPtr, CWWTPAutoChannelInfo *chanChangedInfo);
CWBool CWWTPAssembleMsgElemAutoTxPowerCurrentValue(CWProtocolMessage *msgPtr, CWRadioFreqType radioType);
CWBool CWWTPAssembleMsgElemVendorPayloadCurrentStationInfo(CWProtocolMessage *msgPtr);
CWBool CWWTPAssembleMsgElemVendorPayloadWlanStatisticsInfo(CWProtocolMessage *msgPtr);
CWBool CWWTPAssembleMsgElemVendorPayloadSitesurveyInfo(CWProtocolMessage *msgPtr, CWWTPSitesurveyInfo *sitesurveyInfo);
CWBool CWWTPAssembleMsgElemVendorPayloadMeshInfo(CWProtocolMessage *msgPtr);
/* Switch */
CWBool CWWTPAssembleMsgElemVendorPayloadSwitchTopology(CWProtocolMessage *msgPtr);
CWBool CWWTPAssembleMsgElemVendorPayloadSwitchPoeInfo(CWProtocolMessage *msgPtr);
CWBool CWWTPAssembleMsgElemVendorPayloadSwitchPortInfo(CWProtocolMessage *msgPtr);
CWBool CWWTPAssembleMsgElemVendorPayloadSwitchTrunkInfo(CWProtocolMessage *msgPtr);
/* Common */
CWBool CWWTPAssembleMsgElemVendorPayloadWtpCfg(CWProtocolMessage *msgPtr);
CWBool CWWTPAssembleMsgElemVendorPayloadCurrentCfg(CWProtocolMessage *msgPtr);
CWBool CWWTPAssembleMsgElemLogMsg(CWProtocolMessage *msgPtr, int group, const char *category, int level, const char *msg);
//---------------------------------------------------------/
CWBool CWParseACDescriptor(CWProtocolMessage *msgPtr, int len, CWACInfoValues *valPtr);					// 1
CWBool CWParseACIPv4List(CWProtocolMessage *msgPtr, int len, ACIPv4ListValues *valPtr);					// 2
CWBool CWParseACIPv6List(CWProtocolMessage *msgPtr, int len, ACIPv6ListValues *valPtr);					// 3
CWBool CWParseAddStation(CWProtocolMessage *msgPtr, int len);								// 8
CWBool CWParseCWControlIPv4Addresses(CWProtocolMessage *msgPtr, int len, CWProtocolIPv4NetworkInterface *valPtr);	//10
CWBool CWParseCWControlIPv6Addresses(CWProtocolMessage *msgPtr, int len, CWProtocolIPv6NetworkInterface *valPtr);	//11
CWBool CWParseCWTimers(CWProtocolMessage *msgPtr, int len, CWProtocolConfigureResponseValues *valPtr);			//12
CWBool CWParseDecryptErrorReportPeriod(CWProtocolMessage *msgPtr, int len, WTPDecryptErrorReportValues *valPtr);	//16
CWBool CWParseIdleTimeout(CWProtocolMessage *msgPtr, int len, CWProtocolConfigureResponseValues *valPtr);		//26
CWBool CWParseWTPFallback(CWProtocolMessage *msgPtr, int len, CWProtocolConfigureResponseValues *valPtr);		//37
CWBool CWParseConnectionID(CWProtocolMessage *msgPtr, int len, int *id);
//si trova in CWProtocol.h
//CWBool CWParseACName(CWProtocolMessage *msgPtr, int len, char **valPtr);						// 4

//---------------------------------------------------------/
void CWWTPResetRebootStatistics(WTPRebootStatisticsInfo *rebootStatistics);
CWBool CWWTPInitWtpInfo();
int CWWTPGetDiscoveryType(void);
int CWWTPGetMaxRadios(void);
int CWWTPGetRadiosInUse(void);
int CWWTPGetEncCapabilities(void);
CWBool CWWTPGetBoardData(CWWTPVendorInfos *valPtr);
CWBool CWWTPGetWTPDescriptor(CWWTPVendorInfos *valPtr);
int CWWTPGetMACType(void);
int CWWTPGetSessionID(void);
int CWWTPGetIPv4Address(void);
int CWWTPGetIPv4StatusDuplicate(void);
int CWWTPGetIPv6StatusDuplicate(void);
CWBool CWWTPGetName(char **pstr);
CWBool CWWTPGetLocation(char **pstr);
CWBool CWWTPGetRadiosInfomation(CWRadiosInformation *valPtr);
int CWWTPGetACIndex();
char *CWWTPGetACName();
int CWWTPGetFrameTunnelMode();
CWBool CWGetWTPRadiosOperationalState(int radioID, CWRadiosOperationalInfo *valPtr);
CWBool CWAssembleMsgElemDecryptErrorReport(CWProtocolMessage *msgPtr, int radioID);
CWBool CWAssembleMsgElemDuplicateIPv6Address(CWProtocolMessage *msgPtr);

CWBool CWWTPGetACNameWithPriority(CWACNamesWithPriority *valPtr);
void CWWTPDestroyACNameWithPriority(CWACNamesWithPriority *valPtr);
//---------------------------------------------------------/
void CWWTPDestroyVendorInfos(CWWTPVendorInfos *valPtr);

//----------------------Board ---------------------------/
#if 0
CWBool CWWTPBoardInitConfiguration();

int CWWTPBoardGetMaxRadio();

int CWWTPBoardGetMaxRadioWlans(int radioIdx);

int CWWTPBoardGetMaxRadioWlansWepKeys(int radioIdx, int wlanIdx);

int CWWTPBoardGetMaxLanPortNum();

int CWWTPBoardGetEncryptionCapabilities();

int CWWTPBoardGetRebootTime(CWBool factory);

int CWWTPBoardGetImageBurningTime(char *imagePath);

int CWWTPBoardGetImageRebootTime(char *imagePath);

int CWWTPBoardGetApplyCfgTime(CWBool *rejoin);

int CWWTPBoardGetSystemUpTime();

void CWWTPBoardGetWtpCfgCap(CWWtpCfgCap cap);

void CWWTPBoardGetRadioCfgCap(int radioIdx, CWWtpCfgCap cap);

void CWWTPBoardGetWlanCfgCap(int radioIdx, int wlanIdx, CWWtpCfgCap cap);

CWBool CWWTPBoardGetBaseMac(CWMacAddress mac);

CWBool CWWTPBoardGetRadioMac(int radioIdx, CWMacAddress mac);

CWBool CWWTPBoardGetWlanBssid(int radioIdx, int wlanIdx, CWMacAddress mac);

CWBool CWWTPBoardGetModelName(char **pstr);

CWBool CWWTPBoardGetSku(char **pstr);

CWBool CWWTPBoardGetCapCode(unsigned int *code);

CWBool CWWTPBoardGetRadioMaxTxPower(int radioIdx, int *power);

CWBool CWWTPBoardGetSerialNum(char **pstr);

CWBool CWWTPBoardGetHardwareVersion(char **pstr);

CWBool CWWTPBoardGetSoftwareVersion(char **pstr);

CWBool CWWTPBoardGetBootVersion(char **pstr);

CWBool CWWTPBoardGetNameCfg(char **pstr);
CWBool CWWTPBoardSetNameCfg(char *pstr);

CWBool CWWTPBoardGetLocationCfg(char **pstr);
CWBool CWWTPBoardSetLocationCfg(char *pstr);

CWBool CWWTPBoardGetIPv4Cfg(CWIPv4Cfg *cfg);
CWBool CWWTPBoardSetIPv4Cfg(CWIPv4Cfg *cfg);

CWBool CWWTPBoardGetRadioOperationModeCfg(int radioIdx, int *mode);
CWBool CWWTPBoardSetRadioOperationModeCfg(int radioIdx, int mode);

CWBool CWWTPBoardGetRadioCountryCodeCfg(int radioIdx, int *country);
CWBool CWWTPBoardSetRadioCountryCodeCfg(int radioIdx, int country);

CWBool CWWTPBoardGetRadioChannelHTModeCfg(int radioIdx, int *mode);
CWBool CWWTPBoardSetRadioChannelHTModeCfg(int radioIdx, int mode);

CWBool CWWTPBoardGetRadioChannelExtCfg(int radioIdx, int *mode);
CWBool CWWTPBoardSetRadioChannelExtCfg(int radioIdx, int mode);

CWBool CWWTPBoardGetRadioChannelCfg(int radioIdx, int *mode);
CWBool CWWTPBoardSetRadioChannelCfg(int radioIdx, int mode);

CWBool CWWTPBoardGetRadioWirelessModeCfg(int radioIdx, int *mode);
CWBool CWWTPBoardSetRadioWirelessModeCfg(int radioIdx, int mode);

CWBool CWWTPBoardGetRadioLimitedClientsEnableCfg(int radioIdx, int *enable);
CWBool CWWTPBoardSetRadioLimitedClientsEnableCfg(int radioIdx, int enable);

CWBool CWWTPBoardGetRadioLimitedClientsCfg(int radioIdx, int *clients);
CWBool CWWTPBoardSetRadioLimitedClientsCfg(int radioIdx, int clients);

CWBool CWWTPBoardGetRadioTxPowerCfg(int radioIdx, int *power);
CWBool CWWTPBoardSetRadioTxPowerCfg(int radioIdx, int power);

CWBool CWWTPBoardGetRadioAggregationEnableCfg(int radioIdx, int *enable);
CWBool CWWTPBoardSetRadioAggregationEnableCfg(int radioIdx, int enable);

CWBool CWWTPBoardGetRadioAggregationFramesCfg(int radioIdx, int *frames);
CWBool CWWTPBoardSetRadioAggregationFramesCfg(int radioIdx, int frames);

CWBool CWWTPBoardGetRadioAggregationMaxBytesCfg(int radioIdx, int *maxBytes);
CWBool CWWTPBoardSetRadioAggregationMaxBytesCfg(int radioIdx, int maxBytes);

CWBool CWWTPBoardGetPortalEnableCfg(int radioIdx, int *enable);
CWBool CWWTPBoardSetPortalEnableCfg(int radioIdx, int enable);

CWBool CWWTPBoardGetPortalLoginTypeCfg(int radioIdx, int *type);
CWBool CWWTPBoardSetPortalLoginTypeCfg(int radioIdx, int type);

CWBool CWWTPBoardGetPortalRadiusCfg(int radioIdx, unsigned int *addr);
CWBool CWWTPBoardSetPortalRadiusCfg(int radioIdx, unsigned int addr);

CWBool CWWTPBoardGetPortalRedirectCfg(int radioIdx, char * * pstr);
CWBool CWWTPBoardSetPortalRedirectCfg(int radioIdx, char *pstr);

CWBool CWWTPBoardGetPortalWalledGardenCfg(int radioIdx, int *enable);
CWBool CWWTPBoardSetPortalWalledGardenCfg(int radioIdx, int enable);

CWBool CWWTPBoardGetPortalWalledGardenPageCfg(int radioIdx, char **pstr);
CWBool CWWTPBoardSetPortalWalledGardenPageCfg(int radioIdx, char *pstr);

CWBool CWWTPBoardGetPortalRadiusPortCfg(int radioIdx, unsigned short *port);
CWBool CWWTPBoardSetPortalRadiusPortCfg(int radioIdx, unsigned short port);

CWBool CWWTPBoardGetPortalRadiusSecretCfg(int radioIdx, char **pstr);
CWBool CWWTPBoardSetPortalRadiusSecretCfg(int radioIdx, char *pstr);

CWBool CWWTPBoardSetPortalSessionTimeoutCfg(int radioIdx, int time);
CWBool CWWTPBoardGetPortalSessionTimeoutCfg(int radioIdx, int *time);

CWBool CWWTPBoardGetPortalSessionTimeoutEnableCfg(int radioIdx, int *enable);
CWBool CWWTPBoardSetPortalSessionTimeoutEnableCfg(int radioIdx, int enable);

CWBool CWWTPBoardSetPortalIdleTimeoutCfg(int radioIdx, int time);
CWBool CWWTPBoardGetPortalIdleTimeoutCfg(int radioIdx, int *time);

CWBool CWWTPBoardGetRadioFastHandoverStatusCfg(int radioIdx, int *enable);
CWBool CWWTPBoardSetRadioFastHandoverStatusCfg(int radioIdx, int enable);

CWBool CWWTPBoardGetRadioFastHandoverRssiCfg(int radioIdx, int *rssi);
CWBool CWWTPBoardSetRadioFastHandoverRssiCfg(int radioIdx, int rssi);

CWBool CWWTPBoardGetWlanEnableCfg(int radioIdx, int wlanIdx, int *enable);
CWBool CWWTPBoardSetWlanEnableCfg(int radioIdx, int wlanIdx, int enable);

CWBool CWWTPBoardGetPortalIdleTimeoutEnableCfg(int radioIdx, int *enable);
CWBool CWWTPBoardSetPortalIdleTimeoutEnableCfg(int radioIdx, int enable);

CWBool CWWTPBoardGetPortalAccountingEnableCfg(int radioIdx, int *enable);
CWBool CWWTPBoardSetPortalAccountingEnableCfg(int radioIdx, int enable);

CWBool CWWTPBoardSetPortalAccountingIntervalCfg(int radioIdx, int time);
CWBool CWWTPBoardGetPortalAccountingIntervalCfg(int radioIdx, int *time);

CWBool CWWTPBoardGetPortalAuthTypeCfg(int radioIdx, int *type);
CWBool CWWTPBoardSetPortalAuthTypeCfg(int radioIdx, int type);

CWBool CWWTPBoardGetPortalExternalServerCfg(int radioIdx, char **pstr);
CWBool CWWTPBoardSetPortalExternalServerCfg(int radioIdx, char *pstr);

CWBool CWWTPBoardGetPortalExternalSecretCfg(int radioIdx, char **pstr);
CWBool CWWTPBoardSetPortalExternalSecretCfg(int radioIdx, char *pstr);

CWBool CWWTPBoardGetPortalAccountingServerCfg(int radioIdx, unsigned int *addr);
CWBool CWWTPBoardSetPortalAccountingServerCfg(int radioIdx, unsigned int addr);

CWBool CWWTPBoardGetPortalAccountingPortCfg(int radioIdx, unsigned short *port);
CWBool CWWTPBoardSetPortalAccountingPortCfg(int radioIdx, unsigned short port);

CWBool CWWTPBoardGetPortalAccountingSecretCfg(int radioIdx, char **pstr);
CWBool CWWTPBoardSetPortalAccountingSecretCfg(int radioIdx, char *pstr);

CWBool CWWTPBoardGetPortalUamformatCfg(int radioIdx, char **pstr);
CWBool CWWTPBoardSetPortalUamformatCfg(int radioIdx, char *pstr);

CWBool CWWTPBoardGetPortalLocalAuthCfg(int radioIdx, char **pstr);
CWBool CWWTPBoardSetPortalLocalAuthCfg(int radioIdx, char *pstr);

CWBool CWWTPBoardGetPortalPortCfg(int radioIdx, unsigned short *port);
CWBool CWWTPBoardSetPortalPortCfg(int radioIdx, unsigned short port);

CWBool CWWTPBoardGetPortalHttpsEnableCfg(int radioIdx, int *enable);
CWBool CWWTPBoardSetPortalHttpsEnableCfg(int radioIdx, int enable);

CWBool CWWTPBoardGetPortalRadiusSecret2Cfg(int radioIdx, char **pstr);
CWBool CWWTPBoardSetPortalRadiusSecret2Cfg(int radioIdx, char *pstr);

CWBool CWWTPBoardGetPortalRadius2Cfg(int radioIdx, unsigned int *addr);
CWBool CWWTPBoardSetPortalRadius2Cfg(int radioIdx, unsigned int addr);


CWBool CWWTPBoardGetWlanPortalEnableCfg(int radioIdx, int wlanIdx , int *enable);
CWBool CWWTPBoardSetWlanPortalEnableCfg(int radioIdx, int wlanIdx , int enable);

CWBool CWWTPBoardGetWlanPortalLoginTypeCfg(int radioIdx, int wlanIdx , int *type);
CWBool CWWTPBoardSetWlanPortalLoginTypeCfg(int radioIdx, int wlanIdx , int type);

CWBool CWWTPBoardGetWlanPortalRadiusCfg(int radioIdx, int wlanIdx , unsigned int *addr);
CWBool CWWTPBoardSetWlanPortalRadiusCfg(int radioIdx, int wlanIdx , unsigned int addr);

CWBool CWWTPBoardGetWlanPortalRedirectCfg(int radioIdx, int wlanIdx , char * * pstr);
CWBool CWWTPBoardSetWlanPortalRedirectCfg(int radioIdx, int wlanIdx , char *pstr);

CWBool CWWTPBoardGetWlanPortalWalledGardenCfg(int radioIdx, int wlanIdx , int *enable);
CWBool CWWTPBoardSetWlanPortalWalledGardenCfg(int radioIdx, int wlanIdx , int enable);

CWBool CWWTPBoardGetWlanPortalWalledGardenPageCfg(int radioIdx, int wlanIdx , char **pstr);
CWBool CWWTPBoardSetWlanPortalWalledGardenPageCfg(int radioIdx, int wlanIdx , char *pstr);

CWBool CWWTPBoardGetWlanPortalRadiusPortCfg(int radioIdx, int wlanIdx , unsigned short *port);
CWBool CWWTPBoardSetWlanPortalRadiusPortCfg(int radioIdx, int wlanIdx , unsigned short port);

CWBool CWWTPBoardGetWlanPortalRadiusSecretCfg(int radioIdx, int wlanIdx , char **pstr);
CWBool CWWTPBoardSetWlanPortalRadiusSecretCfg(int radioIdx, int wlanIdx , char *pstr);

CWBool CWWTPBoardSetWlanPortalSessionTimeoutCfg(int radioIdx, int wlanIdx , int time);
CWBool CWWTPBoardGetWlanPortalSessionTimeoutCfg(int radioIdx, int wlanIdx , int *time);

CWBool CWWTPBoardGetWlanPortalSessionTimeoutEnableCfg(int radioIdx, int wlanIdx , int *enable);
CWBool CWWTPBoardSetWlanPortalSessionTimeoutEnableCfg(int radioIdx, int wlanIdx , int enable);

CWBool CWWTPBoardSetWlanPortalIdleTimeoutCfg(int radioIdx, int wlanIdx , int time);
CWBool CWWTPBoardGetWlanPortalIdleTimeoutCfg(int radioIdx, int wlanIdx , int *time);


CWBool CWWTPBoardGetWlanPortalIdleTimeoutEnableCfg(int radioIdx, int wlanIdx , int *enable);
CWBool CWWTPBoardSetWlanPortalIdleTimeoutEnableCfg(int radioIdx, int wlanIdx , int enable);

CWBool CWWTPBoardGetWlanPortalAccountingEnableCfg(int radioIdx, int wlanIdx , int *enable);
CWBool CWWTPBoardSetWlanPortalAccountingEnableCfg(int radioIdx, int wlanIdx , int enable);

CWBool CWWTPBoardSetWlanPortalAccountingIntervalCfg(int radioIdx, int wlanIdx , int time);
CWBool CWWTPBoardGetWlanPortalAccountingIntervalCfg(int radioIdx, int wlanIdx , int *time);

CWBool CWWTPBoardGetWlanPortalAuthTypeCfg(int radioIdx, int wlanIdx , int *type);
CWBool CWWTPBoardSetWlanPortalAuthTypeCfg(int radioIdx, int wlanIdx , int type);

CWBool CWWTPBoardGetWlanPortalExternalServerCfg(int radioIdx, int wlanIdx , char **pstr);
CWBool CWWTPBoardSetWlanPortalExternalServerCfg(int radioIdx, int wlanIdx , char *pstr);

CWBool CWWTPBoardGetWlanPortalExternalSecretCfg(int radioIdx, int wlanIdx , char **pstr);
CWBool CWWTPBoardSetWlanPortalExternalSecretCfg(int radioIdx, int wlanIdx , char *pstr);

CWBool CWWTPBoardGetWlanPortalAccountingServerCfg(int radioIdx, int wlanIdx , unsigned int *addr);
CWBool CWWTPBoardSetWlanPortalAccountingServerCfg(int radioIdx, int wlanIdx , unsigned int addr);

CWBool CWWTPBoardGetWlanPortalAccountingPortCfg(int radioIdx, int wlanIdx , unsigned short *port);
CWBool CWWTPBoardSetWlanPortalAccountingPortCfg(int radioIdx, int wlanIdx , unsigned short port);

CWBool CWWTPBoardGetWlanPortalAccountingSecretCfg(int radioIdx, int wlanIdx , char **pstr);
CWBool CWWTPBoardSetWlanPortalAccountingSecretCfg(int radioIdx, int wlanIdx , char *pstr);

CWBool CWWTPBoardGetWlanPortalUamformatCfg(int radioIdx, int wlanIdx , char **pstr);
CWBool CWWTPBoardSetWlanPortalUamformatCfg(int radioIdx, int wlanIdx , char *pstr);

CWBool CWWTPBoardGetWlanPortalLocalAuthCfg(int radioIdx, int wlanIdx , char **pstr);
CWBool CWWTPBoardSetWlanPortalLocalAuthCfg(int radioIdx, int wlanIdx , char *pstr);

CWBool CWWTPBoardGetWlanPortalPortCfg(int radioIdx, int wlanIdx , unsigned short *port);
CWBool CWWTPBoardSetWlanPortalPortCfg(int radioIdx, int wlanIdx , unsigned short port);

CWBool CWWTPBoardGetWlanPortalHttpsEnableCfg(int radioIdx, int wlanIdx , int *enable);
CWBool CWWTPBoardSetWlanPortalHttpsEnableCfg(int radioIdx, int wlanIdx , int enable);

CWBool CWWTPBoardGetWlanPortalRadiusSecret2Cfg(int radioIdx, int wlanIdx , char **pstr);
CWBool CWWTPBoardSetWlanPortalRadiusSecret2Cfg(int radioIdx, int wlanIdx , char *pstr);

CWBool CWWTPBoardGetWlanPortalRadius2Cfg(int radioIdx, int wlanIdx , unsigned int *addr);
CWBool CWWTPBoardSetWlanPortalRadius2Cfg(int radioIdx, int wlanIdx , unsigned int addr);

CWBool CWWTPBoardSetWlanPortalNetworkTypeCfg(int radioIdx, int wlanIdx , int type);
CWBool CWWTPBoardGetWlanPortalNetworkTypeCfg(int radioIdx, int wlanIdx , int *type);


CWBool CWWTPBoardGetWlanSsidCfg(int radioIdx, int wlanIdx, char **pstr);
CWBool CWWTPBoardSetWlanSsidCfg(int radioIdx, int wlanIdx, char *pstr);

CWBool CWWTPBoardGetWlanSuppressedSsidCfg(int radioIdx, int wlanIdx, int *enable);
CWBool CWWTPBoardSetWlanSuppressedSsidCfg(int radioIdx, int wlanIdx, int enable);

CWBool CWWTPBoardGetWlanStationSeparationCfg(int radioIdx, int wlanIdx, int *enable);
CWBool CWWTPBoardSetWlanStationSeparationCfg(int radioIdx, int wlanIdx, int enable);

CWBool CWWTPBoardGetWlanLayer2IsolationCfg(int radioIdx, int wlanIdx, int *enable);
CWBool CWWTPBoardSetWlanLayer2IsolationCfg(int radioIdx, int wlanIdx, int enable);

CWBool CWWTPBoardGetWlanIsolationCfg(int radioIdx, int wlanIdx, int *enable);
CWBool CWWTPBoardSetWlanIsolationCfg(int radioIdx, int wlanIdx, int enable);

CWBool CWWTPBoardGetWlanVlanCfg(int radioIdx, int wlanIdx, int *vlan);
CWBool CWWTPBoardSetWlanVlanCfg(int radioIdx, int wlanIdx, int vlan);

CWBool CWWTPBoardGetWlanSecurityCfg(int radioIdx, int wlanIdx, int *mode);
CWBool CWWTPBoardSetWlanSecurityCfg(int radioIdx, int wlanIdx, int mode);

CWBool CWWTPBoardGetWepAuthTypeCfg(int radioIdx, int wlanIdx, int *type);
CWBool CWWTPBoardSetWepAuthTypeCfg(int radioIdx, int wlanIdx, int type);

CWBool CWWTPBoardGetWepInputMethodCfg(int radioIdx, int wlanIdx, int *method);
CWBool CWWTPBoardSetWepInputMethodCfg(int radioIdx, int wlanIdx, int method);

CWBool CWWTPBoardGetWepKeyLengthCfg(int radioIdx, int wlanIdx, int *len);
CWBool CWWTPBoardSetWepKeyLengthCfg(int radioIdx, int wlanIdx, int len);

CWBool CWWTPBoardGetWepDefaultKeyIdCfg(int radioIdx, int wlanIdx, int *keyId);
CWBool CWWTPBoardSetWepDefaultKeyIdCfg(int radioIdx, int wlanIdx, int keyId);

CWBool CWWTPBoardGetWepKeyCfg(int radioIdx, int wlanIdx, int keyIdx, char **pstr);
CWBool CWWTPBoardSetWepKeyCfg(int radioIdx, int wlanIdx, int keyIdx, char *pstr);

CWBool CWWTPBoardGetWpaEncryptionCfg(int radioIdx, int wlanIdx, int *mode);
CWBool CWWTPBoardSetWpaEncryptionCfg(int radioIdx, int wlanIdx, int mode);

CWBool CWWTPBoardGetWlanWpaPMFEnableCfg(int radioIdx, int wlanIdx, int *enable);
CWBool CWWTPBoardSetWlanWpaPMFEnableCfg(int radioIdx, int wlanIdx, int enable);

CWBool CWWTPBoardGetWpaPassphraseCfg(int radioIdx, int wlanIdx, char **pstr);
CWBool CWWTPBoardSetWpaPassphraseCfg(int radioIdx, int wlanIdx, char *pstr);

CWBool CWWTPBoardGetWpaGroupKeyUpdateIntervalCfg(int radioIdx, int wlanIdx, int *interval);
CWBool CWWTPBoardSetWpaGroupKeyUpdateIntervalCfg(int radioIdx, int wlanIdx, int interval);

CWBool CWWTPBoardGetSuiteBEnableCfg(int radioIdx, int wlanIdx, int *enable);
CWBool CWWTPBoardSetSuiteBEnableCfg(int radioIdx, int wlanIdx, int enable);

CWBool CWWTPBoardGetWpaRadiusAddressCfg(int radioIdx, int wlanIdx, unsigned int *addr);
CWBool CWWTPBoardSetWpaRadiusAddressCfg(int radioIdx, int wlanIdx, unsigned int addr);

CWBool CWWTPBoardGetWpaRadiusPortCfg(int radioIdx, int wlanIdx, unsigned short *port);
CWBool CWWTPBoardSetWpaRadiusPortCfg(int radioIdx, int wlanIdx, unsigned short port);

CWBool CWWTPBoardGetWpaRadiusSecretCfg(int radioIdx, int wlanIdx, char **pstr);
CWBool CWWTPBoardSetWpaRadiusSecretCfg(int radioIdx, int wlanIdx, char *pstr);

CWBool CWWTPBoardGetRadiusAccountingEnableCfg(int radioIdx, int wlanIdx, int *enable);
CWBool CWWTPBoardSetRadiusAccountingEnableCfg(int radioIdx, int wlanIdx, int enable);

CWBool CWWTPBoardGetRadiusAccountingAddressCfg(int radioIdx, int wlanIdx, unsigned int *addr);
CWBool CWWTPBoardSetRadiusAccountingAddressCfg(int radioIdx, int wlanIdx, unsigned int addr);

CWBool CWWTPBoardGetRadiusAccountingPortCfg(int radioIdx, int wlanIdx, unsigned short *port);
CWBool CWWTPBoardSetRadiusAccountingPortCfg(int radioIdx, int wlanIdx, unsigned short port);

CWBool CWWTPBoardGetRadiusAccountingSecretCfg(int radioIdx, int wlanIdx, char **pstr);
CWBool CWWTPBoardSetRadiusAccountingSecretCfg(int radioIdx, int wlanIdx, char *pstr);

CWBool CWWTPBoardApplyCfg();
CWBool CWWTPBoardCancelCfg();

CWBool CWWTPBoardGetRadioType(int radioIdx, CWRadioType *type);

CWBool CWWTPBoardGetRadioOperationalState(int radioIdx, CWRadioState *state, CWOperationalCause *cause);

CWBool CWWTPBoardGetRadioDecryptErrorReport(int radioIdx, CWMacAddress **errorMacList, unsigned char *numEntries);

CWBool CWWTPBoardGetDefaultInterfaceName(char **pstr);

CWBool CWWTPBoardBurnImage(char *imagePath);

CWBool CWWTPBoardReboot();

CWBool CWWTPBoardGetWlanStations(int radioIdx, int wlanIdx, int *count, CWStation **station);

CWBool CWWTPBoardGetWlanStatistics(int radioIdx, int wlanIdx, CWWlanStatistics *statistics);

CWBool CWWTPBoardGetWlanAclModeCfg(int radioIdx, int wlanIdx, int *mode);
CWBool CWWTPBoardSetWlanAclModeCfg(int radioIdx, int wlanIdx, int mode);

CWBool CWWTPBoardGetWlanAclMacListCfg(int radioIdx, int wlanIdx, int *count, CWMacAddress **macs);
CWBool CWWTPBoardSetWlanAclMacListCfg(int radioIdx, int wlanIdx, int count, CWMacAddress *macs);

CWBool CWWTPBoardGetDns1Cfg(unsigned int *addr);
CWBool CWWTPBoardSetDns1Cfg(unsigned int addr);

CWBool CWWTPBoardGetDns2Cfg(unsigned int *addr);
CWBool CWWTPBoardSetDns2Cfg(unsigned int addr);

CWBool CWWTPBoardGetAdminCfg(char **pstr);
CWBool CWWTPBoardSetAdminCfg(char *pstr);

CWBool CWWTPBoardGetPasswordMD5Cfg(unsigned char **md5);
CWBool CWWTPBoardSetPasswordMD5Cfg(unsigned char *md5);

CWBool CWWTPBoardGetPasswordCfg(char **pstr);
CWBool CWWTPBoardSetPasswordCfg(char *pstr);

CWBool CWWTPBoardGetRadioDataRateCfg(int radioIdx, int *rate);
CWBool CWWTPBoardSetRadioDataRateCfg(int radioIdx, int rate);

CWBool CWWTPBoardGetRadioBitRateCfg(int radioIdx, int *rate);
CWBool CWWTPBoardSetRadioBitRateCfg(int radioIdx, int rate);

CWBool CWWTPBoardGetRadioWirelessModeAXEnableCfg(int radioIdx, int *enable);
CWBool CWWTPBoardSetRadioWirelessModeAXEnableCfg(int radioIdx, int enable);

CWBool CWWTPBoardGetRadioRtsCtsThresholdCfg(int radioIdx, int *threshold);
CWBool CWWTPBoardSetRadioRtsCtsThresholdCfg(int radioIdx, int threshold);

CWBool CWWTPBoardGetRadiusAccountingIntermiIntervalCfg(int radioIdx, int wlanIdx, int *interval);
CWBool CWWTPBoardSetRadiusAccountingIntermiIntervalCfg(int radioIdx, int wlanIdx, int interval);

CWBool CWWTPBoardGetRadioObeyRegulatoryPowerCfg(int radioIdx, int *enable);
CWBool CWWTPBoardSetRadioObeyRegulatoryPowerCfg(int radioIdx, int enable);

CWBool CWWTPBoardSetAcMode(CWBool enable);

CWBool CWWTPBoardFactoryReset();

CWBool CWWTPBoardGetAcAddress(CWAcAddress *acAddr);
CWBool CWWTPBoardSetAcAddress(const CWAcAddress *acAddr);

CWBool CWWTPBoardGetAcAddressWithDhcpOption(char **acAddr);

CWBool CWWTPBoardGetGuestNetworkAddressCfg(unsigned int *addr);
CWBool CWWTPBoardSetGuestNetworkAddressCfg(unsigned int addr);

CWBool CWWTPBoardGetGuestNetworkMaskCfg(unsigned int *addr);
CWBool CWWTPBoardSetGuestNetworkMaskCfg(unsigned int addr);

CWBool CWWTPBoardGetGuestNetworkDhcpStartCfg(unsigned int *addr);
CWBool CWWTPBoardSetGuestNetworkDhcpStartCfg(unsigned int addr);

CWBool CWWTPBoardGetGuestNetworkDhcpEndCfg(unsigned int *addr);
CWBool CWWTPBoardSetGuestNetworkDhcpEndCfg(unsigned int addr);

CWBool CWWTPBoardGetGuestNetworkWinsServerCfg(unsigned int *addr);
CWBool CWWTPBoardSetGuestNetworkWinsServerCfg(unsigned int addr);

CWBool CWWTPBoardSetBandSteeringCfg(int enable);
CWBool CWWTPBoardGetBandSteeringCfg(int *enable);

CWBool CWWTPBoardSetFastHandoverStatusCfg(int enable);
CWBool CWWTPBoardGetFastHandoverStatusCfg(int *enable);

CWBool CWWTPBoardSetFastHandoverRssiCfg(int rssi);
CWBool CWWTPBoardGetFastHandoverRssiCfg(int *rssi);

CWBool CWWTPBoardSetRoamingEnableCfg(int radioIdx, int wlanIdx, int enable);
CWBool CWWTPBoardGetRoamingEnableCfg(int radioIdx, int wlanIdx, int *enable);

CWBool CWWTPBoardSetRoamingAdvSearchCfg(int radioIdx, int wlanIdx, int enable);
CWBool CWWTPBoardGetRoamingAdvSearchCfg(int radioIdx, int wlanIdx, int *enable);

CWBool CWWTPBoardSetDownloadLimitCfg(int radioIdx, int wlanIdx, int limit);
CWBool CWWTPBoardGetDownloadLimitCfg(int radioIdx, int wlanIdx, int *limit);

CWBool CWWTPBoardSetUploadLimitCfg(int radioIdx, int wlanIdx, int limit);
CWBool CWWTPBoardGetUploadLimitCfg(int radioIdx, int wlanIdx, int *limit);

CWBool CWWTPBoardGetDownloadPerUserEnableCfg(int radioIdx, int wlanIdx, int *enable);
CWBool CWWTPBoardSetDownloadPerUserEnableCfg(int radioIdx, int wlanIdx, int enable);

CWBool CWWTPBoardGetUploadPerUserEnableCfg(int radioIdx, int wlanIdx, int *enable);
CWBool CWWTPBoardSetUploadPerUserEnableCfg(int radioIdx, int wlanIdx, int enable);

CWBool CWWTPBoardGetRadioDistance(int radioIdx, int *distance);
CWBool CWWTPBoardSetRadioDistance(int radioIdx, int distance);

CWBool CWWTPBoardGetCurrentIPv4(CWIPv4Cfg *cfg);
CWBool CWWTPBoardGetCurrentDns(unsigned int *dns1, unsigned int *dns2);
CWBool CWWTPBoardGetRadioCurrentChannel(int radioIdx, int *channel);
CWBool CWWTPBoardGetRadioCurrentTxPower(int radioIdx, int *power);
CWBool CWWTPBoardGetRadioCurrentAvailableChannelList(int i32RadioIdx, unsigned char *pAvalaibleChanCount, unsigned char **pAvailableChanList);

CWBool CWWTPBoardGetSitesurvey(CWRadioFreqType radioType, unsigned char *version, int *count, CWWTPSitesurvey **sitesurvey);
CWBool CWWTPBoardGetNextFastScanChannel(int radioIdx, int *channel);
CWBool CWWTPBoardSetFastScanDurationTime(int radioIdx, unsigned int duration);
CWBool CWWTPBoardSetScanDwellTime(int radioIdx, unsigned int min, unsigned int max);
CWBool CWWTPBoardGetChannelUtilization(CWRadioFreqType radioType, int channel, unsigned char *chanUtil);
CWBool CWWTPBoardGetScan(CWBool bDisplay, CWWTPSitesurveyInfo *sitesurveyInfo);


CWBool CWWTPBoardSetRadioAutoTxPowerStrength(CWRadioFreqType radioType, int strength);

CWBool CWWTPBoardKickmac(CWWTPKickmacInfo *kicks);

CWBool CWWTPBoardGetLedPowerCfg(int *enable);
CWBool CWWTPBoardSetLedPowerCfg(int enable);

CWBool CWWTPBoardGetLedLanCfg(int *enable);
CWBool CWWTPBoardSetLedLanCfg(int enable);

CWBool CWWTPBoardGetLedWlan0Cfg(int *enable);
CWBool CWWTPBoardSetLedWlan0Cfg(int enable);

CWBool CWWTPBoardGetLedWlan1Cfg(int *enable);
CWBool CWWTPBoardSetLedWlan1Cfg(int enable);

CWBool CWWTPBoardGetLedWlan2Cfg(int *enable);
CWBool CWWTPBoardSetLedWlan2Cfg(int enable);


CWBool CWWTPBoardGetLedMeshCfg(int *enable);
CWBool CWWTPBoardSetLedMeshCfg(int enable);

CWBool CWWTPBoardGetManageVlanCfg(int *vlan);
CWBool CWWTPBoardSetManageVlanCfg(int vlan);

CWBool CWWTPBoardGetLanPortEnableCfg(int port, int *enable);
CWBool CWWTPBoardSetLanPortEnableCfg(int port, int enable);

CWBool CWWTPBoardGetLanPortVlanIdCfg(int port, int *vid);
CWBool CWWTPBoardSetLanPortVlanIdCfg(int port, int vid);

CWBool CWWTPBoardGetLanPortVlanModeCfg(int port, int *mode);
CWBool CWWTPBoardSetLanPortVlanModeCfg(int port, int mode);

CWBool CWWTPBoardGetBandSteeringMode(int *mode);
CWBool CWWTPBoardSetBandSteeringMode(int mode);

CWBool CWWTPBoardGetBandSteeringPercentEnable(int *enable);
CWBool CWWTPBoardSetBandSteeringPercentEnable(int enable);

CWBool CWWTPBoardGetBandSteeringRssiEnable(int *enable);
CWBool CWWTPBoardSetBandSteeringRssiEnable(int enable);

CWBool CWWTPBoardGetBandSteeringRssi(int *rssi);
CWBool CWWTPBoardSetBandSteeringRssi(int rssi);

CWBool CWWTPBoardGetBandSteeringPercent(int *present);
CWBool CWWTPBoardSetBandSteeringPercent(int present);

/*bandsteer wlan*/
CWBool CWWTPBoardGetWlanBandSteeringMode(int radioIdx, int wlanIdx,int *mode);
CWBool CWWTPBoardSetWlanBandSteeringMode(int radioIdx, int wlanIdx,int mode);

CWBool CWWTPBoardGetWlanBandSteeringPercentEnable(int radioIdx, int wlanIdx,int *enable);
CWBool CWWTPBoardSetWlanBandSteeringPercentEnable(int radioIdx, int wlanIdx,int enable);

CWBool CWWTPBoardGetWlanBandSteeringRssiEnable(int radioIdx, int wlanIdx,int *enable);
CWBool CWWTPBoardSetWlanBandSteeringRssiEnable(int radioIdx, int wlanIdx,int enable);

CWBool CWWTPBoardGetWlanBandSteeringRssi(int radioIdx, int wlanIdx,int *rssi);
CWBool CWWTPBoardSetWlanBandSteeringRssi(int radioIdx, int wlanIdx,int rssi);

CWBool CWWTPBoardGetWlanBandSteeringPercent(int radioIdx, int wlanIdx,int *present);
CWBool CWWTPBoardSetWlanBandSteeringPercent(int radioIdx, int wlanIdx,int present);

/*end*/
CWBool CWWTPBoardGetRadioCurrentChannel(int radioIdx, int *channel);

CWBool CWWTPBoardSetWlanNasIdEnableCfg(int radioIdx, int wlanIdx, int enable);
CWBool CWWTPBoardGetWlanNasIdEnableCfg(int radioIdx, int wlanIdx, int *enable);
CWBool CWWTPBoardSetWlanNasPortEnableCfg(int radioIdx, int wlanIdx, int enable);
CWBool CWWTPBoardGetWlanNasPortEnableCfg(int radioIdx, int wlanIdx, int *enable);
CWBool CWWTPBoardGetWlanNasIdCfg(int radioIdx, int wlanIdx, char **pstr);
CWBool CWWTPBoardSetWlanNasIdCfg(int radioIdx, int wlanIdx, char *pstr);
CWBool CWWTPBoardGetWlanNasPortCfg(int radioIdx, int wlanIdx, unsigned short *port);
CWBool CWWTPBoardSetWlanNasPortCfg(int radioIdx, int wlanIdx, unsigned short port);
CWBool CWWTPBoardSetWlanNasIPEnableCfg(int radioIdx, int wlanIdx, int enable);
CWBool CWWTPBoardGetWlanNasIPEnableCfg(int radioIdx, int wlanIdx, int *enable);

CWBool CWWTPBoardGetWlanNasIPCfg(int radioIdx, int wlanIdx, unsigned int *addr);
CWBool CWWTPBoardSetWlanNasIPCfg(int radioIdx, int wlanIdx, unsigned int addr);


CWBool CWWTPBoardGetMeshEnableTotalCfg(int *enable);
CWBool CWWTPBoardSetMeshEnableTotalCfg(int enable);
CWBool CWWTPBoardGetRadioMeshEnableCfg(int radioIdx, int *enable);
CWBool CWWTPBoardSetRadioMeshEnableCfg(int radioIdx, int enable);
CWBool CWWTPBoardGetRadioMeshIDCfg(int radioIdx, char **pstr);
CWBool CWWTPBoardSetRadioMeshIDCfg(int radioIdx, char *pstr);
CWBool CWWTPBoardGetRadioMeshWPAKeyCfg(int radioIdx, char **pstr);
CWBool CWWTPBoardSetRadioMeshWPAKeyCfg(int radioIdx, char *pstr);
CWBool CWWTPBoardGetRadioMeshLinkRobustThresholdCfg(int radioIdx, short *linkRobustRhreshold);
CWBool CWWTPBoardSetRadioMeshLinkRobustThresholdCfg(int radioIdx, short linkRobustRhreshold);
CWBool CWWTPBoardGetRadioMeshInfo(int radioIdx, CWRadioMeshInfo *meshStatus);

#ifdef CW_WTP_SWITCH

int CWWTPSwitchGetMaxLogicPortNum();
int CWWTPSwitchGetMaxPhysicalPortNum();
int CWWTPSwitchGetMaxTrunkPortNum();
int CWWTPSwitchGetMaxPoePortNum();
#define CWWTPSwitchGetPortNo(_port) (_port + 1)
int CWWTPSwitchGetPoePortNo(int idx);
CWBool CWWTPSwitchEnableDiscoveryTrap(int enable);
CWBool CWWTPSwitchGetPortInfo(CWWTPSwitchPortInfo *portInfo);

CWBool CWWTPSwitchGetPoeInfo(CWWTPSwitchPoeInfo *poeInfo);
CWBool CWWTPSwitchGetTopologyInfo(CWWTPSwitchTopologyInfo *topoInfo, int *msgSize);
CWBool CWWTPSwitchGetTrunkInfo(CWWTPSwitchTrunkInfo *trunkInfo, int *msgsize);
CWBool CWWTPSwitchGetPortSpeedCfg(int port, int *mode);
CWBool CWWTPSwitchSetPortSpeedCfg(int port, int mode);
CWBool CWWTPSwitchGetPortFlowCtlCfg(int port, int *flowctrl);
CWBool CWWTPSwitchSetPortFlowCtlCfg(int port, int flowctrl);
CWBool CWWTPSwitchGetPortDescpCfg(int port, char **descp);
CWBool CWWTPSwitchSetPortDescpCfg(int port, char *descp);

CWBool CWWTPSwitchGetPoePowerBudgetCfg(int *powerBuget);
CWBool CWWTPSwitchSetPoePowerBudgetCfg(int powerBuget);
CWBool CWWTPSwitchGetPoePortEnableCfg(int idx, int *enable);
CWBool CWWTPSwitchSetPoePortEnableCfg(int idx, int enable);
CWBool CWWTPSwitchGetPoePortPriorityCfg(int idx, int *priority);
CWBool CWWTPSwitchSetPoePortPriorityCfg(int idx, int priority);
CWBool CWWTPSwitchGetPoePortPowerLimitCfg(int idx, int *powerLimit);
CWBool CWWTPSwitchSetPoePortPowerLimitCfg(int idx, int powerLimit);
CWBool CWWTPSwitchGetPoePortPowerLimitTypeCfg(int idx, int *powerLimitType);
CWBool CWWTPSwitchSetPoePortPowerLimitTypeCfg(int idx, int powerLimitType);
CWBool CWWTPSwitchGetHwInfo(int *HwInfo, int *len);
CWBool CWWTPBoardSetRedundantManageEnable(int enable);
CWBool CWWTPBoardGetRedundantManageEnable(int *enable);
CWBool CWWTPBoardGetRedundantManagedMacList(int *count, CWMacAddress **macs);
CWBool CWWTPBoardSetRedundantManagedMacList(int count, CWMacAddress *macs);
CWBool CWWTPStartSwitchLogPollingTask();
void CWWTPSwitchStopLogPollingTask();

#endif/*CW_WTP_SWITCH*/

CWBool CWWTPBoardGetAcListCfg(int *count, CWHostName **hostName);
CWBool CWWTPBoardSetAcListCfg(int count, CWHostName *hostName);

CWBool CWWTPBoardGetForceAcAddress(CWHostName acAddr);
CWBool CWWTPBoardSetForceAcAddress(const CWHostName acAddr);

/* CWBool CWWTPBoardSetAutoChannelSelectionACS(CWBool enable); */
CWBool CWWTPBoardSetRadioAutoChannelSelectionACS(int radioIdx, CWBool enable);

CWBool CWWTPBoardGetLogRemoteEnable( int *Enabled);
CWBool CWWTPBoardSetLogRemoteEnable( int Enabled);

CWBool CWWTPBoardGetLogTrafficEnable( int *Enabled);
CWBool CWWTPBoardSetLogTrafficEnable( int Enabled);


CWBool CWWTPBoardGetLogRemoteCfg( char **pcfg);
CWBool CWWTPBoardSetLogRemoteCfg( char *pcfg);

CWBool CWWTPBoardSetUTCTime(int intime);
CWBool CWWTPBoardGetUTCTime(int *outtime);


CWBool CWWTPBoardGetTimeZone( int *timezone);
CWBool CWWTPBoardSetTimeZone( int timezone);


CWBool CWWTPBoardGetComEnable(void);


CWBool CWWTPBoardSetDownloadModeCfg(int radioIdx, int wlanIdx, CWRateMode mode);
CWBool CWWTPBoardGetDownloadModeCfg(int radioIdx, int wlanIdx, CWRateMode *mode);

CWBool CWWTPBoardSetUploadModeCfg(int radioIdx, int wlanIdx, CWRateMode mode);
CWBool CWWTPBoardGetUploadModeCfg(int radioIdx, int wlanIdx, CWRateMode *mode);


CWBool CWWTPBoardGetWlanL2IsolateWhiteMacListCfg(int radioIdx, int wlanIdx, int *count, CWMacAddress **macs);
CWBool CWWTPBoardSetWlanL2IsolateWhiteMacListCfg(int radioIdx, int wlanIdx, int count, CWMacAddress *macs);

CWBool CWWTPBoradPreSetConfig(CWACInfoValues *acinfo);

CWBool CWWTPBoardGetWlanHotspot20Josn(int radioIdx, int wlanIdx, char **jsonStr);
CWBool CWWTPBoardSetWlanHotspot20Josn(int radioIdx, int wlanIdx,  char *jsonStr);

#endif
#endif

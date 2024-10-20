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

#include <CWWTP.h>

#ifndef __CAPWAP_WTPBoardApiWireless_HEADER__
#define __CAPWAP_WTPBoardApiWireless_HEADER__

/*_____________________________________________________*/
/*  *******************___TYPES___*******************  */
#define MAX_SSID_SIZE 32
#define MAX_SECRET_SIZE 64
#define HOTSPOT_JSONSTR_SIZE 10240
#define WIRELESS_WIFIX_FORMAT   "wireless.wifi%d.%s"

#if SUPPORT_WLAN_5G_2_SETTING
#define WLAN_IF_NAME(_radio,_wlan) (_radio == 0 ? wlanIfNameRadio0[_wlan] : _radio == 2 ? wlanIfNameRadio2[_wlan] : wlanIfNameRadio1[_wlan])
#elif SUPPORT_WLAN_5G_SETTING
#define WLAN_IF_NAME(_radio,_wlan) (_radio == 0 ? wlanIfNameRadio0[_wlan] : wlanIfNameRadio1[_wlan])
#else
#define WLAN_IF_NAME(_radio,_wlan) wlanIfNameRadio0[_wlan]
#endif
#if SUPPORT_WLAN_5G_2_SETTING
#define HIDDEN_WLAN_IF_NAME(_radio) (_radio == 0 ? "ath29" : _radio == 2 ? "ath49" : "ath59")
#else
#define HIDDEN_WLAN_IF_NAME(_radio) (_radio == 0 ? "ath29" : "ath59")
#endif
#define GUEST_WLAN_IDX(_radio)     (CWWTPBoardGetMaxRadioWlans(_radio) - 1)
#define UCI_WLAN_IDX(_radio,_wlan) (_wlan == GUEST_WLAN_IDX(_radio) ? 25 : _wlan)
#define BRIDGE_NETWORK_ID(_rdIdx, _wlIdx)  ((_rdIdx == 0) ? (_wlIdx + 1) : (_wlIdx + 9))
#define WLAN_CMP(_rdIdx1, _wlIdx1, _rdIdx2, _wlIdx2) \
    ({ \
        int _ret; \
        if(_rdIdx1 == _rdIdx2) \
            _ret = _wlIdx1 - _wlIdx2; \
        else \
            _ret = _rdIdx1 - _rdIdx2; \
        _ret; \
    })
#if SUPPORT_WLAN_5G_2_SETTING
#define WIFI_IF_NAME(_radio)   (_radio == 2 ? "wifi2" : _radio == 1 ? "wifi1" : "wifi0")
#define WIFI_5G_2_IF_NAME	"wifi2"
#else
#define WIFI_IF_NAME(_radio)   (_radio ? "wifi1" : "wifi0")
#endif

#if (HWMODE_AC || HWMODE_AX) && SUPPORT_WLAN_5G_SETTING
#if SUPPORT_WLAN_5G_2_SETTING
#define IS_11AC_RADIO(_radio)   ((_radio == 1) || (_radio == 2))
#else
#define IS_11AC_RADIO(_radio)   (_radio == 1)
#endif
#else
#define IS_11AC_RADIO(_radio)   (0)
#endif

#define	STD_OPN		0x0001
#define	STD_WEP		0x0002
#define	STD_WPA		0x0004
#define	STD_WPA2	0x0008

#define	ENC_WEP		0x0010
#define	ENC_TKIP	0x0020
#define	ENC_WRAP	0x0040
#define	ENC_CCMP	0x0080
#define ENC_WEP40	0x1000
#define	ENC_WEP104	0x0100

#define	AUTH_OPN	0x0200
#define	AUTH_PSK	0x0400
#define	AUTH_MGT	0x0800

int CWWTPBoardGetMaxRadio();
int CWWTPBoardConfigShowOnly();
int CWWTPBoardGetMaxRadioWlans(int radioIdx);
int CWWTPBoardGetRadioDisabled(int radioIdx);
int CWWTPBoardGetRadioMode(int radioIdx);
int CWWTPBoardGetEncryptionCapabilities();
int CWWTPBoardGetMaxRadioWlansWepKeys(int radioIdx, int wlanIdx);

CWBool CWWTPBoardGetWlanBssid(int radioIdx, int wlanIdx, CWMacAddress bssid);
CWBool CWWTPBoardGetRadioMaxTxPower(int radioIdx, int *power);
CWBool CWWTPBoardGetRadioType(int radioIdx, CWRadioType *type);
CWBool CWWTPBoardGetRadioMac(int radioIdx, CWMacAddress mac);
CWBool CWWTPBoardGetRadioOperationModeCfg(int radioIdx, int *mode);
CWBool CWWTPBoardSetRadioOperationModeCfg(int radioIdx, int mode);
CWBool CWWTPBoardGetRadioType(int radioIdx, CWRadioType *type);
CWBool CWWTPBoardGetRadioMac(int radioIdx, CWMacAddress mac);
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
CWBool CWWTPBoardGetWlanEnableCfg(int radioIdx, int wlanIdx, int *enable);
CWBool CWWTPBoardSetWlanEnableCfg(int radioIdx, int wlanIdx, int enable);
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
CWBool CWWTPBoardGetWepDefaultKeyIdCfg(int radioIdx, int wlanIdx, int *keyId);
CWBool CWWTPBoardSetWepDefaultKeyIdCfg(int radioIdx, int wlanIdx, int keyId);
CWBool CWWTPBoardGetWepKeyLengthCfg(int radioIdx, int wlanIdx, int *len);
CWBool CWWTPBoardSetWepKeyLengthCfg(int radioIdx, int wlanIdx, int len);
CWBool CWWTPBoardGetWepKeyCfg(int radioIdx, int wlanIdx, int keyIdx, char **pstr);
CWBool CWWTPBoardSetWepKeyCfg(int radioIdx, int wlanIdx, int keyIdx, char *pstr);
CWBool CWWTPBoardGetWpaEncryptionCfg(int radioIdx, int wlanIdx, int *mode);
CWBool CWWTPBoardSetWpaEncryptionCfg(int radioIdx, int wlanIdx, int mode);
CWBool CWWTPBoardGetWpaPassphraseCfg(int radioIdx, int wlanIdx, char **pstr);
CWBool CWWTPBoardSetWpaPassphraseCfg(int radioIdx, int wlanIdx, char *pstr);
CWBool CWWTPBoardGetWpaGroupKeyUpdateIntervalCfg(int radioIdx, int wlanIdx, int *interval);
CWBool CWWTPBoardSetWpaGroupKeyUpdateIntervalCfg(int radioIdx, int wlanIdx, int interval);
CWBool CWWTPBoardGetSuiteBEnableCfg(int radioIdx, int wlanIdx, int *enable);
CWBool CWWTPBoardSetSuiteBEnableCfg(int radioIdx, int wlanIdx, int enable);
CWBool CWWTPBoardGetRadiusAddressCfg(int radioIdx, int wlanIdx, unsigned int *addr);
CWBool CWWTPBoardSetRadiusAddressCfg(int radioIdx, int wlanIdx, unsigned int addr);
CWBool CWWTPBoardGetRadiusPortCfg(int radioIdx, int wlanIdx, unsigned short *port);
CWBool CWWTPBoardSetRadiusPortCfg(int radioIdx, int wlanIdx, unsigned short port);
CWBool CWWTPBoardGetRadiusSecretCfg(int radioIdx, int wlanIdx, char **pstr);
CWBool CWWTPBoardSetRadiusSecretCfg(int radioIdx, int wlanIdx, char *pstr);
CWBool CWWTPBoardGetRadiusAccountingEnableCfg(int radioIdx, int wlanIdx, int *enable);
CWBool CWWTPBoardSetRadiusAccountingEnableCfg(int radioIdx, int wlanIdx, int enable);
CWBool CWWTPBoardGetRadiusAccountingAddressCfg(int radioIdx, int wlanIdx, unsigned int *addr);
CWBool CWWTPBoardSetRadiusAccountingAddressCfg(int radioIdx, int wlanIdx, unsigned int addr);
CWBool CWWTPBoardGetRadiusAccountingPortCfg(int radioIdx, int wlanIdx, unsigned short *port);
CWBool CWWTPBoardSetRadiusAccountingPortCfg(int radioIdx, int wlanIdx, unsigned short port);
CWBool CWWTPBoardGetRadiusAccountingSecretCfg(int radioIdx, int wlanIdx, char **pstr);
CWBool CWWTPBoardSetRadiusAccountingSecretCfg(int radioIdx, int wlanIdx, char *pstr);
CWBool CWWTPBoardIsolationConfig(void);
CWBool CWWTPBoardGetRadioOperationalState(int radioIdx, CWRadioState *state, CWOperationalCause *cause);
CWBool CWWTPBoardGetRadioDecryptErrorReport(int radioIdx, CWMacAddress **errorMacList, unsigned char *numEntries);
CWBool CWWTPBoardGetSitesurvey(CWRadioFreqType radioType, unsigned char *version, int *count, CWWTPSitesurvey **sitesurvey);
CWBool CWGetHealthyTxPowerValue(int i32MaxTxPower, int int32HealingTxpower, int *i32healthyTxpower);
CWBool CWWTPBoardSetRadioAutoTxPower(CWRadioFreqType radioType, int int32HealingTxpower);
CWBool CWWTPBoardSetRadioAutoTxPowerStrength(CWRadioFreqType radioType, int strength);
CWBool CWWTPBoardKickmac(CWWTPKickmacInfo *kicks);
CWBool CWWTPBoardGetWlanStations(int radioIdx, int wlanIdx, int *count, CWStation **station);
CWBool CWWTPBoardGetWlanStatistics(int radioIdx, int wlanIdx, CWWlanStatistics *statistics);
CWBool CWWTPBoardGetWlanAclModeCfg(int radioIdx, int wlanIdx, int *mode);
CWBool CWWTPBoardSetWlanAclModeCfg(int radioIdx, int wlanIdx, int mode);
CWBool CWWTPBoardGetWlanAclMacListCfg(int radioIdx, int wlanIdx, int *count, CWMacAddress **macs);
CWBool CWWTPBoardSetWlanAclMacListCfg(int radioIdx, int wlanIdx, int count, CWMacAddress *macs);
CWBool CWWTPBoardGetRadioDataRateCfg(int radioIdx, int *rate);
CWBool CWWTPBoardSetRadioDataRateCfg(int radioIdx, int rate);
CWBool CWWTPBoardGetRadioRtsCtsThresholdCfg(int radioIdx, int *threshold);
CWBool CWWTPBoardSetRadioRtsCtsThresholdCfg(int radioIdx, int threshold);
CWBool CWWTPBoardGetRadiusAccountingIntermiIntervalCfg(int radioIdx, int wlanIdx, int *interval);
CWBool CWWTPBoardSetRadiusAccountingIntermiIntervalCfg(int radioIdx, int wlanIdx, int interval);
CWBool CWWTPBoardGetWpaRadiusAddressCfg(int radioIdx, int wlanIdx, unsigned int *addr);
CWBool CWWTPBoardSetWpaRadiusAddressCfg(int radioIdx, int wlanIdx, unsigned int addr);
CWBool CWWTPBoardGetWpaRadiusPortCfg(int radioIdx, int wlanIdx, unsigned short *port);
CWBool CWWTPBoardSetWpaRadiusPortCfg(int radioIdx, int wlanIdx, unsigned short port);
CWBool CWWTPBoardGetWpaRadiusSecretCfg(int radioIdx, int wlanIdx, char **pstr);
CWBool CWWTPBoardSetWpaRadiusSecretCfg(int radioIdx, int wlanIdx, char *pstr);
CWBool CWWTPBoardGetRadioObeyRegulatoryPowerCfg(int radioIdx, int *enable);
CWBool CWWTPBoardSetRadioObeyRegulatoryPowerCfg(int radioIdx, int enable);
CWBool CWWTPBoardGetRadioBackgroundScanningCfg(int radioIdx, int *enable);
CWBool CWWTPBoardSetRadioBackgroundScanningCfg(int radioIdx, int enable);
CWBool CWWTPBoardGetBandSteeringCfg(int *enable);
CWBool CWWTPBoardSetBandSteeringCfg(int enable);
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
CWBool CWWTPBoardGetFastHandoverStatusCfg(int *enable);
CWBool CWWTPBoardSetFastHandoverStatusCfg(int enable);
CWBool CWWTPBoardGetFastHandoverRssiCfg(int *rssi);
CWBool CWWTPBoardSetFastHandoverRssiCfg(int rssi);
CWBool CWWTPBoardGetRadioFastHandoverStatusCfg(int radioIdx, int *enable);
CWBool CWWTPBoardSetRadioFastHandoverStatusCfg(int radioIdx, int enable);
CWBool CWWTPBoardGetRadioFastHandoverRssiCfg(int radioIdx, int *rssi);
CWBool CWWTPBoardSetRadioFastHandoverRssiCfg(int radioIdx, int rssi);
CWBool CWWTPBoardGetDownloadLimitCfg(int radioIdx, int wlanIdx, int *limit);
CWBool CWWTPBoardSetDownloadLimitCfg(int radioIdx, int wlanIdx, int limit);
CWBool CWWTPBoardGetUploadLimitCfg(int radioIdx, int wlanIdx, int *limit);
CWBool CWWTPBoardSetUploadLimitCfg(int radioIdx, int wlanIdx, int limit);
CWBool CWWTPBoardGetDownloadPerUserEnableCfg(int radioIdx, int wlanIdx, int *enable);
CWBool CWWTPBoardSetDownloadPerUserEnableCfg(int radioIdx, int wlanIdx, int enable);
CWBool CWWTPBoardGetUploadPerUserEnableCfg(int radioIdx, int wlanIdx, int *enable);
CWBool CWWTPBoardSetUploadPerUserEnableCfg(int radioIdx, int wlanIdx, int enable);
CWBool CWWTPBoardSetWlanMaxDownloadLimitCfg(int radioIdx, int wlanIdx, int limit);
CWBool CWWTPBoardSetWlanPerUserDownloadLimitCfg(int radioIdx, int wlanIdx, int limit);
CWBool CWWTPBoardSetWlanMaxUploadLimitCfg(int radioIdx, int wlanIdx, int limit);
CWBool CWWTPBoardSetWlanPerUserUploadLimitCfg(int radioIdx, int wlanIdx, int limit);
CWBool CWWTPBoardGetRoamingEnableCfg(int radioIdx, int wlanIdx, int *enable);
CWBool CWWTPBoardSetRoamingEnableCfg(int radioIdx, int wlanIdx, int enable);
CWBool CWWTPBoardGetRoamingAdvSearchCfg(int radioIdx, int wlanIdx, int *enable);
CWBool CWWTPBoardSetRoamingAdvSearchCfg(int radioIdx, int wlanIdx, int enable);
CWBool CWWTPBoardGetRadioDistance(int radioIdx, int *distance);
CWBool CWWTPBoardSetRadioDistance(int radioIdx, int distance);
CWBool CWWTPBoardGetRadioCurrentTxPower(int radioIdx, int *power);
CWBool CWWTPBoardGetRadioCurrentChannel(int radioIdx, int *channel);
CWBool CWWTPBoardGetNextFastScanChannel(int radioIdx, int *channel);
#if 0//SUPPORT_WLAN_5G_2_SETTING 
CWBool CWWTPBoardGetNextFastScanChannel_5G_2(int radioIdx, int *channel);
#endif
CWBool CWWTPBoardSetFastScanDurationTime(int radioIdx, unsigned int duration);
CWBool CWWTPBoardGetScan(CWBool bDisplay, CWWTPSitesurveyInfo *sitesurveyInfo);
CWBool CWWTPBoardGetChannelUtilization(CWRadioFreqType radioType, int channel, unsigned char *chanUtil);
CWBool CWWTPBoardSetScanDwellTime(int radioIdx, unsigned int min, unsigned int max);
#if 0
CWBool CWWTPBoardSetAutoChannelSelectionACS(CWBool enable);
#else
CWBool CWWTPBoardSetRadioAutoChannelSelectionACS(int radioIdx, CWBool enable);
#endif
CWBool CWWTPBoardGetRadioCurrentAvailableChannelList(int i32RadioIdx, unsigned char *pAvalaibleChanCount, unsigned char **pAvailableChanList);
CWBool CWWTPBoardGetWlanWpaPMFEnableCfg(int radioIdx, int wlanIdx, int *enable);
CWBool CWWTPBoardSetWlanWpaPMFEnableCfg(int radioIdx, int wlanIdx, int enable);
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

CWBool CWWTPBoardSetDownloadModeCfg(int radioIdx, int wlanIdx, CWRateMode mode);
CWBool CWWTPBoardGetDownloadModeCfg(int radioIdx, int wlanIdx, CWRateMode *mode);

CWBool CWWTPBoardSetUploadModeCfg(int radioIdx, int wlanIdx, CWRateMode mode);
CWBool CWWTPBoardGetUploadModeCfg(int radioIdx, int wlanIdx, CWRateMode *mode);

CWBool CWWTPBoardGetWlanL2IsolateWhiteMacListCfg(int radioIdx, int wlanIdx, int *count, CWMacAddress **macs);
CWBool CWWTPBoardSetWlanL2IsolateWhiteMacListCfg(int radioIdx, int wlanIdx, int count, CWMacAddress *macs);

CWBool CWWTPBoardGetRadioBitRateCfg(int radioIdx, int *rate);
CWBool CWWTPBoardSetRadioBitRateCfg(int radioIdx, int rate);

CWBool CWWTPBoardGetWlanHotspot20Josn(int radioIdx, int wlanIdx, char **jsonStr);
CWBool CWWTPBoardSetWlanHotspot20Josn(int radioIdx, int wlanIdx,  char *jsonStr);
/*end*/
#endif

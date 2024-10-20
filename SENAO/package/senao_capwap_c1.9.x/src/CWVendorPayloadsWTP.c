/************************************************************************************************
 * Copyright (c) 2006-2009 Laboratorio di Sistemi di Elaborazione e Bioingegneria Informatica	*
 *                          Universita' Campus BioMedico - Italy								*
 *																								*
 * This program is free software; you can redistribute it and/or modify it under the terms		*
 * of the GNU General Public License as published by the free Software Foundation; either		*
 * version 2 of the License, or (at your option) any later version.								*
 *																								*
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY				*
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A				*
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.						*
 *																								*
 * You should have received a copy of the GNU General Public License along with this			*
 * program; if not, write to the:																*
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,							*
 * MA  02111-1307, USA.																			*
 *																								*
 * -------------------------------------------------------------------------------------------- *
 * Project:  Capwap																				*
 *																								*
 * Authors : Matteo Latini (mtylty@gmail.com)
 *           Donato Capitella (d.capitella@gmail.com)
 *
 ************************************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/statvfs.h>

#ifdef DMALLOC
#include "../dmalloc-5.5.0/dmalloc.h"
#endif

#include "CWWTP.h"
#include "CWVendorPayloads.h"

/************************************************************************************/
/*                                  HELP macro                                                                                             */
/************************************************************************************/

#define CFG_MSG_INFO_ENTRY(_msgType, _getnext) \
	{_msgType, #_msgType, CWCfgMsgGet_##_msgType, CWCfgMsgSet_##_msgType, _getnext, NULL}

#define CFG_MSG_INFO_ENTRY_IMP_ARRAY_TYPE(_msgType, _type, _on_init, _get_method, _set_method) \
	CWErrorCode CWCfgMsgGet_##_msgType(int keyLen, void *keyPtr, int *valLen, void **valPtr) { \
		CWBool ret; \
		_type *array = NULL; \
		int arraySize = 0; \
		_on_init; \
		ret = (_get_method); \
		if(!ret) \
			return CWErrorGetLastErrorCode(); \
		*valPtr = (void*) array; \
		*valLen = arraySize * sizeof(_type); \
		return CW_ERROR_SUCCESS; \
	} \
	CWErrorCode CWCfgMsgSet_##_msgType(int keyLen, void *keyPtr, int valLen, void *valPtr) { \
		CWBool ret; \
		_type *array = NULL; \
		int arraySize = 0; \
		_on_init; \
		arraySize = valLen / sizeof(_type); \
		array = valPtr; \
		ret = (_set_method); \
		return ret ? CW_ERROR_SUCCESS : CWErrorGetLastErrorCode(); \
	}

#define CFG_MSG_INFO_ENTRY_IMP_IPV4_ADDR_TYPE(_msgType, _on_init, _get_method, _set_method) \
	CWErrorCode CWCfgMsgGet_##_msgType(int keyLen, void *keyPtr, int *valLen, void **valPtr) { \
		unsigned int addr; \
		CWBool ret; \
		_on_init; \
		ret = (_get_method); \
		if(!ret) \
			return CWErrorGetLastErrorCode(); \
		CW_CFG_MSG_GET_IPV4_ADDR(addr); \
		return CW_ERROR_SUCCESS; \
	} \
	CWErrorCode CWCfgMsgSet_##_msgType(int keyLen, void *keyPtr, int valLen, void *valPtr) { \
		unsigned int addr; \
		CWBool ret; \
		_on_init; \
		CW_CFG_MSG_SET_IPV4_ADDR(addr); \
		ret = (_set_method); \
		return ret ? CW_ERROR_SUCCESS : CWErrorGetLastErrorCode(); \
	}
#define CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(_msgType, _on_init, _get_method, _set_method) \
	CWErrorCode CWCfgMsgGet_##_msgType(int keyLen, void *keyPtr, int *valLen, void **valPtr) { \
		int val32; \
		CWBool ret; \
		_on_init; \
		ret = (_get_method); \
		if(!ret) \
			return CWErrorGetLastErrorCode(); \
		CW_CFG_MSG_GET_VAL32(val32); \
		return CW_ERROR_SUCCESS; \
	} \
	CWErrorCode CWCfgMsgSet_##_msgType(int keyLen, void *keyPtr, int valLen, void *valPtr) { \
		int val32; \
		CWBool ret; \
		_on_init; \
		CW_CFG_MSG_SET_VAL32(val32); \
		ret = (_set_method); \
		return ret ? CW_ERROR_SUCCESS : CWErrorGetLastErrorCode(); \
	}

#define CFG_MSG_INFO_ENTRY_IMP_VAL16_TYPE(_msgType, _on_init, _get_method, _set_method) \
	CWErrorCode CWCfgMsgGet_##_msgType(int keyLen, void *keyPtr, int *valLen, void **valPtr) { \
		short val16; \
		CWBool ret; \
		_on_init; \
		ret = (_get_method); \
		if(!ret) \
			return CWErrorGetLastErrorCode(); \
		CW_CFG_MSG_GET_VAL16(val16); \
		return CW_ERROR_SUCCESS; \
	} \
	CWErrorCode CWCfgMsgSet_##_msgType(int keyLen, void *keyPtr, int valLen, void *valPtr) { \
		short val16; \
		CWBool ret; \
		_on_init; \
		CW_CFG_MSG_SET_VAL16(val16); \
		ret = (_set_method); \
		return ret ? CW_ERROR_SUCCESS : CWErrorGetLastErrorCode(); \
	}

#define CFG_MSG_INFO_ENTRY_IMP_VAL8_TYPE(_msgType, _on_init, _get_method, _set_method) \
	CWErrorCode CWCfgMsgGet_##_msgType(int keyLen, void *keyPtr, int *valLen, void **valPtr) { \
		char val8; \
		CWBool ret; \
		_on_init; \
		ret = (_get_method); \
		if(!ret) \
			return CWErrorGetLastErrorCode(); \
		CW_CFG_MSG_GET_VAL8(val8); \
		return CW_ERROR_SUCCESS; \
	} \
	CWErrorCode CWCfgMsgSet_##_msgType(int keyLen, void *keyPtr, int valLen, void *valPtr) { \
		char val; \
		CWBool ret; \
		_on_init; \
		CW_CFG_MSG_SET_VAL8(val); \
		ret = (_set_method); \
		return ret ? CW_ERROR_SUCCESS : CWErrorGetLastErrorCode(); \
	}

#define CFG_MSG_INFO_ENTRY_IMP_OBJECT_TYPE(_msgType, _type, _on_init, _get_method, _set_method) \
	CWErrorCode CWCfgMsgGet_##_msgType(int keyLen, void *keyPtr, int *valLen, void **valPtr) { \
		_type *objPtr; \
		CWBool ret; \
		_on_init; \
		CW_CREATE_OBJECT_ERR(objPtr, _type, { \
				CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL); \
				return CW_ERROR_OUT_OF_MEMORY; }); \
		ret = (_get_method); \
		if(!ret) { \
			CW_FREE_OBJECT(objPtr); \
			return CWErrorGetLastErrorCode(); \
		} \
		*valLen = sizeof(_type); \
		*valPtr = (void*) objPtr; \
		return CW_ERROR_SUCCESS; \
	} \
	CWErrorCode CWCfgMsgSet_##_msgType(int keyLen, void *keyPtr, int valLen, void *valPtr) { \
		_type *objPtr; \
		CWBool ret; \
		if(valLen < sizeof(_type) || !valPtr) \
			return CW_ERROR_WRONG_ARG; \
		_on_init; \
		objPtr = (_type *) valPtr; \
		ret = (_set_method); \
		return ret ? CW_ERROR_SUCCESS : CWErrorGetLastErrorCode(); \
	}

#define CFG_MSG_INFO_ENTRY_IMP_STR_TYPE(_msgType, _on_init, _get_method, _set_method) \
	CWErrorCode CWCfgMsgGet_##_msgType(int keyLen, void *keyPtr, int *valLen, void **valPtr) { \
		char *str = NULL; \
		CWBool ret; \
		_on_init; \
		ret = (_get_method); \
		if(!ret) \
			return CWErrorGetLastErrorCode(); \
		CW_CFG_MSG_GET_STR(str); \
		return CW_ERROR_SUCCESS; \
	} \
	CWErrorCode CWCfgMsgSet_##_msgType(int keyLen, void *keyPtr, int valLen, void *valPtr) { \
		char *str = NULL; \
		CWBool ret; \
		_on_init; \
		CW_CFG_MSG_SET_STR(str); \
		ret = (_set_method); \
		CW_FREE_OBJECT(str); \
		return ret ? CW_ERROR_SUCCESS : CWErrorGetLastErrorCode(); \
	}

#define CW_WTP_CFG_MSG_GET_PORT_INDEX()\
	CW_CFG_MSG_GET_PORT_INDEX(CWWTPSwitchGetMaxLogicPortNum())

#define CW_WTP_CFG_MSG_GET_PHYSIC_PORT_INDEX()\
	CW_CFG_MSG_GET_PORT_INDEX(CWWTPSwitchGetMaxPhysicalPortNum())

#define CW_WTP_CFG_MSG_GET_POE_PORT_INDEX()\
	CW_CFG_MSG_GET_PORT_INDEX(CWWTPSwitchGetMaxPoePortNum())

#define CW_WTP_CFG_MSG_GET_LAN_PORT_INDEX()\
	CW_CFG_MSG_GET_PORT_INDEX(CWWTPBoardGetMaxLanPortNum())

#define CW_WTP_CFG_MSG_GET_RADIO_INDEX() \
	CW_CFG_MSG_GET_RADIO_INDEX(CWWTPBoardGetMaxRadio())

#define CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX() \
	CW_CFG_MSG_GET_RADIO_WLAN_INDEX(CWWTPBoardGetMaxRadio(), \
		CWWTPBoardGetMaxRadioWlans(radioIdx))

#define CW_WTP_CFG_MSG_GET_RADIO_WLAN_WEPKEY_INDEX() \
	CW_CFG_MSG_GET_RADIO_WLAN_WEPKEY_INDEX(CWWTPBoardGetMaxRadio(), \
		CWWTPBoardGetMaxRadioWlans(radioIdx), \
		CWWTPBoardGetMaxRadioWlansWepKeys(radioIdx, wlanIdx))

#define CW_WTP_CFG_MSG_GETNEXT_RADIO_INDEX() \
	CW_CFG_MSG_GETNEXT_RADIO_INDEX(CWWTPBoardGetMaxRadio())

#define CW_WTP_CFG_MSG_GETNEXT_RADIO_WLAN_INDEX() \
	CW_CFG_MSG_GETNEXT_RADIO_WLAN_INDEX(CWWTPBoardGetMaxRadio(), \
		CWWTPBoardGetMaxRadioWlans(radioIdx))

#define CW_WTP_CFG_MSG_GETNEXT_RADIO_WLAN_WEPKEY_INDEX() \
	CW_CFG_MSG_GETNEXT_RADIO_WLAN_WEPKEY_INDEX(CWWTPBoardGetMaxRadio(), \
		CWWTPBoardGetMaxRadioWlans(radioIdx), \
		CWWTPBoardGetMaxRadioWlansWepKeys(radioIdx, wlanIdx))

#define CW_WTP_CFG_MSG_GETNEXT_PORT_INDEX() \
	CW_CFG_MSG_GETNEXT_PORT_INDEX(CWWTPSwitchGetMaxLogicPortNum())

#define CW_WTP_CFG_MSG_GETNEXT_PHYSIC_PORT_INDEX() \
	CW_CFG_MSG_GETNEXT_PORT_INDEX(CWWTPSwitchGetMaxPhysicalPortNum())

#define CW_WTP_CFG_MSG_GETNEXT_POE_PORT_INDEX() \
	CW_CFG_MSG_GETNEXT_PORT_INDEX(CWWTPSwitchGetMaxPoePortNum())

#define CW_WTP_CFG_MSG_GETNEXT_LAN_PORT_INDEX() \
	CW_CFG_MSG_GETNEXT_PORT_INDEX(CWWTPBoardGetMaxLanPortNum())


/***************************************************************************/
/*                                 function implementation                                                            */
/***************************************************************************/

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef CW_WTP_AP

CWErrorCode CWCfgMsgGetLanPortNextKey(int keyLen, void *keyPtr, int *nextkeyLen, void **nextkeyPtr)
{
    CW_WTP_CFG_MSG_GETNEXT_LAN_PORT_INDEX();
}

CWErrorCode CWCfgMsgGetRadioNextKey(int keyLen, void *keyPtr, int *nextkeyLen, void **nextkeyPtr)
{
    CW_WTP_CFG_MSG_GETNEXT_RADIO_INDEX();
}

CWErrorCode CWCfgMsgGetWlanNextKey(int keyLen, void *keyPtr, int *nextkeyLen, void **nextkeyPtr)
{
    CW_WTP_CFG_MSG_GETNEXT_RADIO_WLAN_INDEX();
}

CWErrorCode CWCfgMsgGetWepKeyNextKey(int keyLen, void *keyPtr, int *nextkeyLen, void **nextkeyPtr)
{
    CW_WTP_CFG_MSG_GETNEXT_RADIO_WLAN_WEPKEY_INDEX();
}

CFG_MSG_INFO_ENTRY_IMP_STR_TYPE(WTP_CFG_AP_NAME,
                                CFG_MSG_INIT_DO_NOTHING,
                                CWWTPBoardGetNameCfg(&str),
                                CWWTPBoardSetNameCfg(str)
                               )

CFG_MSG_INFO_ENTRY_IMP_STR_TYPE(WTP_CFG_AP_LOCATION,
                                CFG_MSG_INIT_DO_NOTHING,
                                CWWTPBoardGetLocationCfg(&str),
                                CWWTPBoardSetLocationCfg(str)
                               )

CFG_MSG_INFO_ENTRY_IMP_OBJECT_TYPE(WTP_CFG_AP_IPV4, CWIPv4Cfg,
                                   CFG_MSG_INIT_DO_NOTHING,
                                   CWWTPBoardGetIPv4Cfg(objPtr),
                                   CWWTPBoardSetIPv4Cfg(objPtr)
                                  )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_RADIO_NUM,
                                  CFG_MSG_INIT_DO_NOTHING,
                                  (val32 = CWWTPBoardGetMaxRadio()) >= 0 ? CW_TRUE : CWErrorRaise(CW_ERROR_WRONG_ARG, NULL),
                                  val32 == CWWTPBoardGetMaxRadio() ? CW_TRUE : CWErrorRaise(CW_ERROR_WRONG_ARG, NULL)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_RADIO_OPERATION_MODE,
                                  CW_WTP_CFG_MSG_GET_RADIO_INDEX(),
                                  CWWTPBoardGetRadioOperationModeCfg(radioIdx, &val32),
                                  CWWTPBoardSetRadioOperationModeCfg(radioIdx, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_RADIO_WIRELESS_MODE,
                                  CW_WTP_CFG_MSG_GET_RADIO_INDEX(),
                                  CWWTPBoardGetRadioWirelessModeCfg(radioIdx, &val32),
                                  CWWTPBoardSetRadioWirelessModeCfg(radioIdx, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_RADIO_COUNTRY_CODE,
                                  CW_WTP_CFG_MSG_GET_RADIO_INDEX(),
                                  CWWTPBoardGetRadioCountryCodeCfg(radioIdx, &val32),
                                  CWWTPBoardSetRadioCountryCodeCfg(radioIdx, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_RADIO_CHANNEL_HT_MODE,
                                  CW_WTP_CFG_MSG_GET_RADIO_INDEX(),
                                  CWWTPBoardGetRadioChannelHTModeCfg(radioIdx, &val32),
                                  CWWTPBoardSetRadioChannelHTModeCfg(radioIdx, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_RADIO_CHANNEL_EXT,
                                  CW_WTP_CFG_MSG_GET_RADIO_INDEX(),
                                  CWWTPBoardGetRadioChannelExtCfg(radioIdx, &val32),
                                  CWWTPBoardSetRadioChannelExtCfg(radioIdx, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_RADIO_CHANNEL,
                                  CW_WTP_CFG_MSG_GET_RADIO_INDEX(),
                                  CWWTPBoardGetRadioChannelCfg(radioIdx, &val32),
                                  CWWTPBoardSetRadioChannelCfg(radioIdx, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_RADIO_LIMITED_CLIENTS_ENABLE,
                                  CW_WTP_CFG_MSG_GET_RADIO_INDEX(),
                                  CWWTPBoardGetRadioLimitedClientsEnableCfg(radioIdx, &val32),
                                  CWWTPBoardSetRadioLimitedClientsEnableCfg(radioIdx, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_RADIO_LIMITED_CLIENTS,
                                  CW_WTP_CFG_MSG_GET_RADIO_INDEX(),
                                  CWWTPBoardGetRadioLimitedClientsCfg(radioIdx, &val32),
                                  CWWTPBoardSetRadioLimitedClientsCfg(radioIdx, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_RADIO_TX_POWER,
                                  CW_WTP_CFG_MSG_GET_RADIO_INDEX(),
                                  CWWTPBoardGetRadioTxPowerCfg(radioIdx, &val32),
                                  CWWTPBoardSetRadioTxPowerCfg(radioIdx, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_RADIO_AGGRE_ENABLE,
                                  CW_WTP_CFG_MSG_GET_RADIO_INDEX(),
                                  CWWTPBoardGetRadioAggregationEnableCfg(radioIdx, &val32),
                                  CWWTPBoardSetRadioAggregationEnableCfg(radioIdx, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_RADIO_AGGRE_FRAMES,
                                  CW_WTP_CFG_MSG_GET_RADIO_INDEX(),
                                  CWWTPBoardGetRadioAggregationFramesCfg(radioIdx, &val32),
                                  CWWTPBoardSetRadioAggregationFramesCfg(radioIdx, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_RADIO_AGGRE_MAXBYTES,
                                  CW_WTP_CFG_MSG_GET_RADIO_INDEX(),
                                  CWWTPBoardGetRadioAggregationMaxBytesCfg(radioIdx, &val32),
                                  CWWTPBoardSetRadioAggregationMaxBytesCfg(radioIdx, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_RADIO_WLAN_NUM,
                                  CW_WTP_CFG_MSG_GET_RADIO_INDEX(),
                                  (val32 = CWWTPBoardGetMaxRadioWlans(radioIdx)) > 0 ? CW_TRUE : CWErrorRaise(CW_ERROR_WRONG_ARG, NULL),
                                  val32 == CWWTPBoardGetMaxRadioWlans(radioIdx) ? CW_TRUE : CWErrorRaise(CW_ERROR_WRONG_ARG, NULL)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_WLAN_ENABLE,
                                  CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                  CWWTPBoardGetWlanEnableCfg(radioIdx, wlanIdx, &val32),
                                  CWWTPBoardSetWlanEnableCfg(radioIdx, wlanIdx, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_STR_TYPE(WTP_CFG_AP_WLAN_SSID,
                                CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                CWWTPBoardGetWlanSsidCfg(radioIdx, wlanIdx, &str),
                                CWWTPBoardSetWlanSsidCfg(radioIdx, wlanIdx, str)
                               )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_WLAN_SUPPRESSED_SSID,
                                  CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                  CWWTPBoardGetWlanSuppressedSsidCfg(radioIdx, wlanIdx, &val32),
                                  CWWTPBoardSetWlanSuppressedSsidCfg(radioIdx, wlanIdx, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_WLAN_STA_SEPARATION,
                                  CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                  CWWTPBoardGetWlanStationSeparationCfg(radioIdx, wlanIdx, &val32),
                                  CWWTPBoardSetWlanStationSeparationCfg(radioIdx, wlanIdx, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_WLAN_LAYER2_ISOLATION,
                                  CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                  CWWTPBoardGetWlanLayer2IsolationCfg(radioIdx, wlanIdx, &val32),
                                  CWWTPBoardSetWlanLayer2IsolationCfg(radioIdx, wlanIdx, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_WLAN_ISOLATION,
                                  CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                  CWWTPBoardGetWlanIsolationCfg(radioIdx, wlanIdx, &val32),
                                  CWWTPBoardSetWlanIsolationCfg(radioIdx, wlanIdx, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_WLAN_VLAN,
                                  CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                  CWWTPBoardGetWlanVlanCfg(radioIdx, wlanIdx, &val32),
                                  CWWTPBoardSetWlanVlanCfg(radioIdx, wlanIdx, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_WLAN_SECURITY_MODE,
                                  CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                  CWWTPBoardGetWlanSecurityCfg(radioIdx, wlanIdx, &val32),
                                  CWWTPBoardSetWlanSecurityCfg(radioIdx, wlanIdx, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_WLAN_WEP_AUTH_TYPE,
                                  CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                  CWWTPBoardGetWepAuthTypeCfg(radioIdx, wlanIdx, &val32),
                                  CWWTPBoardSetWepAuthTypeCfg(radioIdx, wlanIdx, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_WLAN_WEP_INPUT_METHOD,
                                  CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                  CWWTPBoardGetWepInputMethodCfg(radioIdx, wlanIdx, &val32),
                                  CWWTPBoardSetWepInputMethodCfg(radioIdx, wlanIdx, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_WLAN_WEP_KEY_LENGTH,
                                  CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                  CWWTPBoardGetWepKeyLengthCfg(radioIdx, wlanIdx, &val32),
                                  CWWTPBoardSetWepKeyLengthCfg(radioIdx, wlanIdx, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_WLAN_WEP_DEF_KEY_ID,
                                  CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                  CWWTPBoardGetWepDefaultKeyIdCfg(radioIdx, wlanIdx, &val32),
                                  CWWTPBoardSetWepDefaultKeyIdCfg(radioIdx, wlanIdx, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_STR_TYPE(WTP_CFG_AP_WLAN_WEP_KEY,
                                CW_WTP_CFG_MSG_GET_RADIO_WLAN_WEPKEY_INDEX(),
                                CWWTPBoardGetWepKeyCfg(radioIdx, wlanIdx, wepKeyIdx, &str),
                                CWWTPBoardSetWepKeyCfg(radioIdx, wlanIdx, wepKeyIdx, str)
                               )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_WLAN_WPA_ENCRYPT_MODE,
                                  CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                  CWWTPBoardGetWpaEncryptionCfg(radioIdx, wlanIdx, &val32),
                                  CWWTPBoardSetWpaEncryptionCfg(radioIdx, wlanIdx, val32)
                                 )
CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_WLAN_WPA_PMF_ENABLE,
                                  CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                  CWWTPBoardGetWlanWpaPMFEnableCfg(radioIdx, wlanIdx, &val32),
                                  CWWTPBoardSetWlanWpaPMFEnableCfg(radioIdx, wlanIdx, val32)
                                 )


CFG_MSG_INFO_ENTRY_IMP_STR_TYPE(WTP_CFG_AP_WLAN_WPA_PASSPHRASE,
                                CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                CWWTPBoardGetWpaPassphraseCfg(radioIdx, wlanIdx, &str),
                                CWWTPBoardSetWpaPassphraseCfg(radioIdx, wlanIdx, str)
                               )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_WLAN_WPA_GROUP_KEY_INT,
                                  CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                  CWWTPBoardGetWpaGroupKeyUpdateIntervalCfg(radioIdx, wlanIdx, &val32),
                                  CWWTPBoardSetWpaGroupKeyUpdateIntervalCfg(radioIdx, wlanIdx, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_WLAN_SUITEB_ENABLE,
                                  CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                  CWWTPBoardGetSuiteBEnableCfg(radioIdx, wlanIdx, &val32),
                                  CWWTPBoardSetSuiteBEnableCfg(radioIdx, wlanIdx, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_IPV4_ADDR_TYPE(WTP_CFG_AP_WLAN_RADIUS_ADDR,
                                      CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                      CWWTPBoardGetWpaRadiusAddressCfg(radioIdx, wlanIdx, &addr),
                                      CWWTPBoardSetWpaRadiusAddressCfg(radioIdx, wlanIdx, addr)
                                     )

CFG_MSG_INFO_ENTRY_IMP_VAL16_TYPE(WTP_CFG_AP_WLAN_RADIUS_PORT,
                                  CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                  CWWTPBoardGetWpaRadiusPortCfg(radioIdx, wlanIdx, (unsigned short *)&val16),
                                  CWWTPBoardSetWpaRadiusPortCfg(radioIdx, wlanIdx, (unsigned short)val16)
                                 )

CFG_MSG_INFO_ENTRY_IMP_STR_TYPE(WTP_CFG_AP_WLAN_RADIUS_SECRET,
                                CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                CWWTPBoardGetWpaRadiusSecretCfg(radioIdx, wlanIdx, &str),
                                CWWTPBoardSetWpaRadiusSecretCfg(radioIdx, wlanIdx, str)
                               )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_WLAN_RADIUS_ACC_ENABLE,
                                  CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                  CWWTPBoardGetRadiusAccountingEnableCfg(radioIdx, wlanIdx, &val32),
                                  CWWTPBoardSetRadiusAccountingEnableCfg(radioIdx, wlanIdx, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_IPV4_ADDR_TYPE(WTP_CFG_AP_WLAN_RADIUS_ACC_ADDR,
                                      CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                      CWWTPBoardGetRadiusAccountingAddressCfg(radioIdx, wlanIdx, &addr),
                                      CWWTPBoardSetRadiusAccountingAddressCfg(radioIdx, wlanIdx, addr)
                                     )

CFG_MSG_INFO_ENTRY_IMP_VAL16_TYPE(WTP_CFG_AP_WLAN_RADIUS_ACC_PORT,
                                  CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                  CWWTPBoardGetRadiusAccountingPortCfg(radioIdx, wlanIdx, (unsigned short *)&val16),
                                  CWWTPBoardSetRadiusAccountingPortCfg(radioIdx, wlanIdx, (unsigned short)val16)
                                 )

CFG_MSG_INFO_ENTRY_IMP_STR_TYPE(WTP_CFG_AP_WLAN_RADIUS_ACC_SECRET,
                                CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                CWWTPBoardGetRadiusAccountingSecretCfg(radioIdx, wlanIdx, &str),
                                CWWTPBoardSetRadiusAccountingSecretCfg(radioIdx, wlanIdx, str)
                               )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_WLAN_ACL_MODE,
                                  CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                  CWWTPBoardGetWlanAclModeCfg(radioIdx, wlanIdx, &val32),
                                  CWWTPBoardSetWlanAclModeCfg(radioIdx, wlanIdx, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_ARRAY_TYPE(WTP_CFG_AP_WLAN_ACL_MAC_LIST, CWMacAddress,
                                  CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                  CWWTPBoardGetWlanAclMacListCfg(radioIdx, wlanIdx, &arraySize, &array),
                                  CWWTPBoardSetWlanAclMacListCfg(radioIdx, wlanIdx, arraySize, array)
                                 )

CFG_MSG_INFO_ENTRY_IMP_ARRAY_TYPE(WTP_CFG_AP_WLAN_L2_ISOLATION_WHITE_MAC_LIST, CWMacAddress,
                                  CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                  CWWTPBoardGetWlanL2IsolateWhiteMacListCfg(radioIdx, wlanIdx, &arraySize, &array),
                                  CWWTPBoardSetWlanL2IsolateWhiteMacListCfg(radioIdx, wlanIdx, arraySize, array)
                                 )


CFG_MSG_INFO_ENTRY_IMP_IPV4_ADDR_TYPE(WTP_CFG_AP_DNS1,
                                      CFG_MSG_INIT_DO_NOTHING,
                                      CWWTPBoardGetDns1Cfg(&addr),
                                      CWWTPBoardSetDns1Cfg(addr)
                                     )

CFG_MSG_INFO_ENTRY_IMP_IPV4_ADDR_TYPE(WTP_CFG_AP_DNS2,
                                      CFG_MSG_INIT_DO_NOTHING,
                                      CWWTPBoardGetDns2Cfg(&addr),
                                      CWWTPBoardSetDns2Cfg(addr)
                                     )

CFG_MSG_INFO_ENTRY_IMP_STR_TYPE(WTP_CFG_AP_ADMIN,
                                CFG_MSG_INIT_DO_NOTHING; ,
                                CWWTPBoardGetAdminCfg(&str),
                                CWWTPBoardSetAdminCfg(str)
                               )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_RADIO_DATA_RATE,
                                  CW_WTP_CFG_MSG_GET_RADIO_INDEX(),
                                  CWWTPBoardGetRadioDataRateCfg(radioIdx, &val32),
                                  CWWTPBoardSetRadioDataRateCfg(radioIdx, val32)
                                 )
CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_RADIO_BIT_RATE,
                                  CW_WTP_CFG_MSG_GET_RADIO_INDEX(),
                                  CWWTPBoardGetRadioBitRateCfg(radioIdx, &val32),
                                  CWWTPBoardSetRadioBitRateCfg(radioIdx, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_RADIO_AX_ENABLE,
                                  CW_WTP_CFG_MSG_GET_RADIO_INDEX(),
                                  CWWTPBoardGetRadioWirelessModeAXEnableCfg(radioIdx, &val32),
                                  CWWTPBoardSetRadioWirelessModeAXEnableCfg(radioIdx, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_RADIO_RTSCTS_THRESHOLD,
                                  CW_WTP_CFG_MSG_GET_RADIO_INDEX(),
                                  CWWTPBoardGetRadioRtsCtsThresholdCfg(radioIdx, &val32),
                                  CWWTPBoardSetRadioRtsCtsThresholdCfg(radioIdx, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_WLAN_RADIUS_ACC_INTERIM_INT,
                                  CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                  CWWTPBoardGetRadiusAccountingIntermiIntervalCfg(radioIdx, wlanIdx, &val32),
                                  CWWTPBoardSetRadiusAccountingIntermiIntervalCfg(radioIdx, wlanIdx, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_RADIO_OBEY_REGULATORY_POWER,
                                  CW_WTP_CFG_MSG_GET_RADIO_INDEX(),
                                  CWWTPBoardGetRadioObeyRegulatoryPowerCfg(radioIdx, &val32),
                                  CWWTPBoardSetRadioObeyRegulatoryPowerCfg(radioIdx, val32)
                                 )

/*Added by larger for Band Streeing*/
CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_BAND_STREERING,
                                  CFG_MSG_INIT_DO_NOTHING,
                                  CWWTPBoardGetBandSteeringCfg(&val32),
                                  CWWTPBoardSetBandSteeringCfg(val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_BANDSTREERING_MODE,
                                  CFG_MSG_INIT_DO_NOTHING,
                                  CWWTPBoardGetBandSteeringMode(&val32),
                                  CWWTPBoardSetBandSteeringMode(val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_BANDSTREERING_PERCENT_ENABLE,
                                  CFG_MSG_INIT_DO_NOTHING,
                                  CWWTPBoardGetBandSteeringPercentEnable(&val32),
                                  CWWTPBoardSetBandSteeringPercentEnable(val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_BANDSTREERING_RSSI_ENABLE,
                                  CFG_MSG_INIT_DO_NOTHING,
                                  CWWTPBoardGetBandSteeringRssiEnable(&val32),
                                  CWWTPBoardSetBandSteeringRssiEnable(val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_BANDSTREERING_RSSI,
                                  CFG_MSG_INIT_DO_NOTHING,
                                  CWWTPBoardGetBandSteeringRssi(&val32),
                                  CWWTPBoardSetBandSteeringRssi(val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_BANDSTREERING_PERCENT,
                                  CFG_MSG_INIT_DO_NOTHING,
                                  CWWTPBoardGetBandSteeringPercent(&val32),
                                  CWWTPBoardSetBandSteeringPercent(val32)
                                 )
/*bandstreer for wlan*/

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_WLAN_BANDSTREERING_MODE,
                                  CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                  CWWTPBoardGetWlanBandSteeringMode(radioIdx, wlanIdx, &val32),
                                  CWWTPBoardSetWlanBandSteeringMode(radioIdx, wlanIdx, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_WLAN_BANDSTREERING_PERCENT_ENABLE,
                                  CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                  CWWTPBoardGetWlanBandSteeringPercentEnable(radioIdx, wlanIdx, &val32),
                                  CWWTPBoardSetWlanBandSteeringPercentEnable(radioIdx, wlanIdx, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_WLAN_BANDSTREERING_RSSI_ENABLE,
                                  CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                  CWWTPBoardGetWlanBandSteeringRssiEnable(radioIdx, wlanIdx, &val32),
                                  CWWTPBoardSetWlanBandSteeringRssiEnable(radioIdx, wlanIdx, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_WLAN_BANDSTREERING_RSSI,
                                  CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                  CWWTPBoardGetWlanBandSteeringRssi(radioIdx, wlanIdx, &val32),
                                  CWWTPBoardSetWlanBandSteeringRssi(radioIdx, wlanIdx, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_WLAN_BANDSTREERING_PERCENT,
                                  CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                  CWWTPBoardGetWlanBandSteeringPercent(radioIdx, wlanIdx, &val32),
                                  CWWTPBoardSetWlanBandSteeringPercent(radioIdx, wlanIdx, val32)
                                 )

/*end*/
CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_FAST_HANDOVER_STATUS,
                                  CFG_MSG_INIT_DO_NOTHING,
                                  CWWTPBoardGetFastHandoverStatusCfg(&val32),
                                  CWWTPBoardSetFastHandoverStatusCfg(val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_FAST_HANDOVER_RSSI,
                                  CFG_MSG_INIT_DO_NOTHING,
                                  CWWTPBoardGetFastHandoverRssiCfg(&val32),
                                  CWWTPBoardSetFastHandoverRssiCfg(val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_WLAN_DOWNLOAD_LIMIT,
                                  CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                  CWWTPBoardGetDownloadLimitCfg(radioIdx, wlanIdx, &val32),
                                  CWWTPBoardSetDownloadLimitCfg(radioIdx, wlanIdx, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_WLAN_UPLOAD_LIMIT,
                                  CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                  CWWTPBoardGetUploadLimitCfg(radioIdx, wlanIdx, &val32),
                                  CWWTPBoardSetUploadLimitCfg(radioIdx, wlanIdx, val32)
                                 )
CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_WLAN_DOWNLOAD_PERUSER_ENABLE,
                                  CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                  CWWTPBoardGetDownloadPerUserEnableCfg(radioIdx, wlanIdx, &val32),
                                  CWWTPBoardSetDownloadPerUserEnableCfg(radioIdx, wlanIdx, val32)
                                 )
CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_WLAN_DOWNLOAD_LIMIT_MODE,
                                      CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                      CWWTPBoardGetDownloadModeCfg(radioIdx, wlanIdx, (CWRateMode*)&val32),
                                      CWWTPBoardSetDownloadModeCfg(radioIdx, wlanIdx, (CWRateMode)val32)
                                     )
CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_WLAN_UPLOAD_LIMIT_MODE,
                                      CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                      CWWTPBoardGetUploadModeCfg(radioIdx, wlanIdx,(CWRateMode*) &val32),
                                      CWWTPBoardSetUploadModeCfg(radioIdx, wlanIdx,(CWRateMode)val32)
                                     )


CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_WLAN_UPLOAD_PERUSER_ENABLE,
                                  CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                  CWWTPBoardGetUploadPerUserEnableCfg(radioIdx, wlanIdx, &val32),
                                  CWWTPBoardSetUploadPerUserEnableCfg(radioIdx, wlanIdx, val32)
                                 )



CFG_MSG_INFO_ENTRY_IMP_IPV4_ADDR_TYPE(WTP_CFG_AP_GUEST_NETWORK_IP,
                                      CFG_MSG_INIT_DO_NOTHING; ,
                                      CWWTPBoardGetGuestNetworkAddressCfg(&addr),
                                      CWWTPBoardSetGuestNetworkAddressCfg(addr)
                                     )

CFG_MSG_INFO_ENTRY_IMP_IPV4_ADDR_TYPE(WTP_CFG_AP_GUEST_NETWORK_MASK,
                                      CFG_MSG_INIT_DO_NOTHING; ,
                                      CWWTPBoardGetGuestNetworkMaskCfg(&addr),
                                      CWWTPBoardSetGuestNetworkMaskCfg(addr)
                                     )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_WLAN_ROAMING_ENABLE,
                                  CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                  CWWTPBoardGetRoamingEnableCfg(radioIdx, wlanIdx, &val32),
                                  CWWTPBoardSetRoamingEnableCfg(radioIdx, wlanIdx, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_WLAN_ROAMING_ADV_SEARCH,
                                  CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                  CWWTPBoardGetRoamingAdvSearchCfg(radioIdx, wlanIdx, &val32),
                                  CWWTPBoardSetRoamingAdvSearchCfg(radioIdx, wlanIdx, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_IPV4_ADDR_TYPE(WTP_CFG_AP_GUEST_NETWORK_DHCP_START,
                                      CFG_MSG_INIT_DO_NOTHING; ,
                                      CWWTPBoardGetGuestNetworkDhcpStartCfg(&addr),
                                      CWWTPBoardSetGuestNetworkDhcpStartCfg(addr)
                                     )

CFG_MSG_INFO_ENTRY_IMP_IPV4_ADDR_TYPE(WTP_CFG_AP_GUEST_NETWORK_DHCP_END,
                                      CFG_MSG_INIT_DO_NOTHING; ,
                                      CWWTPBoardGetGuestNetworkDhcpEndCfg(&addr),
                                      CWWTPBoardSetGuestNetworkDhcpEndCfg(addr)
                                     )

CFG_MSG_INFO_ENTRY_IMP_IPV4_ADDR_TYPE(WTP_CFG_AP_GUEST_NETWORK_WINS_SERVER,
                                      CFG_MSG_INIT_DO_NOTHING; ,
                                      CWWTPBoardGetGuestNetworkWinsServerCfg(&addr),
                                      CWWTPBoardSetGuestNetworkWinsServerCfg(addr)
                                     )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_RADIO_DISTANCE,
                                  CW_WTP_CFG_MSG_GET_RADIO_INDEX() ,
                                  CWWTPBoardGetRadioDistance(radioIdx, &val32),
                                  CWWTPBoardSetRadioDistance(radioIdx, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_ARRAY_TYPE(WTP_CFG_AP_PASSWORD_MD5, unsigned char,
                                  CFG_MSG_INIT_DO_NOTHING; ,
                                  (CWWTPBoardGetPasswordMD5Cfg(&array) &&(arraySize = 16) ? CW_TRUE : CW_FALSE),
                                  (arraySize == 16 ? (CWWTPBoardSetPasswordMD5Cfg(array) ? CW_TRUE : CW_FALSE) : CWErrorRaise(CW_ERROR_WRONG_ARG, NULL))
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_LED_POWER,
                                  CFG_MSG_INIT_DO_NOTHING,
                                  CWWTPBoardGetLedPowerCfg(&val32),
                                  CWWTPBoardSetLedPowerCfg(val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_LED_LAN,
                                  CFG_MSG_INIT_DO_NOTHING,
                                  CWWTPBoardGetLedLanCfg(&val32),
                                  CWWTPBoardSetLedLanCfg(val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_LED_WLAN0,
                                  CFG_MSG_INIT_DO_NOTHING,
                                  CWWTPBoardGetLedWlan0Cfg(&val32),
                                  CWWTPBoardSetLedWlan0Cfg(val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_LED_WLAN1,
                                  CFG_MSG_INIT_DO_NOTHING,
                                  CWWTPBoardGetLedWlan1Cfg(&val32),
                                  CWWTPBoardSetLedWlan1Cfg(val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_LED_WLAN2,
                                  CFG_MSG_INIT_DO_NOTHING,
                                  CWWTPBoardGetLedWlan2Cfg(&val32),
                                  CWWTPBoardSetLedWlan2Cfg(val32)
                                 )


CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_LED_MESH,
                                  CFG_MSG_INIT_DO_NOTHING,
                                  CWWTPBoardGetLedMeshCfg(&val32),
                                  CWWTPBoardSetLedMeshCfg(val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_RADIO_PORTAL_ENABLE,
                                  CW_WTP_CFG_MSG_GET_RADIO_INDEX(),
                                  CWWTPBoardGetPortalEnableCfg(radioIdx, &val32),
                                  CWWTPBoardSetPortalEnableCfg(radioIdx, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_RADIO_PORTAL_LOGIN_TYPE,
                                  CW_WTP_CFG_MSG_GET_RADIO_INDEX(),
                                  CWWTPBoardGetPortalLoginTypeCfg(radioIdx, &val32),
                                  CWWTPBoardSetPortalLoginTypeCfg(radioIdx, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_IPV4_ADDR_TYPE(WTP_CFG_AP_RADIO_PORTAL_RADIUS_SERVER,
                                      CW_WTP_CFG_MSG_GET_RADIO_INDEX() ,
                                      CWWTPBoardGetPortalRadiusCfg(radioIdx, &addr),
                                      CWWTPBoardSetPortalRadiusCfg(radioIdx, addr)
                                     )

CFG_MSG_INFO_ENTRY_IMP_STR_TYPE(WTP_CFG_AP_RADIO_PORTAL_REDIRECT_BEHAVIOR,
                                CW_WTP_CFG_MSG_GET_RADIO_INDEX(),
                                CWWTPBoardGetPortalRedirectCfg(radioIdx, &str),
                                CWWTPBoardSetPortalRedirectCfg(radioIdx, str)
                               )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_RADIO_PORTAL_WALLEDGARDEN,
                                  CW_WTP_CFG_MSG_GET_RADIO_INDEX(),
                                  CWWTPBoardGetPortalWalledGardenCfg(radioIdx, &val32),
                                  CWWTPBoardSetPortalWalledGardenCfg(radioIdx, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_STR_TYPE(WTP_CFG_AP_RADIO_PORTAL_WALLEDGARDEN_PAGE,
                                CW_WTP_CFG_MSG_GET_RADIO_INDEX(),
                                CWWTPBoardGetPortalWalledGardenPageCfg(radioIdx, &str),
                                CWWTPBoardSetPortalWalledGardenPageCfg(radioIdx, str)
                               )

CFG_MSG_INFO_ENTRY_IMP_VAL16_TYPE(WTP_CFG_AP_RADIO_PORTAL_RADIUS_PORT,
                                  CW_WTP_CFG_MSG_GET_RADIO_INDEX(),
                                  CWWTPBoardGetPortalRadiusPortCfg(radioIdx, (unsigned short *)&val16),
                                  CWWTPBoardSetPortalRadiusPortCfg(radioIdx, (unsigned short)val16)
                                 )

CFG_MSG_INFO_ENTRY_IMP_STR_TYPE(WTP_CFG_AP_RADIO_PORTAL_RADIUS_SECRET,
                                CW_WTP_CFG_MSG_GET_RADIO_INDEX(),
                                CWWTPBoardGetPortalRadiusSecretCfg(radioIdx, &str),
                                CWWTPBoardSetPortalRadiusSecretCfg(radioIdx, str)
                               )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_RADIO_PORTAL_SESSION_TIMEOUT,
                                  CW_WTP_CFG_MSG_GET_RADIO_INDEX(),
                                  CWWTPBoardGetPortalSessionTimeoutCfg(radioIdx, &val32),
                                  CWWTPBoardSetPortalSessionTimeoutCfg(radioIdx, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_RADIO_PORTAL_SETIMEOUT_ENABLE,
                                  CW_WTP_CFG_MSG_GET_RADIO_INDEX(),
                                  CWWTPBoardGetPortalSessionTimeoutEnableCfg(radioIdx, &val32),
                                  CWWTPBoardSetPortalSessionTimeoutEnableCfg(radioIdx, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_RADIO_PORTAL_IDLE_TIMEOUT,
                                  CW_WTP_CFG_MSG_GET_RADIO_INDEX(),
                                  CWWTPBoardGetPortalIdleTimeoutCfg(radioIdx, &val32),
                                  CWWTPBoardSetPortalIdleTimeoutCfg(radioIdx, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_RADIO_PORTAL_IDTIMEOUT_ENABLE,
                                  CW_WTP_CFG_MSG_GET_RADIO_INDEX(),
                                  CWWTPBoardGetPortalIdleTimeoutEnableCfg(radioIdx, &val32),
                                  CWWTPBoardSetPortalIdleTimeoutEnableCfg(radioIdx, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_RADIO_PORTAL_ACCOUNTING_ENABLE,
                                  CW_WTP_CFG_MSG_GET_RADIO_INDEX(),
                                  CWWTPBoardGetPortalAccountingEnableCfg(radioIdx, &val32),
                                  CWWTPBoardSetPortalAccountingEnableCfg(radioIdx, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_RADIO_PORTAL_ACCOUNTING_TIME,
                                  CW_WTP_CFG_MSG_GET_RADIO_INDEX(),
                                  CWWTPBoardGetPortalAccountingIntervalCfg(radioIdx, &val32),
                                  CWWTPBoardSetPortalAccountingIntervalCfg(radioIdx, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_RADIO_PORTAL_AUTH_TYPE,
                                  CW_WTP_CFG_MSG_GET_RADIO_INDEX(),
                                  CWWTPBoardGetPortalAuthTypeCfg(radioIdx, &val32),
                                  CWWTPBoardSetPortalAuthTypeCfg(radioIdx, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_STR_TYPE(WTP_CFG_AP_RADIO_PORTAL_EXTERNAL_SERVER,
                                CW_WTP_CFG_MSG_GET_RADIO_INDEX(),
                                CWWTPBoardGetPortalExternalServerCfg(radioIdx, &str),
                                CWWTPBoardSetPortalExternalServerCfg(radioIdx, str)
                               )

CFG_MSG_INFO_ENTRY_IMP_STR_TYPE(WTP_CFG_AP_RADIO_PORTAL_EXTERNAL_SECRET,
                                CW_WTP_CFG_MSG_GET_RADIO_INDEX(),
                                CWWTPBoardGetPortalExternalSecretCfg(radioIdx, &str),
                                CWWTPBoardSetPortalExternalSecretCfg(radioIdx, str)
                               )

CFG_MSG_INFO_ENTRY_IMP_IPV4_ADDR_TYPE(WTP_CFG_AP_RADIO_PORTAL_ACCOUNTING_SERVER,
                                      CW_WTP_CFG_MSG_GET_RADIO_INDEX() ,
                                      CWWTPBoardGetPortalAccountingServerCfg(radioIdx, &addr),
                                      CWWTPBoardSetPortalAccountingServerCfg(radioIdx, addr)
                                     )

CFG_MSG_INFO_ENTRY_IMP_VAL16_TYPE(WTP_CFG_AP_RADIO_PORTAL_ACCOUNTING_PORT,
                                  CW_WTP_CFG_MSG_GET_RADIO_INDEX(),
                                  CWWTPBoardGetPortalAccountingPortCfg(radioIdx, (unsigned short *)&val16),
                                  CWWTPBoardSetPortalAccountingPortCfg(radioIdx, (unsigned short)val16)
                                 )

CFG_MSG_INFO_ENTRY_IMP_STR_TYPE(WTP_CFG_AP_RADIO_PORTAL_ACCOUNTING_SECRET,
                                CW_WTP_CFG_MSG_GET_RADIO_INDEX(),
                                CWWTPBoardGetPortalAccountingSecretCfg(radioIdx, &str),
                                CWWTPBoardSetPortalAccountingSecretCfg(radioIdx, str)
                               )

CFG_MSG_INFO_ENTRY_IMP_STR_TYPE(WTP_CFG_AP_RADIO_PORTAL_UAMFORMAT,
                                CW_WTP_CFG_MSG_GET_RADIO_INDEX(),
                                CWWTPBoardGetPortalUamformatCfg(radioIdx, &str),
                                CWWTPBoardSetPortalUamformatCfg(radioIdx, str)
                               )

CFG_MSG_INFO_ENTRY_IMP_STR_TYPE(WTP_CFG_AP_RADIO_PORTAL_LOCAL_AUTH,
                                CW_WTP_CFG_MSG_GET_RADIO_INDEX(),
                                CWWTPBoardGetPortalLocalAuthCfg(radioIdx, &str),
                                CWWTPBoardSetPortalLocalAuthCfg(radioIdx, str)
                               )

CFG_MSG_INFO_ENTRY_IMP_VAL16_TYPE(WTP_CFG_AP_RADIO_PORTAL_PORT,
                                  CW_WTP_CFG_MSG_GET_RADIO_INDEX(),
                                  CWWTPBoardGetPortalPortCfg(radioIdx, (unsigned short *)&val16),
                                  CWWTPBoardSetPortalPortCfg(radioIdx, (unsigned short)val16)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_RADIO_PORTAL_HTTPS_ENABLE,
                                  CW_WTP_CFG_MSG_GET_RADIO_INDEX(),
                                  CWWTPBoardGetPortalHttpsEnableCfg(radioIdx, &val32),
                                  CWWTPBoardSetPortalHttpsEnableCfg(radioIdx, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_STR_TYPE(WTP_CFG_AP_RADIO_PORTAL_RADIUS_SECRET2,
                                CW_WTP_CFG_MSG_GET_RADIO_INDEX(),
                                CWWTPBoardGetPortalRadiusSecret2Cfg(radioIdx, &str),
                                CWWTPBoardSetPortalRadiusSecret2Cfg(radioIdx, str)
                               )

CFG_MSG_INFO_ENTRY_IMP_IPV4_ADDR_TYPE(WTP_CFG_AP_RADIO_PORTAL_RADIUS_SERVER2,
                                      CW_WTP_CFG_MSG_GET_RADIO_INDEX() ,
                                      CWWTPBoardGetPortalRadius2Cfg(radioIdx, &addr),
                                      CWWTPBoardSetPortalRadius2Cfg(radioIdx, addr)
                                     )

/*portal for wlan*/

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_WLAN_PORTAL_ENABLE,
                                  CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                  CWWTPBoardGetWlanPortalEnableCfg(radioIdx, wlanIdx, &val32),
                                  CWWTPBoardSetWlanPortalEnableCfg(radioIdx, wlanIdx, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_WLAN_PORTAL_LOGIN_TYPE,
                                  CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                  CWWTPBoardGetWlanPortalLoginTypeCfg(radioIdx, wlanIdx, &val32),
                                  CWWTPBoardSetWlanPortalLoginTypeCfg(radioIdx, wlanIdx, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_IPV4_ADDR_TYPE(WTP_CFG_AP_WLAN_PORTAL_RADIUS_SERVER,
                                      CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX() ,
                                      CWWTPBoardGetWlanPortalRadiusCfg(radioIdx, wlanIdx, &addr),
                                      CWWTPBoardSetWlanPortalRadiusCfg(radioIdx, wlanIdx, addr)
                                     )

CFG_MSG_INFO_ENTRY_IMP_STR_TYPE(WTP_CFG_AP_WLAN_PORTAL_REDIRECT_BEHAVIOR,
                                CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                CWWTPBoardGetWlanPortalRedirectCfg(radioIdx, wlanIdx, &str),
                                CWWTPBoardSetWlanPortalRedirectCfg(radioIdx, wlanIdx, str)
                               )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_WLAN_PORTAL_WALLEDGARDEN,
                                  CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                  CWWTPBoardGetWlanPortalWalledGardenCfg(radioIdx, wlanIdx, &val32),
                                  CWWTPBoardSetWlanPortalWalledGardenCfg(radioIdx, wlanIdx, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_STR_TYPE(WTP_CFG_AP_WLAN_PORTAL_WALLEDGARDEN_PAGE,
                                CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                CWWTPBoardGetWlanPortalWalledGardenPageCfg(radioIdx, wlanIdx, &str),
                                CWWTPBoardSetWlanPortalWalledGardenPageCfg(radioIdx, wlanIdx, str)
                               )

CFG_MSG_INFO_ENTRY_IMP_VAL16_TYPE(WTP_CFG_AP_WLAN_PORTAL_RADIUS_PORT,
                                  CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                  CWWTPBoardGetWlanPortalRadiusPortCfg(radioIdx, wlanIdx, (unsigned short *)&val16),
                                  CWWTPBoardSetWlanPortalRadiusPortCfg(radioIdx, wlanIdx, (unsigned short)val16)
                                 )

CFG_MSG_INFO_ENTRY_IMP_STR_TYPE(WTP_CFG_AP_WLAN_PORTAL_RADIUS_SECRET,
                                CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                CWWTPBoardGetWlanPortalRadiusSecretCfg(radioIdx, wlanIdx, &str),
                                CWWTPBoardSetWlanPortalRadiusSecretCfg(radioIdx, wlanIdx, str)
                               )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_WLAN_PORTAL_SESSION_TIMEOUT,
                                  CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                  CWWTPBoardGetWlanPortalSessionTimeoutCfg(radioIdx, wlanIdx, &val32),
                                  CWWTPBoardSetWlanPortalSessionTimeoutCfg(radioIdx, wlanIdx, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_WLAN_PORTAL_SETIMEOUT_ENABLE,
                                  CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                  CWWTPBoardGetWlanPortalSessionTimeoutEnableCfg(radioIdx, wlanIdx, &val32),
                                  CWWTPBoardSetWlanPortalSessionTimeoutEnableCfg(radioIdx, wlanIdx, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_WLAN_PORTAL_IDLE_TIMEOUT,
                                  CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                  CWWTPBoardGetWlanPortalIdleTimeoutCfg(radioIdx, wlanIdx, &val32),
                                  CWWTPBoardSetWlanPortalIdleTimeoutCfg(radioIdx, wlanIdx, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_WLAN_PORTAL_IDTIMEOUT_ENABLE,
                                  CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                  CWWTPBoardGetWlanPortalIdleTimeoutEnableCfg(radioIdx, wlanIdx, &val32),
                                  CWWTPBoardSetWlanPortalIdleTimeoutEnableCfg(radioIdx, wlanIdx, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_WLAN_PORTAL_ACCOUNTING_ENABLE,
                                  CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                  CWWTPBoardGetWlanPortalAccountingEnableCfg(radioIdx, wlanIdx, &val32),
                                  CWWTPBoardSetWlanPortalAccountingEnableCfg(radioIdx, wlanIdx,val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_WLAN_PORTAL_ACCOUNTING_TIME,
                                  CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                  CWWTPBoardGetWlanPortalAccountingIntervalCfg(radioIdx, wlanIdx, &val32),
                                  CWWTPBoardSetWlanPortalAccountingIntervalCfg(radioIdx, wlanIdx, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_WLAN_PORTAL_AUTH_TYPE,
                                  CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                  CWWTPBoardGetWlanPortalAuthTypeCfg(radioIdx, wlanIdx, &val32),
                                  CWWTPBoardSetWlanPortalAuthTypeCfg(radioIdx, wlanIdx, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_STR_TYPE(WTP_CFG_AP_WLAN_PORTAL_EXTERNAL_SERVER,
                                CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                CWWTPBoardGetWlanPortalExternalServerCfg(radioIdx, wlanIdx, &str),
                                CWWTPBoardSetWlanPortalExternalServerCfg(radioIdx, wlanIdx, str)
                               )

CFG_MSG_INFO_ENTRY_IMP_STR_TYPE(WTP_CFG_AP_WLAN_PORTAL_EXTERNAL_SECRET,
                                CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                CWWTPBoardGetWlanPortalExternalSecretCfg(radioIdx, wlanIdx, &str),
                                CWWTPBoardSetWlanPortalExternalSecretCfg(radioIdx, wlanIdx, str)
                               )

CFG_MSG_INFO_ENTRY_IMP_IPV4_ADDR_TYPE(WTP_CFG_AP_WLAN_PORTAL_ACCOUNTING_SERVER,
                                      CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX() ,
                                      CWWTPBoardGetWlanPortalAccountingServerCfg(radioIdx, wlanIdx, &addr),
                                      CWWTPBoardSetWlanPortalAccountingServerCfg(radioIdx, wlanIdx, addr)
                                     )

CFG_MSG_INFO_ENTRY_IMP_VAL16_TYPE(WTP_CFG_AP_WLAN_PORTAL_ACCOUNTING_PORT,
                                  CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                  CWWTPBoardGetWlanPortalAccountingPortCfg(radioIdx, wlanIdx, (unsigned short *)&val16),
                                  CWWTPBoardSetWlanPortalAccountingPortCfg(radioIdx, wlanIdx, (unsigned short)val16)
                                 )

CFG_MSG_INFO_ENTRY_IMP_STR_TYPE(WTP_CFG_AP_WLAN_PORTAL_ACCOUNTING_SECRET,
                                CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                CWWTPBoardGetWlanPortalAccountingSecretCfg(radioIdx, wlanIdx, &str),
                                CWWTPBoardSetWlanPortalAccountingSecretCfg(radioIdx, wlanIdx, str)
                               )
CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_WLAN_PORTAL_NETRORK_MODE,
                                      CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                      CWWTPBoardGetWlanPortalNetworkTypeCfg(radioIdx, wlanIdx, &val32),
                                      CWWTPBoardSetWlanPortalNetworkTypeCfg(radioIdx, wlanIdx, val32)
                                     )


CFG_MSG_INFO_ENTRY_IMP_STR_TYPE(WTP_CFG_AP_WLAN_PORTAL_UAMFORMAT,
                                CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                CWWTPBoardGetWlanPortalUamformatCfg(radioIdx, wlanIdx, &str),
                                CWWTPBoardSetWlanPortalUamformatCfg(radioIdx, wlanIdx, str)
                               )

CFG_MSG_INFO_ENTRY_IMP_STR_TYPE(WTP_CFG_AP_WLAN_PORTAL_LOCAL_AUTH,
                                CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                CWWTPBoardGetWlanPortalLocalAuthCfg(radioIdx, wlanIdx, &str),
                                CWWTPBoardSetWlanPortalLocalAuthCfg(radioIdx, wlanIdx, str)
                               )

CFG_MSG_INFO_ENTRY_IMP_VAL16_TYPE(WTP_CFG_AP_WLAN_PORTAL_PORT,
                                  CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                  CWWTPBoardGetWlanPortalPortCfg(radioIdx, wlanIdx, (unsigned short *)&val16),
                                  CWWTPBoardSetWlanPortalPortCfg(radioIdx, wlanIdx, (unsigned short)val16)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_WLAN_PORTAL_HTTPS_ENABLE,
                                  CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                  CWWTPBoardGetWlanPortalHttpsEnableCfg(radioIdx, wlanIdx, &val32),
                                  CWWTPBoardSetWlanPortalHttpsEnableCfg(radioIdx, wlanIdx, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_STR_TYPE(WTP_CFG_AP_WLAN_PORTAL_RADIUS_SECRET2,
                                CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                CWWTPBoardGetWlanPortalRadiusSecret2Cfg(radioIdx, wlanIdx, &str),
                                CWWTPBoardSetWlanPortalRadiusSecret2Cfg(radioIdx, wlanIdx, str)
                               )

CFG_MSG_INFO_ENTRY_IMP_IPV4_ADDR_TYPE(WTP_CFG_AP_WLAN_PORTAL_RADIUS_SERVER2,
                                      CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX() ,
                                      CWWTPBoardGetWlanPortalRadius2Cfg(radioIdx, wlanIdx, &addr),
                                      CWWTPBoardSetWlanPortalRadius2Cfg(radioIdx, wlanIdx, addr)
                                     )





/*end*/
CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_RADIO_FAST_HANDOVER_STATUS,
                                  CW_WTP_CFG_MSG_GET_RADIO_INDEX() ,
                                  CWWTPBoardGetRadioFastHandoverStatusCfg(radioIdx, &val32),
                                  CWWTPBoardSetRadioFastHandoverStatusCfg(radioIdx, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_RADIO_FAST_HANDOVER_RSSI,
                                  CW_WTP_CFG_MSG_GET_RADIO_INDEX() ,
                                  CWWTPBoardGetRadioFastHandoverRssiCfg(radioIdx, &val32),
                                  CWWTPBoardSetRadioFastHandoverRssiCfg(radioIdx, val32)
                                 )


CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_MANAGEMENT_VLAN_ID,
                                  CFG_MSG_INIT_DO_NOTHING,
                                  CWWTPBoardGetManageVlanCfg(&val32),
                                  CWWTPBoardSetManageVlanCfg(val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_ARRAY_TYPE(WTP_CFG_AP_AC_LIST, CWHostName,
                                  CFG_MSG_INIT_DO_NOTHING,
                                  CWWTPBoardGetAcListCfg(&arraySize, &array),
                                  CWWTPBoardSetAcListCfg(arraySize, array)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_LAN_PORT_NUM,
                                  CFG_MSG_INIT_DO_NOTHING,
                                  (val32 = CWWTPBoardGetMaxLanPortNum()) >= 0 ? CW_TRUE : CWErrorRaise(CW_ERROR_WRONG_ARG, NULL),
                                  val32 == CWWTPBoardGetMaxLanPortNum() ? CW_TRUE : CWErrorRaise(CW_ERROR_WRONG_ARG, NULL)
                                 )


CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_LAN_PORT_ENABLE,
                                  CW_WTP_CFG_MSG_GET_LAN_PORT_INDEX(),
                                  CWWTPBoardGetLanPortEnableCfg(port, &val32),
                                  CWWTPBoardSetLanPortEnableCfg(port, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_LAN_PORT_VLAN_ID,
                                  CW_WTP_CFG_MSG_GET_LAN_PORT_INDEX(),
                                  CWWTPBoardGetLanPortVlanIdCfg(port, &val32),
                                  CWWTPBoardSetLanPortVlanIdCfg(port, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_LAN_PORT_VLAN_MODE,
                                  CW_WTP_CFG_MSG_GET_LAN_PORT_INDEX(),
                                  CWWTPBoardGetLanPortVlanModeCfg(port, &val32),
                                  CWWTPBoardSetLanPortVlanModeCfg(port, val32)
                                 )


CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_WLAN_NAS_ID_ENABLE,
                                  CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                  CWWTPBoardGetWlanNasIdEnableCfg(radioIdx, wlanIdx, &val32),
                                  CWWTPBoardSetWlanNasIdEnableCfg(radioIdx, wlanIdx, val32)
                                 )
CFG_MSG_INFO_ENTRY_IMP_STR_TYPE(WTP_CFG_AP_WLAN_NAS_ID,
                                  CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                  CWWTPBoardGetWlanNasIdCfg(radioIdx, wlanIdx, &str),
                                  CWWTPBoardSetWlanNasIdCfg(radioIdx, wlanIdx, str)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_WLAN_NAS_IP_ENABLE,
                                  CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                  CWWTPBoardGetWlanNasIPEnableCfg(radioIdx, wlanIdx, &val32),
                                  CWWTPBoardSetWlanNasIPEnableCfg(radioIdx, wlanIdx, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_IPV4_ADDR_TYPE(WTP_CFG_AP_WLAN_NAS_IP,
                                      CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                      CWWTPBoardGetWlanNasIPCfg(radioIdx, wlanIdx, &addr),
                                      CWWTPBoardSetWlanNasIPCfg(radioIdx, wlanIdx, addr)
                                     )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_WLAN_NAS_PORT_ENABLE,
                                  CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                  CWWTPBoardGetWlanNasPortEnableCfg(radioIdx, wlanIdx, &val32),
                                  CWWTPBoardSetWlanNasPortEnableCfg(radioIdx, wlanIdx, val32)
                                 )
CFG_MSG_INFO_ENTRY_IMP_VAL16_TYPE(WTP_CFG_AP_WLAN_NAS_PORT,
                                  CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                  CWWTPBoardGetWlanNasPortCfg(radioIdx, wlanIdx, (unsigned short *)&val16),
                                  CWWTPBoardSetWlanNasPortCfg(radioIdx, wlanIdx, (unsigned short)val16)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_MESH_ENABLE_TOTAL,
                                  CFG_MSG_INIT_DO_NOTHING ,
                                  CWWTPBoardGetMeshEnableTotalCfg(&val32),
                                  CWWTPBoardSetMeshEnableTotalCfg(val32)
                                 )
CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_MESH_ENABLE,
                                  CW_WTP_CFG_MSG_GET_RADIO_INDEX(),
                                  CWWTPBoardGetRadioMeshEnableCfg(radioIdx, &val32),
                                  CWWTPBoardSetRadioMeshEnableCfg(radioIdx, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_STR_TYPE(WTP_CFG_AP_MESH_ID,
                                  CW_WTP_CFG_MSG_GET_RADIO_INDEX(),
                                  CWWTPBoardGetRadioMeshIDCfg(radioIdx, &str),
                                  CWWTPBoardSetRadioMeshIDCfg(radioIdx, str)
                                 )

CFG_MSG_INFO_ENTRY_IMP_STR_TYPE(WTP_CFG_AP_MESH_WPA_KEY,
                                  CW_WTP_CFG_MSG_GET_RADIO_INDEX(),
                                  CWWTPBoardGetRadioMeshWPAKeyCfg(radioIdx, &str),
                                  CWWTPBoardSetRadioMeshWPAKeyCfg(radioIdx, str)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL16_TYPE(WTP_CFG_AP_MESH_LINK_ROBUST_THRESHOLD,
                                  CW_WTP_CFG_MSG_GET_RADIO_INDEX(),
                                  CWWTPBoardGetRadioMeshLinkRobustThresholdCfg(radioIdx, &val16),
                                  CWWTPBoardSetRadioMeshLinkRobustThresholdCfg(radioIdx, val16)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_LOG_REMOTE_SERVER_ENABLE,
                                  CFG_MSG_INIT_DO_NOTHING,
                                  CWWTPBoardGetLogRemoteEnable(&val32),
                                  CWWTPBoardSetLogRemoteEnable(val32)
                                 )


CFG_MSG_INFO_ENTRY_IMP_STR_TYPE(WTP_CFG_AP_LOG_REMOTE_SERVER_CONFIG,
                                CFG_MSG_INIT_DO_NOTHING,
                                CWWTPBoardGetLogRemoteCfg(&str),
                                CWWTPBoardSetLogRemoteCfg(str)
                               )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_TIME_ZONE,
                                  CFG_MSG_INIT_DO_NOTHING,
                                  CWWTPBoardGetTimeZone(&val32),
                                  CWWTPBoardSetTimeZone(val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_AP_LOG_TRAFFIC_ENABLE,
                                  CFG_MSG_INIT_DO_NOTHING,
                                  CWWTPBoardGetLogTrafficEnable(&val32),
                                  CWWTPBoardSetLogTrafficEnable(val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_STR_TYPE(WTP_CFG_AP_WLAN_HOTSPOT20_JSON,
                                CW_WTP_CFG_MSG_GET_RADIO_WLAN_INDEX(),
                                CWWTPBoardGetWlanHotspot20Josn(radioIdx, wlanIdx, &str),
                                CWWTPBoardSetWlanHotspot20Josn(radioIdx, wlanIdx, str)
                               )



CWWtpCfgMsgInfo gCfgMsgInfo[] = CW_CFG_MSG_AP_INFO;
#define WTP_CFG_END   WTP_CFG_AP_END

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#elif defined(CW_WTP_SWITCH)

CWErrorCode CWCfgMsgGetPortNextKey(int keyLen, void *keyPtr, int *nextkeyLen, void **nextkeyPtr)
{
    CW_WTP_CFG_MSG_GETNEXT_PORT_INDEX();
}

CWErrorCode CWCfgMsgGetPhysicPortNextKey(int keyLen, void *keyPtr, int *nextkeyLen, void **nextkeyPtr)
{
    CW_WTP_CFG_MSG_GETNEXT_PHYSIC_PORT_INDEX();
}

CWErrorCode CWCfgMsgGetPoePortNextKey(int keyLen, void *keyPtr, int *nextkeyLen, void **nextkeyPtr)
{
    CW_WTP_CFG_MSG_GETNEXT_POE_PORT_INDEX();
}

CFG_MSG_INFO_ENTRY_IMP_STR_TYPE(WTP_CFG_SW_NAME,
                                CFG_MSG_INIT_DO_NOTHING,
                                CWWTPBoardGetNameCfg(&str),
                                CWWTPBoardSetNameCfg(str)
                               )

CFG_MSG_INFO_ENTRY_IMP_STR_TYPE(WTP_CFG_SW_LOCATION,
                                CFG_MSG_INIT_DO_NOTHING,
                                CWWTPBoardGetLocationCfg(&str),
                                CWWTPBoardSetLocationCfg(str)
                               )

CFG_MSG_INFO_ENTRY_IMP_OBJECT_TYPE(WTP_CFG_SW_IPV4, CWIPv4Cfg,
                                   CFG_MSG_INIT_DO_NOTHING,
                                   CWWTPBoardGetIPv4Cfg(objPtr),
                                   CWWTPBoardSetIPv4Cfg(objPtr)
                                  )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_SW_REDUDNANCY,
                                  CFG_MSG_INIT_DO_NOTHING,
                                  CWWTPBoardGetRedundantManageEnable(&val32),
                                  CWWTPBoardSetRedundantManageEnable(val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_ARRAY_TYPE(WTP_CFG_SW_REDUNDANT_MANAGED_MAC_LIST, CWMacAddress,
                                  CFG_MSG_INIT_DO_NOTHING,
                                  CWWTPBoardGetRedundantManagedMacList(&arraySize, &array),
                                  CWWTPBoardSetRedundantManagedMacList(arraySize, array)
                                 )

CFG_MSG_INFO_ENTRY_IMP_IPV4_ADDR_TYPE(WTP_CFG_SW_DNS1,
                                      CFG_MSG_INIT_DO_NOTHING,
                                      CWWTPBoardGetDns1Cfg(&addr),
                                      CWWTPBoardSetDns1Cfg(addr)
                                     )


CFG_MSG_INFO_ENTRY_IMP_IPV4_ADDR_TYPE(WTP_CFG_SW_DNS2,
                                      CFG_MSG_INIT_DO_NOTHING,
                                      CWWTPBoardGetDns2Cfg(&addr),
                                      CWWTPBoardSetDns2Cfg(addr)
                                     )

CFG_MSG_INFO_ENTRY_IMP_STR_TYPE(WTP_CFG_SW_ADMIN,
                                CFG_MSG_INIT_DO_NOTHING; ,
                                CWWTPBoardGetAdminCfg(&str),
                                CWWTPBoardSetAdminCfg(str)
                               )

CFG_MSG_INFO_ENTRY_IMP_STR_TYPE(WTP_CFG_SW_PASSWORD,
                                CFG_MSG_INIT_DO_NOTHING; ,
                                CWWTPBoardGetPasswordCfg(&str),
                                CWWTPBoardSetPasswordCfg(str)
                               )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_SW_PORT_NUM,
                                  CFG_MSG_INIT_DO_NOTHING,
                                  (val32 = CWWTPSwitchGetMaxLogicPortNum()) > 0 ? CW_TRUE : CWErrorRaise(CW_ERROR_WRONG_ARG, NULL),
                                  val32 == CWWTPSwitchGetMaxLogicPortNum() ? CW_TRUE : CWErrorRaise(CW_ERROR_WRONG_ARG, NULL)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_SW_PORT_NO,
                                  CW_WTP_CFG_MSG_GET_PORT_INDEX(),
                                  (val32 = CWWTPSwitchGetPortNo(port)) > 0 ? CW_TRUE : CWErrorRaise(CW_ERROR_WRONG_ARG, NULL),
                                  val32 == CWWTPSwitchGetPortNo(port) ? CW_TRUE : CWErrorRaise(CW_ERROR_WRONG_ARG, NULL)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_SW_PORT_SPEED_MODE,
                                  CW_WTP_CFG_MSG_GET_PORT_INDEX(),
                                  CWWTPSwitchGetPortSpeedCfg(port, &val32),
                                  CWWTPSwitchSetPortSpeedCfg(port, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_SW_PORT_FLOWCTL,
                                  CW_WTP_CFG_MSG_GET_PORT_INDEX(),
                                  CWWTPSwitchGetPortFlowCtlCfg(port, &val32),
                                  CWWTPSwitchSetPortFlowCtlCfg(port, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_STR_TYPE(WTP_CFG_SW_PORT_DESCRIPTION,
                                CW_WTP_CFG_MSG_GET_PORT_INDEX(),
                                CWWTPSwitchGetPortDescpCfg(port, &str),
                                CWWTPSwitchSetPortDescpCfg(port, str)
                               )


CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_SW_POE_POWER_BUDGET,
                                  CFG_MSG_INIT_DO_NOTHING,
                                  CWWTPSwitchGetPoePowerBudgetCfg(&val32),
                                  CWWTPSwitchSetPoePowerBudgetCfg(val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_SW_POE_PORT_NUM,
                                  CFG_MSG_INIT_DO_NOTHING,
                                  (val32 = CWWTPSwitchGetMaxPoePortNum()) >= 0 ? CW_TRUE : CWErrorRaise(CW_ERROR_WRONG_ARG, NULL),
                                  val32 == CWWTPSwitchGetMaxPoePortNum() ? CW_TRUE : CWErrorRaise(CW_ERROR_WRONG_ARG, NULL)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_SW_POE_PORT_NO,
                                  CW_WTP_CFG_MSG_GET_POE_PORT_INDEX(),
                                  (val32 = CWWTPSwitchGetPoePortNo(port)) >= 0 ? CW_TRUE : CWErrorRaise(CW_ERROR_WRONG_ARG, NULL),
                                  val32 == CWWTPSwitchGetPoePortNo(port) ? CW_TRUE : CWErrorRaise(CW_ERROR_WRONG_ARG, NULL)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_SW_POE_PORT_ENABLE,
                                  CW_WTP_CFG_MSG_GET_POE_PORT_INDEX(),
                                  CWWTPSwitchGetPoePortEnableCfg(port, &val32),
                                  CWWTPSwitchSetPoePortEnableCfg(port, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_SW_POE_PORT_PRIORITY,
                                  CW_WTP_CFG_MSG_GET_POE_PORT_INDEX(),
                                  CWWTPSwitchGetPoePortPriorityCfg(port, &val32),
                                  CWWTPSwitchSetPoePortPriorityCfg(port, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_SW_POE_PORT_POWER_LIMIT_TYPE,
                                  CW_WTP_CFG_MSG_GET_POE_PORT_INDEX(),
                                  CWWTPSwitchGetPoePortPowerLimitTypeCfg(port, &val32),
                                  CWWTPSwitchSetPoePortPowerLimitTypeCfg(port, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_VAL32_TYPE(WTP_CFG_SW_POE_PORT_POWER_LIMIT,
                                  CW_WTP_CFG_MSG_GET_POE_PORT_INDEX(),
                                  CWWTPSwitchGetPoePortPowerLimitCfg(port, &val32),
                                  CWWTPSwitchSetPoePortPowerLimitCfg(port, val32)
                                 )

CFG_MSG_INFO_ENTRY_IMP_ARRAY_TYPE(WTP_CFG_SW_AC_LIST, CWHostName,
                                  CFG_MSG_INIT_DO_NOTHING,
                                  CWWTPBoardGetAcListCfg(&arraySize, &array),
                                  CWWTPBoardSetAcListCfg(arraySize, array)
                                 )

CWWtpCfgMsgInfo gCfgMsgInfo[] = CW_CFG_MSG_SW_INFO;
#define WTP_CFG_END      WTP_CFG_SW_END
#endif /* CW_WTP_SWITCH */

/*check ac capbility*/
CWBool CWWTPCheckAcCap(int type)
{
       int infoIndex=0;
       int infoCount=0;   

    
       /*check capility*/
       if(!gACInfoPtr->cfgCapInfo)
       {
         /*Ac capability is not supported before version c1.9.29 */
         CWDebugLog("cfgCapInfo is NULL");
         if(type > WTP_CFG_AP_RADIO_BIT_RATE)
         {
           return CW_FALSE;
         }
         else
         {
            return CW_TRUE;
         }
         
       }
       else
       {
          if( gACInfoPtr->cfgCapInfo->count == 0)
          {
             /*Ac capability is not supported before version c1.9.29 */
             CWDebugLog("Not support ac capability");
             return CW_FALSE;
          }
          infoCount=gACInfoPtr->cfgCapInfo->count ;
       }
       
       for(infoIndex =0 ; infoIndex < infoCount; infoIndex++)
       {         
         if(CW_WTP_CFG_CAP_CHECK(gACInfoPtr->cfgCapInfo->cfgCap[infoIndex], type))
         {             
             return CW_TRUE;
         }
         else
         {
            return CW_FALSE;  
         }
          
       }
       return CW_FALSE;
}


CWBool CWWTPGetWtpCfg(CWWtpCfgMsgList *cfgList)
{
    unsigned short cfgType;
    int preKeyLen, keyLen, valLen;
    void *preKeyPtr, *keyPtr, *valPtr;
    CWErrorCode ret;

    if(cfgList == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    CWDebugLog("Retrieve WTP configuration start");

    for(cfgType = WTP_CFG_START; cfgType < WTP_CFG_END; cfgType++)
    {
        /* DEBUG ONLY */
        if(gCfgMsgInfo[cfgType].type != cfgType)
        {
            CWLog("!!!!!!!!!!! gCfgMsgInfo[%u].type %s != %u !!!!!!!!!!!!!",
                  cfgType, gCfgMsgInfo[cfgType].name, gCfgMsgInfo[cfgType].type);
            exit(EXIT_FAILURE);
        }

        /*if type is not supported in AC, Don't sent cofiguration to AC*/
        if(!CWWTPCheckAcCap(cfgType))
        {
           CWDebugLog("%s is not supported in AC, ingore it",gCfgMsgInfo[cfgType].name);
           continue;
        }
       

        if(gCfgMsgInfo[cfgType].getNextKey)
        {
            preKeyLen = 0;
            preKeyPtr = NULL;
            valLen = 0;
            valPtr = NULL;
            do
            {
                //CWDebugLog("CWWTPGetWtpCfg getNextKey %s", gCfgMsgInfo[cfgType].name);
                ret = gCfgMsgInfo[cfgType].getNextKey(preKeyLen, preKeyPtr, &keyLen, &keyPtr);
                CW_FREE_OBJECT(preKeyPtr);
                if(ret == CW_ERROR_NONE)
                {
                    break;
                }
                if(ret != CW_ERROR_SUCCESS)
                {
                    CWWtpCfgMsgListFree(cfgList);
                    return CW_FALSE;
                }
                //CWDebugLog("CWWTPGetWtpCfg getMsg %s", gCfgMsgInfo[cfgType].name);
                ret = gCfgMsgInfo[cfgType].getMsg(keyLen, keyPtr, &valLen, &valPtr);
                if(ret == CW_ERROR_SUCCESS)
                {
                    CW_CREATE_OBJECT_SIZE_ERR(preKeyPtr, keyLen,
                    {
                        CW_FREE_OBJECT(valPtr);
                        CWWtpCfgMsgListFree(cfgList);
                        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
                    });
                    CW_COPY_MEMORY(preKeyPtr, keyPtr, keyLen);

                    if(!CWWtpCfgMsgListAdd(cfgList, cfgType, keyLen, preKeyPtr, valLen, valPtr))
                    {
                        CW_FREE_OBJECT(keyPtr);
                        CW_FREE_OBJECT(valPtr);
                        CWWtpCfgMsgListFree(cfgList);
                        return CW_FALSE;
                    }
                }
                else if(ret != CW_ERROR_NOT_SUPPORTED)
                {
                    CW_FREE_OBJECT(keyPtr);
                    CWWtpCfgMsgListFree(cfgList);
                    return CW_FALSE;
                }

                preKeyLen = keyLen;
                preKeyPtr = keyPtr;
            }
            while(1);
        }
        else
        {
            //CWDebugLog("CWWTPGetWtpCfg getMsg %s", gCfgMsgInfo[cfgType].name);
            ret = gCfgMsgInfo[cfgType].getMsg(0, NULL, &valLen, &valPtr);
            if(ret == CW_ERROR_SUCCESS)
            {
                if(!CWWtpCfgMsgListAdd(cfgList, cfgType, 0, NULL, valLen, valPtr))
                {
                    CW_FREE_OBJECT(valPtr);
                    CWWtpCfgMsgListFree(cfgList);
                    return CW_FALSE;
                }
            }
            else if(ret != CW_ERROR_NOT_SUPPORTED)
            {
                CWWtpCfgMsgListFree(cfgList);
                return CW_FALSE;
            }
        }
    }

    CWDebugLog("Retrieve WTP configuration end");

    return CW_TRUE;
}

CWBool CWWTPAssembleMsgElemVendorPayloadWtpCfg(CWProtocolMessage *msgPtr)
{
    CWWtpCfgMsgList cfgList = {0, NULL, NULL};

    CWDebugLog("CWAssembleMsgElemVendorPayloadWtpCfg called");

    if(msgPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    if(!CWWTPGetWtpCfg(&cfgList))
    {
        return CW_FALSE;
    }

    if(!CWAssembleMsgElemVendorPayloadWtpCfg(msgPtr, &cfgList))
    {
        CWWtpCfgMsgListFree(&cfgList);
        return CW_FALSE;
    }

    CWWtpCfgMsgListFree(&cfgList);

    CWDebugLog("CWAssembleMsgElemVendorPayloadWtpCfg...exit");

    return CW_TRUE;
}

CWBool CWWTPSaveWtpCfg(CWWtpCfgMsgList *cfgList)
{
    CWWtpCfgMsgNode *cfgNode;
    CWErrorCode ret;
    int valLen;
    void *keyPtr, *valPtr;
    char tmpMsg[256];

    CWDebugLog("CWWTPSaveWtpCfg called");

    if(cfgList == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    gWTPResultCode = CW_PROTOCOL_SUCCESS;
    gWTPCfgResult.apply = CW_FALSE;
    gWTPCfgResult.handle = WTP_CFG_ERROR_HANDLE_NONE;
    gWTPCfgResult.rejoin = CW_FALSE;
    gWTPCfgResult.waitSec = 0;
    gWTPCfgResult.message[0] = '\0';

    /* Clear the rollback list */
    CW_ASSERT(gWTPCfgRollbackList.head == NULL);

    cfgNode = cfgList->head;
    while(cfgNode)
    {
        if(cfgNode->type >= WTP_CFG_END)
        {
            CWDebugLog("CWWTPSaveWtpCfg id %u is unsupported", cfgNode->type);
            cfgNode = cfgNode->next;
            continue;
        }

        CWDebugLog("CWWTPSaveWtpCfg get %s", gCfgMsgInfo[cfgNode->type].name);

        /* Find the difference wtp needs to apply */
        ret = gCfgMsgInfo[cfgNode->type].getMsg(cfgNode->keyLen, cfgNode->keyPtr, &valLen, &valPtr);
        if(ret == CW_ERROR_SUCCESS)
        {
            if(valLen != cfgNode->valLen || memcmp(valPtr, cfgNode->valPtr, valLen))
            {
                CWDebugLog("CWWTPSaveWtpCfg set %s", gCfgMsgInfo[cfgNode->type].name);
                ret = gCfgMsgInfo[cfgNode->type].setMsg(cfgNode->keyLen, cfgNode->keyPtr,
                                                        cfgNode->valLen, cfgNode->valPtr);
                if(ret != CW_ERROR_SUCCESS)
                {
                    CWLog("CWWTPSaveWtpCfg set %s failed ret %s msg %s",
                          gCfgMsgInfo[cfgNode->type].name, CWErrorCodeString(ret),
                          CWErrorGetLastErrorMsg() ? CWErrorGetLastErrorMsg() : "");

                    sprintf(tmpMsg, "Failed at %s error %s msg %s\n",
                            gCfgMsgInfo[cfgNode->type].name,
                            CWErrorCodeString(ret),
                            CWErrorGetLastErrorMsg() ? CWErrorGetLastErrorMsg() : "");

                    if(sizeof(gWTPCfgResult.message) - strlen(gWTPCfgResult.message) > strlen(tmpMsg))
                    {
                        strcat(gWTPCfgResult.message, tmpMsg);
                    }
                    else
                    {
                        CWLog("CWWTPSaveWtpCfg message buffer is not enough");
                    }

                    if(cfgNode->keyLen)
                    {
                        CW_CREATE_OBJECT_SIZE_ERR(keyPtr, cfgNode->keyLen,
                        {
                            CWWtpCfgMsgListFree(&gWTPCfgRollbackList);
                            CWWTPBoardCancelCfg();
                            gWTPResultCode = CW_PROTOCOL_FAILURE;
                            return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
                        });
                        CW_COPY_MEMORY(keyPtr, cfgNode->keyPtr, cfgNode->keyLen);
                    }
                    else
                    {
                        keyPtr = NULL;
                    }

                    if(!CWWtpCfgMsgListAdd(&gWTPCfgRollbackList, cfgNode->type, cfgNode->keyLen,
                                           keyPtr, valLen, valPtr))
                    {
                        CWWtpCfgMsgListFree(&gWTPCfgRollbackList);
                        CWWTPBoardCancelCfg();
                        gWTPResultCode = CW_PROTOCOL_FAILURE;
                        return CW_FALSE;
                    }
                }
                else
                {
                    gWTPCfgResult.apply = CW_TRUE; /* we have at least 1 cfg changed */
                    CW_FREE_OBJECT(valPtr);
                }
            }
            else
            {
                CW_FREE_OBJECT(valPtr);
            }
        }
        else
        {
            CWLog("CWWTPSaveWtpCfg get %s failed ret %s keyLen %u keyPtr 0x%x",
                  gCfgMsgInfo[cfgNode->type].name, CWErrorCodeString(ret), cfgNode->keyLen, cfgNode->keyPtr);
        }

        cfgNode = cfgNode->next;
    }

    if(gWTPResultCode == CW_PROTOCOL_SUCCESS)
    {
        if(gWTPCfgResult.apply)
        {
            gWTPCfgResult.waitSec = CWWTPBoardGetApplyCfgTime(&(gWTPCfgResult.rejoin));
            /* APPLY after reponse sent */
        }
    }
    else
    {
        CWWTPBoardCancelCfg();
    }

    return CW_TRUE;
}

CWBool CWWTPAssembleMsgElemVendorPayloadCurrentCfg(CWProtocolMessage *msgPtr)
{
    int radioIdx;
    int wlanIdx;
    int chanlistIdx;
    int totalWlanCount = 0;
    int totalChanCount = 0;
    int len;
    CWWTPCurrentCfgInfo curCfg;

    CW_ZERO_MEMORY(&curCfg, sizeof(curCfg));

    if(!CWWTPBoardGetCurrentIPv4(&curCfg.ipv4) ||
       !CWWTPBoardGetCurrentDns(&curCfg.dns1, &curCfg.dns2))
    {
        return CW_FALSE;
    }

#ifdef CW_WTP_AP
    curCfg.radioCount = CWWTPBoardGetMaxRadio();

    if(curCfg.radioCount != 0)
    {
        CW_CREATE_ZERO_ARRAY_ERR(curCfg.radio, curCfg.radioCount, CWWTPCurrentRadioCfgInfo,
        {
            return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
        });
    }

    for(radioIdx = 0; radioIdx < curCfg.radioCount; radioIdx++)
    {
        if(!CWWTPBoardGetRadioCurrentChannel(radioIdx, &curCfg.radio[radioIdx].channel) ||
           !CWWTPBoardGetRadioCurrentTxPower(radioIdx, &curCfg.radio[radioIdx].txPower))
        {
            goto error_exit;
        }

        curCfg.radio[radioIdx].wlanCount = CWWTPBoardGetMaxRadioWlans(radioIdx);
        CW_CREATE_ZERO_ARRAY_ERR(curCfg.radio[radioIdx].wlan, curCfg.radio[radioIdx].wlanCount, CWWTPCurrentWlanCfgInfo,
        {
            CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
            goto error_exit;
        });

        for(wlanIdx = 0; wlanIdx < curCfg.radio[radioIdx].wlanCount; wlanIdx++)
        {
            if(!CWWTPBoardGetWlanBssid(radioIdx, wlanIdx, curCfg.radio[radioIdx].wlan[wlanIdx].bssid))
            {
                goto error_exit;
            }
        }
        totalWlanCount += curCfg.radio[radioIdx].wlanCount;

        if(!CWWTPBoardGetRadioCurrentAvailableChannelList(radioIdx, &curCfg.radio[radioIdx].channelCount,
                &(curCfg.radio[radioIdx].channelList)))
        {
            CWLog("*** failed to get radio current available channnel");
            goto error_exit;
        }

        totalChanCount += curCfg.radio[radioIdx].channelCount;
    }
#endif /* CW_WTP_AP */

    len = 4 + 2 + 1 + (4 * 5) + 1 + (curCfg.radioCount * (4 + 4 + 1)) + (totalWlanCount * 6) + (curCfg.radioCount * 1) + totalChanCount;
    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, len,
    {
        CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
        goto error_exit;
    });

    CWProtocolStore32(msgPtr, CW_VENDOR_ID);
    CWProtocolStore16(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_WTP_CURRENT_CFG_INFO);

    CWProtocolStore8(msgPtr, 0); /* version */
    CWProtocolStoreIPv4Address(msgPtr, curCfg.ipv4.address);
    CWProtocolStoreIPv4Address(msgPtr, curCfg.ipv4.mask);
    CWProtocolStoreIPv4Address(msgPtr, curCfg.ipv4.gateway);
    CWProtocolStoreIPv4Address(msgPtr, curCfg.dns1);
    CWProtocolStoreIPv4Address(msgPtr, curCfg.dns2);
    CWProtocolStore8(msgPtr, (unsigned char) curCfg.radioCount);
    for(radioIdx = 0; radioIdx < curCfg.radioCount; radioIdx++)
    {
        CWProtocolStore32(msgPtr, curCfg.radio[radioIdx].channel);
        CWProtocolStore32(msgPtr, curCfg.radio[radioIdx].txPower);
        CWProtocolStore8(msgPtr, (unsigned char) curCfg.radio[radioIdx].wlanCount);
        for(wlanIdx = 0; wlanIdx < curCfg.radio[radioIdx].wlanCount; wlanIdx++)
        {
            CWProtocolStoreRawBytes(msgPtr, (char *) curCfg.radio[radioIdx].wlan[wlanIdx].bssid, 6);
        }

        CWProtocolStore8(msgPtr, curCfg.radio[radioIdx].channelCount);
        for(chanlistIdx = 0; chanlistIdx < curCfg.radio[radioIdx].channelCount; chanlistIdx++)
        {
            CWProtocolStore8(msgPtr, curCfg.radio[radioIdx].channelList[chanlistIdx]);
            //CWLog("curCfg.radio[%d].channelList[%d]=%u", radioIdx, chanlistIdx, curCfg.radio[radioIdx].channelList[chanlistIdx]);
        }
    }

    CWWtpCurrentCfgInfoFree(&curCfg);

    return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE);

error_exit:

    CWWtpCurrentCfgInfoFree(&curCfg);

    return CW_FALSE;
}

#ifdef CW_WTP_SWITCH
CWBool CWWTPAssembleMsgElemVendorPayloadSwitchTopology(CWProtocolMessage *msgPtr)
{
    CWWTPSwitchTopologyInfo topoInfo;
    int i, j;
    int len;

    CW_ZERO_MEMORY(&topoInfo, sizeof(CWWTPSwitchTopologyInfo));

    if(!CWWTPSwitchGetTopologyInfo(&topoInfo, &len))
    {
        return CW_FALSE;
    }

    len += 4 + 2;
    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, len,
    {
        CWWtpSwitchTopologyInfoFree(&topoInfo);
        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
    });

    CWProtocolStore32(msgPtr, CW_VENDOR_ID);
    CWProtocolStore16(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_SWITCH_TOPOLOGY);
    CWProtocolStore16(msgPtr, (unsigned short) topoInfo.devCount);

    for(i = 0; i < topoInfo.devCount; i++)
    {
        CWProtocolStore32(msgPtr, topoInfo.devInfo[i].type);
        CWProtocolStore32(msgPtr, topoInfo.devInfo[i].sysTime);
        CWProtocolStore32(msgPtr, topoInfo.devInfo[i].updateTime);
        CWProtocolStore32(msgPtr, topoInfo.devInfo[i].linkInfoLastTime);
        CWProtocolStore32(msgPtr, topoInfo.devInfo[i].noLinkInfoFirstTime);
        CWProtocolStoreRawBytes(msgPtr, (char *)topoInfo.devInfo[i].mac, 6);
        CWProtocolStoreIPv4Address(msgPtr, topoInfo.devInfo[i].ip);
        CWProtocolStore32(msgPtr, topoInfo.devInfo[i].nameLen);
        if(topoInfo.devInfo[i].nameLen)
        {
            CWProtocolStoreRawBytes(msgPtr, topoInfo.devInfo[i].name, topoInfo.devInfo[i].nameLen);
        }
        CWProtocolStore32(msgPtr, topoInfo.devInfo[i].descLen);
        if(topoInfo.devInfo[i].descLen)
        {
            CWProtocolStoreRawBytes(msgPtr, topoInfo.devInfo[i].desc, topoInfo.devInfo[i].descLen);
        }
        CWProtocolStore32(msgPtr, topoInfo.devInfo[i].linkCount);
        for(j = 0; j < topoInfo.devInfo[i].linkCount; j++)
        {
            CWProtocolStore32(msgPtr, topoInfo.devInfo[i].linkList[j].localPort);
            CWProtocolStoreRawBytes(msgPtr, (char *)topoInfo.devInfo[i].linkList[j].mac, 6);
            CWProtocolStore32(msgPtr, topoInfo.devInfo[i].linkList[j].remotePortLen);
            if(topoInfo.devInfo[i].linkList[j].remotePortLen)
            {
                CWProtocolStoreRawBytes(msgPtr, topoInfo.devInfo[i].linkList[j].remotePort,
                                        topoInfo.devInfo[i].linkList[j].remotePortLen);
            }
        }
    }

    CWWtpSwitchTopologyInfoFree(&topoInfo);

    return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE);
}

CWBool CWWTPAssembleMsgElemVendorPayloadSwitchPoeInfo(CWProtocolMessage *msgPtr)
{
    CWWTPSwitchPoeInfo poeInfo;
    int len;
    int port;

    CW_ZERO_MEMORY(&poeInfo, sizeof(CWWTPSwitchPoeInfo));

    if(!CWWTPSwitchGetPoeInfo(&poeInfo))
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    len = 4 + 2 + 2 + 4 + 4 + (poeInfo.infoCount * (4 * 11));
    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, len,
    {
        CW_FREE_OBJECT(poeInfo.info);
        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
    });

    CWProtocolStore32(msgPtr, CW_VENDOR_ID);
    CWProtocolStore16(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_SWITCH_POE_INFO);
    CWProtocolStore16(msgPtr, (unsigned short) poeInfo.infoCount);
    CWProtocolStore32(msgPtr, poeInfo.powerBudget);
    CWProtocolStore32(msgPtr, poeInfo.allocPower);

    for(port = 0; port < poeInfo.infoCount; port++)
    {
        CWProtocolStore32(msgPtr, poeInfo.info[port].port);
        CWProtocolStore32(msgPtr, poeInfo.info[port].state);
        CWProtocolStore32(msgPtr, poeInfo.info[port].priority);
        CWProtocolStore32(msgPtr, poeInfo.info[port].powerLimitType);
        CWProtocolStore32(msgPtr, poeInfo.info[port].powerLimit);
        CWProtocolStore32(msgPtr, poeInfo.info[port].status);
        CWProtocolStore32(msgPtr, poeInfo.info[port].outputVoltage);
        CWProtocolStore32(msgPtr, poeInfo.info[port].outputCurrent);
        CWProtocolStore32(msgPtr, poeInfo.info[port].outputPower);
        CWProtocolStore32(msgPtr, poeInfo.info[port].temperature);
        CWProtocolStore32(msgPtr, poeInfo.info[port].class);
    }

    CW_FREE_OBJECT(poeInfo.info);

    return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE);
}

CWBool CWWTPAssembleMsgElemVendorPayloadSwitchPortInfo(CWProtocolMessage *msgPtr)
{
    int port;
    int len;
    CWWTPSwitchPortInfo portInfo;

    CW_ZERO_MEMORY(&portInfo, sizeof(CWWTPSwitchPortInfo));

    if(!CWWTPSwitchGetPortInfo(&portInfo))
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    len = 4 + 2 + 2 + (portInfo.infoCount * (4 * 7));
    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, len,
    {
        CW_FREE_OBJECT(portInfo.info);
        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
    });

    CWProtocolStore32(msgPtr, CW_VENDOR_ID);
    CWProtocolStore16(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_SWITCH_PORT_INFO);
    CWProtocolStore16(msgPtr, (unsigned short) portInfo.infoCount);

    for(port = 0; port < portInfo.infoCount; port++)
    {
        CWProtocolStore32(msgPtr, portInfo.info[port].port);
        CWProtocolStore32(msgPtr, portInfo.info[port].status);
        CWProtocolStore32(msgPtr, portInfo.info[port].linkStatus);
        CWProtocolStore32(msgPtr, portInfo.info[port].speed);
        CWProtocolStore32(msgPtr, portInfo.info[port].duplex);
        CWProtocolStore32(msgPtr, portInfo.info[port].flowControl);
        CWProtocolStore32(msgPtr, portInfo.info[port].autoNeg);
    }

    CW_FREE_OBJECT(portInfo.info);

    return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE);
}

CWBool CWWTPAssembleMsgElemVendorPayloadSwitchTrunkInfo(CWProtocolMessage *msgPtr)
{
    int port;
    int len;
    CWWTPSwitchTrunkInfo trunkInfo;

    CW_ZERO_MEMORY(&trunkInfo, sizeof(CWWTPSwitchTrunkInfo));

    if(!CWWTPSwitchGetTrunkInfo(&trunkInfo, &len))
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    len += 4 + 2 + 2 + (trunkInfo.infoCount * (4 * 4));
    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, len,
    {
        CW_FREE_OBJECT(trunkInfo.info);
        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
    });

    CWProtocolStore32(msgPtr, CW_VENDOR_ID);
    CWProtocolStore16(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_SWITCH_TRUNK_INFO);
    CWProtocolStore16(msgPtr, (unsigned short) trunkInfo.infoCount);

    for(port = 0; port < trunkInfo.infoCount; port++)
    {
        CWProtocolStore32(msgPtr, trunkInfo.info[port].id);
        CWProtocolStore32(msgPtr, trunkInfo.info[port].mode);
        CWProtocolStore32(msgPtr, trunkInfo.info[port].activePort_len);
        CWProtocolStoreStr(msgPtr, trunkInfo.info[port].activePort);
        CWProtocolStore32(msgPtr, trunkInfo.info[port].memberPort_len);
        CWProtocolStoreStr(msgPtr, trunkInfo.info[port].memberPort);
    }

    CWWtpSwitchTrunkInfoFree(&trunkInfo);

    return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE);
}

#endif /* CW_WTP_SWITCH */

#ifdef CW_WTP_AP
CWBool CWWTPSaveBackgroundSitesurveyValues(CWBackgroundSitesurveyValues *pBgStSvy)
{
    CWLog("[%02X:%02X:%02X:%02X:%02X:%02X] Background Sitesurvey Request radiotype: %s enable: %u interval: %u",
          CW_MAC_PRINT_LIST(gWTPIfaceMac),
          pBgStSvy->radioType == CW_RADIOFREQTYPE_5G ? "5G" : pBgStSvy->radioType == CW_RADIOFREQTYPE_2G ? "2.4G" : "unknown",
          pBgStSvy->bEnable, pBgStSvy->uint32Interval);

    CWWTPSetBackgroundSitesurveyInterval(pBgStSvy->radioType, pBgStSvy->uint32Interval);
    CWWTPEnableBackgroundSitesurvey(pBgStSvy->radioType, pBgStSvy->bEnable);

    gWTPResultCode = CW_PROTOCOL_SUCCESS;

    return CW_TRUE;
}

CWBool CWWTPSaveAutoTxpowerHealingValues(CWAutoTxPowerHealingValues *pTxPwHealingVal)
{
    CWLog("[%02X:%02X:%02X:%02X:%02X:%02X] Tx Power Healing Request radiotype: %s strength: %d%%",
          CW_MAC_PRINT_LIST(gWTPIfaceMac),
          pTxPwHealingVal->radioType == CW_RADIOFREQTYPE_5G ? "5G" : pTxPwHealingVal->radioType == CW_RADIOFREQTYPE_2G ?"2.4G" :"5-1G",
          pTxPwHealingVal->strength);

    if(CWWTPBoardSetRadioAutoTxPowerStrength(pTxPwHealingVal->radioType, pTxPwHealingVal->strength))
    {
        gWTPResultCode = CW_PROTOCOL_SUCCESS;
    }
    else
    {
        CWErrorHandleLast();
        gWTPResultCode = CW_PROTOCOL_FAILURE;
    }

    return CW_TRUE;
}

CWBool CWWTPAssembleMsgElemVendorPayloadCurrentStationInfo(CWProtocolMessage *msgPtr)
{
    int radioIdx;
    int wlanIdx;
    int staIdx, i, staCount, len;
    int totalOsTypeLen = 0, totalHostNameLen = 0;
    static CWWTPStationInfo *staInfo = NULL;

    /* Note: staInfo is declared as static, so we don't need to init in next time */
    if(!staInfo)
    {
        CW_CREATE_ZERO_OBJECT_ERR(staInfo, CWWTPStationInfo, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

        for(radioIdx = 0; radioIdx < CWWTPBoardGetMaxRadio(); radioIdx++)
        {
            for(wlanIdx = 0; wlanIdx < CWWTPBoardGetMaxRadioWlans(radioIdx); wlanIdx++)
            {
                staInfo->infoCount++;
            }
        }
        if(staInfo->infoCount != 0)
        {
            CW_CREATE_ZERO_ARRAY_ERR(staInfo->info, staInfo->infoCount, CWWTPStation,
            {
                CW_FREE_OBJECT(staInfo);
                return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
            });
        }
    }

    staIdx = 0;
    staCount = 0;
    for(radioIdx = 0; radioIdx < CWWTPBoardGetMaxRadio(); radioIdx++)
    {
        for(wlanIdx = 0; wlanIdx < CWWTPBoardGetMaxRadioWlans(radioIdx); wlanIdx++)
        {
            staInfo->info[staIdx].radioIdx = radioIdx;
            staInfo->info[staIdx].wlanIdx = wlanIdx;

            if(!CWWTPBoardGetWlanStations(radioIdx, wlanIdx, &(staInfo->info[staIdx].stationCount),
                                          &(staInfo->info[staIdx].station)))
            {
                goto error_exit;
            }

            for(i = 0; i < staInfo->info[staIdx].stationCount; i++)
            {
                totalOsTypeLen += staInfo->info[staIdx].station[i].osTypeLen;
                totalHostNameLen += staInfo->info[staIdx].station[i].hostNameLen;
            }
            staCount += staInfo->info[staIdx].stationCount;
            staIdx++;
        }
    }

    len = 4 + 2 + 2 + (staInfo->infoCount * (1 + 1 + 2)) + \
          (staCount * (6 + 4 + 4 + 4 + 1 + 1 + 4)) + totalOsTypeLen + totalHostNameLen;
    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, len,
    {
        CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
        goto error_exit;
    });

    CWProtocolStore32(msgPtr, CW_VENDOR_ID);
    CWProtocolStore16(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CURRENT_STATION_INFO);
    CWProtocolStore16(msgPtr, (unsigned short) staInfo->infoCount);
    for(staIdx = 0; staIdx < staInfo->infoCount; staIdx++)
    {
        CWProtocolStore8(msgPtr, (unsigned char) staInfo->info[staIdx].radioIdx);
        CWProtocolStore8(msgPtr, (unsigned char) staInfo->info[staIdx].wlanIdx);
        CWProtocolStore16(msgPtr, (unsigned short) staInfo->info[staIdx].stationCount);
        for(i = 0; i < staInfo->info[staIdx].stationCount; i++)
        {
            CWProtocolStoreRawBytes(msgPtr, (char *) staInfo->info[staIdx].station[i].mac, 6);
            CWProtocolStore32(msgPtr, staInfo->info[staIdx].station[i].txKB);
            CWProtocolStore32(msgPtr, staInfo->info[staIdx].station[i].rxKB);
            CWProtocolStore32(msgPtr, staInfo->info[staIdx].station[i].rssi);

            CWProtocolStore8(msgPtr, staInfo->info[staIdx].station[i].osTypeLen);
            if(staInfo->info[staIdx].station[i].osTypeLen)
            {
                CWProtocolStoreRawBytes(msgPtr, staInfo->info[staIdx].station[i].osType,
                                        staInfo->info[staIdx].station[i].osTypeLen);
            }
            CWProtocolStore8(msgPtr, staInfo->info[staIdx].station[i].hostNameLen);
            if(staInfo->info[staIdx].station[i].hostNameLen)
            {
                CWProtocolStoreRawBytes(msgPtr, staInfo->info[staIdx].station[i].hostName,
                                        staInfo->info[staIdx].station[i].hostNameLen);
            }
            CWProtocolStoreIPv4Address(msgPtr, staInfo->info[staIdx].station[i].address);

            CW_FREE_OBJECT(staInfo->info[staIdx].station[i].osType);
            CW_FREE_OBJECT(staInfo->info[staIdx].station[i].hostName);
        }
        CW_FREE_OBJECT(staInfo->info[staIdx].station);
        staInfo->info[staIdx].stationCount = 0;
    }

    return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE);

error_exit:

    for(staIdx = 0; staIdx < staInfo->infoCount; staIdx++)
    {
        for(i = 0; i < staInfo->info[staIdx].stationCount; i++)
        {
            CW_FREE_OBJECT(staInfo->info[staIdx].station[i].osType);
            CW_FREE_OBJECT(staInfo->info[staIdx].station[i].hostName);
        }
        CW_FREE_OBJECT(staInfo->info[staIdx].station);
        staInfo->info[staIdx].stationCount = 0;
    }

    return CW_FALSE;
}

CWBool CWWTPAssembleMsgElemVendorPayloadWlanStatisticsInfo(CWProtocolMessage *msgPtr)
{
    int radioIdx;
    int wlanIdx;
    int statIdx, len;
    static CWWTPStatisticsInfo *statInfo = NULL;

    if(!statInfo)
    {
        CW_CREATE_ZERO_OBJECT_ERR(statInfo, CWWTPStatisticsInfo,
                                  return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

        for(radioIdx = 0; radioIdx < CWWTPBoardGetMaxRadio(); radioIdx++)
        {
            for(wlanIdx = 0; wlanIdx < CWWTPBoardGetMaxRadioWlans(radioIdx); wlanIdx++)
            {
                statInfo->infoCount++;
            }
        }
        if(statInfo->infoCount != 0)
        {
            CW_CREATE_ZERO_ARRAY_ERR(statInfo->info, statInfo->infoCount, CWWlanStatisticsInfo,
            {
                CW_FREE_OBJECT(statInfo);
                return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
            });
        }
    }

    statIdx = 0;
    for(radioIdx = 0; radioIdx < CWWTPBoardGetMaxRadio(); radioIdx++)
    {
        for(wlanIdx = 0; wlanIdx < CWWTPBoardGetMaxRadioWlans(radioIdx); wlanIdx++)
        {
            statInfo->info[statIdx].radioIdx = radioIdx;
            statInfo->info[statIdx].wlanIdx = wlanIdx;
            if(!CWWTPBoardGetWlanStatistics(radioIdx, wlanIdx, &(statInfo->info[statIdx].statistics)))
            {
                return CW_FALSE;
            }
            statIdx++;
        }
    }

    len = 4 + 2 + 2 + (statInfo->infoCount * (1 + 1 + (8 * 4)));
    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, len,
    {
        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
    });

    CWProtocolStore32(msgPtr, CW_VENDOR_ID);
    CWProtocolStore16(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_WLAN_STATISTICS);
    CWProtocolStore16(msgPtr, (unsigned short) statInfo->infoCount);
    for(statIdx = 0; statIdx < statInfo->infoCount; statIdx++)
    {
        CWProtocolStore8(msgPtr, (unsigned char)statInfo->info[statIdx].radioIdx);
        CWProtocolStore8(msgPtr, (unsigned char)statInfo->info[statIdx].wlanIdx);
        CWProtocolStore32(msgPtr, statInfo->info[statIdx].statistics.txPkts);
        CWProtocolStore32(msgPtr, statInfo->info[statIdx].statistics.txErrPkts);
        CWProtocolStore32(msgPtr, statInfo->info[statIdx].statistics.txDrpPkts);
        CWProtocolStore32(msgPtr, statInfo->info[statIdx].statistics.txBytes);
        CWProtocolStore32(msgPtr, statInfo->info[statIdx].statistics.rxPkts);
        CWProtocolStore32(msgPtr, statInfo->info[statIdx].statistics.rxErrPkts);
        CWProtocolStore32(msgPtr, statInfo->info[statIdx].statistics.rxDrpPkts);
        CWProtocolStore32(msgPtr, statInfo->info[statIdx].statistics.rxBytes);
    }

    return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE);
}

CWBool CWWTPAssembleMsgElemVendorPayloadSitesurveyInfo(CWProtocolMessage *msgPtr, CWWTPSitesurveyInfo *sitesurveyInfo)
{
    int surveyIdx, len;

    len = 4 + 2 + 1 + 2 + (sitesurveyInfo->infoCount * (6 + 1 + 33 + 1 + 4 + 4 + 1 + 1));

    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, len,
    {
        CW_FREE_OBJECT(sitesurveyInfo->info);
        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
    });

    CWProtocolStore32(msgPtr, CW_VENDOR_ID); // 4
    CWProtocolStore16(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_SITESURVEY_RESULT); // 2
    CWProtocolStore8(msgPtr, (unsigned char) sitesurveyInfo->radio); // 1
    CWProtocolStore16(msgPtr, (unsigned short) sitesurveyInfo->infoCount); // 2
    for(surveyIdx = 0; surveyIdx < sitesurveyInfo->infoCount; surveyIdx++)
    {
        CWProtocolStoreRawBytes(msgPtr, (char *) sitesurveyInfo->info[surveyIdx].bssid, 6); // 6
        CWProtocolStore8(msgPtr, sitesurveyInfo->info[surveyIdx].ssidLen); // 1
        CWProtocolStoreRawBytes(msgPtr, sitesurveyInfo->info[surveyIdx].ssid, 33); // 33
        CWProtocolStore8(msgPtr, (unsigned char)sitesurveyInfo->info[surveyIdx].mode); // 1
        CWProtocolStore32(msgPtr, sitesurveyInfo->info[surveyIdx].chan); // 4
        CWProtocolStore32(msgPtr, sitesurveyInfo->info[surveyIdx].signal); // 4
        CWProtocolStore8(msgPtr, (unsigned char)sitesurveyInfo->info[surveyIdx].enc); // 1
        CWProtocolStore8(msgPtr, (unsigned char)sitesurveyInfo->info[surveyIdx].type); // 1
    }

    return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE);
}

CWBool CWWTPAssembleMsgElemVendorPayloadMeshInfo(CWProtocolMessage *msgPtr)
{
    int radioIdx, len;
    static CWWTPMeshInfo *meshInfo = NULL;

    if(!meshInfo)
    {
        CW_CREATE_ZERO_OBJECT_ERR(meshInfo, CWWTPMeshInfo,
                                  return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

        meshInfo->infoCount = CWWTPBoardGetMaxRadio();
        CW_CREATE_ZERO_ARRAY_ERR(meshInfo->info, meshInfo->infoCount, CWRadioMeshInfo,
        {
            CW_FREE_OBJECT(meshInfo);
            return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
        });
    }

    for(radioIdx = 0; radioIdx < meshInfo->infoCount; radioIdx++)
    {
        if(!CWWTPBoardGetRadioMeshInfo(radioIdx, &meshInfo->info[radioIdx]))
        {
            return CW_FALSE;
        }
    }

    len = 4 + 2 + 2 + (meshInfo->infoCount * (4 + 6));
    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, len,
    {
        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
    });

    CWProtocolStore32(msgPtr, CW_VENDOR_ID);
    CWProtocolStore16(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_MESH_INFO);
    CWProtocolStore16(msgPtr, (unsigned short) meshInfo->infoCount);
    for(radioIdx = 0; radioIdx < meshInfo->infoCount; radioIdx++)
    {
        CWProtocolStore32(msgPtr, meshInfo->info[radioIdx].role);
        CWProtocolStoreRawBytes(msgPtr, (char *) meshInfo->info[radioIdx].bssid, 6);
    }

    return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE);
}

#endif /* #ifdef CW_WTP_AP */

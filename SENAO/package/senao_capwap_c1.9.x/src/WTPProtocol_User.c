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


#include "CWWTP.h"
#include "CWVersion.h"

#ifdef DMALLOC
#include "../dmalloc-5.5.0/dmalloc.h"
#endif

int CWWTPGetDiscoveryType()
{
    return gWTPDiscType;
}

#ifdef CW_WTP_AP
int CWWTPGetMaxRadios()
{
    return CWWTPBoardGetMaxRadio();
}

int CWWTPGetRadiosInUse()
{
    return CWWTPBoardGetMaxRadio();
}

int CWWTPGetEncCapabilities()
{
    return CWWTPBoardGetEncryptionCapabilities();
}
#endif

CWBool CWWTPGetBoardData(CWWTPVendorInfos *valPtr)
{
    CWMacAddress *mac;
    char *str = NULL;
#ifdef CW_WTP_AP
    unsigned int *code;
#endif

    if(valPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

#ifdef CW_WTP_AP
    valPtr->vendorInfosCount = 5;
#else /* CW_WTP_SWITCH */
    valPtr->vendorInfosCount = 3;
#endif

    CW_CREATE_ZERO_ARRAY_ERR(valPtr->info, valPtr->vendorInfosCount, CWWTPVendorInfoValues,
                             return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if(!CWWTPBoardGetModelName(&str))
    {
        CWWTPDestroyVendorInfos(valPtr);
        return CW_FALSE;
    }
    valPtr->info[0].vendorIdentifier = CW_VENDOR_ID;
    valPtr->info[0].type = CW_BOARD_MODEL_NAME;
    valPtr->info[0].length = strlen(str);
    valPtr->info[0].valuePtr = (int *) str;

    if(!CWWTPBoardGetSerialNum(&str))
    {
        CWWTPDestroyVendorInfos(valPtr);
        return CW_FALSE;
    }
    valPtr->info[1].vendorIdentifier = CW_VENDOR_ID;
    valPtr->info[1].type = CW_BOARD_SERIAL_NUMBER;
    valPtr->info[1].length = strlen(str);
    valPtr->info[1].valuePtr = (int *) str;

    CW_CREATE_ARRAY_ERR(mac, 1, CWMacAddress,
    {
        CWWTPDestroyVendorInfos(valPtr);
        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
    });
    valPtr->info[2].vendorIdentifier = CW_VENDOR_ID;
    valPtr->info[2].type = CW_BOARD_BASE_MAC;
    valPtr->info[2].length = 6; // just 6 bytes
    valPtr->info[2].valuePtr = (int *) mac;
    CW_COPY_MEMORY(mac, gWTPIfaceMac, 6);

#ifdef CW_WTP_AP
    /* SENAO ADD: capability code */
    CW_CREATE_OBJECT_ERR(code, unsigned int,
    {
        CWWTPDestroyVendorInfos(valPtr);
        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
    });

    if(!CWWTPBoardGetCapCode(code))
    {
        CWWTPDestroyVendorInfos(valPtr);
        return CW_FALSE;
    }

    *code = htonl(*code); /* network byte order */
    valPtr->info[3].vendorIdentifier = CW_VENDOR_ID;
    valPtr->info[3].type = CW_BOARD_SENAO_CAP_CODE;
    valPtr->info[3].length = 4;
    valPtr->info[3].valuePtr = (int *) code;

    /* SENAO ADD: sku */
    if(!CWWTPBoardGetSku(&str))
    {
        CWWTPDestroyVendorInfos(valPtr);
        return CW_FALSE;
    }
    valPtr->info[4].vendorIdentifier = CW_VENDOR_ID;
    valPtr->info[4].type = CW_BOARD_SENAO_SKU;
    valPtr->info[4].length = strlen(str);
    valPtr->info[4].valuePtr = (int *) str;
#endif /* CW_WTP_AP */

    return CW_TRUE;
}

CWBool CWWTPGetWTPDescriptor(CWWTPVendorInfos *valPtr)
{
    char *str = NULL;
#ifdef CW_WTP_SWITCH
    int *HwInfo;
    int len;
    int retLen;
#endif

    if(valPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

#ifdef CW_WTP_SWITCH
    valPtr->vendorInfosCount = 6; // we fill 6 information (just the required ones)
#else /* CW_WTP_AP */
    valPtr->vendorInfosCount = 5; // we fill 5 information (just the required ones)
#endif
    CW_CREATE_ZERO_ARRAY_ERR(valPtr->info, valPtr->vendorInfosCount, CWWTPVendorInfoValues,
                             return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if(!CWWTPBoardGetHardwareVersion(&str))
    {
        CWWTPDestroyVendorInfos(valPtr);
        return CW_FALSE;
    }
    valPtr->info[0].vendorIdentifier = 0;
    valPtr->info[0].type = CW_WTP_HARDWARE_VERSION;
    valPtr->info[0].length = strlen(str);
    valPtr->info[0].valuePtr = (int *) str;

    if(!CWWTPBoardGetSoftwareVersion(&str))
    {
        CWWTPDestroyVendorInfos(valPtr);
        return CW_FALSE;
    }
    valPtr->info[1].vendorIdentifier = 0;
    valPtr->info[1].type = CW_WTP_SOFTWARE_VERSION;
    valPtr->info[1].length = strlen(str);
    valPtr->info[1].valuePtr = (int *) str;

    if(!CWWTPBoardGetBootVersion(&str))
    {
        CWWTPDestroyVendorInfos(valPtr);
        return CW_FALSE;
    }
    valPtr->info[2].vendorIdentifier = 0;
    valPtr->info[2].type = CW_WTP_BOOT_VERSION;
    valPtr->info[2].length = strlen(str);
    valPtr->info[2].valuePtr = (int *) str;

    /* Add SENAO CAPWAP version */
    CW_CREATE_STRING_FROM_STRING_ERR(str, CW_VERSION,
    {
        CWWTPDestroyVendorInfos(valPtr);
        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
    });
    valPtr->info[3].vendorIdentifier = CW_VENDOR_ID;
    valPtr->info[3].type = CW_WTP_SENAO_CAPWAP_VERSION;
    valPtr->info[3].length = strlen(str);
    valPtr->info[3].valuePtr = (int *) str;

    /* Add SENAO device name */
    if(!CWWTPBoardGetNameCfg(&str))
    {
        CWWTPDestroyVendorInfos(valPtr);
        return CW_FALSE;
    }
    valPtr->info[4].vendorIdentifier = CW_VENDOR_ID;
    valPtr->info[4].type = CW_WTP_SENAO_DEVICE_NAME;
    valPtr->info[4].length = strlen(str);
    valPtr->info[4].valuePtr = (int *) str;

#ifdef CW_WTP_SWITCH
    /* Add SENAO port info */
    len = (12) + (2 * CWWTPSwitchGetMaxPhysicalPortNum());

    CW_CREATE_ZERO_ARRAY_ERR(HwInfo, len, int,
                             return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if(!CWWTPSwitchGetHwInfo(HwInfo, &retLen))
    {
        CWWTPDestroyVendorInfos(valPtr);
        return CW_FALSE;
    }

    if(len != (retLen))
    {
        CWLog("Error:Length is not correct len=%d relen=%d", len, retLen * 4);
        CWWTPDestroyVendorInfos(valPtr);
        return CW_FALSE;
    }

    valPtr->info[5].vendorIdentifier = CW_VENDOR_ID;
    valPtr->info[5].type = CW_WTP_SENAO_SWITCH_PORT_HW_INFO;
    valPtr->info[5].length = len * 4;
    valPtr->info[5].valuePtr = (int *) HwInfo;
#endif

    return CW_TRUE;
}

void CWWTPDestroyVendorInfos(CWWTPVendorInfos *valPtr)
{
    int i;

    if(valPtr == NULL)
    {
        return;
    }

    for(i = 0; i < valPtr->vendorInfosCount; i++)
    {
        CW_FREE_OBJECT((valPtr->info)[i].valuePtr);
    }

    CW_FREE_OBJECT(valPtr->info);
}

int CWWTPGetFrameTunnelMode()
{
    //it may be also 802.3_FrameTunnelMode - NativeFrameTunnelMode - All
    return CW_LOCAL_BRIDGING;
}

int CWWTPGetMACType()
{
    return CW_LOCAL_MAC;
}

int CWWTPGetSessionID()
{
    return CWRandomIntInRange(0, INT_MAX);
}

int CWWTPGetIPv4Address()
{
    struct sockaddr_in myAddr;
    unsigned int len = sizeof(myAddr);

    //CWDebugLog("WTPGetIPv4Address");

    /* assume the socket is connected */
    getsockname(gWTPSocket, (struct sockaddr *) &myAddr, &len);

    return ntohl(myAddr.sin_addr.s_addr); 	// TO-DO: this is garbage if we are an IPv6 client
}

void CWWTPGetIPv6Address(struct sockaddr_in6 *myAddr)
{

    unsigned int len = sizeof(*myAddr);

    /* assume the socket is connected */
    getsockname(gWTPSocket, (struct sockaddr *) myAddr, &len);
}

int CWWTPGetIPv4StatusDuplicate()
{
    return gIPv4StatusDuplicate;
}

int CWWTPGetIPv6StatusDuplicate()
{
    return gIPv6StatusDuplicate;
}

CWBool CWWTPGetName(char **pstr)
{
    return CWWTPBoardGetNameCfg(pstr);
}

CWBool CWWTPGetLocation(char **pstr)
{
    return CWWTPBoardGetLocationCfg(pstr);
}

CWBool CWWTPGetACNameWithPriority(CWACNamesWithPriority *valPtr)
{
    int i;

    if(valPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    valPtr->count = gWTPAcNamePriInfo.count;
    if(valPtr->count == 0)
    {
        valPtr->ACs = NULL;
        return CW_TRUE;
    }

    CW_CREATE_ARRAY_ERR(valPtr->ACs, valPtr->count, CWACNameWithPriorityValues,
                        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
    CW_ZERO_MEMORY(valPtr->ACs, valPtr->count * sizeof(CWACNameWithPriorityValues));

    for(i = 0; i < valPtr->count; i++)
    {
        valPtr->ACs[i].priority = gWTPAcNamePriInfo.ACs[i].priority;
        CW_CREATE_STRING_FROM_STRING_ERR(valPtr->ACs[i].ACName, gWTPAcNamePriInfo.ACs[i].ACName,
        {
            CWWTPDestroyACNameWithPriority(valPtr);
            return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
        });
    }
    return CW_TRUE;
}

void CWWTPDestroyACNameWithPriority(CWACNamesWithPriority *valPtr)
{
    int i;

    for(i = 0; i < valPtr->count; i++)
    {
        CW_FREE_OBJECT(valPtr->ACs[i].ACName);
    }
    CW_FREE_OBJECT(valPtr->ACs);
}

#ifdef CW_WTP_AP
CWBool CWWTPGetRadiosInfomation(CWRadiosInformation *valPtr)
{
    int i, j;

    if(valPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    valPtr->radiosCount = CWWTPGetMaxRadios();
    CW_CREATE_ZERO_ARRAY_ERR(valPtr->radios, valPtr->radiosCount, CWRadioInformationValues,
                             return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    for(i = 0; i < valPtr->radiosCount; i++)
    {
        valPtr->radios[i].ID = i + 1;

        if(!CWWTPBoardGetRadioType(i, &(valPtr->radios[i].type)))
        {
            goto err_exit;
        }

        if(!CWWTPBoardGetRadioMac(i, valPtr->radios[i].mac))
        {
            goto err_exit;
        }

        if(!CWWTPBoardGetRadioMaxTxPower(i, &(valPtr->radios[i].maxTxPower)))
        {
            goto err_exit;
        }

        valPtr->radios[i].maxWlans = CWWTPBoardGetMaxRadioWlans(i);
        CW_CREATE_ZERO_ARRAY_ERR(valPtr->radios[i].wlans, valPtr->radios[i].maxWlans, CWWlanInformationValues,
        {
            CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
            goto err_exit;
        });

        for(j = 0; j < valPtr->radios[i].maxWlans; j++)
        {
            if(!CWWTPBoardGetWlanBssid(i, j, valPtr->radios[i].wlans[j].bssid))
            {
                goto err_exit;
            }
        }
    }

    return CW_TRUE;

err_exit:

    for(i = 0; i < valPtr->radiosCount; i++)
    {
        CW_FREE_OBJECT(valPtr->radios[i].wlans);
    }
    CW_FREE_OBJECT(valPtr->radios);

    return CW_FALSE;
}

/* L'AC ha la funzione ridefinita */
CWBool CWGetWTPRadiosAdminState(CWRadiosAdminInfo *valPtr)
{
    int i;
    int operMode;

    if(valPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    valPtr->radiosCount = CWWTPGetMaxRadios();
    CW_CREATE_ZERO_ARRAY_ERR(valPtr->radios, valPtr->radiosCount, CWRadioAdminInfoValues,
    {
        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
    });

    for(i = 0; i < CWWTPBoardGetMaxRadio(); i++)
    {
        valPtr->radios[i].ID = i + 1;
        if(!CWWTPBoardGetRadioOperationModeCfg(i, &operMode))
        {
            CW_FREE_OBJECT(valPtr->radios);
            return CW_FALSE;
        }
        valPtr->radios[i].state = operMode == CW_RADIO_OPERATION_MODE_DISABLED ? DISABLED : ENABLED;
    }

    return CW_TRUE;
}

CWBool CWGetWTPRadiosOperationalState(int radioID, CWRadiosOperationalInfo *valPtr)
{
    int i;

    if(valPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    if(radioID > CWWTPBoardGetMaxRadio())
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    if(radioID <= 0)
    {
        valPtr->radiosCount = CWWTPBoardGetMaxRadio();
        CW_CREATE_ARRAY_ERR(valPtr->radios, CWWTPBoardGetMaxRadio(), CWRadioOperationalInfoValues,
        {
            return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
        });

        for(i = 0; i < valPtr->radiosCount; i++)
        {
            valPtr->radios[i].ID = i + 1;
            if(!CWWTPBoardGetRadioOperationalState(i, &(valPtr->radios[i].state), &(valPtr->radios[i].cause)))
            {
                CW_FREE_OBJECT(valPtr->radios);
                return CW_FALSE;
            }
        }
    }
    else
    {
        valPtr->radiosCount = 1;
        CW_CREATE_ARRAY_ERR(valPtr->radios, 1, CWRadioOperationalInfoValues,
        {
            return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
        });

        valPtr->radios[0].ID = radioID;
        if(!CWWTPBoardGetRadioOperationalState(radioID - 1, &(valPtr->radios[0].state), &(valPtr->radios[0].cause)))
        {
            CW_FREE_OBJECT(valPtr->radios);
            return CW_FALSE;
        }
    }
    return CW_TRUE;
}

CWBool CWGetDecryptErrorReport(int radioID, CWDecryptErrorReportInfo *valPtr)
{
    int i;

    if(valPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }
    if(radioID > CWWTPBoardGetMaxRadio())
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    if(radioID <= 0)
    {
        valPtr->radiosCount = CWWTPBoardGetMaxRadio();
        CW_CREATE_ARRAY_ERR(valPtr->radios, valPtr->radiosCount, CWDecryptErrorReportValues,
        {
            return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
        });

        for(i = 0; i < valPtr->radiosCount; i++)
        {
            valPtr->radios[i].ID = i + 1;
            valPtr->radios[i].length = 6;
            if(!CWWTPBoardGetRadioDecryptErrorReport(i, &(valPtr->radios[i].errorMACAddressList),
                    &(valPtr->radios[i].numEntries)))
            {
                CWDestroyDecryptErrorReport(valPtr);
                return CW_FALSE;
            }
        }
    }
    else
    {
        valPtr->radiosCount = 1;
        CW_CREATE_ARRAY_ERR(valPtr->radios, 1, CWDecryptErrorReportValues,
        {
            return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
        });

        valPtr->radios[0].ID = radioID;
        valPtr->radios[0].length = 6;
        if(!CWWTPBoardGetRadioDecryptErrorReport(radioID - 1, &(valPtr->radios[0].errorMACAddressList),
                &(valPtr->radios[0].numEntries)))
        {
            CWDestroyDecryptErrorReport(valPtr);
            return CW_FALSE;
        }
    }

    return CW_TRUE;
}

void CWDestroyDecryptErrorReport(CWDecryptErrorReportInfo *valPtr)
{
    int i;

    for(i = 0; i < valPtr->radiosCount; i++)
    {
        CW_FREE_OBJECT(valPtr->radios[i].errorMACAddressList);
    }
    CW_FREE_OBJECT(valPtr->radios);
}
#endif /* CW_WTP_AP */

int CWWTPGetACIndex()
{
    return 1; //valore predefinito
}

char *CWWTPGetACName()
{
    return gACInfoPtr->name;
}

int CWWTPGetStatisticsTimer()
{
    return gWTPStatisticsTimer;
}

#ifdef CW_WTP_AP
CWBool CWWTPGetWtpCfgCapInfo(CWWtpCfgCapInfo *info)
{
    int i, j, capIdx;

    info->count = 0;

    for(i = 0; i < CWWTPBoardGetMaxRadio(); i++)
    {
        info->count += CWWTPBoardGetMaxRadioWlans(i);
    }

    info->count += 1 + CWWTPBoardGetMaxRadio();

    CW_CREATE_ARRAY_ERR(info->cfgCap, info->count, CWWtpCfgCap,
    {
        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
    });

    capIdx = 0;
    CWWTPBoardGetWtpCfgCap(info->cfgCap[capIdx++]);
    for(i = 0; i < CWWTPBoardGetMaxRadio(); i++)
    {
        CWWTPBoardGetRadioCfgCap(i, info->cfgCap[capIdx++]);
        for(j = 0; j < CWWTPBoardGetMaxRadioWlans(i); j++)
        {
            CWWTPBoardGetWlanCfgCap(i, j, info->cfgCap[capIdx++]);
        }
    }

    return CW_TRUE;
}
#elif defined(CW_WTP_SWITCH)
CWBool CWWTPGetWtpCfgCapInfo(CWWtpCfgCapInfo *info)
{
    info->count = 1;
    CW_CREATE_ARRAY_ERR(info->cfgCap, info->count, CWWtpCfgCap,
    {
        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
    });

    CWWTPBoardGetWtpCfgCap(info->cfgCap[0]);
    return CW_TRUE;
}
#endif

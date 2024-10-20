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

#ifdef DMALLOC
#include "../dmalloc-5.5.0/dmalloc.h"
#endif
#include "CWVendorPayloads.h"

/*____________________________________________________________________________*/
/*  *****************************___ASSEMBLE___*****************************  */
CWBool CWAssembleMsgElemACName(CWProtocolMessage *msgPtr)
{
    char *name;

    if(msgPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    name = CWWTPGetACName();
    //	CWDebugLog("AC Name: %s", name);

    // create message
    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, strlen(name), return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    CWProtocolStoreStr(msgPtr, name);

    return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_AC_NAME_CW_TYPE);
}

CWBool CWAssembleMsgElemACNameWithPriority(CWProtocolMessage *msgPtr)
{
    const int ac_pri_length = 1;
    CWACNamesWithPriority AcPriority;
    CWProtocolMessage *msgs;
    int len = 0;
    int i, j;

    CWDebugLog("CWAssembleMsgElemACNameWithPriority...");

    if(msgPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    if(!CWWTPGetACNameWithPriority(&AcPriority))
    {
        return CW_FALSE;
    }

    if(AcPriority.count == 0)
    {
        return CW_TRUE;
    }

    CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgs, AcPriority.count,
                                     return  CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    // create message
    for(i = 0; i < AcPriority.count; i++)
    {
        // create message
        CW_CREATE_PROTOCOL_MESSAGE(msgs[i], ac_pri_length + strlen(AcPriority.ACs[i].ACName),
        {
            for(j = i - 1; j >= 0; j--)
            {
                CW_FREE_PROTOCOL_MESSAGE(msgs[i]);
            }
            CW_FREE_OBJECT(msgs);
            CWWTPDestroyACNameWithPriority(&AcPriority);
            return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
        });
        CWProtocolStore8(&(msgs[i]), AcPriority.ACs[i].priority); // ID of the AC
        CWProtocolStoreStr(&(msgs[i]), AcPriority.ACs[i].ACName); // name of the AC
        if(!(CWAssembleMsgElem(&(msgs[i]), CW_MSG_ELEMENT_AC_NAME_WITH_PRI_CW_TYPE)))
        {
            for(j = i; j >= 0; j--)
            {
                CW_FREE_PROTOCOL_MESSAGE(msgs[i]);
            }
            CW_FREE_OBJECT(msgs);
            CWWTPDestroyACNameWithPriority(&AcPriority);
            return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
        }
        //		CWDebugLog("AC Name with index: %d - %s", ACsinfo.ACNameIndex[i].index, ACsinfo.ACNameIndex[i].ACName);
        len += msgs[i].offset;
    }

    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, len,
    {
        for(j = i - 1; j >= 0; j--)
        {
            CW_FREE_PROTOCOL_MESSAGE(msgs[i]);
        }
        CW_FREE_OBJECT(msgs);
        CWWTPDestroyACNameWithPriority(&AcPriority);
        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
    });

    for(i = 0; i < gWTPAcNamePriInfo.count; i++)
    {
        CWProtocolStoreMessage(msgPtr, &(msgs[i]));
        CW_FREE_PROTOCOL_MESSAGE(msgs[i]);
    }

    CW_FREE_OBJECT(msgs);
    CWWTPDestroyACNameWithPriority(&AcPriority);
    return CW_TRUE;
}

CWBool CWAssembleMsgElemDataTransferData(CWProtocolMessage *msgPtr, int data_type)
{
    if(msgPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    char *debug_data = " #### DATA DEBUG INFO #### ";  //to be changed...
    // create message
    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, 2 + strlen(debug_data),
                               return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    CWProtocolStore8(msgPtr, data_type);
    CWProtocolStore8(msgPtr, strlen(debug_data));
    CWProtocolStoreStr(msgPtr, debug_data);

    return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_DATA_TRANSFER_DATA_CW_TYPE);
}

CWBool CWAssembleMsgElemDiscoveryType(CWProtocolMessage *msgPtr, CWDiscoveryType type)
{
    if(msgPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    // create message
    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, 1, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    //	CWDebugLog("Discovery Type: %d", CWWTPGetDiscoveryType());

    CWProtocolStore8(msgPtr, type);

    return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_DISCOVERY_TYPE_CW_TYPE);
}

CWBool CWAssembleMsgElemLocationData(CWProtocolMessage *msgPtr)
{
    char *location;

    if(msgPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    if(!CWWTPGetLocation(&location))
    {
        return CW_FALSE;
    }

    // create message
    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, strlen(location),
    {
        CW_FREE_OBJECT(location);
        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
    });

    //	CWDebugLog("Location Data: %s", location);
    CWProtocolStoreStr(msgPtr, location);
    CW_FREE_OBJECT(location);
    return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_LOCATION_DATA_CW_TYPE);
}

CWBool CWAssembleMsgElemStatisticsTimer(CWProtocolMessage *msgPtr)
{
    const int statistics_timer_length = 2;

    CWDebugLog("CWAssembleMsgElemStatisticsTimer...");

    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, statistics_timer_length, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    CWProtocolStore16(msgPtr, CWWTPGetStatisticsTimer());

    //	CWDebugLog("Statistics Timer: %d", CWWTPGetStatisticsTimer());

    return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_STATISTICS_TIMER_CW_TYPE);
}

CWBool CWAssembleMsgElemWTPBoardData(CWProtocolMessage *msgPtr)
{
    const int VENDOR_ID_LENGTH = 4; 	//Vendor Identifier is 4 bytes long
    const int TLV_HEADER_LENGTH = 4; 	//Type and Length of a TLV field is 4 byte long
    CWWTPVendorInfos infos;
    int i, size = 0;

    if(msgPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    // get infos
    if(!CWWTPGetBoardData(&infos))
    {
        return CW_FALSE;
    }

    //Calculate msg elem size
    size = VENDOR_ID_LENGTH;
    for(i = 0; i < infos.vendorInfosCount; i++)
    {
        size += (TLV_HEADER_LENGTH + ((infos.info)[i]).length);
    }

    // create message
    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, size, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    CWProtocolStore32(msgPtr, ((infos.info)[0].vendorIdentifier));
    for(i = 0; i < infos.vendorInfosCount; i++)
    {
        CWProtocolStore16(msgPtr, ((infos.info)[i].type));
        CWProtocolStore16(msgPtr, ((infos.info)[i].length));

        if(((infos.info)[i].length))
        {
            CWProtocolStoreRawBytes(msgPtr, (char *)((infos.info)[i].valuePtr), (infos.info)[i].length);
        }
    }

    CWWTPDestroyVendorInfos(&infos);

    return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_WTP_BOARD_DATA_CW_TYPE);
}

CWBool CWAssembleMsgElemWTPDescriptor(CWProtocolMessage *msgPtr)
{
    const int GENERIC_RADIO_INFO_LENGTH = 6;//First 4 bytes for Max Radios, Radios In Use and Encryption Capability
    const int VENDOR_ID_LENGTH = 4; 	//Vendor Identifier is 4 bytes long
    const int TLV_HEADER_LENGTH = 4; 	//Type and Length of a TLV field is 4 byte long
    CWWTPVendorInfos infos;
    int i, size = 0;

    if(msgPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    // get infos
    if(!CWWTPGetWTPDescriptor(&infos))
    {
        return CW_FALSE;
    }

    //Calculate msg elem size
    size = GENERIC_RADIO_INFO_LENGTH;
    for(i = 0; i < infos.vendorInfosCount; i++)
    {
        size += (VENDOR_ID_LENGTH + TLV_HEADER_LENGTH + ((infos.info)[i]).length);
    }

    // create message
    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, size,
    {
        CWWTPDestroyVendorInfos(&infos);
        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
    });

#ifdef CW_WTP_SWITCH
    CWProtocolStore8(msgPtr, 0); // no radio
    CWProtocolStore8(msgPtr, 0); // no radio
    CWProtocolStore8(msgPtr, 1); // no support of Encrypt
    CWProtocolStore8(msgPtr, CW_WTP_WBID_802_3); // WBID = IEEE 802.3 for SENAO switch
    CWProtocolStore16(msgPtr, 0); // encryption capabilities
#else /* CW_WTP_AP */
    CWProtocolStore8(msgPtr, CWWTPGetMaxRadios()); // number of radios supported by the WTP
    CWProtocolStore8(msgPtr, CWWTPGetRadiosInUse()); // number of radios present in the WTP
    CWProtocolStore8(msgPtr, 1); // Num Encrypt in the WTP
    CWProtocolStore8(msgPtr, CW_WTP_WBID_802_11); // WBID = IEEE 802.11
    CWProtocolStore16(msgPtr, CWWTPGetEncCapabilities()); // encryption capabilities
#endif

    for(i = 0; i < infos.vendorInfosCount; i++)
    {
        CWProtocolStore32(msgPtr, ((infos.info)[i].vendorIdentifier));
        CWProtocolStore16(msgPtr, ((infos.info)[i].type));
        CWProtocolStore16(msgPtr, ((infos.info)[i].length));
        if(((infos.info)[i].length))
        {
            CWProtocolStoreRawBytes(msgPtr, (char *)((infos.info)[i].valuePtr), (infos.info)[i].length);
        }

        //		CWDebugLog("WTP Descriptor Vendor ID: %d", (infos.vendorInfos)[i].vendorIdentifier);
        //		CWDebugLog("WTP Descriptor Type: %d", (infos.vendorInfos)[i].type);
        //		CWDebugLog("WTP Descriptor Length: %d", (infos.vendorInfos)[i].length);
        //		CWDebugLog("WTP Descriptor Value: %d", *((infos.vendorInfos)[i].valuePtr));

        //CWDebugLog("Vendor Info \"%d\" = %d - %d - %d", i, (infos.vendorInfos)[i].vendorIdentifier, (infos.vendorInfos)[i].type, (infos.vendorInfos)[i].length);
    }

    CWWTPDestroyVendorInfos(&infos);

    return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_WTP_DESCRIPTOR_CW_TYPE);
}

CWBool CWAssembleMsgElemWTPFrameTunnelMode(CWProtocolMessage *msgPtr)
{
    if(msgPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    // create message
    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, 1, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    //	CWDebugLog("Frame Tunnel Mode: %d", CWWTPGetFrameTunnelMode());
    CWProtocolStore8(msgPtr, CWWTPGetFrameTunnelMode()); // frame encryption

    return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_WTP_FRAME_TUNNEL_MODE_CW_TYPE);
}

CWBool CWAssembleMsgElemWTPIPv4Address(CWProtocolMessage *msgPtr)
{
    if(msgPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    // create message
    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, 4, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    //	CWDebugLog("WTP IPv4 Address: %d", CWWTPGetIPv4Address());
    CWProtocolStore32(msgPtr, CWWTPGetIPv4Address());

    return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_CW_LOCAL_IPV4_ADDRESS_CW_TYPE);
}

CWBool CWAssembleMsgElemWTPMACType(CWProtocolMessage *msgPtr)
{
    if(msgPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    // create message
    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, 1, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    //	CWDebugLog("WTP MAC Type: %d", CWWTPGetMACType());
    CWProtocolStore8(msgPtr, CWWTPGetMACType()); // mode of operation of the WTP (local, split, ...)

    return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_WTP_MAC_TYPE_CW_TYPE);
}

CWBool CWAssembleMsgElemWTPName(CWProtocolMessage *msgPtr)
{
    char *name;

    if(msgPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    if(!CWWTPGetName(&name))
    {
        return CW_FALSE;
    }

    // create message
    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, strlen(name),
    {
        CW_FREE_OBJECT(name);
        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
    });

    //	CWDebugLog("WTPName: %s", name);
    CWProtocolStoreStr(msgPtr, name);
    CW_FREE_OBJECT(name);
    return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_WTP_NAME_CW_TYPE);
}

#ifdef CW_WTP_AP
CWBool CWAssembleMsgElemWTPRadioStatistics(CWProtocolMessage *msgPtr, int radio)
{
    const int radio_statistics_length = 20;

    if(msgPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    if(radio < 0 || radio > gRadiosInfo.radioCount)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, radio_statistics_length, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    CWProtocolStore8(msgPtr, radio);
    CWProtocolStore8(msgPtr, gRadiosInfo.info[radio].statistics.lastFailureType);
    CWProtocolStore16(msgPtr, gRadiosInfo.info[radio].statistics.resetCount);
    CWProtocolStore16(msgPtr, gRadiosInfo.info[radio].statistics.SWFailureCount);
    CWProtocolStore16(msgPtr, gRadiosInfo.info[radio].statistics.HWFailuireCount);
    CWProtocolStore16(msgPtr, gRadiosInfo.info[radio].statistics.otherFailureCount);
    CWProtocolStore16(msgPtr, gRadiosInfo.info[radio].statistics.unknownFailureCount);
    CWProtocolStore16(msgPtr, gRadiosInfo.info[radio].statistics.configUpdateCount);
    CWProtocolStore16(msgPtr, gRadiosInfo.info[radio].statistics.channelChangeCount);
    CWProtocolStore16(msgPtr, gRadiosInfo.info[radio].statistics.bandChangeCount);
    CWProtocolStore16(msgPtr, gRadiosInfo.info[radio].statistics.currentNoiseFloor);

    //	CWDebugLog("");
    //	CWDebugLog("WTPRadioStatistics of radio: \"%d\"", radio);
    //	CWDebugLog("WTPRadioStatistics(1): %d - %d - %d", gRadiosInfo.radiosInfo[radio].statistics.lastFailureType, gRadiosInfo.radiosInfo[radio].statistics.resetCount, gRadiosInfo.radiosInfo[radio].statistics.SWFailureCount);
    //	CWDebugLog("WTPRadioStatistics(2): %d - %d - %d", gRadiosInfo.radiosInfo[radio].statistics.HWFailuireCount, gRadiosInfo.radiosInfo[radio].statistics.otherFailureCount, gRadiosInfo.radiosInfo[radio].statistics.unknownFailureCount);
    //	CWDebugLog("WTPRadioStatistics(3): %d - %d - %d - %d", gRadiosInfo.radiosInfo[radio].statistics.configUpdateCount, gRadiosInfo.radiosInfo[radio].statistics.channelChangeCount,gRadiosInfo.radiosInfo[radio].statistics.bandChangeCount,gRadiosInfo.radiosInfo[radio].statistics.currentNoiseFloor);

    //return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_WTP_RADIO_STATISTICS_CW_TYPE);

    CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_WTP_RADIO_STATISTICS_CW_TYPE);

    return CW_TRUE;
}
#endif /* CW_WTP_AP */

CWBool CWAssembleMsgElemWTPRebootStatistics(CWProtocolMessage *msgPtr)
{
    const int reboot_statistics_length = 15;

    CWDebugLog("CWAssembleMsgElemWTPRebootStatistics...");

    if(msgPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, reboot_statistics_length, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    CWProtocolStore16(msgPtr, gWTPRebootStatistics.rebootCount);
    CWProtocolStore16(msgPtr, gWTPRebootStatistics.ACInitiatedCount);
    CWProtocolStore16(msgPtr, gWTPRebootStatistics.linkFailurerCount);
    CWProtocolStore16(msgPtr, gWTPRebootStatistics.SWFailureCount);
    CWProtocolStore16(msgPtr, gWTPRebootStatistics.HWFailuireCount);
    CWProtocolStore16(msgPtr, gWTPRebootStatistics.otherFailureCount);
    CWProtocolStore16(msgPtr, gWTPRebootStatistics.unknownFailureCount);
    CWProtocolStore8(msgPtr, gWTPRebootStatistics.lastFailureType);

    //	CWDebugLog("");
    //	CWDebugLog("WTPRebootStat(1): %d - %d - %d", gWTPRebootStatistics.rebootCount, gWTPRebootStatistics.ACInitiatedCount, gWTPRebootStatistics.linkFailurerCount);
    //	CWDebugLog("WTPRebootStat(2): %d - %d - %d", gWTPRebootStatistics.SWFailureCount, gWTPRebootStatistics.HWFailuireCount, gWTPRebootStatistics.otherFailureCount);
    //	CWDebugLog("WTPRebootStat(3): %d - %d", gWTPRebootStatistics.unknownFailureCount, gWTPRebootStatistics.lastFailureType);

    return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_WTP_REBOOT_STATISTICS_CW_TYPE);
}

//test version
CWBool CWAssembleMsgElemDuplicateIPv4Address(CWProtocolMessage *msgPtr)
{
    const int duplicate_ipv4_length = 11;
    char *macAddress;

    if(msgPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    // create message
    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, duplicate_ipv4_length, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    //	CWDebugLog("");
    //	CWDebugLog("Duplicate IPv4 Address: %d", CWWTPGetIPv4Address());

    CWProtocolStore32(msgPtr, CWWTPGetIPv4Address());

    CWProtocolStore8(msgPtr, CWWTPGetIPv4StatusDuplicate());

    CWProtocolStore8(msgPtr, 6);

    CW_CREATE_ARRAY_ERR(macAddress, 6, char, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
    macAddress[0] = 103;
    macAddress[1] = 204;
    macAddress[2] = 204;
    macAddress[3] = 190;
    macAddress[4] = 180;
    macAddress[5] = 0;

    CWProtocolStoreRawBytes(msgPtr, macAddress, 6);
    CW_FREE_OBJECT(macAddress);

    //CWProtocolStore8(msgPtr, CWWTPGetIPv4StatusDuplicate());

    return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_DUPLICATE_IPV4_ADDRESS_CW_TYPE);
}

//test version
CWBool CWAssembleMsgElemDuplicateIPv6Address(CWProtocolMessage *msgPtr)
{
    const int duplicate_ipv6_length = 23;
    char *macAddress;

    if(msgPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    // create message
    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, duplicate_ipv6_length, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    //	CWDebugLog("");
    //	CWDebugLog("Duplicate IPv6 Address");

    struct sockaddr_in6 myAddr;
    CWWTPGetIPv6Address(&myAddr);
    CWProtocolStoreRawBytes(msgPtr, (char *)myAddr.sin6_addr.s6_addr, 16);

    CWProtocolStore8(msgPtr, CWWTPGetIPv6StatusDuplicate());

    CWProtocolStore8(msgPtr, 6);

    CW_CREATE_ARRAY_ERR(macAddress, 6, char, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
    macAddress[0] = 103;
    macAddress[1] = 204;
    macAddress[2] = 204;
    macAddress[3] = 190;
    macAddress[4] = 180;
    macAddress[5] = 0;

    CWProtocolStoreRawBytes(msgPtr, macAddress, 6);
    CW_FREE_OBJECT(macAddress);

    //CWProtocolStore8(msgPtr, CWWTPGetIPv6StatusDuplicate());

    return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_DUPLICATE_IPV6_ADDRESS_CW_TYPE);
}

#ifdef CW_WTP_AP
CWBool CWAssembleMsgElemRadioAdminState(CWProtocolMessage *msgPtr)
{
    const int radio_Admin_State_Length = 2;
    CWRadiosAdminInfo infos;
    CWProtocolMessage *msgs;
    int len = 0;
    int i;
    int j;

    CWDebugLog("CWAssembleMsgElemRadioAdminState...");

    if(msgPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    if(!CWGetWTPRadiosAdminState(&infos))
    {
        return CW_FALSE;
    }

    CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgs, infos.radiosCount, return  CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    for(i = 0; i < infos.radiosCount; i++)
    {
        // create message
        CW_CREATE_PROTOCOL_MESSAGE(msgs[i], radio_Admin_State_Length, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
        CWProtocolStore8(&(msgs[i]), infos.radios[i].ID); // ID of the radio
        CWProtocolStore8(&(msgs[i]), infos.radios[i].state); // state of the radio
        //CWProtocolStore8(&(msgs[i]), infos.radios[i].cause);

        if(!(CWAssembleMsgElem(&(msgs[i]), CW_MSG_ELEMENT_RADIO_ADMIN_STATE_CW_TYPE)))
        {
            for(j = i; j >= 0; j--)
            {
                CW_FREE_PROTOCOL_MESSAGE(msgs[j]);
            }
            CW_FREE_OBJECT(infos.radios);
            CW_FREE_OBJECT(msgs);
            return CW_FALSE;
        }

        len += msgs[i].offset;
        //		CWDebugLog("Radio Admin State: %d - %d - %d", infos.radios[i].ID, infos.radios[i].state, infos.radios[i].cause);
    }

    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, len,
    {
        for(j = i - 1; j >= 0; j--)
        {
            CW_FREE_PROTOCOL_MESSAGE(msgs[j]);
        }
        CW_FREE_OBJECT(infos.radios);
        CW_FREE_OBJECT(msgs);
        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
    });

    for(i = 0; i < infos.radiosCount; i++)
    {
        CWProtocolStoreMessage(msgPtr, &(msgs[i]));
        CW_FREE_PROTOCOL_MESSAGE(msgs[i]);
    }

    CW_FREE_OBJECT(infos.radios);
    CW_FREE_OBJECT(msgs);

    return CW_TRUE;
}

//if radioID is negative return Radio Operational State for all radios
CWBool CWAssembleMsgElemRadioOperationalState(int radioID, CWProtocolMessage *msgPtr)
{
    const int radio_Operational_State_Length = 3;
    CWRadiosOperationalInfo infos;
    CWProtocolMessage *msgs;
    int len = 0;
    int i;

    if(msgPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    if(!(CWGetWTPRadiosOperationalState(radioID, &infos)))
    {
        return CW_FALSE;
    }

    /*added by larger*/
    if(infos.radiosCount == 0)
    {
        return CW_TRUE;
    }



    CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgs, (infos.radiosCount), return  CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    for(i = 0; i < infos.radiosCount; i++)
    {
        // create message
        CW_CREATE_PROTOCOL_MESSAGE(msgs[i], radio_Operational_State_Length, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
        CWProtocolStore8(&(msgs[i]), infos.radios[i].ID); // ID of the radio
        CWProtocolStore8(&(msgs[i]), infos.radios[i].state); // state of the radio
        CWProtocolStore8(&(msgs[i]), infos.radios[i].cause);

        if(!(CWAssembleMsgElem(&(msgs[i]), CW_MSG_ELEMENT_RADIO_OPERAT_STATE_CW_TYPE)))
        {
            int j;
            for(j = i; j >= 0; j--)
            {
                CW_FREE_PROTOCOL_MESSAGE(msgs[j]);
            }
            CW_FREE_OBJECT(infos.radios);
            CW_FREE_OBJECT(msgs);
            return CW_FALSE;
        }

        len += msgs[i].offset;
        //		CWDebugLog("Radio Operational State: %d - %d - %d", infos.radios[i].ID, infos.radios[i].state, infos.radios[i].cause);
    }

    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, len, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    for(i = 0; i < infos.radiosCount; i++)
    {
        CWProtocolStoreMessage(msgPtr, &(msgs[i]));
        CW_FREE_PROTOCOL_MESSAGE(msgs[i]);
    }

    CW_FREE_OBJECT(msgs);
    CW_FREE_OBJECT(infos.radios);

    return CW_TRUE;
}

CWBool CWAssembleMsgElemDecryptErrorReport(CWProtocolMessage *msgPtr, int radioID)
{
    int decrypy_Error_Report_Length = 0;
    CWDecryptErrorReportInfo infos;
    CWProtocolMessage *msgs;
    int len = 0;
    int i;

    if(msgPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    if(!(CWGetDecryptErrorReport(radioID, &infos)))
    {
        return CW_FALSE;
    }

    CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgs, (infos.radiosCount),
    {
        CWDestroyDecryptErrorReport(&infos);
        return  CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
    });

    for(i = 0; i < infos.radiosCount; i++)
    {
        // create message
        decrypy_Error_Report_Length = 2 + sizeof(CWMacAddress) * (infos.radios[i].numEntries);

        CW_CREATE_PROTOCOL_MESSAGE(msgs[i], decrypy_Error_Report_Length, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
        CWProtocolStore8(&(msgs[i]), infos.radios[i].ID); // ID of the radio
        CWProtocolStore8(&(msgs[i]), infos.radios[i].numEntries); // state of the radio

        CWProtocolStore8(&(msgs[i]), (unsigned char)sizeof(CWMacAddress) * (infos.radios[i].numEntries));

        CWProtocolStoreRawBytes(&(msgs[i]), (char *) * (infos.radios[i].errorMACAddressList), sizeof(CWMacAddress) * (infos.radios[i].numEntries));

        /*
        CWDebugLog("###numEntries = %d", infos.radios[i].numEntries);
        CWDebugLog("j = %d", sizeof(CWMacAddress)*(infos.radios[i].numEntries));

        int j;
        for (j=(sizeof(CWMacAddress)*(infos.radios[i].numEntries)); j>0; j--)
        	CWDebugLog("##(%d/6) = %d", j, msgs[i].msg[(msgs[i].offset)-j]);
        */

        if(!(CWAssembleMsgElem(&(msgs[i]), CW_MSG_ELEMENT_CW_DECRYPT_ER_REPORT_CW_TYPE)))
        {
            int j;
            for(j = i; j >= 0; j--)
            {
                CW_FREE_PROTOCOL_MESSAGE(msgs[j]);
            }
            CW_FREE_OBJECT(msgs);
            CWDestroyDecryptErrorReport(&infos);
            return CW_FALSE;
        }

        len += msgs[i].offset;
        //		CWDebugLog("Radio Decrypt Error Report of radio \"%d\" = %d", infos.radios[i].ID, infos.radios[i].numEntries);
    }

    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, len,
    {
        for(i = 0; i < infos.radiosCount; i++)
        {
            CW_FREE_PROTOCOL_MESSAGE(msgs[i]);
        }
        CW_FREE_OBJECT(msgs);
        CWDestroyDecryptErrorReport(&infos);
        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
    });

    for(i = 0; i < infos.radiosCount; i++)
    {
        CWProtocolStoreMessage(msgPtr, &(msgs[i]));
        CW_FREE_PROTOCOL_MESSAGE(msgs[i]);
    }

    CW_FREE_OBJECT(msgs);
    CWDestroyDecryptErrorReport(&infos);

    return CW_TRUE;

}

CWBool CWAssembleMsgElemWTPRadioInformation(CWProtocolMessage *msgPtr)
{
    CWProtocolMessage *msgs = NULL;
    CWRadiosInformation radioInfo;
    int len = 0;
    int i, j;

    if(!CWWTPGetRadiosInfomation(&radioInfo))
    {
        return CW_FALSE;
    }

    // create one message element for each radio
    CW_CREATE_ZERO_ARRAY_ERR(msgs, radioInfo.radiosCount, CWProtocolMessage,
    {
        CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
        goto err_exit;
    });

    for(i = 0; i < radioInfo.radiosCount; i++)
    {
        // create message
        CW_CREATE_PROTOCOL_MESSAGE(msgs[i], 1 + 1 + 4 + 1 + 6 + 1 + (6 * radioInfo.radios[i].maxWlans),
        {
            CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
            goto err_exit;
        });
        CWProtocolStore8(&(msgs[i]), 3); /* SENAO ADD: version of the radio info, for backward-compatiable, this version should start from 3 */
        CWProtocolStore8(&(msgs[i]), radioInfo.radios[i].ID); // ID of the radio
        CWProtocolStore32(&(msgs[i]), radioInfo.radios[i].type); // type of the radio
        CWProtocolStore8(&(msgs[i]), (unsigned char) radioInfo.radios[i].maxTxPower); /* SENAO ADD: maximun TX Power of the radio */
        CWProtocolStoreRawBytes(&(msgs[i]), (char *) radioInfo.radios[i].mac, 6); /* SENAO ADD: radio mac */
        CWProtocolStore8(&(msgs[i]), (unsigned char) radioInfo.radios[i].maxWlans); /* SENAO ADD: maximun number of wlans */
        for(j = 0; j < radioInfo.radios[i].maxWlans; j++)
        {
            CWProtocolStoreRawBytes(&(msgs[i]), (char *) radioInfo.radios[i].wlans[j].bssid, 6); /* SENAO ADD: wlan bssid */
        }
        if(!(CWAssembleMsgElem(&(msgs[i]), CW_MSG_ELEMENT_WTP_RADIO_INFO_CW_TYPE)))
        {
            goto err_exit;
        }
        len += msgs[i].offset;
    }

    // return all the messages as one big message
    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, len,
    {
        CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
        goto err_exit;
    });

    for(i = 0; i < radioInfo.radiosCount; i++)
    {
        CWProtocolStoreMessage(msgPtr, &(msgs[i]));

        /* free memory in the final loop */
        CW_FREE_OBJECT(radioInfo.radios[i].wlans);
        CW_FREE_PROTOCOL_MESSAGE(msgs[i]);
    }

    CW_FREE_OBJECT(radioInfo.radios);
    CW_FREE_OBJECT(msgs);

    return CW_TRUE;

err_exit:

    for(i = 0; i < radioInfo.radiosCount; i++)
    {
        CW_FREE_OBJECT(radioInfo.radios[i].wlans);
        CW_FREE_PROTOCOL_MESSAGE(msgs[i]);
    }
    CW_FREE_OBJECT(radioInfo.radios);
    CW_FREE_OBJECT(msgs);

    return CW_FALSE;
}

CWBool CWWTPAssembleMsgElemSitesurveyInfo(CWProtocolMessage *msgPtr, CWWTPSitesurveyInfo *sitesurveyInfo)
{
    int surveyIdx, len, i;
    unsigned int capCode;
    int radioIdx;

    if(!CWWTPBoardGetCapCode(&capCode))
    {
        return CW_FALSE;
    }

    if(!CWWTPGetRadioIndex(sitesurveyInfo->radio, &radioIdx))
    {
        return CW_FALSE;
    }

    if(!CWWTPCheckSitesurveyDoing(sitesurveyInfo->radio))
    {
        CWDebugLog("sitesurvey is not running");
        return CWErrorRaise(CW_ERROR_OPERATION_ABORTED, NULL);
    }

    if(sitesurveyInfo->version == 0)
    {
        len = 1 + 4 + 4 + 1 + 2 + (sitesurveyInfo->infoCount * (6 + 1 + 33 + 1 + 4 + 4 + 1 + 1));
    }
    else if(sitesurveyInfo->version == 1)
    {
        if(sitesurveyInfo->radio == CW_RADIOFREQTYPE_2G)
        {
            len = 1 + 4 + 4 + 1 + 2 + (sitesurveyInfo->infoCount * (6 + 1 + 33 + 1 + 4 + 4 + 1 + 1 + 4 + 4)) + (CW_WIRELESS_RADIO_2G_UTILIZATION_FIELD_SIZE);
        }
        else
        {
            len = 1 + 4 + 4 + 1 + 2 + (sitesurveyInfo->infoCount * (6 + 1 + 33 + 1 + 4 + 4 + 1 + 1 + 4 + 4)) + (CW_WIRELESS_RADIO_5G_UTILIZATION_FIELD_SIZE);
        }
    }
    else
    {
        CWDebugLog("*** unknown sitesurvey version");
        return CWErrorRaise(CW_ERROR_OPERATION_ABORTED, NULL);
    }

    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, len,
    {
        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
    });

    CWProtocolStore8(msgPtr, (unsigned char) sitesurveyInfo->radio); // 1
    CWProtocolStore8(msgPtr, sitesurveyInfo->version); // 1
    CWProtocolStore32(msgPtr, sitesurveyInfo->curChannel); // 4
    CWProtocolStore32(msgPtr, sitesurveyInfo->curTxPower); // 4
    CWProtocolStore16(msgPtr, (unsigned short) sitesurveyInfo->infoCount); // 2
    for(surveyIdx = 0; surveyIdx < sitesurveyInfo->infoCount; surveyIdx++)
    {
        CWProtocolStoreRawBytes(msgPtr, (char *) sitesurveyInfo->info[surveyIdx].bssid, 6); // 6
        CWProtocolStore8(msgPtr, sitesurveyInfo->info[surveyIdx].ssidLen); // 1
        CWProtocolStoreRawBytes(msgPtr, sitesurveyInfo->info[surveyIdx].ssid, 33); // 33
        CWProtocolStore8(msgPtr, (unsigned char) sitesurveyInfo->info[surveyIdx].mode); // 1
        CWProtocolStore32(msgPtr, sitesurveyInfo->info[surveyIdx].chan); // 4
        CWProtocolStore32(msgPtr, sitesurveyInfo->info[surveyIdx].signal); // 4
        CWProtocolStore8(msgPtr, (unsigned char) sitesurveyInfo->info[surveyIdx].enc); // 1
        CWProtocolStore8(msgPtr, (unsigned char) sitesurveyInfo->info[surveyIdx].type); // 1

        if(sitesurveyInfo->version == 1)
        {
            CWProtocolStore32(msgPtr, sitesurveyInfo->info[surveyIdx].htmode); // 4
            CWProtocolStore32(msgPtr, sitesurveyInfo->info[surveyIdx].extch); // 4
        }
    }

    if(sitesurveyInfo->version == 1)
    {
        for(i = 0; i < ((sitesurveyInfo->radio == CW_RADIOFREQTYPE_2G)?(CW_WIRELESS_RADIO_2G_UTILIZATION_FIELD_SIZE):(CW_WIRELESS_RADIO_5G_UTILIZATION_FIELD_SIZE)); i++)
        {
            CWProtocolStore8(msgPtr, sitesurveyInfo->chanUtil[i]); // 1
#if 0
            CWLog("chanUtil[%u]=%u", i, sitesurveyInfo->chanUtil[i]);
#endif
        }
    }

    CWDebugLog("Sitesurvey Radio: %s EntryNum: %d", (sitesurveyInfo->radio == CW_RADIOFREQTYPE_2G ? "2.4G" : sitesurveyInfo->radio == CW_RADIOFREQTYPE_5G ? "5G":"5-1G"),
               sitesurveyInfo->infoCount);

    return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_WTP_EVENT_SITESURVEY_CW_TYPE);
}

CWBool CWWTPAssembleMsgElemAutoChannelChangeInfo(CWProtocolMessage *msgPtr, CWWTPAutoChannelInfo *chanChangedInfo)
{
    int len = 1 + 4 + 4;

    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, len,
    {
        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
    });

    CWProtocolStore8(msgPtr, (unsigned char)(chanChangedInfo->radioType)); // 1
    CWProtocolStore32(msgPtr, chanChangedInfo->oldChannel); // 4
    CWProtocolStore32(msgPtr, chanChangedInfo->newChannel); // 4

    CWDebugLog("auto channel change radio: %s from: %d to %d",
               (chanChangedInfo->radioType == CW_RADIOFREQTYPE_5G ? "5G" : chanChangedInfo->radioType == CW_RADIOFREQTYPE_2G ?"2.4G":"5-1G"),
               chanChangedInfo->oldChannel,
               chanChangedInfo->newChannel);

    return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_WTP_EVENT_AUTOCHANNEL_CW_TYPE);
}

CWBool CWWTPAssembleMsgElemAutoTxPowerCurrentValue(CWProtocolMessage *msgPtr, CWRadioFreqType radioType)
{
    unsigned int capCode;
    int radioIdx;
    int curPower = 0;

    if(msgPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    CWWTPBoardGetCapCode(&capCode);
    CWGetRadioIndex(radioType, capCode, &radioIdx);
    CWWTPBoardGetRadioCurrentTxPower(radioIdx, &curPower);

    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, 4 + 2 + 4,
    {
        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
    });

    CWProtocolStore32(msgPtr, CW_VENDOR_ID);
    CWProtocolStore16(msgPtr, (unsigned short) CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_AUTO_TXPOWER_RESPONSE);
    CWProtocolStore32(msgPtr, curPower);

    return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE);
}
#endif /* CW_WTP_AP */

CWBool CWWTPAssembleMsgElemLogMsg(CWProtocolMessage *msgPtr, int group, const char *category, int level, const char *msg)
{
    int cateLen, msgLen;

    if(msgPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    cateLen = strlen(category);
    msgLen = strlen(msg);

    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, 1 + 1 + cateLen + 1 + 2 + msgLen,
    {
        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
    });

    CWProtocolStore8(msgPtr, (unsigned char) group);
    CWProtocolStore8(msgPtr, (unsigned char) cateLen);
    CWProtocolStoreRawBytes(msgPtr, category, cateLen);
    CWProtocolStore8(msgPtr, (unsigned char) level);
    CWProtocolStore16(msgPtr, msgLen);
    CWProtocolStoreRawBytes(msgPtr, msg, msgLen);

    return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_WTP_EVENT_LOG_MSG_CW_TYPE);
}

/*_________________________________________________________________________*/
/*  *****************************___PARSE___*****************************  */

CWBool CWParseACDescriptor(CWProtocolMessage *msgPtr, int len, CWACInfoValues *valPtr)
{
    int i = 0, theOffset = 0;

    CW_PARSE_MSG_ELEMMENT_START();

    valPtr->stations = CWProtocolRetrieve16(msgPtr);
    //	CWDebugLog("AC Descriptor Stations: %d", valPtr->stations);

    valPtr->limit	= CWProtocolRetrieve16(msgPtr);
    //	CWDebugLog("AC Descriptor Limit: %d", valPtr->limit);

    valPtr->activeWTPs = CWProtocolRetrieve16(msgPtr);
    //	CWDebugLog("AC Descriptor Active WTPs: %d", valPtr->activeWTPs);

    valPtr->maxWTPs	= CWProtocolRetrieve16(msgPtr);
    //	CWDebugLog("AC Descriptor Max WTPs: %d",	valPtr->maxWTPs);

    valPtr->security = CWProtocolRetrieve8(msgPtr);
    //	CWDebugLog("AC Descriptor Security: %d",	valPtr->security);

    valPtr->RMACField = CWProtocolRetrieve8(msgPtr);
    //	CWDebugLog("AC Descriptor Radio MAC Field: %d",	valPtr->security);

    //	valPtr->WirelessField= CWProtocolRetrieve8(msgPtr);
    //	CWDebugLog("AC Descriptor Wireless Field: %d",	valPtr->security);

    CWProtocolRetrieve8(msgPtr);			//Reserved

    valPtr->DTLSPolicy = CWProtocolRetrieve8(msgPtr); // DTLS Policy
    //	CWDebugLog("DTLS Policy: %d",	valPtr->DTLSPolicy);

    valPtr->vendorInfos.vendorInfosCount = 0;

    theOffset = msgPtr->offset;

    // see how many vendor ID we have in the message
    while((msgPtr->offset - oldOffset) < len)  	// oldOffset stores msgPtr->offset's value at the beginning of this function.
    {
        // See the definition of the CW_PARSE_MSG_ELEMMENT_START() macro.
        int tmp, id = 0, type = 0;

        //CWDebugLog("differenza:%d, offset:%d, oldOffset:%d", (msgPtr->offset-oldOffset), (msgPtr->offset), oldOffset);

        id = CWProtocolRetrieve32(msgPtr);
        CWDebugLog("ID: %d", id); // ID

        type = CWProtocolRetrieve16(msgPtr);
        CWDebugLog("TYPE: %d", type); // type

        tmp = CWProtocolRetrieve16(msgPtr);
        msgPtr->offset += tmp; // len
        //		CWDebugLog("offset %d", msgPtr->offset);
        valPtr->vendorInfos.vendorInfosCount++;
    }

    msgPtr->offset = theOffset;

    // actually read each vendor ID
    CW_CREATE_ARRAY_ERR(valPtr->vendorInfos.vendorInfos, valPtr->vendorInfos.vendorInfosCount, CWACVendorInfoValues,
                        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
    //	CWDebugLog("len %d", len);
    //	CWDebugLog("vendorInfosCount %d", valPtr->vendorInfos.vendorInfosCount);
    for(i = 0; i < valPtr->vendorInfos.vendorInfosCount; i++)
    {
        //		CWDebugLog("vendorInfosCount %d vs %d", i, valPtr->vendorInfos.vendorInfosCount);
        (valPtr->vendorInfos.vendorInfos)[i].vendorIdentifier = CWProtocolRetrieve32(msgPtr);
        (valPtr->vendorInfos.vendorInfos)[i].type = CWProtocolRetrieve16(msgPtr);
        (valPtr->vendorInfos.vendorInfos)[i].length = CWProtocolRetrieve16(msgPtr);
        if((valPtr->vendorInfos.vendorInfos)[i].length)
        {
            (valPtr->vendorInfos.vendorInfos)[i].valuePtr = (int *)(CWProtocolRetrieveRawBytes(msgPtr, (valPtr->vendorInfos.vendorInfos)[i].length));
            if((valPtr->vendorInfos.vendorInfos)[i].valuePtr == NULL)
            {
                return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
            }
        }
        else
        {
            (valPtr->vendorInfos.vendorInfos)[i].valuePtr = NULL;
        }
#if 0
        if((valPtr->vendorInfos.vendorInfos)[i].length == 4)
        {
            *((valPtr->vendorInfos.vendorInfos)[i].valuePtr) = ntohl(*((valPtr->vendorInfos.vendorInfos)[i].valuePtr));
        }
#endif

        //		CWDebugLog("AC Descriptor Vendor ID: %d", (valPtr->vendorInfos.vendorInfos)[i].vendorIdentifier);
        //		CWDebugLog("AC Descriptor Type: %d", (valPtr->vendorInfos.vendorInfos)[i].type);
        //		CWDebugLog("AC Descriptor Value: %d", *((valPtr->vendorInfos.vendorInfos)[i].valuePtr));
    }
    //	CWDebugLog("AC Descriptor Out");
    CW_PARSE_MSG_ELEMENT_END();
}

CWBool CWParseACIPv4List(CWProtocolMessage *msgPtr, int len, ACIPv4ListValues *valPtr)
{
    int i;
    CW_PARSE_MSG_ELEMMENT_START();

    if(len == 0 || ((len % 4) != 0))
    {
        return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Malformed AC IPv4 List Messame Element");
    }

    valPtr->ACIPv4ListCount = (len / 4);

    CW_CREATE_ARRAY_ERR(valPtr->ACIPv4List, valPtr->ACIPv4ListCount, int, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    for(i = 0; i < valPtr->ACIPv4ListCount; i++)
    {
        (valPtr->ACIPv4List)[i] = CWProtocolRetrieve32(msgPtr);
    }

    CW_PARSE_MSG_ELEMENT_END();
}

CWBool CWParseACIPv6List(CWProtocolMessage *msgPtr, int len, ACIPv6ListValues *valPtr)
{
    int i;
    void *ptr;
    CW_PARSE_MSG_ELEMMENT_START();

    if(len == 0 || ((len % 16) != 0))
    {
        return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Malformed AC IPv6 List Messame Element");
    }

    valPtr->ACIPv6ListCount = (len / 16);

    CW_CREATE_ARRAY_ERR(valPtr->ACIPv6List, valPtr->ACIPv6ListCount, struct in6_addr, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    for(i = 0; i < valPtr->ACIPv6ListCount; i++)
    {
        ptr =  CWProtocolRetrieveRawBytes(msgPtr, 16);
        if(ptr)
        {
            CW_COPY_MEMORY(&((valPtr->ACIPv6List)[i]), ptr, 16);
            CW_FREE_OBJECT(ptr);
        }
        //		CWUseSockNtop(&addr, CWDebugLog("AC IPv6 List: %s",str););
    }

    CW_PARSE_MSG_ELEMENT_END();
}

CWBool CWParseAddStation(CWProtocolMessage *msgPtr, int len)
{
    int radioID = 0, Length = 0;
    unsigned char *StationMacAddress;

    //CW_PARSE_MSG_ELEMMENT_START();	 sostituire al posto delle righe successive quando passerò valPtr alla funzione CWarseAddStation
    /*--------------------------------------------------------------------------------------*/
    int oldOffset;
    if(msgPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }
    oldOffset = msgPtr->offset;
    /*----------------------------------------------------------------------------------*/

    radioID = CWProtocolRetrieve8(msgPtr);
    CWDebugLog("radio ID %d", radioID);
    Length = CWProtocolRetrieve8(msgPtr);
    CWDebugLog("Length of mac address field %d", Length);
    StationMacAddress = (unsigned char *)CWProtocolRetrieveRawBytes(msgPtr, Length);

    CWDebugLog("STATION'S MAC ADDRESS TO FORWARD TRAFFIC: %02X:%02X:%02X:%02X:%02X:%02X",
               StationMacAddress[0] & 0xFF,
               StationMacAddress[1] & 0xFF,
               StationMacAddress[2] & 0xFF,
               StationMacAddress[3] & 0xFF,
               StationMacAddress[4] & 0xFF,
               StationMacAddress[5] & 0xFF);

    CW_PARSE_MSG_ELEMENT_END();
}

CWBool CWParseCWControlIPv4Addresses(CWProtocolMessage *msgPtr, int len, CWProtocolIPv4NetworkInterface *valPtr)
{
    CW_PARSE_MSG_ELEMMENT_START();

    valPtr->addr.sin_addr.s_addr = htonl(CWProtocolRetrieve32(msgPtr));
    valPtr->addr.sin_family = AF_INET;
    valPtr->addr.sin_port = htons(CW_CONTROL_PORT);

    CWUseSockNtop((&(valPtr->addr)), CWDebugLog("Interface Address: %s", str););

    valPtr->WTPCount = CWProtocolRetrieve16(msgPtr);
    //	CWDebugLog("WTP Count: %d",	valPtr->WTPCount);

    CW_PARSE_MSG_ELEMENT_END();
}

CWBool CWParseCWControlIPv6Addresses(CWProtocolMessage *msgPtr, int len, CWProtocolIPv6NetworkInterface *valPtr)
{
    CW_PARSE_MSG_ELEMMENT_START();

    CW_COPY_MEMORY(&(valPtr->addr.sin6_addr), CWProtocolRetrieveRawBytes(msgPtr, 16), 16);
    valPtr->addr.sin6_family = AF_INET6;
    valPtr->addr.sin6_port = htons(CW_CONTROL_PORT);

    CWUseSockNtop((&(valPtr->addr)), CWDebugLog("Interface Address: %s", str););

    valPtr->WTPCount = CWProtocolRetrieve16(msgPtr);
    //	CWDebugLog("WTP Count: %d",	valPtr->WTPCount);

    CW_PARSE_MSG_ELEMENT_END();
}

CWBool CWParseCWTimers(CWProtocolMessage *msgPtr, int len, CWProtocolConfigureResponseValues *valPtr)
{
    CW_PARSE_MSG_ELEMMENT_START();

    valPtr->discoveryTimer	= CWProtocolRetrieve8(msgPtr);
    //	CWDebugLog("Discovery Timer: %d", valPtr->discoveryTimer);
    valPtr->echoRequestTimer = CWProtocolRetrieve8(msgPtr);
    //	CWDebugLog("Echo Timer: %d", valPtr->echoRequestTimer);

    CW_PARSE_MSG_ELEMENT_END();
}

CWBool CWParseDecryptErrorReportPeriod(CWProtocolMessage *msgPtr, int len, WTPDecryptErrorReportValues *valPtr)
{
    CW_PARSE_MSG_ELEMMENT_START();

    valPtr->radioID = CWProtocolRetrieve8(msgPtr);
    valPtr->reportInterval = CWProtocolRetrieve16(msgPtr);
    //	CWDebugLog("Decrypt Error Report Period: %d - %d", valPtr->radioID, valPtr->reportInterval);

    CW_PARSE_MSG_ELEMENT_END();
}

CWBool CWParseIdleTimeout(CWProtocolMessage *msgPtr, int len, CWProtocolConfigureResponseValues *valPtr)
{
    CW_PARSE_MSG_ELEMMENT_START();

    valPtr->idleTimeout = CWProtocolRetrieve32(msgPtr);
    //	CWDebugLog("Idle Timeout: %d", valPtr->idleTimeout);

    CW_PARSE_MSG_ELEMENT_END();
}

CWBool CWParseWTPFallback(CWProtocolMessage *msgPtr, int len, CWProtocolConfigureResponseValues *valPtr)
{
    CW_PARSE_MSG_ELEMMENT_START();

    valPtr->fallback = CWProtocolRetrieve8(msgPtr);
    //	CWDebugLog("WTP Fallback: %d", valPtr->fallback);

    CW_PARSE_MSG_ELEMENT_END();
}

CWBool CWParseConnectionID(CWProtocolMessage *msgPtr, int len, int *valPtr)
{
    CW_PARSE_MSG_ELEMMENT_START();

    *valPtr = CWProtocolRetrieve32(msgPtr);

    CW_PARSE_MSG_ELEMENT_END();
}

void CWWTPResetRebootStatistics(WTPRebootStatisticsInfo *rebootStatistics)
{
    rebootStatistics->rebootCount = 0;
    rebootStatistics->ACInitiatedCount = 0;
    rebootStatistics->linkFailurerCount = 0;
    rebootStatistics->SWFailureCount = 0;
    rebootStatistics->HWFailuireCount = 0;
    rebootStatistics->otherFailureCount = 0;
    rebootStatistics->unknownFailureCount = 0;
    rebootStatistics->lastFailureType = NOT_SUPPORTED;
}


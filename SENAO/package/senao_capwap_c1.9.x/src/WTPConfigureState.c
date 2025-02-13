/*******************************************************************************************
 * Copyright (c) 2006-7 Laboratorio di Sistemi di Elaborazione e Bioingegneria Informatica *
 *                      Universita' Campus BioMedico - Italy                               *
 *                                                                                         *
 * This program is free software; you can redistribute it and/or modify it under the terms *
 * of the GNU General Public License as published by the Free Software Foundation; either  *
 * version 2 of the License, or (at your option) any later version.                        *
 *                                                                                         *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY         *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A 	   *
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
#include "CWVendorPayloads.h"
#include "WTPDataRecordStatistics.h"
#ifdef DMALLOC
#include "../dmalloc-5.5.0/dmalloc.h"
#endif

/* void CWWTPResponseTimerExpired(void *arg, CWTimerID id); */
CWBool CWAssembleConfigureRequest(CWProtocolMessage **messagesPtr,
                                  int *fragmentsNumPtr,
                                  int PMTU,
                                  int seqNum,
                                  void *valuesPtr);

CWBool CWParseConfigureResponseMessage(char *msg,
                                       int len,
                                       int seqNum,
                                       CWProtocolConfigureResponseValues *valuesPtr,
                                       int combineLen);

CWBool CWSaveConfigureResponseMessage(CWProtocolConfigureResponseValues *configureResponse);

void CWDestroyConfigureResponseValues(CWProtocolConfigureResponseValues *configureResponse);

/*_________________________________________________________*/
/*  *******************___FUNCTIONS___*******************  */

/*
 * Manage Configure State.
 */
CWStateTransition CWWTPEnterConfigure()
{
    int seqNum;
    CWProtocolConfigureResponseValues values;

    CWDebugLog("######### Configure State #########");

    /* send Configure Request */
    seqNum = CWGetSeqNum();

    if(!CWErr(CWWTPSendAcknowledgedPacket(seqNum,
                                          NULL,
                                          CWAssembleConfigureRequest,
                                          (void *)CWParseConfigureResponseMessage,
                                          (void *)CWSaveConfigureResponseMessage, &values,
                                          gACInfoPtr->timer->configState)))
    {
        return CW_ENTER_DISCOVERY;
    }

    return CW_ENTER_DATA_CHECK;
}

/*
void CWWTPResponseTimerExpired(void *arg, CWTimerID id)
{
	CWLog("WTP Response Configure Timer Expired");
	CWNetworkCloseSocket(gWTPSocket);
}
*/

/*
 * Send Configure Request on the active session.
 */
CWBool CWAssembleConfigureRequest(CWProtocolMessage **messagesPtr,
                                  int *fragmentsNumPtr,
                                  int PMTU,
                                  int seqNum,
                                  void* valuesPtr)
{

    CWProtocolMessage 	*msgElems = NULL;
    CWProtocolMessage 	*msgElemsBinding = NULL;
#ifdef CW_WTP_AP
    const int 		msgElemCount = 6;
#else
    const int 		msgElemCount = 5;
#endif
    const int 		msgElemBindingCount = 0;
    int k = -1;

    if(messagesPtr == NULL || fragmentsNumPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems,
                                     msgElemCount,
                                     return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    CWDebugLog("Assembling Configure Request...");
    /*Avoid the NAT session timeout*/
    CWWTPStartKeepAliveTask();

    /* Assemble Message Elements */
    if((!(CWAssembleMsgElemACName(&(msgElems[++k])))) ||
#ifdef CW_WTP_AP
       (!(CWAssembleMsgElemRadioAdminState(&(msgElems[++k])))) ||
#endif
       (!(CWAssembleMsgElemStatisticsTimer(&(msgElems[++k])))) ||
       (!(CWAssembleMsgElemWTPRebootStatistics(&(msgElems[++k])))) ||
       (!(CWAssembleMsgElemACNameWithPriority(&(msgElems[++k])))) ||
       (!(CWWTPAssembleMsgElemVendorPayloadWtpCfg(&(msgElems[++k]))))
      )
    {
        int i;
        for(i = 0; i <= k; i++)
        {
            CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);
        }
        CW_FREE_OBJECT(msgElems);
        CWWTPStopKeepAliveTask();
        /* error will be handled by the caller */
        return CW_FALSE;
    }
    CWWTPStopKeepAliveTask();

    if(!(CWAssembleMessage(messagesPtr,
                           fragmentsNumPtr,
                           PMTU,
                           seqNum,
                           CW_MSG_TYPE_VALUE_CONFIGURE_REQUEST,
                           msgElems,
                           msgElemCount,
                           msgElemsBinding,
                           msgElemBindingCount,
                           CW_PACKET_CRYPT
                          )))
    {
        return CW_FALSE;
    }

    CWDebugLog("Configure Request Assembled");
    return CW_TRUE;
}

CWBool CWParseConfigureResponseMessage(char *msg,
                                       int len,
                                       int seqNum,
                                       CWProtocolConfigureResponseValues *valuesPtr,
                                       int combineLen)
{
    CWControlHeaderValues 	controlVal;
    CWProtocolMessage 	completeMsg;
    CWProtocolVendorSpecificValues vendorValues;
    int 			offsetTillMessages;
    int 			i = 0;
    int 			j = 0;

    if(msg == NULL || valuesPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    CWDebugLog("Parsing Configure Response...");

    completeMsg.msg = msg;
    completeMsg.offset = 0;
    /*
    valuesPtr->echoRequestTimer=0;
    valuesPtr->radioOperationalInfoCount=0;
    valuesPtr->radiosDecryptErrorPeriod.radiosCount=0;
    valuesPtr->bindingValues = NULL;
    valuesPtr->ACIPv4ListInfo.ACIPv4ListCount=0;
    valuesPtr->ACIPv4ListInfo.ACIPv4List=NULL;
    valuesPtr->ACIPv6ListInfo.ACIPv6ListCount=0;
    valuesPtr->ACIPv6ListInfo.ACIPv6List=NULL;
    valuesPtr->staticIPInfo = NULL;
    */
    CW_ZERO_MEMORY(valuesPtr, sizeof(CWProtocolConfigureResponseValues));
    valuesPtr->clientStateChgEventEnable = CW_TRUE; /* default is enable if AC doesn't provide it */

    /* error will be handled by the caller */
    if(!(CWParseControlHeader(&completeMsg, &controlVal)))
    {
        return CW_FALSE;
    }

    /* different type */
    if(controlVal.messageTypeValue != CW_MSG_TYPE_VALUE_CONFIGURE_RESPONSE)
        return CWErrorRaise(CW_ERROR_INVALID_FORMAT,
                            "Message is not Configure Response as Expected");

    if(controlVal.seqNum != seqNum)
        return CWErrorRaise(CW_ERROR_INVALID_FORMAT,
                            "Different Sequence Number");

    /* skip timestamp */
    controlVal.msgElemsLen -= CW_CONTROL_HEADER_OFFSET_FOR_MSG_ELEMS;

    if(combineLen !=0)
    {
       CWDebugLog("%s change packet length from %d to %d",__FUNCTION__,controlVal.msgElemsLen,combineLen);
       controlVal.msgElemsLen = combineLen;
    }

    offsetTillMessages = completeMsg.offset;

    /* parse message elements */
    while((completeMsg.offset - offsetTillMessages) < controlVal.msgElemsLen)
    {
        unsigned short int type = 0;	/* = CWProtocolRetrieve32(&completeMsg); */
        unsigned int len = 0;	/* = CWProtocolRetrieve16(&completeMsg); */

        CWParseFormatMsgElem(&completeMsg, &type, &len);
        /* CWDebugLog("Parsing Message Element: %u, len: %u", type, len); */

        switch(type)
        {
            case CW_MSG_ELEMENT_AC_IPV4_LIST_CW_TYPE:
                CW_FREE_OBJECT(valuesPtr->ACIPv4ListInfo.ACIPv4List);
                if(!(CWParseACIPv4List(&completeMsg, len, &(valuesPtr->ACIPv4ListInfo))))
                {
                    CWDestroyConfigureResponseValues(valuesPtr);
                    return CW_FALSE;
                }
                break;
            case CW_MSG_ELEMENT_AC_IPV6_LIST_CW_TYPE:
                CW_FREE_OBJECT(valuesPtr->ACIPv6ListInfo.ACIPv6List);
                if(!(CWParseACIPv6List(&completeMsg, len, &(valuesPtr->ACIPv6ListInfo))))
                {
                    CWDestroyConfigureResponseValues(valuesPtr);
                    return CW_FALSE;
                }
                break;
            case CW_MSG_ELEMENT_CW_TIMERS_CW_TYPE:
                if(!(CWParseCWTimers(&completeMsg, len, valuesPtr)))
                {
                    CWDestroyConfigureResponseValues(valuesPtr);
                    return CW_FALSE;
                }
                break;
            case CW_MSG_ELEMENT_RADIO_OPERAT_STATE_CW_TYPE:
                /*
                 * just count how many radios we have, so we
                 * can allocate the array
                 */
                valuesPtr->radioOperationalInfoCount++;
                completeMsg.offset += len;
                break;
            case CW_MSG_ELEMENT_CW_DECRYPT_ER_REPORT_PERIOD_CW_TYPE:
                valuesPtr->radiosDecryptErrorPeriod.radiosCount++;
                completeMsg.offset += len;
                break;
            case CW_MSG_ELEMENT_IDLE_TIMEOUT_CW_TYPE:
                if(!(CWParseIdleTimeout(&completeMsg, len, valuesPtr)))
                {
                    CWDestroyConfigureResponseValues(valuesPtr);
                    return CW_FALSE;
                }
                break;
            case CW_MSG_ELEMENT_WTP_FALLBACK_CW_TYPE:
                if(!(CWParseWTPFallback(&completeMsg, len, valuesPtr)))
                {
                    CWDestroyConfigureResponseValues(valuesPtr);
                    return CW_FALSE;
                }
                break;
            case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE:
            case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE_32_LEN:
                if(!CWParseVendorPayload(&completeMsg, len, &vendorValues))
                {
                    CWDestroyConfigureResponseValues(valuesPtr);
                    return CW_FALSE;
                }
                if(vendorValues.vendorPayloadType == CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_WTP_CFG)
                {
                    CWDestroyVendorSpecificValues(&(valuesPtr->vendorValuesWtpCfg));
                    CW_COPY_MEMORY(&(valuesPtr->vendorValuesWtpCfg), &vendorValues, sizeof(CWProtocolVendorSpecificValues));
                }
                else if(vendorValues.vendorPayloadType == CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_STATION_STATISTIC_INTERVAL)
                {
                    valuesPtr->statsUploadInterval = (int)(long) vendorValues.payload;
                }
                else if(vendorValues.vendorPayloadType == CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_STATION_STATISTIC_POLL_INTERVAL)
                {
                    valuesPtr->statsPollInterval = (int)(long) vendorValues.payload;
                }
                else if(vendorValues.vendorPayloadType == CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_STATION_STATISTIC_MAX_CLIENTS)
                {
                    valuesPtr->statsMaxClients = (int)(long) vendorValues.payload;
                }
                else if(vendorValues.vendorPayloadType == CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CLIENT_STATE_CHANGE_EVENT_ENABLE)
                {
                    valuesPtr->clientStateChgEventEnable = (CWBool) vendorValues.payload;
                }
                else if(vendorValues.vendorPayloadType == CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_BACKGROUND_SITESURVEY)
                {
                    if(((CWBackgroundSitesurveyValues *)vendorValues.payload)->radioType == CW_RADIOFREQTYPE_2G)
                    {
                        CW_COPY_MEMORY(&(valuesPtr->bgStSvy2gVal), (CWBackgroundSitesurveyValues *)vendorValues.payload, sizeof(CWBackgroundSitesurveyValues));
                        CWDestroyVendorSpecificValues(&vendorValues);
                    }
                    else if(((CWBackgroundSitesurveyValues *)vendorValues.payload)->radioType == CW_RADIOFREQTYPE_5G)
                    {
                        CW_COPY_MEMORY(&(valuesPtr->bgStSvy5gVal), (CWBackgroundSitesurveyValues *)vendorValues.payload, sizeof(CWBackgroundSitesurveyValues));
                        CWDestroyVendorSpecificValues(&vendorValues);
                    }
                    else if(((CWBackgroundSitesurveyValues *)vendorValues.payload)->radioType == CW_RADIOFREQTYPE_5G_1)
                    {
                        CW_COPY_MEMORY(&(valuesPtr->bgStSvy5gOneVal), (CWBackgroundSitesurveyValues *)vendorValues.payload, sizeof(CWBackgroundSitesurveyValues));
                        CWDestroyVendorSpecificValues(&vendorValues);
                    }
                    else
                    {
                        CWDestroyConfigureResponseValues(valuesPtr);
                        return CW_FALSE;
                    }
                }
                else if(vendorValues.vendorPayloadType == CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_MEMORY_LOG_THRESHOLD)
                {
                    valuesPtr->memLogThreshold = (int)(long) vendorValues.payload;
                }
                else
                {
                    CWDestroyVendorSpecificValues(&vendorValues);
                }
                break;
            default:
                completeMsg.offset += len;
                CWLog("Unrecognized Message Element %d in Configure Response", type);
        }
    }

    if(completeMsg.offset != len)
    {
        CWDestroyConfigureResponseValues(valuesPtr);
        return CWErrorRaise(CW_ERROR_INVALID_FORMAT,
                            "Garbage at the End of the Message");
    }

    if((*valuesPtr).radioOperationalInfoCount != 0)
    {
        CW_CREATE_ARRAY_ERR((*valuesPtr).radioOperationalInfo, (*valuesPtr).radioOperationalInfoCount,
                            CWRadioOperationalInfoValues,
        {
            CWDestroyConfigureResponseValues(valuesPtr);
            return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
        });
    }

    if((*valuesPtr).radiosDecryptErrorPeriod.radiosCount != 0)
    {
        CW_CREATE_ARRAY_ERR((*valuesPtr).radiosDecryptErrorPeriod.radios,
                            (*valuesPtr).radiosDecryptErrorPeriod.radiosCount,
                            WTPDecryptErrorReportValues,
        {
            CWDestroyConfigureResponseValues(valuesPtr);
            return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
        });
    }

    completeMsg.offset = offsetTillMessages;

    while(completeMsg.offset - offsetTillMessages < controlVal.msgElemsLen)
    {
        unsigned short int type = 0;	/* = CWProtocolRetrieve32(&completeMsg); */
        unsigned int len = 0;	/* = CWProtocolRetrieve16(&completeMsg); */

        CWParseFormatMsgElem(&completeMsg, &type, &len);

        switch(type)
        {
            case CW_MSG_ELEMENT_RADIO_OPERAT_STATE_CW_TYPE:
                /* will be handled by the caller */
                if(!(CWParseWTPRadioOperationalState(&completeMsg,
                                                     len,
                                                     &(valuesPtr->radioOperationalInfo[i]))))
                {
                    CWDestroyConfigureResponseValues(valuesPtr);
                    return CW_FALSE;
                }
                i++;
                break;
            case CW_MSG_ELEMENT_CW_DECRYPT_ER_REPORT_PERIOD_CW_TYPE:
                if(!(CWParseDecryptErrorReportPeriod(&completeMsg,
                                                     len,
                                                     &(valuesPtr->radiosDecryptErrorPeriod.radios[j]))))
                {
                    CWDestroyConfigureResponseValues(valuesPtr);
                    return CW_FALSE;
                }
                j++;
                break;
            default:
                completeMsg.offset += len;
                break;
        }
    }

    CWDebugLog("Configure Response Parsed");
    return CW_TRUE;
}

CWBool CWSaveConfigureResponseMessage(CWProtocolConfigureResponseValues *configureResponse)
{
    if(configureResponse == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    if(gACInfoPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_NEED_RESOURCE, NULL);
    }

    CWDebugLog("Saving Configure Response...");

    if(configureResponse->ACIPv4ListInfo.ACIPv4ListCount > 0)
    {
        CW_FREE_OBJECT(gACInfoPtr->ACIPv4ListInfo.ACIPv4List);
        gACInfoPtr->ACIPv4ListInfo.ACIPv4ListCount = configureResponse->ACIPv4ListInfo.ACIPv4ListCount;
        gACInfoPtr->ACIPv4ListInfo.ACIPv4List = configureResponse->ACIPv4ListInfo.ACIPv4List;
        configureResponse->ACIPv4ListInfo.ACIPv4List = NULL;
    }

    if((configureResponse->ACIPv6ListInfo).ACIPv6ListCount > 0)
    {
        CW_FREE_OBJECT(gACInfoPtr->ACIPv6ListInfo.ACIPv6List);
        gACInfoPtr->ACIPv6ListInfo.ACIPv6ListCount = configureResponse->ACIPv6ListInfo.ACIPv6ListCount;
        gACInfoPtr->ACIPv6ListInfo.ACIPv6List = configureResponse->ACIPv6ListInfo.ACIPv6List;
        configureResponse->ACIPv6ListInfo.ACIPv6List = NULL;
    }

    if(configureResponse->echoRequestTimer > 0)
    {
        gWTPEchoInterval = configureResponse->echoRequestTimer;
        CWDebugLog("Responsed Echo Interval %u", gWTPEchoInterval);
    }

    if(configureResponse->memLogThreshold > 0)
    {
        gWTPMemUsageLogThreshold = configureResponse->memLogThreshold;
        CWDebugLog("Memory Info Log Threshold %u", gWTPMemUsageLogThreshold);
    }

#ifdef CW_WTP_AP
    if(configureResponse->statsUploadInterval > 0)
    {
        CWWTPSetStatsUploadInterval(configureResponse->statsUploadInterval);
        CWDebugLog("Statistics Upload Interval %u", configureResponse->statsUploadInterval);
    }

    if(configureResponse->statsPollInterval > 0)
    {
        CWWTPSetStatsPollInterval(configureResponse->statsPollInterval);
        CWDebugLog("Statistics Poll Interval %u", configureResponse->statsUploadInterval);
    }

    if(configureResponse->statsMaxClients > 0)
    {
        CWWTPSetStatsMaxClients(configureResponse->statsMaxClients);
        CWDebugLog("Statistics Max Clients %u", configureResponse->statsMaxClients);
    }

    CWWTPSetClientChangeEventEnable(configureResponse->clientStateChgEventEnable);
    CWDebugLog("Client State Change Event Enable %u", configureResponse->clientStateChgEventEnable);

    if(configureResponse->bgStSvy2gVal.uint32Interval)
    {
        CWWTPSaveBackgroundSitesurveyValues(&(configureResponse->bgStSvy2gVal));
        CWDebugLog("WTP Background Sitesurvey Radiotype: 2G Enable: %u Interval: %u",
                   configureResponse->bgStSvy2gVal.bEnable, configureResponse->bgStSvy2gVal.uint32Interval);
    }
    if(configureResponse->bgStSvy5gVal.uint32Interval)
    {
        CWWTPSaveBackgroundSitesurveyValues(&(configureResponse->bgStSvy5gVal));
        CWDebugLog("WTP Background Sitesurvey Radiotype: 5G Enable: %u Interval: %u",
                   configureResponse->bgStSvy5gVal.bEnable, configureResponse->bgStSvy5gVal.uint32Interval);
    }
    if(configureResponse->bgStSvy5gOneVal.uint32Interval)
    {
        CWWTPSaveBackgroundSitesurveyValues(&(configureResponse->bgStSvy5gOneVal));
        CWDebugLog("WTP Background Sitesurvey Radiotype: 5G1 Enable: %u Interval: %u",
                   configureResponse->bgStSvy5gOneVal.bEnable, configureResponse->bgStSvy5gOneVal.uint32Interval);
    }
#endif /* CW_WTP_AP */

    /* AC may provide the configurations which need to be applied */
    if(configureResponse->vendorValuesWtpCfg.payload)
    {
        if(!CWWTPSaveWtpCfg((CWWtpCfgMsgList *)(configureResponse->vendorValuesWtpCfg.payload)))
        {
            CWDestroyConfigureResponseValues(configureResponse);
            return CW_FALSE;
        }
    }
    else
    {
        gWTPResultCode = CW_PROTOCOL_SUCCESS;
        CW_ZERO_MEMORY(&gWTPCfgResult, sizeof(gWTPCfgResult));
    }
    /*
         * It is not clear to me what the original developers intended to
         * accomplish. One thing's for sure: radioOperationalInfo, radiosDecryptErrorPeriod.radios,
         * and bidingValues get allocated and are never freed,
         * so we do it here...
         *
         * BUGs ML02-ML04-ML05
         * 16/10/2009 - Donato Capitella
         */
    CWDestroyConfigureResponseValues(configureResponse);
    CWDebugLog("Configure Response Saved");
    return CW_TRUE;
}


void CWDestroyConfigureResponseValues(CWProtocolConfigureResponseValues *configureResponse)
{
    CW_FREE_OBJECT(configureResponse->ACIPv4ListInfo.ACIPv4List);
    CW_FREE_OBJECT(configureResponse->ACIPv6ListInfo.ACIPv6List);
    CW_FREE_OBJECT(configureResponse->radioOperationalInfo);
    CW_FREE_OBJECT(configureResponse->radiosDecryptErrorPeriod.radios);
    CW_FREE_OBJECT(configureResponse->staticIPInfo);
    CWDestroyVendorSpecificValues(&(configureResponse->vendorValuesWtpCfg));
}


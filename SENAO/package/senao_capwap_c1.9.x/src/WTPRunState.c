/************************************************************************************************
 * Copyright (c) 2006-2009 Laboratorio di Sistemi di Elaborazione e Bioingegneria Informatica	*
 *                          Universita' Campus BioMedico - Italy								*
 *																								*
 * This program is free software; you can redistribute it and/or modify it under the terms		*
 * of the GNU General Public License as published by the Free Software Foundation; either		*
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
 * Authors : Ludovico Rossi (ludo@bluepixysw.com)												*
 *           Del Moro Andrea (andrea_delmoro@libero.it)											*
 *           Giovannini Federica (giovannini.federica@gmail.com)								*
 *           Massimo Vellucci (m.vellucci@unicampus.it)											*
 *           Mauro Bisson (mauro.bis@gmail.com)													*
 *	         Antonio Davoli (antonio.davoli@gmail.com)											*
 ************************************************************************************************/

#include "CWWTP.h"
#include "CWVendorPayloads.h"
#include "WTPDataRecordStatistics.h"
#ifdef DMALLOC
#include "../dmalloc-5.5.0/dmalloc.h"
#endif

CWBool CWWTPManageGenericRunMessage(CWProtocolMessage *msgPtr, CWStateTransition *state);

CWBool CWParseWTPEventResponseMessage(char *msg,
                                      int len,
                                      int seqNum,
                                      void *values);

CWBool CWSaveWTPEventResponseMessage(void *WTPEventResp);

CWBool CWParseConfigurationUpdateRequest(char *msg,
        int len,
        CWProtocolConfigurationUpdateRequestValues *valuesPtr);

CWBool CWSaveConfigurationUpdateRequest(CWProtocolConfigurationUpdateRequestValues *valuesPtr,
                                        int *vendorPayloadType);

CWBool CWAssembleConfigurationUpdateResponse(CWProtocolMessage **messagesPtr,
        int *fragmentsNumPtr,
        int PMTU,
        int seqNum,
        CWProtocolConfigurationUpdateRequestValues *values);

CWBool CWSaveClearConfigurationRequest(CWProtocolResultCode *resultCode);

CWBool CWAssembleClearConfigurationResponse(CWProtocolMessage **messagesPtr,
        int *fragmentsNumPtr,
        int PMTU,
        int seqNum,
        CWProtocolResultCode resultCode);

CWBool CWAssembleStationConfigurationResponse(CWProtocolMessage **messagesPtr,
        int *fragmentsNumPtr,
        int PMTU,
        int seqNum,
        CWProtocolResultCode resultCode);

CWBool CWSendEchoRequest();

CWBool CWAssembleResetResponse(CWProtocolMessage **messagesPtr,
                               int *fragmentsNumPtr,
                               int PMTU,
                               int seqNum,
                               CWProtocolResultCode resultCode);

CWBool CWAssembleChangeACResponse(CWProtocolMessage **messagesPtr,
                                  int *fragmentsNumPtr,
                                  int PMTU,
                                  int seqNum,
                                  CWProtocolResultCode resultCode);

CWBool CWAssembleSitesurveyResponse(CWProtocolMessage **messagesPtr,
                                    int *fragmentsNumPtr,
                                    int PMTU,
                                    int seqNum,
                                    CWProtocolResultCode resultCode,
                                    CWWTPSitesurveyInfo *sitesurveyInfo);

CWBool CWAssembleKickmacResponse(CWProtocolMessage **messagesPtr,
                                 int *fragmentsNumPtr,
                                 int PMTU,
                                 int seqNum,
                                 CWProtocolResultCode resultCode);

CWBool CWAssembleShellCmdResponse(CWProtocolMessage **messagesPtr,
                                  int *fragmentsNumPtr,
                                  int PMTU,
                                  int seqNum,
                                  CWProtocolResultCode code,
                                  char *output);

CWBool CWAssembleFailureResponse(CWProtocolMessage **messagesPtr,
                                 int *fragmentsNumPtr,
                                 int PMTU,
                                 int seqNum,
                                 int msgType,
                                 CWProtocolResultCode resultCode);

CWBool CWAssembleStandAloneResponse(CWProtocolMessage **messagesPtr,
                                    int *fragmentsNumPtr,
                                    int PMTU,
                                    int seqNum,
                                    CWProtocolResultCode resultCode);

CWBool CWParseStationConfigurationRequest(char *msg, int len);

CWBool CWParseApplyConfigRequest(char *msg, int len, CWProtocolVendorSpecificValues *venderValuesPtr);

void CWConfirmRunStateToACWithEchoRequest();

void CWDestroyConfigurationUpdateRequestValues(CWProtocolConfigurationUpdateRequestValues *valuesPtr);

CWBool CWParseEchoResponse(char *msg, int len, CWProtocolEchoResponseValues *values);
CWBool CWParseSitesurveyRequest(char *msg, int len, unsigned char *radio);
CWBool CWParseKickmacRequest(char *msg, int len, CWProtocolVendorSpecificValues *kickMac);
CWBool CWParseShellCmdRequest(char *msg, int len, CWProtocolVendorSpecificValues *shellCmd);
CWBool CWParseChangeACRequest(char *msg, int len, CWHostName host);
CWBool CWParseWaitingTime(char *msg, int len, int *time);
CWBool CWWTPHandleResponse(int msgType, int seqNum, char *data, int len);

CWTimerID gWTPHeartBeatTimerID = -1;
CWBool gWTPSendEchoRequest;
CWBool gWTPSendCurCfg;
int gWTPCurCfgSeqNum;
int gWTPEchoInterval = CW_ECHO_INTERVAL_DEFAULT;
CWThreadMutex gWTPEchoDataMutex = CW_MUTEX_INITIALIZER;
int gWTPMemUsageLogThreshold = CW_DEF_MEM_USAGE_LOG_THRESHOLD; /* in percentage */
unsigned int gWTPMemUsageLast; /* in percentage */
int gWTPLastReqMsgType = 0;
CWProtocolMessage *gWTPLastRespMsg = NULL;
int gWTPLastRespMsgFragmentsNum = 0;

static CWThread gWTPRecvMsgThread;
static volatile CWBool gWTPRecvMsgThreadRun = CW_FALSE;

CWBool CWAssembleApplyConfigResponse(CWProtocolMessage **completeMsgPtr, int *fragmentsNumPtr,
                                     int PMTU, int seqNum, CWWtpCfgResult *cfgResult)
{
    CWProtocolMessage *msgElems = NULL;
    int msgElemCount = 2;
    CWProtocolMessage *msgElemsBinding = NULL;
    const int msgElemBindingCount = 0;
    int k = -1;

    if(completeMsgPtr == NULL || fragmentsNumPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems,
                                     msgElemCount,
                                     return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    CWDebugLog("Assembling Apply Config Response...");

    if(!CWAssembleMsgElemResultCode(&(msgElems[++k]), CW_PROTOCOL_SUCCESS) ||
       !CWAssembleMsgElemVendorPayloadWtpCfgResult(&(msgElems[++k]), cfgResult)
      )
    {
        int j;
        for(j = 0; j < k; j++)
        {
            CW_FREE_PROTOCOL_MESSAGE(msgElems[j]);
        }
        CW_FREE_OBJECT(msgElems);
        /* error will be handled by the caller */
        return CW_FALSE;
    }

    if(!(CWAssembleMessage(completeMsgPtr,
                           fragmentsNumPtr,
                           PMTU,
                           seqNum,
                           CW_MSG_TYPE_VALUE_APPLY_CONFIG_RESPONSE,
                           msgElems,
                           msgElemCount,
                           msgElemsBinding,
                           msgElemBindingCount,
                           CW_PACKET_CRYPT)))
    {
        return CW_FALSE;
    }

    return CW_TRUE;
}

static CWBool CWWTPWaitApplyConfigRequest(CWWtpCfgResult *cfgResult, CWProtocolVendorSpecificValues *venderValues)
{
    struct timespec timeout;

    CWDebugLog("CWWTPWaitApplyConfigRequest start");

    CWThreadMutexLock(&gInterfaceMutex);

    do
    {
        CWProtocolMessage msg = {NULL, 0, 0};
        CWControlHeaderValues controlVal;
        CWProtocolMessage *messages = NULL;
        int fragmentsNum = 0;

        CWDebugLog("[%02X:%02X:%02X:%02X:%02X:%02X] waiting packet...",
                   CW_MAC_PRINT_LIST(gWTPIfaceMac));

        timeout.tv_sec = time(0) + gWTPEchoInterval + 3; /* keepalive interval = echo interval */
        timeout.tv_nsec = 0;

        if(!CWErr(CWWaitThreadConditionTimeout(&gInterfaceWait, &gInterfaceMutex, &timeout)))
        {
            if(CWErrorGetLastErrorCode() == CW_ERROR_TIME_EXPIRED)
            {
                CWThreadMutexUnlock(&gInterfaceMutex);
                return CW_FALSE;
            }

            CWErrorHandleLast();
            continue;
        }

        if(CWGetCountElementFromSafeList(gPacketReceiveList) == 0)
        {
            CWLog("No packet received");
            continue;
        }

        CWThreadMutexUnlock(&gInterfaceMutex);

        if(!CWWTPReceiveMessage(&msg, gACInfoPtr->timer->fragment))
        {
            CWLog("Failure Receiving Message");
            CWErrorHandleLast();
            goto wait_again;
        }

        msg.offset = 0;

        if(!CWParseControlHeader(&msg, &controlVal))
        {
            CWLog("Failure Parsing Message");
            CWErrorHandleLast();
            goto wait_again;
        }

        /* request */
        if(controlVal.messageTypeValue & 0x1)
        {
            /* our last response is lost, resend response */
            if(gWTPLastReqSeqNum == controlVal.seqNum &&
               gWTPLastReqMsgType == controlVal.messageTypeValue)
            {
                CWLog("[%02X:%02X:%02X:%02X:%02X:%02X] Resend the response of last request msgType %d seqNum %d",
                      CW_MAC_PRINT_LIST(gWTPIfaceMac), gWTPLastReqMsgType, gWTPLastReqSeqNum);

                CWWTPSendMsg(gWTPLastRespMsg, gWTPLastRespMsgFragmentsNum,
                             CW_TRUE, CW_FALSE);
            }
            else if(controlVal.messageTypeValue == CW_MSG_TYPE_VALUE_APPLY_CONFIG_REQUEST)
            {
                CWDebugLog("[%02X:%02X:%02X:%02X:%02X:%02X] Apply Config Request Received",
                           CW_MAC_PRINT_LIST(gWTPIfaceMac));

                if(!CWWTPUpdateRequestSeqNum(controlVal.seqNum))
                {
                    CWErrorHandleLast();
                    goto wait_again;
                }

                if(!CWParseApplyConfigRequest(msg.msg + msg.offset,
                                              controlVal.msgElemsLen - CW_CONTROL_HEADER_OFFSET_FOR_MSG_ELEMS,
                                              venderValues))
                {
                    CWErrorHandleLast();
                    goto wait_again;
                }

                if(!CWAssembleApplyConfigResponse(&messages, &fragmentsNum,
                                                  gWTPPathMTU, controlVal.seqNum, cfgResult))
                {
                    CWErrorHandleLast();
                    CW_FREE_PROTOCOL_MESSAGE(msg);
                    CWDestroyVendorSpecificValues(venderValues);
                    return CW_FALSE;
                }

                CWWTPSendMsg(messages, fragmentsNum, CW_TRUE, CW_TRUE);
                CW_FREE_PROTOCOL_MESSAGE(msg);
                return CW_TRUE;
            }
            else
            {
                CWLog("[%02X:%02X:%02X:%02X:%02X:%02X] Receive resquest msgType %d while WTP is busy",
                      CW_MAC_PRINT_LIST(gWTPIfaceMac), controlVal.messageTypeValue);
            }
        }
        /* response */
        else if(!CWWTPHandleResponse(controlVal.messageTypeValue, controlVal.seqNum,
                                     msg.msg + msg.offset,
                                     controlVal.msgElemsLen - CW_CONTROL_HEADER_OFFSET_FOR_MSG_ELEMS))
        {
            CWLog("Failure Handling Response");
            CWErrorHandleLast();
        }

wait_again:
        CW_FREE_PROTOCOL_MESSAGE(msg);
        CWThreadMutexLock(&gInterfaceMutex);
    }
    while(1);
}

static CW_THREAD_RETURN_TYPE CWWTPRecvMsgTask(void *arg)
{
    CWDebugLog("CWWTPRecvMsgTask start");

    CWThreadMutexLock(&gInterfaceMutex);

    do
    {
        CWDebugLog("[%02X:%02X:%02X:%02X:%02X:%02X] waiting packet...", CW_MAC_PRINT_LIST(gWTPIfaceMac));

        if(!CWErr(CWWaitThreadCondition(&gInterfaceWait, &gInterfaceMutex)))
        {
            CWLog("CWWaitThreadCondition failed");
            CWErrorHandleLast();
        }
        else if(!gWTPRecvMsgThreadRun)
        {
            break;
        }
        else if(CWGetCountElementFromSafeList(gPacketReceiveList) > 0)
        {
            CWProtocolMessage msg = {NULL, 0, 0};
            CWControlHeaderValues controlVal;

            CWThreadMutexUnlock(&gInterfaceMutex);

            if(!CWWTPReceiveMessage(&msg, gACInfoPtr->timer->fragment))
            {
                CWLog("Failure Receiving Message");
                CWErrorHandleLast();
            }
            else
            {
                msg.offset = 0;

                if(!CWParseControlHeader(&msg, &controlVal))
                {
                    CWLog("Failure Parsing Control Header");
                    CWErrorHandleLast();
                }
                else
                {
                    if(controlVal.messageTypeValue & 0x1)
                    {
                        /* our last response is lost, resend response */
                        if(gWTPLastReqSeqNum == controlVal.seqNum &&
                           gWTPLastReqMsgType == controlVal.messageTypeValue)
                        {
                            CWLog("[%02X:%02X:%02X:%02X:%02X:%02X] Resend the response of last request msgType %d seqNum %d",
                                  CW_MAC_PRINT_LIST(gWTPIfaceMac), gWTPLastReqMsgType, gWTPLastReqSeqNum);

                            CWWTPSendMsg(gWTPLastRespMsg, gWTPLastRespMsgFragmentsNum,
                                         CW_TRUE, CW_FALSE);
                        }
                        else
                        {
                            CWLog("[%02X:%02X:%02X:%02X:%02X:%02X] Receive resquest msgType %d while WTP is busy",
                                  CW_MAC_PRINT_LIST(gWTPIfaceMac), controlVal.messageTypeValue);
                        }
                    }
                    /* response */
                    else if(!CWWTPHandleResponse(controlVal.messageTypeValue, controlVal.seqNum,
                                                 msg.msg + msg.offset,
                                                 controlVal.msgElemsLen - CW_CONTROL_HEADER_OFFSET_FOR_MSG_ELEMS))
                    {
                        CWLog("Failure Handling Response");
                        CWErrorHandleLast();
                    }
                }
            }

            CWThreadMutexLock(&gInterfaceMutex);

            CW_FREE_PROTOCOL_MESSAGE(msg);
        }
    }
    while(gWTPRecvMsgThreadRun);

    CWThreadMutexUnlock(&gInterfaceMutex);

    CWDebugLog("CWWTPRecvMsgTask exit");

    CWErrorFree();

    return (void *) 0;
}

CWBool CWWTPSendCurCfg(CWBool status)
{
    CWThreadMutexLock(&gWTPEchoDataMutex);

    gWTPSendCurCfg = status;

    CWThreadMutexUnlock(&gWTPEchoDataMutex);

    return CW_TRUE;
}

CWBool CWWTPStartRecvMsgTask()
{
    if(!gWTPRecvMsgThreadRun)
    {
        gWTPRecvMsgThreadRun = CW_TRUE;

        if(!CWErr(CWCreateThread(&gWTPRecvMsgThread,
                                 CWWTPRecvMsgTask,
                                 NULL)))
        {
            CWLog("Error starting CWWTPRecvMsgTask Thread");

            gWTPRecvMsgThreadRun = CW_FALSE;

            return CW_FALSE;
        }
    }

    return CW_TRUE;
}

void CWWTPStopRecvMsgTask()
{
    if(gWTPRecvMsgThreadRun)
    {
        CWThreadMutexLock(&gInterfaceMutex);

        gWTPRecvMsgThreadRun = CW_FALSE;

        CWSignalThreadCondition(&gInterfaceWait);

        CWThreadMutexUnlock(&gInterfaceMutex);

        CWDebugLog("Waiting CWWTPRecvMsgTask Thread exit...");

        CWThreadJoin(gWTPRecvMsgThread);
    }
}

void CWWTPWaitTransmissionTime()
{
    int waitTime, transTime, nowTime;

    /* if connection id is not provided by AC, wait random seconds */
    if(gWTPConnectionId == 0)
    {
        waitTime = CWRandomIntInRange(0, gWTPEchoInterval - 1);
        CWDebugLog("Waiting %d seconds before transmission",
                   waitTime);
    }
    else
    {
        transTime = (gWTPConnectionId - 1) % gWTPEchoInterval;
        nowTime = time(NULL) % gWTPEchoInterval;
        if(transTime >= nowTime)
        {
            waitTime = transTime - nowTime;
        }
        else
        {
            waitTime = transTime + (gWTPEchoInterval - nowTime);
        }

        CWDebugLog("Waiting %d seconds before transmission (nowTime %d transTime %d)",
                   waitTime, nowTime, transTime);
    }

    if(waitTime)
    {
        CWWaitSec(waitTime);
    }

    CWLog("[%02X:%02X:%02X:%02X:%02X:%02X] Start Transmission", CW_MAC_PRINT_LIST(gWTPIfaceMac));
}

/*
 * Manage Run State.
 */
CWStateTransition CWWTPEnterRun()
{
    CWStateTransition state = CW_ENTER_RUN;
    time_t recvTime = time(NULL);

    CWDebugLog("######### WTP enters in RUN State #########");
    CWLog("Echo Interval: %d sec", gWTPEchoInterval);
    CWLog("Peer Dead Timer: %d sec", gCWNeighborDeadInterval);
    CWLog("Retransmit Timer: %u sec", gCWRetransmitTimer);
    CWLog("Statistics Poll Timer: %d sec", gWTPStatisticsPollTimer);
    CWLog("Statistics Timer: %d sec", gWTPStatisticsTimer);
    CWLog("Connection ID: %d", gWTPConnectionId);

    /* wait some time before first echo packet to prevent brust */
    CWWTPWaitTransmissionTime();

    CWWTPHaltPendingRequest(CW_FALSE);

    /* Send an echo request to AC in the beginning,
     * in order to notify AC that wtp is no longer busy (if configuration update occured in previous state) */
    gWTPSendCurCfg = CW_TRUE; /* contain current cfg in next echo request */
    gWTPMemUsageLast = 0;

#ifdef CW_WTP_AP
    CWWTPResetAutoTxPower();
#endif
    CWSendAndStartEchoTimer();
#ifdef CW_WTP_AP
    CWWTPStartStats();
    CWWTPStartAllSitesurvey();
#endif
#ifdef CW_WTP_SWITCH
    CWWTPStartSwitchLogPollingTask();
#endif

    CW_REPEAT_FOREVER
    {
        struct timespec timeout;
        int recvCount;

        /* Wait packet */

        CWThreadMutexLock(&gInterfaceMutex);
        /*
         * if there are no frames from stations
         * and no packets from AC...
         */
        if(CWGetCountElementFromSafeList(gPacketReceiveList) == 0)
        {
            /*
             * wait for a frame or packet in peerDead timer.
             */
retry:
            timeout.tv_sec = time(0) + gCWNeighborDeadInterval;
            timeout.tv_nsec = 0;

            CWDebugLog("[%02X:%02X:%02X:%02X:%02X:%02X] waiting packet...", CW_MAC_PRINT_LIST(gWTPIfaceMac));

            if(!CWErr(CWWaitThreadConditionTimeout(&gInterfaceWait, &gInterfaceMutex, &timeout)))
            {
                if(CWErrorGetLastErrorCode() == CW_ERROR_TIME_EXPIRED)
                {
                    CWThreadMutexUnlock(&gInterfaceMutex);

                    CWLog("[%02X:%02X:%02X:%02X:%02X:%02X] No Message from AC for %u seconds, restart Discovery State",
                    CW_MAC_PRINT_LIST(gWTPIfaceMac), time(NULL) - recvTime);

                    state = CW_ENTER_DISCOVERY;

                    break;
                }
                else
                {
                    goto retry;
                }
            }
        }

        recvCount = CWGetCountElementFromSafeList(gPacketReceiveList);

        CWThreadMutexUnlock(&gInterfaceMutex);

        if(recvCount > 0)
        {
            CWProtocolMessage msg = {NULL, 0, 0};
            recvTime = time(NULL);

            CWDebugLog("[%02X:%02X:%02X:%02X:%02X:%02X] receive %d packets",
                       CW_MAC_PRINT_LIST(gWTPIfaceMac), recvCount);

            if(!CWWTPReceiveMessage(&msg, gACInfoPtr->timer->fragment))
            {
                CWLog("Failure Receiving Message");

                CW_FREE_PROTOCOL_MESSAGE(msg);

                if(CWErrorGetLastErrorCode() != CW_ERROR_TIME_EXPIRED)
                {
                    CWLog("--> Critical Error in Receiving Packet... we enter DISCOVERY State");
                    CWErrorHandleLast();
                    state = CW_ENTER_DISCOVERY;
                    break;
                }
                else
                {
                    continue;
                }
            }

            if(!CWErr(CWWTPManageGenericRunMessage(&msg, &state)))
            {
                if(CWErrorGetLastErrorCode() == CW_ERROR_INVALID_FORMAT)
                {
                    /* Log and ignore message */
                    CWLog("--> Received something different from a valid Run Message");
                }
                else
                {
                    CWLog("--> Critical Error Managing Generic Run Message... we enter DISCOVERY State");
                    CWErrorHandleLast();
                    state = CW_ENTER_DISCOVERY;
                }
            }

            CW_FREE_PROTOCOL_MESSAGE(msg);

            if(state != CW_ENTER_RUN)
            {
                break;
            }
        }
    }

    CWStopEchoTimer();
#ifdef CW_WTP_AP
    CWWTPStopStats();
    CWWTPStopAllSitesurvey();
#endif
#ifdef CW_WTP_SWITCH
    CWWTPSwitchStopLogPollingTask();
#endif
    CWWTPRemoveAllPendingRequest();
    CWWTPHaltPendingRequest(CW_TRUE);

    if(gWTPLastRespMsg)
    {
        CWFreeMessageFragments(gWTPLastRespMsg, gWTPLastRespMsgFragmentsNum);
        CW_FREE_OBJECT(gWTPLastRespMsg);
    }

    return state;
}

CWBool CWWTPHandleResponse(int msgType, int seqNum, char *data, int len)
{
    if(msgType == CW_MSG_TYPE_VALUE_KEEP_ALIVE_RESPONSE ||
       CWWTPRemovePendingRequestByResponse(msgType, seqNum))
    {
        switch(msgType)
        {
            case CW_MSG_TYPE_VALUE_ECHO_RESPONSE:
            {
                CWProtocolEchoResponseValues resValues;

                if(!CWParseEchoResponse(data, len, &resValues))
                {
                    return CW_FALSE;
                }

                CWDebugLog("Echo Response received");

                CWThreadMutexLock(&gWTPEchoDataMutex);

                if(gWTPSendCurCfg && gWTPCurCfgSeqNum == seqNum)
                {
                    gWTPSendCurCfg = CW_FALSE;
                }

                CWThreadMutexUnlock(&gWTPEchoDataMutex);

#ifdef CW_WTP_AP
                if(resValues.statsMaxClients > 0)
                {
                    CWWTPSetStatsMaxClients(resValues.statsMaxClients);
                    CWDebugLog("Statistics Max Clients %u", resValues.statsMaxClients);
                }
#endif
                if(resValues.debugLog >= 0 && gEnabledDebugLog < 2)
                {
                    gEnabledDebugLog = resValues.debugLog;
                }

                if(resValues.utc_time > 0)
                {
                    CWWTPBoardSetUTCTime(resValues.utc_time);
                }

                break;
            }
            case CW_MSG_TYPE_VALUE_WTP_EVENT_RESPONSE:
                CWDebugLog("WTP Event Response received");
                break;
            case CW_MSG_TYPE_VALUE_KEEP_ALIVE_RESPONSE:
                CWDebugLog("WTP Keep Alive Response received");
                break;
#if 0 /* Unused */
            case CW_MSG_TYPE_VALUE_CHANGE_STATE_EVENT_RESPONSE:
                CWDebugLog("Change State Event Response received");
                break;
            case CW_MSG_TYPE_VALUE_DATA_TRANSFER_RESPONSE:
                CWDebugLog("Data Transfer Response received");
                break;
#endif
            default:
                /*
                 * We can't recognize the received Response: we
                 * ignore the message and log the event.
                 */
                return CWErrorRaise(CW_ERROR_INVALID_FORMAT,
                                    "Received Response Message Type %d is not valid in Run State",
                                    msgType);
        }
    }

    return CW_TRUE;
}

CWBool CWWTPManageGenericRunMessage(CWProtocolMessage *msgPtr, CWStateTransition *state)
{
    CWControlHeaderValues controlVal;
    CWBool waitApply = CW_FALSE;

    if(msgPtr == NULL || state == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    msgPtr->offset = 0;

    /* will be handled by the caller */
    if(!(CWParseControlHeader(msgPtr, &controlVal)))
    {
        return CW_FALSE;
    }

    int len = controlVal.msgElemsLen - CW_CONTROL_HEADER_OFFSET_FOR_MSG_ELEMS;

    if(msgPtr ->combine_len != 0)
    {
        len = msgPtr->combine_len;
         CWDebugLog("%s change packet length from %d to %d",__FUNCTION__,controlVal.msgElemsLen,len);
    }
    char *msgData = msgPtr->msg + msgPtr->offset;

    /* check if it is a request message */
    if(controlVal.messageTypeValue & 1)
    {
        CWProtocolMessage *messages = NULL;
        int fragmentsNum = 0;
        CWBool toSend = CW_FALSE;
        CWBool toApply = CW_FALSE;

        /* Check the sequence number of the request mesage */
        if(!CWWTPUpdateRequestSeqNum(controlVal.seqNum))
        {
            /* our last response is lost, resend response */
            if(gWTPLastReqSeqNum == controlVal.seqNum &&
               gWTPLastReqMsgType == controlVal.messageTypeValue)
            {
                CWLog("[%02X:%02X:%02X:%02X:%02X:%02X] Resend the response of last request msgType %d seqNum %d",
                      CW_MAC_PRINT_LIST(gWTPIfaceMac), gWTPLastReqMsgType, gWTPLastReqSeqNum);

                CWWTPSendMsg(gWTPLastRespMsg, gWTPLastRespMsgFragmentsNum,
                             CW_TRUE, CW_FALSE);
                return CW_TRUE;
            }

            return CWErrorRaise(CW_ERROR_INVALID_FORMAT, NULL);
        }

        switch(controlVal.messageTypeValue)
        {
            case CW_MSG_TYPE_VALUE_CONFIGURE_UPDATE_REQUEST:
            {
                CWProtocolConfigurationUpdateRequestValues reqValues;

                CWDebugLog("Configuration Update Request received");

                if(!CWParseConfigurationUpdateRequest(msgData, len, &reqValues))
                {
                    return CW_FALSE;
                }

                /* if there is an image identifier, go to image data */
                if(reqValues.imageId.imageName[0] != '\0')
                {
                    CW_COPY_MEMORY(&gWTPImageId, &reqValues.imageId, sizeof(CWImageIdentifier));
                    *state = CW_ENTER_IMAGE;
                }
                else
                {
                    int payloadType;

                    /* Request to change WTP Configuration */
                    if(!CWSaveConfigurationUpdateRequest(&reqValues, &payloadType))
                    {
                        CWDestroyConfigurationUpdateRequestValues(&reqValues);
                        return CW_FALSE;
                    }

                    if(!CWAssembleConfigurationUpdateResponse(&messages,
                            &fragmentsNum,
                            gWTPPathMTU,
                            controlVal.seqNum,
                            &reqValues))
                    {
                        CWWtpCfgMsgListFree(&gWTPCfgRollbackList);
                        CWDestroyConfigurationUpdateRequestValues(&reqValues);
                        return CW_FALSE;
                    }
                    CWWtpCfgMsgListFree(&gWTPCfgRollbackList);

                    toSend = CW_TRUE;

                    if(payloadType == CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_WTP_CFG)
                    {
                        toApply = CW_TRUE;
                        waitApply = reqValues.waitApply;
                        /* Stop echo timer, echo request must not be sent after the response sent
                           until the end of applying configuration */
                        if((gWTPCfgResult.apply && gWTPCfgResult.waitSec) || waitApply)
                        {
                            CWStopTimers();
                        }
                    }
#ifdef CW_WTP_AP
                    else if(payloadType == CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_BACKGROUND_SITESURVEY)
                    {
                        CWBackgroundSitesurveyValues *pBgStSvy = (CWBackgroundSitesurveyValues *)(reqValues.vendorValuesCfg.payload);

                        if(!CWWTPCheckSitesurveyDoing(pBgStSvy->radioType) && pBgStSvy->bEnable)
                        {
                            CWWTPStartSitesurvey(pBgStSvy->radioType);
                        }
                        else if(CWWTPCheckSitesurveyDoing(pBgStSvy->radioType) && !pBgStSvy->bEnable)
                        {
                            CWWTPStopSitesurvey(pBgStSvy->radioType);
                        }
                    }
#endif
                }

                CWDestroyConfigurationUpdateRequestValues(&reqValues);
                break;
            }
            case CW_MSG_TYPE_VALUE_CHANGE_AC_REQUEST:
            {
                CWHostName host;
                char *pstr;
                CWProtocolResultCode resultCode = CW_PROTOCOL_SUCCESS;
                CWAcAddress acAddr;

                if(!CWParseChangeACRequest(msgData, len, host))
                {
                    return CW_FALSE;
                }

                CWLog("Checking the connectivity of new AC address %s...", host);

                if(!CWParseAcAddrString(host, &acAddr) || acAddr.hostName[0] == '\0')
                {
                    resultCode = CW_PROTOCOL_FAILURE_INCORRECT_DATA;
                }
                else
                {
                    /* Use findAc to check the connectivity */
                    pstr = CWCreateStringByCmdStdout("findAc --ip %s --port %d --cid %d --if %s",
                                                     acAddr.hostName,
                                                     acAddr.port,
                                                     acAddr.controllerId,
                                                     gWTPIface);
                    if(pstr && pstr[0] != '\0')
                    {
                        CWLog("findAc result: %s", pstr);
                        if(strncmp("Success", pstr, 7) != 0)
                        {
                            resultCode = CW_PROTOCOL_FAILURE;
                        }
                    }
                    CW_FREE_OBJECT(pstr);
                }

                if(!CWAssembleChangeACResponse(&messages, &fragmentsNum, gWTPPathMTU,
                                               controlVal.seqNum, resultCode))
                {
                    return CW_FALSE;
                }

                if(resultCode == CW_PROTOCOL_SUCCESS)
                {
                    CW_CREATE_STRING_FROM_STRING_ERR(gChgAcAddr, host,
                                                     return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

                    CWLog("Start to connect new AC address %s...", gChgAcAddr);

                    *state = CW_ENTER_DISCOVERY;
                    gWTPRejoinAc = CW_TRUE;
                    gWTPFastJoin = CW_TRUE;
                }

                toSend = CW_TRUE;
                break;
            }
            case CW_MSG_TYPE_VALUE_CLEAR_CONFIGURATION_REQUEST:
            {
                if(!CWParseWaitingTime(msgData, len, &gWTPWaitTimeBeforeReset))
                {
                    return CW_FALSE;
                }

                if(!CWAssembleClearConfigurationResponse(&messages, &fragmentsNum,
                        gWTPPathMTU, controlVal.seqNum,
                        CW_PROTOCOL_SUCCESS))
                {
                    return CW_FALSE;
                }

                toSend = CW_TRUE;

                gWTPFactoryReset = CW_TRUE;
                *state = CW_ENTER_RESET;
                break;
            }
#if 0 /* Unused */
            case CW_MSG_TYPE_VALUE_STATION_CONFIGURATION_REQUEST:
            {
                CWProtocolResultCode resultCode = CW_PROTOCOL_FAILURE;

                if(!CWResetTimers())
                {
                    return CW_FALSE;
                }

                CWDebugLog("Station Configuration Request received");

                if(!CWParseStationConfigurationRequest((msgPtr->msg) + (msgPtr->offset), len))
                {
                    return CW_FALSE;
                }

                if(!CWAssembleStationConfigurationResponse(&messages, &fragmentsNum, gWTPPathMTU, controlVal.seqNum, resultCode))
                {
                    return CW_FALSE;
                }

                toSend = CW_TRUE;
                break;
            }
#endif
            case CW_MSG_TYPE_VALUE_RESET_REQUEST:
            {
                if(!CWParseWaitingTime(msgData, len, &gWTPWaitTimeBeforeReset))
                {
                    return CW_FALSE;
                }

                if(!CWAssembleResetResponse(&messages, &fragmentsNum, gWTPPathMTU,
                                            controlVal.seqNum, CW_PROTOCOL_SUCCESS))
                {
                    return CW_FALSE;
                }

                toSend = CW_TRUE;
                *state = CW_ENTER_RESET;
                break;
            }
            case CW_MSG_TYPE_VALUE_STANDALONE_MODE_REQUEST:
            {
                CWAcAddress acAddr;

                if(!CWAssembleStandAloneResponse(&messages, &fragmentsNum,
                                                 gWTPPathMTU, controlVal.seqNum,
                                                 CW_PROTOCOL_SUCCESS))
                {
                    return CW_FALSE;
                }
                CWWTPBoardSetAcMode(CW_FALSE);
                acAddr.hostName[0] = '\0';
                CWWTPBoardSetAcAddress(&acAddr);

                toSend = CW_TRUE;
                *state = CW_ENTER_DISCOVERY;

                break;
            }
#ifdef CW_WTP_AP
            case CW_MSG_TYPE_VALUE_SITESURVEY_REQUEST:
            {
                unsigned char radio;
                CWWTPSitesurveyInfo sitesurveyInfo;
                CWBool ret;

                CWDebugLog("Sitesurvey Request received");

                if(!CWParseSitesurveyRequest(msgData, len, &radio))
                {
                    return CW_FALSE;
                }

                sitesurveyInfo.radio = (CWRadioFreqType) radio;

                if(sitesurveyInfo.radio == CW_RADIOFREQTYPE_2G)
                {
                    CWWTPStop2gSitesurvey();
                }
                else if(sitesurveyInfo.radio == CW_RADIOFREQTYPE_5G)
                {
                    CWWTPStop5gSitesurvey();
                }
                else if(sitesurveyInfo.radio == CW_RADIOFREQTYPE_5G_1)
                {
                   CWWTPStop5gOneSitesurvey();
                }
                ret = CWWTPBoardGetSitesurvey(sitesurveyInfo.radio,
                                              &(sitesurveyInfo.version),
                                              &(sitesurveyInfo.infoCount),
                                              &(sitesurveyInfo.info));

                if(!CWAssembleSitesurveyResponse(&messages, &fragmentsNum, gWTPPathMTU,
                                                 controlVal.seqNum,
                                                 ret ? CW_PROTOCOL_SUCCESS : CW_PROTOCOL_FAILURE,
                                                 &sitesurveyInfo))
                {
                    CW_FREE_OBJECT(sitesurveyInfo.info);
                    return CW_FALSE;
                }

                if(sitesurveyInfo.radio == CW_RADIOFREQTYPE_2G)
                {
                    CWWTPStart2gSitesurvey();
                }
                else if(sitesurveyInfo.radio == CW_RADIOFREQTYPE_5G)
                {
                    CWWTPStart5gSitesurvey();
                }

                else if(sitesurveyInfo.radio == CW_RADIOFREQTYPE_5G_1)
                {
                    CWWTPStart5gOneSitesurvey();
                }

                toSend = CW_TRUE;
                CW_FREE_OBJECT(sitesurveyInfo.info);

                break;
            }
            case CW_MSG_TYPE_VALUE_KICKMAC_REQUEST:
            {
                CWProtocolVendorSpecificValues vendorPayloadKickInfo;
                CWBool ret;

                if(!CWParseKickmacRequest(msgData, len, &vendorPayloadKickInfo))
                {
                    CWLog("CWParseKickmacRequest error!!");
                    return CW_FALSE;
                }

                ret = CWWTPBoardKickmac((CWWTPKickmacInfo *) vendorPayloadKickInfo.payload);

                CWDestroyVendorSpecificValues(&vendorPayloadKickInfo);

                if(!CWAssembleKickmacResponse(&messages, &fragmentsNum, gWTPPathMTU,
                                              controlVal.seqNum,
                                              ret ? CW_PROTOCOL_SUCCESS : CW_PROTOCOL_FAILURE))
                {
                    return CW_FALSE;
                }

                toSend = CW_TRUE;

                break;
            }
#endif /* CW_WTP_AP */
            case CW_MSG_TYPE_VALUE_SHELL_CMD_REQUEST:
            {
                CWProtocolVendorSpecificValues vendorPayloadShellCmd;
                CWShellCmdInfo *shellCmd;
                char outputFile[32];
                char pidFile[32];
                char *pid = NULL;
                char *cmdOutput = NULL;
                char procFile[32];
                FILE *fp;
                CWBool ret;
                int timeoutMs;
                int pollInt = 200000;  /* 200ms */

                if(!CWParseShellCmdRequest(msgData, len, &vendorPayloadShellCmd))
                {
                    CWLog("CWParseShellCmdRequest error!!");
                    return CW_FALSE;
                }

                sprintf(outputFile, "/tmp/wtp_sh.%x.out", getpid());
                sprintf(pidFile, "/tmp/wtp_sh.%x.pid", getpid());

                shellCmd = (CWShellCmdInfo *) vendorPayloadShellCmd.payload;

                CWSystem("%s %s> %s %s & echo $! > %s",
                         shellCmd->cmd,
                         shellCmd->outputType == 2 ? "2" : "",
                         outputFile,
                         shellCmd->outputType == 0 ? "2>&1" : "",
                         pidFile);

                ret = CWCreateStringByFile(pidFile, &pid);
                unlink(pidFile);

                if(ret && pid[0] != '\0')
                {
                    pid[strlen(pid) - 1] = '\0'; /* remove '\n' */
                    sprintf(procFile, "/proc/%s/cmdline", pid);
                    timeoutMs = shellCmd->timeout * 1000000;

                    do
                    {
                        fp = fopen(procFile, "r");
                        if(fp)
                        {
                            fclose(fp);
                            if(timeoutMs <= 0)
                            {
                                CWLog("PID %s doesn't terminate within %d seconds, kill it",
                                      pid, shellCmd->timeout);
                                CWSystem("kill -9 %s", pid);
                                break;
                            }
                            else
                            {
                                CWWaitMicroSec(pollInt);
                                timeoutMs -= pollInt;
                            }
                        }
                        else
                        {
                            break;
                        }
                    }
                    while(1);

                    ret = CWCreateStringByFile(outputFile, &cmdOutput);
                    unlink(outputFile);
                }

                CWDestroyVendorSpecificValues(&vendorPayloadShellCmd);
                CW_FREE_OBJECT(pid);

                if(!CWAssembleShellCmdResponse(&messages, &fragmentsNum,
                                               gWTPPathMTU, controlVal.seqNum,
                                               ret ? CW_PROTOCOL_SUCCESS : CW_PROTOCOL_FAILURE,
                                               cmdOutput))
                {
                    CW_FREE_OBJECT(cmdOutput);
                    return CW_FALSE;
                }

                CW_FREE_OBJECT(cmdOutput);

                toSend = CW_TRUE;

                break;
            }
            default:
                /*
                 * We can't recognize the received Request so
                 * we have to send a corresponding response
                 * containing a failure result code
                 */
                CWLog("--> Not a valid Request of msgType %d in Run State...", controlVal.messageTypeValue);

                if(!CWAssembleFailureResponse(&messages, &fragmentsNum, gWTPPathMTU, controlVal.seqNum,
                                              controlVal.messageTypeValue + 1,
                                              CW_PROTOCOL_FAILURE_UNRECOGNIZED_REQ))
                {
                    return CW_FALSE;
                }

                toSend = CW_TRUE;
        }

        if(toSend)
        {
            CWWTPSendMsg(messages, fragmentsNum, CW_TRUE, CW_FALSE);
            CWDebugLog("Response Message Sent");

            if(gWTPLastRespMsg)
            {
                CWFreeMessageFragments(gWTPLastRespMsg, gWTPLastRespMsgFragmentsNum);
                CW_FREE_OBJECT(gWTPLastRespMsg);
            }

            gWTPLastReqMsgType = controlVal.messageTypeValue;
            gWTPLastRespMsg = messages;
            gWTPLastRespMsgFragmentsNum = fragmentsNum;
        }

        if(toApply)
        {
            if(gWTPResultCode == CW_PROTOCOL_SUCCESS)
            {
                if(!gWTPCfgResult.apply && !waitApply)
                {
                    CWLog("########### No Configuration Applied ###########");
                }
                else if(gWTPCfgResult.waitSec || waitApply)
                {
                    /* The applying will take some time */
                    CWLog("########### Wait Applying Configuration ###########");
#ifdef CW_WTP_AP
                    CWWTPStopAllSitesurvey();
                    CWWTPStopStats();
#endif
                    CWWTPHaltPendingRequest(CW_TRUE);

                    if(!gWTPCfgResult.rejoin || waitApply)
                    {
                        CWWTPStartKeepAliveTask();
                    }

                    if(waitApply)
                    {
                        CWProtocolVendorSpecificValues vendorValues;
                        CWWTPApplyConfigInfo *applyInfo;

                        if(!CWWTPWaitApplyConfigRequest(&gWTPCfgResult, &vendorValues))
                        {
                            CWLog("Failed to wait applying config request");
                            CWWTPBoardCancelCfg();
                            *state = CW_ENTER_DISCOVERY;
                            return CW_FALSE;
                        }

                        applyInfo = (CWWTPApplyConfigInfo *) vendorValues.payload;
                        if(applyInfo->cancel)
                        {
                            CWWTPBoardCancelCfg();
                            gWTPCfgResult.apply = CW_FALSE;
                            gWTPCfgResult.rejoin = CW_FALSE;
                        }
                        else
                        {
                            CWLog("Waiting %d seconds before applying configuration", applyInfo->waitTime);
                            CWWaitSec(applyInfo->waitTime);
                        }

                        CWDestroyVendorSpecificValues(&vendorValues);
                    }

                    if(gWTPCfgResult.apply)
                    {
                        CWWTPStartRecvMsgTask();

                        CWWTPBoardApplyCfg();

                        CWWTPStopRecvMsgTask();
                    }

                    if(gWTPCfgResult.rejoin)
                    {
                        CWLog("Rejoin AC");

                        CWWTPStopKeepAliveTask();
                        *state = CW_ENTER_DISCOVERY;
                        gWTPRejoinAc = CW_TRUE;
                        gWTPFastJoin = CW_TRUE;
                        return CW_TRUE;
                    }
                    else
                    {
                        CWWTPWaitTransmissionTime();

                        CWWTPStopKeepAliveTask();

                        if(gWTPCfgResult.apply)
                        {
                            /* Send current cfg in next echo request */
                            CWWTPSendCurCfg(CW_TRUE);
                        }

                        CWWTPHaltPendingRequest(CW_FALSE);
                        CWSendAndStartEchoTimer();
#ifdef CW_WTP_AP
                        CWWTPStartStats();
                        CWWTPStartAllSitesurvey();
#endif
                    }
                }
                else if(gWTPCfgResult.apply)
                {
                    CWLog("########### Applying Configuration ###########");

                    CWWTPBoardApplyCfg();

                    if(gWTPCfgResult.rejoin)
                    {
                        CWLog("Rejoin AC...");
                        *state = CW_ENTER_DISCOVERY;
                        gWTPRejoinAc = CW_TRUE;
                        gWTPFastJoin = CW_TRUE;
                    }

                    /* Send current cfg in next echo request */
                    CWWTPSendCurCfg(CW_TRUE);
                }
            }
            else
            {
                switch(gWTPCfgResult.handle)
                {
                    case WTP_CFG_ERROR_HANDLE_ROLLBACK:
                        break;
                    case WTP_CFG_ERROR_HANDLE_RESYNC:
                        *state = CW_ENTER_CONFIGURE;
                        break;
                    case WTP_CFG_ERROR_HANDLE_UNMANAGED:
                        *state = CW_ENTER_DISCOVERY;
                        gWTPRejoinAc = CW_FALSE;
                        break;
                    default:
                        break;
                }
            }
        }
    }
    /* we have received a response */
    else if(!CWWTPHandleResponse(controlVal.messageTypeValue, controlVal.seqNum, msgData, len))
    {
        CWLog("Failed to Handling Response");
        return CW_FALSE;
    }

    return CW_TRUE;
}

/*______________________________________________________________*/
/*  *******************___TIMER HANDLERS___*******************  */
void CWWTPHeartBeatTimerExpiredHandler(void *arg)
{
    CWDebugLog("#________ Echo Request Timer Expired ________#");

    CWThreadMutexLock(&gWTPEchoDataMutex);

    if(!gWTPSendEchoRequest)
    {
        CWDebugLog("Cannot Send Echo Request Now");
        CWThreadMutexUnlock(&gWTPEchoDataMutex);
        return;
    }

    gWTPHeartBeatTimerID = timer_add(gWTPEchoInterval,
                                     0,
                                     CWWTPHeartBeatTimerExpiredHandler,
                                     NULL);
    CWDebugLog("Echo Request Timer Restarted");

    /* Send WTP echo Request */
    if(!CWSendEchoRequest())
    {
        CWErrorHandleLast();
        CWThreadMutexUnlock(&gWTPEchoDataMutex);
        return;
    }

    CWThreadMutexUnlock(&gWTPEchoDataMutex);
}

CWBool CWSendAndStartEchoTimer()
{
    gWTPSendEchoRequest = CW_TRUE;

    CWWTPHeartBeatTimerExpiredHandler(NULL);

    return CW_TRUE;
}

CWBool CWStartEchoTimer()
{
    CWThreadMutexLock(&gWTPEchoDataMutex);

    gWTPSendEchoRequest = CW_TRUE;
    gWTPHeartBeatTimerID = timer_add(gWTPEchoInterval,
                                     0,
                                     CWWTPHeartBeatTimerExpiredHandler,
                                     NULL);
    if(gWTPHeartBeatTimerID == -1)
    {
        CWLog("Echo Request Timer Add failed");
        CWThreadMutexUnlock(&gWTPEchoDataMutex);
        return CW_FALSE;
    }

    CWDebugLog("Echo Request Timer Started");

    CWThreadMutexUnlock(&gWTPEchoDataMutex);

    return CW_TRUE;
}

CWBool CWStopEchoTimer()
{
    CWThreadMutexLock(&gWTPEchoDataMutex);

    gWTPSendEchoRequest = CW_FALSE;
    if(gWTPHeartBeatTimerID != -1)
    {
        timer_rem(gWTPHeartBeatTimerID, NULL);
        gWTPHeartBeatTimerID = -1;
    }

    CWDebugLog("Echo Request Timer Stopped");

    CWThreadMutexUnlock(&gWTPEchoDataMutex);

    return CW_TRUE;
}

CWBool CWResetTimers()
{
    if(!CWStopEchoTimer())
    {
        return CW_FALSE;
    }

    if(!CWStartEchoTimer())
    {
        return CW_FALSE;
    }

    return CW_TRUE;
}

CWBool CWStopTimers()
{
    if(!CWStopEchoTimer())
    {
        return CW_FALSE;
    }

    return CW_TRUE;
}

/*__________________________________________________________________*/
/*  *******************___ASSEMBLE FUNCTIONS___*******************  */

CWBool CWSendEchoRequest()
{
    CWMemoryInfo memInfo;
    CWProtocolMessage *msgElems = NULL;
    int j, k = -1, seqNum;
    int UTCTime=0;
#ifdef CW_WTP_AP
    int msgElemCount = 4;
#elif defined(CW_WTP_SWITCH)
    int msgElemCount = 5;
#endif

    CWDebugLog("Assembling Echo Request...");

    if(gWTPSendCurCfg)
    {
        CWDebugLog("Upload Current Cfg");
        msgElemCount++;
    }

    if(CWGetMemoryInfo(&memInfo))
    {
        CWDebugLog("Upload MemoryInfo %u/%u", memInfo.used, memInfo.total);
        msgElemCount++;
    }
    else
    {
        memInfo.total = 0;
    }

    CWWTPBoardGetUTCTime(&UTCTime);

    CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems,
                                     msgElemCount,
                                     return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
#ifdef CW_WTP_SWITCH
    if((!(CWWTPAssembleMsgElemVendorPayloadSwitchPortInfo(&(msgElems[++k])))) ||
       (!(CWWTPAssembleMsgElemVendorPayloadSwitchPoeInfo(&(msgElems[++k])))) ||
       (!(CWWTPAssembleMsgElemVendorPayloadSwitchTopology(&(msgElems[++k])))) ||
       (!(CWWTPAssembleMsgElemVendorPayloadSwitchTrunkInfo(&(msgElems[++k])))) ||
#elif defined(CW_WTP_AP)
    if((!(CWWTPAssembleMsgElemVendorPayloadCurrentStationInfo(&(msgElems[++k])))) ||
       (!(CWWTPAssembleMsgElemVendorPayloadWlanStatisticsInfo(&(msgElems[++k])))) ||
       (!(CWWTPAssembleMsgElemVendorPayloadMeshInfo(&(msgElems[++k])))) ||
#endif
       (memInfo.total && !(CWAssembleMsgElemVendorPayloadMemoryInfo(&(msgElems[++k]), &memInfo))) ||
       (gWTPSendCurCfg && !(CWWTPAssembleMsgElemVendorPayloadCurrentCfg(&(msgElems[++k])))) ||
       ( !CWAssembleMsgElemVendorPayloadUTCTime(&(msgElems[++k]),UTCTime) )
      )
    {
        for(j = 0; j < k; j++)
        {
            CW_FREE_PROTOCOL_MESSAGE(msgElems[j]);
        }
        CW_FREE_OBJECT(msgElems);
        /* error will be handled by the caller */
        return CW_FALSE;
    }

    seqNum = CWGetSeqNum();

    if(gWTPSendCurCfg)
    {
        gWTPCurCfgSeqNum = seqNum;
    }

    CWDebugLog("Echo Request Sent");

    if(!CWWTPSendPendingRequestEx(CW_MSG_TYPE_VALUE_ECHO_REQUEST,
                                  seqNum, msgElems, msgElemCount, 1, gWTPEchoInterval, CW_TRUE))
    {
        for(k = 0; k < msgElemCount; k++)
        {
            CW_FREE_PROTOCOL_MESSAGE(msgElems[k]);
        }
        CW_FREE_OBJECT(msgElems);
        /* error will be handled by the caller */
        return CW_FALSE;
    }

    if(memInfo.total)
    {
        /* Check memory usage reachs the log threshold */
        if(memInfo.used * 100 >= memInfo.total * gWTPMemUsageLogThreshold)
        {
            unsigned int usage, memUsed;
            char *process;

            /* usage in percentage */
            usage = (memInfo.used * 100) / memInfo.total;

            /* send event log only when the usage is increasing */
            if(usage > gWTPMemUsageLast)
            {
                CWLog("Memory Usage %u%% Reaches Log Thresdhold %u%%",
                      usage, gWTPMemUsageLogThreshold);

                CWDebugLog("Send Memory Usage Log Event");

                if(CWGetTopMemoryProcess(&process, &memUsed))
                {
                    CWWTPSendLogEvent(SN_EG_DEV_STATE, SN_LOG_CAT_WTP,
                                      SN_LOG_SEV_WARNING,
                                      "memory usage reaches %u%%, top process is %s using %u KB",
                                      usage, process, memUsed);
                    CW_FREE_OBJECT(process);
                }
                else
                {
                    CWWTPSendLogEvent(SN_EG_DEV_STATE, SN_LOG_CAT_WTP,
                                      SN_LOG_SEV_WARNING,
                                      "memory usage reaches %u%%",
                                      usage);
                }

                gWTPMemUsageLast = usage;
            }
        }
        else
        {
            gWTPMemUsageLast = 0;
        }
    }

    return CW_TRUE;
}

CWBool CWAssembleResetResponse(CWProtocolMessage **messagesPtr,
                               int *fragmentsNumPtr,
                               int PMTU,
                               int seqNum,
                               CWProtocolResultCode resultCode)
{
    CWProtocolMessage *msgElems = NULL;
    const int msgElemCount = resultCode == CW_PROTOCOL_SUCCESS ? 2 : 1;
    CWProtocolMessage *msgElemsBinding = NULL;
    const int msgElemBindingCount = 0;
    int k = -1;

    if(messagesPtr == NULL || fragmentsNumPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems,
                                     msgElemCount,
                                     return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    CWDebugLog("Assembling Reset Response...");

    if((!(CWAssembleMsgElemResultCode(&(msgElems[++k]), resultCode))) ||
       (msgElemCount == 2 && !(CWAssembleMsgElemVendorPayloadWaitingTime(&(msgElems[++k]), CWWTPBoardGetRebootTime(CW_FALSE))))
      )
    {
        int j;
        for(j = 0; j < k; j++)
        {
            CW_FREE_PROTOCOL_MESSAGE(msgElems[j]);
        }
        CW_FREE_OBJECT(msgElems);
        /* error will be handled by the caller */
        return CW_FALSE;
    }

    if(!(CWAssembleMessage(messagesPtr,
                           fragmentsNumPtr,
                           PMTU,
                           seqNum,
                           CW_MSG_TYPE_VALUE_RESET_RESPONSE,
                           msgElems,
                           msgElemCount,
                           msgElemsBinding,
                           msgElemBindingCount,
                           CW_PACKET_CRYPT
                          )))
    {
        return CW_FALSE;
    }

    CWDebugLog("Reset Response Assembled");

    return CW_TRUE;
}


CWBool CWAssembleChangeACResponse(CWProtocolMessage **messagesPtr,
                                  int *fragmentsNumPtr,
                                  int PMTU,
                                  int seqNum,
                                  CWProtocolResultCode resultCode)
{
    CWProtocolMessage *msgElems = NULL;
    const int msgElemCount =  1;
    CWProtocolMessage *msgElemsBinding = NULL;
    const int msgElemBindingCount = 0;
    int k = -1;

    if(messagesPtr == NULL || fragmentsNumPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems,
                                     msgElemCount,
                                     return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    CWDebugLog("Assembling Change AC Response...");

    if((!(CWAssembleMsgElemResultCode(&(msgElems[++k]), resultCode)))
      )
    {
        int j;
        for(j = 0; j < k; j++)
        {
            CW_FREE_PROTOCOL_MESSAGE(msgElems[j]);
        }
        CW_FREE_OBJECT(msgElems);
        /* error will be handled by the caller */
        return CW_FALSE;
    }

    if(!(CWAssembleMessage(messagesPtr,
                           fragmentsNumPtr,
                           PMTU,
                           seqNum,
                           CW_MSG_TYPE_VALUE_CHANGE_AC_RESPONSE,
                           msgElems,
                           msgElemCount,
                           msgElemsBinding,
                           msgElemBindingCount,
                           CW_PACKET_CRYPT
                          )))
    {
        return CW_FALSE;
    }

    CWDebugLog("Change AC Response Assembled");

    return CW_TRUE;
}

CWBool CWAssembleWTPDataTransferRequest(CWProtocolMessage **messagesPtr, int *fragmentsNumPtr,
                                        int PMTU, int seqNum, CWList msgElemList)
{
    CWProtocolMessage *msgElems = NULL;
    int msgElemCount = 0;
    CWProtocolMessage *msgElemsBinding = NULL;
    const int msgElemBindingCount = 0;
    int i;
    CWListElement *current;
    int k = -1;

    if(messagesPtr == NULL || fragmentsNumPtr == NULL || msgElemList == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    msgElemCount = CWCountElementInList(msgElemList);

    if(msgElemCount > 0)
    {
        CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems, msgElemCount, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
    }
    else
    {
        msgElems = NULL;
    }

    CWDebugLog("Assembling WTP Data Transfer Request...");

    current = msgElemList;
    for(i = 0; i < msgElemCount; i++)
    {
        switch(((CWMsgElemData *) current->data)->type)
        {
            case CW_MSG_ELEMENT_DATA_TRANSFER_DATA_CW_TYPE:
                if(!(CWAssembleMsgElemDataTransferData(&(msgElems[++k]), ((CWMsgElemData *) current->data)->value)))
                {
                    goto cw_assemble_error;
                }
                break;
            /*case CW_MSG_ELEMENT_DATA_TRANSFER_MODE_CW_TYPE:
            	if (!(CWAssembleMsgElemDataTansferMode(&(msgElems[++k]))))
            		goto cw_assemble_error;
            	break;*/

            default:
                goto cw_assemble_error;
                break;
        }

        current = current->next;
    }

    if(!(CWAssembleMessage(messagesPtr,
                           fragmentsNumPtr,
                           PMTU,
                           seqNum,
                           CW_MSG_TYPE_VALUE_DATA_TRANSFER_REQUEST,
                           msgElems,
                           msgElemCount,
                           msgElemsBinding,
                           msgElemBindingCount,
#ifdef CW_NO_DTLS
                           CW_PACKET_PLAIN
#else
                           CW_PACKET_CRYPT
#endif
                          )))
    {
        return CW_FALSE;
    }

    CWDebugLog("WTP Data Transfer Request Assembled");

    return CW_TRUE;

cw_assemble_error:

    for(i = 0; i <= k; i++)
    {
        CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);
    }

    CW_FREE_OBJECT(msgElems);
    return CW_FALSE; // error will be handled by the caller
}

CWBool CWAssembleWTPEventRequest(CWProtocolMessage **messagesPtr,
                                 int *fragmentsNumPtr,
                                 int PMTU,
                                 int seqNum,
                                 CWList msgElemList)
{

    CWProtocolMessage *msgElems = NULL;
    int msgElemCount = 0;
    CWProtocolMessage *msgElemsBinding = NULL;
    const int msgElemBindingCount = 0;
    int i;
    CWListElement *current;
    int k = -1;

    if(messagesPtr == NULL || fragmentsNumPtr == NULL || msgElemList == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    msgElemCount = CWCountElementInList(msgElemList);

    if(msgElemCount > 0)
    {
        CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems,
                                         msgElemCount,
                                         return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
    }
    else
    {
        msgElems = NULL;
    }

    CWDebugLog("Assembling WTP Event Request...");

    current = msgElemList;
    for(i = 0; i < msgElemCount; i++)
    {

        switch(((CWMsgElemData *) current->data)->type)
        {
#if 0
            case CW_MSG_ELEMENT_CW_DECRYPT_ER_REPORT_CW_TYPE:
                if(!(CWAssembleMsgElemDecryptErrorReport(&(msgElems[++k]), ((CWMsgElemData *) current->data)->value)))
                {
                    goto cw_assemble_error;
                }
                break;
#endif
            case CW_MSG_ELEMENT_DUPLICATE_IPV4_ADDRESS_CW_TYPE:
                if(!(CWAssembleMsgElemDuplicateIPv4Address(&(msgElems[++k]))))
                {
                    goto cw_assemble_error;
                }
                break;
            case CW_MSG_ELEMENT_DUPLICATE_IPV6_ADDRESS_CW_TYPE:
                if(!(CWAssembleMsgElemDuplicateIPv6Address(&(msgElems[++k]))))
                {
                    goto cw_assemble_error;
                }
                break;
#if 0
            case CW_MSG_ELEMENT_WTP_OPERAT_STATISTICS_CW_TYPE:
                if(!(CWAssembleMsgElemWTPOperationalStatistics(&(msgElems[++k]), ((CWMsgElemData *) current->data)->value)))
                {
                    goto cw_assemble_error;
                }
                break;
            case CW_MSG_ELEMENT_WTP_RADIO_STATISTICS_CW_TYPE:
                if(!(CWAssembleMsgElemWTPRadioStatistics(&(msgElems[++k]), ((CWMsgElemData *) current->data)->value)))
                {
                    goto cw_assemble_error;
                }
                break;
            case CW_MSG_ELEMENT_WTP_REBOOT_STATISTICS_CW_TYPE:
                if(!(CWAssembleMsgElemWTPRebootStatistics(&(msgElems[++k]))))
                {
                    goto cw_assemble_error;
                }
                break;
#endif
            default:
                goto cw_assemble_error;
                break;
        }
        current = current->next;
    }

    if(!(CWAssembleMessage(messagesPtr,
                           fragmentsNumPtr,
                           PMTU,
                           seqNum,
                           CW_MSG_TYPE_VALUE_WTP_EVENT_REQUEST,
                           msgElems,
                           msgElemCount,
                           msgElemsBinding,
                           msgElemBindingCount,
                           CW_PACKET_CRYPT
                          )))
    {
        return CW_FALSE;
    }

    CWDebugLog("WTP Event Request Assembled");

    return CW_TRUE;

cw_assemble_error:
    {
        int i;
        for(i = 0; i <= k; i++)
        {
            CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);
        }
        CW_FREE_OBJECT(msgElems);
        return CW_FALSE; // error will be handled by the caller
    }
}

/*Update 2009:
	Added values to args... values is used to determine if we have some
	payload (in this case only vendor and only UCI) to send back with the
	configuration update response*/
CWBool CWAssembleConfigurationUpdateResponse(CWProtocolMessage **messagesPtr,
        int *fragmentsNumPtr,
        int PMTU,
        int seqNum,
        CWProtocolConfigurationUpdateRequestValues *valuePtr)
{

    CWProtocolMessage *msgElems = NULL;
    int msgElemCount = 0;
    CWProtocolMessage *msgElemsBinding = NULL;
    const int msgElemBindingCount = 0;

    if(messagesPtr == NULL || fragmentsNumPtr == NULL || valuePtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    CWDebugLog("Assembling Configuration Update Response...");

    if(valuePtr->vendorValuesCfg.vendorId == CW_VENDOR_ID)
    {
        switch(valuePtr->vendorValuesCfg.vendorPayloadType)
        {
            case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_WTP_CFG:
            {
                msgElemCount = gWTPCfgRollbackList.head ? 3 : 2;

                CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems, msgElemCount, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

                if(!(CWAssembleMsgElemResultCode(&msgElems[0], gWTPResultCode)))
                {
                    CW_FREE_OBJECT(msgElems);
                    return CW_FALSE;
                }

                if(!(CWAssembleMsgElemVendorPayloadWtpCfgResult(&msgElems[1], &gWTPCfgResult)))
                {
                    CW_FREE_PROTOCOL_MESSAGE(msgElems[0]);
                    CW_FREE_OBJECT(msgElems);
                    return CW_FALSE;
                }

                if(gWTPCfgRollbackList.head)
                {
                    if(!(CWAssembleMsgElemVendorPayloadWtpCfg(&msgElems[2], &gWTPCfgRollbackList)))
                    {
                        CW_FREE_PROTOCOL_MESSAGE(msgElems[0]);
                        CW_FREE_PROTOCOL_MESSAGE(msgElems[1]);
                        CW_FREE_OBJECT(msgElems);
                        return CW_FALSE;
                    }
                }
                break;
            }
#ifdef CW_WTP_AP
            case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_BACKGROUND_SITESURVEY:
            {
                msgElemCount = 1;

                CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems, msgElemCount, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

                if(!(CWAssembleMsgElemResultCode(&msgElems[0], gWTPResultCode)))
                {
                    CW_FREE_OBJECT(msgElems);
                    return CW_FALSE;
                }
                break;
            }
            case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_AUTO_TXPOWER:
            {
                msgElemCount = 2;

                CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems, msgElemCount, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

                if(!(CWAssembleMsgElemResultCode(&msgElems[0], gWTPResultCode)))
                {
                    CW_FREE_OBJECT(msgElems);
                    return CW_FALSE;
                }

                if(!(CWWTPAssembleMsgElemAutoTxPowerCurrentValue(&msgElems[1], ((CWAutoTxPowerHealingValues *)(valuePtr->vendorValuesCfg.payload))->radioType)))
                {
                    CW_FREE_PROTOCOL_MESSAGE(msgElems[0]);
                    CW_FREE_OBJECT(msgElems);
                    return CW_FALSE;
                }
                break;
            }
#endif /* CW_WTP_AP */
            default:
            {
                /*Result Code only*/
                msgElemCount = 1;

                CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems, msgElemCount, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

                if(!(CWAssembleMsgElemResultCode(&msgElems[0], gWTPResultCode)))
                {
                    CW_FREE_OBJECT(msgElems);
                    return CW_FALSE;
                }
            }
        }
    }
    else
    {
        /*Result Code only*/
        msgElemCount = 1;

        CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems, msgElemCount, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

        if(!(CWAssembleMsgElemResultCode(&msgElems[0], gWTPResultCode)))
        {
            CW_FREE_OBJECT(msgElems);
            return CW_FALSE;
        }
    }

    if(!(CWAssembleMessage(messagesPtr,
                           fragmentsNumPtr,
                           PMTU,
                           seqNum,
                           CW_MSG_TYPE_VALUE_CONFIGURE_UPDATE_RESPONSE,
                           msgElems,
                           msgElemCount,
                           msgElemsBinding,
                           msgElemBindingCount,
                           CW_PACKET_CRYPT
                          )))
    {
        return CW_FALSE;
    }

    CWDebugLog("Configuration Update Response Assembled");

    return CW_TRUE;
}

CWBool CWAssembleClearConfigurationResponse(CWProtocolMessage **messagesPtr, int *fragmentsNumPtr,
        int PMTU, int seqNum, CWProtocolResultCode resultCode)
{
    CWProtocolMessage *msgElems = NULL;
    const int msgElemCount = resultCode == CW_PROTOCOL_SUCCESS ? 2 : 1;
    CWProtocolMessage *msgElemsBinding = NULL;
    const int msgElemBindingCount = 0;
    int k = -1;

    if(messagesPtr == NULL || fragmentsNumPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    CWDebugLog("Assembling Clear Configuration Response...");

    CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems,
                                     msgElemCount,
                                     return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if(!(CWAssembleMsgElemResultCode(&(msgElems[++k]), resultCode)) ||
       (msgElemCount == 2 && !(CWAssembleMsgElemVendorPayloadWaitingTime(&(msgElems[++k]), CWWTPBoardGetRebootTime(CW_TRUE))))
      )
    {
        int j;
        for(j = 0; j < k; j++)
        {
            CW_FREE_PROTOCOL_MESSAGE(msgElems[j]);
        }
        CW_FREE_OBJECT(msgElems);
        /* error will be handled by the caller */
        return CW_FALSE;
    }

    if(!(CWAssembleMessage(messagesPtr,
                           fragmentsNumPtr,
                           PMTU,
                           seqNum,
                           CW_MSG_TYPE_VALUE_CLEAR_CONFIGURATION_RESPONSE,
                           msgElems,
                           msgElemCount,
                           msgElemsBinding,
                           msgElemBindingCount,
                           CW_PACKET_CRYPT
                          )))
    {
        return CW_FALSE;
    }

    CWDebugLog("Clear Configuration Response Assembled");

    return CW_TRUE;
}
CWBool CWAssembleStandAloneResponse(CWProtocolMessage **messagesPtr, int *fragmentsNumPtr,
                                    int PMTU, int seqNum, CWProtocolResultCode resultCode)
{
    CWProtocolMessage *msgElems = NULL;
    int msgElemCount = 1;
    CWProtocolMessage *msgElemsBinding = NULL;
    const int msgElemBindingCount = 0;
    int k = -1;

    if(messagesPtr == NULL || fragmentsNumPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    CWDebugLog("Assembling standalone Response...");

    CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems,
                                     msgElemCount,
                                     return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if(!(CWAssembleMsgElemResultCode(&msgElems[++k], resultCode)))
    {
        CW_FREE_OBJECT(msgElems);
        return CW_FALSE;
    }

    if(!(CWAssembleMessage(messagesPtr,
                           fragmentsNumPtr,
                           PMTU,
                           seqNum,
                           CW_MSG_TYPE_VALUE_STANDALONE_MODE_RESPONSE,
                           msgElems,
                           msgElemCount,
                           msgElemsBinding,
                           msgElemBindingCount,
                           CW_PACKET_CRYPT
                          )))
    {
        return CW_FALSE;
    }

    CWDebugLog("Standalone Response Assembled");

    return CW_TRUE;
}

CWBool CWAssembleStationConfigurationResponse(CWProtocolMessage **messagesPtr, int *fragmentsNumPtr,
        int PMTU, int seqNum, CWProtocolResultCode resultCode)
{
    CWProtocolMessage *msgElems = NULL;
    const int msgElemCount = 1;
    CWProtocolMessage *msgElemsBinding = NULL;
    const int msgElemBindingCount = 0;

    if(messagesPtr == NULL || fragmentsNumPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    CWDebugLog("Assembling Sattion Configuration Response...");

    CW_CREATE_OBJECT_ERR(msgElems, CWProtocolMessage, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if(!(CWAssembleMsgElemResultCode(msgElems, resultCode)))
    {
        CW_FREE_OBJECT(msgElems);
        return CW_FALSE;
    }

    if(!(CWAssembleMessage(messagesPtr,
                           fragmentsNumPtr,
                           PMTU,
                           seqNum,
                           CW_MSG_TYPE_VALUE_STATION_CONFIGURATION_RESPONSE,
                           msgElems,
                           msgElemCount,
                           msgElemsBinding,
                           msgElemBindingCount,
                           CW_PACKET_CRYPT
                          )))
    {
        return CW_FALSE;
    }

    CWDebugLog("Station Configuration Response Assembled");

    return CW_TRUE;
}

CWBool CWAssembleShellCmdResponse(CWProtocolMessage **messagesPtr,
                                  int *fragmentsNumPtr,
                                  int PMTU,
                                  int seqNum,
                                  CWProtocolResultCode code,
                                  char *output)
{
    CWProtocolMessage *msgElems = NULL;
    const int msgElemCount = code == CW_PROTOCOL_SUCCESS ? 2 : 1;
    CWProtocolMessage *msgElemsBinding = NULL;
    const int msgElemBindingCount = 0;
    int k = -1;

    if(messagesPtr == NULL || fragmentsNumPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    CWDebugLog("Assembling Shell Cmd Response...");

    CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems,
                                     msgElemCount,
                                     return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if(!(CWAssembleMsgElemResultCode(&(msgElems[++k]), code)) ||
       (code == CW_PROTOCOL_SUCCESS && !(CWAssembleMsgElemVendorPayloadShellCmdOutput(&(msgElems[++k]), output)))
      )
    {
        int j;
        for(j = 0; j < k; j++)
        {
            CW_FREE_PROTOCOL_MESSAGE(msgElems[j]);
        }
        CW_FREE_OBJECT(msgElems);
        /* error will be handled by the caller */
        return CW_FALSE;
    }

    if(!(CWAssembleMessage(messagesPtr,
                           fragmentsNumPtr,
                           PMTU,
                           seqNum,
                           CW_MSG_TYPE_VALUE_SHELL_CMD_RESPONSE,
                           msgElems,
                           msgElemCount,
                           msgElemsBinding,
                           msgElemBindingCount,
                           CW_PACKET_CRYPT
                          )))
    {
        return CW_FALSE;
    }

    CWDebugLog("Shell Cmd Response Assembled");

    return CW_TRUE;
}

CWBool CWAssembleFailureResponse(CWProtocolMessage **messagesPtr,
                                 int *fragmentsNumPtr,
                                 int PMTU,
                                 int seqNum,
                                 int msgType,
                                 CWProtocolResultCode resultCode)
{
    CWProtocolMessage *msgElems = NULL;

    CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems,
                                     1,
                                     return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if(!(CWAssembleMsgElemResultCode(&msgElems[0], resultCode)))
    {
        CW_FREE_OBJECT(msgElems);
        return CW_FALSE;
    }

    if(!(CWAssembleMessage(messagesPtr,
                           fragmentsNumPtr,
                           PMTU,
                           seqNum,
                           msgType,
                           msgElems,
                           1,
                           NULL,
                           0,
                           CW_PACKET_CRYPT
                          )))
    {
        return CW_FALSE;
    }

    CWDebugLog("Failure Response of msgType %d Assembled", msgType);

    return CW_TRUE;
}

#ifdef CW_WTP_AP
CWBool CWAssembleSitesurveyResponse(CWProtocolMessage **messagesPtr,
                                    int *fragmentsNumPtr,
                                    int PMTU,
                                    int seqNum,
                                    CWProtocolResultCode resultCode,
                                    CWWTPSitesurveyInfo *sitesurveyInfo)
{
    CWProtocolMessage *msgElems = NULL;
    int msgElemCount = 1;
    CWProtocolMessage *msgElemsBinding = NULL;
    const int msgElemBindingCount = 0;
    int k = -1;

    if(messagesPtr == NULL || fragmentsNumPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    CWDebugLog("Assembling Sitesurvey Response...");

    if(resultCode == CW_PROTOCOL_SUCCESS)
    {
        msgElemCount++;
    }

    CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems,
                                     msgElemCount,
                                     return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if(!(CWAssembleMsgElemResultCode(&msgElems[++k], resultCode)) ||
       (resultCode == CW_PROTOCOL_SUCCESS && !CWWTPAssembleMsgElemVendorPayloadSitesurveyInfo(&(msgElems[++k]), sitesurveyInfo))
      )
    {
        int j;
        for(j = 0; j <= k; j++)
        {
            CW_FREE_PROTOCOL_MESSAGE(msgElems[j]);
        }
        CW_FREE_OBJECT(msgElems);
        /* error will be handled by the caller */
        return CW_FALSE;
    }

    if(!(CWAssembleMessage(messagesPtr,
                           fragmentsNumPtr,
                           PMTU,
                           seqNum,
                           CW_MSG_TYPE_VALUE_SITESURVEY_RESPONSE,
                           msgElems,
                           msgElemCount,
                           msgElemsBinding,
                           msgElemBindingCount,
                           CW_PACKET_CRYPT
                          )))
    {
        return CW_FALSE;
    }

    CWDebugLog("Sitesurvey Response Assembled");

    return CW_TRUE;
}

CWBool CWAssembleKickmacResponse(CWProtocolMessage **messagesPtr,
                                 int *fragmentsNumPtr,
                                 int PMTU,
                                 int seqNum,
                                 CWProtocolResultCode resultCode)
{
    CWProtocolMessage *msgElems = NULL;
    int msgElemCount = 1;
    CWProtocolMessage *msgElemsBinding = NULL;
    const int msgElemBindingCount = 0;
    int k = -1;

    if(messagesPtr == NULL || fragmentsNumPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    CWDebugLog("Assembling Kickmac Response...");

    CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems,
                                     msgElemCount,
                                     return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if(!(CWAssembleMsgElemResultCode(&msgElems[++k], resultCode)))
    {
        CW_FREE_OBJECT(msgElems);
        return CW_FALSE;
    }

    if(!(CWAssembleMessage(messagesPtr,
                           fragmentsNumPtr,
                           PMTU,
                           seqNum,
                           CW_MSG_TYPE_VALUE_KICKMAC_RESPONSE,
                           msgElems,
                           msgElemCount,
                           msgElemsBinding,
                           msgElemBindingCount,
                           CW_PACKET_CRYPT
                          )))
    {
        return CW_FALSE;
    }

    CWDebugLog("Kickmac Response Assembled");

    return CW_TRUE;
}
#endif /* CW_WTP_AP */

/*_______________________________________________________________*/
/*  *******************___PARSE FUNCTIONS___*******************  */

CWBool CWParseConfigurationUpdateRequest(char *msg,
        int len,
        CWProtocolConfigurationUpdateRequestValues *valuesPtr)
{
    CWProtocolMessage completeMsg;
    CWProtocolVendorSpecificValues vendorValue;
    CWBool vendorGot = CW_FALSE;
    CWBool imgIdGot = CW_FALSE;

    if(msg == NULL || valuesPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    CWDebugLog("Parsing Configuration Update Request...");

    completeMsg.msg = msg;
    completeMsg.offset = 0;

    CW_ZERO_MEMORY(valuesPtr, sizeof(CWProtocolConfigurationUpdateRequestValues));

    /* parse message elements */
    while(completeMsg.offset < len)
    {
        unsigned short int elemType = 0;	/* = CWProtocolRetrieve32(&completeMsg); */
        unsigned int elemLen = 0;	/* = CWProtocolRetrieve16(&completeMsg); */

        CWParseFormatMsgElem(&completeMsg, &elemType, &elemLen);

        /* CWDebugLog("Parsing Message Element: %u, elemLen: %u", elemType, elemLen); */
        CWDebugLog("Parsing Message Element: %u, elemLen: %u", elemType, elemLen);

        switch(elemType)
        {
            case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE:
            case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE_32_LEN:
                if(!CWParseVendorPayload(&completeMsg, elemLen, &vendorValue))
                {
                    CWDestroyConfigurationUpdateRequestValues(valuesPtr);
                    return CW_FALSE;
                }
                if(vendorValue.vendorPayloadType == CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_WTP_CFG ||
                   vendorValue.vendorPayloadType == CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_BACKGROUND_SITESURVEY ||
                   vendorValue.vendorPayloadType == CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_AUTO_TXPOWER)
                {
                    CWDestroyVendorSpecificValues(&(valuesPtr->vendorValuesCfg));
                    CW_COPY_MEMORY(&(valuesPtr->vendorValuesCfg), &vendorValue, sizeof(CWProtocolVendorSpecificValues));
                    vendorGot = CW_TRUE;
                }
                else
                {
                    CWDestroyVendorSpecificValues(&vendorValue);
                }
                break;
            case CW_MSG_ELEMENT_IMAGE_IDENTIFIER_CW_TYPE:
                if(!CWParseImageIdentifier(&completeMsg, elemLen, &(valuesPtr->imageId)))
                {
                    CWDestroyConfigurationUpdateRequestValues(valuesPtr);
                    return CW_FALSE;
                }
                imgIdGot = CW_TRUE;
                break;
            case CW_MSG_ELEMENT_WAIT_APPLY:
                if(elemLen != 0)
                {
                    CWDestroyConfigurationUpdateRequestValues(valuesPtr);
                    return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Incorrect length of CW_MSG_ELEMENT_WAIT_APPLY");
                }
                valuesPtr->waitApply = CW_TRUE;
                break;
            default:
                completeMsg.offset += elemLen;
                CWLog("Unrecognized Message Element %d in Configuration Update Request", elemType);
                break;
        }
    }

    if(completeMsg.offset != len)
    {
        CWDestroyConfigurationUpdateRequestValues(valuesPtr);
        return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Garbage at the End of the Message");
    }

    if(!vendorGot && !imgIdGot)
    {
        CWDestroyConfigurationUpdateRequestValues(valuesPtr);
        return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "No element in Configuration Update Request");
    }

    CWDebugLog("Configure Update Request Parsed");

    return CW_TRUE;
}

CWBool CWParseStationConfigurationRequest(char *msg, int len)
{
    //CWBool bindingMsgElemFound=CW_FALSE;
    CWProtocolMessage completeMsg;

    if(msg == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    CWDebugLog("Parsing Station Configuration Request...");

    completeMsg.msg = msg;
    completeMsg.offset = 0;

    //valuesPtr->bindingValues = NULL;

    // parse message elements
    while(completeMsg.offset < len)
    {
        unsigned short int elemType = 0;
        unsigned int elemLen = 0;

        CWParseFormatMsgElem(&completeMsg, &elemType, &elemLen);

        switch(elemType)
        {

            case CW_MSG_ELEMENT_ADD_STATION_CW_TYPE:
                if(!(CWParseAddStation(&completeMsg,  elemLen)))
                {
                    return CW_FALSE;
                }
                break;
            default:
                completeMsg.offset += elemLen;
                CWLog("Unrecognized Message Element %d in Station Configuration Request", elemType);
        }
    }

    if(completeMsg.offset != len)
    {
        return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Garbage at the End of the Message");
    }
    /*
    if(bindingMsgElemFound)
    {
    	if(!(CWBindingParseConfigurationUpdateRequest (msg, len, &(valuesPtr->bindingValues))))
    	{
    		return CW_FALSE;
    	}
    }*/

    CWDebugLog("Station Configuration Request Parsed");

    return CW_TRUE;
}

CWBool CWParseApplyConfigRequest(char *msg, int len, CWProtocolVendorSpecificValues *venderValuesPtr)
{
    CWProtocolMessage completeMsg;
    CWProtocolVendorSpecificValues vendorValues;

    if(msg == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    CWDebugLog("Parsing Apply Config Request...");

    completeMsg.msg = msg;
    completeMsg.offset = 0;

    CW_ZERO_MEMORY(venderValuesPtr, sizeof(CWProtocolVendorSpecificValues));

    // parse message elements
    while(completeMsg.offset < len)
    {
        unsigned short int elemType = 0;
        unsigned int elemLen = 0;

        CWParseFormatMsgElem(&completeMsg, &elemType, &elemLen);

        switch(elemType)
        {
            case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE:
                if(!CWParseVendorPayload(&completeMsg, elemLen, &vendorValues))
                {
                    return CW_FALSE;
                }
                if(vendorValues.vendorPayloadType == CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_ALLPY_CONFIG_INFO)
                {
                    CWDestroyVendorSpecificValues(venderValuesPtr);
                    CW_COPY_MEMORY(venderValuesPtr, &vendorValues, sizeof(CWProtocolVendorSpecificValues));
                }
                else
                {
                    CWDestroyVendorSpecificValues(&vendorValues);
                }
                break;
            default:
                completeMsg.offset += elemLen;
                CWLog("Unrecognized Message Element %d in Apply Config Request", elemType);
        }
    }

    if(!venderValuesPtr->payload)
    {
        return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Require CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_ALLPY_CONFIG_INFO");
    }

    if(completeMsg.offset != len)
    {
        CWDestroyVendorSpecificValues(venderValuesPtr);
        return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Garbage at the End of the Message");
    }

    CWDebugLog("Apply Config Request Parsed");

    return CW_TRUE;
}

CWBool CWParseEchoResponse(char *msg, int len, CWProtocolEchoResponseValues *values)
{
    CWProtocolMessage completeMsg;
    CWProtocolVendorSpecificValues vendorValues;

    if(msg == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    CWDebugLog("Parsing Echo Response...");

    completeMsg.msg = msg;
    completeMsg.offset = 0;

    CW_ZERO_MEMORY(values, sizeof(CWProtocolEchoResponseValues));
    values->debugLog = -1;

    // parse message elements
    while(completeMsg.offset < len)
    {
        unsigned short int elemType = 0;
        unsigned int elemLen = 0;

        CWParseFormatMsgElem(&completeMsg, &elemType, &elemLen);

        switch(elemType)
        {
            case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE:
                if(!CWParseVendorPayload(&completeMsg, elemLen, &vendorValues))
                {
                    return CW_FALSE;
                }
                if(vendorValues.vendorPayloadType == CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_STATION_STATISTIC_MAX_CLIENTS)
                {
                    values->statsMaxClients = (int)(long) vendorValues.payload;
                }
                else if(vendorValues.vendorPayloadType == CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_DEBUG_LOG)
                {
                    values->debugLog = (int)(long) vendorValues.payload;
                }
                else if(vendorValues.vendorPayloadType == CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_UTC_TIME)
                {
                    values->utc_time = (int)(long) vendorValues.payload;
                }
                else
                {
                    CWDestroyVendorSpecificValues(&vendorValues);
                }
                break;
            default:
                completeMsg.offset += elemLen;
                CWLog("Unrecognized Message Element %d in Echo Response", elemType);
        }
    }

    if(completeMsg.offset != len)
    {
        return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Garbage at the End of the Message");
    }

    CWDebugLog("Echo Response Parsed");

    return CW_TRUE;
}

CWBool CWParseWTPEventResponseMessage(char *msg, int len, int seqNum, void *values)
{
    CWControlHeaderValues controlVal;
    CWProtocolMessage completeMsg;

    if(msg == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    CWDebugLog("Parsing WTP Event Response...");

    completeMsg.msg = msg;
    completeMsg.offset = 0;

    /* error will be handled by the caller */
    if(!(CWParseControlHeader(&completeMsg, &controlVal)))
    {
        return CW_FALSE;
    }

    /* different type */
    if(controlVal.messageTypeValue != CW_MSG_TYPE_VALUE_WTP_EVENT_RESPONSE)
        return CWErrorRaise(CW_ERROR_INVALID_FORMAT,
                            "Message is not WTP Event Response as Expected");

    if(controlVal.seqNum != seqNum)
        return CWErrorRaise(CW_ERROR_INVALID_FORMAT,
                            "Different Sequence Number");

    /* skip timestamp */
    controlVal.msgElemsLen -= CW_CONTROL_HEADER_OFFSET_FOR_MSG_ELEMS;

    if(controlVal.msgElemsLen != 0)
        return CWErrorRaise(CW_ERROR_INVALID_FORMAT,
                            "WTP Event Response must carry no message element");

    CWDebugLog("WTP Event Response Parsed...");

    return CW_TRUE;
}

CWBool CWParseShellCmdRequest(char *msg, int len, CWProtocolVendorSpecificValues *shellCmd)
{
    CWProtocolMessage completeMsg;
    CWProtocolVendorSpecificValues vendorValues;

    if(msg == NULL || shellCmd == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    CWDebugLog("Parsing Shell Cmd Request...");
    completeMsg.msg = msg;
    completeMsg.offset = 0;

    CW_ZERO_MEMORY(shellCmd, sizeof(CWProtocolVendorSpecificValues));
    CW_ZERO_MEMORY(&vendorValues, sizeof(CWProtocolVendorSpecificValues));

    while(completeMsg.offset < len)
    {
        unsigned short int elemType = 0;	/* = CWProtocolRetrieve32(&completeMsg); */
        unsigned int elemLen = 0;	/* = CWProtocolRetrieve16(&completeMsg); */

        CWParseFormatMsgElem(&completeMsg, &elemType, &elemLen);
        switch(elemType)
        {
            case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE:
                if(!CWParseVendorPayload(&completeMsg, elemLen, &vendorValues))
                {
                    return CW_FALSE;
                }
                if(vendorValues.vendorPayloadType == CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_SHELL_CMD)
                {
                    CWDestroyVendorSpecificValues(shellCmd);
                    CW_COPY_MEMORY(shellCmd, &vendorValues, sizeof(CWProtocolVendorSpecificValues));
                }
                else
                {
                    CWDestroyVendorSpecificValues(&vendorValues);
                }
                break;
            default:
                completeMsg.offset += elemLen;
                CWLog("Unrecognized Message Element %d in Shell Cmd Request", elemType);
                break;
        }
    }

    if(!shellCmd->payload)
    {
        return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "No CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_SHELL_CMD found");
    }

    if(completeMsg.offset != len)
    {
        return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Garbage at the End of the Message");
    }

    CWDebugLog("Shell Cmd Request Request Parsed");

    return CW_TRUE;
}

CWBool CWParseChangeACRequest(char *msg,
                              int len,
                              CWHostName host)
{
    CWProtocolMessage completeMsg;
    unsigned long addr;

    if(msg == NULL || host == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    CWDebugLog("Parsing Change AC Request...");

    completeMsg.msg = msg;
    completeMsg.offset = 0;

    host[0] = '\0';
    while(completeMsg.offset < len)
    {
        unsigned short int elemType = 0;	/* = CWProtocolRetrieve32(&completeMsg); */
        unsigned int elemLen = 0;	/* = CWProtocolRetrieve16(&completeMsg); */

        CWParseFormatMsgElem(&completeMsg, &elemType, &elemLen);
        switch(elemType)
        {
            case CW_MSG_ELEMENT_IPV4_ADDR_CW_TYPE:
                addr = CWProtocolRetrieveIPv4Address(&completeMsg);
                sprintf(host, "%u.%u.%u.%u", CW_IPV4_PRINT_LIST(addr));
                break;
            case CW_MSG_ELEMENT_HOST_NAME_CW_TYPE:
                if(!CWParseHostName(&completeMsg, elemLen, host))
                {
                    return CW_FALSE;
                }
                break;
            default:
                completeMsg.offset += elemLen;
                CWLog("Unrecognized Message Element %d in Change AC Request", elemType);
                break;
        }
    }

    if(host[0] == '\0')
    {
        return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "No AC address found");
    }

    if(completeMsg.offset != len)
    {
        return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Garbage at the End of the Message");
    }

    CWDebugLog("Change AC Request Parsed");
    return CW_TRUE;
}

CWBool CWParseSitesurveyRequest(char *msg,
                                int len,
                                unsigned char *radio)
{
    CWProtocolMessage completeMsg;
    CWProtocolVendorSpecificValues vendorValue;

    if(msg == NULL || radio == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    CWDebugLog("Parsing Sitesurvey Request...");

    completeMsg.msg = msg;
    completeMsg.offset = 0;

    /* parse message elements */
    while(completeMsg.offset < len)
    {
        unsigned short int elemType = 0;	/* = CWProtocolRetrieve32(&completeMsg); */
        unsigned int elemLen = 0;	/* = CWProtocolRetrieve16(&completeMsg); */

        CWParseFormatMsgElem(&completeMsg, &elemType, &elemLen);

        /* CWDebugLog("Parsing Message Element: %u, elemLen: %u", elemType, elemLen); */
        CWDebugLog("Parsing Message Element: %u, elemLen: %u", elemType, elemLen);

        switch(elemType)
        {
            case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE:
                if(!CWParseVendorPayload(&completeMsg, elemLen, &vendorValue))
                {
                    CWDestroyVendorSpecificValues(&vendorValue);
                    return CW_FALSE;
                }
                if(vendorValue.vendorPayloadType == CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_SITESURVEY)
                {
                    *radio = (unsigned char)((int)(long)vendorValue.payload);
                }
                else
                {
                    CWDestroyVendorSpecificValues(&vendorValue);
                }
                break;
            default:
                completeMsg.offset += elemLen;
                CWLog("Unrecognized Message Element %d in Sitesurvey Request", elemType);
                break;
        }
    }

    if(completeMsg.offset != len)
    {
        CWDestroyVendorSpecificValues(&vendorValue);
        return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Garbage at the End of the Message");
    }

    CWDebugLog("Sitesurvey Request Parsed");

    return CW_TRUE;
}

CWBool CWParseKickmacRequest(char *msg, int len, CWProtocolVendorSpecificValues *kickInfo)
{
    CWProtocolMessage completeMsg;
    CWProtocolVendorSpecificValues vendorValues;

    if(msg == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    CWDebugLog("Parsing Kickmac Request...");

    completeMsg.msg = msg;
    completeMsg.offset = 0;

    CW_ZERO_MEMORY(kickInfo, sizeof(CWProtocolVendorSpecificValues));

    // parse message elements
    while(completeMsg.offset < len)
    {
        unsigned short int elemType = 0;
        unsigned int elemLen = 0;

        CWParseFormatMsgElem(&completeMsg, &elemType, &elemLen);

        switch(elemType)
        {
            case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE:
                if(!CWParseVendorPayload(&completeMsg, elemLen, &vendorValues))
                {
                    return CW_FALSE;
                }
                if(vendorValues.vendorPayloadType == CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_KICKMAC)
                {
                    CWDestroyVendorSpecificValues(kickInfo);
                    CW_COPY_MEMORY(kickInfo, &vendorValues, sizeof(CWProtocolVendorSpecificValues));
                }
                else
                {
                    CWDestroyVendorSpecificValues(&vendorValues);
                }
                break;
            default:
                completeMsg.offset += elemLen;
                CWLog("Unrecognized Message Element %d in Kick mac", elemType);
        }
    }

    if(!kickInfo->payload)
    {
        return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "NO CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_KICKMAC");
    }

    if(completeMsg.offset != len)
    {
        return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Garbage at the End of the Message");
    }

    CWDebugLog("Kick mac Parsed");

    return CW_TRUE;
}

CWBool CWParseWaitingTime(char *msg,
                          int len,
                          int *time)
{
    CWProtocolMessage completeMsg;
    CWProtocolVendorSpecificValues vendorValues;

    if(msg == NULL || time == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    completeMsg.msg = msg;
    completeMsg.offset = 0;
    *time = 0;

    while(completeMsg.offset < len)
    {
        unsigned short int elemType = 0;
        unsigned int elemLen = 0;

        CWParseFormatMsgElem(&completeMsg, &elemType, &elemLen);
        switch(elemType)
        {
            case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE:
                if(!(CWParseVendorPayload(&completeMsg, elemLen, &vendorValues)))
                {
                    return CW_FALSE;
                }
                if(vendorValues.vendorPayloadType != CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_WAITING_TIME)
                {
                    CWDestroyVendorSpecificValues(&vendorValues);
                }
                else
                {
                    *time = (int)(long) vendorValues.payload;
                }
                break;
            default:
                completeMsg.offset += elemLen;
                break;
        }
    }

    if(completeMsg.offset != len)
    {
        return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Garbage at the End of the Message");
    }

    return CW_TRUE;
}

/*______________________________________________________________*/
/*  *******************___SAVE FUNCTIONS___*******************  */
CWBool CWSaveWTPEventResponseMessage(void *WTPEventResp)
{
    CWDebugLog("Saving WTP Event Response...");
    CWDebugLog("WTP Response Saved");
    return CW_TRUE;
}

CWBool CWSaveConfigurationUpdateRequest(CWProtocolConfigurationUpdateRequestValues *valuesPtr, int *vendorPayloadType)
{
    CWProtocolVendorSpecificValues *vendorPtr;

    if(valuesPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    vendorPtr = &(valuesPtr->vendorValuesCfg);

    if(vendorPtr->vendorId != CW_VENDOR_ID)
    {
        CWLog("Unknow vendorId %d", vendorPtr->vendorId);
        gWTPResultCode = CW_PROTOCOL_FAILURE_UNRECOGNIZED_MSG_ELEM;
        *vendorPayloadType = vendorPtr->vendorPayloadType;
        return CW_TRUE;
    }

    /*Find out which custom vendor paylod really is...*/
    switch(vendorPtr->vendorPayloadType)
    {
        case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_WTP_CFG:
            if(!CWWTPSaveWtpCfg((CWWtpCfgMsgList *)(vendorPtr->payload)))
            {
                return CW_FALSE;
            }
            break;
#ifdef CW_WTP_AP
        case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_BACKGROUND_SITESURVEY:
            if(!CWWTPSaveBackgroundSitesurveyValues((CWBackgroundSitesurveyValues *)(vendorPtr->payload)))
            {
                return CW_FALSE;
            }
            break;
        case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_AUTO_TXPOWER:
            if(!CWWTPSaveAutoTxpowerHealingValues((CWAutoTxPowerHealingValues *)(vendorPtr->payload)))
            {
                return CW_FALSE;
            }
            break;
#endif
        default:
            gWTPResultCode = CW_PROTOCOL_FAILURE_UNRECOGNIZED_MSG_ELEM;
            CWLog("Unknow vendorPayloadType %d", vendorPtr->vendorPayloadType);
    }

    *vendorPayloadType = vendorPtr->vendorPayloadType;

    return CW_TRUE;
}

#if 0
CWBool CWSaveClearConfigurationRequest(CWProtocolResultCode *resultCode)
{
    *resultCode = CW_TRUE;

    /*Back to manufacturing default configuration*/

    if(!CWErr(CWWTPLoadConfiguration()) || !CWErr(CWWTPInitConfiguration()))
    {
        CWLog("Can't restore default configuration...");
        return CW_FALSE;
    }

    *resultCode = CW_TRUE;
    return CW_TRUE;
}
#endif

void CWDestroyConfigurationUpdateRequestValues(CWProtocolConfigurationUpdateRequestValues *valuesPtr)
{
    if(!valuesPtr)
    {
        return;
    }

    CWDestroyVendorSpecificValues(&(valuesPtr->vendorValuesCfg));
}

CWBool CWWTPSendLogEvent(int group, const char *category, int level, const char *msgFmt, ...)
{
    CWProtocolMessage *msgElems;
    char *msg;
    int msgLen;
    va_list va;

    va_start(va, msgFmt);

    msgLen = vsnprintf(NULL, 0, msgFmt, va); /* get how many buffer we need */

    va_end(va);

    msgLen++; /* + 1 for end of string */

    CW_CREATE_OBJECT_SIZE_ERR(msg, msgLen,
                              return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    va_start(va, msgFmt);

    vsnprintf(msg, msgLen, msgFmt, va);

    va_end(va);

    CWDebugLog("Send Log Event: group %d category %s level %d [%s]",
               group, category, level, msg);

    CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems, 1,
    {
        CW_FREE_OBJECT(msg);
        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
    });

    if(!CWWTPAssembleMsgElemLogMsg(&(msgElems[0]), group, category, level, msg))
    {
        CW_FREE_OBJECT(msg);
        CW_FREE_OBJECT(msgElems);
        return CW_FALSE;
    }

    CW_FREE_OBJECT(msg);

    if(!CWWTPSendPendingRequest(CW_MSG_TYPE_VALUE_WTP_EVENT_REQUEST, CWGetSeqNum(), msgElems, 1))
    {
        CW_FREE_PROTOCOL_MESSAGE(msgElems[0]);
        CW_FREE_OBJECT(msgElems);
        return CW_FALSE;
    }

    return CW_TRUE;
}

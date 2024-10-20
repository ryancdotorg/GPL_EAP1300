#include "CWWTP.h"
#include "CWVendorPayloads.h"
#include "WTPDataRecordStatistics.h"

#define STATS_POLLING_INTERVAL        5
#define STATS_MAX_POLLING_SEQ         0xFFFFFFF

typedef enum
{
    WTP_CLIENT_NONE = 0,
    WTP_CLIENT_OFFLINE,
    WTP_CLIENT_ONLINE
} CWClientChangeState;

typedef struct stlist_node
{
    CWMacAddress mac;
    unsigned int txLast;
    unsigned int txTotal;
    unsigned int rxLast;
    unsigned int rxTotal;
    int startTime;
    int lastTime;
    int pollSeq;
    unsigned char osTypeLen;
    unsigned char hostNameLen;
    char *osType;
    char *hostName;
    unsigned int address;
    CWClientChangeState changeState;
    struct stlist_node *next;
} CWClientStatsNode;

typedef struct
{
    int uploadCount;
    CWClientStatsNode *list;
} CWClientStatsInfo;

static CWClientStatsInfo **gClientStatsList = NULL;
static int gStateChangeCount = 0;
static int gMaxStatsUploadCount = 30; /* default */
static volatile CWBool gRunStatsPolling = CW_FALSE;
static volatile CWBool gHaltStats = CW_FALSE; /* keep statistics data when task end */
static int gPollingSeq;
static CWThreadMutex gStatsMutex = CW_MUTEX_INITIALIZER;
static CWThreadCondition gStatsCond = CW_COND_INITIALIZER;
static CWThread gStatsPollingThread;

static CWBool UpdateStatsNode(int radioIdx, int wlanIdx, CWStation *station);
static CWBool AddStatsNode(int radioIdx, int wlanIdx, CWStation *station);
static void ResetStatsNode();
static CW_THREAD_RETURN_TYPE StatsPollingTask(void *arg);
static CWBool CWAssembleMsgElemVendorPayloadStationStatistics(CWProtocolMessage *msgPtr);
static CWBool CWAssembleMsgElemWTPClientChange(CWProtocolMessage *msgPtr);

void CWWTPStartStats()
{
    if(!gRunStatsPolling)
    {
        gPollingSeq = 0;
        gRunStatsPolling = CW_TRUE;
        gHaltStats = CW_FALSE;

        if(!CWErr(CWCreateThread(&gStatsPollingThread, StatsPollingTask, NULL)))
        {
            CWLog("Error starting Thread that record clinet statistics on monitoring interface");
            gRunStatsPolling = CW_FALSE;
        }
    }
}

void CWWTPStopStats()
{
    if(gRunStatsPolling)
    {
        CWThreadMutexLock(&gStatsMutex);

        gRunStatsPolling = CW_FALSE;
        gHaltStats = CW_FALSE;
        CWSignalThreadCondition(&gStatsCond);

        CWThreadMutexUnlock(&gStatsMutex);

        /* wait for the end of thread */
        CWThreadJoin(gStatsPollingThread);
    }
}

void CWWTPHaltStats(CWBool enable)
{
    if(gRunStatsPolling && enable)
    {
        CWThreadMutexLock(&gStatsMutex);

        gRunStatsPolling = CW_FALSE;
        gHaltStats = CW_TRUE;
        CWSignalThreadCondition(&gStatsCond);

        CWThreadMutexUnlock(&gStatsMutex);

        /* wait for the end of thread */
        CWThreadJoin(gStatsPollingThread);
    }
    else if(!gRunStatsPolling && !enable)
    {
        gRunStatsPolling = CW_TRUE;
        gHaltStats = CW_FALSE;

        if(!CWErr(CWCreateThread(&gStatsPollingThread, StatsPollingTask, NULL)))
        {
            CWLog("Error starting Thread that record clinet statistics on monitoring interface");
            gRunStatsPolling = CW_FALSE;
        }
    }
}

void CWWTPSetStatsUploadInterval(int interval)
{
    gWTPStatisticsTimer = interval;
}

void CWWTPSetStatsPollInterval(int interval)
{
    gWTPStatisticsPollTimer = interval;
}

void CWWTPSetClientChangeEventEnable(CWBool enable)
{
    gWTPClientStateChangeEventEnable = enable;
}

void CWWTPSetStatsMaxClients(int maxClients)
{
    CWThreadMutexLock(&gStatsMutex);

    gMaxStatsUploadCount = maxClients;

    CWThreadMutexUnlock(&gStatsMutex);
}

static void ResetStatsNode()
{
    CWClientStatsNode *prev, *node;
    int radioIdx, wlanIdx;

    for(radioIdx = 0; radioIdx < CWWTPBoardGetMaxRadio(); radioIdx++)
    {
        for(wlanIdx = 0; wlanIdx < CWWTPBoardGetMaxRadioWlans(radioIdx); wlanIdx++)
        {
            prev = NULL;
            node = gClientStatsList[radioIdx][wlanIdx].list;
            while(node != NULL)
            {
                /* if this client was found in last polling, its polling seq must be equal to gPollingSeq */
                if(node->pollSeq == gPollingSeq)
                {
                    node->txTotal = 0; // reset for next period
                    node->rxTotal = 0;
                    node->startTime = CWGetUptime();
                    node->lastTime = node->startTime;
                    prev = node;
                    node = node->next;
                }
                /* Not found in last polling, delete it */
                else
                {
                    if(prev == NULL)
                    {
                        gClientStatsList[radioIdx][wlanIdx].list = node->next;
                    }
                    else
                    {
                        prev->next = node->next;
                    }

                    CWDebugLog("Del Client Stats: radio=%d, wlan=%d mac=%x:%x:%x:%x:%x:%x",
                               radioIdx, wlanIdx,
                               CW_MAC_PRINT_LIST(node->mac));

                    CW_FREE_OBJECT(node->hostName);
                    CW_FREE_OBJECT(node->osType);
                    CW_FREE_OBJECT(node);

                    if(prev == NULL)
                    {
                        node = gClientStatsList[radioIdx][wlanIdx].list;
                    }
                    else
                    {
                        node = prev->next;
                    }
                }
            }
        }
    }
}

static void ResetStateChangeNode()
{
    CWClientStatsNode *node;
    int radioIdx, wlanIdx;

    for(radioIdx = 0; radioIdx < CWWTPBoardGetMaxRadio(); radioIdx++)
    {
        for(wlanIdx = 0; wlanIdx < CWWTPBoardGetMaxRadioWlans(radioIdx); wlanIdx++)
        {
            node = gClientStatsList[radioIdx][wlanIdx].list;
            while(node != NULL)
            {
                node->changeState = WTP_CLIENT_NONE;
                node = node->next;
            }
        }
    }

    gStateChangeCount = 0;
}

static void CheckOfflineStateNode()
{
    CWClientStatsNode *node = NULL;
    int radioIdx, wlanIdx;

    for(radioIdx = 0; radioIdx < CWWTPBoardGetMaxRadio(); radioIdx++)
    {
        for(wlanIdx = 0; wlanIdx < CWWTPBoardGetMaxRadioWlans(radioIdx); wlanIdx++)
        {
            node = gClientStatsList[radioIdx][wlanIdx].list;
            while(node != NULL)
            {
                if((node->pollSeq == gPollingSeq - 1) ||
                   (node->pollSeq == STATS_MAX_POLLING_SEQ && gPollingSeq == 1))
                {
                    // Marked as offline
                    node->changeState = WTP_CLIENT_OFFLINE;
                    gStateChangeCount++;
                }
                node = node->next;
            }
        }
    }
}

/*update node
* return :0 --not found entry
   	     :1---update entry

*/
static CWBool UpdateStatsNode(int radioIdx, int wlanIdx, CWStation *station)
{
    CWClientStatsNode *node;

    node = gClientStatsList[radioIdx][wlanIdx].list;
    while(node != NULL)
    {
        if(!memcmp(node->mac, station->mac, 6))
        {
            /* check this client's polling sequence number is continuious */
            if((gPollingSeq != node->pollSeq + 1) &&
               (gPollingSeq != 1 && node->pollSeq != STATS_MAX_POLLING_SEQ))
            {
                /* Sequence number is not continuious,
                   this client was disconnect before, reset last tx/rx */
                node->rxLast = 0;
                node->txLast = 0;

                CWDebugLog("Update Client Stats: radio=%d wlan=%d mac=%x:%x:%x:%x:%x:%x " \
                           "osType=%s hostname=%s ip=%u.%u.%u.%u disconnected before",
                           radioIdx,
                           wlanIdx,
                           CW_MAC_PRINT_LIST(node->mac),
                           node->osType,
                           node->hostName,
                           CW_IPV4_PRINT_LIST(node->address));

                // Marked as go-online
                if(gWTPClientStateChangeEventEnable)
                {
                    node->changeState = WTP_CLIENT_ONLINE;
                    gStateChangeCount++;
                }
            }

            node->pollSeq = gPollingSeq;
            node->lastTime = CWGetUptime();

            /* check if rx overflow */
            if(station->rxKB < node->rxLast)
            {
                node->rxTotal += station->rxKB;
            }
            else
            {
                node->rxTotal += station->rxKB - (node->rxLast);
            }

            /* check if tx overflow */
            if(station->txKB < node->txLast)
            {
                node->txTotal += station->txKB;
            }
            else
            {
                node->txTotal += station->txKB - (node->txLast);
            }
            node->rxLast = station->rxKB;
            node->txLast = station->txKB;

            CW_FREE_OBJECT(node->osType);
            node->osTypeLen = station->osTypeLen;
            if(station->osTypeLen)
            {
                node->osType = station->osType; // deliver the memory
                station->osType = NULL;
            }

            CW_FREE_OBJECT(node->hostName);
            node->hostNameLen = station->hostNameLen;
            if(station->hostNameLen)
            {
                node->hostName = station->hostName; // deliver the memory
                station->hostName = NULL;
            }

            node->address = station->address;

            CWDebugLog("Update Client Stats: radio=%d wlan=%d mac=%x:%x:%x:%x:%x:%x rxTotal=%d txTotal=%d " \
                       "osType=%s hostname=%s ip=%u.%u.%u.%u",
                       radioIdx,
                       wlanIdx,
                       CW_MAC_PRINT_LIST(node->mac),
                       node->rxTotal,
                       node->txTotal,
                       node->osType,
                       node->hostName,
                       CW_IPV4_PRINT_LIST(node->address));

            return CW_TRUE;
        }

        node = node->next;
    }

    return CW_FALSE;
}

static CWBool AddStatsNode(int radioIdx, int wlanIdx, CWStation *station)
{
    CWClientStatsNode *node;

    node = gClientStatsList[radioIdx][wlanIdx].list;

    CW_CREATE_OBJECT_ERR(node, CWClientStatsNode, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    node->rxLast = station->rxKB;
    node->txLast = station->txKB;
    node->txTotal = 0;
    node->rxTotal = 0;
    node->startTime = CWGetUptime();
    node->lastTime = node->startTime;
    node->pollSeq = gPollingSeq;
    CW_COPY_MEMORY(node->mac, station->mac, 6);

    node->osTypeLen = station->osTypeLen;
    node->osType = station->osType; // deliver the memory
    station->osType = NULL;

    node->hostNameLen = station->hostNameLen;
    node->hostName = station->hostName; // deliver the memory
    station->hostName = NULL;

    if(gWTPClientStateChangeEventEnable && gPollingSeq != 0)
    {
        node->changeState = WTP_CLIENT_ONLINE;
        gStateChangeCount++;
    }

    node->address = station->address;
    CWDebugLog("Add Client Stats: radio=%d wlan=%d mac=%x:%x:%x:%x:%x:%x rxLast=%u txLast=%u " \
               "osType=%s hostname=%s ip=%u.%u.%u.%u",
               radioIdx,
               wlanIdx,
               CW_MAC_PRINT_LIST(node->mac),
               node->rxLast,
               node->txLast,
               node->osType,
               node->hostName,
               CW_IPV4_PRINT_LIST(node->address));

    /* Add to list head */
    if(gClientStatsList[radioIdx][wlanIdx].list == NULL)
    {
        node->next = NULL;
    }
    else
    {
        node->next = gClientStatsList[radioIdx][wlanIdx].list;
    }
    gClientStatsList[radioIdx][wlanIdx].list = node;

    return CW_TRUE;
}

static void ClearStatsNode()
{
    CWClientStatsNode *tmp, *node;
    int radioIdx, wlanIdx;

    for(radioIdx = 0; radioIdx < CWWTPBoardGetMaxRadio(); radioIdx++)
    {
        for(wlanIdx = 0; wlanIdx < CWWTPBoardGetMaxRadioWlans(radioIdx); wlanIdx++)
        {
            node = gClientStatsList[radioIdx][wlanIdx].list;
            while(node)
            {
                tmp = node;
                node = node->next;
                CW_FREE_OBJECT(tmp->hostName);
                CW_FREE_OBJECT(tmp->osType);
                CW_FREE_OBJECT(tmp);
            }
            gClientStatsList[radioIdx][wlanIdx].list = NULL;
        }
    }
}

static CWBool SendClientEvent(CWBool sendClientChg, CWBool sendClientStats)
{
    CWProtocolMessage *msgElems;
    int ElemNum = 0;
    int k = -1, i;

    if(sendClientChg)
    {
        ElemNum++;
    }

    if(sendClientStats)
    {
        ElemNum++;
    }

    if(!ElemNum)
    {
        /* nothing to be sent */
        return CW_TRUE;
    }

    CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems,
                                     ElemNum,
                                     return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if((sendClientChg && !(CWAssembleMsgElemWTPClientChange(&(msgElems[++k])))) ||
       (sendClientStats && !(CWAssembleMsgElemVendorPayloadStationStatistics(&(msgElems[++k]))))
      )
    {
        for(i = 0; i <= k; i++)
        {
            CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);
        }
        CW_FREE_OBJECT(msgElems);
        /* error will be handled by the caller */
        return CW_FALSE;
    }

    if(!CWWTPSendPendingRequest(CW_MSG_TYPE_VALUE_WTP_EVENT_REQUEST, CWGetSeqNum(), msgElems, ElemNum))
    {
        for(i = 0; i < ElemNum; i++)
        {
            CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);
        }
        CW_FREE_OBJECT(msgElems);
        /* error will be handled by the caller */
        return CW_FALSE;
    }

    return CW_TRUE;
}

static CW_THREAD_RETURN_TYPE StatsPollingTask(void *arg)
{
    struct timespec timeout;
    int radioIdx, wlanIdx, staIdx, staCount;
    time_t lastUploadTime, now;
    CWStation *station = NULL;
    CWBool uploadStatsExpired;

    CWDebugLog("StatsPollingTask start");

    CWThreadMutexLock(&gStatsMutex);

    if(!gClientStatsList)
    {
        CW_CREATE_ARRAY_ERR(gClientStatsList, CWWTPBoardGetMaxRadio(), CWClientStatsInfo*,
        {
            return (void *) 1;
        });

        for(radioIdx = 0; radioIdx < CWWTPBoardGetMaxRadio(); radioIdx++)
        {
            CW_CREATE_ZERO_ARRAY_ERR(gClientStatsList[radioIdx], CWWTPBoardGetMaxRadioWlans(radioIdx), CWClientStatsInfo,
            {
                return (void *) 1;
            });
        }
    }

    lastUploadTime = time(NULL);

    do
    {
        ResetStateChangeNode();

        for(radioIdx = 0; radioIdx < CWWTPBoardGetMaxRadio(); radioIdx++)
        {
            for(wlanIdx = 0; wlanIdx < CWWTPBoardGetMaxRadioWlans(radioIdx); wlanIdx++)
            {
                if(CWWTPBoardGetWlanStations(radioIdx, wlanIdx, &staCount, &station) && staCount != 0)
                {
                    for(staIdx = 0; staIdx < staCount; staIdx++)
                    {
                        if(UpdateStatsNode(radioIdx, wlanIdx, &(station[staIdx])) == CW_FALSE)
                        {
                            AddStatsNode(radioIdx, wlanIdx, &(station[staIdx]));
                        }
                        CW_FREE_OBJECT(station[staIdx].osType);
                        CW_FREE_OBJECT(station[staIdx].hostName);
                    }
                    CW_FREE_OBJECT(station);
                }
            }
        }

        CheckOfflineStateNode();

        now = time(NULL);

        uploadStatsExpired = abs(now - lastUploadTime) >= gWTPStatisticsTimer;

        SendClientEvent(gWTPClientStateChangeEventEnable && gStateChangeCount > 0, uploadStatsExpired);

        if(uploadStatsExpired)
        {
            ResetStatsNode();
            lastUploadTime = now;
        }

        /* wait for next polling time */
        timeout.tv_sec = now + gWTPStatisticsPollTimer; /* seconds */
        timeout.tv_nsec = 0; /* nanoseconds */

        CWWaitThreadConditionTimeout(&gStatsCond, &gStatsMutex, &timeout);

        /* increase polling sequence number each time */
        gPollingSeq = gPollingSeq >= STATS_MAX_POLLING_SEQ ? 1 : gPollingSeq + 1;
    }
    while(gRunStatsPolling);

    if(!gHaltStats)
    {
        ClearStatsNode();
    }

    CWThreadMutexUnlock(&gStatsMutex);

    CWDebugLog("StatsPollingTask exit");

    CWErrorFree();

    return (void *) 0;
}

CWBool CWAssembleMsgElemWTPClientChange(CWProtocolMessage *msgPtr)
{
    CWClientStatsNode *node;
    int msgLen = 4; /* 4 for number of client */
    int count = 0;
    int radioIdx, wlanIdx;

    for(radioIdx = 0; radioIdx < CWWTPBoardGetMaxRadio(); radioIdx++)
    {
        for(wlanIdx = 0; wlanIdx < CWWTPBoardGetMaxRadioWlans(radioIdx); wlanIdx++)
        {
            node = gClientStatsList[radioIdx][wlanIdx].list;
            while(node)
            {
                if(node->changeState != WTP_CLIENT_NONE)
                {
                    msgLen += 1 + 1 + 1 + 4 + 6 + 1 + node->hostNameLen;
                    count++;
                }
                node = node->next;
            }
        }
    }

    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, msgLen, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    CWProtocolStore32(msgPtr, count);

    for(radioIdx = 0; radioIdx < CWWTPBoardGetMaxRadio(); radioIdx++)
    {
        for(wlanIdx = 0; wlanIdx < CWWTPBoardGetMaxRadioWlans(radioIdx); wlanIdx++)
        {
            node = gClientStatsList[radioIdx][wlanIdx].list;
            while(node)
            {
                if(node->changeState != WTP_CLIENT_NONE)
                {
                    CWProtocolStore8(msgPtr, radioIdx);
                    CWProtocolStore8(msgPtr, wlanIdx);
                    CWProtocolStore8(msgPtr, node->changeState == WTP_CLIENT_OFFLINE ? 0 : 1);
                    CWProtocolStoreIPv4Address(msgPtr, node->address);
                    CWProtocolStoreRawBytes(msgPtr, (char *) node->mac, 6);
                    CWProtocolStore8(msgPtr, node->hostNameLen);
                    if(node->hostNameLen)
                    {
                        CWProtocolStoreRawBytes(msgPtr, node->hostName, node->hostNameLen);
                    }
                }
                node = node->next;
            }
        }
    }

    return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_WTP_EVENT_CLIENT_CHANGE_CW_TYPE);
}

CWBool CWAssembleMsgElemVendorPayloadStationStatistics(CWProtocolMessage *msgPtr)
{
    int radioIdx, wlanIdx;
    int wlanCount, clientCount, len;
    int interval;
    CWClientStatsNode *node = NULL;
    int totalOsTypeLen = 0, totalHostNameLen = 0;

    wlanCount = 0;
    clientCount = 0;

    for(radioIdx = 0; radioIdx < CWWTPBoardGetMaxRadio(); radioIdx++)
    {
        for(wlanIdx = 0; wlanIdx < CWWTPBoardGetMaxRadioWlans(radioIdx); wlanIdx++)
        {
            gClientStatsList[radioIdx][wlanIdx].uploadCount = 0;
            node = gClientStatsList[radioIdx][wlanIdx].list;
            while(node && clientCount < gMaxStatsUploadCount) // check maximum statistics count is reached
            {
                if(node->rxTotal + node->txTotal > 0) // ignore if the client has no traffic
                {
                    totalOsTypeLen += node->osTypeLen;
                    totalHostNameLen += node->hostNameLen;
                    gClientStatsList[radioIdx][wlanIdx].uploadCount++;
                    clientCount++;
                }
                node = node->next;
            }
            wlanCount++;
        }
    }

    len = 4 + 2 + 2 + (wlanCount * (1 + 1 + 2)) + (clientCount * (6 + 4 + 4 + 4 + 1 + 1 + 4)) + totalOsTypeLen + totalHostNameLen;

    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, len, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    CWProtocolStore32(msgPtr, CW_VENDOR_ID);
    CWProtocolStore16(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_STATION_STATISTIC);
    CWProtocolStore16(msgPtr, (unsigned short) wlanCount);

    for(radioIdx = 0; radioIdx < CWWTPBoardGetMaxRadio(); radioIdx++)
    {
        for(wlanIdx = 0; wlanIdx < CWWTPBoardGetMaxRadioWlans(radioIdx); wlanIdx++)
        {
            CWProtocolStore8(msgPtr, radioIdx);
            CWProtocolStore8(msgPtr, wlanIdx);
            CWProtocolStore16(msgPtr, (unsigned short) gClientStatsList[radioIdx][wlanIdx].uploadCount);

            clientCount = 0;
            node = gClientStatsList[radioIdx][wlanIdx].list;
            while(node && clientCount < gClientStatsList[radioIdx][wlanIdx].uploadCount)
            {
                if(node->rxTotal + node->txTotal > 0) // ignore if the client has no traffic
                {
                    CWDebugLog("Sent Client Stats: radioIdx=%d wlanIdx=%d mac=%02X:%02X:%02X:%02X:%02X:%02X " \
                               "rx=%d tx=%d osType=%s hostname=%s ip=%u.%u.%u.%u",
                               radioIdx,
                               wlanIdx,
                               CW_MAC_PRINT_LIST(node->mac),
                               node->rxTotal,
                               node->txTotal,
                               node->osType,
                               node->hostName,
                               CW_IPV4_PRINT_LIST(node->address));

                    CWProtocolStoreRawBytes(msgPtr, (char *) node->mac, 6);
                    CWProtocolStore32(msgPtr, node->txTotal);
                    CWProtocolStore32(msgPtr, node->rxTotal);
                    interval = node->lastTime - node->startTime;
                    if(interval <= 0)
                    {
                        interval = STATS_POLLING_INTERVAL;
                    }
                    CWProtocolStore32(msgPtr, interval);
                    CWProtocolStore8(msgPtr, node->osTypeLen);
                    if(node->osTypeLen)
                    {
                        CWProtocolStoreRawBytes(msgPtr, node->osType, node->osTypeLen);
                    }
                    CWProtocolStore8(msgPtr, node->hostNameLen);
                    if(node->hostNameLen)
                    {
                        CWProtocolStoreRawBytes(msgPtr, node->hostName, node->hostNameLen);
                    }
                    CWProtocolStoreIPv4Address(msgPtr, node->address);

                    clientCount++;
                }
                node = node->next;
            }
        }
    }

    return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE);
}


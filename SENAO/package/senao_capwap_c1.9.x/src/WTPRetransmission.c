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

#ifdef DMALLOC
#include "../dmalloc-5.5.0/dmalloc.h"
#endif

typedef struct tx_node_s
{
    int msgType;
    int seqNum;
    int transmitCount;
    int maxTransCount;
    int interval;
    CWProtocolMessage *msgElems;
    int msgElemNum;
    int timer;
    int txTime;
    int fragmentsNum;
    int encrypt;
    struct tx_node_s *next;
} CWTransmitNode;

static CWTransmitNode *gTxList = NULL;
static CWThreadMutex gTxListMutex = CW_MUTEX_INITIALIZER;
static CWBool gHalt = CW_TRUE;

static CWBool CWWTPSendRequest(CWTransmitNode *txNode)
{
    CWProtocolMessage *messages = NULL, *msgElems = NULL;
    int fragmentsNum = 0, i, j;

    CWDebugLog("CWWTPSendRequest for msgType %d seqNum %d", txNode->msgType, txNode->seqNum);

    if(txNode->msgElemNum)
    {
        CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems,
                                         txNode->msgElemNum,
                                         return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

        for(i = 0; i < txNode->msgElemNum; i++)
        {
            CW_CREATE_OBJECT_SIZE_ERR(msgElems[i].msg, txNode->msgElems[i].offset,
            {
                for(j = 0; j < i; j++)
                {
                    CW_FREE_PROTOCOL_MESSAGE(msgElems[j]);
                }
                CW_FREE_OBJECT(msgElems);
                return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
            });
            msgElems[i].offset = txNode->msgElems[i].offset;
            CW_COPY_MEMORY(msgElems[i].msg, txNode->msgElems[i].msg, msgElems[i].offset);
        }
    }

    if(!(CWAssembleMessage(&messages,
                           &fragmentsNum,
                           gWTPPathMTU,
                           txNode->seqNum,
                           txNode->msgType,
                           msgElems,
                           txNode->msgElemNum,
                           NULL,
                           0,
                           txNode->encrypt
                          )))
    {
        return CW_FALSE;
    }

    txNode->txTime = time(NULL);
    txNode->fragmentsNum = fragmentsNum;

    return CWWTPSendMsg(messages, fragmentsNum, txNode->encrypt, CW_TRUE);
}

static void CWWTPRetransmitTimerExpiredHandler(void *arg)
{
    CWTransmitNode *txThisNode = (CWTransmitNode *) arg;
    CWTransmitNode *txNode, *txNodePre;

    CWThreadMutexLock(&gTxListMutex);

    txNode = gTxList;
    txNodePre = NULL;
    while(txNode)
    {
        if(txNode == txThisNode)
        {
            break;
        }
        txNodePre = txNode;
        txNode = txNode->next;
    }

    if(txNode)
    {
        if(txNode->transmitCount >= txNode->maxTransCount)
        {
            CWLog("[%02X:%02X:%02X:%02X:%02X:%02X] Reach the maximum transmit count %d for msgType %d seqNum %d interval %d fragmentsNum %d",
                  CW_MAC_PRINT_LIST(gWTPIfaceMac), txNode->transmitCount, txNode->msgType, txNode->seqNum, txNode->interval, txNode->fragmentsNum);

            /* remove this tx node from this */
            if(txNodePre)
            {
                txNodePre->next = txNode->next;
            }
            else
            {
                gTxList = txNode->next;
            }

            CWFreeMessageFragments(txNode->msgElems, txNode->msgElemNum);
            CW_FREE_OBJECT(txNode->msgElems);
            CW_FREE_OBJECT(txNode);
        }
        else if(gHalt)
        {
            CWDebugLog("Retransmission for msgType %d seqNum %d is halted now",
                       txNode->msgType, txNode->seqNum);
            txNode->timer = -1;
        }
        else
        {
            CWDebugLog("[%02X:%02X:%02X:%02X:%02X:%02X] Retransmission for msgType %d seqNum %d transmitCount %d",
                       CW_MAC_PRINT_LIST(gWTPIfaceMac), txNode->msgType, txNode->seqNum, txNode->transmitCount);

            CWWTPSendRequest(txNode);
            txNode->transmitCount++;
            txNode->timer = timer_add(txNode->interval,
                                      0, CWWTPRetransmitTimerExpiredHandler, txNode);
        }
    }

    CWThreadMutexUnlock(&gTxListMutex);
}

CWBool CWWTPSendPendingRequestEx(unsigned char msgType, int seqNum, CWProtocolMessage *msgElems,
                                 int msgElemNum, int transCount, int interval, CWBool encrypt)
{
    CWTransmitNode *txNode;

    CW_CREATE_OBJECT_ERR(txNode, CWTransmitNode, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    CWThreadMutexLock(&gTxListMutex);

    txNode->msgType = msgType;
    txNode->seqNum = seqNum;
    txNode->msgElems = msgElems;
    txNode->msgElemNum = msgElemNum;
    txNode->maxTransCount = transCount;
    txNode->interval = interval;
    txNode->encrypt = encrypt;
    if(!gHalt)
    {
        CWWTPSendRequest(txNode);
        txNode->transmitCount = 1;
        txNode->timer = timer_add(interval, 0, CWWTPRetransmitTimerExpiredHandler, txNode);
    }
    else
    {
        CWDebugLog("Transmission for msgType %d seqNum %d is halted now",
                   txNode->msgType, txNode->seqNum);
        txNode->transmitCount = 0;
        txNode->timer = -1;
    }

    txNode->next = gTxList;
    gTxList = txNode;

    CWThreadMutexUnlock(&gTxListMutex);

    return CW_TRUE;
}

CWBool CWWTPRemovePendingRequestByResponse(unsigned char msgType, int seqNum)
{
    CWTransmitNode *txNode, *txNodePre;
    CWBool ret = CW_FALSE;

    CWThreadMutexLock(&gTxListMutex);

    txNode = gTxList;
    txNodePre = NULL;
    while(txNode)
    {
        /* response msg type should equal to request msg type + 1 */
        if(txNode->msgType == msgType - 1 && txNode->seqNum == seqNum)
        {
            break;
        }
        txNodePre = txNode;
        txNode = txNode->next;
    }

    if(txNode)
    {
#ifdef CW_BUILD_X86
        CWLog
#else
        CWDebugLog
#endif
        ("[%02X:%02X:%02X:%02X:%02X:%02X] Receive a response of msgType %d seqNum %d respTime %d" ,
         CW_MAC_PRINT_LIST(gWTPIfaceMac), txNode->msgType, txNode->seqNum, time(NULL) - txNode->txTime);

        /* remove this tx node from list */
        if(txNodePre)
        {
            txNodePre->next = txNode->next;
        }
        else
        {
            gTxList = txNode->next;
        }

        timer_rem(txNode->timer, NULL);
        CWFreeMessageFragments(txNode->msgElems, txNode->msgElemNum);
        CW_FREE_OBJECT(txNode->msgElems);
        CW_FREE_OBJECT(txNode);

        ret = CW_TRUE;
    }
    else
    {
        CWLog("Pending request of msgType %d seqNum %d is not found", msgType - 1, seqNum);
    }

    CWThreadMutexUnlock(&gTxListMutex);

    return ret;
}

void CWWTPRemoveAllPendingRequest()
{
    CWTransmitNode *txNode, *txNodeTmp;

    CWDebugLog("CWWTPRemoveAllPendingRequest called");

    CWThreadMutexLock(&gTxListMutex);

    txNode = gTxList;
    while(txNode)
    {
        txNodeTmp = txNode;
        txNode = txNode->next;
        timer_rem(txNodeTmp->timer, NULL);
        CWFreeMessageFragments(txNodeTmp->msgElems, txNodeTmp->msgElemNum);
        CW_FREE_OBJECT(txNodeTmp->msgElems);
        CW_FREE_OBJECT(txNodeTmp);
    }
    gTxList = NULL;

    CWThreadMutexUnlock(&gTxListMutex);
}

void CWWTPHaltPendingRequest(CWBool enable)
{
    CWTransmitNode *txNode;

    CWThreadMutexLock(&gTxListMutex);

    if(gHalt != enable)
    {
        gHalt = enable;

        if(!enable)
        {
            txNode = gTxList;
            while(txNode)
            {
                if(txNode->timer == -1)
                {
                    if(txNode->transmitCount < txNode->maxTransCount)
                    {
                        CWWTPSendRequest(txNode);
                        txNode->transmitCount++;
                    }
                    txNode->timer = timer_add(txNode->interval, 0,
                                              CWWTPRetransmitTimerExpiredHandler,
                                              txNode);
                }
                txNode = txNode->next;
            }
        }
    }

    CWThreadMutexUnlock(&gTxListMutex);
}


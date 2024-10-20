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

CWBool CWAssembleChangeStateEventRequest(CWProtocolMessage **messagesPtr,
        int *fragmentsNumPtr,
        int PMTU,
        int seqNum,
        void *valuesPtr);

CWBool CWParseChangeStateEventResponseMessage(char *msg,
        int len,
        int seqNum,
        void *values,
        int combineLen);

CWBool CWSaveChangeStateEventResponseMessage(void *changeStateEventResp);

CWStateTransition CWWTPEnterDataCheck()
{
    int seqNum;

    CWDebugLog("######### Data Check State #########");

    /* Send Change State Event Request */
    seqNum = CWGetSeqNum();

    if(!CWErr(CWWTPSendAcknowledgedPacket(seqNum,
                                          NULL,
                                          CWAssembleChangeStateEventRequest,
                                          CWParseChangeStateEventResponseMessage,
                                          CWSaveChangeStateEventResponseMessage,
                                          NULL,
                                          gACInfoPtr->timer->changeState)))
    {
        CWWtpCfgMsgListFree(&gWTPCfgRollbackList);
        return CW_ENTER_DISCOVERY;
    }

    CWWtpCfgMsgListFree(&gWTPCfgRollbackList);

    if(gWTPResultCode == CW_PROTOCOL_SUCCESS)
    {
        if(gWTPCfgResult.apply)
        {
            CWDebugLog("Applying configuration...");

            if(!gWTPCfgResult.rejoin)
            {
                CWWTPStartKeepAliveTask();
            }

            CWWTPBoardApplyCfg();

            if(gWTPCfgResult.rejoin)
            {
                gWTPRejoinAc = CW_TRUE;
                gWTPFastJoin = CW_TRUE;
                return CW_ENTER_DISCOVERY;
            }
            else
            {
                CWWTPStopKeepAliveTask();
            }
        }
    }
    else
    {
        switch(gWTPCfgResult.handle)
        {
            case WTP_CFG_ERROR_HANDLE_RESYNC:
                return CW_ENTER_CONFIGURE;
            case WTP_CFG_ERROR_HANDLE_UNMANAGED:
                gWTPRejoinAc = CW_FALSE;
                return CW_ENTER_DISCOVERY;
			default:
				return CW_ENTER_RUN;
        }
    }

    return CW_ENTER_RUN;
}

CWBool CWAssembleChangeStateEventRequest(CWProtocolMessage **messagesPtr,
        int *fragmentsNumPtr,
        int PMTU,
        int seqNum,
        void* valuesPtr)
{
    CWProtocolMessage 	*msgElems = NULL;
    CWProtocolMessage 	*msgElemsBinding = NULL;
#ifdef CW_WTP_AP
    const int		msgElemCount = gWTPCfgRollbackList.head ? 4 : 3;
#else
    const int		msgElemCount = gWTPCfgRollbackList.head ? 3 : 2;
#endif
    int 			msgElemBindingCount = 0;
    //int 			resultCode = CW_PROTOCOL_SUCCESS;
    int 			k = -1;

    if(messagesPtr == NULL || fragmentsNumPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems,
                                     msgElemCount,
                                     return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    CWDebugLog("Assembling Change State Event Request...");

    /* Assemble Message Elements */
#ifdef CW_WTP_AP
    if(!(CWAssembleMsgElemRadioOperationalState(-1, &(msgElems[++k]))) ||
#else
    if(
#endif
            !(CWAssembleMsgElemResultCode(&(msgElems[++k]), gWTPResultCode)) ||
            !(CWAssembleMsgElemVendorPayloadWtpCfgResult(&(msgElems[++k]), &gWTPCfgResult)) ||
            (gWTPCfgRollbackList.head && !(CWAssembleMsgElemVendorPayloadWtpCfg(&(msgElems[++k]), &gWTPCfgRollbackList)))
      )
    {
        int i;

        for(i = 0; i <= k; i++)
        {
            CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);
        }
        CW_FREE_OBJECT(msgElems);
        /* error will be handled by the caller */
        return CW_FALSE;
    }

    if(!(CWAssembleMessage(messagesPtr,
                           fragmentsNumPtr,
                           PMTU,
                           seqNum,
                           CW_MSG_TYPE_VALUE_CHANGE_STATE_EVENT_REQUEST,
                           msgElems, msgElemCount,
                           msgElemsBinding,
                           msgElemBindingCount,
                           CW_PACKET_CRYPT
                          )))
    {
        return CW_FALSE;
    }

    CWDebugLog("Change State Event Request Assembled");
    return CW_TRUE;
}

CWBool CWParseChangeStateEventResponseMessage(char *msg,
        int len,
        int seqNum,
        void *values,
        int combineLen)
{
    CWControlHeaderValues controlVal;
    CWProtocolMessage completeMsg;

    if(msg == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    CWDebugLog("Parsing Change State Event Response...");

    completeMsg.msg = msg;
    completeMsg.offset = 0;

    /* error will be handled by the caller */
    if(!(CWParseControlHeader(&completeMsg, &controlVal)))
    {
        return CW_FALSE;
    }

    if(controlVal.messageTypeValue != CW_MSG_TYPE_VALUE_CHANGE_STATE_EVENT_RESPONSE)
        return CWErrorRaise(CW_ERROR_INVALID_FORMAT,
                            "Message is not Change State Event Response as Expected");

    if(controlVal.seqNum != seqNum)
    {
        return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Different Sequence Number");
    }

    /* skip timestamp */
    controlVal.msgElemsLen -= CW_CONTROL_HEADER_OFFSET_FOR_MSG_ELEMS;

    if(combineLen !=0)
    {
       CWDebugLog("%s change packet length from %d to %d",__FUNCTION__,controlVal.msgElemsLen,combineLen);
       controlVal.msgElemsLen = combineLen;
    }

    if(controlVal.msgElemsLen != 0)
        return CWErrorRaise(CW_ERROR_INVALID_FORMAT,
                            "Change State Event Response must carry no message elements");

    CWDebugLog("Change State Event Response Parsed");
    return CW_TRUE;
}

CWBool CWSaveChangeStateEventResponseMessage(void *changeStateEventResp)
{
    CWDebugLog("Saving Change State Event Response...");
    CWDebugLog("Change State Event Response Saved");
    return CW_TRUE;
}

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

#ifdef DMALLOC
#include "../dmalloc-5.5.0/dmalloc.h"
#endif

int gCWWaitJoin = CW_WAIT_JOIN_DEFAULT;
extern int gWTPInitMTU;

/*__________________________________________________________*/
/*  *******************___PROTOTYPES___*******************  */
CWBool CWAssembleJoinRequest(CWProtocolMessage **messagesPtr,
                             int *fragmentsNumPtr,
                             int PMTU,
                             int seqNum,
                             void* valuesPtr);

CWBool CWParseJoinResponseMessage(char *msg,
                                  int len,
                                  int seqNum,
                                  CWProtocolJoinResponseValues *valuesPtr,int combineLen);

CWBool CWSaveJoinResponseMessage(CWProtocolJoinResponseValues *joinResponse);

/*_____________________________________________________*/
/*  *******************___FUNCTIONS___*******************  */

/*
 * Manage Join State.
 */
CWStateTransition CWWTPEnterJoin()
{
    int seqNum;
    CWProtocolJoinResponseValues values;
    CWAcAddress acAddr;

    CWDebugLog("######### Join State #########");

    /* reset Join state */
    CWNetworkCloseSocket(gWTPSocket);
    CWSecurityDestroySession(gWTPSession);
    CWSecurityDestroyContext(gWTPSecurityContext);
    gWTPSecurityContext = NULL;
    gWTPSession = NULL;
    gWTPLastReqSeqNum = -1;

    if(!gACInfoPtr)
    {
        CWLog("No AC Info in join state!!");
        return CW_ENTER_DISCOVERY;
    }

    /* Initialize gACInfoPtr */
    gACInfoPtr->ACIPv4ListInfo.ACIPv4ListCount = 0;
    gACInfoPtr->ACIPv4ListInfo.ACIPv4List = NULL;
    gACInfoPtr->ACIPv6ListInfo.ACIPv6ListCount = 0;
    gACInfoPtr->ACIPv6ListInfo.ACIPv6List = NULL;

    /* Init DTLS session */
    if(!CWErr(CWNetworkInitSocketClient(&gWTPSocket,
                                        &(gACInfoPtr->preferredAddress),
                                        gWTPIface,
                                        gWTPSourcePort)))
    {
        return CW_ENTER_DISCOVERY;
    }

    /*Sigma : Set cert path*/
    CWSetCertDir(gWTPCertDir);

    if(gACInfoPtr->security == CW_X509_CERTIFICATE)
    {
        if(gWTPTryDefaultCert)
        {
            if(!CWErr(CWSecurityInitContext(&gWTPSecurityContext,
                                            CW_WTP_CA_NAME_DEFAULT,
                                            CW_WTP_CERT_NAME_DEFAULT,
                                            CW_WTP_PRIVATE_KEY_DEFAULT,
                                            CW_WTP_PKEY_PASSWORD_DEFAULT,
                                            CW_TRUE,
                                            NULL)))
            {
                return CW_ENTER_DISCOVERY;
            }
        }
        else
        {
            CWWTPCheckCurrentCertAvailable();

            if(!CWErr(CWSecurityInitContext(&gWTPSecurityContext,
                                            CW_WTP_CA_NAME_CURRENT,
                                            CW_WTP_CERT_NAME_CURRENT,
                                            CW_WTP_PRIVATE_KEY_CURRENT,
                                            CW_WTP_PKEY_PASSWORD_CURRENT,
                                            CW_TRUE,
                                            NULL)))
            {
                return CW_ENTER_DISCOVERY;
            }
        }
    }
    else
    {
        /* pre-shared keys */
        if(!CWErr(CWSecurityInitContext(&gWTPSecurityContext,
                                        NULL,
                                        NULL,
                                        NULL,
                                        NULL,
                                        CW_TRUE,
                                        NULL)))
        {
            return CW_ENTER_DISCOVERY;
        }
    }

    if(!CWWTPStartRxTask())
    {
        return CW_ENTER_DISCOVERY;
    }

    /* if the mtu provided by AC is not zero, apply the mtu to establish the connection */
    if(gACInfoPtr->mtu)
    {
        gWTPPathMTU = gACInfoPtr->mtu;
    }
    else
    {
        gWTPPathMTU = gWTPInitMTU;
    }

    /* Note: gWTPPathMTU will be updated in this function */
    if(!CWErr(CWSecurityInitSessionClient(gWTPSocket,
                                          &(gACInfoPtr->preferredAddress),
                                          gWTPSecurityContext,
                                          &gWTPSession,
                                          &gWTPPathMTU,
                                          CWWTPSecurityRxCB,
                                          NULL,
                                          gACInfoPtr->controllerId)))
    {
        /*20140124 sigma: if current cert is not default cert and DTLS handshake didn't time out, reset the session and try again*/
        if(!gWTPDtlsHandshakeTimeout)
        {
            if(!gWTPTryDefaultCert)
            {
                CWLog("Handshake with current Certificate failed, try to use default Certificate");
                gWTPTryDefaultCert = CW_TRUE;
                gWTPFastJoin = CW_TRUE;
            }
            else
            {
                CWLog("Handshake with default Certificate failed ?");
                gWTPTryDefaultCert = CW_FALSE;
                gWTPRejoinAc = CW_FALSE;
            }
        }

        return CW_ENTER_DISCOVERY;
    }

    if(gCWForceMTU)
    {
        gWTPPathMTU = gCWForceMTU;
    }

    CWDebugLog("Path MTU for this Session: %d", gWTPPathMTU);

    /* send Join Request */
    seqNum = CWGetSeqNum();

    if(!CWErr(CWWTPSendAcknowledgedPacket(seqNum,
                                          NULL,
                                          CWAssembleJoinRequest,
                                          (void *)CWParseJoinResponseMessage,
                                          (void *)CWSaveJoinResponseMessage,
                                          &values,
                                          gACInfoPtr->timer->joinState)))
    {
        return CW_ENTER_DISCOVERY;
    }

    CWDebugLog("Join Completed");

    /* Turn on AC mode */
    CWWTPBoardSetAcMode(CW_TRUE);

    /* Save this AC address */
    CWSetAcAddr(CW_ADDR_GET_IP(&gACInfoPtr->incomingAddress),
                CW_ADDR_GET_PORT(&gACInfoPtr->incomingAddress),
                gACInfoPtr->controllerId,
                &acAddr);
    CWWTPBoardSetAcAddress(&acAddr);

    /* Check if AC request to upgrade image */
    if(gWTPImageId.imageName[0] != '\0')
    {
        CWDebugLog("Upgrade Image %s", gWTPImageId.imageName);
        return CW_ENTER_IMAGE;
    }

    /* 20131008 sigma: DTLS session established, enter cert reset state */
    return CW_ENTER_CERT_CHECK;
}

CWBool CWAssembleJoinRequest(CWProtocolMessage **messagesPtr,
                             int *fragmentsNumPtr,
                             int PMTU,
                             int seqNum,
                             void* valuesPtr)
{

    CWProtocolMessage *msgElems = NULL;
#ifdef CW_WTP_AP
    const int msgElemCount = 10;
#else
    const int msgElemCount = 9;
#endif
    CWProtocolMessage *msgElemsBinding = NULL;
    const int msgElemBindingCount = 0;
    int k = -1, i;

    if(messagesPtr == NULL || fragmentsNumPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems,
                                     msgElemCount,
                                     return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    CWDebugLog("Sending Join Request...");

    /* Assemble Message Elements */
    if((!(CWAssembleMsgElemLocationData(&(msgElems[++k])))) ||
            (!(CWAssembleMsgElemWTPBoardData(&(msgElems[++k])))) ||
            (!(CWAssembleMsgElemWTPDescriptor(&(msgElems[++k])))) ||
            (!(CWAssembleMsgElemWTPIPv4Address(&(msgElems[++k])))) ||
            (!(CWAssembleMsgElemWTPName(&(msgElems[++k])))) ||
            (!(CWAssembleMsgElemSessionID(&(msgElems[++k]), CWWTPGetSessionID()))) ||
            (!(CWAssembleMsgElemWTPFrameTunnelMode(&(msgElems[++k])))) ||
            (!(CWAssembleMsgElemWTPMACType(&(msgElems[++k])))) ||
#ifdef CW_WTP_AP
            (!(CWAssembleMsgElemWTPRadioInformation(&(msgElems[++k])))) ||
#endif
            (!(CWAssembleMsgElemVendorPayloadSystemUpTime(&(msgElems[++k]), CWWTPBoardGetSystemUpTime())))
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

    return CWAssembleMessage(messagesPtr,
                             fragmentsNumPtr,
                             PMTU,
                             seqNum,
                             CW_MSG_TYPE_VALUE_JOIN_REQUEST,
                             msgElems,
                             msgElemCount,
                             msgElemsBinding,
                             msgElemBindingCount,
                             CW_PACKET_CRYPT
                            );
}


/*
 * Parse Join Response and return informations in *valuesPtr.
 */
CWBool CWParseJoinResponseMessage(char *msg,
                                  int len,
                                  int seqNum,
                                  CWProtocolJoinResponseValues *valuesPtr,
                                  int combineLen)
{

    CWControlHeaderValues controlVal;
    CWProtocolMessage completeMsg;
    int offsetTillMessages;

    if(msg == NULL || valuesPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    CWDebugLog("Parsing Join Response...");

    completeMsg.msg = msg;
    completeMsg.offset = 0;

    /* error will be handled by the caller */
    if(!(CWParseControlHeader(&completeMsg, &controlVal)))
    {
        return CW_FALSE;
    }

    if(controlVal.messageTypeValue != CW_MSG_TYPE_VALUE_JOIN_RESPONSE)
        return CWErrorRaise(CW_ERROR_INVALID_FORMAT,
                            "Message is not Join Response as Expected");

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

    /* Mauro */
    CW_ZERO_MEMORY(valuesPtr, sizeof(CWProtocolJoinResponseValues));

    /* parse message elements */
    while((completeMsg.offset - offsetTillMessages) < controlVal.msgElemsLen)
    {
        unsigned short int type = 0;
        unsigned int len = 0;

        CWParseFormatMsgElem(&completeMsg, &type, &len);

        CWDebugLog("Parsing Message Element: %u, len: %u", type, len);
        /*
        valuesPtr->ACInfoPtr.IPv4AddressesCount = 0;
        valuesPtr->ACInfoPtr.IPv6AddressesCount = 0;
        valuesPtr->ACIPv4ListInfo.ACIPv4ListCount = 0;
        valuesPtr->ACIPv4ListInfo.ACIPv4List = NULL;
        valuesPtr->ACIPv6ListInfo.ACIPv6ListCount = 0;
        valuesPtr->ACIPv6ListInfo.ACIPv6List = NULL;
        */
        switch(type)
        {
            case CW_MSG_ELEMENT_AC_DESCRIPTOR_CW_TYPE:
                /* will be handled by the caller */
                if(!(CWParseACDescriptor(&completeMsg, len, &(valuesPtr->ACInfoPtr))))
                {
                    return CW_FALSE;
                }
                break;
            case CW_MSG_ELEMENT_AC_IPV4_LIST_CW_TYPE:
                if(!(CWParseACIPv4List(&completeMsg, len, &(valuesPtr->ACIPv4ListInfo))))
                {
                    return CW_FALSE;
                }
                break;
            case CW_MSG_ELEMENT_AC_IPV6_LIST_CW_TYPE:
                if(!(CWParseACIPv6List(&completeMsg, len, &(valuesPtr->ACIPv6ListInfo))))
                {
                    return CW_FALSE;
                }
                break;
            case CW_MSG_ELEMENT_RESULT_CODE_CW_TYPE:
                if(!(CWParseResultCode(&completeMsg, len, &(valuesPtr->code))))
                {
                    return CW_FALSE;
                }
                break;
            case CW_MSG_ELEMENT_AC_NAME_CW_TYPE:
                /* will be handled by the caller */
                if(!(CWParseACName(&completeMsg, len, &(valuesPtr->ACInfoPtr.name))))
                {
                    return CW_FALSE;
                }
                break;
            case CW_MSG_ELEMENT_CW_CONTROL_IPV4_ADDRESS_CW_TYPE:
                /*
                 * just count how many interfacess we
                 * have, so we can allocate the array
                 */
                valuesPtr->ACInfoPtr.IPv4AddressesCount++;
                completeMsg.offset += len;
                break;
            case CW_MSG_ELEMENT_CW_CONTROL_IPV6_ADDRESS_CW_TYPE:
                /*
                 * just count how many interfacess we
                 * have, so we can allocate the array
                 */
                valuesPtr->ACInfoPtr.IPv6AddressesCount++;
                completeMsg.offset += len;
                break;
                /*
                case CW_MSG_ELEMENT_SESSION_ID_CW_TYPE:
                	if(!(CWParseSessionID(&completeMsg, len, valuesPtr))) return CW_FALSE;
                	break;
                */
            case CW_MSG_ELEMENT_IMAGE_IDENTIFIER_CW_TYPE:
                if(!(CWParseImageIdentifier(&completeMsg, len, &(valuesPtr->imageId))))
                {
                    return CW_FALSE;
                }
                break;
            case CW_MSG_ELEMENT_CONNECTION_ID:
                if(!(CWParseConnectionID(&completeMsg, len, &(valuesPtr->connId))))
                {
                    return CW_FALSE;
                }
                break;
            default:
                completeMsg.offset += len;
                CWLog("Unrecognized Message Element %d in Join Response", type);
                break;
        }

        /* CWDebugLog("bytes: %d/%d", (completeMsg.offset-offsetTillMessages), controlVal.msgElemsLen); */
    }

    if(completeMsg.offset != len)
        return CWErrorRaise(CW_ERROR_INVALID_FORMAT,
                            "Garbage at the End of the Message");

    /* actually read each interface info */
    CW_CREATE_ARRAY_ERR(valuesPtr->ACInfoPtr.IPv4Addresses,
                        valuesPtr->ACInfoPtr.IPv4AddressesCount,
                        CWProtocolIPv4NetworkInterface,
                        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if(valuesPtr->ACInfoPtr.IPv6AddressesCount > 0)
    {
        CW_CREATE_ARRAY_ERR(valuesPtr->ACInfoPtr.IPv6Addresses,
                            valuesPtr->ACInfoPtr.IPv6AddressesCount,
                            CWProtocolIPv6NetworkInterface,
                            return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
    }

    int i = 0;
    int j = 0;

    completeMsg.offset = offsetTillMessages;
    while((completeMsg.offset - offsetTillMessages) < controlVal.msgElemsLen)
    {
        unsigned short int type = 0;	/* = CWProtocolRetrieve32(&completeMsg); */
        unsigned int len = 0;	/* = CWProtocolRetrieve16(&completeMsg); */

        CWParseFormatMsgElem(&completeMsg, &type, &len);

        switch(type)
        {
            case CW_MSG_ELEMENT_CW_CONTROL_IPV4_ADDRESS_CW_TYPE:
                /* will be handled by the caller */
                if(!(CWParseCWControlIPv4Addresses(&completeMsg,
                                                   len,
                                                   &(valuesPtr->ACInfoPtr.IPv4Addresses[i]))))
                {
                    return CW_FALSE;
                }
                i++;
                break;
            case CW_MSG_ELEMENT_CW_CONTROL_IPV6_ADDRESS_CW_TYPE:
                /* will be handled by the caller */
                if(!(CWParseCWControlIPv6Addresses(&completeMsg,
                                                   len,
                                                   &(valuesPtr->ACInfoPtr.IPv6Addresses[j]))))
                {
                    return CW_FALSE;
                }
                j++;
                break;
            default:
                completeMsg.offset += len;
                break;
        }
    }

    return CW_TRUE;
}

CWBool CWSaveJoinResponseMessage(CWProtocolJoinResponseValues *joinResponse)
{

    if(joinResponse == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    if((joinResponse->code == CW_PROTOCOL_SUCCESS) ||
            (joinResponse->code == CW_PROTOCOL_SUCCESS_NAT))
    {

        if(gACInfoPtr == NULL)
        {
            return CWErrorRaise(CW_ERROR_NEED_RESOURCE, NULL);
        }

        gACInfoPtr->stations = (joinResponse->ACInfoPtr).stations;
        gACInfoPtr->limit = (joinResponse->ACInfoPtr).limit;
        gACInfoPtr->activeWTPs = (joinResponse->ACInfoPtr).activeWTPs;
        gACInfoPtr->maxWTPs = (joinResponse->ACInfoPtr).maxWTPs;
        gACInfoPtr->security = (joinResponse->ACInfoPtr).security;
        gACInfoPtr->RMACField = (joinResponse->ACInfoPtr).RMACField;

        /* BUG-ML07
             * Before overwriting the field vendorInfos we'd better
             * free it (it was allocated during the Discovery State by
             * the function CWParseACDescriptor()).
             *
             * 19/10/2009 - Donato Capitella
             */
        int i;
        for(i = 0; i < gACInfoPtr->vendorInfos.vendorInfosCount; i++)
        {
            CW_FREE_OBJECT(gACInfoPtr->vendorInfos.vendorInfos[i].valuePtr);
        }
        CW_FREE_OBJECT(gACInfoPtr->vendorInfos.vendorInfos);


        gACInfoPtr->vendorInfos = (joinResponse->ACInfoPtr).vendorInfos;

        if(joinResponse->ACIPv4ListInfo.ACIPv4ListCount > 0)
        {
            gACInfoPtr->ACIPv4ListInfo.ACIPv4ListCount = joinResponse->ACIPv4ListInfo.ACIPv4ListCount;
            gACInfoPtr->ACIPv4ListInfo.ACIPv4List = joinResponse->ACIPv4ListInfo.ACIPv4List;
        }

        if(joinResponse->ACIPv6ListInfo.ACIPv6ListCount > 0)
        {
            gACInfoPtr->ACIPv6ListInfo.ACIPv6ListCount = joinResponse->ACIPv6ListInfo.ACIPv6ListCount;
            gACInfoPtr->ACIPv6ListInfo.ACIPv6List = joinResponse->ACIPv6ListInfo.ACIPv6List;
        }

        /*
             * This field name was allocated for storing the AC name; however, it
             * doesn't seem to be used and it is certainly lost when we exit
             * CWWTPEnterJoin() as joinResponse is actually a local variable of that
             * function.
             *
             * Thus, it seems good to free it now.
             *
             * BUG ML03
             * 16/10/2009 - Donato Capitella
             */
        CW_FREE_OBJECT(joinResponse->ACInfoPtr.name);
        /* BUG ML08 */
        CW_FREE_OBJECT(joinResponse->ACInfoPtr.IPv4Addresses);

        if(joinResponse->imageId.imageName[0] != '\0')
        {
            CW_COPY_MEMORY(&gWTPImageId, &(joinResponse->imageId), sizeof(gWTPImageId));
        }
        else
        {
            gWTPImageId.imageName[0] = '\0';
        }

        gWTPConnectionId = joinResponse->connId;

        CWDebugLog("Join Response Saved");
        return CW_TRUE;
    }
    else
    {
        CWLog("Join Response said \"Failure\"");
        return CW_FALSE;
    }
}

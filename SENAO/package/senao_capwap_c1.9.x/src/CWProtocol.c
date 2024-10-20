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


#include "CWCommon.h"
#include "CWVendorPayloads.h"

#ifdef DMALLOC
#include "../dmalloc-5.5.0/dmalloc.h"
#endif

static const int gCWIANATimes256 = CW_IANA_ENTERPRISE_NUMBER * 256;
static const int gMaxDTLSHeaderSize = 25; // see http://crypto.stanford.edu/~nagendra/papers/dtls.pdf
static const int gCapwapHdrSize = 8; // note: this do not include optional Wireless field
static const int gDTLSTailerSize = 8; // note: CBC padding

// stores 8 bits in the message, increments the current offset in bytes
void CWProtocolStore8(CWProtocolMessage *msgPtr, unsigned char val)
{
    CW_COPY_MEMORY(&((msgPtr->msg)[(msgPtr->offset)]), &(val), 1);
    (msgPtr->offset) += 1;
}

// stores 16 bits in the message, increments the current offset in bytes
void CWProtocolStore16(CWProtocolMessage *msgPtr, unsigned short val)
{
    val = htons(val);
    CW_COPY_MEMORY(&((msgPtr->msg)[(msgPtr->offset)]), &(val), 2);
    (msgPtr->offset) += 2;
}

// stores 32 bits in the message, increments the current offset in bytes
void CWProtocolStore32(CWProtocolMessage *msgPtr, unsigned int val)
{
    val = htonl(val);
    CW_COPY_MEMORY(&((msgPtr->msg)[(msgPtr->offset)]), &(val), 4);
    (msgPtr->offset) += 4;
}

void CWProtocolStoreIPv4Address(CWProtocolMessage *msgPtr, unsigned int addr)
{
    CW_COPY_MEMORY(&((msgPtr->msg)[(msgPtr->offset)]), &(addr), 4);
    (msgPtr->offset) += 4;
}

// stores a string in the message, increments the current offset in bytes. Doesn't store
// the '\0' final character.
void CWProtocolStoreStr(CWProtocolMessage *msgPtr, const char *str)
{
    int len = strlen(str);
    CW_COPY_MEMORY(&((msgPtr->msg)[(msgPtr->offset)]), str, len);
    (msgPtr->offset) += len;
}

// stores another message in the message, increments the current offset in bytes.
void CWProtocolStoreMessage(CWProtocolMessage *msgPtr, CWProtocolMessage *msgToStorePtr)
{
    CW_COPY_MEMORY(&((msgPtr->msg)[(msgPtr->offset)]), msgToStorePtr->msg, msgToStorePtr->offset);
    (msgPtr->offset) += msgToStorePtr->offset;
}

// stores len bytes in the message, increments the current offset in bytes.
void CWProtocolStoreRawBytes(CWProtocolMessage *msgPtr, const char *bytes, int len)
{
    CW_COPY_MEMORY(&((msgPtr->msg)[(msgPtr->offset)]), bytes, len);
    (msgPtr->offset) += len;
}

// retrieves 8 bits from the message, increments the current offset in bytes.
unsigned char CWProtocolRetrieve8(CWProtocolMessage *msgPtr)
{
    unsigned char val;

    CW_COPY_MEMORY(&val, &((msgPtr->msg)[(msgPtr->offset)]), 1);
    (msgPtr->offset) += 1;

    return val;
}

// retrieves 16 bits from the message, increments the current offset in bytes.
unsigned short CWProtocolRetrieve16(CWProtocolMessage *msgPtr)
{
    unsigned short val;

    CW_COPY_MEMORY(&val, &((msgPtr->msg)[(msgPtr->offset)]), 2);
    (msgPtr->offset) += 2;

    return ntohs(val);
}

// retrieves 32 bits from the message, increments the current offset in bytes.
unsigned int CWProtocolRetrieve32(CWProtocolMessage *msgPtr)
{
    unsigned int val;

    CW_COPY_MEMORY(&val, &((msgPtr->msg)[(msgPtr->offset)]), 4);
    (msgPtr->offset) += 4;

    return ntohl(val);
}

unsigned int CWProtocolRetrieveIPv4Address(CWProtocolMessage *msgPtr)
{
    unsigned int val;

    CW_COPY_MEMORY(&val, &((msgPtr->msg)[(msgPtr->offset)]), 4);
    (msgPtr->offset) += 4;

    return val;
}

// retrieves a string (not null-terminated) from the message, increments the current offset in bytes.
// Adds the '\0' char at the end of the string which is returned
char *CWProtocolRetrieveStr(CWProtocolMessage *msgPtr, int len)
{
    char *str;

    CW_CREATE_OBJECT_SIZE_ERR(str, (len + 1), return NULL;);

    CW_COPY_MEMORY(str, &((msgPtr->msg)[(msgPtr->offset)]), len);
    str[len] = '\0';
    (msgPtr->offset) += len;

    return str;
}

char *CWProtocolRetrieveStrNoCreate(CWProtocolMessage *msgPtr, char *str, int len)
{
    CW_COPY_MEMORY(str, &((msgPtr->msg)[(msgPtr->offset)]), len);
    str[len] = '\0';
    (msgPtr->offset) += len;

    return str;
}

// retrieves len bytes from the message, increments the current offset in bytes.
char *CWProtocolRetrieveRawBytes(CWProtocolMessage *msgPtr, int len)
{
    char *bytes;

    CW_CREATE_OBJECT_SIZE_ERR(bytes, len, return NULL;);

    CW_COPY_MEMORY(bytes, &((msgPtr->msg)[(msgPtr->offset)]), len);
    (msgPtr->offset) += len;

    return bytes;
}

char *CWProtocolRetrieveRawBytesNoCreate(CWProtocolMessage *msgPtr, char *bytes, int len)
{
    CW_COPY_MEMORY(bytes, &((msgPtr->msg)[(msgPtr->offset)]), len);
    (msgPtr->offset) += len;

    return bytes;
}

char *CWProtocolRetrieveRawBytesPtr(CWProtocolMessage *msgPtr, char **bytes, int len)
{
    *bytes = &(msgPtr->msg[msgPtr->offset]);
    (msgPtr->offset) += len;

    return *bytes;
}

void CWProtocolDestroyMsgElemData(void *f)
{
    CW_FREE_OBJECT(f);
}

// Assemble a Message Element creating the appropriate header and storing the message.
CWBool CWAssembleMsgElem(CWProtocolMessage *msgPtr, unsigned int type)
{
    CWProtocolMessage completeMsg;

    if(msgPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    CW_CREATE_PROTOCOL_MESSAGE(completeMsg, 6 + (msgPtr->offset), return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    // store header
    CWProtocolStore16(&completeMsg, type);

    if((type != CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE_32_LEN) )
    {
       CWProtocolStore16(&completeMsg, msgPtr->offset); // size of the body
    }
    else
    {
       CWDebugLog("Save length in 32 bits length =%u",msgPtr->offset);
       CWProtocolStore32(&completeMsg, msgPtr->offset); // size of the body
    }

    // store body
    CWProtocolStoreMessage(&completeMsg, msgPtr);

    CW_FREE_PROTOCOL_MESSAGE(*msgPtr);

    msgPtr->msg = completeMsg.msg;
    msgPtr->offset = completeMsg.offset;

    return CW_TRUE;
}

// Assembles the Transport Header
CWBool CWAssembleTransportHeader(CWProtocolMessage *transportHdrPtr, CWProtocolTransportHeaderValues *valuesPtr)
{
    unsigned int val = 0;
    int size =8;

    if(transportHdrPtr == NULL || valuesPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }
    if (valuesPtr ->protocolVersion == CW_MULTIC_PACKET )
       size = 16;

    // meaningful bytes of the header (no wirless header and MAC address)
    CW_CREATE_PROTOCOL_MESSAGE(*transportHdrPtr, size,
                               return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    CWSetField32(val,
                 CW_TRANSPORT_HEADER_VERSION_START,
                 CW_TRANSPORT_HEADER_VERSION_LEN,
                 valuesPtr ->protocolVersion); // current version of CAPWAP

    CWSetField32(val,
                 CW_TRANSPORT_HEADER_TYPE_START,
                 CW_TRANSPORT_HEADER_TYPE_LEN,
                 (valuesPtr->payloadType == CW_PACKET_PLAIN) ? 0 : 1);

    CWSetField32(val,
                 CW_TRANSPORT_HEADER_HLEN_START,
                 CW_TRANSPORT_HEADER_HLEN_LEN,
                 2);

    CWSetField32(val,
                 CW_TRANSPORT_HEADER_RID_START,
                 CW_TRANSPORT_HEADER_RID_LEN,
                 0); // only one radio per WTP?

    CWSetField32(val,
                 CW_TRANSPORT_HEADER_WBID_START,
                 CW_TRANSPORT_HEADER_WBID_LEN,
                 1); // Wireless Binding ID

    CWSetField32(val,
                 CW_TRANSPORT_HEADER_T_START,
                 CW_TRANSPORT_HEADER_T_LEN,
                 1);

    CWSetField32(val,
                 CW_TRANSPORT_HEADER_F_START,
                 CW_TRANSPORT_HEADER_F_LEN,
                 valuesPtr->isFragment); // is fragment

    CWSetField32(val,
                 CW_TRANSPORT_HEADER_L_START,
                 CW_TRANSPORT_HEADER_L_LEN,
                 valuesPtr->last); // last fragment

    CWSetField32(val,
                 CW_TRANSPORT_HEADER_W_START,
                 CW_TRANSPORT_HEADER_W_LEN,
                 0);

    CWSetField32(val,
                 CW_TRANSPORT_HEADER_M_START,
                 CW_TRANSPORT_HEADER_M_LEN,
                 0); // no radio MAC address

    CWSetField32(val,
                 CW_TRANSPORT_HEADER_K_START,
                 CW_TRANSPORT_HEADER_K_LEN,
                 0); // Keep alive flag

    CWSetField32(val,
                 CW_TRANSPORT_HEADER_LAST_FRAGMENT_START,
                 CW_TRANSPORT_HEADER_LAST_FRAGMENT_LEN,
                 valuesPtr->fragEnd); // last fragment of packet

    CWSetField32(val,
                 CW_TRANSPORT_HEADER_FIRST_FRAGMENT_START,
                 CW_TRANSPORT_HEADER_FIRST_FRAGMENT_LEN,
                 valuesPtr->firstFrag); // first fragment of packet

    CWProtocolStore32(transportHdrPtr, val);
    // end of first 32 bits

    val = 0;

    CWSetField32(val,
                 CW_TRANSPORT_HEADER_FRAGMENT_ID_START,
                 CW_TRANSPORT_HEADER_FRAGMENT_ID_LEN,
                 valuesPtr->fragmentID); // fragment ID

    CWSetField32(val,
                 CW_TRANSPORT_HEADER_FRAGMENT_OFFSET_START,
                 CW_TRANSPORT_HEADER_FRAGMENT_OFFSET_LEN,
                 valuesPtr->fragmentOffset); // fragment offset

    CWSetField32(val,
                 CW_TRANSPORT_HEADER_RESERVED_START,
                 CW_TRANSPORT_HEADER_RESERVED_LEN,
                 0); // required

    CWProtocolStore32(transportHdrPtr, val);
    CWDebugLog("Protocol Version : %d", valuesPtr ->protocolVersion);
    if(valuesPtr ->protocolVersion == CW_MULTIC_PACKET)//for 1:mutli-packet  0:single-packet  
    {       
       val=0;
        CWSetField32(val,
                 CW_TRANSPORT_HEADER_NEXT_FRAGMENT_ID_START,
                 CW_TRANSPORT_HEADER_NEXT_FRAGMENT_ID_LEN,
                 valuesPtr->nextFragmentID);
      //16 bit is not defined for reserved 
       CWProtocolStore32(transportHdrPtr, val);
       CWProtocolStore32(transportHdrPtr, valuesPtr ->totalLen);
    }
    // end of second 32 bits

    return CW_TRUE;
}

// Assembles the Control Header
CWBool CWAssembleControlHeader(CWProtocolMessage *controlHdrPtr, CWControlHeaderValues *valPtr)
{
    if(controlHdrPtr == NULL || valPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    CW_CREATE_PROTOCOL_MESSAGE(*controlHdrPtr, 8,	 // meaningful bytes of the header
                               return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    CWProtocolStore32(controlHdrPtr, gCWIANATimes256 + valPtr->messageTypeValue); // = IANA Enterprise Number * 256 + Message Type Value
    CWProtocolStore8(controlHdrPtr, valPtr->seqNum);
    CWProtocolStore16(controlHdrPtr, (CW_CONTROL_HEADER_OFFSET_FOR_MSG_ELEMS + (valPtr->msgElemsLen))); // 7 is for the next 8+32+16 bits (= 7 bytes), MessageElementsLength+flags + timestamp
    CWProtocolStore8(controlHdrPtr, 0); // flags
    //CWProtocolStore32(controlHdrPtr, ((unsigned int)(time(NULL))) ); // timestamp

    return CW_TRUE;
}

CWBool CWAssembleMsgElemResultCode(CWProtocolMessage *msgPtr, CWProtocolResultCode code)
{
    if(msgPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    // create message
    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, 4, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    CWProtocolStore32(msgPtr, code);
    CWDebugLog("Result Code: %d", code);

    return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_RESULT_CODE_CW_TYPE);
}

CWBool CWAssembleMsgElemUpgFlow(CWProtocolMessage *msgPtr, CWUpgFlow flow)
{
    if(msgPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    // create message
    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, 4, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    CWProtocolStore32(msgPtr, flow);
    CWDebugLog("upgFlow: %d", flow);

    return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_UPG_FLOW);
}

CWBool CWAssembleMsgElemImageDownloadStatus(CWProtocolMessage *msgPtr, CWWTPImageDownloadStatus *status)
{
    if(msgPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    // create message
    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, 6, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    CWProtocolStore32(msgPtr, status->size);
    CWProtocolStore8(msgPtr, status->end);
    CWProtocolStore8(msgPtr, status->resultCode);

    return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_IMAGE_DOWNLOAD_STATUS);
}

CWBool CWAssembleMsgElemImageInitDownload(CWProtocolMessage *msgPtr)
{
    if(msgPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    msgPtr->offset = 0;

    return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_INITIATED_DOWNLOAD_CW_TYPE);
}

CWBool CWAssembleMsgElemImageData(CWProtocolMessage *msgPtr, CWImageData *imageData)
{
    if(msgPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }
    if(imageData == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    if(imageData->failed)
    {
        CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, 1,
                                   return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
        CWProtocolStore8(msgPtr, 5);
    }
    else
    {
        if(imageData->dataLen == 0 || imageData->data == NULL)
        {
            return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
        }

        CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, 1 + imageData->dataLen,
                                   return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
        if(imageData->eof)
        {
            CWProtocolStore8(msgPtr, 2);
        }
        else
        {
            CWProtocolStore8(msgPtr, 1);
        }
        CWProtocolStoreRawBytes(msgPtr, imageData->data, imageData->dataLen);
    }

    return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_IMAGE_DATA_CW_TYPE);
}

CWBool CWAssembleMsgElemImageIdentifier(CWProtocolMessage *msgPtr, CWImageIdentifier *imageId)
{
    int imageNamelen;

    if(msgPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }
    if(imageId == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    imageNamelen = strlen(imageId->imageName);
    if(!imageNamelen)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    // create message
    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, 4 + 1 + 2 + imageNamelen, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    CWProtocolStore32(msgPtr, CW_VENDOR_ID);
    CWProtocolStore8(msgPtr, imageId->necessary);
    CWProtocolStore16(msgPtr, imageNamelen);
    CWProtocolStoreRawBytes(msgPtr, imageId->imageName, imageNamelen);

    return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_IMAGE_IDENTIFIER_CW_TYPE);
}

CWBool CWAssembleMsgElemBackgroundSitesurveyValue(CWProtocolMessage *msgPtr, CWBackgroundSitesurveyValues *bgStsrvyVal)
{
    if(msgPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }
    if(bgStsrvyVal == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }
    // create message
    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, (4 + 2 + 4 + 4 + 4), return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
    CWProtocolStore32(msgPtr, CW_VENDOR_ID);
    CWProtocolStore16(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_BACKGROUND_SITESURVEY);
    CWProtocolStore32(msgPtr, bgStsrvyVal->radioType);
    CWProtocolStore32(msgPtr, bgStsrvyVal->bEnable);
    CWProtocolStore32(msgPtr, bgStsrvyVal->uint32Interval);

    CWDebugLog("AC background sitesurvey request radiotype: %s enable: %u interval: %u", (bgStsrvyVal->radioType==0 ?"2G": bgStsrvyVal->radioType==1 ? "5G1":"5G2" ), bgStsrvyVal->bEnable, bgStsrvyVal->uint32Interval);
    return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE);
}

CWBool CWAssembleMsgElemAutoTxPowerHealingValue(CWProtocolMessage *msgPtr, CWAutoTxPowerHealingValues *txPwHealingVal)
{
    if(msgPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }
    if(txPwHealingVal == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }
    // create message
    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, (4 + 2 + 4 + 4), return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
    CWProtocolStore32(msgPtr, CW_VENDOR_ID);
    CWProtocolStore16(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_AUTO_TXPOWER);
    CWProtocolStore32(msgPtr, txPwHealingVal->radioType);
    CWProtocolStore32(msgPtr, txPwHealingVal->strength);

    return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE);
}

CWBool CWAssembleMsgElemImageInformation(CWProtocolMessage *msgPtr, CWImageInformation *imageInfo)
{
    if(msgPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }
    if(imageInfo == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    // create message
    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, 4 + 16, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    CWProtocolStore32(msgPtr, imageInfo->fileSize);
    CWProtocolStoreRawBytes(msgPtr, (char *)imageInfo->hash, 16);

    return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_IMAGE_INFO_CW_TYPE);
}

// Assemble a CAPWAP Control Packet, with the given Message Elements, Sequence Number and Message Type. Create Transport and Control Headers.
// completeMsgPtr is an array of fragments (can be of size 1 if the packet doesn't need fragmentation
CWBool CWAssembleMessage(CWProtocolMessage **completeMsgPtr, int *fragmentsNumPtr, int PMTU, int seqNum, int msgTypeValue, CWProtocolMessage *msgElems, const int msgElemNum, CWProtocolMessage *msgElemsBinding, const int msgElemBindingNum, int crypted)
{
    
    int calc_offset = 0;
    int iCountPacket = 0;
    int max_fragmentsNum = 0;
    int calc_fragement = 0;

    CWProtocolMessage transportHdr, controlHdr, msg;
    int msgElemsLen = 0;
    int i,j,k = 0;

    CWProtocolTransportHeaderValues transportVal;
    CWControlHeaderValues controlVal;

    if(completeMsgPtr == NULL || fragmentsNumPtr == NULL || (msgElems == NULL && msgElemNum > 0) || (msgElemsBinding == NULL && msgElemBindingNum > 0))
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    //Calculate the whole size of the Msg Elements
    for(i = 0; i < msgElemNum; i++)
    {
        msgElemsLen += msgElems[i].offset;
    }
    for(i = 0; i < msgElemBindingNum; i++)
    {
        msgElemsLen += msgElemsBinding[i].offset;
    }

    //Assemble Control Header
    controlVal.messageTypeValue = msgTypeValue;
    controlVal.msgElemsLen = msgElemsLen;
    controlVal.seqNum = seqNum;

    if(!(CWAssembleControlHeader(&controlHdr, &controlVal)))
    {
        CW_FREE_PROTOCOL_MESSAGE(controlHdr);
        for(i = 0; i < msgElemNum; i++)
        {
            CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);
        }
        CW_FREE_OBJECT(msgElems);
        for(i = 0; i < msgElemBindingNum; i++)
        {
            CW_FREE_PROTOCOL_MESSAGE(msgElemsBinding[i]);
        }
        CW_FREE_OBJECT(msgElemsBinding);
        return CW_FALSE; // will be handled by the caller
    }

    // assemble the message putting all the data consecutively
    CW_CREATE_PROTOCOL_MESSAGE(msg, controlHdr.offset + msgElemsLen, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    CWProtocolStoreMessage(&msg, &controlHdr);
    for(i = 0; i < msgElemNum; i++)   // store in the request all the Message Elements
    {
        CWProtocolStoreMessage(&msg, &(msgElems[i]));
    }
    for(i = 0; i < msgElemBindingNum; i++)   // store in the request all the Message Elements
    {
        CWProtocolStoreMessage(&msg, &(msgElemsBinding[i]));
    }

    //Free memory not needed anymore
    CW_FREE_PROTOCOL_MESSAGE(controlHdr);
    for(i = 0; i < msgElemNum; i++)
    {
        CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);
    }
    CW_FREE_OBJECT(msgElems);

    for(i = 0; i < msgElemBindingNum; i++)
    {
        CW_FREE_PROTOCOL_MESSAGE(msgElemsBinding[i]);
    }
    CW_FREE_OBJECT(msgElemsBinding);

    if(PMTU == 0)
    {
        PMTU = CW_PMTU_DEFAULT;
    }

    CWDebugLog("PMTU: %d Pkt Len %d", PMTU, msg.offset);

    // handle fragmentation
    PMTU = PMTU - gMaxDTLSHeaderSize - gCapwapHdrSize - gDTLSTailerSize;

    if(PMTU > 0)
    {
        PMTU = (PMTU / 8) * 8; // CAPWAP fragments are made of groups of 8 bytes
        if(PMTU == 0)
        {
            goto cw_dont_fragment;
        }

        if( msg.offset >  MAX_PACKET_SIZE)
        {
           iCountPacket = msg.offset / MAX_PACKET_SIZE;     
           max_fragmentsNum = MAX_PACKET_SIZE / PMTU;  
        }

        calc_offset= msg.offset - (max_fragmentsNum * PMTU * iCountPacket);
        *fragmentsNumPtr = iCountPacket* max_fragmentsNum + calc_offset / PMTU;
       
        if((calc_offset % PMTU) != 0)
        {
            (*fragmentsNumPtr)++;
        }
        CWDebugLog("Fragments #: %d", *fragmentsNumPtr);
    }
    else
    {
cw_dont_fragment:
        *fragmentsNumPtr = 1;
    }

    if(*fragmentsNumPtr == 1)
    {
        CWDebugLog("1 Fragment");

        CW_CREATE_OBJECT_ERR(*completeMsgPtr, CWProtocolMessage, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

        transportVal.isFragment = transportVal.last = transportVal.fragmentOffset = transportVal.fragmentID = 0;
        transportVal.nextFragmentID = 0;
        transportVal.payloadType = crypted;
        transportVal.fragEnd =0;
        transportVal.totalLen = msgElemsLen;
        transportVal.protocolVersion = CW_SINGLE_PACKET;
        //		transportVal.last = 1;

        // Assemble Message Elements
        if(!(CWAssembleTransportHeader(&transportHdr, &transportVal)))
        {
            CW_FREE_PROTOCOL_MESSAGE(msg);
            CW_FREE_PROTOCOL_MESSAGE(transportHdr);
            CW_FREE_OBJECT(completeMsgPtr);
            return CW_FALSE; // will be handled by the caller
        }

        // assemble the message putting all the data consecutively
        CW_CREATE_PROTOCOL_MESSAGE(((*completeMsgPtr)[0]), transportHdr.offset + msg.offset, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

        CWProtocolStoreMessage(&((*completeMsgPtr)[0]), &transportHdr);
        CWProtocolStoreMessage(&((*completeMsgPtr)[0]), &msg);

        CW_FREE_PROTOCOL_MESSAGE(transportHdr);
        CW_FREE_PROTOCOL_MESSAGE(msg);
    }
    else
    {


        
        int totalSize = msg.offset;  
        int fragID = 0;
        int nextFragId = 0;
        int firstFrag =0;
        int fragType = CW_SINGLE_PACKET;

        if(iCountPacket > 0)
           fragType = CW_MULTIC_PACKET;

        CWDebugLog("%d Fragments", *fragmentsNumPtr);
        CWDebugLog("packetCount %d maxfrag =%d",iCountPacket,max_fragmentsNum);

        CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(*completeMsgPtr, *fragmentsNumPtr, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
        
        
        msg.offset = 0;
        for (i = 0; i < iCountPacket; i++)
        {
           if( msg.offset == 0)
           {
              fragID = CWGetFragmentID();
              nextFragId = CWGetFragmentID();
              firstFrag = 1;
           }
           else
           {
              fragID = nextFragId;
              nextFragId = CWGetFragmentID();
              firstFrag = 0;
           }

           for(j = 0; j < max_fragmentsNum; j++)
           {
               
               int fragSize;   
               transportVal.isFragment = 1;
               transportVal.fragmentOffset = msg.offset / 8;
               transportVal.fragmentID = fragID;
               transportVal.nextFragmentID = nextFragId;
               transportVal.payloadType = crypted;
               transportVal.firstFrag = firstFrag;
               transportVal.totalLen = msgElemsLen;
               transportVal.protocolVersion = fragType;

               if(msg.offset == 0)
                 k=0;
               else
                 k++;              

               fragSize = PMTU;
               if(j < ( max_fragmentsNum - 1)) // not last fragment of packet
               {
                     
                   transportVal.last = 0;
                   transportVal.fragEnd = 0;
               }
               else     
               {                   
                   if( *fragmentsNumPtr == (k+1)) //check last fragment
                   {
                      transportVal.last = 1;
                      CWLog("Last one fragement with One Packet size");
                   }
                   transportVal.fragEnd = 1; //last fragment of this packet
               }
               
               CWDebugLog("packet msg k= %d fragid =%d endfrag %d Fragment #:%d, offset:%d, bytes stored:%d/%d nextid %d",
               k,fragID,transportVal.fragEnd,j, transportVal.fragmentOffset, fragSize, totalSize,transportVal.nextFragmentID);
   
               // Assemble Transport Header for this fragment
               if(!(CWAssembleTransportHeader(&transportHdr, &transportVal)))
               {
                   CW_FREE_PROTOCOL_MESSAGE(msg);
                   CW_FREE_PROTOCOL_MESSAGE(transportHdr);
                   CW_FREE_OBJECT(completeMsgPtr);
                   return CW_FALSE; // will be handled by the caller
               }
   
               CW_CREATE_PROTOCOL_MESSAGE(((*completeMsgPtr)[k]), transportHdr.offset + fragSize, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
   
               CWProtocolStoreMessage(&((*completeMsgPtr)[k]), &transportHdr);
               CWProtocolStoreRawBytes(&((*completeMsgPtr)[k]), &((msg.msg)[msg.offset]), fragSize);
               msg.offset += fragSize;
   
               CW_FREE_PROTOCOL_MESSAGE(transportHdr);
               
           }
        }

       //Last packet
       calc_fragement = *fragmentsNumPtr - (iCountPacket* max_fragmentsNum);
 
        if( msg.offset == 0)
        {
           fragID = CWGetFragmentID();
           firstFrag = 1;
        }
        else
        {
           fragID = nextFragId;
           firstFrag = 0;
           nextFragId = CWGetFragmentID();
        }

        for(i = 0; i < calc_fragement; i++)   // for each fragment to assemble
        {
            int fragSize;

            transportVal.isFragment = 1;
            transportVal.fragmentOffset = (msg.offset - (max_fragmentsNum *iCountPacket * PMTU) ) / 8;
            transportVal.fragmentID = fragID;
            transportVal.nextFragmentID = fragID;
            transportVal.firstFrag = firstFrag;
            transportVal.totalLen = msgElemsLen;
            transportVal.protocolVersion = fragType;

            if(msg.offset == 0)                         
               k=0;            
            else            
               k++;           
            
            transportVal.payloadType = crypted;

            if(i < (calc_fragement - 1)) // not last fragment
            {
                fragSize = PMTU;
                transportVal.last = 0;
                transportVal.fragEnd = 0;
            }
            else     // last fragment
            {
                fragSize = totalSize -(max_fragmentsNum *iCountPacket * PMTU ) - ((calc_fragement - 1) * PMTU);
                transportVal.last = 1;
                transportVal.fragEnd = 1;                 
            }

             CWDebugLog("packet msg k= %d fragid =%d endfrag %d Fragment #:%d, offset:%d, bytes stored:%d/%d netxid %d",
                        k,fragID,transportVal.fragEnd,i, transportVal.fragmentOffset, fragSize, totalSize,transportVal.nextFragmentID );


            // Assemble Transport Header for this fragment
            if(!(CWAssembleTransportHeader(&transportHdr, &transportVal)))
            {
                CW_FREE_PROTOCOL_MESSAGE(msg);
                CW_FREE_PROTOCOL_MESSAGE(transportHdr);
                CW_FREE_OBJECT(completeMsgPtr);
                return CW_FALSE; // will be handled by the caller
            }

            CW_CREATE_PROTOCOL_MESSAGE(((*completeMsgPtr)[k]), transportHdr.offset + fragSize, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

            CWProtocolStoreMessage(&((*completeMsgPtr)[k]), &transportHdr);
            CWProtocolStoreRawBytes(&((*completeMsgPtr)[k]), &((msg.msg)[msg.offset]), fragSize);
            msg.offset += fragSize;

            CW_FREE_PROTOCOL_MESSAGE(transportHdr);
        }
        CW_FREE_PROTOCOL_MESSAGE(msg);

    }

    return CW_TRUE;
}

void CWProtocolDestroyFragment(void *f)
{
    CW_FREE_OBJECT(((CWProtocolFragment *)f)->data);
    CW_FREE_OBJECT(f);
}

CWBool CWCompareFragment(void *newFrag, void *oldFrag)
{
    CWProtocolFragment *newEl = (CWProtocolFragment *) newFrag;
    CWProtocolFragment *oldEl = (CWProtocolFragment *) oldFrag;

    if((newEl->transportVal.fragmentID == oldEl->transportVal.fragmentID) &&
       (newEl->transportVal.fragmentOffset == oldEl->transportVal.fragmentOffset))
    {
        return CW_TRUE;
    }

    return CW_FALSE;
}

// parse a sigle fragment. If it is the last fragment we need or the only fragment, return the reassembled message in
// *reassembleMsg. If we need at lest one more fragment, save this fragment in the list. You then call this function again
// with a new fragment and the same list untill we got all the fragments.
CWBool CWProtocolParseFragment(char *buf, int readBytes, CWList *fragmentsListPtr, CWProtocolMessage *reassembledMsg, CWBool *dataFlagPtr)
{
    CWProtocolTransportHeaderValues values;
    CWProtocolMessage msg;
    int totalSize;
    int icount_packet = 0;
    int icount_frag = 0;  
    

    msg.msg = buf;
    msg.offset = 0;

    *dataFlagPtr = CW_FALSE;

    if(!CWParseTransportHeader(&msg, &values, dataFlagPtr))
    {
        return CW_FALSE;
    }

    if(values.isFragment == 0)   // single fragment
    {
        CWDebugLog("Single Fragment");
        /*	if(*fragmentsListPtr != NULL) { // we are receiving another fragmented message,
        		return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Received Fragment with Different ID"); // discard this packet
        	}
        */
        CW_CREATE_PROTOCOL_MESSAGE(*reassembledMsg, (readBytes - msg.offset), return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

        CWProtocolStoreRawBytes(reassembledMsg, &(buf[msg.offset]), (readBytes - msg.offset));
        reassembledMsg->data_msgType = msg.data_msgType;
        return CW_TRUE;
    }
    else
    {
        CWListElement *el;
        CWProtocolFragment *fragPtr;
        int currentSize;
        int bLast= CW_FALSE;
        int firstFrag = 0;

        CW_CREATE_OBJECT_ERR(fragPtr, CWProtocolFragment, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

        fragPtr->transportVal.fragmentID = values.fragmentID;
        fragPtr->transportVal.fragmentOffset = values.fragmentOffset;
        fragPtr->transportVal.last = values.last;
        fragPtr->transportVal.nextFragmentID = values.nextFragmentID;
        fragPtr->transportVal.fragEnd = values.fragEnd;
        fragPtr->transportVal.firstFrag = values.firstFrag;
        fragPtr->transportVal.protocolVersion = values.protocolVersion;
        fragPtr->transportVal.totalLen = values.totalLen;
        CWDebugLog("Received Fragment ID:%d, offset:%d, notLast:%d endFrag:%d protocol %d totalen %d nextfrag %d", 
                  fragPtr->transportVal.fragmentID, 
                  fragPtr->transportVal.fragmentOffset, 
                  fragPtr->transportVal.last,
                  fragPtr->transportVal.fragEnd,
                  fragPtr->transportVal.protocolVersion,
                  fragPtr->transportVal.totalLen,
                  fragPtr->transportVal.nextFragmentID);
        if(fragPtr->transportVal.last == 1)
           bLast= CW_TRUE;
           
        fragPtr->dataLen = (readBytes - msg.offset);

        if(*fragmentsListPtr == NULL ||   // empty list
           (((CWProtocolFragment *)((*fragmentsListPtr)->data))->transportVal.fragmentID) == fragPtr->transportVal.fragmentID || // this fragment is in the set of fragments we are receiving
           (((CWProtocolFragment *)((*fragmentsListPtr)->data))->transportVal.nextFragmentID) == fragPtr->transportVal.fragmentID
           ) 
            /*{
                        CW_CREATE_OBJECT_SIZE_ERR(fragPtr->data, fragPtr->dataLen, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
                        CW_COPY_MEMORY(fragPtr->data, &(buf[msg.offset]), fragPtr->dataLen);

                        if(!CWAddElementToList(fragmentsListPtr, fragPtr)) {
                            CWDeleteList(fragmentsListPtr, CWProtocolDestroyFragment);
                            CW_FREE_OBJECT(fragPtr);
                            return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
                       }
                    }*/
        {
            CWListElement *aux = NULL;
            aux = CWSearchInList(*fragmentsListPtr, fragPtr, CWCompareFragment);
            if(aux == NULL)
            {
                CW_CREATE_OBJECT_SIZE_ERR(fragPtr->data, fragPtr->dataLen, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
                CW_COPY_MEMORY(fragPtr->data, &(buf[msg.offset]), fragPtr->dataLen);

                if(!CWAddElementToList(fragmentsListPtr, fragPtr))
                {
                    CWDeleteList(fragmentsListPtr, CWProtocolDestroyFragment);
                    CW_FREE_OBJECT(fragPtr);
                    return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
                }
            }
            else
            {
                CWDebugLog("Received a copy of a fragment already in List");
                //If the fragment is the last fragment, check if we have all the fragments
                if ((fragPtr->transportVal.fragEnd != 1) && (fragPtr->transportVal.last != 1))
                {
                    CW_FREE_OBJECT(fragPtr);
                    return CWErrorRaise(CW_ERROR_NEED_RESOURCE, NULL);
                }
                CW_FREE_OBJECT(fragPtr);
            }
        }
        else
        {
            CWDebugLog("Discarded old fragments for different fragment ID: %d Vs %d", fragPtr->transportVal.fragmentID, (((CWProtocolFragment *)((*fragmentsListPtr)->data))->transportVal).fragmentID);
            CWDeleteList(fragmentsListPtr, CWProtocolDestroyFragment);
            CW_CREATE_OBJECT_SIZE_ERR(fragPtr->data, fragPtr->dataLen, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
            CW_COPY_MEMORY(fragPtr->data, &(buf[msg.offset]), fragPtr->dataLen);
            if(!CWAddElementToList(fragmentsListPtr, fragPtr))
            {
                CWDeleteList(fragmentsListPtr, CWProtocolDestroyFragment);
                CW_FREE_OBJECT(fragPtr);
                return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
            }
        }

        // check if we have all the fragments
        icount_frag=0;
        totalSize =0;
        for(el = *fragmentsListPtr, totalSize = 0; (el != NULL && bLast == CW_TRUE); el = el->next)
        {
            icount_frag++;
            //new version: for multi-packet
            if((((CWProtocolFragment *)(el->data))->transportVal.fragEnd) == 1)// last element  of packet
            {
                totalSize += (((CWProtocolFragment *)(el->data))->transportVal.fragmentOffset) * 8;
                totalSize += (((CWProtocolFragment *)(el->data))->dataLen);
            }

            //old version:for single packet
            if(((((CWProtocolFragment *)(el->data))->transportVal.last) == 1) &&
               ((((CWProtocolFragment *)(el->data))->transportVal.fragEnd) == 0))// last element 
            {
                totalSize = (((CWProtocolFragment *)(el->data))->transportVal.fragmentOffset) * 8;
                totalSize += ((CWProtocolFragment *)(el->data))->dataLen;
            }

            if(((CWProtocolFragment *)(el->data))->transportVal.fragmentOffset == 0)
              icount_packet++;//calculate number of packet
        }
               

        if(totalSize == 0)   // we haven't the last fragment
        {
            return CWErrorRaise(CW_ERROR_NEED_RESOURCE, NULL); // we need at least one more fragment
        }

        // calculate the size of all the fragments we have so far
        for(el = *fragmentsListPtr, currentSize = 0; el != NULL; el = el->next)
        {
            currentSize += (((CWProtocolFragment *)(el->data))->dataLen);   

            if( icount_packet ==1 )
            {
               firstFrag = ((CWProtocolFragment *)(el->data))->transportVal.fragmentID;
            }
            else
            {
               if( ((CWProtocolFragment *)(el->data))->transportVal.firstFrag == 1 )
                 firstFrag = ((CWProtocolFragment *)(el->data))->transportVal.fragmentID;
            }
            
        }

        CWDebugLog("totalSize = %d , currentSize = %d", totalSize, currentSize);

        if(currentSize != totalSize)
        {
            return CWErrorRaise(CW_ERROR_NEED_RESOURCE, NULL); // we need at least one more fragment
        }
        else
        {
            int currentOffset = 0;  
            int currentFragID =firstFrag;/*To Do: get first fragment id*/

            CWDebugLog("___Received All Fragments___");

            CW_CREATE_PROTOCOL_MESSAGE(*reassembledMsg, currentSize, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

            CW_REPEAT_FOREVER
            {
                CWBool found = CW_FALSE;

                // find the fragment in the list with the currend offset
                for(el = *fragmentsListPtr, currentSize = 0; el != NULL; el = el->next)
                {
                 
                    if(((((CWProtocolFragment *)(el->data))->transportVal.fragmentOffset) == currentOffset)&&
                       ((((CWProtocolFragment *)(el->data))->transportVal.fragmentID) == currentFragID))
                    {
                        found = CW_TRUE; 
                        break;
                    }
                }

                if(!found)   // mmm... we should have all the fragment, but we haven't a fragment for the current offset
                {
                    CWDeleteList(fragmentsListPtr, CWProtocolDestroyFragment);
                    CW_FREE_PROTOCOL_MESSAGE(*reassembledMsg);
                    return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Bad Fragmented Messsage");
                }

                   CWDebugLog("add fragment offset(%d) fragId (%d)",
                        (((CWProtocolFragment *)(el->data))->transportVal.fragmentOffset),
                        (((CWProtocolFragment *)(el->data))->transportVal.fragmentID));
                 
                if( currentFragID == firstFrag && currentOffset == 0)
                {
                   
                   if(icount_packet == 1)
                      reassembledMsg->combine_len = 0;
                   else
                      reassembledMsg->combine_len = ((CWProtocolFragment *)(el->data))->transportVal.totalLen;

                   CWDebugLog("%s  icount_packet = %d combine_len =%d",
                              __FUNCTION__,
                              icount_packet,
                              reassembledMsg->combine_len);
                }
                CWProtocolStoreRawBytes(reassembledMsg, (((CWProtocolFragment *)(el->data))->data), (((CWProtocolFragment *)(el->data))->dataLen));
                reassembledMsg->data_msgType = msg.data_msgType;

                if((((CWProtocolFragment *)(el->data))->transportVal.last) == 1)   // last fragment
                {
                    CWDebugLog("Message Reassembled  Len = %d",reassembledMsg->offset);

                    CWDeleteList(fragmentsListPtr, CWProtocolDestroyFragment);
                    return CW_TRUE;
                }

                if((((CWProtocolFragment *)(el->data))->transportVal.fragEnd) == 1)
                {
                   currentFragID=((CWProtocolFragment *)(el->data))->transportVal.nextFragmentID;
                   currentOffset =0;
                }
                else
                {
                   currentOffset += ((((CWProtocolFragment *)(el->data))->dataLen) / 8);
                }            
            }
        }
    }
}

// Parse Transport Header
CWBool CWParseTransportHeader(CWProtocolMessage *msgPtr, CWProtocolTransportHeaderValues *valuesPtr, CWBool *dataFlagPtr)
{
    int transport4BytesLen;
    int val;
    int optionalWireless = 0;
    //int version, rid;

    if(msgPtr == NULL || valuesPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    //CWDebugLog("Parse Transport Header");
    val = CWProtocolRetrieve32(msgPtr);

    valuesPtr->protocolVersion=CWGetField32(val, CW_TRANSPORT_HEADER_VERSION_START, CW_TRANSPORT_HEADER_VERSION_LEN);
    
    //version = CWGetField32(val, CW_TRANSPORT_HEADER_VERSION_START, CW_TRANSPORT_HEADER_VERSION_LEN);
    //    CWDebugLog("VERSION: %d", version);

    valuesPtr->payloadType = CWGetField32(val, CW_TRANSPORT_HEADER_TYPE_START, CW_TRANSPORT_HEADER_TYPE_LEN);
    //    CWDebugLog("PAYLOAD TYPE: %d", valuesPtr->payloadType);

    transport4BytesLen = CWGetField32(val,	CW_TRANSPORT_HEADER_HLEN_START, CW_TRANSPORT_HEADER_HLEN_LEN);
    //    CWDebugLog("HLEN: %d", transport4BytesLen);

    //rid = CWGetField32(val, CW_TRANSPORT_HEADER_RID_START, CW_TRANSPORT_HEADER_RID_LEN);
    //    CWDebugLog("RID: %d", rid);

    //    CWDebugLog("WBID: %d", CWGetField32(val, CW_TRANSPORT_HEADER_WBID_START, CW_TRANSPORT_HEADER_WBID_LEN));

    valuesPtr->type = CWGetField32(val, CW_TRANSPORT_HEADER_T_START, CW_TRANSPORT_HEADER_T_LEN);
    //CWDebugLog("TYPE: %d", valuesPtr->type);

    valuesPtr->isFragment = CWGetField32(val, CW_TRANSPORT_HEADER_F_START, CW_TRANSPORT_HEADER_F_LEN);
    //CWDebugLog("IS FRAGMENT: %d", valuesPtr->isFragment);

    valuesPtr->last = CWGetField32(val, CW_TRANSPORT_HEADER_L_START, CW_TRANSPORT_HEADER_L_LEN);
    //CWDebugLog("NOT LAST: %d", valuesPtr->last);

    optionalWireless = CWGetField32(val, CW_TRANSPORT_HEADER_W_START, CW_TRANSPORT_HEADER_W_LEN);
    //	CWDebugLog("OPTIONAL WIRELESS: %d", optionalWireless);

    valuesPtr->keepAlive = CWGetField32(val, CW_TRANSPORT_HEADER_K_START, CW_TRANSPORT_HEADER_K_LEN);
    //	CWDebugLog("KEEP ALIVE: %d", valuesPtr->keepAlive);
     
    valuesPtr->fragEnd= CWGetField32(val, CW_TRANSPORT_HEADER_LAST_FRAGMENT_START, CW_TRANSPORT_HEADER_LAST_FRAGMENT_LEN);
    valuesPtr->firstFrag= CWGetField32(val, CW_TRANSPORT_HEADER_FIRST_FRAGMENT_START, CW_TRANSPORT_HEADER_FIRST_FRAGMENT_LEN);

    val = CWProtocolRetrieve32(msgPtr);

    valuesPtr->fragmentID = CWGetField32(val, CW_TRANSPORT_HEADER_FRAGMENT_ID_START, CW_TRANSPORT_HEADER_FRAGMENT_ID_LEN);
    //	CWDebugLog("FRAGMENT_ID: %d", valuesPtr->fragmentID);

    valuesPtr->fragmentOffset = CWGetField32(val, CW_TRANSPORT_HEADER_FRAGMENT_OFFSET_START, CW_TRANSPORT_HEADER_FRAGMENT_OFFSET_LEN);
    //	CWDebugLog("FRAGMENT_OFFSET: %d", valuesPtr->fragmentOffset);

    if(valuesPtr->protocolVersion == CW_MULTIC_PACKET)//for multi-pack
    {
       val = CWProtocolRetrieve32(msgPtr);
       valuesPtr->nextFragmentID = CWGetField32(val, CW_TRANSPORT_HEADER_NEXT_FRAGMENT_ID_START, CW_TRANSPORT_HEADER_NEXT_FRAGMENT_ID_LEN);
       valuesPtr->totalLen = CWProtocolRetrieve32(msgPtr);
    }
    else
    {
       valuesPtr->nextFragmentID = valuesPtr->fragmentID;
       valuesPtr->totalLen = 0;
    }
    return (transport4BytesLen == 2 || (transport4BytesLen == 4 && optionalWireless == 1)) ? CW_TRUE : CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Malformed Transport Header"); //TEMP?
}

// Parse Control Header
CWBool CWParseControlHeader(CWProtocolMessage *msgPtr, CWControlHeaderValues *valPtr)
{
    unsigned char flags = 0;

    if(msgPtr == NULL || valPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    CWDebugLog("Parse Control Header");
    valPtr->messageTypeValue = CWProtocolRetrieve32(msgPtr) - gCWIANATimes256;
    CWDebugLog("MESSAGE_TYPE: %u",	valPtr->messageTypeValue);

    valPtr->seqNum = CWProtocolRetrieve8(msgPtr);
    CWDebugLog("SEQUENCE_NUMBER: %u", valPtr->seqNum);

    valPtr->msgElemsLen = CWProtocolRetrieve16(msgPtr);
    CWDebugLog("MESSAGE_ELEMENT_LENGTH: %u", valPtr->msgElemsLen);

    flags = CWProtocolRetrieve8(msgPtr);
    CWDebugLog("FLAGS: %u",	flags);

    //	valPtr->timestamp = CWProtocolRetrieve32(msgPtr);
    //	CWDebugLog("TIME_STAMP: %u",	valPtr->timestamp);

    return CW_TRUE;
}

//## Assemble a Message Response containing a Failure (Unrecognized Message) Result Code
CWBool CWAssembleUnrecognizedMessageResponse(CWProtocolMessage **messagesPtr, int *fragmentsNumPtr, int PMTU, int seqNum, int msgType)
{
    CWProtocolMessage *msgElems = NULL;
    const int msgElemCount = 1;
    CWProtocolMessage *msgElemsBinding = NULL;
    int msgElemBindingCount = 0;

    if(messagesPtr == NULL || fragmentsNumPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    CWLog("Assembling Unrecognized Message Response...");

    CW_CREATE_OBJECT_ERR(msgElems, CWProtocolMessage, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if(!(CWAssembleMsgElemResultCode(msgElems, CW_PROTOCOL_FAILURE_UNRECOGNIZED_REQ)))
    {
        CW_FREE_OBJECT(msgElems);
        return CW_FALSE;
    }

    if(!(CWAssembleMessage(messagesPtr, fragmentsNumPtr, PMTU, seqNum, msgType, msgElems, msgElemCount, msgElemsBinding, msgElemBindingCount, CW_PACKET_CRYPT)))
    {
        return CW_FALSE;
    }

    CWLog("Unrecognized Message Response Assembled");

    return CW_TRUE;
}

CWBool CWAssembleMsgElemSessionID(CWProtocolMessage *msgPtr, int sessionID)
{
    if(msgPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    // create message
    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, 4, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    CWProtocolStore32(msgPtr, sessionID);

    return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_SESSION_ID_CW_TYPE);
}

CWBool CWParseACName(CWProtocolMessage *msgPtr, int len, char **valPtr)
{
    CW_PARSE_MSG_ELEMMENT_START();

    *valPtr = CWProtocolRetrieveStr(msgPtr, len);
    if(valPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
    }
    CWDebugLog("AC Name:%s", *valPtr);

    CW_PARSE_MSG_ELEMENT_END();
}

CWBool CWParseWTPRadioAdminState(CWProtocolMessage *msgPtr, int len, CWRadioAdminInfoValues *valPtr)
{
    CW_PARSE_MSG_ELEMMENT_START();

    valPtr->ID = CWProtocolRetrieve8(msgPtr);
    valPtr->state = CWProtocolRetrieve8(msgPtr);
    //valPtr->cause = CWProtocolRetrieve8(msgPtr);

    CWDebugLog("WTP Radio Admin State: %d - %d", valPtr->ID, valPtr->state);

    CW_PARSE_MSG_ELEMENT_END();
}

CWBool CWParseWTPRadioOperationalState(CWProtocolMessage *msgPtr, int len, CWRadioOperationalInfoValues *valPtr)
{
    CW_PARSE_MSG_ELEMMENT_START();

    valPtr->ID = CWProtocolRetrieve8(msgPtr);
    valPtr->state = CWProtocolRetrieve8(msgPtr);
    valPtr->cause = CWProtocolRetrieve8(msgPtr);

    CWDebugLog("WTP Radio Operational State: %d - %d - %d", valPtr->ID, valPtr->state, valPtr->cause);

    CW_PARSE_MSG_ELEMENT_END();
}

CWBool CWParseImageIdentifier(CWProtocolMessage *msgPtr, int len, CWImageIdentifier *valPtr)
{
    int imageNameLen, vendorId;

    CW_PARSE_MSG_ELEMMENT_START();

    vendorId = CWProtocolRetrieve32(msgPtr);
    if(vendorId != CW_VENDOR_ID)
    {
        return CWErrorRaise(CW_ERROR_NOT_SUPPORTED, "Unknown Vendor Id in Image Identifier");
    }

    valPtr->necessary = CWProtocolRetrieve8(msgPtr);

    imageNameLen = CWProtocolRetrieve16(msgPtr);
    if(!imageNameLen)
    {
        return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Zero length of image name in Image Identifier");
    }

    if(imageNameLen >= sizeof(valPtr->imageName))
    {
        return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "The length of image name is too long in Image Identifier");
    }

    CWProtocolRetrieveStrNoCreate(msgPtr, valPtr->imageName, imageNameLen);

    CW_PARSE_MSG_ELEMENT_END();
}

CWBool CWParseImageData(CWProtocolMessage *msgPtr, int len, CWImageData *valPtr)
{
    int dataType;

    CW_PARSE_MSG_ELEMMENT_START();

    dataType = CWProtocolRetrieve8(msgPtr);

    if(dataType == 5)
    {
        valPtr->failed = CW_TRUE;
    }
    else
    {
        valPtr->failed = CW_FALSE;
        if(dataType == 2)
        {
            valPtr->eof = CW_TRUE;
        }
        else
        {
            valPtr->eof = CW_FALSE;
        }
        valPtr->dataLen = len - 1;
        if(!valPtr->dataLen)
        {
            return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Invalid Image Data Length");
        }

        CWProtocolRetrieveRawBytesPtr(msgPtr, &(valPtr->data), valPtr->dataLen);
    }

    CW_PARSE_MSG_ELEMENT_END();
}

CWBool CWParseImageInformation(CWProtocolMessage *msgPtr, int len, CWImageInformation *valPtr)
{
    CW_PARSE_MSG_ELEMMENT_START();

    valPtr->fileSize = CWProtocolRetrieve32(msgPtr);
    CWProtocolRetrieveRawBytesNoCreate(msgPtr, (char *)valPtr->hash, 16);

    CW_PARSE_MSG_ELEMENT_END();
}

CWBool CWParseFormatMsgElem(CWProtocolMessage *completeMsg, unsigned short int *type, unsigned int *len)
{
    *type = CWProtocolRetrieve16(completeMsg);
    if( *type == CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE_32_LEN)
       *len = CWProtocolRetrieve32(completeMsg);
    else
       *len = CWProtocolRetrieve16(completeMsg);
  
    return CW_TRUE;
}

CWBool CWParseResultCode(CWProtocolMessage *msgPtr, int len, CWProtocolResultCode *valPtr)
{
    CW_PARSE_MSG_ELEMMENT_START();

    *valPtr = CWProtocolRetrieve32(msgPtr);
    CWDebugLog("Result Code: %d",	*valPtr);

    CW_PARSE_MSG_ELEMENT_END();
}

CWBool CWParseUpgFlow(CWProtocolMessage *msgPtr, int len, int *valPtr)
{
    CW_PARSE_MSG_ELEMMENT_START();

    *valPtr = CWProtocolRetrieve32(msgPtr);
    CWDebugLog("UpgFlow: %d", *valPtr);

    CW_PARSE_MSG_ELEMENT_END();
}

CWBool CWParseImageDownloadStatus(CWProtocolMessage *msgPtr, int len, CWWTPImageDownloadStatus *valPtr)
{
    CW_PARSE_MSG_ELEMMENT_START();

    valPtr->size = CWProtocolRetrieve32(msgPtr);
    valPtr->end = CWProtocolRetrieve8(msgPtr);
    valPtr->resultCode = CWProtocolRetrieve8(msgPtr);

    CW_PARSE_MSG_ELEMENT_END();
}

void CWWTPResetRadioStatistics(CWRadioStatisticsInfo *radioStatistics)
{
    radioStatistics->lastFailureType = UNKNOWN_TYPE;
    radioStatistics->resetCount = 0;
    radioStatistics->SWFailureCount = 0;
    radioStatistics->HWFailuireCount = 0;
    radioStatistics->otherFailureCount = 0;
    radioStatistics->unknownFailureCount = 0;
    radioStatistics->configUpdateCount = 0;
    radioStatistics->channelChangeCount = 0;
    radioStatistics->bandChangeCount = 0;
    radioStatistics->currentNoiseFloor = 0;
}

void CWFreeMessageFragments(CWProtocolMessage *messages, int fragmentsNum)
{
    int i;

    for(i = 0; i < fragmentsNum; i++)
    {
        CW_FREE_PROTOCOL_MESSAGE(messages[i]);
    }
}

CWBool CWParseVendorPayload(CWProtocolMessage *msgPtr, int len, CWProtocolVendorSpecificValues *valPtr)
{
    int value32;
    unsigned char value8;

    CW_PARSE_MSG_ELEMMENT_START();

    valPtr->vendorId = CWProtocolRetrieve32(msgPtr);
    valPtr->vendorPayloadType = (unsigned short) CWProtocolRetrieve16(msgPtr);

    //    CWDebugLog("CWParseVendorPayload len %d oldOffset %d", len, oldOffset);

    if(valPtr->vendorId != CW_VENDOR_ID)
    {
        CWLog("CWParseVendorPayload other Vendor Id %d found", valPtr->vendorId);
        msgPtr->offset += (len - 6);
        valPtr->vendorPayloadType = 0;
        valPtr->payload = NULL;
        return CW_TRUE;
    }

    switch(valPtr->vendorPayloadType)
    {
        case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_WTP_CFG:
            CW_CREATE_ZERO_OBJECT_ERR(valPtr->payload, CWWtpCfgMsgList,
            {
                return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
            });
            if(!CWParseWtpCfgPayload(msgPtr, valPtr->payload))
            {
                CW_FREE_OBJECT(valPtr->payload);
                return CW_FALSE;
            }
            break;
        case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_WTP_CFG_RESULT:
            CW_CREATE_ZERO_OBJECT_ERR(valPtr->payload, CWWtpCfgResult,
            {
                return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
            });
            if(!CWParseWtpCfgResultPayload(msgPtr, valPtr->payload))
            {
                CW_FREE_OBJECT(valPtr->payload);
                return CW_FALSE;
            }
            break;
        case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_BACKGROUND_SITESURVEY:
            CW_CREATE_ZERO_OBJECT_ERR(valPtr->payload, CWBackgroundSitesurveyValues,
            {
                return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
            });
            if(!CWParseBackgroundSitesurveyPayload(msgPtr, (CWBackgroundSitesurveyValues *)valPtr->payload))
            {
                CW_FREE_OBJECT(valPtr->payload);
                return CW_FALSE;
            }
            break;
        case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_AUTO_TXPOWER:
            CW_CREATE_ZERO_OBJECT_ERR(valPtr->payload, CWAutoTxPowerHealingValues,
            {
                return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
            });
            if(!CWParseAutoHealTxPowerPayload(msgPtr, (CWAutoTxPowerHealingValues *)valPtr->payload))
            {
                CW_FREE_OBJECT(valPtr->payload);
                return CW_FALSE;
            }
            break;
        case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_UTC_TIME:
        case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_AUTO_TXPOWER_RESPONSE:
        case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_STATION_STATISTIC_INTERVAL:
        case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_STATION_STATISTIC_MAX_CLIENTS:
        case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_WAITING_TIME:
        case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_SYSTEM_UPTIME:
        case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_STATION_STATISTIC_POLL_INTERVAL:
        case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_MTU:
        case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_WTP_PACKET_INTERVEL:
            if(!CWParseValue32(msgPtr, &value32))
            {
                return CW_FALSE;
            }
            valPtr->payload = (void *)(long)value32;
            break;
        case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CURRENT_STATION_INFO:
        case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_STATION_STATISTIC: /*Added by larger*/
            CW_CREATE_ZERO_OBJECT_ERR(valPtr->payload, CWWTPStationInfo,
            {
                return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
            });
            if(!CWParseWtpStationInfo(msgPtr, valPtr->payload, valPtr->vendorPayloadType))
            {
                CW_FREE_OBJECT(valPtr->payload);
                return CW_FALSE;
            }
            break;
        case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_WLAN_STATISTICS:
            CW_CREATE_ZERO_OBJECT_ERR(valPtr->payload, CWWTPStatisticsInfo,
            {
                return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
            });
            if(!CWParseWtpStatisticsInfo(msgPtr, valPtr->payload))
            {
                CW_FREE_OBJECT(valPtr->payload);
                return CW_FALSE;
            }
            break;
        case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_IP_CFG_INFO:
            CW_CREATE_ZERO_OBJECT_ERR(valPtr->payload, CWIPCfgInfo,
            {
                return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
            });
            if(!CWParseIpCfg(msgPtr, valPtr->payload))
            {
                CW_FREE_OBJECT(valPtr->payload);
                return CW_FALSE;
            }
            break;
        case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CAPWAP_TIMER:
            CW_CREATE_ZERO_OBJECT_ERR(valPtr->payload, CWTimer,
            {
                return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
            });
            if(!CWParseCapwapTimer(msgPtr, valPtr->payload))
            {
                CW_FREE_OBJECT(valPtr->payload);
                return CW_FALSE;
            }
            break;
        case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_DEBUG_LOG:
        case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_MEMORY_LOG_THRESHOLD:
        case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CLIENT_STATE_CHANGE_EVENT_ENABLE:
            if(!CWParseValue8(msgPtr, &value8))
            {
                return CW_FALSE;
            }
            valPtr->payload = (void *)(long) value8;
            break;
        case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_WTP_CFG_CAP_INFO: 
        case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_AC_CFG_CAP_INFO: 
            CW_CREATE_ZERO_OBJECT_ERR(valPtr->payload, CWWtpCfgCapInfo,
            {
                return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
            });
            if(!CWParseWtpCfgCapInfo(msgPtr, valPtr->payload))
            {
                CW_FREE_OBJECT(valPtr->payload);
                return CW_FALSE;
            }
            break;
        case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_WTP_CURRENT_CFG_INFO:
            CW_CREATE_ZERO_OBJECT_ERR(valPtr->payload, CWWTPCurrentCfgInfo,
            {
                return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
            });
            if(!CWParseWtpCurrentCfgInfo(msgPtr, valPtr->payload))
            {
                CW_FREE_OBJECT(valPtr->payload);
                return CW_FALSE;
            }
            break;
        case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_PROXY_WTP_ADDRESS:
        case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_PROXY_AC_ADDRESS:
            CW_CREATE_ZERO_OBJECT_ERR(valPtr->payload, CWNetworkLev4Address,
            {
                return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
            });
            if(!CWParseProxyAddress(msgPtr, valPtr->payload))
            {
                CW_FREE_OBJECT(valPtr->payload);
                return CW_FALSE;
            }
            break;
        case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_PROXY_RESPONSE_WTP_INFO:
            CW_CREATE_ZERO_OBJECT_ERR(valPtr->payload, CWProxyRespWTPInfo,
            {
                return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
            });
            if(!CWParseProxyRespWTPInfo(msgPtr, valPtr->payload))
            {
                CW_FREE_OBJECT(valPtr->payload);
                return CW_FALSE;
            }
            break;
        case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_SITESURVEY:
            if(!CWParseValue8(msgPtr, &value8))
            {
                return CW_FALSE;
            }
            valPtr->payload = (void *)(long) value8;
            break;
        case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_SITESURVEY_RESULT:
            CW_CREATE_ZERO_OBJECT_ERR(valPtr->payload, CWWTPSitesurveyInfo,
            {
                return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
            });
            if(!CWParseWtpSitesurveyInfo(msgPtr, valPtr->payload))
            {
                return CW_FALSE;
            }
            break;
        case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_KICKMAC:
            CW_CREATE_ZERO_OBJECT_ERR(valPtr->payload, CWWTPKickmacInfo,
            {
                return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
            });
            if(!CWParseKickmacInfoPayload(msgPtr, valPtr->payload))
            {
                CW_FREE_OBJECT(valPtr->payload);
                return CW_FALSE;
            }
            break;
        case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_SWITCH_PORT_INFO:
            CW_CREATE_ZERO_OBJECT_ERR(valPtr->payload, CWWTPSwitchPortInfo,
            {
                return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
            });
            if(!CWParseWtpSwitchPortInfo(msgPtr, valPtr->payload))
            {
                CW_FREE_OBJECT(valPtr->payload);
                return CW_FALSE;
            }
            break;
        case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_SWITCH_POE_INFO:
            CW_CREATE_ZERO_OBJECT_ERR(valPtr->payload, CWWTPSwitchPoeInfo,
            {
                return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
            });
            if(!CWParseWtpSwitchPoeInfo(msgPtr, valPtr->payload))
            {
                CW_FREE_OBJECT(valPtr->payload);
                return CW_FALSE;
            }
            break;
        case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_SWITCH_TOPOLOGY:
            CW_CREATE_ZERO_OBJECT_ERR(valPtr->payload, CWWTPSwitchTopologyInfo,
            {
                return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
            });
            if(!CWParseWtpSwitchTopology(msgPtr, valPtr->payload))
            {
                CW_FREE_OBJECT(valPtr->payload);
                return CW_FALSE;
            }
            break;
        case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_SWITCH_TRUNK_INFO:
            CW_CREATE_ZERO_OBJECT_ERR(valPtr->payload, CWWTPSwitchTrunkInfo,
            {
                return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
            });
            if(!CWParseWtpSwitchTrunkInfo(msgPtr, valPtr->payload))
            {
                CW_FREE_OBJECT(valPtr->payload);
                return CW_FALSE;
            }
            break;
        case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_SHELL_CMD:
            CW_CREATE_ZERO_OBJECT_ERR(valPtr->payload, CWShellCmdInfo,
            {
                return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
            });
            if(!CWParseShellCmdPayload(msgPtr, valPtr->payload))
            {
                CW_FREE_OBJECT(valPtr->payload);
                return CW_FALSE;
            }
            break;
        case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_SHELL_CMD_OUTPUT:
            CW_CREATE_ZERO_OBJECT_ERR(valPtr->payload, CWStringValue,
            {
                return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
            });
            if(!CWParseString(msgPtr, valPtr->payload))
            {
                CW_FREE_OBJECT(valPtr->payload);
                return CW_FALSE;
            }
            break;
        case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_MEMORY_INFO:
            CW_CREATE_ZERO_OBJECT_ERR(valPtr->payload, CWMemoryInfo,
            {
                return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
            });
            if(!CWParseMemoryInfoPayload(msgPtr, valPtr->payload))
            {
                CW_FREE_OBJECT(valPtr->payload);
                return CW_FALSE;
            }
            break;
        case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_ALLPY_CONFIG_INFO:
            CW_CREATE_ZERO_OBJECT_ERR(valPtr->payload, CWWTPApplyConfigInfo,
            {
                return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
            });
            if(!CWParseApplyConfigInfoPayload(msgPtr, valPtr->payload))
            {
                CW_FREE_OBJECT(valPtr->payload);
                return CW_FALSE;
            }
            break;
        case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_MESH_INFO:
            CW_CREATE_ZERO_OBJECT_ERR(valPtr->payload, CWWTPMeshInfo,
            {
                return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
            });
            if(!CWParseMeshInfoPayload(msgPtr, valPtr->payload))
            {
                CW_FREE_OBJECT(valPtr->payload);
                return CW_FALSE;
            }
            break;
        default:
            CWLog("CWParseVendorPayload unknown Payload Type %d", valPtr->vendorPayloadType);
            msgPtr->offset += (len - 6);
            valPtr->vendorPayloadType = 0;
            valPtr->payload = NULL;
    }

    //CWDebugLog("CWParseVendorPayload msgPtr->offset %d", msgPtr->offset);
    CW_PARSE_MSG_ELEMENT_END();
}

void CWDestroyVendorSpecificValues(CWProtocolVendorSpecificValues *valPtr)
{
    if(valPtr == NULL)
    {
        return;
    }

    if(valPtr->payload)
    {
        switch(valPtr->vendorPayloadType)
        {
            case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_WTP_CFG:
                CWWtpCfgMsgListFree((CWWtpCfgMsgList *) valPtr->payload);
                CW_FREE_OBJECT(valPtr->payload);
                break;
            case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CURRENT_STATION_INFO:
            case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_STATION_STATISTIC:
                CWWtpStationInfoFree((CWWTPStationInfo *) valPtr->payload);
                CW_FREE_OBJECT(valPtr->payload);
                break;
            case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_WLAN_STATISTICS:
                CW_FREE_OBJECT(((CWWTPStatisticsInfo *) valPtr->payload)->info);
                CW_FREE_OBJECT(valPtr->payload);
                break;
            case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_WTP_CFG_CAP_INFO:
                CW_FREE_OBJECT(((CWWtpCfgCapInfo *) valPtr->payload)->cfgCap);
                CW_FREE_OBJECT(valPtr->payload);
                break;
            case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_WTP_CURRENT_CFG_INFO:
                CWWtpCurrentCfgInfoFree((CWWTPCurrentCfgInfo *) valPtr->payload);
                CW_FREE_OBJECT(valPtr->payload);
                break;
            case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_SITESURVEY_RESULT:
                CW_FREE_OBJECT(((CWWTPSitesurveyInfo *) valPtr->payload)->info);
                CW_FREE_OBJECT(valPtr->payload);
                break;
            case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_KICKMAC:
                CW_FREE_OBJECT(((CWWTPKickmacInfo *) valPtr->payload)->client);
                CW_FREE_OBJECT(valPtr->payload);
                break;
            case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_SWITCH_PORT_INFO:
                //CW_FREE_OBJECT(((CWWTPSwitchPortInfo *)(valPtr->payload))->info);
                CWWtpSwitchPortInfoFree((CWWTPSwitchPortInfo *) valPtr->payload);
                
                CW_FREE_OBJECT(valPtr->payload);
                break;
            case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_SWITCH_POE_INFO:
                CW_FREE_OBJECT(((CWWTPSwitchPoeInfo *) valPtr->payload)->info);
                CW_FREE_OBJECT(valPtr->payload);
                break;
            case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_SWITCH_TOPOLOGY:
                CWWtpSwitchTopologyInfoFree((CWWTPSwitchTopologyInfo *) valPtr->payload);
                CW_FREE_OBJECT(valPtr->payload);
                break;
            case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_SHELL_CMD:
                CW_FREE_OBJECT(((CWShellCmdInfo *) valPtr->payload)->cmd);
                CW_FREE_OBJECT(valPtr->payload);
                break;
            case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_SHELL_CMD_OUTPUT:
                CW_FREE_OBJECT(((CWStringValue *) valPtr->payload)->ptr);
                CW_FREE_OBJECT(valPtr->payload);
                break;
            case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_SWITCH_TRUNK_INFO:
                CWWtpSwitchTrunkInfoFree((CWWTPSwitchTrunkInfo *) valPtr->payload);
                CW_FREE_OBJECT(valPtr->payload);
                break;
            case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_MESH_INFO:
                CWWtpMeshInfoFree((CWWTPMeshInfo *) valPtr->payload);
                CW_FREE_OBJECT(valPtr->payload);
                break;
            case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_WTP_CFG_RESULT:
            case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_BACKGROUND_SITESURVEY:
            case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_IP_CFG_INFO:
            case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CAPWAP_TIMER:
            case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_PROXY_WTP_ADDRESS:
            case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_PROXY_AC_ADDRESS:
            case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_MEMORY_INFO:
            case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_ALLPY_CONFIG_INFO:
                CW_FREE_OBJECT(valPtr->payload);
                break;
            default:
                valPtr->payload = NULL;
                break;
        }
    }
    valPtr->vendorId = 0;
    valPtr->vendorPayloadType = 0;
}

CWBool CWGetVendorInfoValue(CWWTPVendorInfos *vendorInfo, int type, void **ptr, int *len)
{
    int i;

    for(i = 0; i < vendorInfo->vendorInfosCount; i++)
    {
        if(vendorInfo->info[i].type == type &&
           (vendorInfo->info[i].vendorIdentifier == 0 || vendorInfo->info[i].vendorIdentifier == CW_VENDOR_ID)
          )
        {
            *ptr = (void *) vendorInfo->info[i].valuePtr;
            *len = vendorInfo->info[i].length;
            return CW_TRUE;
        }
    }
    *ptr = NULL;
    *len = 0;
    return CW_FALSE;
}

CWBool CWAssembleMsgElemVendorPayloadWtpCfg(CWProtocolMessage *msgPtr, CWWtpCfgMsgList *cfgList)
{
    CWWtpCfgMsgNode *cfgNode;
    int type;

    if(msgPtr == NULL || cfgList == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, 4 + 2 + cfgList->msgSize + 2 /* plus type size of WTP_CFG_NONE */,
    {
        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
    });

    CWProtocolStore32(msgPtr, CW_VENDOR_ID);
    CWProtocolStore16(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_WTP_CFG);
    cfgNode = cfgList->head;
    while(cfgNode)
    {
        CWProtocolStore16(msgPtr, (unsigned short) cfgNode->type);
        CWProtocolStore16(msgPtr, (unsigned short) cfgNode->keyLen);
        if(cfgNode->keyLen)
        {
            CWProtocolStoreRawBytes(msgPtr, (char *) cfgNode->keyPtr, cfgNode->keyLen);
        }
        CWProtocolStore16(msgPtr, (unsigned short) cfgNode->valLen);
        if(cfgNode->valLen)
        {
            CWProtocolStoreRawBytes(msgPtr, (char *) cfgNode->valPtr, cfgNode->valLen);
        }
        cfgNode = cfgNode->next;
    }
    CWProtocolStore16(msgPtr, WTP_CFG_NONE); /* end of message */

   
    if( msgPtr->offset < MAX_PACKET_SIZE)
       type = CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE;
    else
       type = CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE_32_LEN;

    CWDebugLog("###%s type= %d msg->offset=%d",__FUNCTION__,type,msgPtr->offset);
    return CWAssembleMsgElem(msgPtr, type);
}

CWBool CWAssembleMsgElemVendorPayloadWtpCfgResult(CWProtocolMessage *msgPtr, CWWtpCfgResult *cfgRes)
{
    int msgStrLen;
    int msgLen;

    if(msgPtr == NULL || cfgRes == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    msgStrLen = strlen(cfgRes->message);
    msgLen = 4 + 2 + 1 + 1 + 1 + 2 + 2 + msgStrLen;

    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, msgLen,
    {
        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
    });

    CWProtocolStore32(msgPtr, CW_VENDOR_ID);
    CWProtocolStore16(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_WTP_CFG_RESULT);
    CWProtocolStore8(msgPtr, (unsigned char) cfgRes->apply);
    CWProtocolStore8(msgPtr, (unsigned char) cfgRes->handle);
    CWProtocolStore8(msgPtr, (unsigned char) cfgRes->rejoin);
    CWProtocolStore16(msgPtr, (unsigned short) cfgRes->waitSec);
    CWProtocolStore16(msgPtr, (unsigned short) msgStrLen);
    if(msgStrLen)
    {
        CWProtocolStoreRawBytes(msgPtr, cfgRes->message, msgStrLen);
    }

    return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE);
}

CWBool CWAssembleMsgElemVendorPayloadNoValue(CWProtocolMessage *msgPtr, int payloadType)
{
    if(msgPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, 4 + 2 ,
    {
        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
    });

    CWProtocolStore32(msgPtr, CW_VENDOR_ID);
    CWProtocolStore16(msgPtr, (unsigned short) payloadType);

    return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE);
}



CWBool CWAssembleMsgElemVendorPayloadValue32(CWProtocolMessage *msgPtr, int value, int payloadType)
{
    if(msgPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, 4 + 2 + 4,
    {
        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
    });

    CWProtocolStore32(msgPtr, CW_VENDOR_ID);
    CWProtocolStore16(msgPtr, (unsigned short) payloadType);
    CWProtocolStore32(msgPtr, (int) value);

    return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE);
}

CWBool CWAssembleMsgElemVendorPayloadString(CWProtocolMessage *msgPtr, const char *str, int payloadType)
{
    unsigned short strLen;

    if(msgPtr == NULL || !str)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    strLen = (unsigned short) strlen(str);

    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, 4 + 2 + 2 + strLen,
    {
        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
    });

    CWProtocolStore32(msgPtr, CW_VENDOR_ID);
    CWProtocolStore16(msgPtr, (unsigned short) payloadType);
    CWProtocolStore16(msgPtr, strLen);
    CWProtocolStoreRawBytes(msgPtr, str, strLen);

    return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE);
}

CWBool CWAssembleMsgElemVendorPayloadValue8(CWProtocolMessage *msgPtr, unsigned char value, int payloadType)
{
    if(msgPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, 4 + 2 + 1,
    {
        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
    });

    CWProtocolStore32(msgPtr, CW_VENDOR_ID);
    CWProtocolStore16(msgPtr, (unsigned short) payloadType);
    CWProtocolStore8(msgPtr, value);

    return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE);
}

CWBool CWAssembleMsgElemVendorPayloadIPCfgInfo(CWProtocolMessage *msgPtr, CWIPCfgInfo *valPtr)
{
    if(msgPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, 4 + 2 + 4 + 4 + 4 + 4 + 4,
    {
        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
    });

    CWProtocolStore32(msgPtr, CW_VENDOR_ID);
    CWProtocolStore16(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_IP_CFG_INFO);

    CWProtocolStoreIPv4Address(msgPtr, valPtr->ip);
    CWProtocolStoreIPv4Address(msgPtr, valPtr->mask);
    CWProtocolStoreIPv4Address(msgPtr, valPtr->gateway);
    CWProtocolStoreIPv4Address(msgPtr, valPtr->dns1);
    CWProtocolStoreIPv4Address(msgPtr, valPtr->dns2);

    return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE);
}

CWBool CWAssembleMsgElemVendorPayloadCapwapTimer(CWProtocolMessage *msgPtr, CWTimer *valPtr)
{
    if(msgPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, 4 + 2 + 10,
    {
        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
    });

    CWProtocolStore32(msgPtr, CW_VENDOR_ID);
    CWProtocolStore16(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CAPWAP_TIMER);

    CWProtocolStore8(msgPtr, (unsigned char)valPtr->dtlsSetup);
    CWProtocolStore8(msgPtr, (unsigned char)valPtr->joinState);
    CWProtocolStore8(msgPtr, (unsigned char)valPtr->imageState);
    CWProtocolStore8(msgPtr, (unsigned char)valPtr->imageData);
    CWProtocolStore8(msgPtr, (unsigned char)valPtr->configState);
    CWProtocolStore8(msgPtr, (unsigned char)valPtr->changeState);
    CWProtocolStore8(msgPtr, (unsigned char)valPtr->peerDead);
    CWProtocolStore8(msgPtr, (unsigned char)valPtr->retransmitInterval);
    CWProtocolStore8(msgPtr, (unsigned char)valPtr->retransmitCount);
    CWProtocolStore8(msgPtr, (unsigned char)valPtr->fragment);

    return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE);
}

CWBool CWAssembleMsgElemVendorPayloadWtpCfgCapInfo(CWProtocolMessage *msgPtr, CWWtpCfgCapInfo *valPtr)
{
    int i;

    if(msgPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, 4 + 2 + 1 + 4 + 4 + (valPtr->count * sizeof(CWWtpCfgCap)),
    {
        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
    });

    CWProtocolStore32(msgPtr, CW_VENDOR_ID);
    CWProtocolStore16(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_WTP_CFG_CAP_INFO);

    CWProtocolStore8(msgPtr, 0); /* VERSION 0 */
    CWProtocolStore32(msgPtr, sizeof(CWWtpCfgCap));
    CWProtocolStore32(msgPtr, valPtr->count);
    for(i = 0; i < valPtr->count; i++)
    {
        CWProtocolStoreRawBytes(msgPtr, (char *)valPtr->cfgCap[i], sizeof(CWWtpCfgCap));
    }

    return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE);
}


CWBool CWAssembleMsgElemVendorPayloadAcCfgCapInfo(CWProtocolMessage *msgPtr, CWWtpCfgCapInfo *valPtr)
{
    int i;

    if(msgPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, 4 + 2 + 1 + 4 + 4 + (valPtr->count * sizeof(CWWtpCfgCap)),
    {
        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
    });

    CWProtocolStore32(msgPtr, CW_VENDOR_ID);
    CWProtocolStore16(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_AC_CFG_CAP_INFO);
    CWProtocolStore8(msgPtr, 0); /* VERSION 0 */
    CWProtocolStore32(msgPtr, sizeof(CWWtpCfgCap));
    CWProtocolStore32(msgPtr, valPtr->count);
    for(i = 0; i < valPtr->count; i++)
    {
        CWProtocolStoreRawBytes(msgPtr, (char *)valPtr->cfgCap[i], sizeof(CWWtpCfgCap));
        
    }

    return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE);
}


CWBool CWAssembleMsgElemVendorPayloadProxyRespWtpInfo(CWProtocolMessage *msgPtr, CWProxyRespWTPInfo *wtpInfo)
{
    if(msgPtr == NULL || wtpInfo == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, 4 + 2 + 6 + 4 + 2,
    {
        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
    });

    CWProtocolStore32(msgPtr, CW_VENDOR_ID);
    CWProtocolStore16(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_PROXY_RESPONSE_WTP_INFO);
    CWProtocolStoreRawBytes(msgPtr, (char *) wtpInfo->mac, 6);
    CWProtocolStoreIPv4Address(msgPtr, wtpInfo->ip);
    CWProtocolStore16(msgPtr, wtpInfo->port);

    return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE);
}

CWBool CWAssembleMsgElemVendorPayloadSitesurvey(CWProtocolMessage *msgPtr, unsigned char *radio)
{
    if(msgPtr == NULL || radio == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, 4 + 2 + 1 /* plus type size of radio type */,
    {
        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
    });

    CWProtocolStore32(msgPtr, CW_VENDOR_ID);
    CWProtocolStore16(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_SITESURVEY);
    CWProtocolStore8(msgPtr, *radio);

    return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE);
}

CWBool CWAssembleMsgElemVendorPayloadKickmac(CWProtocolMessage *msgPtr, CWWTPKickmacInfo *kicks)
{
    int i;
    if(msgPtr == NULL || kicks == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, 4 + 2 + 1 + 1 + 2 + kicks->clientNum * 6 /* plus type size of radio type */,
    {
        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
    });

    CWProtocolStore32(msgPtr, CW_VENDOR_ID);
    CWProtocolStore16(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_KICKMAC);
    CWProtocolStore8(msgPtr, kicks->radio);
    CWProtocolStore8(msgPtr, kicks->wlan);
    CWProtocolStore16(msgPtr, kicks->clientNum);

    for(i = 0; i < kicks->clientNum; i++)
    {
        CWProtocolStoreRawBytes(msgPtr, (char *) kicks->client[i].mac, 6);
    }

    return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE);
}

CWBool CWAssembleMsgElemVendorPayloadShellCmd(CWProtocolMessage *msgPtr, CWShellCmdInfo *valPtr)
{
    if(msgPtr == NULL || !valPtr)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, 4 + 2 + 1 + 4 + 2 + valPtr->cmdLen,
    {
        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
    });

    CWProtocolStore32(msgPtr, CW_VENDOR_ID);
    CWProtocolStore16(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_SHELL_CMD);
    CWProtocolStore8(msgPtr, (unsigned char) valPtr->outputType);
    CWProtocolStore32(msgPtr, valPtr->timeout);
    CWProtocolStore16(msgPtr, (unsigned short) valPtr->cmdLen);
    CWProtocolStoreRawBytes(msgPtr, valPtr->cmd, valPtr->cmdLen);

    return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE);
}

CWBool CWAssembleMsgElemVendorPayloadMemoryInfo(CWProtocolMessage *msgPtr, CWMemoryInfo *valPtr)
{
    if(msgPtr == NULL || !valPtr)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, 4 + 2 + 4 + 4,
    {
        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
    });

    CWProtocolStore32(msgPtr, CW_VENDOR_ID);
    CWProtocolStore16(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_MEMORY_INFO);
    CWProtocolStore32(msgPtr, valPtr->total);
    CWProtocolStore32(msgPtr, valPtr->used);

    return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE);
}

CWBool CWAssembleMsgElemVendorPayloadApplyConfigInfo(CWProtocolMessage *msgPtr, CWWTPApplyConfigInfo *valPtr)
{
    if(msgPtr == NULL || !valPtr)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, 4 + 2 + 1 + 4,
    {
        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
    });

    CWProtocolStore32(msgPtr, CW_VENDOR_ID);
    CWProtocolStore16(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_ALLPY_CONFIG_INFO);
    CWProtocolStore8(msgPtr, valPtr->cancel);
    CWProtocolStore32(msgPtr, valPtr->waitTime);

    return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE);
}

CWBool CWParseWtpCfgPayload(CWProtocolMessage *msgPtr, CWWtpCfgMsgList *cfgList)
{
    unsigned short cfgType;
    unsigned short keyLen, valLen;
    char *keyPtr, *valPtr;

    if(msgPtr == NULL || cfgList == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    do
    {
        cfgType = CWProtocolRetrieve16(msgPtr);
        //CWDebugLog("CWParseWtpCfgPayload msgPtr->offset %d cfgType %d", msgPtr->offset, cfgType);
        if(cfgType == WTP_CFG_NONE) /* end of message */
        {
            break;
        }
#if 0
        if(cfgType < WTP_CFG_START)
        {
            CWLog("Invalid WTP Cfg Type: %u", cfgType);
            CWWtpCfgMsgListFree(cfgList);
            return CWErrorRaise(CW_ERROR_INVALID_FORMAT, NULL);
        }
#endif
        keyLen = CWProtocolRetrieve16(msgPtr);
        //CWDebugLog("CWParseWtpCfgPayload msgPtr->offset %d keyLen %d", msgPtr->offset, keyLen);
        if(keyLen)
        {
            keyPtr = CWProtocolRetrieveRawBytes(msgPtr, keyLen);
            if(!keyPtr)
            {
                CWWtpCfgMsgListFree(cfgList);
                return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
            }
        }
        else
        {
            keyPtr = NULL;
        }

        valLen = CWProtocolRetrieve16(msgPtr);
        if(valLen)
        {
            valPtr = CWProtocolRetrieveRawBytes(msgPtr, valLen);
            if(!valPtr)
            {
                CW_FREE_OBJECT(keyPtr);
                CWWtpCfgMsgListFree(cfgList);
                return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
            }
        }
        else
        {
            valPtr = NULL;
        }
        //CWDebugLog("CWParseWtpCfgPayload msgPtr->offset %d valLen %d", msgPtr->offset, valLen);
        if(!CWWtpCfgMsgListAdd(cfgList, cfgType, keyLen, keyPtr, valLen, valPtr))
        {
            CW_FREE_OBJECT(keyPtr);
            CW_FREE_OBJECT(valPtr);
            CWWtpCfgMsgListFree(cfgList);
            return CW_FALSE;
        }
    }
    while(1);

    return CW_TRUE;
}

CWBool CWParseWtpCfgResultPayload(CWProtocolMessage *msgPtr, CWWtpCfgResult *cfgRes)
{
    unsigned short msgStrLen;

    if(msgPtr == NULL || cfgRes == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    cfgRes->apply = CWProtocolRetrieve8(msgPtr);
    cfgRes->handle = CWProtocolRetrieve8(msgPtr);
    cfgRes->rejoin = CWProtocolRetrieve8(msgPtr);
    cfgRes->waitSec = CWProtocolRetrieve16(msgPtr);
    msgStrLen = CWProtocolRetrieve16(msgPtr);
    if(msgStrLen)
    {
        CWProtocolRetrieveStrNoCreate(msgPtr, cfgRes->message,
                                      sizeof(cfgRes->message) <= msgStrLen ? sizeof(cfgRes->message) - 1 : msgStrLen);
    }
    else
    {
        cfgRes->message[0] = '\0';
    }

    //    CWDebugLog("WTP Cfg Result Type: %d", cfgRes->type);
    //    CWDebugLog("WTP Cfg Result Code: %d", cfgRes->code);
    //    CWDebugLog("WTP Cfg Result Handle: %d", cfgRes->handle);
    //    CWDebugLog("WTP Cfg Result Rejoin: %d", cfgRes->rejoin);
    //    CWDebugLog("WTP Cfg Result WaitSec: %d", cfgRes->waitSec);
    //    CWDebugLog("WTP Cfg Result Msg: %s", cfgRes->message);

    return CW_TRUE;
}

CWBool CWParseShellCmdPayload(CWProtocolMessage *msgPtr, CWShellCmdInfo *valPtr)
{
    if(msgPtr == NULL || valPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    valPtr->outputType = CWProtocolRetrieve8(msgPtr);
    valPtr->timeout = CWProtocolRetrieve32(msgPtr);
    valPtr->cmdLen = CWProtocolRetrieve16(msgPtr);
    valPtr->cmd = CWProtocolRetrieveStr(msgPtr, valPtr->cmdLen);

    return valPtr->cmd ? CW_TRUE : CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
}

CWBool CWParseMemoryInfoPayload(CWProtocolMessage *msgPtr, CWMemoryInfo *valPtr)
{
    if(msgPtr == NULL || valPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    valPtr->total = CWProtocolRetrieve32(msgPtr);
    valPtr->used = CWProtocolRetrieve32(msgPtr);

    return CW_TRUE;
}

CWBool CWParseApplyConfigInfoPayload(CWProtocolMessage *msgPtr, CWWTPApplyConfigInfo *valPtr)
{
    if(msgPtr == NULL || valPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    valPtr->cancel = CWProtocolRetrieve8(msgPtr);
    valPtr->waitTime = CWProtocolRetrieve32(msgPtr);

    return CW_TRUE;
}

CWBool CWParseBackgroundSitesurveyPayload(CWProtocolMessage *msgPtr, CWBackgroundSitesurveyValues *bgStSvyVal)
{
    if(msgPtr == NULL || bgStSvyVal == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    bgStSvyVal->radioType = CWProtocolRetrieve32(msgPtr);
    bgStSvyVal->bEnable = CWProtocolRetrieve32(msgPtr);
    bgStSvyVal->uint32Interval = CWProtocolRetrieve32(msgPtr);

    //CWDebugLog("WTP background sitesurvey request PARSE radiotype: %s enable: %u interval: %u", (bgStSvyVal->radioType?"5G":"2G"), bgStSvyVal->bEnable, bgStSvyVal->uint32Interval);
    return CW_TRUE;
}

CWBool CWParseAutoHealTxPowerPayload(CWProtocolMessage *msgPtr, CWAutoTxPowerHealingValues *txPwHealingVal)
{
    if(msgPtr == NULL || txPwHealingVal == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    txPwHealingVal->radioType = CWProtocolRetrieve32(msgPtr);
    txPwHealingVal->strength = CWProtocolRetrieve32(msgPtr);

    CWDebugLog("WTP auto heal txpower request PARSE radiotype: %s txpower: %d%%",
               (txPwHealingVal->radioType ? "5G" : "2G"), txPwHealingVal->strength);
    return CW_TRUE;
}

CWBool CWParseKickmacInfoPayload(CWProtocolMessage *msgPtr, CWWTPKickmacInfo *kickmac)
{
    int i;

    if(msgPtr == NULL || kickmac == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    kickmac->radio = CWProtocolRetrieve8(msgPtr);
    kickmac->wlan = CWProtocolRetrieve8(msgPtr);
    kickmac->clientNum = CWProtocolRetrieve16(msgPtr);
    CW_CREATE_ZERO_ARRAY_ERR(kickmac->client, kickmac->clientNum,
                             KickmacClient, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
    for(i = 0; i < kickmac->clientNum; i++)
    {
        CWProtocolRetrieveRawBytesNoCreate(msgPtr, (char *) kickmac->client[i].mac, 6);
    }

    return CW_TRUE;
}

CWBool CWParseValue32(CWProtocolMessage *msgPtr, int *value)
{
    if(msgPtr == NULL || value == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    *value = (int) CWProtocolRetrieve32(msgPtr);

    return CW_TRUE;
}

CWBool CWParseValue8(CWProtocolMessage *msgPtr, unsigned char *value)
{
    if(msgPtr == NULL || value == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    *value = CWProtocolRetrieve8(msgPtr);

    return CW_TRUE;
}

CWBool CWParseString(CWProtocolMessage *msgPtr, CWStringValue *value)
{
    if(msgPtr == NULL || value == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    value->len = CWProtocolRetrieve16(msgPtr);
    value->ptr = CWProtocolRetrieveStr(msgPtr, value->len);

    return value->ptr ? CW_TRUE : CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
}

CWBool CWParseWtpStationInfo(CWProtocolMessage *msgPtr, CWWTPStationInfo *staInfo, int payloadType)
{
    int i, j;

    if(msgPtr == NULL || staInfo == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    staInfo->infoCount = CWProtocolRetrieve16(msgPtr);
    if(staInfo->infoCount)
    {
        CW_CREATE_ZERO_ARRAY_ERR(staInfo->info, staInfo->infoCount,
                                 CWWTPStation, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
        for(i = 0; i < staInfo->infoCount; i++)
        {
            staInfo->info[i].radioIdx = CWProtocolRetrieve8(msgPtr);
            staInfo->info[i].wlanIdx = CWProtocolRetrieve8(msgPtr);
            staInfo->info[i].stationCount = CWProtocolRetrieve16(msgPtr);
            if(staInfo->info[i].stationCount)
            {
                CW_CREATE_ZERO_ARRAY_ERR(staInfo->info[i].station, staInfo->info[i].stationCount, CWStation,
                {
                    CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
                    goto error_exit;
                });

                for(j = 0; j < staInfo->info[i].stationCount; j++)
                {
                    CWProtocolRetrieveRawBytesNoCreate(msgPtr, (char *) staInfo->info[i].station[j].mac, 6);
                    staInfo->info[i].station[j].txKB = (int) CWProtocolRetrieve32(msgPtr);
                    staInfo->info[i].station[j].rxKB = (int) CWProtocolRetrieve32(msgPtr);
                    if(payloadType == CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_STATION_STATISTIC)
                    {
                        staInfo->info[i].station[j].interval = (int) CWProtocolRetrieve32(msgPtr);
                    }
                    else
                    {
                        staInfo->info[i].station[j].rssi = (int) CWProtocolRetrieve32(msgPtr);
                    }

                    staInfo->info[i].station[j].osTypeLen = CWProtocolRetrieve8(msgPtr);
                    if(staInfo->info[i].station[j].osTypeLen)
                    {
                        staInfo->info[i].station[j].osType = CWProtocolRetrieveStr(msgPtr, staInfo->info[i].station[j].osTypeLen);
                    }
                    else
                    {
                        CW_CREATE_STRING_FROM_STRING_ERR(staInfo->info[i].station[j].osType, "",
                        {
                            CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
                            goto error_exit;
                        });
                    }
                    staInfo->info[i].station[j].hostNameLen = CWProtocolRetrieve8(msgPtr);
                    if(staInfo->info[i].station[j].hostNameLen)
                    {
                        staInfo->info[i].station[j].hostName = CWProtocolRetrieveStr(msgPtr, staInfo->info[i].station[j].hostNameLen);
                    }
                    else
                    {
                        CW_CREATE_STRING_FROM_STRING_ERR(staInfo->info[i].station[j].hostName, "",
                        {
                            CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
                            goto error_exit;
                        });
                    }
                    staInfo->info[i].station[j].address = CWProtocolRetrieveIPv4Address(msgPtr);
                    /*
                    CWLog("[%s:%d] i:%d j:%d %d %s", __FUNCTION__, __LINE__, i, j, staInfo->info[i].station[j].osTypeLen, staInfo->info[i].station[j].osType);
                    CWLog("[%s:%d] i:%d j:%d %d %s", __FUNCTION__, __LINE__, i, j, staInfo->info[i].station[j].hostNameLen, staInfo->info[i].station[j].hostName);

                    struct in_addr addr1;
                    memcpy(&addr1, &(staInfo->info[i].station[j].address), 4);
                    CWLog("[%s:%d] %lx %s", __FUNCTION__, __LINE__, staInfo->info[i].station[j].address, inet_ntoa(addr1));
                    */
                }
            }
        }
    }
    else
    {
        staInfo->info = NULL;
    }

    return CW_TRUE;

error_exit:

    if(staInfo->info)
    {
        for(i = 0; i < staInfo->infoCount; i++)
        {
            if(staInfo->info[i].station)
            {
                for(j = 0; j < staInfo->info[i].stationCount; j++)
                {
                    CW_FREE_OBJECT(staInfo->info[i].station[j].osType);
                    CW_FREE_OBJECT(staInfo->info[i].station[j].hostName);
                }
                CW_FREE_OBJECT(staInfo->info[i].station);
            }
        }
        CW_FREE_OBJECT(staInfo->info);
    }

    return CW_FALSE;
}

CWBool CWParseWtpSwitchTopology(CWProtocolMessage *msgPtr, CWWTPSwitchTopologyInfo *topoInfo)
{
    int i, j;

    if(msgPtr == NULL || topoInfo == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    topoInfo->devCount = CWProtocolRetrieve16(msgPtr);
    if(topoInfo->devCount)
    {
        CW_CREATE_ZERO_ARRAY_ERR(topoInfo->devInfo, topoInfo->devCount,
                                 CWWTPSwitchDev, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
        for(i = 0; i < topoInfo->devCount; i++)
        {
            topoInfo->devInfo[i].type = CWProtocolRetrieve32(msgPtr);
            topoInfo->devInfo[i].sysTime = CWProtocolRetrieve32(msgPtr);
            topoInfo->devInfo[i].updateTime = CWProtocolRetrieve32(msgPtr);
            topoInfo->devInfo[i].linkInfoLastTime = CWProtocolRetrieve32(msgPtr);
            topoInfo->devInfo[i].noLinkInfoFirstTime = CWProtocolRetrieve32(msgPtr);
            CWProtocolRetrieveRawBytesNoCreate(msgPtr, (char *) topoInfo->devInfo[i].mac, 6);
            topoInfo->devInfo[i].ip = CWProtocolRetrieveIPv4Address(msgPtr);

            topoInfo->devInfo[i].nameLen = CWProtocolRetrieve32(msgPtr);
            if(topoInfo->devInfo[i].nameLen)
            {
                topoInfo->devInfo[i].name = CWProtocolRetrieveStr(msgPtr, topoInfo->devInfo[i].nameLen);
            }
            else
            {
                CW_CREATE_STRING_FROM_STRING_ERR(topoInfo->devInfo[i].name, "",
                {
                    return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
                });
            }

            topoInfo->devInfo[i].descLen = CWProtocolRetrieve32(msgPtr);
            if(topoInfo->devInfo[i].descLen)
            {
                topoInfo->devInfo[i].desc = CWProtocolRetrieveStr(msgPtr, topoInfo->devInfo[i].descLen);
            }
            else
            {
                CW_CREATE_STRING_FROM_STRING_ERR(topoInfo->devInfo[i].desc, "",
                {
                    return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
                });
            }

            topoInfo->devInfo[i].linkCount = CWProtocolRetrieve32(msgPtr);
            if(topoInfo->devInfo[i].linkCount)
            {
                CW_CREATE_ZERO_ARRAY_ERR(topoInfo->devInfo[i].linkList, topoInfo->devInfo[i].linkCount,
                                         CWWTPSwitchDevLink, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
                for(j = 0; j < topoInfo->devInfo[i].linkCount; j++)
                {
                    topoInfo->devInfo[i].linkList[j].localPort = CWProtocolRetrieve32(msgPtr);
                    CWProtocolRetrieveRawBytesNoCreate(msgPtr, (char *) topoInfo->devInfo[i].linkList[j].mac, 6);

                    topoInfo->devInfo[i].linkList[j].remotePortLen = CWProtocolRetrieve32(msgPtr);
                    if(topoInfo->devInfo[i].linkList[j].remotePortLen)
                    {
                        topoInfo->devInfo[i].linkList[j].remotePort = CWProtocolRetrieveStr(msgPtr, topoInfo->devInfo[i].linkList[j].remotePortLen);
                    }
                    else
                    {
                        CW_CREATE_STRING_FROM_STRING_ERR(topoInfo->devInfo[i].linkList[j].remotePort, "",
                        {
                            return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
                        });
                    }
                }
            }
        }
    }
    else
    {
        topoInfo->devInfo = NULL;
    }

    return CW_TRUE;
}

CWBool CWParseWtpSwitchPoeInfo(CWProtocolMessage *msgPtr, CWWTPSwitchPoeInfo *poeInfo)
{
    int i;

    if(msgPtr == NULL || poeInfo == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }
    poeInfo->infoCount = CWProtocolRetrieve16(msgPtr);
    poeInfo->powerBudget = CWProtocolRetrieve32(msgPtr);
    poeInfo->allocPower = CWProtocolRetrieve32(msgPtr);

    if(poeInfo->infoCount)
    {
        CW_CREATE_ZERO_ARRAY_ERR(poeInfo->info, poeInfo->infoCount,
                                 CWWTPSwitchPoePortInfo, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
        for(i = 0; i < poeInfo->infoCount; i++)
        {
            poeInfo->info[i].port = CWProtocolRetrieve32(msgPtr);
            poeInfo->info[i].state = CWProtocolRetrieve32(msgPtr);
            poeInfo->info[i].priority = CWProtocolRetrieve32(msgPtr);
            poeInfo->info[i].powerLimitType = CWProtocolRetrieve32(msgPtr);
            poeInfo->info[i].powerLimit = CWProtocolRetrieve32(msgPtr);
            poeInfo->info[i].status = CWProtocolRetrieve32(msgPtr);
            poeInfo->info[i].outputVoltage = CWProtocolRetrieve32(msgPtr);
            poeInfo->info[i].outputCurrent = CWProtocolRetrieve32(msgPtr);
            poeInfo->info[i].outputPower = CWProtocolRetrieve32(msgPtr);
            poeInfo->info[i].temperature = CWProtocolRetrieve32(msgPtr);
            poeInfo->info[i].class = CWProtocolRetrieve32(msgPtr);
        }
    }
    else
    {
        poeInfo->info = NULL;
    }

    return CW_TRUE;
}

CWBool CWParseWtpSwitchPortInfo(CWProtocolMessage *msgPtr, CWWTPSwitchPortInfo *portInfo)
{
    int i;

    if(msgPtr == NULL || portInfo == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    portInfo->infoCount = CWProtocolRetrieve16(msgPtr);

    if(portInfo->infoCount)
    {
        CW_CREATE_ZERO_ARRAY_ERR(portInfo->info, portInfo->infoCount,
                                 CWWTPSwitchPortStatus, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
        for(i = 0; i < portInfo->infoCount; i++)
        {
            portInfo->info[i].port = CWProtocolRetrieve32(msgPtr);
            portInfo->info[i].status = CWProtocolRetrieve32(msgPtr);
            portInfo->info[i].linkStatus = CWProtocolRetrieve32(msgPtr);
            portInfo->info[i].speed = CWProtocolRetrieve32(msgPtr);
            portInfo->info[i].duplex = CWProtocolRetrieve32(msgPtr);
            portInfo->info[i].flowControl = CWProtocolRetrieve32(msgPtr);
            portInfo->info[i].autoNeg = CWProtocolRetrieve32(msgPtr);
        }
    }
    else
    {
        portInfo->info = NULL;
    }

    return CW_TRUE;
}

CWBool CWParseWtpSwitchTrunkInfo(CWProtocolMessage *msgPtr, CWWTPSwitchTrunkInfo *trunkInfo)
{
    int i;

    if(msgPtr == NULL || trunkInfo == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    trunkInfo->infoCount = CWProtocolRetrieve16(msgPtr);

    if(trunkInfo->infoCount)
    {
        CW_CREATE_ZERO_ARRAY_ERR(trunkInfo->info, trunkInfo->infoCount,
                                 CWWTPSwitchTrunkStatus, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
        for(i = 0; i < trunkInfo->infoCount; i++)
        {
            trunkInfo->info[i].id = CWProtocolRetrieve32(msgPtr);
            trunkInfo->info[i].mode = CWProtocolRetrieve32(msgPtr);

            trunkInfo->info[i].activePort_len = CWProtocolRetrieve32(msgPtr);
            if(trunkInfo->info[i].activePort_len)
            {
                trunkInfo->info[i].activePort = CWProtocolRetrieveStr(msgPtr, trunkInfo->info[i].activePort_len);
            }
            else
            {
                CW_CREATE_STRING_FROM_STRING_ERR(trunkInfo->info[i].activePort, "",
                {
                    return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
                });
            }
            trunkInfo->info[i].memberPort_len = CWProtocolRetrieve32(msgPtr);
            if(trunkInfo->info[i].memberPort_len)
            {
                trunkInfo->info[i].memberPort = CWProtocolRetrieveStr(msgPtr, trunkInfo->info[i].memberPort_len);
            }
            else
            {
                CW_CREATE_STRING_FROM_STRING_ERR(trunkInfo->info[i].memberPort, "",
                {
                    return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
                });
            }
        }
    }
    else
    {
        trunkInfo->info = NULL;
    }

    return CW_TRUE;
}


CWBool CWParseWtpStatisticsInfo(CWProtocolMessage *msgPtr, CWWTPStatisticsInfo *statInfo)
{
    int i;

    if(msgPtr == NULL || statInfo == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    statInfo->infoCount = CWProtocolRetrieve16(msgPtr);

    if(statInfo->infoCount)
    {
        CW_CREATE_ZERO_ARRAY_ERR(statInfo->info, statInfo->infoCount,
                                 CWWlanStatisticsInfo, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
        for(i = 0; i < statInfo->infoCount; i++)
        {
            statInfo->info[i].radioIdx = CWProtocolRetrieve8(msgPtr);
            statInfo->info[i].wlanIdx = CWProtocolRetrieve8(msgPtr);
            statInfo->info[i].statistics.txPkts = CWProtocolRetrieve32(msgPtr);
            statInfo->info[i].statistics.txErrPkts = CWProtocolRetrieve32(msgPtr);
            statInfo->info[i].statistics.txDrpPkts = CWProtocolRetrieve32(msgPtr);
            statInfo->info[i].statistics.txBytes = CWProtocolRetrieve32(msgPtr);
            statInfo->info[i].statistics.rxPkts = CWProtocolRetrieve32(msgPtr);
            statInfo->info[i].statistics.rxErrPkts = CWProtocolRetrieve32(msgPtr);
            statInfo->info[i].statistics.rxDrpPkts = CWProtocolRetrieve32(msgPtr);
            statInfo->info[i].statistics.rxBytes = CWProtocolRetrieve32(msgPtr);
        }
    }
    else
    {
        statInfo->info = NULL;
    }

    return CW_TRUE;
}

CWBool CWParseWtpSitesurveyInfo(CWProtocolMessage *msgPtr, CWWTPSitesurveyInfo *sitesurveyInfo)
{
    int i;

    if(msgPtr == NULL || sitesurveyInfo == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    sitesurveyInfo->radio = CWProtocolRetrieve8(msgPtr);
    sitesurveyInfo->infoCount = CWProtocolRetrieve16(msgPtr);

    if(sitesurveyInfo->infoCount)
    {
        CW_CREATE_ZERO_ARRAY_ERR(sitesurveyInfo->info, sitesurveyInfo->infoCount, CWWTPSitesurvey,
                                 return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
        for(i = 0; i < sitesurveyInfo->infoCount; i++)
        {
            CWProtocolRetrieveRawBytesNoCreate(msgPtr, (char *) sitesurveyInfo->info[i].bssid, 6);
            sitesurveyInfo->info[i].ssidLen = CWProtocolRetrieve8(msgPtr);
            CWProtocolRetrieveRawBytesNoCreate(msgPtr, sitesurveyInfo->info[i].ssid, 33);
            sitesurveyInfo->info[i].mode = CWProtocolRetrieve8(msgPtr);
            sitesurveyInfo->info[i].chan = CWProtocolRetrieve32(msgPtr);
            sitesurveyInfo->info[i].signal = CWProtocolRetrieve32(msgPtr);
            sitesurveyInfo->info[i].enc = CWProtocolRetrieve8(msgPtr);
            sitesurveyInfo->info[i].type = CWProtocolRetrieve8(msgPtr);
        }
    }
    else
    {
        sitesurveyInfo->info = NULL;
    }

    return CW_TRUE;
}

CWBool CWParseIpCfg(CWProtocolMessage *msgPtr, CWIPCfgInfo *valPtr)
{
    if(msgPtr == NULL || valPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    valPtr->ip = CWProtocolRetrieveIPv4Address(msgPtr);
    valPtr->mask = CWProtocolRetrieveIPv4Address(msgPtr);
    valPtr->gateway = CWProtocolRetrieveIPv4Address(msgPtr);
    valPtr->dns1 = CWProtocolRetrieveIPv4Address(msgPtr);
    valPtr->dns2 = CWProtocolRetrieveIPv4Address(msgPtr);
    /*
    CWDebugLog("Is Static: %d", valPtr->isStatic);
    CWDebugLog("Ip: %u.%u.%u.%u", CW_IPV4_PRINT_LIST(valPtr->ip));
    CWDebugLog("Mask: %u.%u.%u.%u", CW_IPV4_PRINT_LIST(valPtr->mask));
    CWDebugLog("Gateway: %u.%u.%u.%u", CW_IPV4_PRINT_LIST(valPtr->gateway));
    CWDebugLog("DNS1: %u.%u.%u.%u", CW_IPV4_PRINT_LIST(valPtr->dns1));
    CWDebugLog("DNS2: %u.%u.%u.%u", CW_IPV4_PRINT_LIST(valPtr->dns2));
    */
    return CW_TRUE;
}

CWBool CWParseCapwapTimer(CWProtocolMessage *msgPtr, CWTimer *valPtr)
{
    if(msgPtr == NULL || valPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    valPtr->dtlsSetup = CWProtocolRetrieve8(msgPtr);
    valPtr->joinState = CWProtocolRetrieve8(msgPtr);
    valPtr->imageState = CWProtocolRetrieve8(msgPtr);
    valPtr->imageData = CWProtocolRetrieve8(msgPtr);
    valPtr->configState = CWProtocolRetrieve8(msgPtr);
    valPtr->changeState = CWProtocolRetrieve8(msgPtr);
    valPtr->peerDead = CWProtocolRetrieve8(msgPtr);
    valPtr->retransmitInterval = CWProtocolRetrieve8(msgPtr);
    valPtr->retransmitCount = CWProtocolRetrieve8(msgPtr);
    valPtr->fragment = CWProtocolRetrieve8(msgPtr);

    return CW_TRUE;
}

CWBool CWParseWtpCfgCapInfo(CWProtocolMessage *msgPtr, CWWtpCfgCapInfo *valPtr)
{
    int version;
    int cfgCapSize;
    int i, j;

    if(msgPtr == NULL || valPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    version = CWProtocolRetrieve8(msgPtr);
    if(version == 0) /* info version */
    {
        cfgCapSize = (int) CWProtocolRetrieve32(msgPtr);
        valPtr->count = (int) CWProtocolRetrieve32(msgPtr);
        CW_CREATE_ZERO_ARRAY_ERR(valPtr->cfgCap, valPtr->count, CWWtpCfgCap, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
        for(i = 0; i < valPtr->count; i++)
        {
            for(j = 0; j < cfgCapSize; j++)
            {
                if(j < sizeof(CWWtpCfgCap))
                {
                    valPtr->cfgCap[i][j] = CWProtocolRetrieve8(msgPtr);
                }
                else
                {
                    CWProtocolRetrieve8(msgPtr);
                }
            }
        }
    }
    else
    {
        return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Unknown CWWtpCfgCapInfo version %d", version);
    }

    return CW_TRUE;
}

CWBool CWParseWtpCurrentCfgInfo(CWProtocolMessage *msgPtr, CWWTPCurrentCfgInfo *valPtr)
{
    int i, j;

    if(msgPtr == NULL || valPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    valPtr->version = CWProtocolRetrieve8(msgPtr);
    if(valPtr->version == 0) /* info version */
    {
        valPtr->ipv4.address = CWProtocolRetrieveIPv4Address(msgPtr);
        valPtr->ipv4.mask = CWProtocolRetrieveIPv4Address(msgPtr);
        valPtr->ipv4.gateway = CWProtocolRetrieveIPv4Address(msgPtr);
        valPtr->dns1 = CWProtocolRetrieveIPv4Address(msgPtr);
        valPtr->dns2 = CWProtocolRetrieveIPv4Address(msgPtr);
        valPtr->radioCount = CWProtocolRetrieve8(msgPtr);
        if(valPtr->radioCount)
        {
            CW_CREATE_ZERO_ARRAY_ERR(valPtr->radio, valPtr->radioCount, CWWTPCurrentRadioCfgInfo,
                                     return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
            for(i = 0; i < valPtr->radioCount; i++)
            {
                valPtr->radio[i].channel = CWProtocolRetrieve32(msgPtr);
                valPtr->radio[i].txPower = CWProtocolRetrieve32(msgPtr);
                valPtr->radio[i].wlanCount = CWProtocolRetrieve8(msgPtr);
                if(valPtr->radio[i].wlanCount)
                {
                    CW_CREATE_ZERO_ARRAY_ERR(valPtr->radio[i].wlan, valPtr->radio[i].wlanCount, CWWTPCurrentWlanCfgInfo,
                    {
                        CWWtpCurrentCfgInfoFree(valPtr);
                        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
                    });
                    for(j = 0; j < valPtr->radio[i].wlanCount; j++)
                    {
                        CWProtocolRetrieveRawBytesNoCreate(msgPtr, (char *) valPtr->radio[i].wlan[j].bssid, 6);
                    }

                    valPtr->radio[i].channelCount = CWProtocolRetrieve8(msgPtr);
                    if(valPtr->radio[i].channelCount)
                    {
                        valPtr->radio[i].channelList = (unsigned char *) CWProtocolRetrieveRawBytes(msgPtr, valPtr->radio[i].channelCount);
                        if(!valPtr->radio[i].channelList)
                        {
                            CWWtpCurrentCfgInfoFree(valPtr);
                            return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
                        }
                    }
                }
            }
        }
        else
        {
            valPtr->radio = NULL;
        }
    }
    else
    {
        return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Unknown CWWTPCurrentCfgInfo version %d", valPtr->version);
    }

    return CW_TRUE;
}

CWBool CWParseProxyAddress(CWProtocolMessage *msgPtr, CWNetworkLev4Address *addr)
{
    unsigned int ip;
    unsigned short port;

    if(msgPtr == NULL || addr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    ip = CWProtocolRetrieveIPv4Address(msgPtr);
    port = CWProtocolRetrieve16(msgPtr);

    CW_ADDR_SET_IP(addr, ip);
    CW_ADDR_SET_PORT(addr, port);

    return CW_TRUE;
}

CWBool CWParseProxyRespWTPInfo(CWProtocolMessage *msgPtr, CWProxyRespWTPInfo *wtpInfo)
{
    if(msgPtr == NULL || wtpInfo == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    CWProtocolRetrieveRawBytesNoCreate(msgPtr, (char *) wtpInfo->mac, 6);
    wtpInfo->ip = CWProtocolRetrieveIPv4Address(msgPtr);
    wtpInfo->port = CWProtocolRetrieve16(msgPtr);

    return CW_TRUE;
}

CWBool CWParseMeshInfoPayload(CWProtocolMessage *msgPtr, CWWTPMeshInfo *valPtr)
{
    int i;

    if(msgPtr == NULL || valPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    valPtr->infoCount = CWProtocolRetrieve16(msgPtr);
    CW_CREATE_ZERO_ARRAY_ERR(valPtr->info, valPtr->infoCount,
                             CWRadioMeshInfo, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    for(i = 0; i < valPtr->infoCount; i++)
    {
        valPtr->info[i].role = CWProtocolRetrieve32(msgPtr);
        CWProtocolRetrieveRawBytesNoCreate(msgPtr, (char *) valPtr->info[i].bssid, 6);
    }

    return CW_TRUE;
}

CWBool CWAssembleMsgElemCertType(CWProtocolMessage *msgPtr, CWWTPCertType certType)
{
    if(msgPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, "CWAssembleMsgElemCertType arguments error.");
    }

    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, 1, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if(certType == CW_WTP_CERT_TYPE_DEFAULT)
    {
        CWProtocolStore8(msgPtr, 1);
    }
    else if(certType == CW_WTP_CERT_TYPE_CURRENT)
    {
        CWProtocolStore8(msgPtr, 2);
    }
    else if(certType == CW_WTP_CERT_TYPE_CURRENT_DEFAULT)
    {
        CWProtocolStore8(msgPtr, 3);
    }
    else
    {
        CWErrorRaise(CW_ERROR_NOT_SUPPORTED, "Not supported CWWTPCertType.");
    }

    return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_WTP_CERT_TYPE_INFO_CW_TYPE);
}

CWBool CWParseCertType(CWProtocolMessage *msgPtr, int len, CWWTPCertType *valPtr)
{
    CW_PARSE_MSG_ELEMMENT_START();

    int queryType = CWProtocolRetrieve8(msgPtr);

    if(queryType == 1)
    {
        *valPtr = CW_WTP_CERT_TYPE_DEFAULT;
    }
    else if(queryType == 2)
    {
        *valPtr = CW_WTP_CERT_TYPE_CURRENT;
    }
    else if(queryType == 3)
    {
        *valPtr = CW_WTP_CERT_TYPE_CURRENT_DEFAULT;
    }

    CW_PARSE_MSG_ELEMENT_END();
}

CWBool CWAssembleMsgElemCertResetReqType(CWProtocolMessage *msgPtr, CWWTPCertRequestType reqType)
{
    if(msgPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, "CWAssembleMsgElemCertResetQueryInfo arguments error.");
    }

    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, 1, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if(reqType == CW_WTP_CERT_REQUEST_TYPE_QUERY)
    {
        CWProtocolStore8(msgPtr, 1);
    }
    else if(reqType == CW_WTP_CERT_REQUEST_TYPE_CERT_REQ)
    {
        CWProtocolStore8(msgPtr, 2);
    }
    else if(reqType == CW_WTP_CERT_REQUEST_TYPE_FACTORY_RESET)
    {
        CWProtocolStore8(msgPtr, 3);
    }
    else
    {
        CWErrorRaise(CW_ERROR_NOT_SUPPORTED, "Not supported CWWTPCertRequestType.");
    }

    return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_WTP_CERT_RESET_REQ_TYPE_CW_TYPE);
}

CWBool CWAssembleMsgElemFactoryResetInterval(CWProtocolMessage *msgPtr, int seconds)
{
    if(msgPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, "CWAssembleMsgElemFactoryResetInterval arguments error.");
    }

    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, 4, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    CWProtocolStore32(msgPtr, seconds);

    return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_WTP_FACTORY_RESET_INTERVAL_CW_TYPE);
}

CWBool CWParseCertResetReqType(CWProtocolMessage *msgPtr, int len, CWWTPCertRequestType *valPtr)
{
    CW_PARSE_MSG_ELEMMENT_START();

    int queryType = CWProtocolRetrieve8(msgPtr);

    if(queryType == 1)
    {
        *valPtr = CW_WTP_CERT_REQUEST_TYPE_QUERY;
    }
    else if(queryType == 2)
    {
        *valPtr = CW_WTP_CERT_REQUEST_TYPE_CERT_REQ;
    }
    else if(queryType == 3)
    {
        *valPtr = CW_WTP_CERT_REQUEST_TYPE_FACTORY_RESET;
    }

    CW_PARSE_MSG_ELEMENT_END();
}

CWBool CWParseFactoryResetInterval(CWProtocolMessage *msgPtr, int len, int *valPtr)
{
    CW_PARSE_MSG_ELEMMENT_START();

    *valPtr = CWProtocolRetrieve32(msgPtr);

    CW_PARSE_MSG_ELEMENT_END();
}

CWBool CWAssembleMsgElemCertResetReqData(CWProtocolMessage *msgPtr, CWWTPCertResetRequestValues *dataPtr)
{
    if(msgPtr == NULL || dataPtr == NULL
       || dataPtr->certReqData.dataLen == 0 || dataPtr->certReqData.data == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, "CWAssembleMsgElemCertResetReqData arguments error.");
    }

    msgPtr->offset = 0;

    CWDebugLog("CW_CREATE_PROTOCOL_MESSAGE begin:%d", dataPtr->certReqData.dataLen);
    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, 16 + dataPtr->certReqData.dataLen, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
    CWProtocolStoreRawBytes(msgPtr, (char *)dataPtr->certReqData.certreqHash, 16);
    CWProtocolStoreRawBytes(msgPtr, dataPtr->certReqData.data, dataPtr->certReqData.dataLen);

    return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_WTP_CERTREQ_DATA_CW_TYPE);
}

CWBool CWParseCertResetReqData(CWProtocolMessage *msgPtr, int len, CWWTPCertReqData *valPtr)
{
    CW_PARSE_MSG_ELEMMENT_START();

    valPtr->dataLen = len - 16;
    if(!valPtr->dataLen)
    {
        return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Invalid Cert request Data Length");
    }

    CWProtocolRetrieveRawBytesNoCreate(msgPtr, (char *)valPtr->certreqHash, 16);
    CWProtocolRetrieveRawBytesPtr(msgPtr, &(valPtr->data), valPtr->dataLen);

    CW_PARSE_MSG_ELEMENT_END();
}

CWBool CWParseCertData(CWProtocolMessage *msgPtr, int len, CWACCertData *valPtr)
{
    CW_PARSE_MSG_ELEMMENT_START();

    int dataType;

    dataType = CWProtocolRetrieve8(msgPtr);

    //    valPtr->failed = CW_FALSE;
    if(dataType == 2)
    {
        valPtr->eof = CW_TRUE;
    }
    else
    {
        valPtr->eof = CW_FALSE;
    }
    valPtr->dataLen = len - 1;
    if(!valPtr->dataLen)
    {
        return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Invalid Cert Data Length");
    }

    valPtr->data = CWProtocolRetrieveRawBytes(msgPtr, valPtr->dataLen);
    if(!valPtr->data)
    {
        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
    }

    CW_PARSE_MSG_ELEMENT_END();
}

CWBool CWParseCaData(CWProtocolMessage *msgPtr, int len, CWACCaData *valPtr)
{
    CW_PARSE_MSG_ELEMMENT_START();

    int dataType;

    dataType = CWProtocolRetrieve8(msgPtr);

    //    valPtr->failed = CW_FALSE;
    if(dataType == 2)
    {
        valPtr->eof = CW_TRUE;
    }
    else
    {
        valPtr->eof = CW_FALSE;
    }
    valPtr->dataLen = len - 1;
    if(!valPtr->dataLen)
    {
        return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Invalid CA Data Length");
    }

    valPtr->data = CWProtocolRetrieveRawBytes(msgPtr, valPtr->dataLen);
    if(!valPtr->data)
    {
        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
    }

    CW_PARSE_MSG_ELEMENT_END();
}

CWBool CWAssembleMsgElemCertResetAction(CWProtocolMessage *msgPtr, CWCertResetAction *valPtr)
{
    if(msgPtr == NULL || valPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, "CWCertResetAction arguments error.");
    }

    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, 1, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if(*valPtr == CW_CERT_RESET_ACTION_NONE)
    {
        CWProtocolStore8(msgPtr, 0);
    }
    else if(*valPtr == CW_CERT_RESET_ACTION_FACTORY_RESET)
    {
        CWProtocolStore8(msgPtr, 1);
    }
    else if(*valPtr == CW_CERT_RESET_ACTION_CERT_UPDATE)
    {
        CWProtocolStore8(msgPtr, 2);
    }
    else
    {
        CWErrorRaise(CW_ERROR_NOT_SUPPORTED, "Not supported CWCertResetAction.");
    }

    return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_AC_CERT_RESET_ACTION_CW_TYPE);
}

CWBool CWParseCertResetAction(CWProtocolMessage *msgPtr, int len, CWCertResetAction *valPtr)
{
    CW_PARSE_MSG_ELEMMENT_START();

    int actType = CWProtocolRetrieve8(msgPtr);

    if(actType == 0)
    {
        *valPtr = CW_CERT_RESET_ACTION_NONE;
    }
    else if(actType == 1)
    {
        *valPtr = CW_CERT_RESET_ACTION_FACTORY_RESET;
    }
    else if(actType == 2)
    {
        *valPtr = CW_CERT_RESET_ACTION_CERT_UPDATE;
    }
    else
    {
        CWErrorRaise(CW_ERROR_WRONG_ARG, "Not supported CWCertResetAction.");
    }

    CW_PARSE_MSG_ELEMENT_END();
}

CWBool CWAssembleMsgElemCertInformation(CWProtocolMessage *msgPtr, CWCertInformation *certInfo)
{
    if(msgPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }
    if(certInfo == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    // create message
    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, 4 + 16 + 4 + 16, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    CWProtocolStore32(msgPtr, certInfo->caFileSize);
    CWProtocolStoreRawBytes(msgPtr, (char *)certInfo->caHash, 16);
    CWProtocolStore32(msgPtr, certInfo->certFileSize);
    CWProtocolStoreRawBytes(msgPtr, (char *)certInfo->certHash, 16);

    return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_AC_CERT_INFO_CW_TYPE);
}

CWBool CWParseCertInformation(CWProtocolMessage *msgPtr, int len, CWCertInformation *valPtr)
{
    CW_PARSE_MSG_ELEMMENT_START();

    valPtr->caFileSize = CWProtocolRetrieve32(msgPtr);
    CWProtocolRetrieveRawBytesNoCreate(msgPtr, (char *)valPtr->caHash, 16);
    valPtr->certFileSize = CWProtocolRetrieve32(msgPtr);
    CWProtocolRetrieveRawBytesNoCreate(msgPtr, (char *)valPtr->certHash, 16);

    CW_PARSE_MSG_ELEMENT_END();
}

CWBool CWAssembleMsgElemCaData(CWProtocolMessage *msgPtr, CWACCaData *caData)
{
    if(msgPtr == NULL || caData == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    if(caData->dataLen == 0 || caData->data == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, 1 + caData->dataLen, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
    if(caData->eof)
    {
        CWProtocolStore8(msgPtr, 2);
    }
    else
    {
        CWProtocolStore8(msgPtr, 1);
    }

    CWProtocolStoreRawBytes(msgPtr, caData->data, caData->dataLen);

    return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_AC_CA_DATA_CW_TYPE);
}

CWBool CWAssembleMsgElemCertData(CWProtocolMessage *msgPtr, CWACCertData *certData)
{
    if(msgPtr == NULL || certData == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    if(certData->dataLen == 0 || certData->data == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, 1 + certData->dataLen, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
    if(certData->eof)
    {
        CWProtocolStore8(msgPtr, 2);
    }
    else
    {
        CWProtocolStore8(msgPtr, 1);
    }

    CWProtocolStoreRawBytes(msgPtr, certData->data, certData->dataLen);

    return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_AC_CERT_DATA_CW_TYPE);
}

CWBool CWAssembleMsgElemIPv4Addr(CWProtocolMessage *msgPtr, unsigned int addr)
{
    if(msgPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    // create message
    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, 4, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    CWProtocolStoreIPv4Address(msgPtr, addr);

    return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_IPV4_ADDR_CW_TYPE);
}

CWBool CWAssembleMsgElemHostName(CWProtocolMessage *msgPtr, const CWHostName host)
{
    int len;

    if(msgPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    len = strlen(host);

    // create message
    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, 1 + len, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    /* The length of hostname should not exceed 255, 1 byte should be enough */
    CWProtocolStore8(msgPtr, (unsigned char) len);

    CWProtocolStoreRawBytes(msgPtr, host, len);

    return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_HOST_NAME_CW_TYPE);
}

CWBool CWAssembleMsgElemConnectionID(CWProtocolMessage *msgPtr, int id)
{
    if(msgPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    // create message
    CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, 4, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    CWProtocolStore32(msgPtr, id);

    return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_CONNECTION_ID);
}

CWBool CWAssembleMsgElemWaitApply(CWProtocolMessage *msgPtr)
{
    if(msgPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    msgPtr->offset = 0;

    return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_WAIT_APPLY);
}

CWBool CWParseHostName(CWProtocolMessage *msgPtr, int len, CWHostName valPtr)
{
    unsigned char hostLen;

    CW_PARSE_MSG_ELEMMENT_START();

    hostLen = CWProtocolRetrieve8(msgPtr);
    CWProtocolRetrieveRawBytesNoCreate(msgPtr, valPtr, hostLen);
    valPtr[hostLen] = '\0';

    CW_PARSE_MSG_ELEMENT_END();
}


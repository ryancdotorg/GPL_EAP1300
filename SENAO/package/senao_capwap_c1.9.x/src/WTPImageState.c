
#include "CWWTP.h"
#include "CWVendorPayloads.h"

#ifdef DMALLOC
#include "../dmalloc-5.5.0/dmalloc.h"
#endif
#define CW_IMG_PATH_SIZE 32
int gCWRebootTime = -1;
int gCWWaitImageData = CW_WTP_IMAGE_DATA_INTERVAL_DEFAULT;
int gCWImageEchoInterval = CW_IMAGE_ECHO_INTERVAL_DEFAULT;
char gCWImgPath[CW_IMG_PATH_SIZE] = {};
CWThreadMutex gWTPImageEchoDataMutex = CW_MUTEX_INITIALIZER;
CWTimerID gWTPImageEchoTimerID = -1;
CWBool gWTPSendImageEchoRequest = CW_FALSE;

CWBool CWAssembleImageEchoRequest(CWProtocolMessage **messagesPtr,
                                  int *fragmentsNumPtr,
                                  int PMTU,
                                  int seqNum,
                                  int waitSeconds);

void CWWTPImageEchoTimerExpiredHandler(void *arg)
{
    CWProtocolMessage *messages = NULL;
    int fragmentsNum;

    CWDebugLog("#________ Image Echo Request Timer Expired ________#");

    CWThreadMutexLock(&gWTPImageEchoDataMutex);

    if(!gWTPSendImageEchoRequest)
    {
        CWDebugLog("Cannot Send Image Echo Request Now");
        CWThreadMutexUnlock(&gWTPImageEchoDataMutex);
        return;
    }

    gWTPImageEchoTimerID = timer_add(gCWImageEchoInterval,
                                     0,
                                     CWWTPImageEchoTimerExpiredHandler,
                                     NULL);
    CWDebugLog("Image Echo Request Timer Restarted");

    if(!CWAssembleImageEchoRequest(&messages,
                                   &fragmentsNum,
                                   gWTPPathMTU,
                                   CWGetSeqNum(),
                                   0))
    {
        CWLog("CWAssembleImageEchoRequest failed");
        CWThreadMutexUnlock(&gWTPImageEchoDataMutex);
        return;
    }

    if(!CWWTPSendMessage(messages, fragmentsNum))
    {
        CWLog("CWWTPSendMessage failed");
        CWThreadMutexUnlock(&gWTPImageEchoDataMutex);
        return;
    }

    CWThreadMutexUnlock(&gWTPImageEchoDataMutex);
}

CWBool CWStartImageEchoTimer()
{
    CWThreadMutexLock(&gWTPImageEchoDataMutex);

    gWTPSendImageEchoRequest = CW_TRUE;
    gWTPImageEchoTimerID = timer_add(gCWImageEchoInterval,
                                     0,
                                     CWWTPImageEchoTimerExpiredHandler,
                                     NULL);
    if(gWTPImageEchoTimerID == -1)
    {
        CWLog("Image Echo Request Timer Add failed");
        CWThreadMutexUnlock(&gWTPImageEchoDataMutex);
        return CW_FALSE;
    }

    CWDebugLog("Image Echo Request Timer Started");

    CWThreadMutexUnlock(&gWTPImageEchoDataMutex);

    return CW_TRUE;
}

CWBool CWStopImageEchoTimer()
{
    CWThreadMutexLock(&gWTPImageEchoDataMutex);

    gWTPSendImageEchoRequest = CW_FALSE;
    if(gWTPImageEchoTimerID != -1)
    {
        timer_rem(gWTPImageEchoTimerID, NULL);
        gWTPImageEchoTimerID = -1;
    }

    CWDebugLog("Image Echo Request Timer Stopped");

    CWThreadMutexUnlock(&gWTPImageEchoDataMutex);

    return CW_TRUE;
}

CWBool CWAssembleImageEchoRequest(CWProtocolMessage **messagesPtr,
                                  int *fragmentsNumPtr,
                                  int PMTU,
                                  int seqNum,
                                  int waitSeconds)
{
    CWProtocolMessage *msgElems = NULL;
    const int msgElemCount = 1;
    CWProtocolMessage *msgElemsBinding = NULL;
    const int msgElemBindingCount = 0;

    if(messagesPtr == NULL || fragmentsNumPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems,
                                     msgElemCount,
                                     return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    CWDebugLog("Assembling Image Echo Request...");

    if(!CWAssembleMsgElemVendorPayloadWaitingTime(&(msgElems[0]), waitSeconds))
    {
        CWErrorHandleLast();
        CW_FREE_OBJECT(msgElems);
        /* error will be handled by the caller */
        return CW_FALSE;
    }

    if(!(CWAssembleMessage(messagesPtr,
                           fragmentsNumPtr,
                           PMTU,
                           seqNum,
                           CW_MSG_TYPE_VALUE_ECHO_REQUEST,
                           msgElems,
                           msgElemCount,
                           msgElemsBinding,
                           msgElemBindingCount,
                           CW_PACKET_CRYPT
                          )))
    {
        return CW_FALSE;
    }

    CWDebugLog("Echo Image Request Assembled");

    return CW_TRUE;
}

CWBool CWAssembleImageResponse(CWProtocolMessage **completeMsgPtr, int *fragmentsNumPtr,
                               int PMTU, int seqNum, CWWTPImageDataResponseValues *responseValues)
{
    CWProtocolMessage *msgElems = NULL;
    int msgElemCount = responseValues->waitSeconds ? 2 : 1;
    CWProtocolMessage *msgElemsBinding = NULL;
    const int msgElemBindingCount = 0;
    int k = -1;

    if(completeMsgPtr == NULL || fragmentsNumPtr == NULL || responseValues == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems,
                                     msgElemCount,
                                     return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    CWDebugLog("Assembling Image Data Response...");

    if(!(CWAssembleMsgElemResultCode(&(msgElems[++k]), responseValues->resultCode)) ||
       (responseValues->waitSeconds && !CWAssembleMsgElemVendorPayloadWaitingTime(&(msgElems[++k]), responseValues->waitSeconds)))
    {
        CWErrorHandleLast();
        int i;
        for(i = 0; i <= k; i++)
        {
            CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);
        }

        CW_FREE_OBJECT(msgElems);
        /* error will be handled by the caller */
        return CW_FALSE;
    }

    if(!(CWAssembleMessage(completeMsgPtr,
                           fragmentsNumPtr,
                           PMTU,
                           seqNum,
                           CW_MSG_TYPE_VALUE_IMAGE_DATA_RESPONSE,
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

CWBool CWAssembleImageRequest(CWProtocolMessage **completeMsgPtr, int *fragmentsNumPtr,
                              int PMTU, int seqNum, void *valuesPtr)
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

    CWDebugLog("Assembling Image Data Request...");

    if(!(CWAssembleMsgElemImageInitDownload(&(msgElems[++k]))) ||
       (!CWAssembleMsgElemImageIdentifier(&(msgElems[++k]), &gWTPImageId))
      )
    {
        CWErrorHandleLast();
        int i;
        for(i = 0; i <= k; i++)
        {
            CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);
        }
        CW_FREE_OBJECT(msgElems);
        /* error will be handled by the caller */
        return CW_FALSE;
    }

    if(!(CWAssembleMessage(completeMsgPtr,
                           fragmentsNumPtr,
                           PMTU,
                           seqNum,
                           CW_MSG_TYPE_VALUE_IMAGE_DATA_REQUEST,
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

CWBool CWAssembleRunUpgResponse(CWProtocolMessage **completeMsgPtr, int *fragmentsNumPtr,
                                int PMTU, int seqNum, int waitingTime)
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

    CWDebugLog("Assembling Run Upg Response...");

    if(!(CWAssembleMsgElemResultCode(&(msgElems[++k]), CW_PROTOCOL_SUCCESS)) ||
       !(CWAssembleMsgElemVendorPayloadWaitingTime(&(msgElems[++k]), waitingTime))
      )
    {
        CWErrorHandleLast();
        int i;
        for(i = 0; i <= k; i++)
        {
            CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);
        }

        CW_FREE_OBJECT(msgElems);
        /* error will be handled by the caller */
        return CW_FALSE;
    }

    if(!(CWAssembleMessage(completeMsgPtr,
                           fragmentsNumPtr,
                           PMTU,
                           seqNum,
                           CW_MSG_TYPE_VALUE_RUN_UPG_RESPONSE,
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

CWBool CWAssembleImageDownloadStatusRequest(CWProtocolMessage **completeMsgPtr, int *fragmentsNumPtr,
                                            int PMTU, int seqNum, CWWTPImageDownloadRequestValues *valuePtr)
{
    CWProtocolMessage *msgElems = NULL;
    int msgElemCount = valuePtr->waitSeconds ? 2 : 1;
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

    CWDebugLog("Assembling Image Download Status Request...");

    if(!CWAssembleMsgElemImageDownloadStatus(&(msgElems[++k]), &(valuePtr->status)) ||
       (valuePtr->waitSeconds && !CWAssembleMsgElemVendorPayloadWaitingTime(&(msgElems[++k]), valuePtr->waitSeconds))
      )
    {
        CWErrorHandleLast();
        int i;
        for(i = 0; i <= k; i++)
        {
            CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);
        }

        CW_FREE_OBJECT(msgElems);
        /* error will be handled by the caller */
        return CW_FALSE;
    }

    if(!(CWAssembleMessage(completeMsgPtr,
                           fragmentsNumPtr,
                           PMTU,
                           seqNum,
                           CW_MSG_TYPE_VALUE_IMAGE_DOWNLOAD_STATUS_REQUEST,
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

CWBool CWParseImageRequestMessage(char *msg, int len, int *seqNumPtr, CWACImageDataRequestValues *valuesPtr)
{
    CWControlHeaderValues controlVal;
    int offsetTillMessages;
    unsigned short int elemType;
    unsigned int elemLen;
    CWProtocolMessage completeMsg;
    CWBool imgDataGot = CW_FALSE;
    CWProtocolVendorSpecificValues vendorValues;

    if(msg == NULL || seqNumPtr == NULL || valuesPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    CWDebugLog("Parse Image Data Request");

    completeMsg.msg = msg;
    completeMsg.offset = 0;

    if(!(CWParseControlHeader(&completeMsg, &controlVal)))
        /* will be handled by the caller */
    {
        return CW_FALSE;
    }

    /* different type */
    if(controlVal.messageTypeValue != CW_MSG_TYPE_VALUE_IMAGE_DATA_REQUEST)
    {
        return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Message is not Image Data Request as Expected");
    }

    *seqNumPtr = controlVal.seqNum;
    /* skip timestamp */
    controlVal.msgElemsLen -= CW_CONTROL_HEADER_OFFSET_FOR_MSG_ELEMS;
    offsetTillMessages = completeMsg.offset;

    CW_ZERO_MEMORY(&vendorValues, sizeof(vendorValues));
    CW_ZERO_MEMORY(valuesPtr, sizeof(CWACImageDataRequestValues));

    /* parse message elements */
    while((completeMsg.offset - offsetTillMessages) < controlVal.msgElemsLen)
    {
        CWParseFormatMsgElem(&completeMsg, &elemType, &elemLen);

        /* CWDebugLog("Parsing Message Element: %u, elemLen: %u", elemType, elemLen); */

        switch(elemType)
        {
            case CW_MSG_ELEMENT_IMAGE_DATA_CW_TYPE:
                if(!(CWParseImageData(&completeMsg, elemLen, &(valuesPtr->imageData))))
                {
                    return CW_FALSE;
                }
                imgDataGot = CW_TRUE;
                break;
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
                    valuesPtr->wait = (int)(long) vendorValues.payload;
                }
                break;
            case CW_MSG_ELEMENT_UPG_FLOW:
                if(!(CWParseUpgFlow(&completeMsg, elemLen, &(valuesPtr->upgFlow))))
                {
                    return CW_FALSE;
                }
                break;
            default:
                completeMsg.offset += elemLen;
                CWLog("Unrecognized Message Element %d in Image Data Request", elemType);
        }
        /*CWDebugLog("bytes: %d/%d", (completeMsg.offset-offsetTillMessages), controlVal.msgElemsLen);*/
    }

    if(completeMsg.offset != len)
    {
        return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Garbage at the End of the Message");
    }

    if(!imgDataGot)
    {
        return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Require CW_MSG_ELEMENT_IMAGE_DATA_CW_TYPE in Image Data Request.");
    }

    return CW_TRUE;
}

CWBool CWParseImageResponseMessage(char *msg, int len, int seqNum, CWACImageDataResponseValues *valuesPtr, int combineLen)
{
    CWControlHeaderValues controlVal;
    int offsetTillMessages;
    unsigned short int elemType;
    unsigned int elemLen;
    CWProtocolMessage completeMsg;
    CWBool resultCodeGot = CW_FALSE;
    CWBool imgInfoGot = CW_FALSE;

    if(msg == NULL || valuesPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    CWDebugLog("Parse Image Data Response");

    completeMsg.msg = msg;
    completeMsg.offset = 0;

    if(!(CWParseControlHeader(&completeMsg, &controlVal)))
        /* will be handled by the caller */
    {
        return CW_FALSE;
    }

    /* different type */
    if(controlVal.messageTypeValue != CW_MSG_TYPE_VALUE_IMAGE_DATA_RESPONSE)
    {
        return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Message is not Image Data Response as Expected");
    }

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
        CWParseFormatMsgElem(&completeMsg, &elemType, &elemLen);

        CWDebugLog("Parsing Message Element: %u, elemLen: %u", elemType, elemLen);

        switch(elemType)
        {
            case CW_MSG_ELEMENT_RESULT_CODE_CW_TYPE:
                if(!(CWParseResultCode(&completeMsg, elemLen, &(valuesPtr->resultCode))))
                {
                    return CW_FALSE;
                }
                resultCodeGot = CW_TRUE;
                break;
            case CW_MSG_ELEMENT_IMAGE_INFO_CW_TYPE:
                if(!(CWParseImageInformation(&completeMsg, elemLen, &(valuesPtr->imageInfo))))
                {
                    return CW_FALSE;
                }
                imgInfoGot = CW_TRUE;
                break;
            default:
                completeMsg.offset += elemLen;
                CWLog("Unrecognized Message Element %d in Image Data Response", elemType);
        }
        /*CWDebugLog("bytes: %d/%d", (completeMsg.offset-offsetTillMessages), controlVal.msgElemsLen);*/
    }

    if(completeMsg.offset != len)
    {
        return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Garbage at the End of the Message");
    }

    if(!resultCodeGot)
    {
        return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Require CW_MSG_ELEMENT_RESULT_CODE_CW_TYPE in Image Data Response.");
    }

    if(!imgInfoGot)
    {
        return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Require CW_MSG_ELEMENT_IMAGE_INFO_CW_TYPE in Image Data Response.");
    }

    return CW_TRUE;
}

CWBool CWParseImageEchoResponseMessage(char *msg)
{
    CWControlHeaderValues controlVal;
    CWProtocolMessage completeMsg;

    if(msg == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    CWDebugLog("Parse Image Echo Response");

    completeMsg.msg = msg;
    completeMsg.offset = 0;

    if(!(CWParseControlHeader(&completeMsg, &controlVal)))
        /* will be handled by the caller */
    {
        return CW_FALSE;
    }

    /* different type */
    if(controlVal.messageTypeValue != CW_MSG_TYPE_VALUE_ECHO_RESPONSE)
    {
        return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Message is not Image Echo Response as Expected");
    }

    return CW_TRUE;
}

CWBool CWParseRunUpgRequestMessage(char *msg, int *seqNumPtr)
{
    CWControlHeaderValues controlVal;
    CWProtocolMessage completeMsg;

    if(msg == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    CWDebugLog("Parse Run Upg Request");

    completeMsg.msg = msg;
    completeMsg.offset = 0;

    if(!(CWParseControlHeader(&completeMsg, &controlVal)))
        /* will be handled by the caller */
    {
        return CW_FALSE;
    }

    /* different type */
    if(controlVal.messageTypeValue != CW_MSG_TYPE_VALUE_RUN_UPG_REQUEST)
    {
        return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Message is not Run Upg Request as Expected");
    }

    *seqNumPtr = controlVal.seqNum;

    return CW_TRUE;
}

CWBool CWParseImageDownloadStatusResponseMessage(char *msg, int len, int seqNum, CWACImageDownloadReponseValues *valuesPtr, int combineLen)
{
    CWControlHeaderValues controlVal;
    CWProtocolMessage completeMsg;
    int offsetTillMessages;
    unsigned short int elemType;
    unsigned int elemLen;

    if(msg == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    CWDebugLog("Parse Image Download Status Response");

    completeMsg.msg = msg;
    completeMsg.offset = 0;

    if(!(CWParseControlHeader(&completeMsg, &controlVal)))
        /* will be handled by the caller */
    {
        return CW_FALSE;
    }

    /* different type */
    if(controlVal.messageTypeValue != CW_MSG_TYPE_VALUE_IMAGE_DOWNLOAD_STATUS_RESPONSE)
    {
        return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Message is not Run Upg Request as Expected");
    }

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

    CW_ZERO_MEMORY(valuesPtr, sizeof(CWACImageDownloadReponseValues));

    /* parse message elements */
    while((completeMsg.offset - offsetTillMessages) < controlVal.msgElemsLen)
    {
        CWParseFormatMsgElem(&completeMsg, &elemType, &elemLen);

        CWDebugLog("Parsing Message Element: %u, elemLen: %u", elemType, elemLen);

        switch(elemType)
        {
            case CW_MSG_ELEMENT_UPG_FLOW:
                if(!(CWParseUpgFlow(&completeMsg, elemLen, &(valuesPtr->upgFlow))))
                {
                    return CW_FALSE;
                }
                break;
            default:
                completeMsg.offset += elemLen;
                CWLog("Unrecognized Message Element %d in Image Download Status Response", elemType);
        }
        /*CWDebugLog("bytes: %d/%d", (completeMsg.offset-offsetTillMessages), controlVal.msgElemsLen);*/
    }

    return CW_TRUE;
}

CWBool CWSaveImageResponseMessage(CWACImageDataResponseValues *valuesPtr)
{
    return CW_TRUE;
}

CWBool CWSaveImageDownloadStatusResponseMessage(CWACImageDownloadReponseValues *valuesPtr)
{
    return CW_TRUE;
}

/*
 * Manage Join State.
 */
CWStateTransition CWWTPEnterImage()
{
    int seqNum;
    CWACImageDataResponseValues responseValuesAc;
    CWWTPImageDataResponseValues responseValuesWtp;
    CWACImageDataRequestValues requestValuesAc;
    FILE *fp = NULL;
    char imgTmpFile[CW_IMG_PATH_SIZE];
    CWProtocolMessage *messages = NULL;
    CWProtocolMessage msg;
    int fragmentsNum = 0, ret, fsize, waitingTime = 0;
    unsigned char md5Hash[16];
    CWUpgFlow upgFlow = CW_UPG_ONE_STEP;
#ifndef CW_BUILD_X86
    CWBool debugLogOrg = gEnabledDebugLog;
#endif

    CWDebugLog("######### Image Data State #########");

    sprintf(imgTmpFile, "/tmp/wtpimg.%x.tmp", getpid());

    /* send Configure Request */
    seqNum = CWGetSeqNum();

    if(!CWErr(CWWTPSendAcknowledgedPacket(seqNum,
                                          NULL,
                                          CWAssembleImageRequest,
                                          (void *)CWParseImageResponseMessage,
                                          (void *)CWSaveImageResponseMessage, &responseValuesAc,
                                          gACInfoPtr->timer->imageState)))
    {
        return CW_ENTER_DISCOVERY;
    }

    if(responseValuesAc.resultCode != CW_PROTOCOL_SUCCESS)
    {
        CWLog("AC Image Data Response resultCode=%d", responseValuesAc.resultCode);

        if(gWTPImageId.necessary)
        {
            return CW_ENTER_DISCOVERY;
        }
        else if(gWTPPreviousState == CW_ENTER_JOIN)
        {
            return CW_ENTER_CONFIGURE;
        }
        else
        {
            return CW_ENTER_RUN;
        }
    }

    if(!strncmp(gWTPImageId.imageName, "http", 4))
    {
        char logFile[32];
        char pidFile[32];
        char *pid = NULL;
        char procFile[32];
        int pollInt = 200000;  /* check wget status every 200ms */
        int statusInt = 0;
        CWWTPImageDownloadRequestValues reqValues;
        CWACImageDownloadReponseValues respValues;

        sprintf(logFile, "/tmp/wtp_wget.%x.log", getpid());
        sprintf(pidFile, "/tmp/wtp_wget.%x.pid", getpid());

        reqValues.status.end = CW_FALSE;
        reqValues.status.resultCode = CW_PROTOCOL_SUCCESS;
        reqValues.waitSeconds = 0;

        CWLog("Start downloading image from %s", gWTPImageId.imageName);

        unlink(imgTmpFile);
        CWSystem("wget '%s' -O %s > %s 2>&1 & echo $! > %s",
                 gWTPImageId.imageName, imgTmpFile, logFile, pidFile);

        ret = CWCreateStringByFile(pidFile, &pid);
        unlink(pidFile);

        if(!ret || pid[0] == '\0')
        {
            return CW_ENTER_DISCOVERY;
        }

        pid[strlen(pid) - 1] = '\0'; /* remove '\n' */
        sprintf(procFile, "/proc/%s/cmdline", pid);

        do
        {
            CWWaitMicroSec(pollInt);
            statusInt += pollInt;

            fp = fopen(procFile, "r");
            if(fp)
            {
                fclose(fp);
                fp = NULL;
            }
            else
            {
                reqValues.status.end = CW_TRUE;
            }

            if(reqValues.status.end || statusInt >= 5000000)
            {
                if(!CWGetFileSize(imgTmpFile, &reqValues.status.size))
                {
                    reqValues.status.size = 0;
                }

                CWDebugLog("Image size %d/%d", reqValues.status.size, responseValuesAc.imageInfo.fileSize);

                if(reqValues.status.end)
                {
                    if(!CWGetFileMD5(imgTmpFile, md5Hash) ||
                       memcmp(responseValuesAc.imageInfo.hash, md5Hash, 16))
                    {
                        CWLog("Image MD5 Sum not match, downloaded size %d/%d",
                              reqValues.status.size, responseValuesAc.imageInfo.fileSize);
                        reqValues.status.resultCode = !reqValues.status.size ? CW_PROTOCOL_FAILURE_OTHER_ERROR :
                                                      responseValuesAc.imageInfo.fileSize != reqValues.status.size ?
                                                      CW_PROTOCOL_FAILURE_INVALID_DATA_LEN : CW_PROTOCOL_FAILURE_INVALID_CHECKSUM;
                        unlink(imgTmpFile);
                    }
                    else
                    {
                        reqValues.waitSeconds = CWWTPBoardGetImageBurningTime(imgTmpFile);
                        waitingTime = reqValues.waitSeconds;
                    }
                }

                if(!CWErr(CWWTPSendAcknowledgedPacket(CWGetSeqNum(),
                                                      (void *) &reqValues,
                                                      (void *) CWAssembleImageDownloadStatusRequest,
                                                      (void *) CWParseImageDownloadStatusResponseMessage,
                                                      (void *) CWSaveImageDownloadStatusResponseMessage, &respValues,
                                                      gACInfoPtr->timer->imageState)))
                {
                    unlink(imgTmpFile);
                    return CW_ENTER_DISCOVERY;
                }

                statusInt = 0;
            }
        }
        while(!reqValues.status.end);

        if(reqValues.status.resultCode != CW_PROTOCOL_SUCCESS)
        {
            goto image_upgrade_stop;
        }

        upgFlow = respValues.upgFlow;
    }
    else
    {
        /* Start Receiving image */
        CWLog("Start receiving image from AC...");

#ifndef CW_BUILD_X86
        /* Disable debug message or it will take very long time to receive image*/
        gEnabledDebugLog = 0;
#endif

        CW_REPEAT_FOREVER
        {
            if(!CWWTPReceiveMessage(&msg, gACInfoPtr->timer->imageData) ||
            !CWParseImageRequestMessage(msg.msg, msg.offset, &seqNum, &requestValuesAc))
            {
                CW_FREE_PROTOCOL_MESSAGE(msg);

                if(CWErrorGetLastErrorCode() == CW_ERROR_INVALID_FORMAT)
                {
                    continue;
                }
                else
                {
                    CWErrorHandleLast();
                    goto image_upgrade_fatal_error;
                }
            }

            if(!CWWTPUpdateRequestSeqNum(seqNum))
            {
                /* resend the response if we get last request again */
                if(seqNum == gWTPLastReqSeqNum)
                {
                    CWLog("Resend the response of last Image Data Request");

                    responseValuesWtp.resultCode = CW_PROTOCOL_SUCCESS;
                    responseValuesWtp.waitSeconds = 0;

                    if(!(CWAssembleImageResponse(&messages,
                                                 &fragmentsNum,
                                                 gWTPPathMTU,
                                                 seqNum,
                                                 &responseValuesWtp)))
                    {
                        CW_FREE_PROTOCOL_MESSAGE(msg);
                        goto image_upgrade_fatal_error;
                    }

                    if(!CWWTPSendMessage(messages, fragmentsNum))
                    {
                        CW_FREE_PROTOCOL_MESSAGE(msg);
                        goto image_upgrade_fatal_error;
                    }
                }

                CW_FREE_PROTOCOL_MESSAGE(msg);
                continue;
            }

            if(requestValuesAc.imageData.failed)
            {
                CW_FREE_PROTOCOL_MESSAGE(msg);
                goto image_upgrade_stop;
            }

            responseValuesWtp.resultCode = CW_PROTOCOL_SUCCESS;
            responseValuesWtp.waitSeconds = 0;

            /* Write image data to tmp file */
            if(!fp)
            {
                fp = fopen(imgTmpFile, "wb+");
                if(!fp)
                {
                    responseValuesWtp.resultCode = CW_PROTOCOL_FAILURE_FIRM_WRT_ERROR;
                }
            }

            if(fp)
            {
                ret = fwrite(requestValuesAc.imageData.data, 1, requestValuesAc.imageData.dataLen, fp);
                if(ret != requestValuesAc.imageData.dataLen)
                {
                    responseValuesWtp.resultCode = CW_PROTOCOL_FAILURE_FIRM_WRT_ERROR;
                }

                if(requestValuesAc.imageData.eof)
                {
#ifndef CW_BUILD_X86
                    gEnabledDebugLog = debugLogOrg;
#endif
                    fclose(fp);
                    fp = NULL;

                    CWDebugLog("Verifying Image Data...");

                    if(!CWGetFileSize(imgTmpFile, &fsize) ||
                       responseValuesAc.imageInfo.fileSize != fsize)
                    {
                        CWLog("File size not match");
                        responseValuesWtp.resultCode = CW_PROTOCOL_FAILURE_INVALID_DATA_LEN;
                        unlink(imgTmpFile);
                    }
                    else if(!CWGetFileMD5(imgTmpFile, md5Hash) ||
                            memcmp(responseValuesAc.imageInfo.hash, md5Hash, 16))
                    {
                        CWLog("File MD5 not match");
                        responseValuesWtp.resultCode = CW_PROTOCOL_FAILURE_INVALID_CHECKSUM;
                        unlink(imgTmpFile);
                    }
                    else
                    {
                        CWDebugLog("Image Data OK");
                        responseValuesWtp.waitSeconds = CWWTPBoardGetImageBurningTime(imgTmpFile);
                        CWDebugLog("Burning image will take %u seconds", responseValuesWtp.waitSeconds);
                    }
                }
            }

            /* Note: Free msg after writing data to tmp file */
            CW_FREE_PROTOCOL_MESSAGE(msg);

            if(requestValuesAc.wait)
            {
#ifndef CW_BUILD_X86
                if(debugLogOrg)
                {
                    gEnabledDebugLog = 1;
                }
#endif
                CWDebugLog("Waiting %d seconds before next response", requestValuesAc.wait);
                CWWaitSec(requestValuesAc.wait);
#ifndef CW_BUILD_X86
                if(debugLogOrg)
                {
                    gEnabledDebugLog = 0;
                }
#endif
            }

            /* Send response */
            if(!(CWAssembleImageResponse(&messages,
                                         &fragmentsNum,
                                         gWTPPathMTU,
                                         seqNum,
                                         &responseValuesWtp)))
            {
                goto image_upgrade_fatal_error;
            }

            if(!CWWTPSendMessage(messages, fragmentsNum))
            {
                goto image_upgrade_fatal_error;
            }

            if(responseValuesWtp.resultCode != CW_PROTOCOL_SUCCESS)
            {
                goto image_upgrade_stop;
            }

            if(requestValuesAc.imageData.eof)
            {
                upgFlow = requestValuesAc.upgFlow;
                waitingTime = responseValuesWtp.waitSeconds;
                break;
            }
        }
    }

    if(upgFlow == CW_UPG_TWO_STEP)
    {
        CWLog("Waiting Run Upg Request...");

        CWStartImageEchoTimer();

        while(1)
        {
            if(!CWWTPReceiveMessage(&msg, CW_IMAGE_ECHO_TIMEOUT(gCWImageEchoInterval)))
            {
                if(CWErrorGetLastErrorCode() == CW_ERROR_INVALID_FORMAT)
                {
                    continue;
                }

                CWStopImageEchoTimer();
                goto image_upgrade_fatal_error;
            }

            if(CWParseRunUpgRequestMessage(msg.msg, &seqNum))
            {
                CW_FREE_PROTOCOL_MESSAGE(msg);
                CWStopImageEchoTimer();

                /* Send response */
                if(!(CWAssembleRunUpgResponse(&messages,
                                              &fragmentsNum,
                                              gWTPPathMTU,
                                              seqNum,
                                              waitingTime + 5)))
                {
                    goto image_upgrade_fatal_error;
                }

                if(!CWWTPSendMessage(messages, fragmentsNum))
                {
                    goto image_upgrade_fatal_error;
                }

                CWWaitSec(5); /* wait a while before burning */
                break;
            }

            CW_FREE_PROTOCOL_MESSAGE(msg);
        }
    }

    /* Start Burning */
    CWLog("Start Burning Image..");

    int waitRebootSec = CWWTPBoardGetImageRebootTime(imgTmpFile);

    if(!CWWTPBoardBurnImage(imgTmpFile))
    {
        CWLog("Burning Image Failed");
        unlink(imgTmpFile);
        return CW_ENTER_DISCOVERY;
    }
    unlink(imgTmpFile);

    /* Start Burning */
    CWLog("Burning Image Done");

    if(!CWAssembleImageEchoRequest(&messages,
                                   &fragmentsNum,
                                   gWTPPathMTU,
                                   CWGetSeqNum(),
                                   waitRebootSec))
    {
        goto image_upgrade_fatal_error;
    }

    if(!CWWTPSendMessage(messages, fragmentsNum))
    {
        goto image_upgrade_fatal_error;
    }

    while(1)
    {
        if(!CWWTPReceiveMessage(&msg, gACInfoPtr->timer->retransmitInterval) ||
           !CWParseImageEchoResponseMessage(msg.msg))
        {
            if(CWErrorGetLastErrorCode() == CW_ERROR_INVALID_FORMAT)
            {
                CW_FREE_PROTOCOL_MESSAGE(msg);
                continue;
            }
            /* reboot anyway */
        }

        CW_FREE_PROTOCOL_MESSAGE(msg);
        break;
    }

    return CW_ENTER_RESET;

image_upgrade_stop:

#ifndef CW_BUILD_X86
    gEnabledDebugLog = debugLogOrg;
#endif

    if(fp)
    {
        fclose(fp);
        unlink(imgTmpFile);
    }

    if(gWTPImageId.necessary)
    {
        return CW_ENTER_DISCOVERY;
    }

    if(gWTPPreviousState == CW_ENTER_JOIN)
    {
        return CW_ENTER_CONFIGURE;
    }
    else
    {
        return CW_ENTER_RUN;
    }

image_upgrade_fatal_error:

#ifndef CW_BUILD_X86
    gEnabledDebugLog = debugLogOrg;
#endif

    if(fp)
    {
        fclose(fp);
        unlink(imgTmpFile);
    }

    return CW_ENTER_DISCOVERY;
}


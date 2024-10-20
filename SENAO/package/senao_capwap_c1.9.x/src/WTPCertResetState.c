#include "CWWTP.h"

#ifdef DMALLOC
#include "../dmalloc-5.5.0/dmalloc.h"
#endif

static CWBool CWWTPCopyCertFile(const char *fileNameSrc, const char *fileNameDst)
{
    char ch;
    FILE *source, *target;
    char filePathSource[PATH_MAX], filePathTarget[PATH_MAX];

    sprintf(filePathSource, "%s%s", gWTPCertDir, fileNameSrc);
    sprintf(filePathTarget, "%s%s", gWTPCertDir, fileNameDst);

    source = fopen(filePathSource, "r");
    if(source == NULL)
    {
        CWLog("Open cert file %s failed.", fileNameSrc);
        return CW_FALSE;
    }

    target = fopen(filePathTarget, "w");
    if(target == NULL)
    {
        fclose(source);
        CWLog("Create cert file %s failed.", fileNameDst);
        return CW_FALSE;
    }

    while((ch = fgetc(source)) != EOF)
    {
        fputc(ch, target);
    }

    fclose(source);
    fclose(target);

    return CW_TRUE;
}

static void CWWTPDelCertFile(const char *fileName)
{
    char filePath[PATH_MAX];

    sprintf(filePath, "%s%s", gWTPCertDir, fileName);
    remove(filePath);
}

CWBool CWWTPSaveCertFile(const char *fileName, const char *certData, int dataSize)
{
    char filePath[PATH_MAX];
    FILE *fp = NULL;
    int ret;

    sprintf(filePath, "%s%s", gWTPCertDir, fileName);

    fp = fopen(filePath, "w");
    if(fp == NULL)
    {
        CWLog("Create new file %s failed", filePath);
        return CW_FALSE;
    }

    ret = fwrite(certData, 1, dataSize, fp);
    if(ret != dataSize)
    {
        CWLog("Save %s file failed, ret = %d", filePath, ret);
        fclose(fp);
        unlink(filePath);
        return CW_FALSE;
    }

    fclose(fp);

    return CW_TRUE;
}

CWBool CWWTPResetCert()
{
    /* reset current WTP ca */
    if(!CWWTPCopyCertFile(CW_WTP_CA_NAME_DEFAULT, CW_WTP_CA_NAME_CURRENT))
    {
        return CW_FALSE;
    }

    /* reset current WTP cert */
    if(!CWWTPCopyCertFile(CW_WTP_CERT_NAME_DEFAULT, CW_WTP_CERT_NAME_CURRENT))
    {
        return CW_FALSE;
    }

    /* reset current WTP cert key file */
    if(!CWWTPCopyCertFile(CW_WTP_PRIVATE_KEY_DEFAULT, CW_WTP_PRIVATE_KEY_CURRENT))
    {
        return CW_FALSE;
    }

    return CW_TRUE;
}

static CWBool CWWTPVerifyCertData(const char *certData, int dataSize, unsigned char hash[])
{
    char filePath[PATH_MAX];
    unsigned char md5_hash[16];
    FILE *fp = NULL;
    int ret;

    /* save to tmp path for verification */
    sprintf(filePath, "/tmp/cert.%u.tmp", getpid());
    fp = fopen(filePath, "w");
    if(fp == NULL)
    {
        CWLog("Create new file \"%s\" failed", filePath);
        return CW_FALSE;
    }

    ret = fwrite(certData, 1, dataSize, fp);
    if(ret != dataSize)
    {
        CWLog("Save file %s failed, ret = %d", filePath, ret);
        fclose(fp);
        unlink(filePath);
        return CW_FALSE;
    }

    fclose(fp);

    /* check if md5 is matched */
    if(!CWGetFileMD5(filePath, md5_hash) || memcmp(hash, md5_hash, 16))
    {
        unlink(filePath);
        return CW_FALSE;
    }

    unlink(filePath);

    return CW_TRUE;
}

void CWWTPCheckCurrentCertIsDefault()
{
    char fileCurrent[PATH_MAX], fileDefault[PATH_MAX];

    sprintf(fileCurrent, "%s%s", gWTPCertDir, CW_WTP_CERT_NAME_CURRENT);
    sprintf(fileDefault, "%s%s", gWTPCertDir, CW_WTP_CERT_NAME_DEFAULT);

    gWTPDefaultCert = CWCheckFileEqual(fileCurrent, fileDefault);

    if(gWTPDefaultCert)
    {
        sprintf(fileCurrent, "%s%s", gWTPCertDir, CW_WTP_CA_NAME_CURRENT);
        sprintf(fileDefault, "%s%s", gWTPCertDir, CW_WTP_CA_NAME_DEFAULT);
        gWTPDefaultCert = CWCheckFileEqual(fileCurrent, fileDefault);
    }
}

/* 2014/01/02 Sigma: Used to make sure current certificate and CA is available, if not, reset to default */
void CWWTPCheckCurrentCertAvailable()
{
    char currentPath[PATH_MAX];
    FILE *fp;

    sprintf(currentPath, "%s%s", gWTPCertDir, CW_WTP_CA_NAME_CURRENT);
    fp = fopen(currentPath, "r");
    if(fp == NULL)
    {
        CWLog("Current CA does not exist, reset all cert files to default");
        CWWTPResetCert();
        gWTPDefaultCert = CW_TRUE;
        return;
    }

    fclose(fp);

    sprintf(currentPath, "%s%s", gWTPCertDir, CW_WTP_CERT_NAME_CURRENT);
    fp = fopen(currentPath, "r");
    if(fp == NULL)
    {
        CWLog("Current Cert does not exist, reset all cert files to default");
        CWWTPResetCert();
        gWTPDefaultCert = CW_TRUE;
        return;
    }

    fclose(fp);
}

/*
CWBool CWWTPAppendKey2CertFile(const char* fileNameKey, const char* fileNameCert)
{
    char filePathKey[64], filePathCert[64];
    int fileSizeKey = 0;
    FILE* fpKey=NULL;
    FILE* fpCert=NULL;

    sprintf(filePathKey, "%s%s", gWTPCertDir, fileNameKey);
    sprintf(filePathCert, "%s%s", gWTPCertDir, fileNameCert);

    if(!CWGetFileSize(filePathKey, &fileSizeKey))
        return CW_FALSE;

    fpKey = fopen(filePathKey, "r");
    if(!fpKey)
        return CW_FALSE;

    fpCert = fopen(filePathCert, "a+");
    if(!fpCert)
        return CW_FALSE;

    char* bufferKey;
    CW_CREATE_OBJECT_SIZE_ERR(bufferKey, fileSizeKey, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    int readResult=0;
    readResult = fread(bufferKey, sizeof(char), fileSizeKey, fpKey);

    if(readResult!= fileSizeKey)
    {
        CWDebugLog("Read private key error: read=%d, expected=%d", readResult, fileSizeKey);
        fclose(fpKey);
        fclose(fpCert);
        CW_FREE_OBJECT(bufferKey);
        return CW_FALSE;
    }

    fwrite(bufferKey, sizeof(char), fileSizeKey, fpCert);

    fclose(fpKey);
    fclose(fpCert);
    CW_FREE_OBJECT(bufferKey);

    return CW_TRUE;

}
*/

CWBool CWParseCertResetRequestMessage(char *msg, int len, int *seqNumPtr, CWACNewCertResponseValues *valuesPtr)
{
    CWControlHeaderValues controlVal;
    int offsetTillMessages;
    unsigned short int elemType;
    unsigned int elemLen;
    CWProtocolMessage completeMsg;
    CWBool caDataGot = CW_FALSE;
    CWBool certDataGot = CW_FALSE;

    if(msg == NULL || seqNumPtr == NULL || valuesPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    CWDebugLog("Parse CA & Cert Data Request");

    completeMsg.msg = msg;
    completeMsg.offset = 0;

    if(!(CWParseControlHeader(&completeMsg, &controlVal)))
        /* will be handled by the caller */
    {
        return CW_FALSE;
    }

    /* different type */
    if(controlVal.messageTypeValue != CW_MSG_TYPE_VALUE_CERT_RESET_REQUEST)
    {
        return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Message is not Cert Reset Request as Expected");
    }

    *seqNumPtr = controlVal.seqNum;
    /* skip timestamp */
    controlVal.msgElemsLen -= CW_CONTROL_HEADER_OFFSET_FOR_MSG_ELEMS;
    offsetTillMessages = completeMsg.offset;

    /* parse message elements */
    while((completeMsg.offset - offsetTillMessages) < controlVal.msgElemsLen)
    {
        CWParseFormatMsgElem(&completeMsg, &elemType, &elemLen);

        /* CWDebugLog("Parsing Message Element: %u, elemLen: %u", elemType, elemLen); */

        switch(elemType)
        {
            case CW_MSG_ELEMENT_AC_CA_DATA_CW_TYPE:
                if(!(CWParseCaData(&completeMsg, elemLen, &(valuesPtr->caData))))
                {
                    return CW_FALSE;
                }

                CWDebugLog("Get CA file length=%d, data:%s", valuesPtr->caData.dataLen, valuesPtr->caData.data);
                caDataGot = CW_TRUE;
                break;
            case CW_MSG_ELEMENT_AC_CERT_DATA_CW_TYPE:
                if(!(CWParseCertData(&completeMsg, elemLen, &(valuesPtr->certData))))
                {
                    return CW_FALSE;
                }

                CWDebugLog("Get Cert file length=%d, data:%s", valuesPtr->certData.dataLen, valuesPtr->certData.data);
                certDataGot = CW_TRUE;
                break;
            default:
                completeMsg.offset += elemLen;
                CWLog("Unrecognized Message Element Type %d in Cert Reset Request Message", elemType);
        }
        /*CWDebugLog("bytes: %d/%d", (completeMsg.offset-offsetTillMessages), controlVal.msgElemsLen);*/
    }

    if(completeMsg.offset != len)
    {
        return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Garbage at the End of the Message");
    }

    if(!caDataGot || !certDataGot)
    {
        return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Require Ca & Cert in Cert Reset Request.");
    }

    return CW_TRUE;
}

CWBool CWParseCertResetResponseMessage(char *msg, int len, int seqNum, CWACCertResetResponseValues *valuesPtr,int combineLen)
{
    CWControlHeaderValues controlVal;
    int offsetTillMessages;
    unsigned short int elemType;
    unsigned int elemLen;
    CWProtocolMessage completeMsg;
    CWBool resultCodeGot = CW_FALSE;
    CWBool certActionGot = CW_FALSE;
    //CWBool certInfoGot = CW_FALSE;

    if(msg == NULL || valuesPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    CWDebugLog("Parse Cert Data Response");

    completeMsg.msg = msg;
    completeMsg.offset = 0;

    if(!(CWParseControlHeader(&completeMsg, &controlVal)))
        /* will be handled by the caller */
    {
        return CW_FALSE;
    }

    /* different type */
    if(controlVal.messageTypeValue != CW_MSG_TYPE_VALUE_CERT_RESET_RESPONSE)
    {
        return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Message is not Cert reset Response as Expected");
    }


    if(controlVal.seqNum != seqNum)
    {
        return CWErrorRaise(CW_ERROR_INVALID_FORMAT,
                            "Different Sequence Number");
    }

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
            case CW_MSG_ELEMENT_AC_CERT_RESET_ACTION_CW_TYPE:
                if(!(CWParseCertResetAction(&completeMsg, elemLen, &(valuesPtr->certAction))))
                {
                    return CW_FALSE;
                }
                CWDebugLog("Received AC certAction:%d", valuesPtr->certAction);
                certActionGot = CW_TRUE;
                break;
            default:
                completeMsg.offset += elemLen;
                CWLog("Unrecognized Message Element Type %d in Cert Data Response", elemType);
        }
        /*CWDebugLog("bytes: %d/%d", (completeMsg.offset-offsetTillMessages), controlVal.msgElemsLen);*/
    }

    if(completeMsg.offset != len)
    {
        return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Garbage at the End of the Message");
    }

    if(!resultCodeGot)
    {
        return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Require CW_MSG_ELEMENT_RESULT_CODE_CW_TYPE in Cert Reset Response.");
    }

    if(!certActionGot)
    {
        return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Require CW_MSG_ELEMENT_AC_CERT_RESET_ACTION_CW_TYPE in Cert Reset Response.");
    }

    /*
        if(!certInfoGot)
        {
            return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Require CW_MSG_ELEMENT_IMAGE_INFO_CW_TYPE in Cert Reset Response.");
        }
    */

    return CW_TRUE;
}

CWBool CWWTPCreateCertRequestFile()
{
    return CW_TRUE;
}
/*
CWBool CWWTPOpenCertRequestFile(FILE** reqFile, int* length, unsigned char hash[])
{
    char filePath[64];

    sprintf(filePath, "%s%s", gWTPCertDir, CW_WTP_FILE_CERT_REQ);

    if(!CWGetFileSize(filePath,length) ||
       !CWGetFileMD5(filePath, hash))
        return CW_FALSE;

    *reqFile = fopen(filePath, "r");

    return *reqFile == NULL ? CW_FALSE : CW_TRUE;
}
*/
CWBool CWAssembleCertResetQueryRequest(CWProtocolMessage **completeMsgPtr, int *fragmentsNumPtr,
                                       int PMTU, int seqNum, void* valuesPtr)
{
    CWProtocolMessage *msgElems = NULL;
    int msgElemCount = 2;
    CWProtocolMessage *msgElemsBinding = NULL;
    CWWTPCertType certType;
    const int msgElemBindingCount = 0;
    int k = -1;

    if(completeMsgPtr == NULL || fragmentsNumPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems,
                                     msgElemCount,
                                     return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if(gWTPDefaultCert)
    {
        CWDebugLog("Assembling Cert Reset Request of CW_WTP_CERT_TYPE_CURRENT_DEFAULT");
        certType = CW_WTP_CERT_TYPE_CURRENT_DEFAULT;
    }
    else if(gWTPTryDefaultCert)
    {
        CWDebugLog("Assembling Cert Reset Request of CW_WTP_CERT_TYPE_DEFAULT");
        certType = CW_WTP_CERT_TYPE_DEFAULT;
    }
    else
    {
        CWDebugLog("Assembling Cert Reset Request of CW_WTP_CERT_TYPE_CURRENT");
        certType = CW_WTP_CERT_TYPE_CURRENT;
    }

    if(!CWAssembleMsgElemCertResetReqType(&(msgElems[++k]), CW_WTP_CERT_REQUEST_TYPE_QUERY) ||
       !CWAssembleMsgElemCertType(&(msgElems[++k]), certType))
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
                           CW_MSG_TYPE_VALUE_CERT_RESET_REQUEST,
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

CWBool CWAssembleCertReqFileRequest(CWProtocolMessage **completeMsgPtr, int *fragmentsNumPtr,
                                    int PMTU, int seqNum, void* valuesPtr)
{
    CWProtocolMessage *msgElems = NULL;
    int msgElemCount = 1;
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

    CWDebugLog("Assembling Cert Reset Request of CW_WTP_CERT_REQUEST_TYPE_CERT_REQ");

    /*Open WTP cert request file*/
    /*
        CWDebugLog("%s, %u : Open WTP certificate request file", __FUNCTION__, __LINE__);
    	FILE *certReqFile;
        int certReqFileSize;
        if(!CWWTPOpenCertRequestFile(&certReqFile, &certReqFileSize, requestValue.certReqData.certreqHash))
            return CWErrorRaise(CW_ERROR_OPERATION_ABORTED, NULL);

    	char *bufferCertReq;

        CW_CREATE_OBJECT_SIZE_ERR(bufferCertReq, certReqFileSize, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    	size_t totalRead;
    	totalRead = fread( bufferCertReq, 1, certReqFileSize, certReqFile);
    	if(totalRead != certReqFileSize){
    		fclose(certReqFile);
            CW_FREE_OBJECT(bufferCertReq);
    		CWDebugLog("%s, %u : Read entire file failed.", __FUNCTION__, __LINE__);
    		return CWErrorRaise(CW_ERROR_OPERATION_ABORTED, NULL);
    	}
    	fclose(certReqFile);

        requestValue.certReqData.dataLen= certReqFileSize;
        requestValue.certReqData.data = bufferCertReq;
    */

    if(!CWAssembleMsgElemCertResetReqType(&(msgElems[++k]), CW_WTP_CERT_REQUEST_TYPE_CERT_REQ))
        /*       !CWAssembleMsgElemCertResetReqData(&(msgElems[++k]), &requestValue))*/
    {
        CWErrorHandleLast();
        int i;
        for(i = 0; i <= k; i++)
        {
            CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);
        }
        //        CW_FREE_OBJECT(bufferCertReq);
        CW_FREE_OBJECT(msgElems);
        /* error will be handled by the caller */
        return CW_FALSE;
    }

    //    CW_FREE_OBJECT(bufferCertReq);

    if(!(CWAssembleMessage(completeMsgPtr,
                           fragmentsNumPtr,
                           PMTU,
                           seqNum,
                           CW_MSG_TYPE_VALUE_CERT_RESET_REQUEST,
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

CWBool CWParseCertReqFileResponseMessage(char *msg, int len, int seqNum, CWACNewCertResponseValues *valuesPtr,int combineLen)
{
    CWControlHeaderValues controlVal;
    int offsetTillMessages;
    unsigned short int elemType;
    unsigned int elemLen;
    CWProtocolMessage completeMsg;
    CWBool resultCodeGot = CW_FALSE;
    CWBool caDataGot = CW_FALSE;
    CWBool certInfoGot = CW_FALSE;
    CWBool certDataGot = CW_FALSE;

    if(msg == NULL || valuesPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    CWDebugLog("Parse Cert Data Response");

    completeMsg.msg = msg;
    completeMsg.offset = 0;

    if(!(CWParseControlHeader(&completeMsg, &controlVal)))
        /* will be handled by the caller */
    {
        return CW_FALSE;
    }

    /* different type */
    if(controlVal.messageTypeValue != CW_MSG_TYPE_VALUE_CERT_RESET_RESPONSE)
    {
        return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Message is not Cert reset Response as Expected");
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
            case CW_MSG_ELEMENT_AC_CERT_INFO_CW_TYPE:
                if(!(CWParseCertInformation(&completeMsg, elemLen, &(valuesPtr->certInfo))))
                {
                    return CW_FALSE;
                }
                certInfoGot = CW_TRUE;
                break;
            case CW_MSG_ELEMENT_AC_CA_DATA_CW_TYPE:
                if(!(CWParseCaData(&completeMsg, elemLen, &(valuesPtr->caData))))
                {
                    return CW_FALSE;
                }
                CWDebugLog("Get CA file length=%d, data:%s", valuesPtr->caData.dataLen, valuesPtr->caData.data);
                caDataGot = CW_TRUE;
                break;
            case CW_MSG_ELEMENT_AC_CERT_DATA_CW_TYPE:
                if(!(CWParseCertData(&completeMsg, elemLen, &(valuesPtr->certData))))
                {
                    return CW_FALSE;
                }
                CWDebugLog("Get Cert file length=%d, data:%s", valuesPtr->certData.dataLen, valuesPtr->certData.data);
                certDataGot = CW_TRUE;
                break;
            default:
                completeMsg.offset += elemLen;
                CWLog("Unrecognized Message Element Type %d in Cert Data Response", elemType);
        }
        /*CWDebugLog("bytes: %d/%d", (completeMsg.offset-offsetTillMessages), controlVal.msgElemsLen);*/
    }

    if(completeMsg.offset != len)
    {
        return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Garbage at the End of the Message");
    }

    if(!resultCodeGot)
    {
        return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Require CW_MSG_ELEMENT_RESULT_CODE_CW_TYPE in Cert Reset Response.");
    }

    if(valuesPtr->resultCode == CW_PROTOCOL_SUCCESS)
    {
        if(!certDataGot)
        {
            return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Require CW_MSG_ELEMENT_AC_CERT_DATA_CW_TYPE in Cert Reset Response.");
        }

        if(!caDataGot)
        {
            return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Require CW_MSG_ELEMENT_AC_CA_DATA_CW_TYPE in Cert Reset Response.");
        }

        if(!certInfoGot)
        {
            return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Require CW_MSG_ELEMENT_AC_CERT_INFO_CW_TYPE in Cert Reset Response.");
        }
    }

    return CW_TRUE;
}

CWBool CWSaveCertResetResponseMessage(CWACImageDataResponseValues *valuesPtr)
{
    //    CWDebugLog("%s, line:%d, Enter CWSaveCertResetResponseMessage",__FUNCTION__, __LINE__);
    return CW_TRUE;
}

CWBool CWAssembleFactoryResetRequest(CWProtocolMessage **completeMsgPtr, int *fragmentsNumPtr,
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

    CWDebugLog("Assembling Factory Reset Request...");

    if(!CWAssembleMsgElemCertResetReqType(&(msgElems[++k]), CW_WTP_CERT_REQUEST_TYPE_FACTORY_RESET) ||
       !CWAssembleMsgElemFactoryResetInterval(&(msgElems[++k]), CWWTPBoardGetRebootTime(CW_TRUE)))
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
                           CW_MSG_TYPE_VALUE_CERT_RESET_REQUEST,
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

CWBool CWParseFactoryResetResponseMessage(char *msg, int len, int seqNum, CWWTPCertResetFactoryResetValues *valuesPtr,int combineLen)
{
    CWControlHeaderValues controlVal;
    int offsetTillMessages;
    unsigned short int elemType;
    unsigned int elemLen;
    CWProtocolMessage completeMsg;
    CWBool resultCodeGot = CW_FALSE;

    if(msg == NULL || valuesPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    CWDebugLog("Parse Cert Factory Reset Response");

    completeMsg.msg = msg;
    completeMsg.offset = 0;

    if(!(CWParseControlHeader(&completeMsg, &controlVal)))
        /* will be handled by the caller */
    {
        return CW_FALSE;
    }

    /* different type */
    if(controlVal.messageTypeValue != CW_MSG_TYPE_VALUE_CERT_RESET_RESPONSE)
    {
        return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Message is not Cert Factory Reset Response as Expected");
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
            default:
                completeMsg.offset += elemLen;
                CWLog("Unrecognized Message Element Type %d in Cert Factory Reset Response", elemType);
        }
        /*CWDebugLog("bytes: %d/%d", (completeMsg.offset-offsetTillMessages), controlVal.msgElemsLen);*/
    }

    if(completeMsg.offset != len)
    {
        return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Garbage at the End of the Message");
    }

    if(!resultCodeGot)
    {
        return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Require CW_MSG_ELEMENT_RESULT_CODE_CW_TYPE in Cert Factory Reset Response.");
    }

    return CW_TRUE;
}

CWBool CWAssembleCertResetResponse(CWProtocolMessage **completeMsgPtr, int *fragmentsNumPtr,
                                   int PMTU, int seqNum, CWWTPImageDataResponseValues *responseValues)
{
    CWProtocolMessage *msgElems = NULL;
    int msgElemCount = 1;
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

    CWDebugLog("Assembling Cert reset Response...");

    if(!(CWAssembleMsgElemResultCode(&(msgElems[++k]), responseValues->resultCode)))
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
                           CW_MSG_TYPE_VALUE_CERT_RESET_RESPONSE,
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

/*
 * Manage Cert Reset State.
 */
CWStateTransition CWWTPEnterCertReset()
{
    CWACNewCertResponseValues responseValuesNewCert;
    CWACCertResetResponseValues responseValuesAC;
    CWWTPCertResetFactoryResetValues responseValuesFactoryReset;
    //   CWWTPCertResetResponseValues responseValuesWTP;
    int seqNum = CWGetSeqNum();

    CWDebugLog("######### Cert Reset State #########");

    if(!CWErr(CWWTPSendAcknowledgedPacket(seqNum,
                                          NULL,
                                          CWAssembleCertResetQueryRequest,
                                          (void *)CWParseCertResetResponseMessage,
                                          (void *)CWSaveCertResetResponseMessage,
                                          &responseValuesAC,
                                          gCWRetransmitTimer
                                         )))
    {
        return CW_ENTER_DISCOVERY;
    }

    if(responseValuesAC.resultCode != CW_PROTOCOL_SUCCESS)
    {
        CWLog("AC Cert Reset Response resultCode=%d", responseValuesAC.resultCode);
        return CW_ENTER_DISCOVERY;
    }

    if(responseValuesAC.certAction == CW_CERT_RESET_ACTION_NONE)
    {
        CWDebugLog("WTP receive CW_CERT_RESET_ACTION_NONE, enter config state.");
        return CW_ENTER_CONFIGURE;
    }
    else if(responseValuesAC.certAction == CW_CERT_RESET_ACTION_FACTORY_RESET)
    {
        CWDebugLog("WTP receive CW_CERT_RESET_ACTION_FACTORY_RESET, enter reset state and do factory reset.");

        if(!CWErr(CWWTPSendAcknowledgedPacket(seqNum,
                                              NULL,
                                              CWAssembleFactoryResetRequest,
                                              (void *)CWParseFactoryResetResponseMessage,
                                              (void *)CWSaveCertResetResponseMessage, &responseValuesFactoryReset,
                                              gCWRetransmitTimer
                                             )))
        {
            return CW_ENTER_DISCOVERY;
        }

        gWTPFactoryReset = CW_TRUE;
        return CW_ENTER_RESET;
        /*
                if(!CWWTPResetCert())
                {
                    CWDebugLog("Reset WTP AC & cert to default failed!!");
                }

                //After resetting cert, use current cert to do handshake next time.

                CWWTPCheckCurrentCertIsDefault();
                gWTPTryDefaultCert = CW_FALSE;
                gWTPFastJoin = CW_TRUE;
                return CW_ENTER_DISCOVERY;
        */
    }
    else if(responseValuesAC.certAction == CW_CERT_RESET_ACTION_CERT_UPDATE)
    {
        responseValuesNewCert.caData.data = NULL;
        responseValuesNewCert.certData.data = NULL;

        if(!CWErr(CWWTPSendAcknowledgedPacket(seqNum,
                                              NULL,
                                              CWAssembleCertReqFileRequest,
                                              (void *)CWParseCertReqFileResponseMessage,
                                              (void *)CWSaveCertResetResponseMessage, &responseValuesNewCert,
                                              gCWRetransmitTimer
                                             )))
        {
            return CW_ENTER_DISCOVERY;
        }

        if(responseValuesAC.resultCode != CW_PROTOCOL_SUCCESS)
        {
            return CW_ENTER_DISCOVERY;
        }

        if(!CWWTPVerifyCertData(responseValuesNewCert.caData.data,
                                responseValuesNewCert.caData.dataLen,
                                responseValuesNewCert.certInfo.caHash))
        {
            CWLog("Received CA data is corrupted", responseValuesAC.resultCode);
            CW_FREE_OBJECT(responseValuesNewCert.caData.data);
            CW_FREE_OBJECT(responseValuesNewCert.certData.data);
            return CW_ENTER_DISCOVERY;
        }

        if(!CWWTPVerifyCertData(responseValuesNewCert.certData.data,
                                responseValuesNewCert.certData.dataLen,
                                responseValuesNewCert.certInfo.certHash))
        {
            CWLog("Received Cert data is corrupted", responseValuesAC.resultCode);
            CW_FREE_OBJECT(responseValuesNewCert.caData.data);
            CW_FREE_OBJECT(responseValuesNewCert.certData.data);
            return CW_ENTER_DISCOVERY;
        }

        /* Remove Current CA & Cert files */
        CWWTPDelCertFile(CW_WTP_CA_NAME_CURRENT);
        CWWTPDelCertFile(CW_WTP_CERT_NAME_CURRENT);

        if(!CWWTPSaveCertFile(CW_WTP_CA_NAME_CURRENT,
                              responseValuesNewCert.caData.data,
                              responseValuesNewCert.caData.dataLen))
        {
            CWLog("Save CA failed");
        }
        else if(!CWWTPSaveCertFile(CW_WTP_CERT_NAME_CURRENT,
                                   responseValuesNewCert.certData.data,
                                   responseValuesNewCert.certData.dataLen))
        {
            CWLog("Save Cert failed");
        }
        else
        {
            CWDebugLog("New CA and Cert file are saved, enter discovery state...");
        }

        /* Free data buffer */
        CW_FREE_OBJECT(responseValuesNewCert.caData.data);
        CW_FREE_OBJECT(responseValuesNewCert.certData.data);

        /*current certificate changed, reset gWTPDefaultCert*/
        CWWTPCheckCurrentCertIsDefault();
        gWTPTryDefaultCert = CW_FALSE;
        gWTPFastJoin = CW_TRUE;

        return CW_ENTER_DISCOVERY;
    }
    else
    {
        CWLog("Unknown certAction %d", responseValuesAC.certAction);
        return CW_ENTER_DISCOVERY;
    }
}



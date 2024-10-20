#include "CWWTP.h"
#include "CWVendorPayloads.h"
#include "WTPProxyMode.h"

static CWThread gProxyModeThread;
static volatile CWBool gProxyModeRun = CW_FALSE;
static CWSocket gProxyModeSock = -1;

static CW_THREAD_RETURN_TYPE ProxyModeTask(void *arg)
{
    char buf[CW_PACKET_BUFFER_SIZE];
    int readBytes;
    CWNetworkLev4Address addr;
    CWControlHeaderValues controlVal;
    CWProtocolTransportHeaderValues transportVal;
    CWProtocolMessage completeMsg;
    CWBool dataFlag;
    int offsetTillMessages;
    CWProtocolVendorSpecificValues vendorValues;
    CWProxyRespWTPInfo *wtpInfo = NULL;
    const int appendElementLen = 2 + 2 + 4 + 2 + 6; /* element type + element len + vendor id + playload type + address from proxy (ip + port) */

    CWDebugLog("ProxyModeTask start");

    while(gProxyModeRun)
    {
        if(!CWErr(CWNetworkReceiveUnsafe(gProxyModeSock,
                                         buf,
                                         sizeof(buf),
                                         0,
                                         &addr,
                                         &readBytes)))
        {
            if(CWErrorGetLastErrorCode() == CW_ERROR_INTERRUPTED)
            {
                continue;
            }
            break;
        }

        completeMsg.msg = buf;
        completeMsg.offset = 0;
        dataFlag = CW_FALSE;

        if(!(CWParseTransportHeader(&completeMsg, &transportVal, &dataFlag)))
            /* will be handled by the caller */
        {
            CWLog("Parse Transport Header failed");
            continue;
        }
        if(!(CWParseControlHeader(&completeMsg, &controlVal)))
            /* will be handled by the caller */
        {
            CWLog("Parse Control Header failed");
            continue;
        }

        /* different type */
        if(controlVal.messageTypeValue == CW_MSG_TYPE_VALUE_DISCOVERY_REQUEST)
        {
            CWUseSockNtop(&addr,
                          CWDebugLog("Receive a Discovery Request from %s", str););

            if(readBytes + appendElementLen > sizeof(buf))
            {
                CWLog("Packet buffer is not enough to add proxy element in Discovery Request");
                continue;
            }

            completeMsg.offset = readBytes;

            CWProtocolStore16(&completeMsg, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE); /* element type */
            CWProtocolStore16(&completeMsg, (4 + 2 + 6)); /* element len */
            CWProtocolStore32(&completeMsg, CW_VENDOR_ID); /* vendor id */
            CWProtocolStore16(&completeMsg, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_PROXY_WTP_ADDRESS); /* playload type */
            CWProtocolStoreIPv4Address(&completeMsg, ((struct sockaddr_in *)&addr)->sin_addr.s_addr);
            CWProtocolStore16(&completeMsg, ntohs(((struct sockaddr_in *)&addr)->sin_port));

            /* Add appendElementLen to Message Element Length */
            *((unsigned short *) &buf[CW_CONTROL_HEADER_ELEMENT_LEN_OFFSET]) = htons(controlVal.msgElemsLen + appendElementLen);

            if(!CWErr(CWNetworkSendUnsafeUnconnected(gProxyModeSock,
                      &(gACInfoPtr->preferredAddress),
                      buf,
                      readBytes + appendElementLen,
                      gACInfoPtr->controllerId)))
            {
                CWLog("Send Proxy Discovery Request to AC failed");
            }

        }
        else if(controlVal.messageTypeValue == CW_MSG_TYPE_VALUE_DISCOVERY_RESPONSE)
        {
            CWUseSockNtop(&addr,
                          CWDebugLog("Receive a Discovery Response from %s", str););

            if(readBytes + appendElementLen > sizeof(buf))
            {
                CWLog("packet buffer is not enough to add AC Address element in Discovery Response");
                continue;
            }

            /* skip timestamp */
            controlVal.msgElemsLen -= CW_CONTROL_HEADER_OFFSET_FOR_MSG_ELEMS;
            offsetTillMessages = completeMsg.offset;

            CW_ZERO_MEMORY(&vendorValues, sizeof(vendorValues));

            /* parse message elements */
            while((completeMsg.offset - offsetTillMessages) < controlVal.msgElemsLen)
            {
                unsigned short int elemType = 0;
                unsigned int elemLen = 0;

                CWParseFormatMsgElem(&completeMsg, &elemType, &elemLen);

                switch(elemType)
                {
                    case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE:
                        if(!CWParseVendorPayload(&completeMsg, elemLen, &vendorValues))
                        {
                            /*CWDestroyVendorSpecificValues(&vendorValues);*/
                        }
                        else if(vendorValues.vendorPayloadType == CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_PROXY_RESPONSE_WTP_INFO)
                        {
                            CW_FREE_OBJECT(wtpInfo);
                            wtpInfo = (CWProxyRespWTPInfo *) vendorValues.payload;
                            vendorValues.payload = NULL;
                        }
                        else
                        {
                            CWDestroyVendorSpecificValues(&vendorValues);
                        }
                        break;
                    default:
                        completeMsg.offset += elemLen;
                        break;
                }
            }

            if(!wtpInfo)
            {
                CWLog("No Proxy WTP Info in the Discovery Response Proxy Message");
                continue;
            }

            if(completeMsg.offset != readBytes)
            {
                CWLog("Garbage at the End of the Discovery Response Proxy Message");
                CW_FREE_OBJECT(wtpInfo);
                continue;
            }

            completeMsg.offset = readBytes;

            CWProtocolStore16(&completeMsg, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE); /* element type */
            CWProtocolStore16(&completeMsg, (4 + 2 + 6)); /* element len */
            CWProtocolStore32(&completeMsg, CW_VENDOR_ID); /* vendor id */
            CWProtocolStore16(&completeMsg, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_PROXY_AC_ADDRESS); /* playload type */
            CWProtocolStoreIPv4Address(&completeMsg, ((struct sockaddr_in *)&addr)->sin_addr.s_addr);
            CWProtocolStore16(&completeMsg, ntohs(((struct sockaddr_in *)&addr)->sin_port));

            /* Add appendElementLen to Message Element Length */
            *((unsigned short *) &buf[CW_CONTROL_HEADER_ELEMENT_LEN_OFFSET]) =
                htons(controlVal.msgElemsLen + CW_CONTROL_HEADER_OFFSET_FOR_MSG_ELEMS + appendElementLen);

            CW_ADDR_SET_IP(&addr, wtpInfo->ip);
            CW_ADDR_SET_PORT(&addr, wtpInfo->port);

            CWUseSockNtop(&addr,
                          CWDebugLog("Discovery Response to WTP %02x:%02x:%02x:%02x:%02x:%02x %s", CW_MAC_PRINT_LIST(wtpInfo->mac), str););

            /* add static route and ARP */
            if(!CWErr(CWNetworkAddHostStaticRoute(gWTPIface, &addr)))
            {
                CWLog("Error Adding Host Static Route");
            }

            if(!CWErr(CWNetworkAddStaticArpEntry(&addr, wtpInfo->mac)))
            {
                CWLog("Error Adding Static ARP");
            }

            if(!CWErr(CWNetworkSendUnsafeUnconnected(gProxyModeSock,
                      &addr,
                      buf,
                      readBytes + appendElementLen,
                      0)))
            {
                CWLog("Send Proxy Discovery Response to WTP failed");
            }

            /* delete static route and ARP */
            if(!CWErr(CWNetworkDelArpEntry(&addr)))
            {
                CWLog("Error Deleting Static ARP");
            }

            if(!CWErr(CWNetworkDelHostStaticRoute(gWTPIface, &addr)))
            {
                CWLog("Error Deleting Host Static Route");
            }

            CW_FREE_OBJECT(wtpInfo);
        }
    }

    CWDebugLog("ProxyModeTask exit");

    CWErrorFree();

    return (void *) 0;
}

CWBool CWWTPStartProxyMode()
{
    int i;
    int yes = 1;
    CWNetworkLev4Address addr;
    struct sockaddr_in bindAddr;

    if(gProxyModeRun)
    {
        return CW_TRUE;
    }

    /* bind a unicast address */
    if((gProxyModeSock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        return CWErrorRaise(CW_ERROR_CREATING, "Can not create socket for proxy mode");
    }

    /* reuse address */
    setsockopt(gProxyModeSock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    CW_ZERO_MEMORY(&bindAddr, sizeof(bindAddr));

    bindAddr.sin_family = AF_INET;
    bindAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    bindAddr.sin_port = htons(CW_CONTROL_PORT);

    if(bind(gProxyModeSock, (struct sockaddr *)&bindAddr, sizeof(bindAddr)) < 0)
    {
        close(gProxyModeSock);
        gProxyModeSock = -1;
        return CWErrorRaise(CW_ERROR_CREATING, "Can not bind address for proxy mode");
    }

    for(i = 0; i < gDiscoveryACCount; i++)
    {
        if(!CWNetworkGetAddressForHost(gDiscoveryACList[i].addrStr, &addr))
        {
            continue;
        }

        if(((struct sockaddr *)&addr)->sa_family == AF_INET)
        {
            /* ensure this is a multicast group */
            if((((struct sockaddr_in *)&addr)->sin_addr.s_addr >> 28) == 14)
            {
                if(mcast_join(gProxyModeSock, (struct sockaddr *)&addr, gWTPIface, 0) != 0)
                {
                    close(gProxyModeSock);
                    gProxyModeSock = -1;
                    return CWErrorRaise(CW_ERROR_CREATING, "Can not join multicast group for proxy mode");
                }

                CWUseSockNtop(&addr,
                              CWLog("Joined Multicast Group: %s", str);
                             );

            }
        }
#ifdef	IPV6
        else
        {
            /* TODO: how to support ipv6 proxy mode*/
        }
#endif
    }

    gProxyModeRun = CW_TRUE;

    if(!CWErr(CWCreateThread(&gProxyModeThread, ProxyModeTask, NULL)))
    {
        close(gProxyModeSock);
        gProxyModeSock = -1;
        gProxyModeRun = CW_FALSE;
        return CWErrorRaise(CW_ERROR_CREATING, "Error starting Thread for proxy mode");
    }

    return CW_TRUE;
}

void CWWTPStopProxyMode()
{
    if(gProxyModeRun)
    {
        gProxyModeRun = CW_FALSE;
        CWNetworkCloseSocket(gProxyModeSock);
        /* wait for the end of thread */
        CWThreadJoin(gProxyModeThread);
        gProxyModeSock = -1;
    }
}

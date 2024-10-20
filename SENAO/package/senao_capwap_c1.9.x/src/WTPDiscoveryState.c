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

/*check clound*/
#define WTP_CLOUND_PATH "/tmp/cloudwtpflag"
#define WTP_KEY_CLOUND  "cloud"
#define WTP_KEY_WTP      "wtp"

/*________________________________________________________________*/
/*  *******************___CAPWAP VARIABLES___*******************  */
int gCWMaxDiscoveries = CW_MAX_DISCOVERY_RETRY;
int gCWMaxRejoinDiscoveries = CW_MAX_REJOIN_DISCOVERY_RETRY;
/*_________________________________________________________*/
/*  *******************___VARIABLES___*******************  */
int gCWDiscoveryInterval = CW_DISCOVERY_INTERVAL_DEFAULT;
int gCWMaxDiscoveryInterval = CW_MAX_DISCOVERY_INTERVAL_DEFAULT;

/*_____________________________________________________*/
/*  *******************___MACRO___*******************  */
#define CWWTPFoundAnAC()	(gACInfoPtr != NULL /*&& gACInfoPtr->preferredAddress.ss_family != AF_UNSPEC*/)

/*__________________________________________________________*/
/*  *******************___PROTOTYPES___*******************  */
CWBool CWReceiveDiscoveryResponse();
CWBool CWWTPEvaluateAC(CWACInfoValues *ACInfoPtr);
CWBool CWReadResponses();
CWBool CWAssembleDiscoveryRequest(CWProtocolMessage **messagesPtr, int seqNum, CWDiscoveryType type);
CWBool CWParseDiscoveryResponseMessage(char *msg,
                                       int len,
                                       int *seqNumPtr,
                                       CWACInfoValues *ACInfoPtr);

void CWDestoryACInfoValues(CWACInfoValues *ACInfoPtr);
CWBool CheckCloudActive()
{
#ifdef CW_CLOUD_CHECK
  FILE * pFile;
  char buffer[32];
  char path[]=WTP_CLOUND_PATH;
  char keyStr[]=WTP_KEY_CLOUND;
  char wtpKeyStr[]=WTP_KEY_WTP;
  memset(buffer,sizeof(buffer),0);
  pFile = fopen (path,"r+w");
  if (pFile!=NULL)
  {
    if(0 == flock(fileno(pFile), LOCK_EX))    
    {
       fgets(buffer , sizeof(buffer) , pFile);
       CWDebugLog("%s Path=%s string=%s keyStr=%s",__FUNCTION__,path,buffer,keyStr);
       if(!strncmp(buffer,keyStr,strlen(keyStr)))
       {          
          fclose (pFile);
          flock(fileno(pFile), LOCK_UN);          
          return CW_TRUE;
       }
       else if(!strncmp(buffer,wtpKeyStr,strlen(wtpKeyStr)))
       {
          flock(fileno(pFile), LOCK_UN);
          fclose (pFile);
          CWSystem("rm -rf %s",path);
          return CW_FALSE;
          
       }
       flock(fileno(pFile), LOCK_UN);
    }
    else
    {
      CWLog("Failed to Lock");
    }   
    
    fclose (pFile);
    return CW_FALSE;
  }
  else
  {    
    return CW_FALSE;
  }
#else
   return CW_FALSE;
#endif
}

CWBool SetWTPActive()
{
#ifdef CW_CLOUD_CHECK

   FILE * pFile;
  char buffer[32];
  char path[]=WTP_CLOUND_PATH;
  char WtpkeyStr[]=WTP_KEY_WTP;
  char keyStr[]=WTP_KEY_CLOUND;
  memset(buffer,sizeof(buffer),0);
  pFile = fopen (path,"w+r");
  if (pFile!=NULL)
  {
    if(0 == flock(fileno(pFile), LOCK_EX))    
    {
       fgets(buffer , sizeof(buffer) , pFile);
       CWDebugLog("%s Path=%s string=%s keyStr=%s",__FUNCTION__,path,buffer,keyStr);
       if(!strncmp(buffer,keyStr,strlen(keyStr)))
       {          
          fclose (pFile);
          flock(fileno(pFile), LOCK_UN);          
          return CW_TRUE;
       } 
       fseek(pFile,0,SEEK_SET);
       fwrite(WtpkeyStr, sizeof(WtpkeyStr), 1, pFile);
       flock(fileno(pFile), LOCK_UN);
    }
    else
    {
      CWLog("Failed to Lock");
    }   
    
    fclose (pFile);
    return CW_FALSE;
  }
  else
  {    
    return CW_FALSE;
  }

#else
   return CW_FALSE;
#endif
}


CWBool CWWTPQueryEzCom(unsigned int *ipAddr, unsigned int *ipLanAddr, unsigned short *bindPort, int *controllerId)
{
    char *resStr, *c1, *c2, *c3, *c4;
    int i;

    if(!gEzComAddr || gEzComAddr[0] == '\0' || !CWWTPBoardGetComEnable())
    {
        return CW_FALSE;
    }

    for(i = 0; i < 2; i++)
    {
        /* First we try TCP */
        if(i == 0)
        {
            CWDebugLog("Start to query EzCom via TCP");

            /* Query with TCP 80 port in first time */
            resStr = CWCreateStringByCmdStdout("ezCom -i %s -p tcp -pn SENAO -sa %s -sp 80 -t 2", gWTPIface, gEzComAddr);
            if(!resStr)
            {
                return CW_FALSE;
            }
        }
        else
        {
            CWDebugLog("Start to query EzCom via UDP");

            /* Use UDP port 53 in second time */
            resStr = CWCreateStringByCmdStdout("ezCom -i %s -p udp -pn SENAO -sa %s -sp 53 -t 2", gWTPIface, gEzComAddr);
            if(!resStr)
            {
                return CW_FALSE;
            }
        }
        CWDebugLog("EzCom result: %s", resStr);

        /* resStr would be in form of ip:port:lan_ip:mac:cid if ezCom replies an AC address */
        if((c1 = strchr(resStr, ':')) && (c2 = strchr(c1 + 1, ':')))
        {
            *c1 = '\0';
            *c2 = '\0';
            if((c3 = strchr(c2 + 1, ':')) && (c4 = strchr(c3 + 1, ':')))
            {
                *c3 = '\0';
                *controllerId = atoi(c4 + 1);
            }
            else
            {
                *controllerId = 0;
            }

            *ipAddr = (unsigned int) inet_addr(resStr);
            *bindPort = (unsigned short) atoi(c1 + 1);
            *ipLanAddr = (unsigned int) inet_addr(c2 + 1);

            CWDebugLog("Get AC address %s:%u Lan Addr %s controllerId %u from EzCom",
                       resStr, *bindPort, c2 + 1, *controllerId);

            CW_FREE_OBJECT(resStr);

            return CW_TRUE;
        }

        if(strstr(resStr, "AC Not Found"))
        {
            CW_FREE_OBJECT(resStr);
            return CW_FALSE;
        }

        CW_FREE_OBJECT(resStr);
    }

    CWDebugLog("Query EzCom FAILED");

    return CW_FALSE;
}

/*_________________________________________________________*/
/*  *******************___FUNCTIONS___*******************  */

/*
 * Manage Discovery State
 */
CWStateTransition CWWTPEnterDiscovery()
{
    int i, preControllerId = 0;
    int discoveryCount = 0;
    int maxDiscoveryCount;
    CWProtocolMessage *msgPtr = NULL;
    CWNetworkLev4Address addr;
    CWBool applyCfg = CW_FALSE, changeIP = CW_FALSE;
    CWAcAddress acAddr;
#ifdef CW_WTP_AP
    int obeyRgrPower;
    int backgroundscanning;
#elif defined(CW_WTP_SWITCH)
    int redundancyStatus = CW_FALSE;
#endif

    CWDebugLog("######### Discovery State #########");

    /* close socket */
    CWNetworkCloseSocket(gWTPSocket);

retry_port:
    /* random port number in the first time and increased by 1 afterward */
    if(gWTPSourcePort == 0)
    {
        gWTPSourcePort = CWRandomIntInRange(20000, 60000);
    }
    else if(++gWTPSourcePort > 60000)
    {
        gWTPSourcePort = 20000;
    }

    if(!CWErr(CWNetworkInitSocketClient(&gWTPSocket, NULL, gWTPIface, gWTPSourcePort)))
    {
        /* error occurs, interface may not ready, try again later */
        CWWaitSec(1);
        goto retry_port;
    }
    
    if(CheckCloudActive())
    {
       /* cloud active,  try again later */
        CWDebugLog("########cloud active##############");
        CWWaitSec(5);
        return CW_ENTER_DISCOVERY;
    }

    if(!CWNetworkAssociateMulticastWithSocket(gWTPIface, gWTPSocket))
    {
        CWLog("Can not associate multicast address with socket");
        /* error occurs, interface may not ready, try again later */
        CWWaitSec(1);
        return CW_ENTER_DISCOVERY;
    }

    if(gChgAcAddr)
    {
        CWLog("AC address is changed to %s", gChgAcAddr);

        if(CWWTPBoardGetForceAcAddress(acAddr.hostName) && acAddr.hostName[0] != '\0')
        {
            /* Change force AC address to new address */
            CWWTPBoardSetForceAcAddress(gChgAcAddr);
        }
        else
        {
            /* Save new AC address to config, ensure the addrsss will be remembered even reboot  */
            strcpy(acAddr.hostName, gChgAcAddr);
            if(CWWTPBoardSetAcListCfg(1, (CWHostName *) acAddr.hostName))
            {
                CWWTPBoardApplyCfg();
            }

            CWParseAcAddrString(gChgAcAddr, &acAddr);
            CWWTPBoardSetAcAddress(&acAddr);
        }

        CW_FREE_OBJECT(gChgAcAddr);
    }

    if(gWTPFastJoin && gACInfoPtr)
    {
        maxDiscoveryCount = gCWMaxRejoinDiscoveries;
        gWTPRejoinAc = CW_TRUE;
        preControllerId = gACInfoPtr->controllerId;

        CW_COPY_MEMORY(&addr, &gACInfoPtr->incomingAddress, sizeof(addr));
        CWUseSockNtop(&addr, CWDebugLog("Fast Join AC %s controllerId %d", str, preControllerId););
    }
    else
    {
        gWTPRejoinAc = CW_FALSE;
        maxDiscoveryCount = gCWMaxDiscoveries;

        if(!CWErr(CWWTPGetDiscoveryACList()))
        {
            return CW_ENTER_DISCOVERY;
        }

        for(i = 0; i < gDiscoveryACCount; i++)
        {
            gDiscoveryACList[i].received = CW_FALSE;
        }
    }

    /* clear previous AC info */
    if(gACInfoPtr)
    {
        CWDestoryACInfoValues(gACInfoPtr);
        CW_FREE_OBJECT(gACInfoPtr);
    }

    /* wait a random time to prevent brust discovery packets sent toward AC if all APs bootup at the same time */
    if(!gWTPFastJoin)
    {
        CWWaitSec(CWRandomIntInRange(0, gCWMaxDiscoveryInterval));
    }

    CWDebugLog("Start Sending Discovery");

    CW_REPEAT_FOREVER
    {
        CWBool sentSomething = CW_FALSE;

        /* we get no responses for a very long time */
        if(discoveryCount == maxDiscoveryCount)
        {
#ifdef CW_WTP_SWITCH
            if(CWWTPBoardGetRedundantManageEnable(&redundancyStatus) && redundancyStatus)
            {
                if(CWWTPBoardGetAcAddress(&acAddr) && acAddr.hostName[0] != '\0')
                {
                    return CW_ENTER_AC;
                }
            }
#endif
            /* Clear last-join AC address if no AC response */
            if(CWWTPBoardGetAcAddress(&acAddr) && acAddr.hostName[0] != '\0')
            {
                acAddr.hostName[0] = '\0';
                CWWTPBoardSetAcAddress(&acAddr);
            }

            gWTPRejoinAc = CW_FALSE;
            gWTPFastJoin = CW_FALSE;
            gWTPTryDefaultCert = CW_FALSE;

            return CW_ENTER_SULKING;
        }

        if(gWTPRejoinAc)
        {
            /* if this AC hasn't responded to us... */
            /* ...send a Discovery Request */
            /* get sequence number (and increase it) */
            gWTPRejoinAcSeqNum = CWGetSeqNum();

            if(!CWErr(CWAssembleDiscoveryRequest(&msgPtr, gWTPRejoinAcSeqNum, CWWTPGetDiscoveryType())))
            {
                return CW_ENTER_DISCOVERY;
            }

            CWUseSockNtop(&addr, CWDebugLog(str););

            if(CWErr(CWNetworkSendUnsafeUnconnected(gWTPSocket,
                                                    &addr,
                                                    (*msgPtr).msg,
                                                    (*msgPtr).offset,
                                                    preControllerId)))
            {
                /*
                 * we sent at least one Request in this loop
                 * (even if we got an error sending it)
                 */
                sentSomething = CW_TRUE;
                CW_FREE_PROTOCOL_MESSAGE(*msgPtr);
                CW_FREE_OBJECT(msgPtr);
            }
            else
            {
                CW_FREE_PROTOCOL_MESSAGE(*msgPtr);
                CW_FREE_OBJECT(msgPtr);

                /* error sending occurs, interface may not ready, try again later */
                CWWaitSec(1);
                return CW_ENTER_DISCOVERY;
            }
        }
        else
        {
            /* send Requests to one or more ACs */
            for(i = 0; i < gDiscoveryACCount; i++)
            {
                /* if this AC hasn't responded to us... */
                if(!gDiscoveryACList[i].received)
                {
                    /* ...send a Discovery Request */
                    /* get sequence number (and increase it) */
                    gDiscoveryACList[i].seqNum = CWGetSeqNum();

                    if(CWNetworkGetAddressForHostTimeout(gDiscoveryACList[i].addrStr, &gDiscoveryACList[i].addr, 2))
                    {
                        CWUseSockNtop(&gDiscoveryACList[i].addr,
                        CWDebugLog("Address %s is resolved as %s", gDiscoveryACList[i].addrStr, str););
                    }
                    else
                    {
                        CWLog("Could not resolve address %s", gDiscoveryACList[i].addrStr);
                        continue;
                    }

                    if(!CWErr(CWAssembleDiscoveryRequest(&msgPtr, gDiscoveryACList[i].seqNum, gDiscoveryACList[i].type)))
                    {
                        return CW_ENTER_DISCOVERY;
                    }

                    if(CWErr(CWNetworkSendUnsafeUnconnected(gWTPSocket,
                                                            &gDiscoveryACList[i].addr,
                                                            (*msgPtr).msg,
                                                            (*msgPtr).offset,
                                                            gDiscoveryACList[i].controllerId)))
                    {
                        /*
                         * we sent at least one Request in this loop
                         * (even if we got an error sending it)
                         */
                        sentSomething = CW_TRUE;
                    }

                    CW_FREE_PROTOCOL_MESSAGE(*msgPtr);
                    CW_FREE_OBJECT(msgPtr);
                }
            }
        }

        /* All AC sent the response (so we didn't send any request) */
        if(!sentSomething && CWWTPFoundAnAC())
        {
            break;
        }

        discoveryCount++;

        /* wait for Responses */
        if(CWErr(CWReadResponses()) && CWWTPFoundAnAC())
        {
            /* we read at least one valid Discovery Response */
            break;
        }

        CWDebugLog("WTP Discovery-To-Discovery (%d)", discoveryCount);
    }

    CWDebugLog("WTP Picks an AC");

    /* crit error: we should have received at least one Discovery Response */
    if(!CWWTPFoundAnAC())
    {
        CWLog("No Discovery response Received");
        gWTPFastJoin = CW_FALSE;
        return CW_ENTER_DISCOVERY;
    }

    if(CheckCloudActive())
    {
        CWDebugLog("########Cloud Active,Enter Discovery##########");
        gWTPFastJoin = CW_FALSE;
        return CW_ENTER_DISCOVERY;
    }
    else
    {
       SetWTPActive();
       CWDebugLog("#########Cloud Not active##########");
    }

    /* Change debug log state by AC's response */
    if(gEnabledDebugLog < 2)
    {
        gEnabledDebugLog = gACInfoPtr->debugLogState;
    }

    /*Change packet interval*/
     if(gACInfoPtr->wtpPacketInterval)
    {
        gWTPPacketInterval= gACInfoPtr->wtpPacketInterval;
    }

    /* Revert any unsaved change before Join */
    CWWTPBoardCancelCfg();

#ifdef CW_WTP_AP
    /* Turn off radio's ObeyRegulatoryPower */
    for(i = 0; i < CWWTPBoardGetMaxRadio(); i++)
    {
        if(CWWTPBoardGetRadioObeyRegulatoryPowerCfg(i, &obeyRgrPower) && obeyRgrPower == CW_TRUE)
        {
            CWDebugLog("Turn off 'Obey Regulatory Power' for radio%u", i);
            CWWTPBoardSetRadioObeyRegulatoryPowerCfg(i, CW_FALSE);
            applyCfg = CW_TRUE;
        }
        if (access("/sbin/BackgroundScan.sh", R_OK) == 0)
        {
            if(CWWTPBoardGetRadioBackgroundScanningCfg(i, &backgroundscanning) && backgroundscanning == CW_FALSE)
            {
                CWDebugLog("Turn on 'Background Scanning' for radio%u", i);
                CWWTPBoardSetRadioBackgroundScanningCfg(i, CW_TRUE);
                applyCfg = CW_TRUE;
            }
        }
    }
#endif
    CWWTPBoradPreSetConfig(gACInfoPtr);

    /* SENAO Jonse 2013/5/24: Change ip setting and restart discovery */
    if(gACInfoPtr->ipCfgInfo)
    {
        CWIPv4Cfg ipv4Cfg, ipv4CfgCurrent;
        unsigned int dns;

        if(gACInfoPtr->ipCfgInfo->ip == 0)
        {
            CWDebugLog("AC requests WTP to apply DHCP mode");
            ipv4Cfg.address = 0;
            ipv4Cfg.mask = 0;
            ipv4Cfg.gateway = 0;
        }
        else
        {
            CWDebugLog("AC requests WTP to apply static IP: %u.%u.%u.%u/%u.%u.%u.%u/%u.%u.%u.%u",
                       CW_IPV4_PRINT_LIST(gACInfoPtr->ipCfgInfo->ip),
                       CW_IPV4_PRINT_LIST(gACInfoPtr->ipCfgInfo->mask),
                       CW_IPV4_PRINT_LIST(gACInfoPtr->ipCfgInfo->gateway));
            ipv4Cfg.address = gACInfoPtr->ipCfgInfo->ip;
            ipv4Cfg.mask = gACInfoPtr->ipCfgInfo->mask;
            ipv4Cfg.gateway = gACInfoPtr->ipCfgInfo->gateway;
        }

        if(!CWWTPBoardGetIPv4Cfg(&ipv4CfgCurrent) || memcmp(&ipv4CfgCurrent, &ipv4Cfg, sizeof(CWIPv4Cfg)))
        {
            CWWTPBoardSetIPv4Cfg(&ipv4Cfg);
            applyCfg = CW_TRUE;
            if(ipv4Cfg.address != ipv4CfgCurrent.address)
            {
                changeIP = CW_TRUE;
            }
        }

        if(CWWTPBoardGetDns1Cfg(&dns) && dns != gACInfoPtr->ipCfgInfo->dns1)
        {
            CWWTPBoardSetDns1Cfg(gACInfoPtr->ipCfgInfo->dns1);
            applyCfg = CW_TRUE;
        }

        if(CWWTPBoardGetDns2Cfg(&dns) && dns != gACInfoPtr->ipCfgInfo->dns2)
        {
            CWWTPBoardSetDns2Cfg(gACInfoPtr->ipCfgInfo->dns2);
            applyCfg = CW_TRUE;
        }
    }

    if(applyCfg)
    {
        CWWTPBoardApplyCfg();
        if(changeIP)
        {
            CWDebugLog("IP is changed, restart discovery");
            gWTPRejoinAc = CW_TRUE;
            gWTPFastJoin = CW_TRUE;
            return CW_ENTER_DISCOVERY;
        }
    }

    if(!gACInfoPtr->timer)
    {
        CW_CREATE_OBJECT_ERR(gACInfoPtr->timer, CWTimer,
        {
            CWLog("Out of Memory");
            return CW_ENTER_DISCOVERY;
        });

        /* Use default timer if AC doesn't provide timer settings */
        gACInfoPtr->timer->dtlsSetup = CW_WTP_WAIT_HANDSHAKE_DEFAULT;
        gACInfoPtr->timer->joinState = CW_WAIT_JOIN_DEFAULT;
        gACInfoPtr->timer->imageState = CW_WAIT_IMAGE_STATE_DEFAULT;
        gACInfoPtr->timer->imageData = CW_WTP_IMAGE_DATA_INTERVAL_DEFAULT;
        gACInfoPtr->timer->configState = CW_WTP_WAIT_CONFIGURE_STATE_DEFAULT;
        gACInfoPtr->timer->changeState = CW_WTP_CHANGE_STATE_INTERVAL_DEFAULT;
        gACInfoPtr->timer->peerDead = CW_NEIGHBORDEAD_INTERVAL_DEFAULT;
        gACInfoPtr->timer->retransmitInterval = CW_WTP_RETRANSMIT_INTERVAL_DEFAULT;
        gACInfoPtr->timer->retransmitCount = CW_MAX_RETRANSMIT_DEFAULT;
        gACInfoPtr->timer->fragment = CW_FRAGMENTS_INTERVAL_DEFAULT;
    }

    if(gACInfoPtr->utc_time)
    {
        CWWTPBoardSetUTCTime(gACInfoPtr->utc_time);
    }

    gCWRetransmitTimer = gACInfoPtr->timer->retransmitInterval;
    gCWMaxRetransmit = gACInfoPtr->timer->retransmitCount;
    gCWNeighborDeadInterval = gACInfoPtr->timer->peerDead;

    /* if the AC is multi homed, we select our favorite AC's interface */
    CWWTPPickACInterface();

    CWUseSockNtop(&(gACInfoPtr->preferredAddress),
                  CWDebugLog("Preferred AC: \"%s\", at address: %s", gACInfoPtr->name, str););

    if(gWTPFastJoin)
    {
        gACInfoPtr->controllerId = preControllerId;
        gWTPFastJoin = CW_FALSE;
    }

    return CW_ENTER_JOIN;
}

/*
 * Wait DiscoveryInterval time while receiving Discovery Responses.
 */
CWBool CWReadResponses()
{
    CWBool result = CW_FALSE;

    struct timeval timeout, before, after, delta, newTimeout;

    timeout.tv_sec = newTimeout.tv_sec = gCWDiscoveryInterval;
    timeout.tv_usec = newTimeout.tv_usec = 0;

    gettimeofday(&before, NULL);

    CW_REPEAT_FOREVER
    {
        /* check if something is available to read until newTimeout */
        if(CWNetworkTimedPollRead(gWTPSocket, &newTimeout))
        {
            /* success
             * if there was no error, raise a "success error", so we can easily handle
             * all the cases in the switch
             */
            CWErrorRaise(CW_ERROR_SUCCESS, NULL);
        }

        switch(CWErrorGetLastErrorCode())
        {
            case CW_ERROR_TIME_EXPIRED:
                goto cw_time_over;
                break;

            case CW_ERROR_SUCCESS:
                result = CWReceiveDiscoveryResponse();
                if(result && gWTPFastJoin)
                {
                    return result;
                }
            case CW_ERROR_INTERRUPTED:
                /*
                 * something to read OR interrupted by the system
                 * wait for the remaining time (NetworkPoll will be recalled with the remaining time)
                 */
                gettimeofday(&after, NULL);

                CWTimevalSubtract(&delta, &after, &before);
                if(CWTimevalSubtract(&newTimeout, &timeout, &delta) == 1)
                {
                    /* negative delta: time is over */
                    goto cw_time_over;
                }
                break;
            default:
                CWErrorHandleLast();
                goto cw_error;
                break;
        }
    }
cw_time_over:
    /* time is over */
    CWDebugLog("Timer expired during receive");
cw_error:
    return result;
}

/*
 * Gets a datagram from network that should be a Discovery Response.
 */
CWBool CWReceiveDiscoveryResponse()
{
    char buf[CW_PACKET_BUFFER_SIZE];
    int i;
    CWNetworkLev4Address addr;
    CWACInfoValues *ACInfoPtr;
    CWAcAddress acAddr;
    int seqNum;
    int readBytes;

    /* receive the datagram */
    if(!CWErr(CWNetworkReceiveUnsafe(gWTPSocket,
                                     buf,
                                     sizeof(buf),
                                     0,
                                     &addr,
                                     &readBytes)))
    {
        return CW_FALSE;
    }

    CW_CREATE_OBJECT_ERR(ACInfoPtr,
                         CWACInfoValues,
                         return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
    CW_ZERO_MEMORY(ACInfoPtr, sizeof(CWACInfoValues));

    /* check if it is a valid Discovery Response */
    if(!CWErr(CWParseDiscoveryResponseMessage(buf, readBytes, &seqNum, ACInfoPtr)))
    {
        CWDestoryACInfoValues(ACInfoPtr);
        CW_FREE_OBJECT(ACInfoPtr);
        return CWErrorRaise(CW_ERROR_INVALID_FORMAT,
                            "Received something different from a "
                            "Discovery Response while in Discovery State");
    }

    if(ACInfoPtr->proxyAddress)
    {
        CW_COPY_NET_ADDR_PORT(&(ACInfoPtr->incomingAddress), ACInfoPtr->proxyAddress);
    }
    else
    {
        CW_COPY_NET_ADDR_PORT(&(ACInfoPtr->incomingAddress), &(addr));
    }

    if(gWTPRejoinAc)
    {
        if(gWTPRejoinAcSeqNum == seqNum)
        {
            /* we received response from this address */
            CWUseSockNtop(&addr,
                          CWDebugLog("Discovery Response from:%s", str););

            CWWTPBoardGetAcAddress(&acAddr);
            ACInfoPtr->controllerId = acAddr.controllerId;

            /* see if this AC is better than the one we have stored */
            CWWTPEvaluateAC(ACInfoPtr);

            return CW_TRUE;
        }
        else
        {
            CWLog("Sequence Number %u of Discovery Response doesn't macth Request %u",
                  seqNum, gWTPRejoinAcSeqNum);
        }
    }
    else
    {
        /* check if the sequence number we got is correct */
        for(i = 0; i < gDiscoveryACCount; i++)
        {
            if(gDiscoveryACList[i].seqNum == seqNum)
            {
                CWUseSockNtop(&addr,
                              CWDebugLog("Discovery Response from:%s", str););

                if(ACInfoPtr->proxyAddress)
                {
                    CWUseSockNtop(ACInfoPtr->proxyAddress,
                                  CWLog("Proxy AC Address %s", str););
                }

                ACInfoPtr->controllerId = gDiscoveryACList[i].controllerId;

                /* see if this AC is better than the one we have stored */
                if(CWWTPEvaluateAC(ACInfoPtr))
                {
                    gWTPDiscType = gDiscoveryACList[i].type;
                }

                /* we received response from this address */
                gDiscoveryACList[i].received = CW_TRUE;

                return CW_TRUE;
            }
        }
    }

    CWDestoryACInfoValues(ACInfoPtr);
    CW_FREE_OBJECT(ACInfoPtr);

    return CWErrorRaise(CW_ERROR_INVALID_FORMAT,
                        "Sequence Number of Response doesn't macth Request");
}

CWBool CWWTPCheckHigerAcPriority(CWACInfoValues *newACInfoPtr)
{
    int i;
    int oldPri = gDiscoveryACCount;
    int newPri = gDiscoveryACCount;
    CWAcAddress acAddr;

    for(i = 0; i < gDiscoveryACCount; i++)
    {
        if(oldPri == gDiscoveryACCount &&
           !CW_ADDR_PORT_CMP(&gACInfoPtr->incomingAddress, &gDiscoveryACList[i].addr))
        {
            CWDebugLog("Old AC address %s pri=%d", gDiscoveryACList[i].addrStr, i);
            oldPri = i;
        }

        if(newPri == gDiscoveryACCount &&
           !CW_ADDR_PORT_CMP(&newACInfoPtr->incomingAddress, &gDiscoveryACList[i].addr))
        {
            CWDebugLog("New AC address %s pri=%d", gDiscoveryACList[i].addrStr, i);
            newPri = i;
        }
    }

    CWDebugLog("newpri=%d oldPri=%d", newPri, oldPri);

    if(newPri < oldPri)
    {
        return CW_TRUE;
    }
    else if(newPri == oldPri &&
            newPri == gDiscoveryACCount && /* receive from discovery address */
            CWWTPBoardGetAcAddress(&acAddr) && acAddr.hostName[0] != '\0')
    {
        /* Check if the AC is last-join AC,  */
        if(inet_addr(acAddr.hostName) == CW_ADDR_GET_IP(&newACInfoPtr->incomingAddress) &&
           acAddr.port == CW_ADDR_GET_PORT(&newACInfoPtr->incomingAddress) &&
           acAddr.controllerId == newACInfoPtr->controllerId)
        {
            return CW_TRUE;
        }
    }

    return CW_FALSE;
}

CWBool CWWTPEvaluateAC(CWACInfoValues *ACInfoPtr)
{
    if(ACInfoPtr == NULL)
    {
        return CW_FALSE;
    }

    if(ACInfoPtr->vendorInfos.vendorInfosCount == 0 ||
       ACInfoPtr->vendorInfos.vendorInfos[0].vendorIdentifier != CW_VENDOR_ID)
    {
        CWLog("WTP Receives Discovery Response but the vendor ID is incorrect");
        CWDestoryACInfoValues(ACInfoPtr);
        CW_FREE_OBJECT(ACInfoPtr);
        return CW_FALSE;
    }

    if(gACInfoPtr == NULL)
    {
        /*
         * this is the first AC we evaluate: so
         *  it's the best AC we examined so far
         */
        gACInfoPtr = ACInfoPtr;
        return CW_TRUE;
    }
    else if(CWWTPCheckHigerAcPriority(ACInfoPtr))
    {
        CWDestoryACInfoValues(gACInfoPtr);
        CW_FREE_OBJECT(gACInfoPtr);
        gACInfoPtr = ACInfoPtr;
        return CW_TRUE;
    }

    CWDestoryACInfoValues(ACInfoPtr);
    CW_FREE_OBJECT(ACInfoPtr);

    /*
     * ... note: we can add our favourite algorithm to pick the best AC.
     * We can also consider to remember all the Discovery Responses we
     * received and not just the best.
     */
    return CW_FALSE;
}

/*
 * Pick one interface of the AC (easy if there is just one interface). The
 * current algorithm just pick the Ac with less WTP communicating with it. If
 * the addresses returned by the AC in the Discovery Response don't include the
 * address of the sender of the Discovery Response, we ignore the address in
 * the Response and use the one of the sender (maybe the AC sees garbage
 * address, i.e. it is behind a NAT).
 */
void CWWTPPickACInterface()
{
    int i, min;
    CWBool foundIncoming = CW_FALSE;
    if(gACInfoPtr == NULL)
    {
        return;
    }

    gACInfoPtr->preferredAddress.ss_family = AF_UNSPEC;

    if(gNetworkPreferredFamily == CW_IPv6)
    {
        goto cw_pick_IPv6;
    }

cw_pick_IPv4:
    if(gACInfoPtr->IPv4Addresses == NULL || gACInfoPtr->IPv4AddressesCount <= 0)
    {
        return;
    }

    min = gACInfoPtr->IPv4Addresses[0].WTPCount;

    CW_COPY_NET_ADDR_PORT(&(gACInfoPtr->preferredAddress),
                          &(gACInfoPtr->IPv4Addresses[0].addr));

    for(i = 1; i < gACInfoPtr->IPv4AddressesCount; i++)
    {

        if(!sock_cmp_addr((struct sockaddr *) & (gACInfoPtr->IPv4Addresses[i]),
                          (struct sockaddr *) & (gACInfoPtr->incomingAddress),
                          sizeof(struct sockaddr_in)))
        {
            foundIncoming = CW_TRUE;
        }

        if(gACInfoPtr->IPv4Addresses[i].WTPCount < min)
        {

            min = gACInfoPtr->IPv4Addresses[i].WTPCount;
            CW_COPY_NET_ADDR_PORT(&(gACInfoPtr->preferredAddress),
                                  &(gACInfoPtr->IPv4Addresses[i].addr));
        }
    }

    if(!foundIncoming)
    {
        /*
         * If the addresses returned by the AC in the Discovery
         * Response don't include the address of the sender of the
         * Discovery Response, we ignore the address in the Response
         * and use the one of the sender (maybe the AC sees garbage
         * address, i.e. it is behind a NAT).
         */
        CW_COPY_NET_ADDR_PORT(&(gACInfoPtr->preferredAddress),
                              &(gACInfoPtr->incomingAddress));
    }
    return;

cw_pick_IPv6:
    /* CWDebugLog("Pick IPv6"); */
    if(gACInfoPtr->IPv6Addresses == NULL || \
       gACInfoPtr->IPv6AddressesCount <= 0)
    {
        goto cw_pick_IPv4;
    }

    min = gACInfoPtr->IPv6Addresses[0].WTPCount;
    CW_COPY_NET_ADDR_PORT(&(gACInfoPtr->preferredAddress),
                          &(gACInfoPtr->IPv6Addresses[0].addr));

    for(i = 1; i < gACInfoPtr->IPv6AddressesCount; i++)
    {

        /*
         * if(!sock_cmp_addr(&(gACInfoPtr->IPv6Addresses[i]),
         * 		     &(gACInfoPtr->incomingAddress),
         * 		     sizeof(struct sockaddr_in6)))
         *
         * 	foundIncoming = CW_TRUE;
         */

        if(gACInfoPtr->IPv6Addresses[i].WTPCount < min)
        {
            min = gACInfoPtr->IPv6Addresses[i].WTPCount;
            CW_COPY_NET_ADDR_PORT(&(gACInfoPtr->preferredAddress),
                                  &(gACInfoPtr->IPv6Addresses[i].addr));
        }
    }
    /*
    if(!foundIncoming) {
    	CW_COPY_NET_ADDR_PORT(&(gACInfoPtr->preferredAddress),
    			     &(gACInfoPtr->incomingAddress));
    }
    */
    return;
}

CWBool CWAssembleDiscoveryRequest(CWProtocolMessage **messagesPtr, int seqNum, CWDiscoveryType type)
{
    CWProtocolMessage *msgElems = NULL;
#ifdef CW_WTP_SWITCH
    const int msgElemCount = 7;
#elif defined(CW_WTP_AP)
    const int msgElemCount = 8;
#endif
    CWWtpCfgCapInfo cfgCapInfo;
    int k = -1, i;
    int fragmentsNum;
    CWIPCfgInfo ipInfo;
    CWIPv4Cfg ipv4Cfg;

    if(messagesPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems, msgElemCount, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if(!CWWTPBoardGetIPv4Cfg(&ipv4Cfg) ||
       !CWWTPBoardGetDns1Cfg(&ipInfo.dns1) ||
       !CWWTPBoardGetDns2Cfg(&ipInfo.dns2)
      )
    {
        CW_FREE_OBJECT(msgElems);
        return CW_FALSE;
    }

    ipInfo.ip = ipv4Cfg.address;
    ipInfo.mask = ipv4Cfg.mask;
    ipInfo.gateway = ipv4Cfg.mask;

    if(!CWWTPGetWtpCfgCapInfo(&cfgCapInfo))
    {
        CW_FREE_OBJECT(msgElems);
        return CW_FALSE;
    }

    /* Assemble Message Elements */
#ifdef CW_WTP_SWITCH
    if((!(CWAssembleMsgElemDiscoveryType(&(msgElems[++k]), type))) ||
       (!(CWAssembleMsgElemWTPBoardData(&(msgElems[++k])))) ||
       (!(CWAssembleMsgElemWTPDescriptor(&(msgElems[++k])))) ||
       (!(CWAssembleMsgElemWTPFrameTunnelMode(&(msgElems[++k])))) ||
       (!(CWAssembleMsgElemWTPMACType(&(msgElems[++k])))) ||
       (!(CWAssembleMsgElemVendorPayloadWtpCfgCapInfo(&(msgElems[++k]), &cfgCapInfo))) ||
       (!(CWAssembleMsgElemVendorPayloadIPCfgInfo(&(msgElems[++k]), &ipInfo)))
      )
#elif defined(CW_WTP_AP)
    if((!(CWAssembleMsgElemDiscoveryType(&(msgElems[++k]), type))) ||
       (!(CWAssembleMsgElemWTPBoardData(&(msgElems[++k])))) ||
       (!(CWAssembleMsgElemWTPDescriptor(&(msgElems[++k])))) ||
       (!(CWAssembleMsgElemWTPFrameTunnelMode(&(msgElems[++k])))) ||
       (!(CWAssembleMsgElemWTPMACType(&(msgElems[++k])))) ||
       (!(CWAssembleMsgElemWTPRadioInformation(&(msgElems[++k])))) ||
       (!(CWAssembleMsgElemVendorPayloadWtpCfgCapInfo(&(msgElems[++k]), &cfgCapInfo))) ||
       (!(CWAssembleMsgElemVendorPayloadIPCfgInfo(&(msgElems[++k]), &ipInfo)))
      )
#endif
    {
        for(i = 0; i <= k; i++)
        {
            CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);
        }
        CW_FREE_OBJECT(msgElems);
        CW_FREE_OBJECT(cfgCapInfo.cfgCap);

        /* error will be handled by the caller */
        return CW_FALSE;
    }

    CW_FREE_OBJECT(cfgCapInfo.cfgCap);

    return CWAssembleMessage(messagesPtr,
                             &fragmentsNum,
                             0,
                             seqNum,
                             CW_MSG_TYPE_VALUE_DISCOVERY_REQUEST,
                             msgElems,
                             msgElemCount,
                             NULL,
                             0,
                             CW_PACKET_PLAIN);
}

/*
 *  Parse Discovery Response and return informations in *ACInfoPtr.
 */
CWBool CWParseDiscoveryResponseMessage(char *msg,
                                       int len,
                                       int *seqNumPtr,
                                       CWACInfoValues *ACInfoPtr)
{
    CWControlHeaderValues controlVal;
    CWProtocolTransportHeaderValues transportVal;
    int offsetTillMessages, i, j;
    CWProtocolVendorSpecificValues vendorValues;
    CWProtocolMessage completeMsg;

    if(msg == NULL || seqNumPtr == NULL || ACInfoPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    CWDebugLog("Parse Discovery Response");

    completeMsg.msg = msg;
    completeMsg.offset = 0;

    CWBool dataFlag = CW_FALSE;
    /* will be handled by the caller */
    if(!(CWParseTransportHeader(&completeMsg, &transportVal, &dataFlag)))
    {
        return CW_FALSE;
    }

    /* will be handled by the caller */
    if(!(CWParseControlHeader(&completeMsg, &controlVal)))
    {
        return CW_FALSE;
    }

    /* different type */
    if(controlVal.messageTypeValue != CW_MSG_TYPE_VALUE_DISCOVERY_RESPONSE)
    {
        return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Message is not Discovery Response as Expected");
    }

    *seqNumPtr = controlVal.seqNum;

    /* skip timestamp */
    controlVal.msgElemsLen -= CW_CONTROL_HEADER_OFFSET_FOR_MSG_ELEMS;

    offsetTillMessages = completeMsg.offset;

    ACInfoPtr->IPv4AddressesCount = 0;
    ACInfoPtr->IPv6AddressesCount = 0;
    ACInfoPtr->debugLogState = gEnabledDebugLog;
    CW_ZERO_MEMORY(&vendorValues, sizeof(vendorValues));

    /* parse message elements */
    while((completeMsg.offset - offsetTillMessages) < controlVal.msgElemsLen)
    {
        unsigned short int type = 0;	/* = CWProtocolRetrieve32(&completeMsg); */
        unsigned int len = 0;	/* = CWProtocolRetrieve16(&completeMsg); */

        CWParseFormatMsgElem(&completeMsg, &type, &len);
        CWDebugLog("Parsing Message Element: %u, len: %u", type, len);

        switch(type)
        {
            case CW_MSG_ELEMENT_AC_DESCRIPTOR_CW_TYPE:
                /* will be handled by the caller */
                if(!(CWParseACDescriptor(&completeMsg, len, ACInfoPtr)))
                {
                    return CW_FALSE;
                }
                break;
            case CW_MSG_ELEMENT_AC_NAME_CW_TYPE:
                /* will be handled by the caller */
                if(!(CWParseACName(&completeMsg, len, &(ACInfoPtr->name))))
                {
                    return CW_FALSE;
                }
                break;
            case CW_MSG_ELEMENT_CW_CONTROL_IPV4_ADDRESS_CW_TYPE:
                /*
                 * just count how many interfacess we have,
                 * so we can allocate the array
                 */
                ACInfoPtr->IPv4AddressesCount++;
                completeMsg.offset += len;
                break;
            case CW_MSG_ELEMENT_CW_CONTROL_IPV6_ADDRESS_CW_TYPE:
                /*
                 * just count how many interfacess we have,
                 * so we can allocate the array
                 */
                ACInfoPtr->IPv6AddressesCount++;
                completeMsg.offset += len;
                break;

            case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE:
                if(!CWParseVendorPayload(&completeMsg, len, &vendorValues))
                {
                    return CW_FALSE;
                }
                else if(vendorValues.vendorPayloadType == CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_IP_CFG_INFO)
                {
                    CW_FREE_OBJECT(ACInfoPtr->ipCfgInfo);
                    ACInfoPtr->ipCfgInfo = (CWIPCfgInfo *) vendorValues.payload;
                    vendorValues.payload = NULL;
                }
                else if(vendorValues.vendorPayloadType == CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CAPWAP_TIMER)
                {
                    CW_FREE_OBJECT(ACInfoPtr->timer);
                    ACInfoPtr->timer = (CWTimer *) vendorValues.payload;
                    vendorValues.payload = NULL;
                }
                else if(vendorValues.vendorPayloadType == CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_DEBUG_LOG)
                {
                    ACInfoPtr->debugLogState = (CWBool) vendorValues.payload;
                }
                else if(vendorValues.vendorPayloadType == CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_PROXY_AC_ADDRESS)
                {
                    CW_FREE_OBJECT(ACInfoPtr->proxyAddress);
                    ACInfoPtr->proxyAddress = (CWNetworkLev4Address *) vendorValues.payload;
                    vendorValues.payload = NULL;
                }
                else if(vendorValues.vendorPayloadType == CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_MTU)
                {
                    ACInfoPtr->mtu = (unsigned int) (long) vendorValues.payload;
                }
                else if(vendorValues.vendorPayloadType == CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_WTP_PACKET_INTERVEL)
                {
                    ACInfoPtr->wtpPacketInterval = (unsigned int) (long) vendorValues.payload;
                }
                else if(vendorValues.vendorPayloadType == CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_UTC_TIME)
                {
                    ACInfoPtr->utc_time = (int)(long)vendorValues.payload;
                }
                else if(vendorValues.vendorPayloadType == CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_AC_CFG_CAP_INFO)
                { 
                    if(ACInfoPtr->cfgCapInfo)
                    {
                       CW_FREE_OBJECT(ACInfoPtr->cfgCapInfo->cfgCap);
                       CW_FREE_OBJECT(ACInfoPtr->cfgCapInfo);
                    }
                    ACInfoPtr->cfgCapInfo = vendorValues.payload;                   
                }                
                else
                {
                    CWDestroyVendorSpecificValues(&vendorValues);
                }
                break;
            default:
                completeMsg.offset += len;
                CWLog("Unrecognized Message Element %d in Discovery Response", type);
        }

        /* CWDebugLog("bytes: %d/%d",
         * 	      (completeMsg.offset-offsetTillMessages),
         * 	      controlVal.msgElemsLen);
         */
    }

    if(completeMsg.offset != len)
        return CWErrorRaise(CW_ERROR_INVALID_FORMAT,
                            "Garbage at the End of the Message");

    /* actually read each interface info */
    CW_CREATE_ARRAY_ERR(ACInfoPtr->IPv4Addresses,
                        ACInfoPtr->IPv4AddressesCount,
                        CWProtocolIPv4NetworkInterface,
                        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if(ACInfoPtr->IPv6AddressesCount > 0)
    {
        CW_CREATE_ARRAY_ERR(ACInfoPtr->IPv6Addresses,
                            ACInfoPtr->IPv6AddressesCount,
                            CWProtocolIPv6NetworkInterface,
                            return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
    }

    i = 0, j = 0;

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
                                                   &(ACInfoPtr->IPv4Addresses[i]))))
                {
                    return CW_FALSE;
                }
                i++;
                break;
            case CW_MSG_ELEMENT_CW_CONTROL_IPV6_ADDRESS_CW_TYPE:
                /* will be handled by the caller */
                if(!(CWParseCWControlIPv6Addresses(&completeMsg,
                                                   len,
                                                   &(ACInfoPtr->IPv6Addresses[j]))))
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

void CWDestoryACInfoValues(CWACInfoValues *ACInfoPtr)
{
    int i;

    if(!ACInfoPtr)
    {
        return;
    }

    CW_FREE_OBJECT(ACInfoPtr->name);
    CW_FREE_OBJECT(ACInfoPtr->IPv4Addresses);
    CW_FREE_OBJECT(ACInfoPtr->IPv6Addresses);
    CW_FREE_OBJECT(ACInfoPtr->ACIPv4ListInfo.ACIPv4List);
    CW_FREE_OBJECT(ACInfoPtr->ACIPv6ListInfo.ACIPv6List);

    for(i = 0; i < ACInfoPtr->vendorInfos.vendorInfosCount; i++)
    {
        CW_FREE_OBJECT(ACInfoPtr->vendorInfos.vendorInfos[i].valuePtr);
    }
    ACInfoPtr->vendorInfos.vendorInfosCount = 0;
    CW_FREE_OBJECT(ACInfoPtr->vendorInfos.vendorInfos);
    CW_FREE_OBJECT(ACInfoPtr->ipCfgInfo);
    CW_FREE_OBJECT(ACInfoPtr->timer);
    CW_FREE_OBJECT(ACInfoPtr->proxyAddress);
    if(ACInfoPtr->cfgCapInfo)
    {
       ACInfoPtr->cfgCapInfo->count=0;
       CW_FREE_OBJECT(ACInfoPtr->cfgCapInfo->cfgCap);
       CW_FREE_OBJECT(ACInfoPtr->cfgCapInfo);
    }
}


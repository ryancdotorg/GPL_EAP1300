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


#ifndef __CAPWAP_CWWTP_HEADER__
#define __CAPWAP_CWWTP_HEADER__

/*_______________________________________________________*/
/*  *******************___INCLUDE___*******************  */

#include "CWCommon.h"
#include "WTPProtocol.h"
#include "WTPSitesurvey.h"

/*______________________________________________________*/
/*  *******************___DEFINE___*******************  */
#ifdef CW_BUILD_X86
#define WTP_LOG_FILE_NAME	"wtp.log.txt"
#else
#define WTP_LOG_FILE_NAME	"/tmp/wtp.log.txt"
#endif

#ifdef CW_WTP_SWITCH
#define SN_LOG_CAT_WTP      SN_LOG_CAT_SWITCH
#else
#define SN_LOG_CAT_WTP      SN_LOG_CAT_AP
#endif

/*_____________________________________________________________*/
/*  *******************___WTP_BOARD_CONST___*******************  */

/*_____________________________________________________*/
/*  *******************___TYPES___*******************  */

typedef struct {
	char *addrStr;
	CWBool received;
	int seqNum;
	CWNetworkLev4Address addr;
    CWDiscoveryType type;
    int controllerId;
} CWACDescriptor;

/*_____________________________________________________________*/
/*  *******************___WTP VARIABLES___*******************  */
//extern char* gInterfaceName;

extern char **gDefaultACAddresses;
extern int gDefaultACAddressCount;

extern int gIPv4StatusDuplicate;
extern int gIPv6StatusDuplicate;
extern char *gChgAcAddr;
extern CWAuthSecurity gWTPForceSecurity;

extern CWSocket gWTPSocket;
extern unsigned short gWTPSourcePort;

extern int gWTPPathMTU;

extern int gDiscoveryACCount;
extern CWACDescriptor *gDiscoveryACList;

extern CWACInfoValues *gACInfoPtr;

extern int gWTPEchoInterval;
extern int gWTPStatisticsTimer;
extern int gWTPStatisticsPollTimer;
extern int gWTPMemUsageLogThreshold;
extern int gWTPConnectionId;
extern CWBool gWTPClientStateChangeEventEnable;
extern WTPRebootStatisticsInfo gWTPRebootStatistics;
extern CWWTPRadiosInfo gRadiosInfo;
extern CWSecurityContext gWTPSecurityContext;
extern CWSecuritySession gWTPSession;
extern int gWTPPacketInterval;

//extern CWPendingRequestMessage gPendingRequestMsgs[MAX_PENDING_REQUEST_MSGS];

extern CWSafeList gPacketReceiveList;
extern CWSafeList gFrameList;
extern CWThreadCondition gInterfaceWait;
extern CWThreadMutex gInterfaceMutex;

/* SENAO Jonse 2013/5/21: add for board data */
extern int gWTPDiscType;
extern CWACNamesWithPriority gWTPAcNamePriInfo;
/* 20130923 Sigma: add for WTP certificates*/
extern char*  gWTPCertDir;
extern CWBool gWTPDefaultCert;
extern CWBool gWTPTryDefaultCert;
extern CWMacAddress gWTPIfaceMac;
extern char *gWTPIface;
extern const char *gEzComAddr;
extern int gEzComCacheTime;
#ifdef CW_BUILD_X86
extern int gWTPSimCapCode;
extern char *gWTPSimSku;
extern int gWTPSimMaxStaPerWlan;
extern int gWTPSimMaxScannedNumPerRadio;
#endif
extern CWProtocolResultCode gWTPResultCode;
extern CWWtpCfgResult gWTPCfgResult;
extern CWWtpCfgMsgList gWTPCfgRollbackList;
extern CWImageIdentifier gWTPImageId;
extern CWStateTransition gWTPPreviousState;
extern CWStateTransition gWTPCurrentState;
extern CWBool gWTPRejoinAc;
extern CWBool gWTPFastJoin;
extern CWBool gWTPFactoryReset;
extern int gWTPWaitTimeBeforeReset;
extern int gWTPRejoinAcSeqNum;
extern int gWTPLastReqSeqNum;
extern CWBool gWTPDtlsHandshakeTimeout;
extern CWBool gWTPUciLogEnable;
/*__________________________________________________________*/
/*  *******************___PROTOTYPES___*******************  */
/* in WTP.c */
unsigned int CWGetSeqNum();
CWBool CWWTPSendMsg(CWProtocolMessage *messages, int fragmentsNum, CWBool encrypt, CWBool freeMsg);
CWBool CWWTPUpdateRequestSeqNum(unsigned char seqNum);
CWBool CWWTPLoadConfiguration();
CWBool CWWTPInitConfiguration();
void CWWTPResetRadioStatistics(CWRadioStatisticsInfo *radioStatistics);
CWBool CWWTPSendMessage(CWProtocolMessage *messages, int fragmentsNum);
CWBool CWWTPReceiveMessage(CWProtocolMessage *msgPtr, int timeout);
CWBool CWWTPSendAcknowledgedPacket(int seqNum,
			             	       void *reqValuesPtr,
				                   CWBool (assembleFunc)(CWProtocolMessage **, int *, int, int, void*),
				                   CWBool (parseFunc)(char*, int, int, void*,int),
				                   CWBool (saveFunc)(void*),
				                   void *resValuesPtr,
				                   int retransmitTimer);
void CWWTPDestroy();

/* in WTPRunState.c */
CWBool CWAssembleWTPDataTansferRequest(CWProtocolMessage **messagesPtr,
				       int *fragmentsNumPtr,
				       int PMTU,
				       int seqNum,
				       CWList msgElemList);

CWBool CWAssembleWTPEventRequest(CWProtocolMessage **messagesPtr,
				 int *fragmentsNumPtr,
				 int PMTU,
				 int seqNum,
				 CWList msgElemList);

CWBool CWWTPStartRxTask();
void CWWTPStopRxTask();
int CWWTPSecurityRxCB(void* cbRxArg, char **bufPtr);

CWBool CWWTPStartKeepAliveTask();
void CWWTPStopKeepAliveTask();

CWBool CWWTPCheckForBindingFrame();

/* in WTPProtocol_User.c */
int CWWTPGetStatisticsTimer ();
CWBool CWWTPGetWtpCfgCapInfo(CWWtpCfgCapInfo *info);
void CWWTPGetIPv6Address(struct sockaddr_in6* myAddr);
CWBool CWGetWTPRadiosAdminState(CWRadiosAdminInfo *valPtr);
CWBool CWGetDecryptErrorReport(int radioID, CWDecryptErrorReportInfo *valPtr);
void CWDestroyDecryptErrorReport(CWDecryptErrorReportInfo *valPtr);
/* in WTPRetransmission.c */
CWBool CWWTPSendPendingRequestEx(unsigned char msgType, int seqNum, CWProtocolMessage *msgElems, int msgElemNum, int transCount, int interval, CWBool encrypt);
#define CWWTPSendPendingRequest(_msgType, _seqNum, _msgElems, _msgElemNum) \
    CWWTPSendPendingRequestEx(_msgType, _seqNum, _msgElems, _msgElemNum, gCWMaxRetransmit, gCWRetransmitTimer, CW_TRUE)
CWBool CWWTPRemovePendingRequestByResponse(unsigned char msgType, int seqNum);
void CWWTPRemoveAllPendingRequest();
void CWWTPHaltPendingRequest(CWBool enable);
CWBool CWWTPSendLogEvent(int group, const char *category, int level, const char *msgFmt, ...);

/* in WTPDiscoveryState.c */
CWStateTransition CWWTPEnterDiscovery();
void CWWTPPickACInterface();

CWStateTransition CWWTPEnterSulking();
CWStateTransition CWWTPEnterJoin();
CWStateTransition CWWTPEnterConfigure();
CWStateTransition CWWTPEnterDataCheck();
CWStateTransition CWWTPEnterRun();
CWStateTransition CWWTPEnterImage();
CWStateTransition CWWTPEnterCertReset();
CWStateTransition CWWTPEnterWaitBurnImage();


CWBool CWSendAndStartEchoTimer();
CWBool CWStartEchoTimer();
CWBool CWStopEchoTimer();
CWBool CWStartNeighborDeadTimer();
CWBool CWStopNeighborDeadTimer();
CWBool CWResetTimers();
CWBool CWStopTimers();
CWBool CWWTPExternalSignalHandlerStart();
//void CWWTPExternalSignalHandlerStop();

void CWWTPHeartBeatTimerExpiredHandler(void *arg);
//void CWWTPRetransmitTimerExpiredHandler(CWTimerArg arg);

void CWWTPCheckCurrentCertIsDefault();
void CWWTPCheckCurrentCertAvailable();
CWBool CWWTPResetCert();

CWBool CWWTPCheckForWTPEventRequest(unsigned int u32MsgElmntType);
CWBool CWWTPQueryEzCom(unsigned int *ipAddr, unsigned int *ipLanAddr, unsigned short *bindPort, int *controllerId);
void CWWTPFreeDiscoveryACList();
CWBool CWWTPGetDiscoveryACList();

CWBool CWWTPGetRadioIndex(CWRadioFreqType radioType, int *radioIdx);
CWBool CWWTPGetRadioType(int radioIdx, CWRadioFreqType *radioType);
CWBool CWWTPCheckAnySSIDEnable(CWRadioFreqType radioType);
CWBool CWWTPResetAutoTxPower();
CWBool CWWTPSendCurCfg(CWBool status);
//extern CWBool WTPExitOnUpdateCommit;
#endif

/************************************************************************************************
 * Copyright (c) 2006-2009 Laboratorio di Sistemi di Elaborazione e Bioingegneria Informatica	*
 *                          Universita' Campus BioMedico - Italy								*
 *																								*
 * This program is free software; you can redistribute it and/or modify it under the terms		*
 * of the GNU General Public License as published by the Free Software Foundation; either		*
 * version 2 of the License, or (at your option) any later version.								*
 *																								*
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY				*
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A				*
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.						*
 *																								*
 * You should have received a copy of the GNU General Public License along with this			*
 * program; if not, write to the:																*
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,							*
 * MA  02111-1307, USA.																			*
 *																								*
 * -------------------------------------------------------------------------------------------- *
 * Project:  Capwap																				*
 *																								*
 * Authors : Ludovico Rossi (ludo@bluepixysw.com)												*
 *           Del Moro Andrea (andrea_delmoro@libero.it)											*
 *           Giovannini Federica (giovannini.federica@gmail.com)								*
 *           Massimo Vellucci (m.vellucci@unicampus.it)											*
 *           Mauro Bisson (mauro.bis@gmail.com)													*
 *	         Antonio Davoli (antonio.davoli@gmail.com)											*
 ************************************************************************************************/

#include "CWWTP.h"
#include "WTPDataRecordStatistics.h"
#include "WTPProxyMode.h"
#include <getopt.h>
#include <ctype.h>
#ifdef DMALLOC
#include "../dmalloc-5.5.0/dmalloc.h"
#endif
#include <dirent.h>

int gEnabledLog = 1;
int gEnabledDebugLog = 0;
int gMaxLogFileSize = 1000 * 1000 * 1; /* 1M by default */
char gLogFileName[] = WTP_LOG_FILE_NAME;

/* addresses of ACs for Discovery */
char **gDefaultACAddresses;
int gDefaultACAddressCount = 0;
#ifdef CW_WTP_SWITCH
char *gACExecCmd = NULL;
char *gWTPExecCmd = NULL;
#endif

int gIPv4StatusDuplicate = 0;
int gIPv6StatusDuplicate = 0;
/* if not NULL, jump Discovery and use this address for Joining */
CWAuthSecurity 	gWTPForceSecurity = CW_X509_CERTIFICATE;

/* used when wtp is requested to change AC */
char *gChgAcAddr = NULL;

/* UDP network socket */
CWSocket gWTPSocket;
unsigned short gWTPSourcePort = 0;
/* DTLS session vars */
CWSecurityContext	gWTPSecurityContext;
CWSecuritySession 	gWTPSession;

/* list used to pass CAPWAP packets from AC to main thread */
CWSafeList 		gPacketReceiveList;

/* used to synchronize access to the lists */
CWThreadCondition    gInterfaceWait;
CWThreadMutex 		gInterfaceMutex;

/* infos about the ACs to discover */
int gDiscoveryACCount = 0;
CWACDescriptor *gDiscoveryACList = NULL;

/* infos on the better AC we discovered so far */
CWACInfoValues *gACInfoPtr = NULL;

/* WTP statistics timer */
int gWTPStatisticsTimer = CW_STATISTIC_TIMER_DEFAULT;
int gWTPStatisticsPollTimer = CW_STATISTIC_POLL_TIMER_DEFAULT;
int gWTPPacketInterval = CW_WTP_PACKET_INTERVEL_DEFAULT;
CWBool gWTPClientStateChangeEventEnable = CW_TRUE;

WTPRebootStatisticsInfo gWTPRebootStatistics;
#ifdef CW_WTP_AP
CWWTPRadiosInfo gRadiosInfo;
#endif

int gWTPInitMTU = 0; /* initial mtu we want, use interface's mtu if it's 0 */
int gWTPPathMTU = 0; /* path MTU of the current session of capwap payload */

/* Connection identifier provide by AC is used to calculate transmission time */
int gWTPConnectionId = 0;

//CWPendingRequestMessage gPendingRequestMsgs[MAX_PENDING_REQUEST_MSGS];

/* SENAO Jonse 2013/5/21: add for board data */
int gWTPDiscType = CW_MSG_ELEMENT_DISCOVERY_TYPE_UNKNOWN;
CWACNamesWithPriority gWTPAcNamePriInfo = {0, NULL};

/* 20130923 Sigma: add for certificates */
char *gWTPCertDir = NULL;
CWBool gWTPDefaultCert = CW_TRUE;         /* Indicate that current and default cert are the same */
CWBool gWTPTryDefaultCert = CW_FALSE;     /* Indicate that try default cert to handshake */

CWProtocolResultCode gWTPResultCode = CW_PROTOCOL_FAILURE;
CWWtpCfgMsgList gWTPCfgRollbackList = {0, NULL, NULL};
CWWtpCfgResult gWTPCfgResult;
CWImageIdentifier gWTPImageId;
CWStateTransition gWTPPreviousState = CW_ENTER_SULKING;
CWStateTransition gWTPCurrentState = CW_ENTER_DISCOVERY;
CWBool gWTPRejoinAc = CW_FALSE;
CWBool gWTPFastJoin = CW_FALSE;
CWBool gWTPFactoryReset = CW_FALSE;
int gWTPWaitTimeBeforeReset = 0;
int gWTPRejoinAcSeqNum;
CWMacAddress gWTPIfaceMac;
char *gWTPIface = NULL;
#ifdef CW_BUILD_X86
const char *gEzComAddr = NULL;
#else
const char *gEzComAddr = CW_DEF_EZCOM_URL; /* ezCom URL */
#endif
int gEzComCacheTime = 60;
#ifdef CW_BUILD_X86
int gWTPSimCapCode = 0x12;
char *gWTPSimSku = "INT";
int gWTPSimMaxStaPerWlan = 5;
int gWTPSimMaxScannedNumPerRadio = 30;
#endif
int gWTPLastReqSeqNum = -1;
CWBool gWTPDtlsHandshakeTimeout = CW_FALSE;
CWBool gWTPProxyModeEnable = CW_FALSE;

#ifdef CW_WTP_SWITCH
CWBool CWWTPEnterAC()
{
    char *buf = NULL, *pstr;
    int macNum = 0;
    int index, len = 0;
    CWMacAddress *mac;
    CWAcAddress acAddr;
    FILE *fp;

    if(!CWWTPBoardGetAcAddress(&acAddr) ||
       !CWWTPBoardGetRedundantManagedMacList(&macNum, &mac) ||
       !CWCreateAcAddrString(&acAddr, &pstr))
    {
        return CW_FALSE;
    }

    /* File that used to pass WTP exec command to redundant AC process */
    fp = fopen("/tmp/wtp_exec", "w");
    if(fp)
    {
        fputs(gWTPExecCmd, fp);
        fclose(fp);
    }

    if(macNum)
    {
        CW_CREATE_OBJECT_SIZE_ERR(buf, 16 + (macNum * 18),
                                  return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

        for(index = 0; index < macNum; index++)
        {
            if(index == 0)
            {
                len += sprintf(&buf[len], "--rednt_mac %02x:%02x:%02x:%02x:%02x:%02x", CW_MAC_PRINT_LIST(mac[index]));
            }
            else
            {
                len += sprintf(&buf[len], ",%02x:%02x:%02x:%02x:%02x:%02x", CW_MAC_PRINT_LIST(mac[index]));
            }
        }
    }

    /* Enable the Multicast to CPU */
    CWWTPSwitchEnableDiscoveryTrap(1);

    CWLog("Enter to AC Mode");

    CWDebugLog("%s --wtp_if %s --wtp_exec_file /tmp/wtp_exec --rednt_ac %s %s",
               gACExecCmd, gWTPIface, pstr, buf ? buf : "");

    CWSystemUnsafe("%s --wtp_if %s --wtp_exec_file /tmp/wtp_exec --rednt_ac %s %s",
                   gACExecCmd, gWTPIface, pstr, buf ? buf : "");

    CW_FREE_OBJECT(buf);
    CW_FREE_OBJECT(pstr);

    return CW_TRUE;
}
#endif

CWBool CWWTPSendMessage(CWProtocolMessage *messages, int fragmentsNum)
{
    int i;

    for(i = 0; i < fragmentsNum; i++)
    {
        if(!CWSecuritySend(gWTPSession,
                           messages[i].msg,
                           messages[i].offset))
        {
            CWDebugLog("Failure sending Request");

            for(; i < fragmentsNum; i++)
            {
                CW_FREE_PROTOCOL_MESSAGE(messages[i]);
            }
            CW_FREE_OBJECT(messages);

            return CW_FALSE;
        }
        CW_FREE_PROTOCOL_MESSAGE(messages[i]);
    }

    CW_FREE_OBJECT(messages);

    return CW_TRUE;
}

/*
 * Receive a message, that can be fragmented. This is useful not only for the Join State
 */
CWBool CWWTPReceiveMessage(CWProtocolMessage *msgPtr, int timeout)
{
    CWList fragments = NULL;
    int readBytes;
    char buf[CW_PACKET_BUFFER_SIZE];
    char *pkt;
    struct timespec timewait;
    CWBool crypted;

    CW_REPEAT_FOREVER
    {
        CWThreadMutexLock(&gInterfaceMutex);

        if(CWGetCountElementFromSafeList(gPacketReceiveList) == 0)
        {
            timewait.tv_sec = time(0) + timeout;
            timewait.tv_nsec = 0;

            if(!CWErr(CWWaitThreadConditionTimeout(&gInterfaceWait, &gInterfaceMutex, &timewait)))
            {
                CWThreadMutexUnlock(&gInterfaceMutex);
                CWDeleteList(&fragments, CWProtocolDestroyFragment);
                return CW_FALSE;
            }
        }

        pkt = CWGetHeadElementFromSafeList(gPacketReceiveList, &readBytes);
        if(!pkt)
        {
            CWThreadMutexUnlock(&gInterfaceMutex);
            CWDeleteList(&fragments, CWProtocolDestroyFragment);
            return CWErrorRaise(CW_ERROR_GENERAL, NULL);
        }

        /* Check this packet is encryted or not by the first byte */
        if(pkt[0] & CW_PACKET_CRYPT)
        {
            crypted = CW_TRUE;
        }
        else
        {
            crypted = CW_FALSE;
            pkt = CWRemoveHeadElementFromSafeList(gPacketReceiveList, &readBytes);
            CW_COPY_MEMORY(buf, pkt, readBytes);
            CW_FREE_OBJECT(pkt);
        }

        CWThreadMutexUnlock(&gInterfaceMutex);

        if(crypted)
        {
            if(!CWSecurityReceive(gWTPSession, buf, CW_BUFFER_SIZE, &readBytes))
            {
                CWDeleteList(&fragments, CWProtocolDestroyFragment);
                return CW_FALSE;
            }
        }

        CWBool dataFlag = CW_FALSE;
        if(!CWProtocolParseFragment(buf, readBytes, &fragments, msgPtr, &dataFlag))
        {
            if(CWErrorGetLastErrorCode() == CW_ERROR_NEED_RESOURCE) /* we need at least one more fragment */
            {
                continue;
            }
            else /* error */
            {
                CWDeleteList(&fragments, CWProtocolDestroyFragment);
                return CW_FALSE;
            }
        }

        /* the message is fully reassembled */
        return CW_TRUE;
    }
}

CWBool CWWTPSendAcknowledgedPacket(int seqNum,
                                   void *reqValuesPtr,
                                   CWBool(assembleFunc)(CWProtocolMessage **, int *, int, int, void*),
                                   CWBool(parseFunc)(char *, int, int, void *,int),
                                   CWBool(saveFunc)(void *),
                                   void *respValuesPtr,
                                   int retransmitTimer)
{
    CWProtocolMessage *messages = NULL;
    CWProtocolMessage msg = {NULL, 0, 0};
    int retransmissionCount = 0;
    int fragmentsNum = 0, i;
    struct timespec timewait;
    CWBool ret = CW_FALSE;

    if(!assembleFunc(&messages,
                     &fragmentsNum,
                     gWTPPathMTU,
                     seqNum,
                     reqValuesPtr))
    {
        return CW_FALSE;
    }

    do
    {
        CWDebugLog("Transmission Num:%d", retransmissionCount);

        for(i = 0; i < fragmentsNum; i++)
        {
            /*avoid to trassmit packet fast*/
            if( i>0 && gWTPPacketInterval >0 )
            {
              CWDebugLog("Packet delay(%d) us",gWTPPacketInterval);
              usleep(gWTPPacketInterval);
            }
            if(!CWSecuritySend(gWTPSession,
                               messages[i].msg,
                               messages[i].offset))
            {
                CWDebugLog("Failure sending Request");
                goto exit;
            }
        }

        timewait.tv_sec = time(0) + retransmitTimer;
        timewait.tv_nsec = 0;

wait_packet:
        CWThreadMutexLock(&gInterfaceMutex);

        if((CWGetCountElementFromSafeList(gPacketReceiveList) > 0) ||
           (CWWaitThreadConditionTimeout(&gInterfaceWait, &gInterfaceMutex, &timewait)))
        {
            CWErrorRaise(CW_ERROR_SUCCESS, NULL);
        }

        CWThreadMutexUnlock(&gInterfaceMutex);

        switch(CWErrorGetLastErrorCode())
        {
            case CW_ERROR_TIME_EXPIRED:
            {
                CWDebugLog("Retransmission time is over");
                retransmissionCount++;
                goto resend_pkt;
            }
            case CW_ERROR_SUCCESS:
            {
                /* there's something to read */
                if(!CWWTPReceiveMessage(&msg, gACInfoPtr->timer->fragment))
                {
                    CWDebugLog("Failure Receiving Response");
                    CW_FREE_PROTOCOL_MESSAGE(msg);

                    if(CWErrorGetLastErrorCode() == CW_ERROR_TIME_EXPIRED)
                    {
                        goto resend_pkt;
                    }
                    else
                    {
                        goto exit;
                    }
                }

                if(!parseFunc(msg.msg, msg.offset, seqNum, respValuesPtr,msg.combine_len))
                {
                    CWErrorHandleLast();
                    CW_FREE_PROTOCOL_MESSAGE(msg);

                    if(CWErrorGetLastErrorCode() == CW_ERROR_INVALID_FORMAT)
                    {
                        goto resend_pkt;
                    }
                    else
                    {
                        CWDebugLog("Failure Parsing Response");
                        goto exit;
                    }
                }

                CW_FREE_PROTOCOL_MESSAGE(msg);

                ret = saveFunc(respValuesPtr);
                goto exit;
            }
            case CW_ERROR_INTERRUPTED:
            {
                goto wait_packet;
            }
            default:
            {
                CWErrorHandleLast();
                goto exit;
            }
        }

resend_pkt:
        if(retransmissionCount++ >= gCWMaxRetransmit)
        {
            /* too many retransmissions */
            CWErrorRaise(CW_ERROR_NEED_RESOURCE, "Peer Dead");
            goto exit;
        }
    }
    while(1);

exit:
    for(i = 0; i < fragmentsNum; i++)
    {
        CW_FREE_PROTOCOL_MESSAGE(messages[i]);
    }
    CW_FREE_OBJECT(messages);

    return ret;
}

#define WTP_ARG_DISC_ADDR	 1
#define WTP_ARG_WORK_DIR	 2
#define WTP_ARG_LOG			 3
#define WTP_ARG_DEBUG_LOG	 4
#define WTP_ARG_LOG_SIZE	 5
#define WTP_ARG_INTERFACE    6
#define WTP_ARG_CERT_DIR     7
#define WTP_ARG_DAEMON       8
#define WTP_ARG_AC_ADDR      9
#define WTP_ARG_PROXY_MODE   10
#define WTP_ARG_CAP_CODE     11
#define WTP_ARG_SKU          12
#define WTP_ARG_UCI_LOG      13
#define WTP_ARG_EZ_COM_ADDR  14
#define WTP_ARG_AC_EXEC_FILE 15
#define WTP_ARG_EZ_COM_CACHE 16
#define WTP_ARG_MAX_STA_PER_WLAN 17
#define WTP_ARG_MTU_SIZE     18

static struct option long_options[] =
{
    {"if",			1,  0,  WTP_ARG_INTERFACE },
#ifdef CW_BUILD_X86
    {"cap_code",	1,  0,  WTP_ARG_CAP_CODE },
    {"sku",         1,  0,  WTP_ARG_SKU },
    {"max_wlan_sta", 1,  0,  WTP_ARG_MAX_STA_PER_WLAN },
#endif
    {"dir",			1,  0,  WTP_ARG_WORK_DIR },
    {"addr",		1,  0,  WTP_ARG_DISC_ADDR },
    {"log",			1,  0,  WTP_ARG_LOG },
    {"dlog",		1,  0,  WTP_ARG_DEBUG_LOG },
    {"log_size",	1,  0,  WTP_ARG_LOG_SIZE },
    {"cert_dir",    1,  0,  WTP_ARG_CERT_DIR },
    {"daemon",      1,  0,  WTP_ARG_DAEMON },
    {"ac",          1,  0,  WTP_ARG_AC_ADDR },
    {"proxy",       1,  0,  WTP_ARG_PROXY_MODE },
#ifdef CW_WTP_AP
    {"ucilog",      1,  0,  WTP_ARG_UCI_LOG },
#endif
    {"ezcom",       1,  0,  WTP_ARG_EZ_COM_ADDR },
    {"ezcom_cache", 1,  0,  WTP_ARG_EZ_COM_CACHE },
#ifdef CW_WTP_SWITCH
    {"ac_exec_file", 1,  0, WTP_ARG_AC_EXEC_FILE },
#endif
    {"mtu"         , 1,  0, WTP_ARG_MTU_SIZE},
    /* TO BE ADDED ... */
    {0,         		0,	0,  0 }
};

static const char *opt_name(int type)
{
    int i = 0;

    while(long_options[i].name)
    {
        if(type == long_options[i].val)
        {
            return long_options[i].name;
        }
        i++;
    }

    return NULL;
}

typedef struct
{
    int c;
    char *arg;
} opt_t;

static void CWWTPSignalHandler_SIGSEGV()
{
    CWLog("Catch SIGSEGV");

    exit(CW_EXIT_RESTART);
}

static void CWWTPSignalHandler_SIGUSR1()
{
    CWDebugLog("Catch SIGUSR1");
}

static void CWWTPSignalHandler_SIGUSR2()
{
    CWDebugLog("Catch SIGUSR2");
}

static CWBool CWWTPInitSigHandler()
{
    CWThreadSetSignals(SIG_BLOCK, 1, SIGUSR2);

    if(signal(SIGSEGV, CWWTPSignalHandler_SIGSEGV) == SIG_ERR)
    {
        return CW_FALSE;
    }

    if(signal(SIGUSR1, CWWTPSignalHandler_SIGUSR1) == SIG_ERR)
    {
        return CW_FALSE;
    }

    if(signal(SIGUSR2, CWWTPSignalHandler_SIGUSR2) == SIG_ERR)
    {
        return CW_FALSE;
    }

    return CW_TRUE;
}

#ifndef CW_LOAD_CONFIG_FILE
CWBool CWInitConfig()
{
    gNetworkPreferredFamily = CW_IPv4;

    if(gDefaultACAddressCount == 0)
    {
        gDefaultACAddressCount = 1;
        CW_CREATE_ARRAY_ERR(gDefaultACAddresses, gDefaultACAddressCount,
                            char*, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

        CW_CREATE_STRING_FROM_STRING_ERR(gDefaultACAddresses[0],
                                         CW_DEF_MULTICAST_ADDRESS,
                                         return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

#ifdef CW_DEBUGGING
        if(gEnabledDebugLog)
        {
            CW_PRINT_STRING_ARRAY(gDefaultACAddresses, gDefaultACAddressCount);
        }
#endif
    }

    return CW_TRUE;
}
#endif /* ! CW_LOAD_CONFIG_FILE */

static int read_to_buf(const char *filename, void *buf, int bufSize)
{
    int fd;
    ssize_t ret;

    // protect
    if(filename==0 || buf==0||bufSize==0)
    {
        // printf("Err: %s, input errors!\n", __FUNCTION__);
        return -1;
    }

    fd = open(filename, O_RDONLY);
    if(fd < 0)
    {
        printf("Err: %s failed to open file %s!\n", __FUNCTION__, filename);
        return -1;
    }
    ret = read(fd, buf, bufSize);
    close(fd);
    return ret;
}

static int capwap_get_pid(const char *proc, int skip_pid)
{
#ifndef READ_BUF_SIZE
#define READ_BUF_SIZE 128
#endif
    DIR *dir;
    struct dirent *next;
    char filename[READ_BUF_SIZE];
    char buffer[READ_BUF_SIZE];
    char name[READ_BUF_SIZE];
    int pid=0;
    // protect
    if(!proc)
    {
        // printf("Err: %s proc is NULL!\n", __FUNCTION__);
        return -1;
    }

    memset(filename, 0, sizeof(filename));
    memset(buffer, 0, sizeof(buffer));
    memset(name, 0, sizeof(name));

    dir = opendir("/proc");
    if(!dir) return -1;

    while((next = readdir(dir)) != NULL)
    {
        /* Must skip ".." since that is outside /proc */
        if(strcmp(next->d_name, "..") == 0)
            continue;

        /* If it isn't a number, we don't want it */
        if(!isdigit(*next->d_name))
            continue;

        /* 1~10 is system process */
        if(atoi(next->d_name) < 10)
            continue;

        sprintf(filename, "/proc/%s/status", next->d_name);

        //cfho 2007-0118, use read_to_buf API instead of open and close file here.
        if(!read_to_buf(filename, buffer, READ_BUF_SIZE-1))
        {
            continue;
        }

        /* Buffer should contain a string like "Name:   binary_name" */
        sscanf(buffer, "%*s %s", name);
        if(strcmp(name, proc) == 0)
        {
            pid=atoi(next->d_name);
            if ( pid == skip_pid )
            {
                pid = 0;
                continue;
            }
            closedir(dir);
            return pid;
        }
    }
    closedir(dir);
    return pid;
}

int main(int argc, char *const *argv)
{
    int opt_num = 0, c, i, addrIdx = 0;
    char *dir = NULL, *tmp = NULL, *forceAC = NULL;
    char work_dir[128];
    opt_t opt[32];
    int daemonMode = 1;
#ifdef CW_WTP_SWITCH
    int cmdLen = 0;
#endif
#ifdef CW_BUILD_X86
    uid_t uid = getuid();

    if(uid != 0)
    {
        fprintf(stderr, "ERROR: Require root permission\n");
        exit(EXIT_FAILURE);
    }
#endif

    int pid = getpid(), wtp_pid = -1;
    fprintf(stderr, "[%s:%d] wtp pid[%d] ###\n", __FUNCTION__, __LINE__, pid);

    while ( ( wtp_pid = capwap_get_pid("wtp", pid) ) > 0 )
    {
        fprintf(stderr, "[%s:%d] kill another wtp pid[%d] ###\n", __FUNCTION__, __LINE__, wtp_pid);
        kill(wtp_pid, SIGKILL);
        wtp_pid = -1;
    }

#ifdef CW_WTP_SWITCH
    /* Get wtp cmd string for this process */
    cmdLen = 0;
    for(i = 0; i < argc; i++)
    {
        cmdLen += strlen(argv[i]) + 1;
    }
    CW_CREATE_OBJECT_SIZE_ERR(gWTPExecCmd, cmdLen + 1,
    {
        fprintf(stderr, "ERROR: Out of memory\n"); return EXIT_FAILURE;
    });

    cmdLen = 0;
    for(i = 0; i < argc; i++)
    {
        cmdLen += sprintf(&gWTPExecCmd[cmdLen], "%s ", argv[i]);
    }
#endif

    opterr = 0;
    /* Parse options */
    while((c = getopt_long(argc, argv, "", long_options, NULL)) != -1)
    {
        switch(c)
        {
            case '?':
                if(isprint(optopt))
                {
                    fprintf(stderr, "ERROR: Unknown option -%c.\n", optopt);
                }
                else
                {
                    if(opt_name(optopt))
                    {
                        fprintf(stderr, "ERROR: Option --%s requires an argument.\n", opt_name(optopt));
                    }
                    else
                    {
                        fprintf(stderr, "ERROR: Unknown option `%s'.\n", argv[optind - 1]);
                    }
                }
                return EXIT_FAILURE;
            default:
                if(opt_num >= sizeof(opt) / sizeof(opt[0]))
                {
                    fprintf(stderr, "ERROR: Too many arguments\n");
                    return EXIT_FAILURE;
                }
                else
                {
                    opt[opt_num].c = c;
                    opt[opt_num].arg = optarg;
                    opt_num++;
                    if(c == WTP_ARG_DISC_ADDR)
                    {
                        gDefaultACAddressCount++;
                    }
                }
        }
    }

    if(gDefaultACAddressCount)
    {
        CW_CREATE_ARRAY_ERR(gDefaultACAddresses, gDefaultACAddressCount, char *,
        {
            fprintf(stderr, "ERROR: Out of memory\n");
            return EXIT_FAILURE;
        });
    }

    for(i = 0; i < opt_num; i++)
    {
        switch(opt[i].c)
        {
            case WTP_ARG_DISC_ADDR:
                CW_CREATE_STRING_FROM_STRING_ERR(gDefaultACAddresses[addrIdx], opt[i].arg,
                {
                    fprintf(stderr, "ERROR: Out of memory\n");
                    return EXIT_FAILURE;
                });
                addrIdx++;
                break;
            case WTP_ARG_WORK_DIR:
                dir = opt[i].arg;
                break;
            case WTP_ARG_INTERFACE:
                gWTPIface = opt[i].arg;
                break;
#ifdef CW_BUILD_X86
            case WTP_ARG_CAP_CODE:
                gWTPSimCapCode = strtol(opt[i].arg, NULL, 0);
                break;
            case WTP_ARG_SKU:
                gWTPSimSku = opt[i].arg;
                break;
            case WTP_ARG_MAX_STA_PER_WLAN:
                gWTPSimMaxStaPerWlan = atoi(opt[i].arg);
                if(gWTPSimMaxStaPerWlan < 0)
                {
                    fprintf(stderr, "ERROR: invalid value of --%s\n", opt_name(opt[i].c));
                    return EXIT_FAILURE;
                }
                break;
#endif
            case WTP_ARG_LOG:
                gEnabledLog = atoi(opt[i].arg) ? CW_TRUE : CW_FALSE;
                break;
            case WTP_ARG_DEBUG_LOG:
                gEnabledDebugLog = atoi(opt[i].arg);
                break;
            case WTP_ARG_LOG_SIZE:
                gMaxLogFileSize = atoi(opt[i].arg);
                break;
            case WTP_ARG_CERT_DIR:
                CW_CREATE_STRING_FROM_STRING_ERR(gWTPCertDir, opt[i].arg,
                {
                    fprintf(stderr, "ERROR: Out of memory\n");
                    return EXIT_FAILURE;
                });
                break;
            case WTP_ARG_DAEMON:
                daemonMode = atoi(opt[i].arg);
                break;
            case WTP_ARG_AC_ADDR:
                forceAC = opt[i].arg;
                break;
            case WTP_ARG_PROXY_MODE:
                gWTPProxyModeEnable = atoi(opt[i].arg) ? CW_TRUE : CW_FALSE;
                break;
#ifdef CW_WTP_AP
            case WTP_ARG_UCI_LOG:
                gWTPUciLogEnable = atoi(opt[i].arg) ? CW_TRUE : CW_FALSE;
                break;
#endif
            case WTP_ARG_EZ_COM_ADDR:
                gEzComAddr = opt[i].arg;
                break;
            case WTP_ARG_EZ_COM_CACHE:
                gEzComCacheTime = atoi(opt[i].arg);
                break;
#ifdef CW_WTP_SWITCH
            case WTP_ARG_AC_EXEC_FILE:
                if(!CWCreateStringByFile(opt[i].arg, &gACExecCmd))
                {
                    fprintf(stderr, "ERROR: open %s failed\n", opt[i].arg);
                    return EXIT_FAILURE;
                }
                break;
#endif
             case WTP_ARG_MTU_SIZE:
                gWTPInitMTU = atoi(opt[i].arg);
                break;
        }
    }

    if(!dir)
    {
        dir = getcwd(work_dir, sizeof(work_dir));
    }

    if(!gWTPCertDir)
    {
        CW_CREATE_STRING_FROM_STRING_ERR(gWTPCertDir, dir,
        {
            fprintf(stderr, "ERROR: Out of memory\n");
            return EXIT_FAILURE;
        });
    }

    if(gWTPCertDir[strlen(gWTPCertDir) - 1] != '/')
    {
        CW_CREATE_STRING_ERR(tmp, strlen(gWTPCertDir) + 1,
        {
            fprintf(stderr, "ERROR: Out of memory\n");
            return EXIT_FAILURE;
        });

        strcpy(tmp, gWTPCertDir);
        strcat(tmp, "/");
        CW_FREE_OBJECT(gWTPCertDir);
        gWTPCertDir = tmp;
    }

    /* Daemon mode */
    if(daemonMode && daemon(0, 1) < 0)
    {
        fprintf(stderr, "ERROR: Could not run as daemon mode\n");
        exit(EXIT_FAILURE);
    }

    if(chdir(dir) != 0)
    {
        fprintf(stderr, "ERROR: Could not change work directory to %s\n", dir);
        exit(EXIT_FAILURE);
    }

    if(gEzComAddr && gEzComAddr[0] != '\0')
    {
        CWSaveStringToFile("/tmp/EzrUrl", gEzComAddr);
    }

    /* fork a child process to execute shell command */
    if(!CWForkShellProcess())
    {
        exit(EXIT_FAILURE);
    }

    if(!CWRespawn())
    {
        exit(EXIT_FAILURE);
    }

    if(!CWLogInitFile(WTP_LOG_FILE_NAME))
    {
        exit(EXIT_FAILURE);
    }

    if(!CWErrorHandlingInitLib())
    {
        exit(EXIT_FAILURE);
    }

    if(!CWWTPInitSigHandler())
    {
        CWLog("Could not init signal hander");
        exit(EXIT_FAILURE);
    }

    /* Capwap receive packets list */
    if(!CWErr(CWCreateSafeList(&gPacketReceiveList)))
    {
        CWLog("Can't start WTP");
        exit(EXIT_FAILURE);
    }

    CWCreateThreadMutex(&gInterfaceMutex);

    CWSetMutexSafeList(gPacketReceiveList, &gInterfaceMutex);

    CWCreateThreadCondition(&gInterfaceWait);

    CWSetConditionSafeList(gPacketReceiveList, &gInterfaceWait);

    if(!gWTPIface && !CWWTPBoardGetDefaultInterfaceName(&gWTPIface))
    {
        CWLog("Can't get WTP Interface Name");
        exit(EXIT_FAILURE);
    }

    if(!CWNetworkGetInterfaceMacAddress(gWTPIface, gWTPIfaceMac))
    {
        CWLog("Can't get WTP Interface MAC");
        exit(EXIT_FAILURE);
    }

    /* Random seed is generated by mac address */
    CWRandomInitLibBySeed(((gWTPIfaceMac[0] ^ gWTPIfaceMac[5]) << 24) |
                          ((gWTPIfaceMac[1] ^ gWTPIfaceMac[4]) << 16) |
                          (gWTPIfaceMac[2] ^ gWTPIfaceMac[3]));

    CWThreadSetSignals(SIG_BLOCK, 1, SIGALRM);
    if(timer_init() == 0)
    {
        CWLog("Can't init timer module");
        exit(EXIT_FAILURE);
    }

    if(!CWErr(CWSecurityInitLib()))
    {
        CWLog("Can't init Security Lib");
        exit(EXIT_FAILURE);
    }

#ifdef CW_LOAD_CONFIG_FILE
    if(!CWErr(CWWTPLoadConfiguration()))
    {
        CWLog("Can't Load Configuration");
        exit(EXIT_FAILURE);
    }
#else
    if(!CWErr(CWInitConfig()))
    {
        CWLog("Can't Init Configuration");
        exit(EXIT_FAILURE);
    }
#endif

    CWWTPCheckCurrentCertIsDefault();

    CWDebugLog("Init WTP Info");

    if(!CWWTPInitConfiguration())
    {
        CWLog("Error Init Configuration");
        exit(EXIT_FAILURE);
    }

    if(forceAC)
    {
        CWWTPBoardSetForceAcAddress(forceAC);
    }

    if(!CWWTPExternalSignalHandlerStart())
    {
        CWLog("Signal Handler Task Init Failed");
        exit(EXIT_FAILURE);
    }

    CW_ZERO_MEMORY(&gWTPCfgResult, sizeof(CWWtpCfgResult));

    CWLog("WTP is running...");

    /* start CAPWAP state machine */
    CW_REPEAT_FOREVER
    {
        switch(gWTPCurrentState)
        {
            case CW_ENTER_DISCOVERY:
                /* Terminate Rx thread */
                CWWTPStopRxTask();
                gWTPCurrentState = CWWTPEnterDiscovery();
                if(gWTPCurrentState != CW_ENTER_DISCOVERY)
                {
                    gWTPPreviousState = CW_ENTER_DISCOVERY;
                }
                break;
            case CW_ENTER_SULKING:
                gWTPCurrentState = CWWTPEnterSulking();
                if(gWTPCurrentState != CW_ENTER_SULKING)
                {
                    gWTPPreviousState = CW_ENTER_SULKING;
                }
                break;
            case CW_ENTER_JOIN:
                gWTPCurrentState = CWWTPEnterJoin();
                if(gWTPCurrentState != CW_ENTER_JOIN)
                {
                    gWTPPreviousState = CW_ENTER_JOIN;
                }
                break;
            case CW_ENTER_CONFIGURE:
                gWTPCurrentState = CWWTPEnterConfigure();
                if(gWTPCurrentState != CW_ENTER_CONFIGURE)
                {
                    gWTPPreviousState = CW_ENTER_CONFIGURE;
                }
                break;
            case CW_ENTER_DATA_CHECK:
                gWTPCurrentState = CWWTPEnterDataCheck();
                if(gWTPCurrentState != CW_ENTER_DATA_CHECK)
                {
                    gWTPPreviousState = CW_ENTER_DATA_CHECK;
                }
                break;
            case CW_ENTER_RUN:
                if(gWTPProxyModeEnable)
                {
                    CWWTPStartProxyMode();
                }
                gWTPCurrentState = CWWTPEnterRun();
                if(gWTPCurrentState != CW_ENTER_RUN)
                {
                    CWWTPStopProxyMode();
                    gWTPPreviousState = CW_ENTER_RUN;
                }
                break;
            case CW_ENTER_IMAGE:
                gWTPCurrentState = CWWTPEnterImage();
                if(gWTPCurrentState != CW_ENTER_IMAGE)
                {
                    gWTPPreviousState = CW_ENTER_IMAGE;
                }
                break;
            case CW_ENTER_CERT_CHECK:
                gWTPCurrentState = CWWTPEnterCertReset();
                if(gWTPCurrentState != CW_ENTER_CERT_CHECK)
                {
                    gWTPPreviousState = CW_ENTER_CERT_CHECK;
                }
                break;
            case CW_ENTER_RESET:
                if(gWTPWaitTimeBeforeReset)
                {
                    CWLog("Wait %d second before reset", gWTPWaitTimeBeforeReset);
                    CWWaitSec(gWTPWaitTimeBeforeReset);
                }
                if(gWTPFactoryReset)
                {
                    CWWTPBoardFactoryReset();
                }
                else
                {
                    CWWTPBoardReboot(); /* Reboot device */
                }
                return 0;
#ifdef CW_WTP_SWITCH
            case CW_ENTER_AC:
                CWWTPEnterAC();
                return 0;
#endif
            default:
                CWLog("Unhandled CAPWAP state %d", gWTPCurrentState);
                CW_ASSERT(0);
                //   case CW_QUIT:
                //       CWLog("Enter CW_QUIT state");
                //       CWWTPDestroy();
                //       return 0;
        }
    }
}

unsigned int CWGetSeqNum()
{
    static unsigned int seqNum = 0;

    if(seqNum == CW_MAX_SEQ_NUM)
    {
        seqNum = 0;
    }
    else
    {
        seqNum++;
    }
    return seqNum;
}

int CWGetFragmentID()
{
    static int fragID = 0;
    return fragID++;
}

#ifdef CW_WTP_AP
CWBool CWWTPGetRadioIndex(CWRadioFreqType radioType, int *radioIdx)
{
    unsigned int capCode;

    if(!CWWTPBoardGetCapCode(&capCode))
    {
        return CW_FALSE;
    }

    return CWGetRadioIndex(radioType, capCode, radioIdx);
}

CWBool CWWTPGetRadioType(int radioIdx, CWRadioFreqType *radioType)
{
    unsigned int capCode;

    if(!CWWTPBoardGetCapCode(&capCode))
    {
        return CW_FALSE;
    }

    return CWGetRadioType(radioType, capCode, radioIdx);
}

CWBool CWWTPCheckAnySSIDEnable(CWRadioFreqType radioType)
{
    int radioIdx, wlanIdx, enable;

    if(!CWWTPGetRadioIndex(radioType, &radioIdx))
    {
        return CW_FALSE;
    }

    for(wlanIdx = 0; wlanIdx < CWWTPBoardGetMaxRadioWlans(radioIdx); wlanIdx++)
    {
        if(CWWTPBoardGetWlanEnableCfg(radioIdx, wlanIdx, &enable) && enable)
        {
            return CW_TRUE;
        }
    }

    return CW_FALSE;
}

CWBool CWWTPResetAutoTxPower()
{
    int radioIdx, txPower;
    CWRadioFreqType radioType;

    for(radioIdx = 0; radioIdx < CWWTPBoardGetMaxRadio(); radioIdx++)
    {
        if (CWWTPBoardGetRadioTxPowerCfg(radioIdx, &txPower) &&
            txPower == CW_RADIO_TX_POWER_AUTO &&
            CWWTPGetRadioType(radioIdx, &radioType))
        {
            CWWTPBoardSetRadioAutoTxPowerStrength(radioType, 100);
        }
    }

    return CW_FALSE;
}
#endif /* CW_WTP_AP */

CWThreadMutex WTPSendPacketMutex = CW_MUTEX_INITIALIZER;

CWBool CWWTPSendMsg(CWProtocolMessage *messages, int fragmentsNum, CWBool encrypt, CWBool freeMsg)
{
    int i;
    CWBool ret = CW_TRUE;

    CWDebugLog("[%02X:%02X:%02X:%02X:%02X:%02X] CWWTPSendMsg with fragmentsNum %d",
               CW_MAC_PRINT_LIST(gWTPIfaceMac), fragmentsNum);

    CWThreadMutexLock(&WTPSendPacketMutex);

    for(i = 0; i < fragmentsNum; i++)
    {
        if(encrypt)
        {
            if(!CWSecuritySend(gWTPSession, messages[i].msg, messages[i].offset))
            {
                CWLog("Failure sending encrypt messages");
                ret = CW_FALSE;
                break;
            }
        }
        else
        {
            if(!CWNetworkSendUnsafeConnected(gWTPSocket, messages[i].msg, messages[i].offset,
                                             gACInfoPtr->controllerId))
            {
                CWLog("Failure sending messages");
                ret = CW_FALSE;
                break;
            }
        }
    }

    CWThreadMutexUnlock(&WTPSendPacketMutex);

    if(freeMsg)
    {
        for(i = 0; i < fragmentsNum; i++)
        {
            CW_FREE_PROTOCOL_MESSAGE(messages[i]);
        }
        CW_FREE_OBJECT(messages);
    }

    return ret;
}

CWBool CWWTPUpdateRequestSeqNum(unsigned char seqNum)
{
    if(gWTPLastReqSeqNum > 0)
    {
        if((gWTPLastReqSeqNum == CW_MAX_SEQ_NUM && seqNum != 0) ||
           (gWTPLastReqSeqNum != CW_MAX_SEQ_NUM && seqNum != gWTPLastReqSeqNum + 1))
        {
            CWLog("[%02X:%02X:%02X:%02X:%02X:%02X] Receive a request msg with unexpected seqNum %u"
                  ", next seqNum should be %d",
                  CW_MAC_PRINT_LIST(gWTPIfaceMac), seqNum,
                  gWTPLastReqSeqNum == CW_MAX_SEQ_NUM ? 0 : gWTPLastReqSeqNum + 1);
            return CW_FALSE;
        }
    }

    gWTPLastReqSeqNum = seqNum;
    return CW_TRUE;
}

#ifdef CW_LOAD_CONFIG_FILE
/*
 * Parses config file and inits WTP configuration.
 */
CWBool CWWTPLoadConfiguration()
{
    CWDebugLog("WTP Loads Configuration");

    /* get saved preferences */
    if(!CWErr(CWParseConfigFile()))
    {
        return CW_FALSE;
    }


    if(gDefaultACAddressCount == 0)
    {
        CWLog("No Default AC address");
        return CWErrorRaise(CW_ERROR_NEED_RESOURCE, "No AC Configured");
    }

    return CW_TRUE;
}
#endif

void CWWTPFreeDiscoveryACList()
{
    int i;

    for(i = 0; i < gDiscoveryACCount; i++)
    {
        CW_FREE_OBJECT(gDiscoveryACList[i].addrStr);
    }

    gDiscoveryACCount = 0;

    CW_FREE_OBJECT(gDiscoveryACList);
}

CWBool CWWTPGetDiscoveryACList()
{
    int i, j;
    CWHostName host;
    CWHostName *acListCfg = NULL;
    int acListCount = 0;
    unsigned int ipVal, ipLocalVal;
    char *forceAcAddrStr = NULL;
    char *ezComAcAddrStr = NULL;
    char *ezComAcLanAddrStr = NULL;
    char *dhcpAcAddrStr=NULL;
    char *lastAcAddrStr = NULL;
    char *tmpAcAddrStr, *c;
    int uptime;
    static char ezComAcCache[32] = "";
    static char ezComLanAcCache[32] = "";
    static int ezComQueryTime = 0;
    CWBool forceAC = CW_FALSE;
    unsigned short port;
    int controllerId;
    CWAcAddress acAddr;

    CWWTPFreeDiscoveryACList();

    if(CWWTPBoardGetForceAcAddress(host) && host[0] != '\0' && CWParseAcAddrString(host, &acAddr))
    {
        /* if force AC is specified, lastAcAddrStr is always force AC address */
        gDiscoveryACCount = 1;
        forceAC = CW_TRUE;

        if(!CWCreateAcAddrString(&acAddr, &forceAcAddrStr))
        {
            goto error;
        }

        CWDebugLog("Force AC = %s", forceAcAddrStr);
    }
    else
    {
        gDiscoveryACCount = gDefaultACAddressCount;

        if(CWWTPBoardGetAcAddress(&acAddr) && acAddr.hostName[0] != '\0')
        {
            unsigned ip, mask;
            CWBool sameSubnet = CW_FALSE;

            ipVal = inet_addr(acAddr.hostName);

            if(CWNetworkGetInterfaceAddressMask(gWTPIface, &ip, &mask))
            {
                sameSubnet = ((ip & mask) == (ipVal & mask));
            }

            /* if last-join AC is in the same subnet, we can ignore this address because
               it could be found from multicast discovery */
            if(!sameSubnet)
            {
                gDiscoveryACCount++;
                if(!CWCreateAcAddrString(&acAddr, &lastAcAddrStr))
                {
                    goto error;
                }

                CWDebugLog("Last join AC = %s", lastAcAddrStr);
            }
            else
            {
                CWDebugLog("Last join AC %u.%u.%u.%u is in the same subnet",
                           CW_IPV4_PRINT_LIST(ipVal));
            }
        }

        /* To reduce the loading of ezCom server, wtp will cache the query result every 30 seconds */
        uptime = CWGetUptime();

        CWDebugLog("gEzComCacheTime %d ezComQueryTime %d uptime %d",
                   gEzComCacheTime, ezComQueryTime, uptime);

        if(gEzComCacheTime > 0 &&
           ezComQueryTime > 0 &&
           uptime > ezComQueryTime &&
           uptime - ezComQueryTime < gEzComCacheTime)
        {
            if(ezComAcCache[0] != '\0')
            {
                CW_CREATE_STRING_FROM_STRING_ERR(ezComAcAddrStr, ezComAcCache,
                                                 CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
                                                 goto error;);
                gDiscoveryACCount++;
            }
            else
            {
                CWDebugLog("No cached ezCom AC");
            }

            if(ezComLanAcCache[0] != '\0')
            {
                CW_CREATE_STRING_FROM_STRING_ERR(ezComAcLanAddrStr, ezComLanAcCache,
                                                 CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
                                                 goto error;);
                gDiscoveryACCount++;
            }
            else
            {
                CWDebugLog("No cached ezCom AC LAN addr");
            }
        }
        else if(CWWTPQueryEzCom(&ipVal, &ipLocalVal, &port, &controllerId))
        {
            CWSetAcAddr(ipVal, port, controllerId, &acAddr);
            if(!CWCreateAcAddrString(&acAddr, &ezComAcAddrStr))
            {
                goto error;
            }
            gDiscoveryACCount++;
            CWDebugLog("ezCom registered AC = %s", ezComAcAddrStr);
            strcpy(ezComAcCache, ezComAcAddrStr);

            if(ipVal != ipLocalVal)
            {
                /* Lan AC address should always have no controllerId */
                CWSetAcAddr(ipLocalVal, port, 0, &acAddr);
                if(!CWCreateAcAddrString(&acAddr, &ezComAcLanAddrStr))
                {
                    goto error;
                }
                gDiscoveryACCount++;
                CWDebugLog("ezCom registered AC LAN addr = %s", ezComAcLanAddrStr);
                strcpy(ezComLanAcCache, ezComAcLanAddrStr);
            }
            else
            {
                ezComLanAcCache[0] = '\0';
            }

            ezComQueryTime = uptime;
        }
        else /* query failed */
        {
            ezComQueryTime = uptime;
            ezComAcCache[0] = '\0';
            ezComLanAcCache[0] = '\0';

            CWDebugLog("No ezCom registered AC");
        }

        /* remove duplicated address */
        if(ezComAcAddrStr && lastAcAddrStr && !strcmp(ezComAcAddrStr, lastAcAddrStr))
        {
            CW_FREE_OBJECT(lastAcAddrStr);
            gDiscoveryACCount--;
        }

        if(ezComAcLanAddrStr && lastAcAddrStr && !strcmp(ezComAcLanAddrStr, lastAcAddrStr))
        {
            CW_FREE_OBJECT(lastAcAddrStr);
            gDiscoveryACCount--;
        }
    }

    /*ac ip form dhcp option*/
    if( CWWTPBoardGetAcAddressWithDhcpOption( &dhcpAcAddrStr ) )
    {
       if(dhcpAcAddrStr && lastAcAddrStr && !strcmp(dhcpAcAddrStr, lastAcAddrStr))
        {
            CW_FREE_OBJECT(lastAcAddrStr);
        }
        else
        {
           gDiscoveryACCount++;
        }
    }

    /* get the configured AC list */
    if(CWWTPBoardGetAcListCfg(&acListCount, &acListCfg))
    {
        gDiscoveryACCount += acListCount;
    }

    /* remove duplicated address with ac list config */
    for(i = 0; i < acListCount; i++)
    {
        if(!CWParseAcAddrString(acListCfg[i], &acAddr))
        {
            continue;
        }

        if(!CWCreateAcAddrString(&acAddr, &tmpAcAddrStr))
        {
            goto error;
        }

        if(ezComAcAddrStr && !strcmp(ezComAcAddrStr, tmpAcAddrStr))
        {
            acListCfg[i][0] = '\0';
            gDiscoveryACCount--;
        }

        if(ezComAcLanAddrStr && !strcmp(ezComAcLanAddrStr, tmpAcAddrStr))
        {
            acListCfg[i][0] = '\0';
            gDiscoveryACCount--;
        }

        if(lastAcAddrStr && !strcmp(lastAcAddrStr, tmpAcAddrStr))
        {
            CW_FREE_OBJECT(lastAcAddrStr);
            gDiscoveryACCount--;
        }

        if(forceAcAddrStr && !strcmp(forceAcAddrStr, tmpAcAddrStr))
        {
            acListCfg[i][0] = '\0';
            gDiscoveryACCount--;
        }

        if(dhcpAcAddrStr && !strcmp(dhcpAcAddrStr, tmpAcAddrStr))
        {
            acListCfg[i][0] = '\0';
            gDiscoveryACCount--;
        }

        CW_FREE_OBJECT(tmpAcAddrStr);
    }

    CW_CREATE_ZERO_ARRAY_ERR(gDiscoveryACList,
                             gDiscoveryACCount,
                             CWACDescriptor,
                             CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
                             goto error;);

    i = 0;

    /* First priority is force AC address */
    if(forceAcAddrStr)
    {
        gDiscoveryACList[i].addrStr = forceAcAddrStr;
        gDiscoveryACList[i].type = CW_MSG_ELEMENT_DISCOVERY_TYPE_CONFIGURED;
        forceAcAddrStr = NULL;
        CWDebugLog("gDiscoveryACList[%d]=%s from force AC", i, gDiscoveryACList[i].addrStr);
        i++;
    }

    /* Second priority is the registered AC LAN addr from ezCom */
    if(ezComAcLanAddrStr)
    {
        gDiscoveryACList[i].addrStr = ezComAcLanAddrStr;
        gDiscoveryACList[i].type = CW_MSG_ELEMENT_DISCOVERY_TYPE_EZCOM;
        ezComAcLanAddrStr = NULL;
        CWDebugLog("gDiscoveryACList[%d]=%s from ezCom LAN addr", i, gDiscoveryACList[i].addrStr);
        i++;
    }

    /* Third priority is the registered AC from ezCom */
    if(ezComAcAddrStr)
    {
        gDiscoveryACList[i].addrStr = ezComAcAddrStr;
        gDiscoveryACList[i].type = CW_MSG_ELEMENT_DISCOVERY_TYPE_EZCOM;
        ezComAcAddrStr = NULL;
        CWDebugLog("gDiscoveryACList[%d]=%s from ezCom", i, gDiscoveryACList[i].addrStr);
        i++;
    }

     /* Fourth priority is the registered AC from ezCom */
    if(dhcpAcAddrStr)
    {
        gDiscoveryACList[i].addrStr = dhcpAcAddrStr;
        gDiscoveryACList[i].type = CW_MSG_ELEMENT_DISCOVERY_TYPE_DHCP;
        dhcpAcAddrStr = NULL;
        CWDebugLog("gDiscoveryACList[%d]=%s# from dhcp option", i, gDiscoveryACList[i].addrStr);
        i++;
    }


    /* AC list saved in configuration */
    for(j = 0; j < acListCount; j++)
    {
        if(acListCfg[j][0] != '\0')
        {
            if(!CWParseAcAddrString(acListCfg[j], &acAddr))
            {
                continue;
            }

            if(!CWCreateAcAddrString(&acAddr, &tmpAcAddrStr))
            {
                goto error;
            }

            gDiscoveryACList[i].addrStr = tmpAcAddrStr;
            gDiscoveryACList[i].type = CW_MSG_ELEMENT_DISCOVERY_TYPE_AC_REFER;
            CWDebugLog("gDiscoveryACList[%d]=%s from AC List configuration", i, gDiscoveryACList[i].addrStr);
            i++;
        }
    }
    CW_FREE_OBJECT(acListCfg);

    /* Last priority is last connected AC address */
    if(lastAcAddrStr)
    {
        gDiscoveryACList[i].addrStr = lastAcAddrStr;
        gDiscoveryACList[i].type = CW_MSG_ELEMENT_DISCOVERY_TYPE_UNKNOWN;
        lastAcAddrStr = NULL;
        CWDebugLog("gDiscoveryACList[%d]=%s from last AC", i, gDiscoveryACList[i].addrStr);
        i++;
    }

    /* remove # character and get controllerId for each AC address */
    for(j = 0; j < i; j++)
    {
        c = strchr(gDiscoveryACList[j].addrStr, '#');
        if(c)
        {
            *c = '\0';
            gDiscoveryACList[j].controllerId = atoi(c + 1);
        }
    }

    /* If force AC is specified, wtp should not send multicast discovery. */
    if(!forceAC)
    {
        /* Lowest priority is the multicast address read from wtp.config */
        for(j = 0; j < gDefaultACAddressCount; j++)
        {
            CW_CREATE_STRING_FROM_STRING_ERR(gDiscoveryACList[i].addrStr, gDefaultACAddresses[j],
                                             CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
                                             goto error;);
            gDiscoveryACList[i].type = CW_MSG_ELEMENT_DISCOVERY_TYPE_UNKNOWN;
            CWDebugLog("gDiscoveryACList[%d]=%s from mulsticast", i, gDiscoveryACList[i].addrStr);
            i++;
        }
    }

    return CW_TRUE;

error:
    CWLog("CWWTPGetDiscoveryACList failed");

    CW_FREE_OBJECT(forceAcAddrStr);
    CW_FREE_OBJECT(lastAcAddrStr);
    CW_FREE_OBJECT(ezComAcAddrStr);
    CW_FREE_OBJECT(ezComAcLanAddrStr);
    CW_FREE_OBJECT(acListCfg);
    CW_FREE_OBJECT(dhcpAcAddrStr);

    if(gDiscoveryACList)
    {
        for(i = 0; i < gDiscoveryACCount; i++)
        {
            CW_FREE_OBJECT(gDiscoveryACList[i].addrStr);
        }
        CW_FREE_OBJECT(gDiscoveryACList);
        gDiscoveryACCount = 0;
    }

    return CW_FALSE;
}

void CWWTPDestroy()
{
    int i;

    CWLog("Destroy WTP");

    for(i = 0; i < gDiscoveryACCount; i++)
    {
        CW_FREE_OBJECT(gDiscoveryACList[i].addrStr);
    }

    timer_destroy();

    CW_FREE_OBJECT(gDiscoveryACList);
    CW_FREE_OBJECTS_ARRAY(gDefaultACAddresses, gDefaultACAddressCount);
#ifdef CW_WTP_AP
    CW_FREE_OBJECT(gRadiosInfo.info);
#endif
}

CWBool CWWTPInitConfiguration()
{
#ifdef CW_WTP_AP
    int i;
#endif

    if(!CWWTPBoardInitConfiguration())
    {
        return CW_FALSE;
    }

    CWWTPResetRebootStatistics(&gWTPRebootStatistics);

#ifdef CW_WTP_AP
    gRadiosInfo.radioCount = CWWTPGetMaxRadios();
    if(gRadiosInfo.radioCount)
    {
        CW_CREATE_ARRAY_ERR(gRadiosInfo.info, gRadiosInfo.radioCount, CWWTPRadioInfoValues,
                            return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

        for(i = 0; i < gRadiosInfo.radioCount; i++)
        {
            gRadiosInfo.info[i].radioID = i + 1;
            /* gRadiosInfo.radiosInfo[i].numEntries = 0; */
            gRadiosInfo.info[i].decryptErrorMACAddressList = NULL;
            gRadiosInfo.info[i].reportInterval = CW_REPORT_INTERVAL_DEFAULT;
            /* Default value for CAPWAP */
            gRadiosInfo.info[i].adminState = ENABLED;
            gRadiosInfo.info[i].operationalState = ENABLED;
            gRadiosInfo.info[i].operationalCause = OP_NORMAL;
            CWWTPResetRadioStatistics(&(gRadiosInfo.info[i].statistics));
        }
    }
    else
    {
        gRadiosInfo.info = NULL;
    }

    for(i = 0; i < gRadiosInfo.radioCount; i++)
    {
        CWWTPBoardSetRadioAutoChannelSelectionACS(i, CW_FALSE);
    }
#endif
    return CW_TRUE;
}

/*
 * Manage DTLS packets.
 */
static CWThread gWTPRxThread;
static volatile CWBool gWTPRxThreadRun = CW_FALSE;

static CW_THREAD_RETURN_TYPE CWWTPRxTask(void *arg)
{
    int 			readBytes;
    char 			buf[CW_PACKET_BUFFER_SIZE];
    CWSocket 		sockDTLS = (CWSocket)(long)arg;
    CWNetworkLev4Address	addr;
    char 			*pData;

    CWDebugLog("CWWTPRxTask run");

    while(gWTPRxThreadRun)
    {
        if(!CWErr(CWNetworkReceiveUnsafe(sockDTLS,
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

        /* Clone data packet */
        CW_CREATE_OBJECT_SIZE_ERR(pData, readBytes, { CWLog("Out Of Memory"); break; });
        CW_COPY_MEMORY(pData, buf, readBytes);

        CWLockSafeList(gPacketReceiveList);

        CWAddElementToSafeListTail(gPacketReceiveList, pData, readBytes);

        CWUnlockSafeList(gPacketReceiveList);
    }

    /* Clear remaining packets in list */
    CWLockSafeList(gPacketReceiveList);

    while(CWGetCountElementFromSafeList(gPacketReceiveList))
    {
        pData = (char *)CWRemoveHeadElementFromSafeList(gPacketReceiveList, &readBytes);
        CW_FREE_OBJECT(pData);
    }

    CWUnlockSafeList(gPacketReceiveList);

    CWErrorFree();

    CWDebugLog("CWWTPRxTask exit");
    return NULL;
}

CWBool CWWTPStartRxTask()
{
    if(!gWTPRxThreadRun)
    {
        gWTPRxThreadRun = CW_TRUE;
        if(!CWErr(CWCreateThread(&gWTPRxThread,
                                 CWWTPRxTask,
                                 (void *)(long)gWTPSocket)))
        {
            CWLog("Error starting Thread that receive DTLS packet");
            gWTPRxThreadRun = CW_FALSE;
            return CW_FALSE;
        }
    }
    return CW_TRUE;
}

void CWWTPStopRxTask()
{
    if(gWTPRxThreadRun)
    {
        gWTPRxThreadRun = CW_FALSE;
        CWNetworkCloseSocket(gWTPSocket);
        CWDebugLog("Waiting DTSL Rx Thread exit...");
        CWThreadJoin(gWTPRxThread);
    }
}

int CWWTPSecurityRxCB(void *cbRxArg, char **bufPtr)
{
    int size;
    int milliSec;

    CWLockSafeList(gPacketReceiveList);

    /* waiting packet happens only when DTLS handshake */
    while(CWGetCountElementFromSafeList(gPacketReceiveList) == 0)
    {
        CWDebugLog("Waiting DTLS handshake packet..");

        gWTPDtlsHandshakeTimeout = CW_FALSE;
        milliSec = 1000 * gACInfoPtr->timer->dtlsSetup;

        if(!CWWaitElementFromSafeListTimeout(gPacketReceiveList, milliSec))
        {
            CWUnlockSafeList(gPacketReceiveList);

            CWLog("Could not receive DTLS handshake packet");

            gWTPDtlsHandshakeTimeout = CW_TRUE;

            return -1;
        }
    }

    *bufPtr = (char *)CWRemoveHeadElementFromSafeList(gPacketReceiveList, &size);

    CWUnlockSafeList(gPacketReceiveList);

    return size;
}

static CWThread gWTPKeepAliveThread;
static volatile CWBool gWTPKeepAliveThreadRun = CW_FALSE;
static CWThreadMutex gWTPKeepAliveMutex = CW_MUTEX_INITIALIZER;
static CWThreadCondition gWTPKeepAliveCond = CW_COND_INITIALIZER;

static CWBool CWWTPSendKeepAliveRequest()
{
    CWProtocolMessage *messages = NULL;
    int fragmentsNum;

    if(!(CWAssembleMessage(&messages,
                           &fragmentsNum,
                           gWTPPathMTU,
                           CWGetSeqNum(),
                           CW_MSG_TYPE_VALUE_KEEP_ALIVE_REQUEST,
                           NULL,
                           0,
                           NULL,
                           0,
                           CW_PACKET_PLAIN
                          )))
    {
        return CW_FALSE;
    }

    return CWWTPSendMsg(messages, fragmentsNum, CW_FALSE, CW_TRUE);
}

static CW_THREAD_RETURN_TYPE CWWTPKeepAliveTask(void *arg)
{
    struct timespec timeout;

    CWDebugLog("CWWTPKeepAliveTask start");

    CWThreadMutexLock(&gWTPKeepAliveMutex);

    do
    {
        /* wait for next polling time */
        timeout.tv_sec = time(NULL) + gWTPEchoInterval; /* seconds */
        timeout.tv_nsec = 0; /* nanoseconds */

        if(!CWWaitThreadConditionTimeout(&gWTPKeepAliveCond, &gWTPKeepAliveMutex, &timeout))
        {
            if(CWErrorGetLastErrorCode() == CW_ERROR_TIME_EXPIRED)
            {
                CWDebugLog("Send keep alive packet");

                CWWTPSendKeepAliveRequest();
            }
        }
    }
    while(gWTPKeepAliveThreadRun);

    CWThreadMutexUnlock(&gWTPKeepAliveMutex);

    CWDebugLog("CWWTPKeepAliveTask exit");

    CWErrorFree();

    return (void *) 0;
}

CWBool CWWTPStartKeepAliveTask()
{
    if(!gWTPKeepAliveThreadRun)
    {
        gWTPKeepAliveThreadRun = CW_TRUE;
        if(!CWErr(CWCreateThread(&gWTPKeepAliveThread,
                                 CWWTPKeepAliveTask,
                                 NULL)))
        {
            CWLog("Error starting Thread that send Keep Alive packet");
            gWTPKeepAliveThreadRun = CW_FALSE;
            return CW_FALSE;
        }
    }

    return CW_TRUE;
}

void CWWTPStopKeepAliveTask()
{
    if(gWTPKeepAliveThreadRun)
    {
        CWThreadMutexLock(&gWTPKeepAliveMutex);

        gWTPKeepAliveThreadRun = CW_FALSE;

        CWSignalThreadCondition(&gWTPKeepAliveCond);

        CWThreadMutexUnlock(&gWTPKeepAliveMutex);

        CWDebugLog("Waiting Keep Alive Thread exit...");

        CWThreadJoin(gWTPKeepAliveThread);
    }
}

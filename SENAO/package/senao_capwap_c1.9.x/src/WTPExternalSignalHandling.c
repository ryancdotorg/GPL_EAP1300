#include "CWWTP.h"
#include <dirent.h>

#ifdef CW_BUILD_X86
#define WTP_SIGNAL_INPUT_FILE_DIR            "wtpsignal/"
#define CW_WTP_SIGNAL_PID_FILE         "wtp.signal.pid"
#else
#define WTP_SIGNAL_INPUT_FILE_DIR            "/tmp/wtpsignal/"
#define CW_WTP_SIGNAL_PID_FILE         "/var/run/wtp.signal.pid"
#endif
#define WTP_SIGNAL_OUTPUT_FILE_DIR     "/tmp"

#define MAX_NAME_LEN 256
#define MAX_PARAM_NUM 32

typedef enum
{
    SIG_AUTO_CHANNEL,
    SIG_WTP_ALL_STATUS,
    SIG_WTP_MEM_INFO,
    SIG_WTP_DETECT_DFS,
    SIG_NONE
} CWWTPSignalType;

typedef struct
{
    char name[MAX_NAME_LEN];
    char *val;
} CWSignalParam;

typedef struct
{
    char inputFile[64];
    char outputFile[64];
    int paramNum;
    CWSignalParam param[MAX_PARAM_NUM];
} CWSignalFileInfo;

typedef struct
{
    const char *name;
    CWWTPSignalType type;
} CWWTPSignalInfo;

static CWSignalFileInfo gSignalFileInfo;

static CWWTPSignalInfo gSignalInfoTable[] =
{
#ifdef CW_WTP_AP
    {"autoch", SIG_AUTO_CHANNEL},
    {"wtp_detect_dfs", SIG_WTP_DETECT_DFS},
#endif
    {"wtp_all_status", SIG_WTP_ALL_STATUS},
    {"wtp_mem_info", SIG_WTP_MEM_INFO},
    {NULL, SIG_NONE}
};

static CWWTPSignalType CWWTPGetSignalType(const char *sigName)
{
    int id;

    for(id = 0; gSignalInfoTable[id].type != SIG_NONE; id++)
    {
        if(!strcmp(gSignalInfoTable[id].name, sigName))
        {
            return gSignalInfoTable[id].type;
        }
    }

    return SIG_NONE;
}

static char *CWWTPGetSignalParam(char *key)
{
    int id;

    for(id = 0; id < gSignalFileInfo.paramNum; id++)
    {
        if(!strcmp(gSignalFileInfo.param[id].name, key))
        {
            return gSignalFileInfo.param[id].val;
        }
    }

    return NULL;
}

static CWBool CWWTPReadSignalFile()
{
    FILE *fp;
    char *c;

    if((fp = fopen(gSignalFileInfo.inputFile, "r")) == NULL)
    {
        CWLog("failed to open signal file %s", gSignalFileInfo.inputFile);
        return CW_FALSE;
    }

    gSignalFileInfo.paramNum = 0;

    while(!feof(fp) && gSignalFileInfo.paramNum < MAX_PARAM_NUM)
    {
        if(fgets(gSignalFileInfo.param[gSignalFileInfo.paramNum].name,
                 MAX_NAME_LEN, fp) != NULL)
        {
            gSignalFileInfo.param[gSignalFileInfo.paramNum].val = NULL;
            c = gSignalFileInfo.param[gSignalFileInfo.paramNum].name;

            /* remove '\n' in the tail */
            while(*c != '\0')
            {
                if(*c == '\n')
                {
                    *c = '\0';
                    break;
                }
                else if(*c == '=' && gSignalFileInfo.param[gSignalFileInfo.paramNum].val == NULL)
                {
                    *c = '\0';
                    gSignalFileInfo.param[gSignalFileInfo.paramNum].val = c + 1;
                }

                c++;
            }

            /* skip empty line */
            if(c == gSignalFileInfo.param[gSignalFileInfo.paramNum].name)
            {
                continue;
            }

            gSignalFileInfo.paramNum++;
        }
    }

    fclose(fp);

    return gSignalFileInfo.paramNum ? CW_TRUE : CW_FALSE;
}

#define CWWTPWriteSignalOutputFile(_str) CWSaveStringToFile(gSignalFileInfo.outputFile, _str)

#ifdef CW_WTP_AP
static int CWWTP5GFeqToChannel(char *feq, int *channel)
{
    int ifeq = atoi(feq);

    if(ifeq < 5000)
    {
        return CW_FALSE;
    }

    if((ifeq - 5000) % 5 != 0)
    {
        return CW_FALSE;
    }

    *channel = (ifeq - 5000) / 5;

    return CW_TRUE;
}

static CWBool SendDFSLogEvent(char *oldCh, char *newCh)
{
    int oldChVal = 0, newChVal = 0;

    if(gWTPCurrentState != CW_ENTER_RUN)
    {
        CWDebugLog("Failed to sent event for DFS");
        CWWTPWriteSignalOutputFile("Success");
        return CW_TRUE;
    }

    if(!CWWTP5GFeqToChannel(oldCh, &oldChVal))
    {
        CWWTPWriteSignalOutputFile("Failed:original channel is invalid");
        return CW_FALSE;
    }

    if(!CWWTP5GFeqToChannel(newCh, &newChVal))
    {
        CWWTPWriteSignalOutputFile("Failed:new channel is invalid");
        return CW_FALSE;
    }

    CWWTPSendLogEvent(SN_EG_DEV_STATE, SN_LOG_CAT_WTP, SN_LOG_SEV_WARNING,
                      "detected radar signal on 5GHz, change channel from channel [%d] to channel [%d]",
                      oldChVal, newChVal);

    /* Send current config to update new channel on AC */
    CWWTPSendCurCfg(CW_TRUE);

    CWWTPWriteSignalOutputFile("Success");

    return CW_TRUE;
}

static CWBool SendAutoChannelChangeEvent(CWRadioFreqType radioType, int oldCh, int newCh)
{
    CWProtocolMessage *msgElems;
    CWWTPAutoChannelInfo info;

    CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems,
                                     1,
                                     return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    info.radioType = radioType;
    info.oldChannel = oldCh;
    info.newChannel = newCh;

    if(!CWWTPAssembleMsgElemAutoChannelChangeInfo(&(msgElems[0]), &info))
    {
        CW_FREE_OBJECT(msgElems);
        return CW_FALSE;
    }

    if(!CWWTPSendPendingRequest(CW_MSG_TYPE_VALUE_WTP_EVENT_REQUEST, CWGetSeqNum(), msgElems, 1))
    {
        CW_FREE_PROTOCOL_MESSAGE(msgElems[0]);
        CW_FREE_OBJECT(msgElems);
        return CW_FALSE;
    }

    return CW_TRUE;
}

static CWBool CWWTPSendAutoChannelInfo()
{
    CWRadioFreqType radioType;
    int oldCh, newCh;

    if(gWTPCurrentState != CW_ENTER_RUN)
    {
        CWLog("channel is changed but WTP is not in run state");
        return CW_TRUE;
    }

    if(gSignalFileInfo.paramNum != 4)
    {
        CWLog("Invaild Format");
        return CW_FALSE;
    }

    if(!strcmp(gSignalFileInfo.param[1].name, "5G"))
    {
        radioType = CW_RADIOFREQTYPE_5G;
    }
    else if(!strcmp(gSignalFileInfo.param[1].name, "2G"))
    {
        radioType = CW_RADIOFREQTYPE_2G;
    }
    else if(!strcmp(gSignalFileInfo.param[1].name, "5G_1"))/* wait for AP support*/
    {
        radioType = CW_RADIOFREQTYPE_5G;
    }
    else
    {
        CWLog("Failed to get type of radio");
        return CW_FALSE;
    }

    oldCh = atoi(gSignalFileInfo.param[2].name);
    newCh = atoi(gSignalFileInfo.param[3].name);

    return SendAutoChannelChangeEvent(radioType, oldCh, newCh);
}

#endif /* CW_WTP_AP */

static int CWWTPSetWTPStatusToFile()
{
    char buf[512];
    CWHostName forceACAddr;
    CWAcAddress acAddr;
    char *val;
    int json = 0;
    int len = 0;

    if(!CWWTPBoardGetAcAddress(&acAddr))
    {
        return CWWTPWriteSignalOutputFile("Error:Failed to get Ac address");
    }

    if(!CWWTPBoardGetForceAcAddress(forceACAddr))
    {
        return CWWTPWriteSignalOutputFile("Error:Failed to get Force Ac address");
    }

    val = CWWTPGetSignalParam("json");
    if(val)
    {
        json = atoi(val);
    }

    if(!json)
    {
        if(CWCreateAcAddrString(&acAddr, &val))
        {
            len += sprintf(&buf[len], "AC Address:%s\n", val);
            CW_FREE_OBJECT(val);
        }
        len += sprintf(&buf[len], "Force AC Address:%s\n", forceACAddr);
        len += sprintf(&buf[len], "WTP State:%d\n", gWTPCurrentState);
    }
    else
    {
        len += sprintf(&buf[len], "\"wtp_info\":{");
        len += sprintf(&buf[len], "\"wtp_state\":%d,", gWTPCurrentState);
        len += sprintf(&buf[len], "\"facip\":\"%s\",", forceACAddr);
        len += sprintf(&buf[len], "\"ac_addr\":\"%s\",", acAddr.hostName);
        len += sprintf(&buf[len], "\"ac_port\":%u,", acAddr.port);
        len += sprintf(&buf[len], "\"ac_cid\":%u", acAddr.controllerId);
        len += sprintf(&buf[len], "}");
    }

    return CWWTPWriteSignalOutputFile(buf);
}

static CWBool CWWTPExternalSignalHandler()
{
    DIR *d;
    struct dirent *dir;

    d = opendir(WTP_SIGNAL_INPUT_FILE_DIR);
    if(d == NULL)
    {
        CWLog("open directory "WTP_SIGNAL_INPUT_FILE_DIR" failed");
        return CW_FALSE;
    }

    while((dir = readdir(d)) != NULL)
    {
        if(dir->d_name[0] == '.')
        {
            continue;
        }

        /* get input/output file path */
        sprintf(gSignalFileInfo.inputFile, WTP_SIGNAL_INPUT_FILE_DIR"/%s", dir->d_name);
        sprintf(gSignalFileInfo.outputFile, WTP_SIGNAL_OUTPUT_FILE_DIR"/%s", dir->d_name);

        /* read file content */
        if(!CWWTPReadSignalFile())
        {
            unlink(gSignalFileInfo.inputFile);
            continue;
        }
        unlink(gSignalFileInfo.inputFile);

        switch(CWWTPGetSignalType(gSignalFileInfo.param[0].name))
        {
#ifdef CW_WTP_AP
            case SIG_AUTO_CHANNEL:
                CWWTPSendAutoChannelInfo();
                break;
            case SIG_WTP_DETECT_DFS:
                SendDFSLogEvent(gSignalFileInfo.param[1].name,
                                gSignalFileInfo.param[2].name);
                break;
#endif
            case SIG_WTP_MEM_INFO:
#ifdef CW_DEBUGGING_MEMORY
                CWMemTracePrint(gSignalFileInfo.outputFile);
#else
                CWWTPWriteSignalOutputFile("Please build with CW_DEBUGGING_MEMORY flag");
#endif
                break;
            case SIG_WTP_ALL_STATUS:
                CWWTPSetWTPStatusToFile();
                break;
            default:
                CWLog("unknow signal type %s", gSignalFileInfo.param[0].name);
        }
    }

    closedir(d);

    return CW_TRUE;
}

static CW_THREAD_RETURN_TYPE CWWTPSignalHandlingTask(void *arg)
{
    sigset_t mask;
    int sig;
    char pid[16];

    CWDebugLog("CWWTPSignalHandlingTask enter");

    CWSystem("rm -rf "WTP_SIGNAL_INPUT_FILE_DIR " 2> /dev/null");

    /* write tid file */
#ifndef SYS_gettid /*pid is tid in Single-threaded process */
    sprintf(pid, "%d", getpid());
#else /* Multi-threaded Processes */
    sprintf(pid, "%d", (int)syscall(SYS_gettid));
#endif
    CWSaveStringToFile(CW_WTP_SIGNAL_PID_FILE, pid);

    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR2);

    CW_REPEAT_FOREVER
    {
        if(sigwait(&mask, &sig) == 0 && sig == SIGUSR2)
        {
            CWDebugLog("Invoke External Signal Handler");
            CWWTPExternalSignalHandler();
        }
    }

    return (void *) 0;
}

CWBool CWWTPExternalSignalHandlerStart()
{
    CWThread thread;

    if(!CWErr(CWCreateDetachedThread(&thread, CWWTPSignalHandlingTask, NULL)))
    {
        return CW_FALSE;
    }

    return CW_TRUE;
}

#if 0
void CWWTPExternalSignalHandlerStop()
{
    CWThreadKill(gExternalSignalHandlingThread, SIGUSR1);

    /* wait for the end of thread */
    CWThreadJoin(gExternalSignalHandlingThread);

    return;
}
#endif


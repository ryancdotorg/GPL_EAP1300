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
#include <openssl/md5.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/sysinfo.h>

#ifdef DMALLOC
#include "../dmalloc-5.5.0/dmalloc.h"
#endif

int gCWForceMTU = 0;
int gCWRetransmitTimer = CW_RETRANSMIT_INTERVAL_DEFAULT;	//Default value for RetransmitInterval
int gCWNeighborDeadInterval = CW_NEIGHBORDEAD_INTERVAL_DEFAULT; //Default value for NeighbourDeadInterval (no less than 2*EchoInterval and no greater than 240)
int gCWMaxRetransmit = CW_MAX_RETRANSMIT_DEFAULT;		//Default value for MaxRetransmit
#ifdef CW_WTP_AP
CWBool gWTPUciLogEnable = CW_FALSE;
#endif

//extern void* b64_malloc(size_t);
extern void* b64_realloc(void*, size_t);

int CWTimevalSubtract(struct timeval *res, const struct timeval *x, const struct timeval *y)
{
    int nsec;
    struct timeval z = *y;

    // Perform the carry for the later subtraction by updating Y
    if(x->tv_usec < z.tv_usec)
    {
        nsec = (z.tv_usec - x->tv_usec) / 1000000 + 1;
        z.tv_usec -= 1000000 * nsec;
        z.tv_sec += nsec;
    }
    if(x->tv_usec - z.tv_usec > 1000000)
    {
        nsec = (x->tv_usec - z.tv_usec) / 1000000;
        z.tv_usec += 1000000 * nsec;
        z.tv_sec -= nsec;
    }

    // Compute the time remaining to wait. `tv_usec' is certainly positive
    if(res != NULL)
    {
        res->tv_sec = x->tv_sec - z.tv_sec;
        res->tv_usec = x->tv_usec - z.tv_usec;
    }

    // Return 1 if result is negative (x < y)
    return ((x->tv_sec < z.tv_sec) || ((x->tv_sec == z.tv_sec) && (x->tv_usec < z.tv_usec)));
}

///////////////////////////////////////////////////////////////////////////
/*
 * CWForkShellProcess is implemented for addressing the issue
 * that we cannot call system or exec in multi-thread program.
 * The reason is discribed at: http://maxim.int.ru/bookshelf/PthreadsProgram/htm/r_44.html
 * My solution is to fork a child process at startup, when parent want to execute a shell command,
 * the command will be sent to child via pipe and executed in child process.
 */
#define SHELL_CMD_BUF_SIZE (1024*4)
#define PIPE_R_FD 0
#define PIPE_W_FD 1

static int gShellCmdPipefd[2];
static int gShellAckPipefd[2];
static char gShellCmd[SHELL_CMD_BUF_SIZE];
static CWThreadMutex gShellMutex = CW_MUTEX_INITIALIZER;

CWBool CWForkShellProcess()
{
    pid_t cpid;

    if(pipe(gShellCmdPipefd) == -1)
    {
        fprintf(stderr, "Create pipe for Shell Process failed");
        return CW_FALSE;
    }

    if(pipe(gShellAckPipefd) == -1)
    {
        fprintf(stderr, "Create pipe for Shell Process failed");
        return CW_FALSE;
    }

    cpid = fork();
    if(cpid == -1)
    {
        fprintf(stderr, "fork Shell Process failed");
        return CW_FALSE;
    }

    if(cpid == 0) /* Child reads command from pipe */
    {
        int n = 0;
        int ret, r, rest;

        close(gShellCmdPipefd[PIPE_W_FD]);

        while(1)
        {
            r = read(gShellCmdPipefd[PIPE_R_FD], &gShellCmd[n], 1);
            if(r < 0)
            {
                if(errno == EAGAIN || errno == EINTR)
                {
                    continue;
                }
                fprintf(stderr, "Read shell command pipe failed: %s\n",
                        strerror(errno));
                break;
            }
            else if(r == 0)
            {
                /* pipe is closed, program should exit */
                break;
            }

            /* execute cmd when a zero-end detected */
            if(gShellCmd[n] == '\0')
            {
                ret = system(gShellCmd);

                /* send the return code back to parent */
                rest = 4;
                do
                {
                    r = write(gShellAckPipefd[PIPE_W_FD], ((char *)&ret) + 4 - rest, rest);
                    if(r < 0)
                    {
                        if(errno == EAGAIN || errno == EINTR)
                        {
                            continue;
                        }
                        fprintf(stderr, "Write shell command pipe failed: %s\n",
                                strerror(errno));
                        break;
                    }
                    rest -= r;
                }
                while(rest);

                n = 0;
            }
            else
            {
                n++;
            }
        }

        close(gShellCmdPipefd[PIPE_R_FD]);
        close(gShellAckPipefd[PIPE_W_FD]);

        printf("Shell Command Process Terminated\n");

        exit(EXIT_SUCCESS);
    }

    /* Parent can go ahead */
    close(gShellCmdPipefd[PIPE_R_FD]);
    close(gShellAckPipefd[PIPE_W_FD]);

    return CW_TRUE;
}

CWBool CWCloseShellProcess()
{
    close(gShellCmdPipefd[PIPE_W_FD]);
    close(gShellAckPipefd[PIPE_R_FD]);
    return CW_TRUE;
}

int _CWSystem(CWBool dbg, const char *fmt, ...)
{
    va_list va;
    int ret, len, rest, exitCode;

    CWThreadMutexLock(&gShellMutex);

    va_start(va, fmt);
    len = vsnprintf(gShellCmd, sizeof(gShellCmd), fmt, va);
    va_end(va);

    len++; /* add a byte for zero-end */

#ifdef CW_WTP_AP
    if(gWTPUciLogEnable)
    {
        if(strstr(gShellCmd, "uci set")
           || strstr(gShellCmd, "uci delete")
           || strstr(gShellCmd, "uci add")
           || strstr(gShellCmd, "uci commit")
           || strstr(gShellCmd, "luci-reload")
           || strstr(gShellCmd, "uci changes")
           || strstr(gShellCmd, "echo"))
        {
            CWSystemCallLog("[UCI Log] %s", gShellCmd);
        }
    }
#endif

    /* send shell command to child process */
    rest = len;
    do
    {
        ret = write(gShellCmdPipefd[PIPE_W_FD], &gShellCmd[len - rest], rest);
        if(ret < 0)
        {
            CWLog("Write pipe error: %s", strerror(errno));
            if(errno == EAGAIN || errno == EINTR)
            {
                CWLog("Write to pipe again");
                continue;
            }
            CWThreadMutexUnlock(&gShellMutex);
            return 1;
        }
        rest -= ret;
    }
    while(rest);

    /* wait the exit code of the command from child process */
    rest = 4;
    do
    {
        ret = read(gShellAckPipefd[PIPE_R_FD], ((char *)&exitCode) + 4 - rest, rest);
        if(ret < 0)
        {
            CWLog("Read pipe error: %s", strerror(errno));
            if(errno == EAGAIN || errno == EINTR)
            {
                CWLog("Read pipe again");
                continue;
            }
            CWThreadMutexUnlock(&gShellMutex);
            return 1;
        }
        rest -= ret;
    }
    while(rest);

    if(dbg)
    {
        CWLog("cmd [%s] exitCode=%d", gShellCmd, exitCode);
    }

    CWThreadMutexUnlock(&gShellMutex);

    return exitCode;
}
///////////////////////////////////////////////////////////////////////////

/* This function should not be used in multi-threads environment */
int CWSystemUnsafe(const char *fmt, ...)
{
    char *cmd;
    int cmdLen;
    int ret;
    va_list va;

    va_start(va, fmt);

    cmdLen = vsnprintf(NULL, 0, fmt, va); /* get how many buffer we need */

    va_end(va);

    cmdLen++; /* + 1 for end of string */

    CW_CREATE_OBJECT_SIZE_ERR(cmd, cmdLen, return 1;);

    va_start(va, fmt);

    vsnprintf(cmd, cmdLen, fmt, va);

    va_end(va);

#if 1 /* vfork */
    int status;
    pid_t pid;

    pid = vfork();
    if(pid < 0)
    {
        CWLog("vfork failed");
        return -1;
    }
    if(pid == 0)
        /* child process */
    {
        execl("/bin/sh", "sh", "-c", cmd, (char *)0);
        _exit(127);
    }
    else
    {
        do
        {
            waitpid(pid, &status, 0);
        }
        while(!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    ret = WEXITSTATUS(status);
#else
    ret = system(cmd);
#endif

    CW_FREE_OBJECT(cmd);

    return ret;
}

CWBool CWWtpCfgMsgListAdd(CWWtpCfgMsgList *cfgList, int type,
                          int keyLen, void *keyPtr, int valLen, void *valPtr)
{
    CWWtpCfgMsgNode *node;
    CWWtpCfgMsgNode *iter_node, *pre_node;

    CW_CREATE_OBJECT_ERR(node, CWWtpCfgMsgNode,
                         return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    node->type = type;
    node->keyLen = keyLen;
    node->keyPtr = keyPtr;
    node->valLen = valLen;
    node->valPtr = valPtr;
    node->next = NULL;

    /* insert to list in order of msg type */
    if(cfgList->head)
    {
        pre_node = NULL;
        iter_node = cfgList->head;
        do
        {
            if(type < iter_node->type)
            {
                break;
            }
            /* replace duplicated cfg in list */
            else if(type == iter_node->type &&
                    keyLen == iter_node->keyLen &&
                    !memcmp(keyPtr, iter_node->keyPtr, keyLen)
                   )
            {
                CW_FREE_OBJECT(node);
                CW_FREE_OBJECT(iter_node->valPtr);
                CW_FREE_OBJECT(iter_node->keyPtr);

                cfgList->msgSize -= iter_node->valLen;
                cfgList->msgSize += valLen;
                iter_node->valLen = valLen;
                iter_node->keyPtr = keyPtr;
                iter_node->valPtr = valPtr;

                return CW_TRUE;
            }
            pre_node = iter_node;
            iter_node = iter_node->next;
        }
        while(iter_node);

        if(!iter_node)
        {
            cfgList->tail->next = node;
            cfgList->tail = node;
        }
        else if(!pre_node)
        {
            node->next = cfgList->head;
            cfgList->head = node;
        }
        else
        {
            node->next = pre_node->next;
            pre_node->next = node;
        }
    }
    else
    {
        cfgList->head = node;
        cfgList->tail = node;
    }

    cfgList->msgSize += 2 + 2 + keyLen + 2 + valLen;

    return CW_TRUE;
}

void CWWtpCfgMsgListFree(CWWtpCfgMsgList *cfgList)
{
    CWWtpCfgMsgNode *node, *tmpNode;

    if(!cfgList)
    {
        return;
    }

    node = cfgList->head;
    while(node)
    {
        tmpNode = node;
        node = node->next;
        CW_FREE_OBJECT(tmpNode->keyPtr);
        CW_FREE_OBJECT(tmpNode->valPtr);
        CW_FREE_OBJECT(tmpNode);
    }

    cfgList->head = NULL;
    cfgList->tail = NULL;
    cfgList->msgSize = 0;
}

CWBool CWWtpCfgMsgListRemove(CWWtpCfgMsgList *cfgList, int type,
                             int keyLen, void *keyPtr)
{
    CWWtpCfgMsgNode *iter_node, *pre_node = NULL;

    if(!cfgList)
    {
        return CW_FALSE;
    }

    iter_node = cfgList->head;
    while(iter_node)
    {
        if(type < iter_node->type)
        {
            return CW_FALSE;
        }
        else if(type == iter_node->type &&
                keyLen == iter_node->keyLen &&
                !memcmp(keyPtr, iter_node->keyPtr, keyLen)
               )
        {
            if(pre_node)
            {
                pre_node->next = iter_node->next;
            }
            else
            {
                cfgList->head = iter_node->next;
            }

            if(iter_node->next == NULL)
            {
                cfgList->tail = pre_node;
            }

            cfgList->msgSize -= 2 + 2 + keyLen + 2 + iter_node->valLen;
            CW_ASSERT(cfgList->msgSize >= 0);

            CW_FREE_OBJECT(iter_node->valPtr);
            CW_FREE_OBJECT(iter_node->keyPtr);
            CW_FREE_OBJECT(iter_node);

            return CW_TRUE;
        }

        pre_node = iter_node;
        iter_node = iter_node->next;
    }

    return CW_FALSE;
}

void CWWtpCfgFree(CWWtpCfg *cfg)
{
    int i, j, k;

    if(!cfg)
    {
        return;
    }

    if(cfg->radios)
    {
        for(i = 0; i < cfg->radioCount; i++)
        {
            CW_FREE_OBJECT(cfg->radios[i].portal.redirBehv);
            CW_FREE_OBJECT(cfg->radios[i].portal.exterServer);
            CW_FREE_OBJECT(cfg->radios[i].portal.extersecret);
            CW_FREE_OBJECT(cfg->radios[i].portal.uamFormat);
            CW_FREE_OBJECT(cfg->radios[i].portal.localAuth);
            CW_FREE_OBJECT(cfg->radios[i].portal.garden.address);
            CW_FREE_OBJECT(cfg->radios[i].portal.radius.secret);
            CW_FREE_OBJECT(cfg->radios[i].portal.accounting.secret);
            CW_FREE_OBJECT(cfg->radios[i].portal.radius_secret2);
            CW_FREE_OBJECT(cfg->radios[i].portal.termofuse);
            CW_FREE_OBJECT(cfg->radios[i].portal.logoPath);
            CW_FREE_OBJECT(cfg->radios[i].portal.capmsg);

            CW_FREE_OBJECT(cfg->radios[i].mesh.id);
            CW_FREE_OBJECT(cfg->radios[i].mesh.wpaKey);
            if(cfg->radios[i].wlans)
            {
                for(j = 0; j < cfg->radios[i].wlanCount; j++)
                {
                    CW_FREE_OBJECT(cfg->radios[i].wlans[j].ssid);
                    CW_FREE_OBJECT(cfg->radios[i].wlans[j].acl.macs);
                    CW_FREE_OBJECT(cfg->radios[i].wlans[j].wpa.passphrase);
                    CW_FREE_OBJECT(cfg->radios[i].wlans[j].wpa.radius.secret);
                    CW_FREE_OBJECT(cfg->radios[i].wlans[j].accounting.secret);
                    CW_FREE_OBJECT(cfg->radios[i].wlans[j].nas.id);
                    CW_FREE_OBJECT(cfg->radios[i].wlans[j].portal.redirBehv);
                    CW_FREE_OBJECT(cfg->radios[i].wlans[j].portal.exterServer);
                    CW_FREE_OBJECT(cfg->radios[i].wlans[j].portal.extersecret);
                    CW_FREE_OBJECT(cfg->radios[i].wlans[j].portal.uamFormat);
                    CW_FREE_OBJECT(cfg->radios[i].wlans[j].portal.localAuth);
                    CW_FREE_OBJECT(cfg->radios[i].wlans[j].portal.garden.address);
                    CW_FREE_OBJECT(cfg->radios[i].wlans[j].portal.radius.secret);
                    CW_FREE_OBJECT(cfg->radios[i].wlans[j].portal.accounting.secret);
                    CW_FREE_OBJECT(cfg->radios[i].wlans[j].portal.radius_secret2);
                    CW_FREE_OBJECT(cfg->radios[i].wlans[j].portal.termofuse);
                    CW_FREE_OBJECT(cfg->radios[i].wlans[j].portal.logoPath);
                    CW_FREE_OBJECT(cfg->radios[i].wlans[j].portal.capmsg);
                    CW_FREE_OBJECT(cfg->radios[i].wlans[j].l2IsoWlist.macs);
                    cfg->radios[i].wlans[j].l2IsoWlist.macCount=0;
                    for(k = 0; k < CW_WTP_CFG_MAX_WEPKEY; k++)
                    {
                        CW_FREE_OBJECT(cfg->radios[i].wlans[j].wep.keys[k]);
                    }
                }
                CW_FREE_OBJECT(cfg->radios[i].wlans);
            }
        }
    }

    if(cfg->portCfg)
    {
        for(i = 0; i < cfg->portNum; i++)
        {
            CW_FREE_OBJECT(cfg->portCfg[i].descp);
        }
        CW_FREE_OBJECT(cfg->portCfg);
    }

    CW_FREE_OBJECT(cfg->lanPortCfg);    
    CW_FREE_OBJECT(cfg->poePortCfg);
    CW_FREE_OBJECT(cfg->name);
    CW_FREE_OBJECT(cfg->location);
    CW_FREE_OBJECT(cfg->radios);
    CW_FREE_OBJECT(cfg->admin);
    CW_FREE_OBJECT(cfg->remoteLogSrvConf);
    CW_FREE_OBJECT(cfg->password);
}

void CWWtpStationInfoFree(CWWTPStationInfo *staInfo)
{
    int i, j;

    if(!staInfo)
    {
        return;
    }

    if(staInfo->info)
    {
        for(i = 0; i < staInfo->infoCount; i++)
        {
            for(j = 0; j < staInfo->info[i].stationCount; j++)
            {
                CW_FREE_OBJECT(staInfo->info[i].station[j].osType);
                CW_FREE_OBJECT(staInfo->info[i].station[j].hostName);
            }
            CW_FREE_OBJECT(staInfo->info[i].station);
        }
        CW_FREE_OBJECT(staInfo->info);
    }
}

void CWWtpCurrentCfgInfoFree(CWWTPCurrentCfgInfo *cfgInfo)
{
    int i;

    if(!cfgInfo)
    {
        return;
    }

    if(cfgInfo->radio)
    {
        for(i = 0; i < cfgInfo->radioCount; i++)
        {
            CW_FREE_OBJECT(cfgInfo->radio[i].wlan);
            CW_FREE_OBJECT(cfgInfo->radio[i].channelList);
        }
        CW_FREE_OBJECT(cfgInfo->radio);
    }
}

void CWWtpSwitchTopologyInfoFree(CWWTPSwitchTopologyInfo *topoInfo)
{
    int i, j;

    if(!topoInfo)
    {
        return;
    }

    if(topoInfo->devInfo)
    {
        for(i = 0; i < topoInfo->devCount; i++)
        {
            CW_FREE_OBJECT(topoInfo->devInfo[i].name);
            CW_FREE_OBJECT(topoInfo->devInfo[i].desc);
            if(topoInfo->devInfo[i].linkList)
            {
                for(j = 0; j < topoInfo->devInfo[i].linkCount; j++)
                {
                    CW_FREE_OBJECT(topoInfo->devInfo[i].linkList[j].remotePort);
                }

                CW_FREE_OBJECT(topoInfo->devInfo[i].linkList);
            }
        }

        CW_FREE_OBJECT(topoInfo->devInfo);
        topoInfo->devCount = 0;
    }
}

CWBool CWParseAcAddrString(const char *str, CWAcAddress *addr)
{
    int i;
    CWBool getPort = CW_FALSE, getTicket = CW_FALSE;

    CW_ZERO_MEMORY(addr->hostName, sizeof(addr->hostName));
    addr->port = CW_CONTROL_PORT;
    addr->controllerId = 0;

    for(i = 0; str[i] != '\0'; i++)
    {
        if(str[i] == ':')
        {
            if(getPort)
            {
                return CWErrorRaise(CW_ERROR_INVALID_FORMAT, NULL);
            }
            addr->port = strtol(&str[i + 1], NULL, 10);
            getPort = CW_TRUE;
        }
        else if(str[i] == '#')
        {
            if(getTicket)
            {
                return CWErrorRaise(CW_ERROR_INVALID_FORMAT, NULL);
            }
            addr->controllerId = strtol(&str[i + 1], NULL, 10);
            getTicket = CW_TRUE;
        }
        else if(i < sizeof(addr->hostName) - 1 && !getPort && !getTicket)
        {
            addr->hostName[i] = str[i];
        }
    }

    return CW_TRUE;
}

void CWSetAcAddr(unsigned int ip, unsigned short port, int controllerId, CWAcAddress *addr)
{
    if(ip)
    {
        sprintf(addr->hostName, "%u.%u.%u.%u", CW_IPV4_PRINT_LIST(ip));
    }
    else
    {
        addr->hostName[0] = '\0';
    }
    addr->port = port;
    addr->controllerId = controllerId;
}

CWBool CWCreateAcAddrString(const CWAcAddress *addr, char **pstr)
{
    if(addr->hostName[0] == '\0')
    {
        CW_CREATE_STRING_FROM_STRING_ERR(*pstr, "", return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
        return CW_TRUE;
    }
    else
    {
        *pstr = CWCreateString("%s:%u#%d", addr->hostName, addr->port, addr->controllerId);
    }

    return *pstr ? CW_TRUE : CW_FALSE;
}


void CWWtpSwitchPortInfoFree(CWWTPSwitchPortInfo*portInfo)
{
    if(!portInfo)
    {
        return;
    }

    if(portInfo->info)
    {
        CW_FREE_OBJECT(portInfo->info);
    }
    portInfo->infoCount = 0;
}


void CWWtpSwitchTrunkInfoFree(CWWTPSwitchTrunkInfo *trunkInfo)
{
    int i;

    if(!trunkInfo)
    {
        return;
    }

    if(trunkInfo->info)
    {
        for(i = 0; i < trunkInfo->infoCount; i++)
        {
            CW_FREE_OBJECT(trunkInfo->info[i].activePort);
            CW_FREE_OBJECT(trunkInfo->info[i].memberPort);
        }

        CW_FREE_OBJECT(trunkInfo->info);
    }
}


CWBool CWParseMacString(const char *str, CWMacAddress mac)
{
    unsigned int mac_tmp[6];
    int i;
    int zero = 1;

    if(sscanf(str, "%x:%x:%x:%x:%x:%x",
              &mac_tmp[0], &mac_tmp[1], &mac_tmp[2], &mac_tmp[3], &mac_tmp[4], &mac_tmp[5]) != 6)
    {
        return 0;
    }
    for(i = 0; i < 6; i++)
    {
        if(mac_tmp[i] > 255)
        {
            return 0;
        }
        if(mac_tmp[i] != 0)
        {
            zero = 0;
        }
        mac[i] = (unsigned char) mac_tmp[i];
    }

    if(zero)
    {
        return 0;
    }

    return 1;
}

int CWParseUnimac(const char *str, CWMacAddress mac)
{
    if(!CWParseMacString(str, mac))
    {
        return CW_FALSE;
    }
    return mac[0] & 0x1 ? 0 : 1;
}

CWBool CWParseAcList(const char *str, CWHostName **acList, int *acNum)
{
    int i, j = 0;
    char *pch, *ptr;
    char *tmpStr;

    if(!str || str[0] == '\0')
    {
        *acNum = 0;
        *acList = NULL;
        return CW_TRUE;
    }

    for(i = 0; str[i] != '\0'; i++)
    {
        if(str[i] == ',')
        {
            j++;
        }
    }

    CW_CREATE_ZERO_ARRAY_ERR(*acList, j + 1, CWHostName,
                             return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    CW_CREATE_STRING_FROM_STRING_LEN_ERR(tmpStr, str, i,
                                         CW_FREE_OBJECT(*acList);
                                         return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    i = 0;
    pch = strtok_r(tmpStr, ",", &ptr);
    while(pch != NULL)
    {
        strcpy((*acList)[i], pch);
        pch = strtok_r(NULL, ",", &ptr);
        i++;
    }
    CW_FREE_OBJECT(tmpStr);

    *acNum = i;

    return CW_TRUE;
}

CWBool CWParseMacList(const char *str, CWMacAddress **macList, int *macNum)
{
    int i, j = 0;
    char *pch, *ptr;
    char *tmpStr;

    if(!str || str[0] == '\0')
    {
        *macNum = 0;
        *macList = NULL;
        return CW_TRUE;
    }

    for(i = 0; str[i] != '\0'; i++)
    {
        if(str[i] == ',')
        {
            j++;
        }
    }

    CW_CREATE_ARRAY_ERR(*macList, j + 1, CWMacAddress,
                        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    CW_CREATE_STRING_FROM_STRING_LEN_ERR(tmpStr, str, i,
    {
        CW_FREE_OBJECT(*macList);
        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
    });

    i = 0;
    pch = strtok_r(tmpStr, ",", &ptr);
    while(pch != NULL)
    {
        if(!CWParseUnimac(pch, (*macList)[i]))
        {
            CW_FREE_OBJECT(tmpStr);
            CW_FREE_OBJECT(*macList);
            return CWErrorRaise(CW_ERROR_INVALID_FORMAT, NULL);
        }

        /* avoid the same mac */
        for(j = 0; j < i; j++)
        {
            if(!memcmp((*macList)[i], (*macList)[j], 6))
            {
                break;
            }
        }
        if(j == i)
        {
            i++;
        }
        pch = strtok_r(NULL, ",", &ptr);
    }
    CW_FREE_OBJECT(tmpStr);

    *macNum = i;

    return CW_TRUE;
}

char *CWMacToString(CWMacAddress mac, char *str)
{
    sprintf(str, "%02X:%02X:%02X:%02X:%02X:%02X",
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return str;
}

CWBool CWSaveStringToFile(const char *file, const char *pstr)
{
    FILE *fp;
    int len;

    fp = fopen(file, "w");
    if(!fp)
    {
        return CWErrorRaise(CW_ERROR_GENERAL, "%s can not be opened", file);
    }

    len = strlen(pstr);

    if(fwrite(pstr, 1, len, fp) != len)
    {
        fclose(fp);
        return CWErrorRaise(CW_ERROR_GENERAL, "%s can not be written", file);
    }

    fclose(fp);

    return CW_TRUE;
}

CWBool CWCreateStringByFile(const char *file, char **pstr)
{
    FILE *fp;
    int fsize;

    fp = fopen(file, "r");
    if(!fp)
    {
        return CWErrorRaise(CW_ERROR_GENERAL, "%s can not be opened", file);
    }

    fseek(fp, 0, SEEK_END);
    fsize = ftell(fp);
    if(fsize < 0)
    {
        fclose(fp);
        return CWErrorRaise(CW_ERROR_GENERAL, NULL);
    }

    CW_CREATE_OBJECT_SIZE_ERR(*pstr, fsize + 1,
    {
        fclose(fp);
        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
    });

    fseek(fp, 0, SEEK_SET);
    if(fread(*pstr, 1, fsize, fp) != fsize)
    {
        fclose(fp);
        CW_FREE_OBJECT(*pstr);
        return CWErrorRaise(CW_ERROR_GENERAL, NULL);
    }
    fclose(fp);

    (*pstr)[fsize] = '\0';

    return CW_TRUE;
}

char *CWCreateString(const char *fmt, ...)
{
    char *pstr;
    int str_len;

    va_list va;

    va_start(va, fmt);

    str_len = vsnprintf(NULL, 0, fmt, va); /* get how many buffer we need */

    va_end(va);

    str_len++; /* + 1 for end of string */

    CW_CREATE_OBJECT_SIZE_ERR(pstr, str_len,
    {
        CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
        return NULL;
    });

    va_start(va, fmt);

    vsnprintf(pstr, str_len, fmt, va);

    va_end(va);

    return pstr;
}

char *_CWCreateStringByCmd(int type, const char *fmt, ...)
{
    char *cmd;
    char tmp_file[32];
    char *pstr;
    int cmd_len;

    va_list va;

    va_start(va, fmt);

    cmd_len = vsnprintf(NULL, 0, fmt, va); /* get how many buffer we need */

    va_end(va);

    cmd_len++; /* + 1 for end of string */

    CW_CREATE_OBJECT_SIZE_ERR(cmd, cmd_len,
    {
        CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
        return NULL;
    });

    va_start(va, fmt);

    cmd_len = vsnprintf(cmd, cmd_len, fmt, va); /* get how many buffer we need */

    va_end(va);

    sprintf(tmp_file, "/tmp/cwcmd.%u.%x.tmp", getpid(), (unsigned int) CWThreadSelf());

    if(type == 1) /* stdout */
    {
        CWSystem("%s > %s", cmd, tmp_file);
    }
    else if(type == 2) /* stderr */
    {
        CWSystem("%s 2> %s", cmd, tmp_file);
    }
    else /* stdout and stderr */
    {
        CWSystem("%s > %s 2>&1", cmd, tmp_file);
    }
    CW_FREE_OBJECT(cmd);

    if(!CWCreateStringByFile(tmp_file, &pstr))
    {
        unlink(tmp_file);
        return NULL;
    }

    unlink(tmp_file);
    return pstr;
}

CWBool CWGetFileSize(char *file, int *size)
{
    FILE *fp;

    if((fp = fopen(file, "rb")) == NULL)
    {
        return CWErrorRaise(CW_ERROR_GENERAL, "Open File Failed");
    }

    fseek(fp, 0, SEEK_END);
    *size = ftell(fp);

    fclose(fp);

    return CW_TRUE;
}

CWBool CWGetFileMD5(char *file, unsigned char chksum[])
{
    MD5_CTX c;
    char buf[512];
    size_t bytes;
    FILE *fp;

    if((fp = fopen(file, "rb")) == NULL)
    {
        return CWErrorRaise(CW_ERROR_GENERAL, "Open File Failed");
    }

    CW_ZERO_MEMORY(chksum, MD5_DIGEST_LENGTH);

    MD5_Init(&c);

    while(!feof(fp))
    {
        bytes = fread(buf, 1, 512, fp);
        if(bytes > 0)
        {
            MD5_Update(&c, buf, bytes);
        }
    }

    MD5_Final(chksum, &c);

    fclose(fp);

    return CW_TRUE;
}

CWBool CWRespawn()
{
    pid_t pid;
    int status;

restart_self:

    pid = fork();
    if(pid < 0)
    {
        fprintf(stderr, "error in fork!");
        return CW_FALSE;
    }
    else if(pid != 0)
    {
        do
        {
            waitpid(pid, &status, 0);
        }
        while(!WIFEXITED(status) && !WIFSIGNALED(status));

        if(WIFSIGNALED(status))
        {
            switch(WTERMSIG(status))
            {
                /* Exit with Termination Signals */
                case SIGKILL:
                case SIGTERM:
                case SIGQUIT:
                case SIGINT:
                case SIGHUP:
                    exit(0);
                    break;
                default:
                    /* Restart when segmantation fault */
                    goto restart_self;
            }
        }
        else if(WEXITSTATUS(status) == CW_EXIT_RESTART)
        {
            goto restart_self;
        }
        else
        {
            exit(WEXITSTATUS(status));
        }
    }

    return CW_TRUE;
}

/*20131004 sigma*/
CWBool CWCheckFileEqual(char *file1, char *file2)
{
    FILE *fp1, *fp2;

    /* open first file */
    if((fp1 = fopen(file1, "rb")) == NULL)
    {
        return CW_FALSE;
    }

    /* open second file */
    if((fp2 = fopen(file2, "rb")) == NULL)
    {
        fclose(fp1);
        return CW_FALSE;
    }

    /* compare file size */
    fseek(fp1, 0, SEEK_END);
    fseek(fp2, 0, SEEK_END);
    if(ftell(fp1) != ftell(fp2))
    {
        fclose(fp1);
        fclose(fp2);
        return CW_FALSE;
    }

    /* compare the files */
    fseek(fp1, 0, SEEK_SET);
    fseek(fp2, 0, SEEK_SET);
    while(!feof(fp1))
    {
        if(fgetc(fp1) != fgetc(fp2))
        {
            fclose(fp1);
            fclose(fp2);
            return CW_FALSE;
        }
    }

    fclose(fp1);
    fclose(fp2);
    return CW_TRUE;
}

int CWGetUptime(void)
{
    struct sysinfo info;

    return sysinfo(&info) ? 0 : info.uptime;
}

CWBool CWGetMemoryInfo(CWMemoryInfo *memInfo)
{
    char buffer[256], *c;
    FILE *fp;

    fp = fopen("/proc/meminfo", "r");
    if(!fp)
    {
        return CWErrorRaise(CW_ERROR_GENERAL,
                            "/proc/meminfo can not be opened");
    }

    while(fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        if(strstr(buffer,"MemTotal:"))
        {
            /* MemTotal */
            c = strchr(buffer, ':');
            if(!c)
            {
                goto error_exit;
            }

            do
            {
                c++;
            }
            while(!isdigit(*c));

            memInfo->total = strtoul(c, NULL, 10);
        }
        else if(strstr(buffer,"MemFree:"))
        {
            /* MemFree */
            c = strchr(buffer, ':');
            if(!c)
            {
                goto error_exit;
            }

            do
            {
                c++;
            }
            while(!isdigit(*c));

            memInfo->used = memInfo->total - strtoul(c, NULL, 10);
        }
        else if(strstr(buffer,"Buffers:"))
        {
            /* Buffers */
            c = strchr(buffer, ':');
            if(!c)
            {
                goto error_exit;
            }

            do
            {
                c++;
            }
            while(!isdigit(*c));

            memInfo->used -= strtoul(c, NULL, 10);
        }
        else if(strstr(buffer,"Cached:"))
        {
            /* Cached */
            c = strchr(buffer, ':');
            if(!c)
            {
                goto error_exit;
            }

            do
            {
                c++;
            }
            while(!isdigit(*c));

            memInfo->used -= strtoul(c, NULL, 10);
        }
    }
    
    CWDebugLog(" MemTotal total u=%u",memInfo->total);
    CWDebugLog(" Memuser used u=%u",memInfo->used);

    fclose(fp);
    return CW_TRUE;

error_exit:
    fclose(fp);
    return CWErrorRaise(CW_ERROR_GENERAL, "/proc/meminfo read error");
}

CWBool CWGetTopMemoryProcess(char **processName, unsigned int *memUsed)
{
    char *buffer, *c, *process = NULL;
    unsigned int mem, maxMem = 0;

#if defined(CW_BUILD_X86)
    buffer = CWCreateStringByCmdStdout("ps aux | awk '{printf(\"%%d %%s %%d %%s %%s\\n\", $2, $1, $5, $8, $11)}'");
#else
    buffer = CWCreateStringByCmdStdout("ps aux");
#endif
    if(!buffer)
    {
        return CW_FALSE;
    }

    /* Skip first line */
    c = buffer;
    while(*c != '\n')
    {
        c++;
    }
    c++;

    while(*c != '\0')
    {
        while(*c == ' ' || *c == '\t')
        {
            c++;
        }

        /* PID */
        while(*c != ' ' && *c != '\t')
        {
            c++;
        }
        do
        {
            c++;
        }
        while(*c == ' ' || *c == '\t');

        /* USER */
        do
        {
            c++;
        }
        while(*c != ' ' && *c != '\t');
        do
        {
            c++;
        }
        while(*c == ' ' || *c == '\t');

        /* VSZ */
        mem = strtoul(c, NULL, 10);
        if(mem > maxMem)
        {
            maxMem = mem;

            /* VSZ */
            do
            {
                c++;
            }
            while(*c != ' ' && *c != '\t');
            do
            {
                c++;
            }
            while(*c == ' ' || *c == '\t');

            /* STAT */
            do
            {
                c++;
            }
            while(*c != ' ' && *c != '\t');
            do
            {
                c++;
            }
            while(*c == ' ' || *c == '\t');

            /* COMMAND */
            process = c;
            do
            {
                c++;
            }
            while(*c != ' ' && *c != '\t' && *c != '\n');
            if(*c == '\n')
            {
                *c = '\0';
            }
            else
            {
                *c = '\0';
                do
                {
                    c++;
                }
                while(*c != '\n');
            }
        }
        else
        {
            do
            {
                c++;
            }
            while(*c != '\n');
        }

        c++;
    }

    if(process)
    {
        CW_CREATE_STRING_FROM_STRING_ERR(*processName, process,
        {
            CW_FREE_OBJECT(buffer);
            return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
        });
        *memUsed = maxMem;
    }

    CW_FREE_OBJECT(buffer);

    return process ? CW_TRUE : CWErrorRaise(CW_ERROR_GENERAL, NULL);
}

#ifdef CW_DEBUGGING_MEMORY

typedef struct mem_trace_info
{
    size_t size;
    time_t time;
    char function[64];
    unsigned int line;
    void *ptr;
    struct mem_trace_info *next;
} CWMemTraceInfo;

static CWThreadMutex memInfoLock = CW_MUTEX_INITIALIZER;
static CWMemTraceInfo *memInfoHead = NULL;

void *CWMemAllocTrace(size_t size, const char *function, unsigned int line)
{
    void *ptr = NULL;
    CWMemTraceInfo *info;

    ptr = malloc(size);

    if(ptr)
    {
        CWThreadMutexLock(&memInfoLock);

        info = (CWMemTraceInfo *) malloc(sizeof(CWMemTraceInfo));
        if(info)
        {
            info->ptr = ptr;
            info->size = size;
            info->time = time(NULL);
            strncpy(info->function, function, sizeof(info->function) - 1);
            info->line = line;
            info->next = memInfoHead;
            memInfoHead = info;
        }

        CWThreadMutexUnlock(&memInfoLock);
    }
    else
    {
        printf("no more memory to create info for memory %p at %s:%u\n", ptr, function, line);
    }

    return ptr;
}

void *CWMemZeroAllocTrace(size_t size, const char *function, unsigned int line)
{
    void *ptr;

    if((ptr = CWMemAllocTrace(size, function, line)))
    {
        CW_ZERO_MEMORY(ptr, size);
    }

    return ptr;
}

void CWMemFreeTrace(void *ptr, const char *function, unsigned int line)
{
    CWMemTraceInfo *info, *prev = NULL;
    int found = 0;

    if(ptr)
    {
        CWThreadMutexLock(&memInfoLock);

        info = memInfoHead;
        while(info)
        {
            if(info->ptr == ptr)
            {
                if(prev)
                {
                    prev->next = info->next;
                }
                else
                {
                    memInfoHead = memInfoHead->next;
                }

                free(info);

                found = 1;
                break;
            }

            prev = info;
            info = info->next;
        }

        CWThreadMutexUnlock(&memInfoLock);

        if(!found)
        {
            printf("Could not find info table to free memory %p at %s:%u\n", ptr, function, line);
        }

        free(ptr);
    }
}

void CWMemTracePrint(char *file)
{
    FILE *fp;
    CWMemTraceInfo *info;

    fp = fopen(file, "w");
    if(!fp)
    {
        CWLog("Can not open %s", file);
        return;
    }

    CWThreadMutexLock(&memInfoLock);

    info = memInfoHead;
    while(info)
    {
        fprintf(fp, "%s:%u:%u:%u\n",
                info->function,
                info->line,
                info->size,
                (unsigned int) info->time);

        info = info->next;
    }

    CWThreadMutexUnlock(&memInfoLock);

    fclose(fp);
}

#endif /* CW_DEBUGGING_MEMORY */

CWBool CWGetRadioIndex(CWRadioFreqType radioType, unsigned int capCode, int *radioIdx)
{
      int a=0,b=0;
    
      a=capCode/0x10;
      b=capCode%0x10;

      if( a> 6)
        return CWErrorRaise(CW_ERROR_WRONG_ARG, "Invalid radioType %u", radioType);
      
      if( a == 0)
      {
           switch(b)
           {
              case 1:/*single 2.4G*/
                  if(radioType == CW_RADIOFREQTYPE_2G)
                    *radioIdx = 0;
                  else
                    return CWErrorRaise(CW_ERROR_WRONG_ARG, "Invalid radioType %u", radioType);
                  break;
                  
              case 2:/*single 5G*/
                  if(radioType == CW_RADIOFREQTYPE_5G)
                    *radioIdx = 0;
                  else
                    return CWErrorRaise(CW_ERROR_WRONG_ARG, "Invalid radioType %u", radioType);
                  break;
                  
              case 3:/*dual band: 2.4g-5g or 2.4g-11ac*/
              case 4:
                  if(radioType == CW_RADIOFREQTYPE_2G)
                    *radioIdx = 0;
                  else if(radioType == CW_RADIOFREQTYPE_5G)
                    *radioIdx = 1;
                  else
                    return CWErrorRaise(CW_ERROR_WRONG_ARG, "Invalid radioType %u", radioType);
                  
                  break;
              case 7:/*triband :2.4g-5gac1-5gac2*/
                  if(radioType == CW_RADIOFREQTYPE_2G)
                    *radioIdx = 0;
                  else if(radioType == CW_RADIOFREQTYPE_5G)
                    *radioIdx = 1;
                  else if(radioType == CW_RADIOFREQTYPE_5G_1)
                    *radioIdx = 2;
                  else
                    return CWErrorRaise(CW_ERROR_WRONG_ARG, "Invalid radioType %u", radioType);
                  break;
                  
              default:
                  return CWErrorRaise(CW_ERROR_WRONG_ARG, "Invalid radioType %u", radioType);
           }
      }
      else
      {
           switch(b)
           {
              case 0:/*single 2.4G*/
                  if(radioType == CW_RADIOFREQTYPE_2G)
                    *radioIdx = 0;
                  else
                    return CWErrorRaise(CW_ERROR_WRONG_ARG, "Invalid radioType %u", radioType);
                  break;
                  
              case 1:/*single 5G*/
                  if(radioType == CW_RADIOFREQTYPE_5G)
                    *radioIdx = 0;
                  else
                    return CWErrorRaise(CW_ERROR_WRONG_ARG, "Invalid radioType %u", radioType);
                  break;
                  
              case 2:/*dual band: 2.4g-5g or 2.4g-11ac*/
              case 3:
                  if(radioType == CW_RADIOFREQTYPE_2G)
                    *radioIdx = 0;
                  else if(radioType == CW_RADIOFREQTYPE_5G)
                    *radioIdx = 1;
                  else
                    return CWErrorRaise(CW_ERROR_WRONG_ARG, "Invalid radioType %u", radioType);
                  break;
                  
                  
              case 6:/*triband :2.4g-5gac1-5gac2*/
                  if(radioType == CW_RADIOFREQTYPE_2G)
                    *radioIdx = 0;
                  else if(radioType == CW_RADIOFREQTYPE_5G)
                    *radioIdx = 1;
                  else if(radioType == CW_RADIOFREQTYPE_5G_1)
                    *radioIdx = 2;
                  else
                    return CWErrorRaise(CW_ERROR_WRONG_ARG, "Invalid radioType %u", radioType);
                  break;
                  
              default:
                  return CWErrorRaise(CW_ERROR_WRONG_ARG, "Invalid radioType %u", radioType);
           }
      }

    CWDebugLog("%s radio=%d cap=0x%2x radiotype=%d",__FUNCTION__,*radioIdx,capCode,radioType);
    return CW_TRUE;
}


CWBool CWGetRadioName(CWRadioFreqType radioType,int radioCount,char *radioName)
{
    if(CW_RADIOFREQTYPE_2G == radioType)
    {
       strcpy(radioName,"2G");
       
    }
    else if(CW_RADIOFREQTYPE_5G == radioType)
    {
       if(radioCount > 2)
          strcpy(radioName,"5G-1");
       else
          strcpy(radioName,"5G");
    }
    else if(CW_RADIOFREQTYPE_5G_1== radioType)
    {
       strcpy(radioName,"5G-2");
    }
    else
    {
       CWLog("radioType is invaild");
       return CW_FALSE;
    }

    return CW_TRUE;
}

CWBool CWGetRadioType(CWRadioFreqType *radioType, unsigned int capCode, int radioIdx)
{
      int a=0,b=0;
    
      a=capCode/0x10;
      b=capCode%0x10;

      if( a> 6)
        return CWErrorRaise(CW_ERROR_WRONG_ARG, "Invalid radioType %u", radioType);
      
      if( a == 0)
      {
           switch(b)
           {
              case 1:/*single 2.4G*/
                  if(radioIdx == 0)
                    *radioType = CW_RADIOFREQTYPE_2G;
                  else
                    return CWErrorRaise(CW_ERROR_WRONG_ARG, "Invalid radioType %u", radioType);
                  break;
                  
              case 2:/*single 5G*/
                  if(radioIdx == 0)
                    *radioType = CW_RADIOFREQTYPE_5G;
                  else
                    return CWErrorRaise(CW_ERROR_WRONG_ARG, "Invalid radioType %u", radioType);               
                  break;
                  
              case 3:/*dual band: 2.4g-5g or 2.4g-11ac*/
              case 4:
                  if(radioIdx == 0)
                    *radioType = CW_RADIOFREQTYPE_2G;
                  else if(radioIdx == 1)
                    *radioType = CW_RADIOFREQTYPE_5G;                 
                  else
                    return CWErrorRaise(CW_ERROR_WRONG_ARG, "Invalid radioType %u", radioType);
                  
                  break;
                  
                  
              case 7:/*triband :2.4g-5gac1-5gac2*/
                  if(radioIdx == 0)
                    *radioType = CW_RADIOFREQTYPE_2G;
                  else if(radioIdx == 1)
                    *radioType = CW_RADIOFREQTYPE_5G;
                  else if(radioIdx == 2)
                    *radioType = CW_RADIOFREQTYPE_5G_1;
                  else
                    return CWErrorRaise(CW_ERROR_WRONG_ARG, "Invalid radioType %u", radioType);
                  
                  break;
                  
              default:
                  return CWErrorRaise(CW_ERROR_WRONG_ARG, "Invalid radioType %u", radioType);
           }
      }
      else
      {
           switch(b)
           {
              case 0:/*single 2.4G*/
                  if(radioIdx == 0)
                    *radioType = CW_RADIOFREQTYPE_2G;
                  else
                    return CWErrorRaise(CW_ERROR_WRONG_ARG, "Invalid radioType %u", radioType);
                  break;
                  
              case 1:/*single 5G*/
                  if(radioIdx == 0)
                    *radioType = CW_RADIOFREQTYPE_5G;
                  else
                    return CWErrorRaise(CW_ERROR_WRONG_ARG, "Invalid radioType %u", radioType);               
                  break;
                  
              case 2:/*dual band: 2.4g-5g or 2.4g-11ac*/
              case 3:
                  if(radioIdx == 0)
                    *radioType = CW_RADIOFREQTYPE_2G;
                  else if(radioIdx == 1)
                    *radioType = CW_RADIOFREQTYPE_5G;                 
                  else
                    return CWErrorRaise(CW_ERROR_WRONG_ARG, "Invalid radioType %u", radioType);
                  
                  break;
                  
                  
              case 6:/*triband :2.4g-5gac1-5gac2*/
                  if(radioIdx == 0)
                    *radioType = CW_RADIOFREQTYPE_2G;
                  else if(radioIdx == 1)
                    *radioType = CW_RADIOFREQTYPE_5G;
                  else if(radioIdx == 2)
                    *radioType = CW_RADIOFREQTYPE_5G_1;
                  else
                    return CWErrorRaise(CW_ERROR_WRONG_ARG, "Invalid radioType %u", radioType);
                  
                  break;
                  
              default:
                  return CWErrorRaise(CW_ERROR_WRONG_ARG, "Invalid radioType %u", radioType);
           }
      }

    CWDebugLog("%s radio=%d cap=0x%2x radiotype=%d",__FUNCTION__,radioIdx,capCode,*radioType);
    return CW_TRUE;
}

char *CWStrtokSingle(char *str, char const *delims)
{
    static char *src = NULL;
    char *p, *ret = 0;

    if(str != NULL)
    {
        src = str;
    }

    if(src == NULL)
    {
        return NULL;
    }

    if((p = strpbrk(src, delims)) != NULL)
    {
        *p  = 0;
        ret = src;
        src = ++p;
    }

    return ret;
}

void CWWtpMeshInfoFree(CWWTPMeshInfo *meshInfo)
{
#if 0
    int i;
#endif

    if(!meshInfo)
    {
        return;
    }

#if 0
    for(i = 0; i < meshInfo->infoCount; i++)
    {

    }
#endif
    CW_FREE_OBJECT(meshInfo->info);
}

int CWConvertRadioIdx(int radioIdx)
{
    // RADIO_MODE: only 2.4g=2, onlu 5g=5, 2.4g+5g=7, 2.4g+5g+5g2=12, 5g+5g2=10
    if ( RADIO_MODE % 5 != 2 ) // this model do not have 2.4g radio
        radioIdx++;

    return radioIdx;
}

char *b64_encode (const unsigned char *src, size_t len) 
{
  int i = 0;
  int j = 0;
  char *enc = NULL;
  size_t size = 0;
  unsigned char buf[4];
  unsigned char tmp[3];

  // alloc
  enc = (char *) b64_malloc(1);
  if (NULL == enc) { return NULL; }

  // parse until end of source
  while (len--) {
    // read up to 3 bytes at a time into `tmp'
    tmp[i++] = *(src++);

    // if 3 bytes read then encode into `buf'
    if (3 == i) {
      buf[0] = (tmp[0] & 0xfc) >> 2;
      buf[1] = ((tmp[0] & 0x03) << 4) + ((tmp[1] & 0xf0) >> 4);
      buf[2] = ((tmp[1] & 0x0f) << 2) + ((tmp[2] & 0xc0) >> 6);
      buf[3] = tmp[2] & 0x3f;

      // allocate 4 new byts for `enc` and
      // then translate each encoded buffer
      // part by index from the base 64 index table
      // into `enc' unsigned char array
      enc = (char *) b64_realloc(enc, size + 4);
      for (i = 0; i < 4; ++i) {
        enc[size++] = b64_table[buf[i]];
      }

      // reset index
      i = 0;
    }
  }

  // remainder
  if (i > 0) {
    // fill `tmp' with `\0' at most 3 times
    for (j = i; j < 3; ++j) {
      tmp[j] = '\0';
    }

    // perform same codec as above
    buf[0] = (tmp[0] & 0xfc) >> 2;
    buf[1] = ((tmp[0] & 0x03) << 4) + ((tmp[1] & 0xf0) >> 4);
    buf[2] = ((tmp[1] & 0x0f) << 2) + ((tmp[2] & 0xc0) >> 6);
    buf[3] = tmp[2] & 0x3f;

    // perform same write to `enc` with new allocation
    for (j = 0; (j < i + 1); ++j) {
      enc = (char *) b64_realloc(enc, size + 1);
      enc[size++] = b64_table[buf[j]];
    }

    // while there is still a remainder
    // append `=' to `enc'
    while ((i++ < 3)) {
      enc = (char *) b64_realloc(enc, size + 1);
      enc[size++] = '=';
    }
  }

  // Make sure we have enough space to add '\0' character at end.
  enc = (char *) b64_realloc(enc, size + 1);
  enc[size] = '\0';

  return enc;
}

unsigned char *b64_decode (const char *src, size_t len) 
{
  return b64_decode_ex(src, len, NULL);
}

unsigned char *b64_decode_ex (const char *src, size_t len, size_t *decsize) 
{
  int i = 0;
  int j = 0;
  int l = 0;
  size_t size = 0;
  unsigned char *dec = NULL;
  unsigned char buf[3];
  unsigned char tmp[4];

  // alloc
  dec = (unsigned char *) b64_malloc(1);
  if (NULL == dec) { return NULL; }

  // parse until end of source
  while (len--) {
    // break if char is `=' or not base64 char
    if ('=' == src[j]) { break; }
    if (!(isalnum(src[j]) || '+' == src[j] || '/' == src[j])) { break; }

    // read up to 4 bytes at a time into `tmp'
    tmp[i++] = src[j++];

    // if 4 bytes read then decode into `buf'
    if (4 == i) {
      // translate values in `tmp' from table
      for (i = 0; i < 4; ++i) {
        // find translation char in `b64_table'
        for (l = 0; l < 64; ++l) {
          if (tmp[i] == b64_table[l]) {
            tmp[i] = l;
            break;
          }
        }
      }

      // decode
      buf[0] = (tmp[0] << 2) + ((tmp[1] & 0x30) >> 4);
      buf[1] = ((tmp[1] & 0xf) << 4) + ((tmp[2] & 0x3c) >> 2);
      buf[2] = ((tmp[2] & 0x3) << 6) + tmp[3];

      // write decoded buffer to `dec'
      dec = (unsigned char *) b64_realloc(dec, size + 3);
      if (dec != NULL){
        for (i = 0; i < 3; ++i) {
          dec[size++] = buf[i];
        }
      } else {
        return NULL;
      }

      // reset
      i = 0;
    }
  }

  // remainder
  if (i > 0) {
    // fill `tmp' with `\0' at most 4 times
    for (j = i; j < 4; ++j) {
      tmp[j] = '\0';
    }

    // translate remainder
    for (j = 0; j < 4; ++j) {
        // find translation char in `b64_table'
        for (l = 0; l < 64; ++l) {
          if (tmp[j] == b64_table[l]) {
            tmp[j] = l;
            break;
          }
        }
    }

    // decode remainder
    buf[0] = (tmp[0] << 2) + ((tmp[1] & 0x30) >> 4);
    buf[1] = ((tmp[1] & 0xf) << 4) + ((tmp[2] & 0x3c) >> 2);
    buf[2] = ((tmp[2] & 0x3) << 6) + tmp[3];

    // write remainer decoded buffer to `dec'
    dec = (unsigned char *) b64_realloc(dec, size + (i - 1));
    if (dec != NULL){
      for (j = 0; (j < i - 1); ++j) {
        dec[size++] = buf[j];
      }
    } else {
      return NULL;
    }
  }

  // Make sure we have enough space to add '\0' character at end.
  dec = (unsigned char *) b64_realloc(dec, size + 1);
  if (dec != NULL){
    dec[size] = '\0';
  } else {
    return NULL;
  }

  // Return back the size of decoded string if demanded.
  if (decsize != NULL) {
    *decsize = size;
  }

  return dec;
}


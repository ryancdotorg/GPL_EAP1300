/*
* mi_shadow_datatype.h- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: XXXX <XXXX@sigmastar.com.cn>
*
* This software is licensed under the terms of the GNU General Public
* License version 2, as published by the Free Software Foundation, and
* may be copied, distributed, and modified under those terms.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
*/
#ifndef _MI_SHADOW_DATATYPE_H_
#define _MI_SHADOW_DATATYPE_H_

#include "mi_sys_datatype.h"

#define MI_SHADOW_DUMP_BUFFER_SIZE (12 * 1024)

typedef enum
{
    MI_SHADOW_CALLBACK_EVENT_ONBINDINPUTPORT,
    MI_SHADOW_CALLBACK_EVENT_ONBINDOUTPUTPORT,
    MI_SHADOW_CALLBACK_EVENT_ONUNBINDINPUTPORT,
    MI_SHADOW_CALLBACK_EVENT_ONUNBINDOUTPUTPORT,
    MI_SHADOW_CALLBACK_EVENT_BINDMAX,

    MI_SHADOW_CALLBACK_EVENT_ONDumpDEVATTR,
    MI_SHADOW_CALLBACK_EVENT_ONDumpCHANNELATTR,
    MI_SHADOW_CALLBACK_EVENT_ONDumpINPUTPORTATTR,
    MI_SHADOW_CALLBACK_EVENT_ONDumpOUTPUTPORTATTR,
    MI_SHADOW_CALLBACK_EVENT_ONHELP,
    MI_SHADOW_CALLBACK_EVENT_DUMPMAX,

    MI_SHADOW_CALLBACK_EVENT_CMDDEBUGLEVEL,
    MI_SHADOW_CALLBACK_EVENT_CMDCHECKPOINT,
    MI_SHADOW_CALLBACK_EVENT_CMDSELFCHECKING,
    MI_SHADOW_CALLBACK_EVENT_CMDWORKMODEENABLE,
    MI_SHADOW_CALLBACK_EVENT_CMDCHNENABLE,
    MI_SHADOW_CALLBACK_EVENT_CMDDUMPALLENABLE,
    MI_SHADOW_CALLBACK_EVENT_CMDDUMPIMAGE,
    MI_SHADOW_CALLBACK_EVENT_CMDMAX,

    MI_SHADOW_CALLBACK_EVENT_EXIT,
    MI_SHADOW_CALLBACK_EVENT_TIMEOUT,
    MI_SHADOW_CALLBACK_EVENT_MAX,
} MI_SHADOW_CALLBACK_EVENT_e;

typedef struct MI_SHADOW_ModuleDevInfo_s
{
    MI_ModuleId_e eModuleId;
    MI_U32 u32DevId;
    MI_U32 u32InputPortNum;
    MI_U32 u32OutputPortNum;
    MI_U32 u32DevChnNum;
}MI_SHADOW_ModuleDevInfo_t;

#define MI_SHADOW_HANDLE MI_HANDLE
typedef MI_S32 (* MI_SHADOW_Callback)(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort, void *pUsrData);
typedef MI_S32 (* MI_SHADOW_DumpCallback)(MI_SHADOW_HANDLE handle, MI_U32  u32DevId, void *pUsrData);
typedef MI_S32 (* MI_SHADOW_CmdCallback)(MI_SHADOW_HANDLE handle, MI_U32 u32DevId, MI_U8 argc, MI_S8 **argv, void *pUsrData);

typedef struct MI_SHADOW_RegisterDevParams_s
{
    MI_SHADOW_ModuleDevInfo_t stModDevInfo;

    MI_SHADOW_Callback OnBindInputPort;
    MI_SHADOW_Callback OnBindOutputPort;
    MI_SHADOW_Callback OnUnBindInputPort;
    MI_SHADOW_Callback OnUnBindOutputPort;

    MI_SHADOW_DumpCallback OnDumpDevAttr;
    MI_SHADOW_DumpCallback OnDumpChannelAttr;
    MI_SHADOW_DumpCallback OnDumpInputPortAttr;
    MI_SHADOW_DumpCallback OnDumpOutPortAttr;
    MI_SHADOW_DumpCallback OnHelp;

    MI_SHADOW_CmdCallback CmdDebugLevel;
    MI_SHADOW_CmdCallback CmdCheckPoint;
    MI_SHADOW_CmdCallback CmdSelfChecking;
    MI_SHADOW_CmdCallback CmdWorkModeEnable;
    MI_SHADOW_CmdCallback CmdChnEnable;
    MI_SHADOW_CmdCallback CmdDumpAllEnable;
    MI_SHADOW_CmdCallback CmdDumpImage;

    void *pUsrData;
    MI_U8 *pDumpBuf;
} MI_SHADOW_RegisterDevParams_t;
#endif ///_MI_SHADOW_DATATYPE_H_

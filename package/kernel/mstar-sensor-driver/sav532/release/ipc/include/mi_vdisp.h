/*
* mi_vdisp.h- Sigmastar
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
#ifndef _MI_VDISP_H_
#define _MI_VDISP_H_
#include "mi_vdisp_datatype.h"

#define VDISP_MAJOR_VERSION 1
#define VDISP_SUB_VERSION 3
#define MACRO_TO_STR(macro) #macro
#define VDISP_VERSION_STR(major_version,sub_version) ({char *tmp = sub_version/100 ? \
                                    "mi_vdisp_version_" MACRO_TO_STR(major_version)"." MACRO_TO_STR(sub_version) : sub_version/10 ? \
                                    "mi_vdisp_version_" MACRO_TO_STR(major_version)".0" MACRO_TO_STR(sub_version) : \
                                    "mi_vdisp_version_" MACRO_TO_STR(major_version)".00" MACRO_TO_STR(sub_version);tmp;})
#define MI_VDISP_API_VERSION VDISP_VERSION_STR(VDISP_MAJOR_VERSION,VDISP_SUB_VERSION)

#ifdef __cplusplus
extern "C" {
#endif

MI_S32 MI_VDISP_Init(void);
MI_S32 MI_VDISP_Exit(void);

MI_S32 MI_VDISP_OpenDevice(MI_VDISP_DEV DevId);
MI_S32 MI_VDISP_CloseDevice(MI_VDISP_DEV DevId);

MI_S32 MI_VDISP_SetOutputPortAttr(MI_VDISP_DEV DevId,
               MI_VDISP_PORT PortId,
               MI_VDISP_OutputPortAttr_t *pstOutputPortAttr);
MI_S32 MI_VDISP_GetOutputPortAttr(MI_VDISP_DEV DevId,
               MI_VDISP_PORT PortId,
               MI_VDISP_OutputPortAttr_t *pstOutputPortAttr);

MI_S32 MI_VDISP_SetInputPortAttr(MI_VDISP_DEV DevId,
               MI_VDISP_PORT PortId,
               MI_VDISP_InputPortAttr_t *pstInputPortAttr);
MI_S32 MI_VDISP_GetInputPortAttr(MI_VDISP_DEV DevId,
               MI_VDISP_PORT PortId,
               MI_VDISP_InputPortAttr_t *pstInputPortAttr);

MI_S32 MI_VDISP_EnableInputPort(MI_VDISP_DEV DevId,
               MI_VDISP_PORT PortId);
MI_S32 MI_VDISP_DisableInputPort(MI_VDISP_DEV DevId,
               MI_VDISP_PORT PortId);

MI_S32 MI_VDISP_StartDev(MI_VDISP_DEV DevId);
MI_S32 MI_VDISP_StopDev(MI_VDISP_DEV DevId);

#ifdef __cplusplus
}
#endif

#endif///_MI_VDISP_H_

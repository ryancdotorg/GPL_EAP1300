/*
* mi_ldc.h- Sigmastar
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
#ifndef __MI_LDC_H__
#define __MI_LDC_H__

#include "mi_ldc_datatype.h"

#define LDC_MAJOR_VERSION 1
#define LDC_SUB_VERSION 3
#define MACRO_TO_STR(macro) #macro
#define LDC_VERSION_STR(major_version,sub_version) ({char *tmp = sub_version/100 ? \
                                    "mi_ldc_version_" MACRO_TO_STR(major_version)"." MACRO_TO_STR(sub_version) : sub_version/10 ? \
                                    "mi_ldc_version_" MACRO_TO_STR(major_version)".0" MACRO_TO_STR(sub_version) : \
                                    "mi_ldc_version_" MACRO_TO_STR(major_version)".00" MACRO_TO_STR(sub_version);tmp;})
#define MI_LDC_API_VERSION LDC_VERSION_STR(LDC_MAJOR_VERSION,LDC_SUB_VERSION)

#ifdef __cplusplus
extern "C" {
#endif


MI_S32 MI_LDC_CreateDevice(MI_LDC_DEV devId, MI_LDC_WorkMode_e eWorkMdoe);
MI_S32 MI_LDC_DestroyDevice(MI_LDC_DEV devId);

MI_S32 MI_LDC_CreateChannel(MI_LDC_DEV devId, MI_LDC_CHN chnId, MI_LDC_ChannelAttr_t *pstChnlAttr);
MI_S32 MI_LDC_DestroyChannel(MI_LDC_DEV devId, MI_LDC_CHN chnId);

MI_S32 MI_LDC_StartChannel(MI_LDC_DEV devId, MI_LDC_CHN chnId);
MI_S32 MI_LDC_StopChannel(MI_LDC_DEV devId, MI_LDC_CHN chnId);

MI_S32 MI_LDC_SetOutputPortAttr(MI_LDC_DEV devId, MI_LDC_CHN chnId, MI_LDC_OutputPortAttr_t *pstOutputAttr);
MI_S32 MI_LDC_GetOutputPortAttr(MI_LDC_DEV devId, MI_LDC_CHN chnId, MI_LDC_OutputPortAttr_t *pstOutputAttr);

MI_S32 MI_LDC_SetConfig(MI_LDC_DEV devId, MI_LDC_CHN chnId, void *pConfigAddr, MI_U32 u32Size);

#ifdef __cplusplus
}
#endif

#endif///_MI_VPE_H_

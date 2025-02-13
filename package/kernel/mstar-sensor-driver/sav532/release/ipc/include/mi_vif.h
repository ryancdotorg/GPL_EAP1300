/*
* mi_vif.h- Sigmastar
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
#ifndef _MI_VIF_H_
#define _MI_VIF_H_


#include "mi_vif_datatype.h"

#define VIF_MAJOR_VERSION 1
#define VIF_SUB_VERSION 3
#define MACRO_TO_STR(macro) #macro
#define VIF_VERSION_STR(major_version,sub_version) ({char *tmp = sub_version/100 ? \
                                    "mi_vif_version_" MACRO_TO_STR(major_version)"." MACRO_TO_STR(sub_version) : sub_version/10 ? \
                                    "mi_vif_version_" MACRO_TO_STR(major_version)".0" MACRO_TO_STR(sub_version) : \
                                    "mi_vif_version_" MACRO_TO_STR(major_version)".00" MACRO_TO_STR(sub_version);tmp;})
#define MI_VIF_API_VERSION VIF_VERSION_STR(VIF_MAJOR_VERSION,VIF_SUB_VERSION)

#ifdef __cplusplus
extern "C" {
#endif

MI_S32 MI_VIF_SetDevAttr(MI_VIF_DEV u32VifDev, MI_VIF_DevAttr_t *pstDevAttr);
MI_S32 MI_VIF_GetDevAttr(MI_VIF_DEV u32VifDev, MI_VIF_DevAttr_t *pstDevAttr);
MI_S32 MI_VIF_EnableDev(MI_VIF_DEV u32VifDev);
MI_S32 MI_VIF_DisableDev(MI_VIF_DEV u32VifDev);
MI_S32 MI_VIF_SetChnPortAttr(MI_VIF_CHN u32VifChn, MI_VIF_PORT u32ChnPort, MI_VIF_ChnPortAttr_t *pstAttr);
MI_S32 MI_VIF_GetChnPortAttr(MI_VIF_CHN u32VifChn, MI_VIF_PORT u32ChnPort, MI_VIF_ChnPortAttr_t *pstAttr);
MI_S32 MI_VIF_EnableChnPort(MI_VIF_CHN u32VifChn, MI_VIF_PORT u32ChnPort);
MI_S32 MI_VIF_DisableChnPort(MI_VIF_CHN u32VifChn, MI_VIF_PORT u32ChnPort);
MI_S32 MI_VIF_Query(MI_VIF_CHN u32VifChn, MI_VIF_PORT u32ChnPort, MI_VIF_ChnPortStat_t *pstStat);
MI_S32 MI_VIF_SetDev2SnrPadMux(MI_VIF_Dev2SnrPadMuxCfg_t *pstVifDevMap, MI_U8 u8Length);

#ifdef __cplusplus
}
#endif

#endif///_MI_VIF_H_


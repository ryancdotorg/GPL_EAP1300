/*
* mi_vdec.h- Sigmastar
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
#ifndef _MI_VDEC_H_
#define _MI_VDEC_H_

#include "mi_vdec_datatype.h"

#define VDEC_MAJOR_VERSION 1
#define VDEC_SUB_VERSION 3
#define MACRO_TO_STR(macro) #macro
#define VDEC_VERSION_STR(major_version,sub_version) ({char *tmp = sub_version/100 ? \
                                    "mi_vdec_version_" MACRO_TO_STR(major_version)"." MACRO_TO_STR(sub_version) : sub_version/10 ? \
                                    "mi_vdec_version_" MACRO_TO_STR(major_version)".0" MACRO_TO_STR(sub_version) : \
                                    "mi_vdec_version_" MACRO_TO_STR(major_version)".00" MACRO_TO_STR(sub_version);tmp;})
#define MI_VDEC_API_VERSION VDEC_VERSION_STR(VDEC_MAJOR_VERSION,VDEC_SUB_VERSION)

#ifdef __cplusplus
extern "C" {
#endif

MI_S32 MI_VDEC_CreateChn(MI_VDEC_CHN VdecChn, MI_VDEC_ChnAttr_t *pstChnAttr);
MI_S32 MI_VDEC_DestroyChn(MI_VDEC_CHN VdecChn);
MI_S32 MI_VDEC_GetChnAttr(MI_VDEC_CHN VdecChn, MI_VDEC_ChnAttr_t *pstChnAttr);
MI_S32 MI_VDEC_StartChn(MI_VDEC_CHN VdecChn);
MI_S32 MI_VDEC_StopChn(MI_VDEC_CHN VdecChn);
MI_S32 MI_VDEC_GetChnStat(MI_VDEC_CHN VdecChn, MI_VDEC_ChnStat_t *pstChnStat);
MI_S32 MI_VDEC_ResetChn(MI_VDEC_CHN VdecChn);
MI_S32 MI_VDEC_SetChnParam(MI_VDEC_CHN VdecChn, MI_VDEC_ChnParam_t *pstChnParam);
MI_S32 MI_VDEC_GetChnParam(MI_VDEC_CHN VdecChn, MI_VDEC_ChnParam_t *pstChnParam);
MI_S32 MI_VDEC_SendStream(MI_VDEC_CHN VdecChn, MI_VDEC_VideoStream_t *pstVideoStream, MI_S32 s32MilliSec);
MI_S32 MI_VDEC_GetUserData(MI_VDEC_CHN VdecChn, MI_VDEC_UserData_t *pstUserData, MI_S32 s32MilliSec);
MI_S32 MI_VDEC_ReleaseUserData(MI_VDEC_CHN VdecChn, MI_VDEC_UserData_t *pstUserData);
MI_S32 MI_VDEC_SetDisplayMode(MI_VDEC_CHN VdecChn, MI_VDEC_DisplayMode_e eDisplayMode);
MI_S32 MI_VDEC_GetDisplayMode(MI_VDEC_CHN VdecChn, MI_VDEC_DisplayMode_e *peDisplayMode);

#ifdef __cplusplus
}
#endif

#endif///_MI_VDEC_H_

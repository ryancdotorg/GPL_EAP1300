/*
* mi_gfx.h- Sigmastar
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
#ifndef _MI_GFX_H_
#define _MI_GFX_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "mi_common.h"
#include "mi_gfx_datatype.h"

#define GFX_MAJOR_VERSION 1
#define GFX_SUB_VERSION 3
#define MACRO_TO_STR(macro) #macro
#define GFX_VERSION_STR(major_version,sub_version) ({char *tmp = sub_version/100 ? \
                                    "mi_gfx_version_" MACRO_TO_STR(major_version)"." MACRO_TO_STR(sub_version) : sub_version/10 ? \
                                    "mi_gfx_version_" MACRO_TO_STR(major_version)".0" MACRO_TO_STR(sub_version) : \
                                    "mi_gfx_version_" MACRO_TO_STR(major_version)".00" MACRO_TO_STR(sub_version);tmp;})
#define MI_GFX_API_VERSION GFX_VERSION_STR(GFX_MAJOR_VERSION,GFX_SUB_VERSION)

/*-------------------------------------------------------------------------------------------
 * Global Functions
-------------------------------------------------------------------------------------------*/

MI_S32 MI_GFX_Open(void);
MI_S32 MI_GFX_Close(void);
MI_S32 MI_GFX_WaitAllDone(MI_BOOL bWaitAllDone, MI_U16 u16TargetFence);
MI_S32 MI_GFX_QuickFill(MI_GFX_Surface_t *pstDst, MI_GFX_Rect_t *pstDstRect,
    MI_U32 u32ColorVal, MI_U16 *pu16Fence);
MI_S32 MI_GFX_GetAlphaThresholdValue(MI_U8 *pu8ThresholdValue);
MI_S32 MI_GFX_SetAlphaThresholdValue(MI_U8 u8ThresholdValue);
MI_S32 MI_GFX_BitBlit(MI_GFX_Surface_t *pstSrc, MI_GFX_Rect_t *pstSrcRect,
    MI_GFX_Surface_t *pstDst,  MI_GFX_Rect_t *pstDstRect, MI_GFX_Opt_t *pstOpt, MI_U16 *pu16Fence);

#ifdef __cplusplus
}
#endif

#endif //_MI_GFX_H_

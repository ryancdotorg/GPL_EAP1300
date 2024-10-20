/*
* mi_rgn.h- Sigmastar
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
#ifndef _MI_RGN_H_
#define _MI_RGN_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "mi_common.h"
#include "mi_rgn_datatype.h"

#define RGN_MAJOR_VERSION 1
#define RGN_SUB_VERSION 3
#define MACRO_TO_STR(macro) #macro
#define RGN_VERSION_STR(major_version,sub_version) ({char *tmp = sub_version/100 ? \
                                    "mi_rgn_version_" MACRO_TO_STR(major_version)"." MACRO_TO_STR(sub_version) : sub_version/10 ? \
                                    "mi_rgn_version_" MACRO_TO_STR(major_version)".0" MACRO_TO_STR(sub_version) : \
                                    "mi_rgn_version_" MACRO_TO_STR(major_version)".00" MACRO_TO_STR(sub_version);tmp;})
#define MI_RGN_API_VERSION RGN_VERSION_STR(RGN_MAJOR_VERSION,RGN_SUB_VERSION)

MI_S32 MI_RGN_Init(MI_RGN_PaletteTable_t *pstPaletteTable);
MI_S32 MI_RGN_DeInit(void);
MI_S32 MI_RGN_Create(MI_RGN_HANDLE hHandle, MI_RGN_Attr_t *pstRegion);
MI_S32 MI_RGN_Destroy (MI_RGN_HANDLE hHandle);
MI_S32 MI_RGN_GetAttr(MI_RGN_HANDLE hHandle, MI_RGN_Attr_t *pstRegion);
MI_S32 MI_RGN_SetBitMap(MI_RGN_HANDLE hHandle, MI_RGN_Bitmap_t *pstBitmap);
MI_S32 MI_RGN_AttachToChn(MI_RGN_HANDLE hHandle, MI_RGN_ChnPort_t* pstChnPort, MI_RGN_ChnPortParam_t *pstChnAttr);
MI_S32 MI_RGN_DetachFromChn(MI_RGN_HANDLE hHandle, MI_RGN_ChnPort_t *pstChnPort);
MI_S32 MI_RGN_SetDisplayAttr(MI_RGN_HANDLE hHandle, MI_RGN_ChnPort_t *pstChnPort, MI_RGN_ChnPortParam_t *pstChnPortAttr);
MI_S32 MI_RGN_GetDisplayAttr(MI_RGN_HANDLE hHandle, MI_RGN_ChnPort_t *pstChnPort, MI_RGN_ChnPortParam_t *pstChnPortAttr);
MI_S32 MI_RGN_GetCanvasInfo(MI_RGN_HANDLE hHandle, MI_RGN_CanvasInfo_t* pstCanvasInfo);
MI_S32 MI_RGN_UpdateCanvas(MI_RGN_HANDLE hHandle);
MI_S32 MI_RGN_ScaleRect(MI_RGN_ChnPort_t *pstChnPort, MI_RGN_Size_t *pstCanvasSize, MI_RGN_Size_t *pstScreenSize);

#ifdef __cplusplus
}
#endif

#endif //_MI_RGN_H_

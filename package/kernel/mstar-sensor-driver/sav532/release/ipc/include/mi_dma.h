/*
* mi_dma.h- Sigmastar
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
#ifndef _MI_DMA_H_
#define _MI_DMA_H_

#include "mi_common.h"
#include "mi_sys.h"
#include "mi_dma_datatype.h"

#ifdef __cplusplus
extern "C" {
#endif

MI_S32 MI_DMA_MemsetPa(MI_PHY phyPa, MI_U32 u32Val, MI_U32 u32Lenth);
MI_S32 MI_DMA_MemcpyPa(MI_PHY phyDst, MI_PHY phySrc, MI_U32 u32Lenth);
MI_S32 MI_DMA_BufFillPa(MI_SYS_FrameData_t *pstBuf, MI_U32 u32Val, MI_SYS_WindowRect_t *pstRect);
MI_S32 MI_DMA_BufBlitPa(MI_SYS_FrameData_t *pstDstBuf, MI_SYS_WindowRect_t *pstDstRect, MI_SYS_FrameData_t *pstSrcBuf, MI_SYS_WindowRect_t *pstSrcRect);
#ifdef __cplusplus
}
#endif

#endif //_MI_DMA_H_
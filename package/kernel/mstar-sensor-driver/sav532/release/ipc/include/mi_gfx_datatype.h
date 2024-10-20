/*
* mi_gfx_datatype.h- Sigmastar
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
#ifndef _MI_GFX_DATATYPE_H_
#define _MI_GFX_DATATYPE_H_
#ifdef __cplusplus
extern "C" {
#endif
#include "mi_common.h"
#include "mi_gfx_datatype.h"

typedef enum
{
    E_MI_GFX_FMT_I1 = 0, /* MS_ColorFormat */
    E_MI_GFX_FMT_I2,
    E_MI_GFX_FMT_I4,
    E_MI_GFX_FMT_I8,
    E_MI_GFX_FMT_FABAFGBG2266,
    E_MI_GFX_FMT_1ABFGBG12355,
    E_MI_GFX_FMT_RGB565,
    E_MI_GFX_FMT_ARGB1555,
    E_MI_GFX_FMT_ARGB4444,
    E_MI_GFX_FMT_ARGB1555_DST,
    E_MI_GFX_FMT_YUV422,
    E_MI_GFX_FMT_ARGB8888,
    E_MI_GFX_FMT_RGBA5551,
    E_MI_GFX_FMT_RGBA4444,
    E_MI_GFX_FMT_ABGR8888,
    E_MI_GFX_FMT_BGRA5551,
    E_MI_GFX_FMT_ABGR1555,
    E_MI_GFX_FMT_ABGR4444,
    E_MI_GFX_FMT_BGRA4444,
    E_MI_GFX_FMT_BGR565,
    E_MI_GFX_FMT_RGBA8888,
    E_MI_GFX_FMT_BGRA8888,
    E_MI_GFX_FMT_MAX
} MI_GFX_ColorFmt_e;


typedef enum
{
    E_MI_GFX_ROP_BLACK = 0,     /*Blackness*/
    E_MI_GFX_ROP_NOTMERGEPEN,   /*~(S2+S1)*/
    E_MI_GFX_ROP_MASKNOTPEN,    /*~S2&S1*/
    E_MI_GFX_ROP_NOTCOPYPEN,    /* ~S2*/
    E_MI_GFX_ROP_MASKPENNOT,    /* S2&~S1 */
    E_MI_GFX_ROP_NOT,           /* ~S1 */
    E_MI_GFX_ROP_XORPEN,        /* S2^S1 */
    E_MI_GFX_ROP_NOTMASKPEN,    /* ~(S2&S1) */
    E_MI_GFX_ROP_MASKPEN,       /* S2&S1 */
    E_MI_GFX_ROP_NOTXORPEN,     /* ~(S2^S1) */
    E_MI_GFX_ROP_NOP,           /* S1 */
    E_MI_GFX_ROP_MERGENOTPEN,   /* ~S2+S1 */
    E_MI_GFX_ROP_COPYPEN,       /* S2 */
    E_MI_GFX_ROP_MERGEPENNOT,   /* S2+~S1 */
    E_MI_GFX_ROP_MERGEPEN,      /* S2+S1 */
    E_MI_GFX_ROP_WHITE,         /* Whiteness */
    E_MI_GFX_ROP_NONE,          /* No Rop Op */
    E_MI_GFX_ROP_MAX,
} MI_GFX_RopCode_e;

typedef enum
{
    E_MI_GFX_RGB_OP_EQUAL = 0,
    E_MI_GFX_RGB_OP_NOT_EQUAL,
    E_MI_GFX_ALPHA_OP_EQUAL,
    E_MI_GFX_ALPHA_OP_NOT_EQUAL,
    E_MI_GFX_ARGB_OP_EQUAL,
    E_MI_GFX_ARGB_OP_NOT_EQUAL,
    E_MI_GFX_CKEY_OP_MAX,
} MI_GFX_ColorKeyOp_e;

typedef enum
{
    E_MI_GFX_DFB_BLD_ZERO = 0,
    E_MI_GFX_DFB_BLD_ONE,
    E_MI_GFX_DFB_BLD_SRCCOLOR,
    E_MI_GFX_DFB_BLD_INVSRCCOLOR,
    E_MI_GFX_DFB_BLD_SRCALPHA,
    E_MI_GFX_DFB_BLD_INVSRCALPHA,
    E_MI_GFX_DFB_BLD_DESTALPHA,
    E_MI_GFX_DFB_BLD_INVDESTALPHA,
    E_MI_GFX_DFB_BLD_DESTCOLOR,
    E_MI_GFX_DFB_BLD_INVDESTCOLOR,
    E_MI_GFX_DFB_BLD_SRCALPHASAT,
    E_MI_GFX_DFB_BLD_NONE,
    E_MI_GFX_DFB_BLD_MAX,
} MI_GFX_DfbBldOp_e;

typedef enum
{
    E_MI_GFX_MIRROR_NONE = 0,
    E_MI_GFX_MIRROR_HORIZONTAL,
    E_MI_GFX_MIRROR_VERTICAL,
    E_MI_GFX_MIRROR_BOTH,
    E_MI_GFX_MIRROR_MAX
} MI_GFX_Mirror_e;

typedef enum
{
    E_MI_GFX_ROTATE_0 = 0,
    E_MI_GFX_ROTATE_90,
    E_MI_GFX_ROTATE_180,
    E_MI_GFX_ROTATE_270,
    E_MI_GFX_ROTATE_MAX
} MI_GFX_Rotate_e;

typedef enum
{
    E_MI_GFX_YUV_YVYU = 0, // YUV422 format
    E_MI_GFX_YUV_YUYV,
    E_MI_GFX_YUV_VYUY,
    E_MI_GFX_YUV_UYVY,
    E_MI_GFX_YUV_MAX
} MI_GFX_Yuv422_e;

typedef enum
{
    E_MI_GFX_ERR_NOT_INIT = MI_GFX_INITIAL_ERROR_CODE,
    E_MI_GFX_ERR_GFX_DRV_NOT_SUPPORT,
    E_MI_GFX_ERR_GFX_DRV_FAIL_FORMAT,
    E_MI_GFX_ERR_GFX_NON_ALIGN_ADDRESS,
    E_MI_GFX_ERR_GFX_NON_ALIGN_PITCH,
    E_MI_GFX_ERR_GFX_DRV_FAIL_OVERLAP,
    E_MI_GFX_ERR_GFX_DRV_FAIL_STRETCH,
    E_MI_GFX_ERR_GFX_DRV_FAIL_ITALIC,
    E_MI_GFX_ERR_GFX_DRV_FAIL_LOCKED,
    E_MI_GFX_ERR_GFX_DRV_FAIL_BLTADDR,
    E_MI_GFX_ERR_MAX
} MI_GFX_ErrCode_e;


/* GFX Module ErrorCode */
#define MI_ERR_GFX_INVALID_PARAM MI_DEF_ERR(E_MI_MODULE_ID_GFX, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_ILLEGAL_PARAM)
#define MI_ERR_GFX_INVALID_DEVID MI_DEF_ERR(E_MI_MODULE_ID_GFX, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_INVALID_DEVID)
#define MI_ERR_GFX_DEV_BUSY MI_DEF_ERR(E_MI_MODULE_ID_GFX, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_BUSY)

#define MI_ERR_GFX_NOT_INIT MI_DEF_ERR(E_MI_MODULE_ID_GFX, E_MI_ERR_LEVEL_ERROR, E_MI_GFX_ERR_NOT_INIT)
#define MI_ERR_GFX_DRV_NOT_SUPPORT MI_DEF_ERR(E_MI_MODULE_ID_GFX, E_MI_ERR_LEVEL_ERROR, E_MI_GFX_ERR_GFX_DRV_NOT_SUPPORT)
#define MI_ERR_GFX_DRV_FAIL_FORMAT MI_DEF_ERR(E_MI_MODULE_ID_GFX, E_MI_ERR_LEVEL_ERROR, E_MI_GFX_ERR_GFX_DRV_FAIL_FORMAT)
#define MI_ERR_GFX_NON_ALIGN_ADDRESS MI_DEF_ERR(E_MI_MODULE_ID_GFX, E_MI_ERR_LEVEL_ERROR, E_MI_GFX_ERR_GFX_NON_ALIGN_ADDRESS)
#define MI_ERR_GFX_NON_ALIGN_PITCH MI_DEF_ERR(E_MI_MODULE_ID_GFX, E_MI_ERR_LEVEL_ERROR, E_MI_GFX_ERR_GFX_NON_ALIGN_PITCH)
#define MI_ERR_GFX_DRV_FAIL_OVERLAP MI_DEF_ERR(E_MI_MODULE_ID_GFX, E_MI_ERR_LEVEL_ERROR, E_MI_GFX_ERR_GFX_DRV_FAIL_OVERLAP)
#define MI_ERR_GFX_DRV_FAIL_STRETCH MI_DEF_ERR(E_MI_MODULE_ID_GFX, E_MI_ERR_LEVEL_ERROR, E_MI_GFX_ERR_GFX_DRV_FAIL_STRETCH)
#define MI_ERR_GFX_DRV_FAIL_ITALIC MI_DEF_ERR(E_MI_MODULE_ID_GFX, E_MI_ERR_LEVEL_ERROR, E_MI_GFX_ERR_GFX_DRV_FAIL_ITALIC)
#define MI_ERR_GFX_DRV_FAIL_LOCKED MI_DEF_ERR(E_MI_MODULE_ID_GFX, E_MI_ERR_LEVEL_ERROR, E_MI_GFX_ERR_GFX_DRV_FAIL_LOCKED)
#define MI_ERR_GFX_DRV_FAIL_BLTADDR MI_DEF_ERR(E_MI_MODULE_ID_GFX, E_MI_ERR_LEVEL_ERROR, E_MI_GFX_ERR_GFX_DRV_FAIL_BLTADDR)

typedef struct MI_GFX_Rect_s
{
    MI_S32 s32Xpos;
    MI_S32 s32Ypos;
    MI_U32 u32Width;
    MI_U32 u32Height;
} MI_GFX_Rect_t;

typedef struct MI_GFX_ColorKey_s
{
    MI_U32 u32ColorStart;
    MI_U32 u32ColorEnd;
} MI_GFX_ColorKeyValue_t;

typedef struct MI_GFX_ColorKeyInfo_s
{
    MI_BOOL bEnColorKey;
    MI_GFX_ColorKeyOp_e eCKeyOp;
    MI_GFX_ColorFmt_e eCKeyFmt;
    MI_GFX_ColorKeyValue_t stCKeyVal;
} MI_GFX_ColorKeyInfo_t;

typedef struct MI_GFX_Surface_s
{
    MI_PHY phyAddr;
    MI_GFX_ColorFmt_e eColorFmt;
    MI_U32 u32Width;
    MI_U32 u32Height;
    MI_U32 u32Stride;
} MI_GFX_Surface_t;

typedef struct MI_GFX_Opt_s
{
    MI_BOOL bEnGfxRop;
    MI_GFX_RopCode_e eRopCode;
    MI_GFX_Rect_t stClipRect;
    MI_GFX_ColorKeyInfo_t stSrcColorKeyInfo;
    MI_GFX_ColorKeyInfo_t stDstColorKeyInfo;
    MI_GFX_DfbBldOp_e eSrcDfbBldOp;
    MI_GFX_DfbBldOp_e eDstDfbBldOp;
    MI_GFX_Mirror_e eMirror;
    MI_GFX_Yuv422_e eSrcYuvFmt;
    MI_GFX_Yuv422_e eDstYuvFmt;
    MI_GFX_Rotate_e eRotate;
} MI_GFX_Opt_t;

#ifdef __cplusplus
}
#endif

#endif///_MI_GFX_DATATYPE_H_

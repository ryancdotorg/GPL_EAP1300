/*
* mi_ldc_datatype.h- Sigmastar
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
#ifndef _MI_LDC_DATATYPE_H_
#define _MI_LDC_DATATYPE_H_
#include "mi_sys_datatype.h"

#ifdef __cplusplus
extern "C" {
#endif


#define MI_LDC_MAX_DEVICE_NUM           1
#define MI_LDC_MAX_CHN_NUM              16
#define MI_LDC_MAX_INPUTPORT_NUM        1
#define MI_LDC_MAX_OUTPUTPORT_NUM       1
#define MI_LDC_MAX_VIEW_NUM             10

typedef enum
{
    E_MI_LDC_ERR_DEV_CREATED = MI_LDC_INITIAL_ERROR_CODE,           // dev has been created
    E_MI_LDC_ERR_DEV_NOT_CREATE,                                    // dev not be created
    E_MI_LDC_ERR_DEV_NOT_DESTROY,                                   // dev not be destroyed
    E_MI_LDC_ERR_CHN_CREATED,                                       // chn has been created
    E_MI_LDC_ERR_CHN_NOT_CREATE,                                    // chn not be created
    E_MI_LDC_ERR_CHN_NOT_STOP,                                      // chn is still working
    E_MI_LDC_ERR_CHN_NOT_DESTROY,                                   // chn not be destroyed
    E_MI_LDC_ERR_PORT_NOT_UNBIND,                                   // port not unbind
} MI_LDC_ErrCode_e;

typedef MI_U32 MI_LDC_DEV;
typedef MI_U32 MI_LDC_CHN;

typedef enum
{
    E_LDC_OFF_MODE,         // slc2 only
    E_LDC_ON_MODE           // ldc + slc2
} MI_LDC_WorkMode_e;

typedef enum
{
    E_LDC_CMDQ_OFF_MODE,
    E_LDC_CMDQ_ON_MODE
} MI_LDC_CmdqWorkMode_e;

typedef struct MI_LDC_OutputPortAttr_s
{
    MI_U16 u16Width;                        // scl2 output width only LDC OFF
    MI_U16 u16Height;                       // scl2 output height only LDC OFF
    MI_SYS_PixelFormat_e  ePixelFmt;        // scl2 output fmt
    MI_SYS_CompressMode_e eCompressMode;
    MI_SYS_WindowRect_t stCropRect;
} MI_LDC_OutputPortAttr_t;

typedef struct MI_LDC_ChannelAttr_s
{
    MI_U16 u16MaxW;
    MI_U16 u16MaxH;
} MI_LDC_ChannelAttr_t;

#define MI_LDC_OK                      MI_DEF_ERR(E_MI_MODULE_ID_LDC, E_MI_ERR_LEVEL_INFO, MI_SUCCESS)
#define MI_ERR_LDC_ILLEGAL_PARAM       MI_DEF_ERR(E_MI_MODULE_ID_LDC, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_ILLEGAL_PARAM)
#define MI_ERR_LDC_NULL_PTR            MI_DEF_ERR(E_MI_MODULE_ID_LDC, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NULL_PTR)
#define MI_ERR_LDC_BUSY                MI_DEF_ERR(E_MI_MODULE_ID_LDC, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_BUSY)
#define MI_ERR_LDC_FAIL                MI_DEF_ERR(E_MI_MODULE_ID_LDC, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_FAILED)
#define MI_ERR_LDC_INVALID_DEVID       MI_DEF_ERR(E_MI_MODULE_ID_LDC, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_INVALID_DEVID)
#define MI_ERR_LDC_NOT_SUPPORT         MI_DEF_ERR(E_MI_MODULE_ID_LDC, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NOT_SUPPORT)
#define MI_ERR_LDC_MOD_INITED          MI_DEF_ERR(E_MI_MODULE_ID_LDC, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_INITED)
#define MI_ERR_LDC_MOD_NOT_INIT        MI_DEF_ERR(E_MI_MODULE_ID_LDC, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NOT_INIT)
#define MI_ERR_LDC_DEV_CREATED         MI_DEF_ERR(E_MI_MODULE_ID_LDC, E_MI_ERR_LEVEL_ERROR, E_MI_LDC_ERR_DEV_CREATED)
#define MI_ERR_LDC_DEV_NOT_CREATE      MI_DEF_ERR(E_MI_MODULE_ID_LDC, E_MI_ERR_LEVEL_ERROR, E_MI_LDC_ERR_DEV_NOT_CREATE)
#define MI_ERR_LDC_DEV_NOT_DESTROY     MI_DEF_ERR(E_MI_MODULE_ID_LDC, E_MI_ERR_LEVEL_ERROR, E_MI_LDC_ERR_DEV_NOT_DESTROY)
#define MI_ERR_LDC_CHN_CREATED         MI_DEF_ERR(E_MI_MODULE_ID_LDC, E_MI_ERR_LEVEL_ERROR, E_MI_LDC_ERR_CHN_CREATED)
#define MI_ERR_LDC_CHN_NOT_CREATE      MI_DEF_ERR(E_MI_MODULE_ID_LDC, E_MI_ERR_LEVEL_ERROR, E_MI_LDC_ERR_CHN_NOT_CREATE)
#define MI_ERR_LDC_CHN_NOT_STOP        MI_DEF_ERR(E_MI_MODULE_ID_LDC, E_MI_ERR_LEVEL_ERROR, E_MI_LDC_ERR_CHN_NOT_STOP)
#define MI_ERR_LDC_CHN_NOT_DESTROY     MI_DEF_ERR(E_MI_MODULE_ID_LDC, E_MI_ERR_LEVEL_ERROR, E_MI_LDC_ERR_CHN_NOT_DESTROY)
#define MI_ERR_LDC_PORT_NOT_DISABLE    MI_DEF_ERR(E_MI_MODULE_ID_LDC, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NOT_DISABLE)
#define MI_ERR_LDC_PORT_NOT_UNBIND     MI_DEF_ERR(E_MI_MODULE_ID_LDC, E_MI_ERR_LEVEL_ERROR, E_MI_LDC_ERR_PORT_NOT_UNBIND)


#ifdef __cplusplus
}
#endif

#endif///_MI_VPE_DATATYPE_H_

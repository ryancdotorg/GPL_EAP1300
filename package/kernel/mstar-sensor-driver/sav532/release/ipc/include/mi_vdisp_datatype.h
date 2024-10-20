/*
* mi_vdisp_datatype.h- Sigmastar
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
#ifndef _MI_VDISP_DATATYPE_H_
#define _MI_VDISP_DATATYPE_H_

#ifdef __cplusplus
extern "C" {
#endif

#define VDISP_MAX_DEVICE_NUM 4
#define VDISP_MAX_CHN_NUM 1
#define VDISP_MAX_INPUTPORT_NUM 16
#define VDISP_MAX_OVERLAYINPUTPORT_NUM 1
#define VDISP_OVERLAYINPUTPORTID VDISP_MAX_INPUTPORT_NUM
#define VDISP_MAX_OUTPUTPORT_NUM 1

typedef MI_S32 MI_VDISP_DEV;
typedef MI_S32 MI_VDISP_PORT;

typedef struct MI_VDISP_OutputPortAttr_s
{
    MI_U32 u32BgColor;          /* Background color of a output port, in YUV format. [23:16]:v, [15:8]:y, [7:0]:u*/
    MI_SYS_PixelFormat_e ePixelFormat; /* pixel format of a output port */
    MI_U64 u64pts; /* current PTS */
    MI_U32 u32FrmRate; /* the frame rate of output port */
    MI_U32 u32Width; /* the frame width of a output port */
    MI_U32 u32Height; /* the frame height of a output port */
} MI_VDISP_OutputPortAttr_t;

typedef struct MI_VDISP_InputPortAttr_s
{
    MI_U32 u32OutX; /* the output frame X position of this input port */
    MI_U32 u32OutY; /* the output frame Y position of this input port */
    MI_U32 u32OutWidth; /* the output frame width of this input port */
    MI_U32 u32OutHeight; /* the output frame height of this input port */
    MI_S32 s32IsFreeRun; /* is this port free run */
} MI_VDISP_InputPortAttr_t;

typedef enum
{
    E_MI_VDISP_ERR_DEV_OPENED=MI_VDISP_INITIAL_ERROR_CODE,
    E_MI_VDISP_ERR_DEV_NOT_OPEN,
    E_MI_VDISP_ERR_DEV_NOT_CLOSE,
    E_MI_VDISP_ERR_PORT_NOT_UNBIND,
}MI_VDISP_ErrCode_e;

#define MI_VDISP_ERR_FAIL (MI_DEF_ERR( E_MI_MODULE_ID_VDISP, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_FAILED))
#define MI_VDISP_ERR_INVALID_DEVID (MI_DEF_ERR( E_MI_MODULE_ID_VDISP, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_INVALID_DEVID))
#define MI_VDISP_ERR_ILLEGAL_PARAM (MI_DEF_ERR( E_MI_MODULE_ID_VDISP, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_ILLEGAL_PARAM))
#define MI_VDISP_ERR_NOT_SUPPORT (MI_DEF_ERR( E_MI_MODULE_ID_VDISP, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NOT_SUPPORT))
#define MI_VDISP_ERR_MOD_INITED (MI_DEF_ERR( E_MI_MODULE_ID_VDISP, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_INITED))
#define MI_VDISP_ERR_MOD_NOT_INIT (MI_DEF_ERR( E_MI_MODULE_ID_VDISP, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NOT_INIT))
#define MI_VDISP_ERR_DEV_OPENED (MI_DEF_ERR( E_MI_MODULE_ID_VDISP, E_MI_ERR_LEVEL_ERROR, E_MI_VDISP_ERR_DEV_OPENED))
#define MI_VDISP_ERR_DEV_NOT_OPEN (MI_DEF_ERR( E_MI_MODULE_ID_VDISP, E_MI_ERR_LEVEL_ERROR, E_MI_VDISP_ERR_DEV_NOT_OPEN))
#define MI_VDISP_ERR_DEV_NOT_STOP (MI_DEF_ERR( E_MI_MODULE_ID_VDISP, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_DEV_NOT_STOPED))
#define MI_VDISP_ERR_DEV_NOT_CLOSE (MI_DEF_ERR( E_MI_MODULE_ID_VDISP, E_MI_ERR_LEVEL_ERROR, E_MI_VDISP_ERR_DEV_NOT_CLOSE))
#define MI_VDISP_ERR_NOT_CONFIG (MI_DEF_ERR( E_MI_MODULE_ID_VDISP, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NOT_CONFIG))
#define MI_VDISP_ERR_PORT_NOT_DISABLE (MI_DEF_ERR( E_MI_MODULE_ID_VDISP, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NOT_DISABLE))
#define MI_VDISP_ERR_PORT_NOT_UNBIND (MI_DEF_ERR( E_MI_MODULE_ID_VDISP, E_MI_ERR_LEVEL_ERROR, E_MI_VDISP_ERR_PORT_NOT_UNBIND))

#ifdef __cplusplus
}
#endif

#endif///_MI_VDISP_H_

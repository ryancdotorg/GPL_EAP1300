/*
* mi_sd_datatype.h- Sigmastar
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
#ifndef _MI_SD_DATATYPE_H_
#define _MI_SD_DATATYPE_H_
#include "mi_sys_datatype.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MI_SD_MAX_CHANNEL_NUM (64)
#define MI_SD_REALTIMEMODE_MAXCHNL_NUM (1)
#define MI_SD_MAX_PORT_NUM (1)
#define MI_SD_MAX_WORKINGLIST_NODE (2)

#define MI_SD_CHANNEL_MAX_WIDTH  (2560)
#define MI_SD_CHANNEL_MAX_HEIGHT (1440)


#define MI_SD_OK (0)
#define MI_ERR_SD_INVALID_CHNID (0xA0078001) //The SD channel ID is invalid.
#define MI_ERR_SD_INVALID_PORTID (0xA0078002) //The SD outport ID is invalid.
#define MI_ERR_SD_ILLEGAL_PARAM (0xA0078003) //The SD parameter is invalid.
#define MI_ERR_SD_EXIST (0xA0078004) //A SD channel is created.
#define MI_ERR_SD_UNEXIST (0xA0078005) //No SD channel is created.
#define MI_ERR_SD_NULL_PTR (0xA0078006) //The pointer of the input parameter is null.
#define MI_ERR_SD_NOT_SUPPORT (0xA0078008) //The operation is not supported.
#define MI_ERR_SD_NOT_PERM (0xA0078009) //The operation is forbidden.
#define MI_ERR_SD_NOMEM (0xA007800C) //The memory fails to be allocated.
#define MI_ERR_SD_NOBUF (0xA007800D) //The buffer pool fails to be allocated.
#define MI_ERR_SD_BUF_EMPTY (0xA007800E) //The picture queue is empty.
#define MI_ERR_SD_NOTREADY (0xA0078010) //The SD is not initialized.
#define MI_ERR_SD_BUSY (0xA0078012) //The SD is busy.


typedef MI_U32 MI_SD_CHANNEL;
typedef MI_U32 MI_SD_PORT;

typedef struct MI_SD_ChannelAttr_s
{
    MI_U16 u16MaxW;
    MI_U16 u16MaxH;
    MI_SYS_WindowRect_t stCropRect;
    MI_BOOL bLdcOnoff;//LDC enable
}MI_SD_ChannelAttr_t;


/*Define attributes of SD port's work mode*/
typedef struct MI_SD_OuputPortAttr_s
{
    MI_U16 u16Width;                         // Width of target image
    MI_U16 u16Height;                        // Height of target image
    MI_SYS_PixelFormat_e  ePixelFormat;            // Pixel format of target image
    MI_SYS_CompressMode_e eCompressMode;   // Compression mode of the output
}MI_SD_OuputPortAttr_t;

#ifdef __cplusplus
}
#endif

#endif///_MI_SD_DATATYPE_H_

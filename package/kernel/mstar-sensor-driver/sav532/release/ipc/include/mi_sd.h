/*
* mi_sd.h- Sigmastar
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
#ifndef _MI_SD_H_
#define _MI_SD_H_

#include "mi_sd_datatype.h"

#define SD_MAJOR_VERSION 1
#define SD_SUB_VERSION 3
#define MACRO_TO_STR(macro) #macro
#define SD_VERSION_STR(major_version,sub_version) ({char *tmp = sub_version/100 ? \
                                    "mi_sd_version_" MACRO_TO_STR(major_version)"." MACRO_TO_STR(sub_version) : sub_version/10 ? \
                                    "mi_sd_version_" MACRO_TO_STR(major_version)".0" MACRO_TO_STR(sub_version) : \
                                    "mi_sd_version_" MACRO_TO_STR(major_version)".00" MACRO_TO_STR(sub_version);tmp;})
#define MI_SD_API_VERSION SD_VERSION_STR(SD_MAJOR_VERSION,SD_SUB_VERSION)

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
/// @brief create a SD channel.
/// @param[out] SDCh: SD channel ID.
/// @param[in] pstAttr: Attribute of SD channel.
/// @return MI_SUCCESS: succeed in creating a channel.
///             MI_ERR_SD_ILLEGAL_PARAM: invalid input patamter.
///             MI_ERR_SD_NULL_PTR: NULL poiter error.
///             MI_ERR_SD_NOT_SUPPORT: not support ptr.
//------------------------------------------------------------------------------
MI_S32 MI_SD_CreateChannel(MI_SD_CHANNEL SDCh, MI_SD_ChannelAttr_t *pstSDChAttr);

//------------------------------------------------------------------------------
/// @brief deatroy a SD channel.
/// @param[in] SDCh: SD channel ID.
/// @return MI_SUCCESS: succeed in deatroying a SD channel.
///             MI_ERR_SD_INVALID_CHNID: Invalid channel ID.
///             MI_ERR_SD_BUSY: SD channel is busy.
//------------------------------------------------------------------------------
MI_S32 MI_SD_DestroyChannel(MI_SD_CHANNEL SDCh);

//------------------------------------------------------------------------------
/// @brief set attribute of SD channel.
/// @param[in] SDCh: SD channel ID.
/// @param[in] pstAttr: Attribute of SD channel.
/// @return MI_SUCCESS: succeed in setting attribute of SD channel.
///             MI_ERR_SD_INVALID_CHNID: Invalid channel ID.
///             MI_ERR_SD_ILLEGAL_PARAM: Invalid input patamter.
///             MI_ERR_SD_NULL_PTR: NULL poiter error.
///             MI_ERR_SD_NOT_SUPPORT: not support.
///             MI_ERR_SD_BUSY:  SD channel is busy.
//------------------------------------------------------------------------------
MI_S32 MI_SD_SetChannelAttr(MI_SD_CHANNEL SDCh, MI_SD_ChannelAttr_t *pstSDChAttr);

//------------------------------------------------------------------------------
/// @brief get attribute of SD channel.
/// @param[in] SDCh: SD channel ID.
/// @param[in] pstAttr: Attribute of SD channel.
/// @return MI_SUCCESS: succeed in getting attribute of SD channel.
///             MI_ERR_SD_INVALID_CHNID: Invalid channel ID.
///             MI_ERR_SD_NULL_PTR: NULL poiter error.
///             MI_ERR_SD_BUSY: Fail SD channel is busy.
//------------------------------------------------------------------------------
MI_S32 MI_SD_GetChannelAttr(MI_SD_CHANNEL SDCh, MI_SD_ChannelAttr_t *pstSDChAttr);

//------------------------------------------------------------------------------
/// @brief start a SD channel.
/// @param[in] SDCh: SD channel ID.
/// @return MI_SUCCESS: succeed in starting a SD channel.
///             MI_ERR_SD_INVALID_CHNID: Invalid channel ID.
///             MI_ERR_SD_BUSY: Fail SD channel is busy.
//------------------------------------------------------------------------------
MI_S32 MI_SD_StartChannel(MI_SD_CHANNEL SDCh);

//------------------------------------------------------------------------------
/// @brief stop a SD channel.
/// @param[in] SDCh: SD channel ID.
/// @return MI_SUCCESS: succeed in stopping a SD channel.
///             MI_ERR_SD_INVALID_CHNID: Invalid channel ID.
///             MI_ERR_SD_BUSY:channel is busy.
//------------------------------------------------------------------------------
MI_S32 MI_SD_StopChannel(MI_SD_CHANNEL SDCh);

//------------------------------------------------------------------------------
/// @brief set attribute of SD channel output port.
/// @param[in] SDCh: SD channel ID.
/// @param[in] pstOutputPortAttr: SD channel output port attribute.
/// @return MI_SUCCESS: succeed in setting attribute of SD channel output port.
///             MI_ERR_SD_INVALID_CHNID: Invalid channel ID.
///             MI_ERR_SD_ILLEGAL_PARAM: Invalid input patamter.
///             MI_ERR_SD_NULL_PTR: NULL poiter error.
///             MI_ERR_SD_BUSY: channel is busy..
//------------------------------------------------------------------------------
MI_S32 MI_SD_SetOutputPortAttr(MI_SD_CHANNEL SDCh, MI_SD_OuputPortAttr_t *pstSDMode);

//------------------------------------------------------------------------------
/// @brief get attribute of SD channel output port.
/// @param[in] SDCh: SD channel ID.
/// @param[out] pstOutputPortAttr: SD channel output port attribute.
/// @return MI_SUCCESS: succeed in getting attribute of SD channel output port.
///             MI_ERR_SD_BUSY: channel is busy.
///             MI_ERR_SD_INVALID_CHNID: Invalid channel ID.
///             MI_ERR_SD_NULL_PTR: NULL poiter error.
//------------------------------------------------------------------------------
MI_S32 MI_SD_GetOutputPortAttr(MI_SD_CHANNEL SDCh, MI_SD_OuputPortAttr_t *pstSDMode);

#ifdef __cplusplus
}
#endif

#endif///_MI_SD_H_

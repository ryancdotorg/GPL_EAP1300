/*
* mi_divp.h- Sigmastar
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
#ifndef _MI_DIVP_H_
#define _MI_DIVP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "mi_divp_datatype.h"

#define DIVP_MAJOR_VERSION 1
#define DIVP_SUB_VERSION 3
#define MACRO_TO_STR(macro) #macro
#define DIVP_VERSION_STR(major_version,sub_version) ({char *tmp = sub_version/100 ? \
                                    "mi_divp_version_" MACRO_TO_STR(major_version)"." MACRO_TO_STR(sub_version) : sub_version/10 ? \
                                    "mi_divp_version_" MACRO_TO_STR(major_version)".0" MACRO_TO_STR(sub_version) : \
                                    "mi_divp_version_" MACRO_TO_STR(major_version)".00" MACRO_TO_STR(sub_version);tmp;})
#define MI_DIVP_API_VERSION DIVP_VERSION_STR(DIVP_MAJOR_VERSION,DIVP_SUB_VERSION)

//------------------------------------------------------------------------------
/// @brief create a DIVP channel.
/// @param[out] pDivpChn: DIVP channel ID.
/// @param[in] pstAttr: Attribute of DIVP channel.
/// @return MI_SUCCESS: succeed in creating a channel.
///             MI_DIVP_ERR_INVALID_PARAM: invalid input patamter.
///             MI_DIVP_ERR_NULL_PTR: NULL poiter error.
///             MI_DIVP_ERR_FAILED: Fail to create a channel.
///             MI_DIVP_ERR_NO_RESOUCE: there is no resource.
//------------------------------------------------------------------------------
MI_S32 MI_DIVP_CreateChn (MI_DIVP_CHN DivpChn, MI_DIVP_ChnAttr_t* pstAttr);

//------------------------------------------------------------------------------
/// @brief deatroy a DIVP channel.
/// @param[in] DivpChn: DIVP channel ID.
/// @return MI_SUCCESS: succeed in deatroying a DIVP channel.
///             MI_DIVP_ERR_INVALID_CHNID: Invalid channel ID.
///             MI_DIVP_ERR_FAILED: Fail to deatroy a DIVP channel.
///             MI_DIVP_ERR_CHN_BUSY:channel is busy.
//------------------------------------------------------------------------------
MI_S32 MI_DIVP_DestroyChn(MI_DIVP_CHN DivpChn);

//------------------------------------------------------------------------------
/// @brief set attribute of DIVP channel.
/// @param[in] DivpChn: DIVP channel ID.
/// @param[in] pstAttr: Attribute of DIVP channel.
/// @return MI_SUCCESS: succeed in setting attribute of DIVP channel.
///             MI_DIVP_ERR_INVALID_CHNID: Invalid channel ID.
///             MI_DIVP_ERR_INVALID_PARAM: Invalid input patamter.
///             MI_DIVP_ERR_NULL_PTR: NULL poiter error.
///             MI_DIVP_ERR_CHN_NOT_SUPPORT: not support.
///             MI_DIVP_ERR_FAILED: Fail to set attribute of DIVP channel.
//------------------------------------------------------------------------------
MI_S32 MI_DIVP_SetChnAttr(MI_DIVP_CHN DivpChn, MI_DIVP_ChnAttr_t* pstAttr);

//------------------------------------------------------------------------------
/// @brief get attribute of DIVP channel.
/// @param[in] DivpChn: DIVP channel ID.
/// @param[in] pstAttr: Attribute of DIVP channel.
/// @return MI_SUCCESS: succeed in getting attribute of DIVP channel.
///             MI_DIVP_ERR_INVALID_CHNID: Invalid channel ID.
///             MI_DIVP_ERR_NULL_PTR: NULL poiter error.
///             MI_DIVP_ERR_FAILED: Fail to get attribute of DIVP channel.
//------------------------------------------------------------------------------
MI_S32 MI_DIVP_GetChnAttr(MI_DIVP_CHN DivpChn, MI_DIVP_ChnAttr_t* pstAttr);

//------------------------------------------------------------------------------
/// @brief start a DIVP channel.
/// @param[in] DivpChn: DIVP channel ID.
/// @return MI_SUCCESS: succeed in starting a DIVP channel.
///             MI_DIVP_ERR_INVALID_CHNID: Invalid channel ID.
///             MI_DIVP_ERR_FAILED: Fail to start a DIVP channel.
//------------------------------------------------------------------------------
MI_S32 MI_DIVP_StartChn(MI_DIVP_CHN DivpChn);

//------------------------------------------------------------------------------
/// @brief stop a DIVP channel.
/// @param[in] DivpChn: DIVP channel ID.
/// @return MI_SUCCESS: succeed in stopping a DIVP channel.
///             MI_DIVP_ERR_INVALID_CHNID: Invalid channel ID.
///             MI_DIVP_ERR_FAILED: Fail to stop a DIVP channel.
///             MI_DIVP_ERR_CHN_BUSY:channel is busy.
//------------------------------------------------------------------------------
MI_S32 MI_DIVP_StopChn(MI_DIVP_CHN DivpChn);

//------------------------------------------------------------------------------
/// @brief set attribute of DIVP channel output port.
/// @param[in] DivpChn: DIVP channel ID.
/// @param[in] pstOutputPortAttr: DIVP channel output port attribute.
/// @return MI_SUCCESS: succeed in setting attribute of DIVP channel output port.
///             MI_DIVP_ERR_INVALID_CHNID: Invalid channel ID.
///             MI_DIVP_ERR_INVALID_PARAM: Invalid input patamter.
///             MI_DIVP_ERR_NULL_PTR: NULL poiter error.
///             MI_DIVP_ERR_FAILED: Fail to set attribute of DIVP channel output port.
//------------------------------------------------------------------------------
MI_S32 MI_DIVP_SetOutputPortAttr (MI_DIVP_CHN DivpChn, MI_DIVP_OutputPortAttr_t * pstOutputPortAttr);

//------------------------------------------------------------------------------
/// @brief get attribute of DIVP channel output port.
/// @param[in] DivpChn: DIVP channel ID.
/// @param[out] pstOutputPortAttr: DIVP channel output port attribute.
/// @return MI_SUCCESS: succeed in getting attribute of DIVP channel output port.
///             MI_DIVP_ERR_FAILED: Fail to get attribute of DIVP channel output port.
///             MI_DIVP_ERR_INVALID_CHNID: Invalid channel ID.
///             MI_DIVP_ERR_NULL_PTR: NULL poiter error.
//------------------------------------------------------------------------------
MI_S32 MI_DIVP_GetOutputPortAttr (MI_DIVP_CHN DivpChn, MI_DIVP_OutputPortAttr_t * pstOutputPortAttr);

//------------------------------------------------------------------------------
/// @brief refresh a DIVP channel.
/// @param[in] DivpChn: DIVP channel ID.
/// @return MI_SUCCESS: succeed in refreshing a DIVP channel.
///             MI_DIVP_ERR_INVALID_CHNID: Invalid channel ID.
///             MI_DIVP_ERR_FAILED: Fail to refresh a DIVP channel.
//------------------------------------------------------------------------------
MI_S32 MI_DIVP_RefreshChn(MI_DIVP_CHN DivpChn);

#ifdef __cplusplus
}
#endif

#endif

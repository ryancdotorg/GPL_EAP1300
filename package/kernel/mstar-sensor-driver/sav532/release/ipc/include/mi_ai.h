/*
* mi_ai.h- Sigmastar
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
#ifndef __MI_AI_H__
#define __MI_AI_H__

#include "mi_aio_datatype.h"
#include "mi_ai_datatype.h"


#define AI_MAJOR_VERSION 1
#define AI_SUB_VERSION 3
#define MACRO_TO_STR(macro) #macro
#define AI_VERSION_STR(major_version,sub_version) ({char *tmp = sub_version/100 ? \
                                    "mi_ai_version_" MACRO_TO_STR(major_version)"." MACRO_TO_STR(sub_version) : sub_version/10 ? \
                                    "mi_ai_version_" MACRO_TO_STR(major_version)".0" MACRO_TO_STR(sub_version) : \
                                    "mi_ai_version_" MACRO_TO_STR(major_version)".00" MACRO_TO_STR(sub_version);tmp;})
#define MI_AI_API_VERSION AI_VERSION_STR(AI_MAJOR_VERSION,AI_SUB_VERSION)

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
// Include files
//=============================================================================


//=============================================================================
// Extern definition
//=============================================================================

//=============================================================================
// Macro definition
//=============================================================================


//=============================================================================
// Data type definition
//=============================================================================


//=============================================================================
// Variable definition
//=============================================================================


//=============================================================================
// Global function definition
//=============================================================================
MI_S32 MI_AI_SetPubAttr(MI_AUDIO_DEV AiDevId, MI_AUDIO_Attr_t *pstAttr);
MI_S32 MI_AI_GetPubAttr(MI_AUDIO_DEV AiDevId, MI_AUDIO_Attr_t*pstAttr);
MI_S32 MI_AI_Enable(MI_AUDIO_DEV AiDevId);
MI_S32 MI_AI_Disable(MI_AUDIO_DEV AiDevId);
MI_S32 MI_AI_EnableChn(MI_AUDIO_DEV AiDevId, MI_AI_CHN AiChn);
MI_S32 MI_AI_DisableChn(MI_AUDIO_DEV AiDevId, MI_AI_CHN AiChn);
MI_S32 MI_AI_GetFrame(MI_AUDIO_DEV AiDevId, MI_AI_CHN AiChn, MI_AUDIO_Frame_t*pstFrm, MI_AUDIO_AecFrame_t *pstAecFrm , MI_S32 s32MilliSec);
MI_S32 MI_AI_ReleaseFrame(MI_AUDIO_DEV AiDevId, MI_AI_CHN AiChn, MI_AUDIO_Frame_t *pstFrm, MI_AUDIO_AecFrame_t *pstAecFrm);
MI_S32 MI_AI_SetChnParam(MI_AUDIO_DEV AiDevId, MI_AI_CHN AiChn, MI_AI_ChnParam_t *pstChnParam);
MI_S32 MI_AI_GetChnParam(MI_AUDIO_DEV AiDevId, MI_AI_CHN AiChn, MI_AI_ChnParam_t *pstChnParam);
MI_S32 MI_AI_EnableReSmp(MI_AUDIO_DEV AiDevId, MI_AI_CHN AiChn, MI_AUDIO_SampleRate_e eOutSampleRate);
MI_S32 MI_AI_DisableReSmp(MI_AUDIO_DEV AiDevId, MI_AI_CHN AiChn);
MI_S32 MI_AI_SetVqeAttr(MI_AUDIO_DEV AiDevId, MI_AI_CHN AiChn, MI_AUDIO_DEV AoDevId, MI_AO_CHN AoChn, MI_AI_VqeConfig_t *pstVqeConfig);
MI_S32 MI_AI_GetVqeAttr(MI_AUDIO_DEV AiDevId, MI_AI_CHN AiChn,  MI_AI_VqeConfig_t *pstVqeConfig);
MI_S32 MI_AI_EnableVqe(MI_AUDIO_DEV AiDevId, MI_AI_CHN AiChn);
MI_S32 MI_AI_DisableVqe(MI_AUDIO_DEV AiDevId, MI_AI_CHN AiChn);
MI_S32 MI_AI_ClrPubAttr(MI_AUDIO_DEV AiDevId);
MI_S32 MI_AI_SaveFile(MI_AUDIO_DEV AiDevId, MI_AI_CHN AiChn,MI_AUDIO_SaveFileInfo_t *pstSaveFileInfo);
MI_S32 MI_AI_SetVqeVolume(MI_AUDIO_DEV AiDevId, MI_AI_CHN AiChn, MI_S32 s32VolumeDb);
MI_S32 MI_AI_GetVqeVolume(MI_AUDIO_DEV AiDevId, MI_AI_CHN AiChn, MI_S32 *ps32VolumeDb);
MI_S32 MI_AI_SetAencAttr(MI_AUDIO_DEV AiDevId, MI_AI_CHN AiChn, MI_AI_AencConfig_t * pstAencConfig);
MI_S32 MI_AI_GetAencAttr(MI_AUDIO_DEV AiDevId, MI_AI_CHN AiChn, MI_AI_AencConfig_t * pstAencConfig);
MI_S32 MI_AI_EnableAenc(MI_AUDIO_DEV AiDevId, MI_AI_CHN AiChn);
MI_S32 MI_AI_DisableAenc(MI_AUDIO_DEV AiDevId, MI_AI_CHN AiChn);
MI_S32 MI_AI_SetAedAttr(MI_AUDIO_DEV AiDevId, MI_AI_CHN AiChn, MI_AI_AedConfig_t *pstAedConfig);
MI_S32 MI_AI_GetAedAttr(MI_AUDIO_DEV AiDevId, MI_AI_CHN AiChn, MI_AI_AedConfig_t *pstAedConfig);
MI_S32 MI_AI_EnableAed(MI_AUDIO_DEV AiDevId, MI_AI_CHN AiChn);
MI_S32 MI_AI_DisableAed(MI_AUDIO_DEV AiDevId, MI_AI_CHN AiChn);
MI_S32 MI_AI_GetAedResult(MI_AUDIO_DEV AiDevId, MI_AI_CHN AiChn, MI_AI_AedResult_t *pstAedResult);

#ifdef __cplusplus
}
#endif

#endif ///__MI_AI_H__

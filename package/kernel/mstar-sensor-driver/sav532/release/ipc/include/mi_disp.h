/*
* mi_disp.h- Sigmastar
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
#ifndef _MI_DISP_H_
#define _MI_DISP_H_

#include "mi_disp_datatype.h"

#define DISP_MAJOR_VERSION 1
#define DISP_SUB_VERSION 3
#define MACRO_TO_STR(macro) #macro
#define DISP_VERSION_STR(major_version,sub_version) ({char *tmp = sub_version/100 ? \
                                    "mi_disp_version_" MACRO_TO_STR(major_version)"." MACRO_TO_STR(sub_version) : sub_version/10 ? \
                                    "mi_disp_version_" MACRO_TO_STR(major_version)".0" MACRO_TO_STR(sub_version) : \
                                    "mi_disp_version_" MACRO_TO_STR(major_version)".00" MACRO_TO_STR(sub_version);tmp;})
#define MI_DISP_API_VERSION DISP_VERSION_STR(DISP_MAJOR_VERSION,DISP_SUB_VERSION)

#ifdef __cplusplus
extern "C" {
#endif

MI_S32 MI_DISP_Enable(MI_DISP_DEV DispDev);
MI_S32 MI_DISP_Disable(MI_DISP_DEV DispDev);
MI_S32 MI_DISP_SetPubAttr(MI_DISP_DEV DispDev, const MI_DISP_PubAttr_t *pstPubAttr);
MI_S32 MI_DISP_GetPubAttr(MI_DISP_DEV DispDev, MI_DISP_PubAttr_t *pstPubAttr);
MI_S32 MI_DISP_DeviceAttach(MI_DISP_DEV DispSrcDev, MI_DISP_DEV DispDstDev);
MI_S32 MI_DISP_DeviceDetach(MI_DISP_DEV DispSrcDev, MI_DISP_DEV DispDstDev);
MI_S32 MI_DISP_EnableVideoLayer(MI_DISP_LAYER DispLayer);
MI_S32 MI_DISP_DisableVideoLayer(MI_DISP_LAYER DispLayer);
MI_S32 MI_DISP_SetVideoLayerAttr(MI_DISP_LAYER DispLayer, const MI_DISP_VideoLayerAttr_t *pstLayerAttr);
MI_S32 MI_DISP_GetVideoLayerAttr(MI_DISP_LAYER DispLayer, MI_DISP_VideoLayerAttr_t *pstLayerAttr);
MI_S32 MI_DISP_BindVideoLayer(MI_DISP_LAYER DispLayer, MI_DISP_DEV DispDev);
MI_S32 MI_DISP_UnBindVideoLayer(MI_DISP_LAYER DispLayer, MI_DISP_DEV DispDev);
MI_S32 MI_DISP_SetPlayToleration(MI_DISP_LAYER DispLayer, MI_U32 u32Toleration);
MI_S32 MI_DISP_GetPlayToleration(MI_DISP_LAYER DispLayer, MI_U32 *pu32Toleration);
MI_S32 MI_DISP_GetScreenFrame(MI_DISP_LAYER DispLayer, MI_DISP_VideoFrame_t *pstVFrame);
MI_S32 MI_DISP_ReleaseScreenFrame(MI_DISP_LAYER DispLayer, MI_DISP_VideoFrame_t *pstVFrame);
MI_S32 MI_DISP_SetVideoLayerAttrBegin(MI_DISP_LAYER DispLayer);
MI_S32 MI_DISP_SetVideoLayerAttrEnd(MI_DISP_LAYER DispLayer);
MI_S32 MI_DISP_SetInputPortAttr(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort, const MI_DISP_InputPortAttr_t *pstInputPortAttr);
MI_S32 MI_DISP_GetInputPortAttr(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort, MI_DISP_InputPortAttr_t *pstInputPortAttr);
MI_S32 MI_DISP_EnableInputPort (MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort);
MI_S32 MI_DISP_DisableInputPort(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort);
MI_S32 MI_DISP_SetInputPortDispPos(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort, const MI_DISP_Position_t *pstDispPos);
MI_S32 MI_DISP_GetInputPortDispPos(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort, MI_DISP_Position_t *pstDispPos);
MI_S32 MI_DISP_PauseInputPort (MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort);
MI_S32 MI_DISP_ResumeInputPort (MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort);
MI_S32 MI_DISP_StepInputPort(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort);
MI_S32 MI_DISP_ShowInputPort(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort);
MI_S32 MI_DISP_HideInputPort(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort);
MI_S32 MI_DISP_SetInputPortSyncMode (MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort, MI_DISP_SyncMode_e eMode);
MI_S32 MI_DISP_QueryInputPortStat(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort, MI_DISP_QueryChannelStatus_t *pstStatus);
MI_S32 MI_DISP_SetZoomInWindow(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort, MI_DISP_VidWinRect_t* pstZoomRect);
MI_S32 MI_DISP_GetVgaParam(MI_DISP_DEV DispDev, MI_DISP_VgaParam_t *pstVgaParam);
MI_S32 MI_DISP_SetVgaParam(MI_DISP_DEV DispDev, MI_DISP_VgaParam_t *pstVgaParam);
MI_S32 MI_DISP_GetHdmiParam(MI_DISP_DEV DispDev, MI_DISP_HdmiParam_t *pstHdmiParam);
MI_S32 MI_DISP_SetHdmiParam(MI_DISP_DEV DispDev, MI_DISP_HdmiParam_t *pstHdmiParam);
MI_S32 MI_DISP_GetCvbsParam(MI_DISP_DEV DispDev, MI_DISP_CvbsParam_t *pstCvbsParam);
MI_S32 MI_DISP_SetCvbsParam(MI_DISP_DEV DispDev, MI_DISP_CvbsParam_t *pstCvbsParam);
MI_S32 MI_DISP_ClearInputPortBuffer(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort, MI_BOOL bClrAll);

#ifdef __cplusplus
}
#endif

#endif///_MI_DISP_H_

////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2012 SigmaStar Technology Corp.
// All rights reserved.
//
// Unless otherwise stipulated in writing, any and all information contained
// herein regardless in any format shall remain the sole proprietary of
// SigmaStar Technology Corp. and be kept in strict confidence
// (SigmaStar Confidential Information) by the recipient.
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of SigmaStar Confidential
// Information is unlawful and strictly prohibited. SigmaStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
////////////////////////////////////////////////////////////////////////////////

/*
*   mi_iqserver.h
*
*   Created on: July 10, 2018
*       Author: Shan Li
*/


#ifndef _MI_IQSERVER_H
#define _MI_IQSERVER_H

#ifdef __cplusplus
extern "C"
{
#endif
#include "mi_iqserver_pretzel_datatype.h"

/*
*   Open IQServer
*   Param:
*       width: sensor width
*       height: sensor height
*/
MI_S32 MI_IQSERVER_Open(MI_U16 width, MI_U16 height, MI_S32 vpeChn);

MI_S32 MI_IQSERVER_SetDataPath(char* path);

/*
*   Close IQServer
*/
MI_S32 MI_IQSERVER_Close();
#endif

#ifdef __cplusplus
}   //end of extern C
#endif

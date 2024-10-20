/*******************************************************************************************
 * Copyright (c) 2006-7 Laboratorio di Sistemi di Elaborazione e Bioingegneria Informatica *
 *                      Universita' Campus BioMedico - Italy                               *
 *                                                                                         *
 * This program is free software; you can redistribute it and/or modify it under the terms *
 * of the GNU General Public License as published by the Free Software Foundation; either  *
 * version 2 of the License, or (at your option) any later version.                        *
 *                                                                                         *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY         *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A 	       *
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.                *
 *                                                                                         *
 * You should have received a copy of the GNU General Public License along with this       *
 * program; if not, write to the:                                                          *
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,                    *
 * MA  02111-1307, USA.                                                                    *
 *                                                                                         *
 * --------------------------------------------------------------------------------------- *
 * Project:  Capwap                                                                        *
 *                                                                                         *
 * Author :  Ludovico Rossi (ludo@bluepixysw.com)                                          *
 *           Del Moro Andrea (andrea_delmoro@libero.it)                                    *
 *           Giovannini Federica (giovannini.federica@gmail.com)                           *
 *           Massimo Vellucci (m.vellucci@unicampus.it)                                    *
 *           Mauro Bisson (mauro.bis@gmail.com)                                            *
 *******************************************************************************************/

#include <CWWTP.h>

#define MAX_MESHID_SIZE 32
#define MAX_MASH_WPAKEY_SIZE 64
#define MAX_MESH_DEVICE_NAME_SIZE 32

CWBool CWWTPBoardSetRadioMeshEnableCfg(int radioIdx,  int enable);
CWBool CWWTPBoardGetRadioMeshEnableCfg(int radioIdx,int *enable);
CWBool CWWTPBoardGetRadioMeshIDCfg(int radioIdx,  char **pstr);
CWBool CWWTPBoardSetRadioMeshIDCfg(int radioIdx,  char *pstr);
CWBool CWWTPBoardGetRadioMeshWPAKeyCfg(int radioIdx,  char **pstr);
CWBool CWWTPBoardSetRadioMeshWPAKeyCfg(int radioIdx,  char *pstr);
CWBool CWWTPBoardSetRadioMeshLinkRobustThresholdCfg(int radioIdx,  short link_robust_threshold);
CWBool CWWTPBoardGetRadioMeshLinkRobustThresholdCfg(int radioIdx,short *link_robust_threshold);
CWBool CWWTPBoardSetMeshEnableTotalCfg(int enable);
CWBool CWWTPBoardGetMeshEnableTotalCfg(int *enable);
CWBool CWWTPBoardGetRadioMeshInfo(int radioIdx, CWRadioMeshInfo *meshInfo);
CWBool CWWTPBoardSetMeshModeCfg(int mode);
CWBool CWWTPBoardGetMeshModeCfg(int *mode);

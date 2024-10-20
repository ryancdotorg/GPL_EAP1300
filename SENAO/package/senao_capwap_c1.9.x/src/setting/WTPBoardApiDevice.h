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

#ifndef __CAPWAP_WTPBoardApiDevice_HEADER__
#define __CAPWAP_WTPBoardApiDevice_HEADER__

/*_____________________________________________________*/
/*  *******************___TYPES___*******************  */
#define MAX_MODELNAME_SIZE 32
#define MAX_SKU_SIZE    8
#define MAX_SERIALNUMBER_SIZE 32

CWBool CWWTPBoardGetModelName(char **pstr);
CWBool CWWTPBoardGetSku(char **pstr);
CWBool CWWTPBoardGetCapCode(unsigned int *code);
CWBool CWWTPBoardGetSerialNum(char **pstr);
CWBool CWWTPBoardGetLedPowerCfg(int *enable);
CWBool CWWTPBoardSetLedPowerCfg(int enable);
CWBool CWWTPBoardGetLedLanCfg(int *enable);
CWBool CWWTPBoardSetLedLanCfg(int enable);
CWBool CWWTPBoardGetLedWlan0Cfg(int *enable);
CWBool CWWTPBoardSetLedWlan0Cfg(int enable);
CWBool CWWTPBoardGetLedWlan1Cfg(int *enable);
CWBool CWWTPBoardSetLedWlan1Cfg(int enable);
CWBool CWWTPBoardGetLedWlan2Cfg(int *enable);
CWBool CWWTPBoardSetLedWlan2Cfg(int enable);
CWBool CWWTPBoardGetLedMeshCfg(int *enable);
CWBool CWWTPBoardSetLedMeshCfg(int enable);

#endif

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

#ifndef __CAPWAP_WTPBoardApiNas_HEADER__
#define __CAPWAP_WTPBoardApiNas_HEADER__

/*_____________________________________________________*/
/*  *******************___TYPES___*******************  */
#define MAX_NASID_SIZE 1024

CWBool CWWTPBoardSetWlanNasIdEnableCfg(int radioIdx, int wlanIdx, int enable);
CWBool CWWTPBoardGetWlanNasIdEnableCfg(int radioIdx, int wlanIdx, int *enable);
CWBool CWWTPBoardSetWlanNasPortEnableCfg(int radioIdx, int wlanIdx, int enable);
CWBool CWWTPBoardGetWlanNasPortEnableCfg(int radioIdx, int wlanIdx, int *enable);
CWBool CWWTPBoardGetWlanNasIdCfg(int radioIdx, int wlanIdx, char **pstr);
CWBool CWWTPBoardSetWlanNasIdCfg(int radioIdx, int wlanIdx, char *pstr);
CWBool CWWTPBoardGetWlanNasPortCfg(int radioIdx, int wlanIdx, unsigned short *port);
CWBool CWWTPBoardSetWlanNasPortCfg(int radioIdx, int wlanIdx, unsigned short port);
CWBool CWWTPBoardSetWlanNasIPEnableCfg(int radioIdx, int wlanIdx, int enable);
CWBool CWWTPBoardGetWlanNasIPEnableCfg(int radioIdx, int wlanIdx, int *enable);
CWBool CWWTPBoardGetWlanNasIPCfg(int radioIdx, int wlanIdx, unsigned int *addr);
CWBool CWWTPBoardSetWlanNasIPCfg(int radioIdx, int wlanIdx, unsigned int addr);
#endif

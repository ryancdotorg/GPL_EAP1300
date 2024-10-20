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

#ifndef __CAPWAP_WTPBoardApiCommon_HEADER__
#define __CAPWAP_WTPBoardApiCommon_HEADER__

/*_____________________________________________________*/
/*  *******************___TYPES___*******************  */
#define MAX_STRING_LENGTH_32    32
#if SUPPORT_WLAN_OUTDOOR_DISTANCE
#define IS_OUTDOOR_AP()         CWWTPBoardGetOutdoor()
#else
#define IS_OUTDOOR_AP()         (0)
#endif

typedef enum
{
    RLF_NETWORK = 1 << 0,
    RLF_WIRELESS = 1 << 1,
    RLF_VLANISOLATION = 1 << 2,
    RLF_LED = 1 << 3,
    RLF_LANPORT = 1 << 4,
    RLF_ECCAPTIVE = 1 << 5,
    RLF_FASTROAMING = 1 << 6,
    RLF_TRAFFICSHAPPING = 1 << 7,
    RLF_SYSTEM = 1 << 8,
    RLF_DHCP = 1 << 9,
    RLF_WIRELESS_SCHEDULE = 1 << 10,
    RLF_PORTAL = 1 << 11,
#if SUPPORT_MAC_FILTER_NO_RELOAD
    RLF_MACFILTER = 1 << 12
#endif
} ReloadFlag_t;

typedef enum
{
    DPF_TRAFFIC_SHAPING = 1 << 0,
    DPF_NAS = 1 << 1,
    DPF_MESH = 1 << 2
} DependentFlag_t;

ReloadFlag_t GetReloadFlag();
ReloadFlag_t GetDependentFlag();
char *CWCreateStringEntry(int str_size);
char *CWCreateStringByUci(const char *fmt, ...);
char *CWCreateUciString(const char *pSrcStr);
void CWWTPBoardGetWtpCfgCap(CWWtpCfgCap cfgCap);
void CWWTPBoardGetRadioCfgCap(int radioIdx, CWWtpCfgCap cfgCap);
void CWWTPBoardGetWlanCfgCap(int radioIdx, int wlanIdx, CWWtpCfgCap cfgCap);
#if SUPPORT_WLAN_5G_2_SETTING
void CWWTPBoardInitWtpCfgCap(CWWtpCfgCap cfgCap);
void CWWTPBoardInitRadioCfgCap(int radioIdx, CWWtpCfgCap cfgCap);
void CWWTPBoardInitWlanCfgCap(int radioIdx, int wlanIdx, CWWtpCfgCap cfgCap);
#endif
int CWWTPBoardGetApplyCfgTime(CWBool *rejoin);
CWBool CWWTPBoardInitConfiguration();
CWBool CWWTPBoardConfigurationCfg();
CWBool CWWTPBoardApplyCfg();
CWBool CWWTPBoardCancelCfg();


/*begin,20160503,andy,merge capwap v4*/
CWBool CWWTPBoardSetLanPortVlanModeCfg(int port, int mode);
CWBool CWWTPBoardGetLanPortVlanModeCfg(int port, int *mode);
/*end,20160503,andy,merge capwap v4*/

#endif

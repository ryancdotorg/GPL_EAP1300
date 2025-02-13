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
#include <gconfig.h>

#ifndef __CAPWAP_WTPBoardApiSystem_HEADER__
#define __CAPWAP_WTPBoardApiSystem_HEADER__

/*_______________________________________________________*/
/*  *******************___DEFINES___*******************  */

#ifndef FACTORY_REBOOT_TIME
#define FACTORY_REBOOT_TIME	180
#endif

#ifndef REBOOT_TIME
#define REBOOT_TIME	120
#endif

#ifndef IMAGE_REBOOT_TIME
#define IMAGE_REBOOT_TIME	180
#endif

/*_____________________________________________________*/
/*  *******************___TYPES___*******************  */
int CWWTPBoardGetRebootTime(CWBool factory);
int CWWTPBoardGetImageRebootTime(char *imagePath);
int CWWTPBoardGetImageBurningTime(char *imagePath);
int CWWTPBoardGetSystemUpTime();
int CWWTPBoardGetOutdoor();
CWBool CWWTPBoardGetHardwareVersion(char **pstr);
CWBool CWWTPBoardGetSoftwareVersion(char **pstr);
CWBool CWWTPBoardGetBootVersion(char **pstr);
CWBool CWWTPBoardGetNameCfg(char **pstr);
CWBool CWWTPBoardSetNameCfg(char *pstr);
CWBool CWWTPBoardGetLocationCfg(char **pstr);
CWBool CWWTPBoardSetLocationCfg(char *pstr);
CWBool CWWTPBoardGetDefaultInterfaceName(char **pstr);
CWBool CWWTPBoardBurnImage(char *imagePath);
CWBool CWWTPBoardReboot();
CWBool CWWTPBoardGetAdminCfg(char **pstr);
CWBool CWWTPBoardSetAdminCfg(char *pstr);
CWBool CWWTPBoardGetPasswordMD5Cfg(unsigned char **md5);
CWBool CWWTPBoardSetPasswordMD5Cfg(unsigned char *md5);
#if SUPPORT_WLAN_5G_2_SETTING
CWBool CWWTPBoardGetApCfgInitMode(CWBool *enable);
CWBool CWWTPBoardSetApCfgInitMode(CWBool enable);
#endif
CWBool CWWTPBoardSetAcMode(CWBool enable);
CWBool CWWTPBoardFactoryReset();
CWBool CWWTPBoardGetAcAddress(CWAcAddress *acAddr);
CWBool CWWTPBoardSetAcAddress(CWAcAddress *acAddr);
CWBool CWWTPBoardGetAcListCfg(int *count, CWHostName **hostName);
CWBool CWWTPBoardSetAcListCfg(int count, CWHostName *hostName);
CWBool CWWTPBoardGetForceAcAddress(CWHostName acAddr);
CWBool CWWTPBoardSetForceAcAddress(const CWHostName acAddr);
CWBool CWWTPBoardGetLogRemoteEnable( int *Enabled);
CWBool CWWTPBoardSetLogRemoteEnable( int Enabled);
CWBool CWWTPBoardGetLogRemoteCfg( char **pcfg);
CWBool CWWTPBoardSetLogRemoteCfg( char *pcfg);
CWBool CWWTPBoardSetUTCTime(int intime);
CWBool CWWTPBoardGetUTCTime(int *outtime);
CWBool CWWTPBoardGetTimeZone( int *timezone);
CWBool CWWTPBoardSetTimeZone( int timezone);
CWBool CWWTPBoardGetLogTrafficEnable(int *Enabled);
CWBool CWWTPBoardSetLogTrafficEnable(int Enabled);

CWBool CWWTPBoardGetComEnable(void);
#endif

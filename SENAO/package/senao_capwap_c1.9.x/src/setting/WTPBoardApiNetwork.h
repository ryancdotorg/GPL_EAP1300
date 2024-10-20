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

#ifndef __CAPWAP_WTPBoardApiNetwork_HEADER__
#define __CAPWAP_WTPBoardApiNetwork_HEADER__

/*_____________________________________________________*/
/*  *******************___TYPES___*******************  */

#define LAN_IF_NAME BRG_DEV

int CWWTPBoardGetMaxLanPortNum();
CWBool CWWTPBoardGetIPv4Cfg(CWIPv4Cfg *cfg);
CWBool CWWTPBoardSetIPv4Cfg(CWIPv4Cfg *cfg);
CWBool CWWTPBoardGetDns1Cfg(unsigned int *addr);
CWBool CWWTPBoardSetDns1Cfg(unsigned int addr);
CWBool CWWTPBoardGetDns2Cfg(unsigned int *addr);
CWBool CWWTPBoardSetDns2Cfg(unsigned int addr);
CWBool CWWTPBoardGetCurrentIPv4(CWIPv4Cfg *cfg);
CWBool CWWTPBoardGetCurrentDns(unsigned int *dns1, unsigned int *dns2);
CWBool CWWTPBoardGetLanPortEnableCfg(int port, int *enable);
CWBool CWWTPBoardSetLanPortEnableCfg(int port, int enable);
CWBool CWWTPBoardGetLanPortVlanIdCfg(int port, int *vid);
CWBool CWWTPBoardSetLanPortVlanIdCfg(int port, int vid);
CWBool CWWTPBoardGetLanPortEnableVLANCfg(int port, int *mode);
CWBool CWWTPBoardSetLanPortEnableVLANCfg(int port, int mode);
CWBool CWWTPBoardGetLanPortVlanModeCfg(int port, int *mode);
CWBool CWWTPBoardSetLanPortVlanModeCfg(int port, int mode);
CWBool CWWTPBoardSetManageVlanCfg(int vlan);
CWBool CWWTPBoardGetManageVlanCfg(int *vlan);
CWBool CWWTPBoardGetGuestNetworkAddressCfg(unsigned int *addr);
CWBool CWWTPBoardSetGuestNetworkAddressCfg(unsigned int addr);
CWBool CWWTPBoardGetGuestNetworkMaskCfg(unsigned int *addr);
CWBool CWWTPBoardSetGuestNetworkMaskCfg(unsigned int addr);
CWBool CWWTPBoardGetGuestNetworkDhcpStartCfg(unsigned int *addr);
CWBool CWWTPBoardSetGuestNetworkDhcpStartCfg(unsigned int addr);
CWBool CWWTPBoardGetGuestNetworkDhcpEndCfg(unsigned int *addr);
CWBool CWWTPBoardSetGuestNetworkDhcpEndCfg(unsigned int addr);
CWBool CWWTPBoardGetGuestNetworkWinsServerCfg(unsigned int *addr);
CWBool CWWTPBoardSetGuestNetworkWinsServerCfg(unsigned int addr);
CWBool CWWTPBoardGetAcAddressWithDhcpOption(char **acAddr);
#endif

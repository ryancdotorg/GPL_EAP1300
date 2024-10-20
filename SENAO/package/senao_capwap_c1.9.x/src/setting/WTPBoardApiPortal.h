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

#ifndef __CAPWAP_WTPBoardApiPortal_HEADER__
#define __CAPWAP_WTPBoardApiPortal_HEADER__

#define RADIUS_SECRET_LENGTH 64
#define SERVER_IPADDR_LENGTH 16
#define URL_LENGTH  1024
#define AUTH_LENGTH  1024

typedef enum {
	CW_Disable=0,
	CW_Enable,
        CW_Briege,
        CW_NAT
} CWPortalNetworkType;

/*_____________________________________________________*/
/*  *******************___TYPES___*******************  */
CWBool CWWTPBoardSetPortalEnableCfg(int radioIdx, int enable);
CWBool CWWTPBoardGetPortalEnableCfg(int radioIdx, int *enable);
CWBool CWWTPBoardSetPortalLoginTypeCfg(int radioIdx, int type);
CWBool CWWTPBoardGetPortalLoginTypeCfg(int radioIdx, int *type);
CWBool CWWTPBoardGetPortalRadiusCfg(int radioIdx, unsigned int *addr);
CWBool CWWTPBoardSetPortalRadiusCfg(int radioIdx, unsigned int addr);
CWBool CWWTPBoardGetPortalRedirectCfg(int radioIdx, char **pstr);
CWBool CWWTPBoardSetPortalRedirectCfg(int radioIdx, char *pstr);
CWBool CWWTPBoardSetPortalWalledGardenCfg(int radioIdx, int enable);
CWBool CWWTPBoardGetPortalWalledGardenCfg(int radioIdx, int *enable);
CWBool CWWTPBoardGetPortalWalledGardenPageCfg(int radioIdx, char **pstr);
CWBool CWWTPBoardSetPortalWalledGardenPageCfg(int radioIdx, char *pstr);
CWBool CWWTPBoardGetPortalRadiusPortCfg(int radioIdx, unsigned short *port);
CWBool CWWTPBoardSetPortalRadiusPortCfg(int radioIdx, unsigned short port);
CWBool CWWTPBoardGetPortalRadiusSecretCfg(int radioIdx, char **pstr);
CWBool CWWTPBoardSetPortalRadiusSecretCfg(int radioIdx, char *pstr);
CWBool CWWTPBoardSetPortalSessionTimeoutCfg(int radioIdx, int time);
CWBool CWWTPBoardGetPortalSessionTimeoutCfg(int radioIdx, int *time);
CWBool CWWTPBoardSetPortalSessionTimeoutEnableCfg(int radioIdx, int enable);
CWBool CWWTPBoardGetPortalSessionTimeoutEnableCfg(int radioIdx, int *enable);
CWBool CWWTPBoardSetPortalIdleTimeoutCfg(int radioIdx, int time);
CWBool CWWTPBoardGetPortalIdleTimeoutCfg(int radioIdx, int *time);
CWBool CWWTPBoardSetPortalIdleTimeoutEnableCfg(int radioIdx, int enable);
CWBool CWWTPBoardGetPortalIdleTimeoutEnableCfg(int radioIdx, int *enable);
CWBool CWWTPBoardSetPortalAccountingEnableCfg(int radioIdx, int enable);
CWBool CWWTPBoardGetPortalAccountingEnableCfg(int radioIdx, int *enable);
CWBool CWWTPBoardSetPortalAccountingIntervalCfg(int radioIdx, int time);
CWBool CWWTPBoardGetPortalAccountingIntervalCfg(int radioIdx, int *time);
CWBool CWWTPBoardSetPortalAuthTypeCfg(int radioIdx, int type);
CWBool CWWTPBoardGetPortalAuthTypeCfg(int radioIdx, int *type);
CWBool CWWTPBoardGetPortalExternalServerCfg(int radioIdx, char **pstr);
CWBool CWWTPBoardSetPortalExternalServerCfg(int radioIdx, char *pstr);
CWBool CWWTPBoardGetPortalExternalSecretCfg(int radioIdx, char **pstr);
CWBool CWWTPBoardSetPortalExternalSecretCfg(int radioIdx, char *pstr);
CWBool CWWTPBoardGetPortalAccountingServerCfg(int radioIdx, unsigned int *addr);
CWBool CWWTPBoardSetPortalAccountingServerCfg(int radioIdx, unsigned int addr);
CWBool CWWTPBoardGetPortalAccountingPortCfg(int radioIdx, unsigned short *port);
CWBool CWWTPBoardSetPortalAccountingPortCfg(int radioIdx, unsigned short port);
CWBool CWWTPBoardGetPortalAccountingSecretCfg(int radioIdx, char **pstr);
CWBool CWWTPBoardSetPortalAccountingSecretCfg(int radioIdx, char *pstr);
CWBool CWWTPBoardGetPortalUamformatCfg(int radioIdx, char **pstr);
CWBool CWWTPBoardSetPortalUamformatCfg(int radioIdx, char *pstr);
CWBool CWWTPBoardGetPortalLocalAuthCfg(int radioIdx, char **pstr);
CWBool CWWTPBoardSetPortalLocalAuthCfg(int radioIdx, char *pstr);
CWBool CWWTPBoardGetPortalPortCfg(int radioIdx, unsigned short *port);
CWBool CWWTPBoardSetPortalPortCfg(int radioIdx, unsigned short port);
CWBool CWWTPBoardSetPortalHttpsEnableCfg(int radioIdx, int enable);
CWBool CWWTPBoardGetPortalHttpsEnableCfg(int radioIdx, int *enable);
CWBool CWWTPBoardGetPortalRadiusSecret2Cfg(int radioIdx, char **pstr);
CWBool CWWTPBoardSetPortalRadiusSecret2Cfg(int radioIdx, char *pstr);
CWBool CWWTPBoardGetPortalRadius2Cfg(int radioIdx, unsigned int *addr);
CWBool CWWTPBoardSetPortalRadius2Cfg(int radioIdx, unsigned int addr);
/*  *******************___BASE ON SSID___*******************  */
CWBool CWWTPBoardGetWlanPortalEnableCfg(int radioIdx, int wlanIdx , int *enable);
CWBool CWWTPBoardSetWlanPortalEnableCfg(int radioIdx, int wlanIdx , int enable);
CWBool CWWTPBoardGetWlanPortalLoginTypeCfg(int radioIdx, int wlanIdx , int *type);
CWBool CWWTPBoardSetWlanPortalLoginTypeCfg(int radioIdx, int wlanIdx , int type);
CWBool CWWTPBoardGetWlanPortalRadiusCfg(int radioIdx, int wlanIdx , unsigned int *addr);
CWBool CWWTPBoardSetWlanPortalRadiusCfg(int radioIdx, int wlanIdx , unsigned int addr);
CWBool CWWTPBoardGetWlanPortalRedirectCfg(int radioIdx, int wlanIdx , char * * pstr);
CWBool CWWTPBoardSetWlanPortalRedirectCfg(int radioIdx, int wlanIdx , char *pstr);
CWBool CWWTPBoardGetWlanPortalWalledGardenCfg(int radioIdx, int wlanIdx , int *enable);
CWBool CWWTPBoardSetWlanPortalWalledGardenCfg(int radioIdx, int wlanIdx , int enable);
CWBool CWWTPBoardGetWlanPortalWalledGardenPageCfg(int radioIdx, int wlanIdx , char **pstr);
CWBool CWWTPBoardSetWlanPortalWalledGardenPageCfg(int radioIdx, int wlanIdx , char *pstr);
CWBool CWWTPBoardGetWlanPortalRadiusPortCfg(int radioIdx, int wlanIdx , unsigned short *port);
CWBool CWWTPBoardSetWlanPortalRadiusPortCfg(int radioIdx, int wlanIdx , unsigned short port);
CWBool CWWTPBoardGetWlanPortalRadiusSecretCfg(int radioIdx, int wlanIdx , char **pstr);
CWBool CWWTPBoardSetWlanPortalRadiusSecretCfg(int radioIdx, int wlanIdx , char *pstr);
CWBool CWWTPBoardSetWlanPortalSessionTimeoutCfg(int radioIdx, int wlanIdx , int time);
CWBool CWWTPBoardGetWlanPortalSessionTimeoutCfg(int radioIdx, int wlanIdx , int *time);
CWBool CWWTPBoardGetWlanPortalSessionTimeoutEnableCfg(int radioIdx, int wlanIdx , int *enable);
CWBool CWWTPBoardSetWlanPortalSessionTimeoutEnableCfg(int radioIdx, int wlanIdx , int enable);
CWBool CWWTPBoardSetWlanPortalIdleTimeoutCfg(int radioIdx, int wlanIdx , int time);
CWBool CWWTPBoardGetWlanPortalIdleTimeoutCfg(int radioIdx, int wlanIdx , int *time);
CWBool CWWTPBoardGetWlanPortalIdleTimeoutEnableCfg(int radioIdx, int wlanIdx , int *enable);
CWBool CWWTPBoardSetWlanPortalIdleTimeoutEnableCfg(int radioIdx, int wlanIdx , int enable);
CWBool CWWTPBoardGetWlanPortalAccountingEnableCfg(int radioIdx, int wlanIdx , int *enable);
CWBool CWWTPBoardSetWlanPortalAccountingEnableCfg(int radioIdx, int wlanIdx , int enable);
CWBool CWWTPBoardSetWlanPortalAccountingIntervalCfg(int radioIdx, int wlanIdx , int time);
CWBool CWWTPBoardGetWlanPortalAccountingIntervalCfg(int radioIdx, int wlanIdx , int *time);
CWBool CWWTPBoardGetWlanPortalAuthTypeCfg(int radioIdx, int wlanIdx , int *type);
CWBool CWWTPBoardSetWlanPortalAuthTypeCfg(int radioIdx, int wlanIdx , int type);
CWBool CWWTPBoardGetWlanPortalExternalServerCfg(int radioIdx, int wlanIdx , char **pstr);
CWBool CWWTPBoardSetWlanPortalExternalServerCfg(int radioIdx, int wlanIdx , char *pstr);
CWBool CWWTPBoardGetWlanPortalExternalSecretCfg(int radioIdx, int wlanIdx , char **pstr);
CWBool CWWTPBoardSetWlanPortalExternalSecretCfg(int radioIdx, int wlanIdx , char *pstr);
CWBool CWWTPBoardGetWlanPortalAccountingServerCfg(int radioIdx, int wlanIdx , unsigned int *addr);
CWBool CWWTPBoardSetWlanPortalAccountingServerCfg(int radioIdx, int wlanIdx , unsigned int addr);
CWBool CWWTPBoardGetWlanPortalAccountingPortCfg(int radioIdx, int wlanIdx , unsigned short *port);
CWBool CWWTPBoardSetWlanPortalAccountingPortCfg(int radioIdx, int wlanIdx , unsigned short port);
CWBool CWWTPBoardGetWlanPortalAccountingSecretCfg(int radioIdx, int wlanIdx , char **pstr);
CWBool CWWTPBoardSetWlanPortalAccountingSecretCfg(int radioIdx, int wlanIdx , char *pstr);
CWBool CWWTPBoardGetWlanPortalUamformatCfg(int radioIdx, int wlanIdx , char **pstr);
CWBool CWWTPBoardSetWlanPortalUamformatCfg(int radioIdx, int wlanIdx , char *pstr);
CWBool CWWTPBoardGetWlanPortalLocalAuthCfg(int radioIdx, int wlanIdx , char **pstr);
CWBool CWWTPBoardSetWlanPortalLocalAuthCfg(int radioIdx, int wlanIdx , char *pstr);
CWBool CWWTPBoardGetWlanPortalPortCfg(int radioIdx, int wlanIdx , unsigned short *port);
CWBool CWWTPBoardSetWlanPortalPortCfg(int radioIdx, int wlanIdx , unsigned short port);
CWBool CWWTPBoardGetWlanPortalHttpsEnableCfg(int radioIdx, int wlanIdx , int *enable);
CWBool CWWTPBoardSetWlanPortalHttpsEnableCfg(int radioIdx, int wlanIdx , int enable);
CWBool CWWTPBoardGetWlanPortalRadiusSecret2Cfg(int radioIdx, int wlanIdx , char **pstr);
CWBool CWWTPBoardSetWlanPortalRadiusSecret2Cfg(int radioIdx, int wlanIdx , char *pstr);
CWBool CWWTPBoardGetWlanPortalRadius2Cfg(int radioIdx, int wlanIdx , unsigned int *addr);
CWBool CWWTPBoardSetWlanPortalRadius2Cfg(int radioIdx, int wlanIdx , unsigned int addr);
CWBool CWWTPBoardGetWlanPortalNetworkTypeCfg(int radioIdx, int wlanIdx , int *type);
CWBool CWWTPBoardSetWlanPortalNetworkTypeCfg(int radioIdx, int wlanIdx , int type);
CWBool CWWTPBoardGetWlanGuestNetworkTypeCfg(int radioIdx, int wlanIdx , int *type);
CWBool CWWTPBoardSetWlanGuestNetworkTypeCfg(int radioIdx, int wlanIdx , int type);
CWBool CWWTPBoardSetWlanPortalNasIdCfg(int radioIdx, int wlanIdx , char *pstr);
CWBool CWWTPBoardSetWlanPortalNasPortCfg(int radioIdx, int wlanIdx , unsigned short port);
CWBool CWWTPBoardSetWlanPortalNasIPCfg(int radioIdx, int wlanIdx, unsigned int addr);
CWBool CWWTPBoardGetWlanPortalVlanTagCfg(int radioIdx, int wlanIdx , int *vlan);
CWBool CWWTPBoardSetWlanPortalVlanTagCfg(int radioIdx, int wlanIdx , int vlan);
#endif

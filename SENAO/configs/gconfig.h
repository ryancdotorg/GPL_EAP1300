/**
 *
 * Copyright (C) , SENAO INTERNATIONAL CO., LTD All rights reserved.
 * No part of this document may be reproduced in any form or by any 
 * means or used to make any derivative work (such as translation,
 * transformation, or adaptation) without permission from Senao
 * Corporation.
 */
#ifndef _GCONFIG_H
#define _GCONFIG_H
//************************************************************************************
//************************************************************************************
#include "autogconfig.h" 
#include "version_config.h"
#define IS_IPQ40XX 1

/*-------------------------------------------------------------------------*/
/*                        Partition Info                                   */
/*-------------------------------------------------------------------------*/
//#define COMBINED_APP_PARTITION_NAME         "firmware"    //ipq806 do not support. ipq806x use sysupgrade upgrade image(kernel + rootfs ) do not need to assign the partition name.

/*-------------------------------------------------------------------------*/
/*                          Product Info                                   */
/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/*                       Option Setting Info                               */
/*-------------------------------------------------------------------------*/
#define REBOOT_TIME 150
#define FACTORY_REBOOT_TIME 180
#define IMAGE_REBOOT_TIME 180
/*-------------------------------------------------------------------------*/
/*                       Interface  Info                                   */
/*-------------------------------------------------------------------------*/


/*
 * Below setting just for rmgmt.
 * char DEFAULT_24G_VAP records  all of 2.4G_VAP. rmgmt will destroy all of 2.4G_VAP except the first one VAP
 * char DEFAULT_5G_VAP  records  all of 5G_VAP.   rmgmt will destroy all of 5G_VAP   except the first one VAP
 * ex: #define DEFAULT_24G_VAP "ath24G-0,ath24G-1,ath24G-2"
*/

/*-------------------------------------------------------------------------*/
/*   set uboot config for setconfig  please refer to setconfig.h               */
/*-------------------------------------------------------------------------*/
#define CFG_ENV_SIZE         	0x10000  /* u-boot config size = 64KB */
#define ENV_HEADER_SIZE      	(sizeof(unsigned long))
#define ENV_SIZE             	(ENV_SIZE - ENV_HEADER_SIZE)
/* Because "void env_crc_update (void)" :
   modify "env_ptr->crc = crc32(0, env_ptr->data, ENV_SIZE)"
    -->   "env_ptr->crc = crc32(0, env_ptr->data, (0x1000 - 4))"
 */
#define CRC32_ENV_SIZE       	0x10000
#define DATA_BUFFER_SIZE     	0x200
#define EXTRA_SERIAL_LENTH 		20

/* please referrence to your CPU manual.*/
#define SYSTEM_IS_LITTLE_ENDIAN 1

/*-------------------------------------------------------------------------*/
/*                        Parameter Info                                   */
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/*                              SNMP OID                                   */
/*-------------------------------------------------------------------------*/
#define SNMP_PRIVATE_MIB_OID 1,3,6,1,4,1
#define ENGENIUS_MESH_MIB_OID  14125,1
#define ENGENIUS_PRIVATE_MIB_OID  14125,2
#define PRODUCT_MESH_MIB_OID  SNMP_PRIVATE_MIB_OID,ENGENIUS_MESH_MIB_OID
#define PRODUCT_PRIVATE_MIB_OID  SNMP_PRIVATE_MIB_OID,ENGENIUS_PRIVATE_MIB_OID

/*---------------------------------------------------------------------------
*    EAP1300 Firmware prefix  + suffix                                      *
*                                                                           *
*  loader                   :openwrt-ipq40xx-eap1300-u-boot.mbn             *
*  kernel                   :openwrt-ipq40xx-eap1300-kernel.bin             *
*  rootfs                   :openwrt-ipq40xx-eap1300-squashfs-root.img      *
*  loader(SBL~APPSBL)       :openwrt-ipq40xx-eap1300-nor-loader-s.img       *
*  Firmware(k+r)            :openwrt-ipq40xx-eap1300-nor-fw-s.img           *
*																			*
*  Note: *-s.img include script at the top of image.                        *
---------------------------------------------------------------------------*/

#define FW_PREFIX "openwrt-ipq40xx-"
/* common */
#define FW_KERNEL_SUFFIX 				"kernel.bin"
#define FW_ROOTFS_SUFFIX 				"ubi-root.img"
/* nor */
#define FW_FIT_NOR_UBOOT_SUFFIX 		"nor-loader-s.img"
#define FW_FIT_NOR_KR_SUFFIX 			"nor-fw-s.img"
#define FW_FIT_NOR_ALL_SUFFIX			"nor-all-s.img"
/* nand */
#define FW_FIT_NAND_UBOOT_SUFFIX     	"nand-loader-s.img"
#define FW_FIT_NAND_KR_SUFFIX        	"nand-fw-s.img"
#define FW_FIT_NAND_ALL_SUFFIX       	"nand-all-s.img"
/* nor + nand */
#define FW_FIT_NORNAND_UBOOT_SUFFIX     "norplusnand-loader-s.img"
#define FW_FIT_NORNAND_KR_SUFFIX        "norplusnand-fw-s.img"
#define FW_FIT_NORNAND_ALL_SUFFIX       "norplusnand-all-s.img"

#if IMG_IS_NOR
#define FW_UBOOT_SUFFIX FW_FIT_NOR_UBOOT_SUFFIX
#define FW_KERNEL_ROOTFS_SUFFIX FW_FIT_NOR_KR_SUFFIX
#elif IMG_IS_NAND
#define FW_UBOOT_SUFFIX FW_FIT_NAND_UBOOT_SUFFIX
#define FW_KERNEL_ROOTFS_SUFFIX FW_FIT_NAND_KR_SUFFIX
#else
#define FW_UBOOT_SUFFIX FW_FIT_NORNAND_UBOOT_SUFFIX
#define FW_KERNEL_ROOTFS_SUFFIX FW_FIT_NORNAND_KR_SUFFIX
#endif /* #if IMG_IS_NOR */

/*---------------------------------------------------------------------------
* for ubi volumes															*
* ex:																		*
* If UBI_VOLUMES "UBI_VOLUME_1,UBI_VOLUME_2,UBI_VOLUME_3"					*
* The number of volumes is 3												*
* If available size of nand is 122M 										*
* The size of UBI_VOLUME_1 is 122M/3=40M									*
---------------------------------------------------------------------------*/
#define IS_SUPPORT_UBI_VOLUMES "no"
#define DEFAULT_UBI_VOLUMES "UBI_VOLUME_1"



/*---------------------------------------------------------------------------
* SENAO uboot version 														*
---------------------------------------------------------------------------*/
#define CONFIG_SENAO_UBOOT_VERSION 1.0.0


/*---------------------------------------------------------------------------
* IP				 														*
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
* SYSTEM FILE				 														*
---------------------------------------------------------------------------*/
#define DHCP_LEASES "/tmp/dhcp.leases"

/*-------------------------------------------------------------------------*/
/*                        Factory config                                    */
/*-------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
* Allow setconfig to modify uboot_env of uboot_ver							*
---------------------------------------------------------------------------*/
#define ALLOW_MODFIY_UBOOT_VER

//************************************************************************************
//************************************************************************************
#endif  //#define _GCONFIG_H

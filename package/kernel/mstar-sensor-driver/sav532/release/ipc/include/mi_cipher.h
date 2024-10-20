/*
* mi_cipher.h- Sigmastar
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
#ifndef _MI_CIPHER_H_
#define _MI_CIPHER_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "mi_common.h"
#include "mi_cipher_datatype.h"

#define CIPHER_MAJOR_VERSION 1
#define CIPHER_SUB_VERSION 3
#define MACRO_TO_STR(macro) #macro
#define CIPHER_VERSION_STR(major_version,sub_version) ({char *tmp = sub_version/100 ? \
                                    "mi_cipher_version_" MACRO_TO_STR(major_version)"." MACRO_TO_STR(sub_version) : sub_version/10 ? \
                                    "mi_cipher_version_" MACRO_TO_STR(major_version)".0" MACRO_TO_STR(sub_version) : \
                                    "mi_cipher_version_" MACRO_TO_STR(major_version)".00" MACRO_TO_STR(sub_version);tmp;})
#define MI_CIPHER_API_VERSION CIPHER_VERSION_STR(CIPHER_MAJOR_VERSION,CIPHER_SUB_VERSION)

MI_S32 MI_CIPHER_Init(void);
MI_S32 MI_CIPHER_Uninit (void);
MI_S32 MI_CIPHER_CreateHandle(MI_HANDLE *phandle);
MI_S32 MI_CIPHER_DestroyHandle(MI_HANDLE handle);
MI_S32 MI_CIPHER_ConfigHandle(MI_HANDLE handle, MI_CIPHER_Config_t *pconfig);
MI_S32 MI_CIPHER_Encrypt(MI_HANDLE handle, void* srcAddr, void* dstAddr , MI_U32 u32ByteLen);
MI_S32 MI_CIPHER_Decrypt(MI_HANDLE handle, void* srcAddr, void* dstAddr, MI_U32 u32ByteLen);
MI_S32 MI_CIPHER_HashInit(MI_CIPHER_HASH_ALGO_e eHashAlgoType, MI_HANDLE *pHashHandle);
MI_S32 MI_CIPHER_HashUnInit(MI_HANDLE hHashHandle);
MI_S32 MI_CIPHER_HashUpdate(MI_HANDLE hHashHandle , MI_U8 *pu8InputData, MI_U32 u32IDataLen);
MI_S32 MI_CIPHER_HashFinal(MI_HANDLE hHashHandle, MI_U8 *pu8OutputHash, MI_U32 *pu32OutputHashLen);
MI_S32 MI_CIPHER_RsaPublicEnCrypt(MI_CIPHER_RSA_PUB_ENC_t *pstRsa, MI_U8 *pu8Input, MI_U32 u32InLen, MI_U8 *pu8Output, MI_U32 *pu32OutLen);
MI_S32 MI_CIPHER_RsaPublicDeCrypt(MI_CIPHER_RSA_PUB_ENC_t *pstRsa, MI_U8 *pu8Input, MI_U32 u32InLen, MI_U8 *pu8Output, MI_U32 *pu32OutLen);
MI_S32 MI_CIPHER_RsaPrivateEnCrypt(MI_CIPHER_RSA_PRI_ENC_t *pstRsa, MI_U8 *pu8Input, MI_U32 u32InLen, MI_U8 *pu8Output, MI_U32 *pu32OutLen);
MI_S32 MI_CIPHER_RsaPrivateEnCrypt(MI_CIPHER_RSA_PRI_ENC_t *pstRsa, MI_U8 *pu8Input, MI_U32 u32InLen, MI_U8 *pu8Output, MI_U32 *pu32OutLen);
MI_S32 MI_CIPHER_RsaSign(MI_CIPHER_RSA_SIGN_t *pstRsaSign, MI_U8 *pu8InHashData, MI_U32 u32HashDataLen, MI_U8 *pu8OutSign, MI_U32 *pu32OutSignLen);
MI_S32 MI_CIPHER_RsaVerify(MI_CIPHER_RSA_VERIFY_t *pstRsaVerify, MI_U8 *pu8InHashData, MI_U32 u32HashDataLen, MI_U8 *pu8InSign, MI_U32 u32InSignLen);

#ifdef __cplusplus
}
#endif

#endif ///_MI_CIPHER_H_

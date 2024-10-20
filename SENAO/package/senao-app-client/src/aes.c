/****************************************************************************
;
;   (C) Unpublished Work of Senao Networks, Inc.  All Rights Reserved.
;
;       THIS WORK IS AN UNPUBLISHED WORK AND CONTAINS CONFIDENTIAL,
;       PROPRIETARY AND TRADESECRET INFORMATION OF SENAO INCORPORATED.
;       ACCESS TO THIS WORK IS RESTRICTED TO (I) SENAO EMPLOYEES WHO HAVE A
;       NEED TO KNOW TO PERFORM TASKS WITHIN THE SCOPE OF THEIR ASSIGNMENTS
;       AND (II) ENTITIES OTHER THAN SENAO WHO HAVE ENTERED INTO APPROPRIATE
;       LICENSE AGREEMENTS.  NO PART OF THIS WORK MAY BE USED, PRACTICED,
;       PERFORMED, COPIED, DISTRIBUTED, REVISED, MODIFIED, TRANSLATED,
;       ABBRIDGED, CONDENSED, EXPANDED, COLLECTED, COMPILED, LINKED, RECAST,
;       TRANSFORMED OR ADAPTED WITHOUT THE PRIOR WRITTEN CONSENT OF SENAO.
;       ANY USE OR EXPLOITATION OF THIS WORK WITHOUT AUTHORIZATION COULD
;       SUBJECT THE PERPERTRATOR TO CRIMINAL AND CIVIL LIABILITY.
;
;----------------------------------------------------------------------------
;
;    Project : app_client
;    Creator :
;    File    : aes.c
;    Abstract:
;
;       Modification History:
;       By              Date        Ver.    Modification Description
;       --------------- --------    -----   -------------------------------------
;
;****************************************************************************/
/*
 * AES cipher functions.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/md5.h>

#include "aes.h"
#include "gconfig.h"

static int dbglevel = 1; // 0: fatal, 1: error, 2: warning, 3: info, 4: debug, 5: dump
#define PRINTF(level,fmt,args...) do{if(dbglevel>=level)printf("%s: "fmt"\n",__FUNCTION__,##args);}while(0)

#define DUMPHEX(buf,len) do { \
    if(dbglevel >= 5) {\
        int i; \
        printf("%s: dump hex data (%d)\n",__FUNCTION__,len);\
        for(i = 0; i < len; i++) { \
            unsigned char c = buf[i];\
            printf("%02x ",c); \
            if((i+1)%16==0) printf("\n");\
        }\
        if(i%16!=0)printf("\n");\
    }\
}while(0)

#define AES_BLOCK_SIZE 256
#define MAX_PADDING_LEN 16

#ifdef APP_AGENT_AES_KEY_BASIC_DATA
#define AES_KEY_BASIC_DATA APP_AGENT_AES_KEY_BASIC_DATA
#else
#warning "!!!APP_AGENT_AES_KEY_BASIC_DATA undefined, please check your gconfig.mk!!!"
#define AES_KEY_BASIC_DATA "!$~DEfAult&*+apP#&@seRvIcE-@"
#endif

typedef enum {
    DECRYPT_BUF,
    ENCRYPT_BUF
} EPC_ENCRYPT_TYPE;


static unsigned char *aes_crypt(const unsigned char *inputtext, int *len, int do_encrypt)
{
    unsigned char *key_data = (unsigned char *)AES_KEY_BASIC_DATA;
    int key_data_len = strlen(AES_KEY_BASIC_DATA);
    int i, nrounds = 5;
    unsigned char key[32], iv[32];
    EVP_CIPHER_CTX *ctx;
    int c_len, f_len = 0;
    unsigned char *outputtext;
    unsigned char salt[] = {1,2,3,4,5,6,7,8};

    /**
     * Create a 256 bit key and IV using the supplied key_data. salt can be added for taste.
     * Fills in the encryption and decryption ctx objects and returns 0 on success
     **/
    /*
     * Gen key & IV for AES 256 CBC mode. A SHA1 digest is used to hash the supplied key material.
     * nrounds is the number of times the we hash the material. More rounds are more secure but
     * slower.
     */
    i = EVP_BytesToKey(EVP_aes_256_cbc(), EVP_sha1(), salt, key_data, key_data_len, nrounds, key, iv);
    if (i != 32) {
        printf("Key size is %d bits - should be 256 bits\n", i);
        return NULL;
    }

    ctx = EVP_CIPHER_CTX_new();
    EVP_CipherInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv, do_encrypt);

    if(do_encrypt)
    {
        /* max ciphertext len for a n bytes of inputtext is n + AES_BLOCK_SIZE -1 bytes */
        c_len = *len + EVP_MAX_BLOCK_LENGTH;
    }
    else
    {
        /* plaintext will always be equal to or lesser than length of ciphertext*/
        c_len = *len;
    }

    outputtext = malloc(c_len);

    /* update outputtext, c_len is filled with the length of outputtext generated,
     *len is the size of inputtext in bytes */
    if(!EVP_CipherUpdate(ctx, outputtext, &c_len, inputtext, *len))
    {
        EVP_CIPHER_CTX_free(ctx);
        return NULL;
    }

    /* update outputtext with the final remaining bytes */
    EVP_CipherFinal_ex(ctx, outputtext + c_len, &f_len);

    *len = c_len + f_len;

    EVP_CIPHER_CTX_free(ctx);

    return outputtext;
}


char* encrypt_data(const char *buf, int len, int* outlen)
{
	char* output;
	int data_len = len;

	if(len <= 0)
		return NULL;

	output = (char *)aes_crypt((const unsigned char *)buf, &data_len, ENCRYPT_BUF);
	*outlen = data_len;
	if(output == NULL || *outlen <= 0)
	{
		if(output != NULL)
			free(output);
		return NULL;
	}
	DUMPHEX(output,*outlen);
	return output;
}

char* decrypt_data(const char *buf, int len, int *outlen)
{
	char* output;
	int data_len = len;

	if(len == 0)
		return NULL;

	DUMPHEX(buf,len);
	output = (char *)aes_crypt((const unsigned char *)buf, &data_len, DECRYPT_BUF);
	*outlen = data_len;
	if(output == NULL || output[0] == '\0' || *outlen <= 0)
	{
		if(output != NULL)
			free(output);
		return NULL;
	}
	return output;
}

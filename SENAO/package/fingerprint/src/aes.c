#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <openssl/evp.h>
#include <openssl/aes.h>
#include "aesapi.h"


/*
 * Encrypt *len bytes of data
 * All data going in & out is considered binary (unsigned char[])
 */
unsigned char *aes_crypt(unsigned char *inputtext, int *len, unsigned char *salt, int do_encrypt)
{
    unsigned char *key_data = (unsigned char *)FINGER_AES_KEY;
    int key_data_len = strlen(FINGER_AES_KEY);
    int i, nrounds = 5;
    unsigned char key[32], iv[32];
    EVP_CIPHER_CTX *ctx;
    int c_len, f_len = 0;
    unsigned char *outputtext;

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


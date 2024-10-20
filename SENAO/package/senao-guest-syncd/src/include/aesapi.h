#ifndef AES_API_H
#define AES_API_H
#include <openssl/evp.h>
#include <openssl/aes.h>


#define GUESTSYNC_AES_KEY "senAoGUesTSync12321"

unsigned char *aes_crypt(unsigned char *inputtext, int *len, unsigned char *salt, int do_encrypt);

#endif //AES_API_H

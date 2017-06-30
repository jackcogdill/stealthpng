// AES encryption/decryption program using OpenSSL EVP apis
// This is public domain code.
// Saju Pillai (saju.pillai@gmail.com)
// Modified by Jack Cogdill (2014)

#ifndef AES_UTIL_H
#define AES_UTIL_H

#include "Error.h"

#define AES_BLOCK_SIZE 256
#include <openssl/evp.h>

// "opaque" encryption, decryption ctx structures that libcrypto uses to record
// status of enc/dec operations
EVP_CIPHER_CTX en, de;

int aes_init(unsigned char *key_data, int key_data_len, unsigned char *salt, EVP_CIPHER_CTX *e_ctx, 
	EVP_CIPHER_CTX *d_ctx);
unsigned char *aes_encrypt(EVP_CIPHER_CTX *e, unsigned char *plaintext, int *len);
unsigned char *aes_decrypt(EVP_CIPHER_CTX *e, unsigned char *ciphertext, int *len);
void aes_clean();

#endif

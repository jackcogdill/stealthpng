#include "AES_util.h"

// Create a 256 bit key and IV using the supplied key_data. salt can be added for taste.
// Fills in the encryption and decryption ctx objects and returns 0 on success
int aes_init(unsigned char *key_data, int key_data_len, unsigned char *salt, EVP_CIPHER_CTX *e_ctx, 
	EVP_CIPHER_CTX *d_ctx) {
	int nrounds = 0x10000; // hash 2^16 times
	unsigned char key[32], iv[32];

	// Gen key & IV for AES 256 CBC mode. A SHA512 digest is used to hash the supplied key material.
	// nrounds is the number of times the we hash the material. More rounds are more secure but
	// slower.
	int i = EVP_BytesToKey(EVP_aes_256_cbc(), EVP_sha512(), salt, key_data, key_data_len, nrounds, key, iv);
	if (i != 32)
		Error("Key size for encryption is %d bits - should be 256 bits\n", i);

	EVP_CIPHER_CTX_init(e_ctx);
	EVP_EncryptInit_ex(e_ctx, EVP_aes_256_cbc(), NULL, key, iv);
	EVP_CIPHER_CTX_init(d_ctx);
	EVP_DecryptInit_ex(d_ctx, EVP_aes_256_cbc(), NULL, key, iv);

	return 0;
}

// Encrypt *len bytes of data
// All data going in & out is considered binary (unsigned char[])
unsigned char *aes_encrypt(EVP_CIPHER_CTX *e, unsigned char *plaintext, int *len) {
	// Max ciphertext len for a n bytes of plaintext is n + AES_BLOCK_SIZE -1 bytes
	int c_len = *len + AES_BLOCK_SIZE, f_len = 0;
	unsigned char *ciphertext = malloc(c_len);

	// Allows reusing of 'e' for multiple encryption cycles
	EVP_EncryptInit_ex(e, NULL, NULL, NULL, NULL);

	// update ciphertext, c_len is filled with the length of ciphertext generated,
	// *len is the size of plaintext in bytes
	EVP_EncryptUpdate(e, ciphertext, &c_len, plaintext, *len);

	// update ciphertext with the final remaining bytes
	EVP_EncryptFinal_ex(e, ciphertext+c_len, &f_len);

	*len = c_len + f_len;
	return ciphertext;
}

// Decrypt *len bytes of ciphertext
unsigned char *aes_decrypt(EVP_CIPHER_CTX *e, unsigned char *ciphertext, int *len) {
	// because we have padding ON, we must allocate an extra cipher block size of memory
	int p_len = *len, f_len = 0;
	unsigned char *plaintext = malloc(p_len + AES_BLOCK_SIZE);
	
	EVP_DecryptInit_ex(e, NULL, NULL, NULL, NULL);
	EVP_DecryptUpdate(e, plaintext, &p_len, ciphertext, *len);
	EVP_DecryptFinal_ex(e, plaintext+p_len, &f_len);

	*len = p_len + f_len;
	return plaintext;
}

void aes_clean() {
	EVP_CIPHER_CTX_cleanup(&en);
	EVP_CIPHER_CTX_cleanup(&de);
}

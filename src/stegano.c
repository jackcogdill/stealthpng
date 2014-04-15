// Steganography program made by Yentup
// Takes a password and can hide plain text or files inside png images (as long as the data fits).

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <libgen.h>

#include "Error.h"
#include "util.h"
#include "png_util.h"
#include "AES_util.h"

// For reference when modifying pixel data:
//
// void process_file(void) {
// 	for (int y = 0; y < height; y++) {
// 		png_byte* row = row_pointers[y];
// 		for (int x = 0; x < width; x++) {
// 			png_byte* ptr = &(row[x*3]);

// 			ptr[0] = ptr[2];
// 			ptr[1] = 0;
// 		}
// 	}
// }

void encode(char *msg, char *img) {
	unsigned char *data, *enc_data, *final_data;
	char *filename = 0;
	int len, plen, final_len;

	FILE *fp = fopen(msg, "rb");
	// Not saving the last null byte (in legal C strings) in either case
	// because all the chars are going into the image
	if (!fp) {
		len = strlen(msg);
		data = malloc(len);
		memcpy(data, msg, len);
	}
	else {
		filename = basename(msg);
		int fplen = strlen(filename)+1;
		char fprefix[fplen];
		sprintf(fprefix, "%s:", filename);

		// Read the file to data
		fseek(fp, 0, SEEK_END);
		int file_len = ftell(fp);
		len = fplen + file_len;
		rewind(fp);

		data = malloc(len);

		// all chars are 1 byte
		int read_size = fread(data, 1, file_len, fp);
		if (file_len != read_size)
			Error("Error reading file");

		// Prepend filename with colons (filenames can't have colons)
		// Filename gets encrypted too
		memmove(data+fplen, data, file_len);
		memcpy(data, fprefix, fplen);
	}
	fclose(fp);

	// Encrypt the data
	enc_data = aes_encrypt(&en, data, &len);
	// len gets updated here ^ to the new len of the encrypted data
	free(data); // Don't need this anymore

	plen = digits(len)+1;
	char prefix[plen];
	sprintf(prefix, filename ? "%d>" : "%d<", len);
	// The different symbols are to differentiate between file data and plaintext

	final_len = plen + len;
	final_data = malloc(final_len);
	memcpy(final_data, enc_data, len);
	free(enc_data); // All we need now is the final data

	// Prepend the encrypted data with the prefix of how long
	// it is so we know where to stop when decoding
	memmove(final_data+plen, final_data, len);
	memcpy(final_data, prefix, plen);


	// -----------------------------------------
	// inject the encrypted data into image here
	// -----------------------------------------

	// TESTING
	FILE *fh = fopen("TEST", "wb");
	for (int i = 0; i < final_len; i++) fputc(final_data[i], fh);
	// ===========

	free(final_data); // Done
}

void decode(char *img) {
	// ------------------------------------
	// encrypted data taken from image here
	// ------------------------------------

	// TESTING
	unsigned char *data, *dec_data;

	int size;
	FILE *fp = fopen("TEST", "rb");
	if (!fp)
		Error("Error opening file for reading");
	else {
		// Read the file to data
		fseek(fp, 0, SEEK_END);
		size = ftell(fp);
		rewind(fp);

		data = malloc(size);

		int read_size = fread(data, 1, size, fp);
		if (size != read_size)
			Error("Error reading file");
	}

	int len = 0, file = 0;
	for (int i = 0; i < size; i++) {
		if (data[i] == '<')
			break;
		else if (data[i] == '>') {
			file = 1;
			break;
		}
		else {
			len *= 10;
			len += data[i] - '0'; // Ascii to int (this works for digits)
		}
	}

	printf("%d\n%s\n", len, file ? "File" : "Plaintext");

	int plen = digits(len) +1; // Prefix length

	printf("%d\n", len);
	dec_data = aes_decrypt(&de, data+plen, &len);
	printf("%d\n", len);

	if (!file) {
		for (int i = 0; i < len; i++) putchar(dec_data[i]);
		puts("");
	}
	else {
		int i = 0;
		for (; i < len; i++) {
			if (dec_data[i] == ':')
				break;
		}
		if (!i || i >= len-1)
			Error("Wrong password or corrupt data.");

		printf("%d\n", i);
		char *filename = malloc(i);
		memcpy(filename, dec_data, i);
		puts(filename);

		// output to file
		FILE *fh = fopen(filename, "wb");
		// +1 for the colon
		for (int j = i+1; j < len; j++) fputc(dec_data[j], fh);
	}
	// ===========
}

int main(int argc, char **argv) {
	// Argument parsing
	char usage[256];
	char *pgrm = argv[0];
	sprintf(usage, "\
Usage: %s -e [message/file] [image]\n\
       %s -d [image]\n\
       %s -s [image(s)]\
", pgrm, pgrm, pgrm);

	if (argc == 1)
		Error(usage);

	char *options = "\n\
Options:\n\
  -h  Show this help message and exit\n\
  -e  Encode data into image\n\
  -d  Decode data from image\n\
  -s  Space available in image(s) (the higher the\n\
      resolution, the more space available)\
";
	char *e_arg = 0,
	     *d_arg = 0,
	     *s_arg = 0;
	int c;
	opterr = 0;

	while ((c = getopt (argc, argv, "hd:e:s:")) != -1) {
		switch(c) {
			case 'h':
				puts(usage);
				puts(options);
				return 0;
				break;
			case 'e':
				e_arg = optarg;
				break;
			case 'd':
				d_arg = optarg;
				break;
			case 's':
				s_arg = optarg;
				break;
			case '?':
				break;
			default:
				Error(usage);
		}
	}

	char (*last_args)[256] = malloc(256 * sizeof(*last_args));
	int index = s_arg ? 1 : 0;
	if (optind < argc) {
		while (optind < argc)
			sprintf(last_args[index++], "%s", argv[optind++]);
	}
	if (s_arg)
		sprintf(last_args[0], "%s", s_arg);

	if (e_arg || d_arg) {
		if (e_arg && !index)
			Error(usage);

		char pass[256];
		// Prompt for password
		sprintf(pass, "%s", getpass("Password for encoding:"));
		if (strcmp(pass, "") == 0)
			Error("No password entered.");

		// Prepare encryption and decryption:
		// The salt paramter is used as a salt in the derivation:
		// it should point to an 8 byte buffer or NULL if no salt is used.
		unsigned char salt[] = {1, 2, 3, 4, 5, 6, 7, 8};

		unsigned char *key_data = (unsigned char *) pass;
		int key_data_len = strlen(pass);

		// Gen key and iv. init the cipher ctx object
		if (aes_init(key_data, key_data_len, salt, &en, &de))
			Error("Error initializing AES cipher");
	}
	// Encode
	if (e_arg)
		encode(e_arg, last_args[0]);

	// Decode
	else if (d_arg)
		decode(d_arg);

	// Print space in image(s)
	else if (s_arg) {
		for (int i = 0; i < index; i++)
			puts(last_args[i]);
	}
	else
		Error(usage);
	if (opterr)
		return 1;

	// Cleanup
	free(last_args);
	aes_clean();
	png_clean();

	return 0;
}

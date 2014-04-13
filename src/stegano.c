// Steganography program made by Yentup
// Takes a password and can hide plain text or files inside png images (as long as the data fits).

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "Error.h"
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
	char *data;
	int len;

	FILE *fp = fopen(msg, "rb");
	if (!fp) {
		data = msg;
		len = strlen(data);
	}
	else {
		// Read the file to data
		fseek(fp, 0, SEEK_END);
		int size = len = ftell(fp);
		rewind(fp);

		data = malloc(size * sizeof(char));
		int read_size = fread(data, sizeof(char), size, fp);

		if (size != read_size)
			Error("Error reading file");
	}
	fclose(fp);

	unsigned char *enc_data;
	enc_data = aes_encrypt(&en, (unsigned char *)data, &len);

	// inject encrypted data into image here
}

void decode(char *img) {
	// unsigned char *data taken from image here

	// int len = strlen(data);

	// char *dec_data;
	// dec_data = (char *) aes_decrypt(&de, data, &len);
}

int main(int argc, char **argv) {
	// Argument parsing
	char usage[256];
	char *pgrm = argv[0];
	sprintf(usage, "Usage: %s -e [message/file] [image]\n\
       %s -d [image]\n\
       %s -s [image(s)]", pgrm, pgrm, pgrm);

	if (argc == 1)
		Error(usage);

	char *options = "\nOptions:\n  -h  Show this help message and exit\n\
  -e  Encode data into image\n  -d  Decode data from image\n  -s  \
Space available in image(s) (the higher the\n\
      resolution, the more space available)";
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

	char pass[256];
	if (e_arg || d_arg) {
		if (e_arg && !index)
			Error(usage);

		// Prompt for password
		sprintf(pass, "%s", getpass("Password for encoding:"));
		if (strcmp(pass, "") == 0)
			Error("No password entered.");

		// The salt paramter is used as a salt in the derivation:
		// it should point to an 8 byte buffer or NULL if no salt is used.
		unsigned char salt[] = {1, 2, 3, 4, 5, 6, 7, 8};

		unsigned char *key_data = (unsigned char *)pass;
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

	free(last_args);
	aes_clean();
	png_clean();

	return 0;
}

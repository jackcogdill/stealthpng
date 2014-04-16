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
	int padding = 3 - (final_len % 3); // Data needs to be divisible by 3 before being put
		// into the image
	int padded_len = final_len + padding;
	final_data = malloc(padded_len);
	memcpy(final_data, enc_data, len);
	free(enc_data); // All we need now is the final data

	// Prepend the encrypted data with the prefix of how long
	// it is so we know where to stop when decoding
	memmove(final_data+plen, final_data, len);
	memcpy(final_data, prefix, plen);
	// Last, add the padding of null bytes to make it divisible by 3
	// Note: this will get removed in the end because
	// we keep the size not including the padding. We need to have it divisibile by 3 because
	// if we have 2 bits left of data, we need to store it in 1 whole pixel, which holds
	// 6 possible bits of information
	for (int i = final_len; i < padded_len; i++)
		final_data[i] = '\0';

	// Checking if possible
	read_png_file(img);
	int space = (width * height) * (3.0f / 4.0f);
	if (padded_len > space) // Make sure there is enough space in the image for the data
		Error("Not enough space in image.\nData size: %s\nSpace available: %s",
			byteconvert(padded_len), byteconvert(space));


	// // Main loop for injection of data
	// for (int i = 0, x = 0, y = 0; i < padded_len; i += 3) {
	// 	// Loop for each color component, adding 2 bits at a time
	// 	for (int j = 0; j < 3; j++) {
	// 		// Loop for each of 4 pixels
	// 		for (int p = 0, x_ = x, y_ = y; p < 4; p++) {
	// 			png_byte* ptr = &(row_pointers[y_][x_*3]);

	// 			// Replace last 2 bits on color component with zeros
	// 			ptr[j] >>= 2;
	// 			ptr[j] <<= 2;

	// 			int sh = p * 2;
	// 			// Add 2 bits of data at a time onto different color components
	// 			ptr[j] |= ((int)final_data[i + j] & (3 << sh)) >> sh;

	// 			x_++;
	// 			if (x_ == width) {
	// 				x_ -= width;
	// 				y_++;
	// 			}
	// 		}
	// 	}

	// 	// Continue to next set of pixels
	// 	x += 4;
	// 	if (x == width) {
	// 		x -= width;
	// 		y++;
	// 	}
	// }
	// write_png_file("new-image.png"); // Save the image with the data inside it


	free(final_data); // Done
}

void decode(char *img) {
	// unsigned char *data, *dec_data;

	// read_png_file(img);

	// char *prefix = malloc(16);
	// int len = 0, file = 0, pindex = 0;
	// int found_size = 0;
	// for (int i = 0, x = 0, y = 0;; i++) {
	// 	int temp[3] = {0, 0, 0};
	// 	// Loop for each color component
	// 	for (int j = 0; j < 3; j++) {
	// 		// Loop for each pixel
	// 		int x_ = x + 4, y_ = y;
	// 		if (x_ == width) {
	// 			x_ -= width;
	// 			y++;
	// 		}
	// 		for (int p = 0; p < 4; p++) {
	// 			png_byte* ptr = &(row_pointers[y_][x_*3]);

	// 			temp[j] <<= 2;
	// 			// Putting data back from the 2 least significant bits
	// 			temp[j] |= ptr[j] & 3;

	// 			x_--;
	// 			if (x_ == 0) {
	// 				x_ = width -1;
	// 				y_--;
	// 			}
	// 		}
	// 	}
	// 	// Continue to next set of pixels
	// 	x += 4;
	// 	if (x == width) {
	// 		x -= width;
	// 		y++;
	// 	}


	// 	for (int k = 0; k < 3; k++) {
	// 		prefix[pindex++] = temp[k];
	// 	}
	// 	for (int k = 0; k < pindex; k++) {
	// 		if (prefix[i] == '<') {
	// 			found_size = 1;
	// 			break;
	// 		}
	// 		else if (prefix[i] == '>') {
	// 			found_size = file = 1;
	// 			break;
	// 		}
	// 		else {
	// 			len *= 10;
	// 			len += prefix[i] - '0'; // Ascii to int (this works for digits)
	// 			// needs this here: Error("Hidden data either nonexistent or corrupted");
	// 		}
	// 	}
	// 	if (found_size) {
	// 		printf("%d\n", len);
	// 		return;
	// 	}
	// 	else if (!found_size && pindex >= 16)
	// 		Error("Hidden data either nonexistent or corrupted");
	// }


	// int plen = digits(len) +1; // Prefix length

	// // Decrypt the data
	// dec_data = aes_decrypt(&de, data+plen, &len);

	// // Output the plaintext
	// if (!file) {
	// 	puts("DATA:\n-----");
	// 	for (int i = 0; i < len; i++) putchar(dec_data[i]);
	// 	puts("");
	// }
	// else {
	// 	int i = 0;
	// 	// Filename cannot exceed 256
	// 	for (; i < len && i <= 256; i++) {
	// 		if (dec_data[i] == ':')
	// 			break;
	// 	}
	// 	if (!i || i >= 256 || i >= len-1)
	// 		Error("Wrong password or corrupt data.");

	// 	printf("%d\n", i);
	// 	char *filename = malloc(i);
	// 	memcpy(filename, dec_data, i);

	// 	// output to file
	// 	FILE *fh = fopen(filename, "wb");
	// 	// +1 for the colon
	// 	for (int j = i+1; j < len; j++) fputc(dec_data[j], fh);
	// }
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
		puts("Space available in image(s):");
		for (int i = 0; i < index; i++) {
			read_png_file(last_args[i]);
			int space = (width * height) * (3.0f / 4.0f);
			space -= 512 +12; // 256 for filename and 256 for AES encryption
			printf("%s: %s\n", basename(last_args[i]), byteconvert(space));
		}
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

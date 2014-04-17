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
		int fplen = strlen(filename)+1; // 1 for the colon
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


	// Loop through all the data, 3 bytes at a time,
	// putting every 3 bytes into groups of 4 pixels
	// (8 bits for 3 bytes is 24 total bits, 6 bits for each of
	// 4 pixels (matching of total 24))
	int pixel_loc = 0;
	int x, y;
	png_byte* pixel;
	for (int i = 0; i < padded_len; i += 3, pixel_loc += 4) {
		// next group of pixels every loop, so add 4 each time

		// Loop for each color component and for each byte of data
		// (3 of each)
		// Each byte has 8 bits, 2 of each go into each of the 4 pixels
		for (int j = 0; j < 3; j++, pixel_loc -= 4) {
			// reset to continue looping through the same pixels every time by doing -= 4

			// Loop for each of 4 pixels to put the data into
			for (int p = 0; p < 4; p++, pixel_loc++) {
				x = pixel_loc % width, y = pixel_loc / width;
				pixel = &(row_pointers[y][x*3]);

				// j for color component
				// Replace last 2 bits with zeros
				pixel[j] &= ~3;

				int sh = p * 2;
				// Add 2 bits of data at a time
				pixel[j] |= ((int)final_data[i + j] & (3 << sh)) >> sh;
				// & 3 gets only the last 2 bits of the data (1 byte) because
				// 3 is 00000011 in binary,
				// 3 << 2 is 00001100, 3 << 4 is 00110000, etc
			}
		}
	}
	write_png_file("new-image.png"); // Save the image with the data inside it


	free(final_data); // Done with this
}

void decode(char *img) {
	unsigned char *data, *dec_data;
	unsigned char prefix[16];
	int pindex = 0, dindex = 0;

	read_png_file(img);

	int found_prefix = 0, file = 0;
	int len = 0; // the length of the data to find
	// Main loop through pixels
	int pixel_loc = 3; // start with 4th one (0, 1, 2, 3)
	int x, y;
	png_byte* pixel;
	for (;; pixel_loc += 4) {
		int temp_data[3] = {0, 0, 0}; // 3 bytes of data at a time (stored as ints,
			// later cast to char)

		// Loop for each color component (and byte of data)
		for (int j = 0; j < 3; j++, pixel_loc += 4) {
			// reset to continue looping through the same pixels every time by doing += 4
			// going through them backwards this time to regain the data

			for (int p = 0; p < 4; p++, pixel_loc--) {
				x = pixel_loc % width, y = pixel_loc / width;
				pixel = &(row_pointers[y][x*3]);

				// slowly expand and add room for 2 bits of data every time
				temp_data[j] <<= 2;
				temp_data[j] |= pixel[j] & 3; // adding on 2 bits
					// & 3 is to only get the last 2 bits on the end
					// (After all, the data is only stored in the least 2 significant
					// bits of each color component of each pixel)
			}
		}

		if (found_prefix) {
			// Stop before adding too much data
			// We can't go over the len, this removes any padded null bytes
			// just to fit into the image evenly
			if (len - (dindex + 3) <= 0) {
				for (int k = 0; dindex < len; k++)
					data[dindex++] = (unsigned char)temp_data[k];
				break; // Found all the data; stop looping
			}
			else {
				for (int k = 0; k < 3; k++)
					data[dindex++] = (unsigned char)temp_data[k];
			}
		}
		else {
			// We're looking for the prefix here
			// First, add the data found so far into the prefix var
			for (int k = 0; k < 3; k++)
				prefix[pindex++] = (unsigned char)temp_data[k];

			// Now check to see if we've found the whole prefix yet
			// We should find it within 16 chars, so 6 iterations (3 chars added each time)
			int l;
			for (l = 0; l < pindex; l++) {
				if (prefix[l] == '<') {
					found_prefix = 1;
					break;
				}
				else if (prefix[l] == '>') {
					found_prefix = file = 1;
					break;
				}
				else {
					// The prefix starts with the length
					// Look for the length, if we can't find it then there's no data here
					len *= 10;
					int digit = prefix[l] - '0'; // Ascii to int (this works for digits)
					if (digit < 0 || digit > 9)
						Error("Hidden data either nonexistent or corrupted");
					len += digit;
				}
			}

			// If we found it, transfer any trailing data from the prefix to the main data
			// Either way, allocate memory to the main data var now
			if (found_prefix) {
				data = malloc(len);

				// Transfer remaining data if there is any
				if (l < pindex -1) {
					// Skip one, the ending of the prefix
					for (l = l + 1; l < pindex; l++)
						data[dindex++] = prefix[l];
				}
			}
			else {
				len = 0; // reset the len if we haven't found the whole number of it yet
				if (pindex >= 16)
					Error("Hidden data either nonexistent or corrupted");
					// Prefix is never this long, just quit at this point
			}
		}
	}

	// Decrypt the data
	dec_data = aes_decrypt(&de, data, &len);
	free(data); // Done with this

	// Output decrypted data
	if (file) {
		int found = 0, i;
		for (i = 0; i <= 256; i++) {
			if (dec_data[i] == ':') {
				found = 1;
				break;
			}
		}
		if (!found)
			Error("Could not find filename from encrypted data");
		char *filename = malloc(i); // i is the lenght of the filename
		memcpy(filename, dec_data, i);

		FILE *fp = fopen(filename, "wb");
		if (!fp)
			Error("File %s could not be opened for writing", filename);

		printf("Saving data to %s\n", filename);
		fwrite(dec_data+i+1, 1, len-i-1, fp); // 1 for the colon
		fclose(fp);
	}
	else
		printf("%s\n", dec_data);

	free(dec_data); // Done
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

		// Prompt for password
		char *pass = getpass("Password for encoding:");
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

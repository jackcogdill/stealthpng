#include "util.h"

char *byteconvert(unsigned long b_) {
	float b = (float) b_;
	int n = 0;
	while (b / 1000 >= 1) {
		b /= 1000;
		n++;
	}
	char *buffer = malloc(32 * sizeof(char));
	sprintf(buffer, "%.2f%c", b, "BKMGT"[n]);
	return buffer;
}

#include "util.h"

// Returns human friendly byte amount
char *byteconvert(unsigned long b_) {
	float b = (float) b_;
	int n = 0;
	while (b / 1000 >= 1) {
		b /= 1000;
		n++;
	}
	char *buffer = malloc(32);
	sprintf(buffer, "%.2f%c", b, "BKMGT"[n]);
	return buffer;
}

// Returns the digits in an int
int digits(int x) {
	return (int)log10((double)x) +1;
}

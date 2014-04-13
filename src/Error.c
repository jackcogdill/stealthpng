#include "Error.h"

void Error(const char *s, ...) {
	va_list args;
	va_start(args, s);
	vfprintf(stderr, s, args);
	fprintf(stderr, "\n");
	va_end(args);
	exit(1);
}

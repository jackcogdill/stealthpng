// Copyright 2002-2010 Guillaume Cottenceau.
// This software may be freely redistributed under the terms
// of the X11 license.
// Modified by Jack Cogdill (2014)

#ifndef PNG_UTIL_H
#define PNG_UTIL_H

#include <stdlib.h>
#include "Error.h"

#define PNG_DEBUG 3
#include <png.h>

// Global vars for image
int width, height;
png_byte color_type;
png_byte bit_depth;
png_bytep *row_pointers;

void read_png_file(char *file_name);
void write_png_file(char *file_name);
void png_clean();

#endif

#ifndef _PIXELSORT_IMAGE_
#define _PIXELSORT_IMAGE_

#include "pixel_stream_context.h"

void pixelsort_image(const char* input, const char* output, void (*mask_ptr)(char*, pixel_stream_context*, int*, int, int, int), void (*rotate_image)(char*, int, int, int), float (*hbs)(char, char, char));

#endif

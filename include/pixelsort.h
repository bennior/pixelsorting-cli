#ifndef _PIXELSORT_
#define _PIXELSORT_

#include "pixel_stream_context.h"

void pixelsort_image(const char* input, const char* output, void (*mask_ptr)(char*, pixel_stream_context*, int*, int, int, int), void (*rotate_image)(char*, int, int, int), float (*hbs)(char, char, char));
void pixelsort_video(const char* input, const char* output, void (*mask_ptr)(char*, pixel_stream_context*, int*, int, int, int), void (*rotate_image)(char*, int, int, int), float (*hbs)(char, char, char));
void pixelsort_video2(const char* input, const char* output, void (*mask_ptr)(char*, pixel_stream_context*, int*, int, int, int), void (*rotate_image)(char*, int, int, int), float (*hbs)(char, char, char));

#endif

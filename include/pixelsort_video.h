#ifndef _PIXELSORT_VIDEO_
#define _PIXELSORT_VIDEO_

#include "pixel_stream_context.h"

void pixelsort_video(const char* input, const char* output, void (*mask_ptr)(char*, pixel_stream_context*, int*, int, int, int), void (*rotate_image)(char*, int, int, int), float (*hbs)(char, char, char));

#endif

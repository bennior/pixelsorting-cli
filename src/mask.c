#include "mask.h"
#include "hbs.h"
#include "pixel_stream_context.h"

void create_mask(char* image, pixel_stream_context* px_str_ctx, int* size, int rows, int columns, int desired_channels) {
 
  float lower_threshold = 0.0;
  float upper_threshold = 0.2;

  int start = 0;
  int prev_start = 0;
  int length = 0;
  int temp = 0;

  for(int i = 0; i < rows * columns; i++) {
      if(get_brightness(image[i * desired_channels], image[i * desired_channels + 1], image[i * desired_channels + 2]) <= upper_threshold && get_brightness(image[i * desired_channels], image[i * desired_channels + 1], image[i * desired_channels + 2]) >= lower_threshold) {

	if(temp != 0) {
	  temp = i;
	}

       } else {

	 if(start == 0) {
	   start = i;
	   temp = start;
	 }
	 length++;
}

      if(temp != start || (i % rows) == 0) {
        if(length > 10) {
// 	  px_str.emplace_back(pixel_stream{start - prev_start, length});
	  insert_element(px_str_ctx, (pixel_stream){start - prev_start, length});
	  prev_start = start;
        }
	length = 0;
        start = 0;
	temp = 0;
      }
  }
  *size = px_str_ctx->used;
}


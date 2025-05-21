#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

#include "hbs.h"
#include "pixelsort_image.h"
#include "quicksort.h"
#include "image_format.h"
#include "pixel_stream_context.h"

void pixelsort_image(const char* input, const char* output, void (*mask)(char*, pixel_stream_context*, int*, int, int, int), void (*rotate_image)(char*, int, int, int), float (*hbs)(char, char, char)) {
  
  if(strcmp(input, "") == 0) {
    printf("Error processing user's input filename\n");
    exit(1);
  }

  if(strcmp(output, "") == 0) {
    printf("Error processing user's input filename\n");
    exit(1);
  }

  int desired_comp = 3;
  int width = 0; int height = 0; int comp = 0;
  
  char* image = (char*) stbi_load(input, &width, &height, &comp, desired_comp);
  if(image == NULL) {
    printf("Error in loading the image\n");
    exit(1);
  }

  const char* format = get_image_format(input);

  if(strcmp(format, "") == 0) {
    printf("Image format is not supported. Supported format are 'png', 'pgm', 'ppm', 'jpg', 'jpeg' and 'bmp'\n");
    exit(1);
  }

  if(strcmp(format, "pgm") == 0 && hbs != get_brightness) {
    printf("Option not available for image format 'pgm'. Use '--sort-by-brightness' or '-B' instead\n");
    exit(1);
  }

  int columns = height;
  int rows = width;
 
  pixel_stream_context px_str_ctx = {NULL, 0, 0};

  if(rotate_image) {
    rotate_image(image, width, height, desired_comp);
    columns = width;
    rows = height;
  }

  if(mask) {
    int size = 0;

    mask(image, &px_str_ctx, &size, rows, columns, desired_comp);

    char* p = image + px_str_ctx.array[0].offset * desired_comp;

    for(int i = 0; i < size; i++, p += px_str_ctx.array[i].offset * desired_comp) {
      quicksort(p, px_str_ctx.array[i].length * desired_comp, desired_comp, hbs);
    }

  } else {

    for(int i = 0; i < columns; i++) {
      quicksort(image + rows * desired_comp * i, rows * desired_comp, desired_comp, hbs);
    }

  }

  free_arr(&px_str_ctx);

  if(rotate_image) {
    rotate_image(image, height, width, desired_comp);
  }

  if(strcmp(format, "png") == 0)
   stbi_write_png(output, width, height, desired_comp, image, width * desired_comp);
  if(strcmp(format, "pgm") == 0)
   stbi_write_png(output, width, height, desired_comp, image, width * desired_comp);
  if(strcmp(format, "ppm") == 0)
   stbi_write_png(output, width, height, desired_comp, image, width * desired_comp);
  if(strcmp(format, "jpg") == 0)
   stbi_write_jpg(output, width, height, desired_comp, image, 100);
  if(strcmp(format, "bmp") == 0)
   stbi_write_bmp(output, width, height, desired_comp, image);
}

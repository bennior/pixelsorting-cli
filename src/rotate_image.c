#include <stdlib.h>

#include "rotate_image.h"

void set_pixel(char* a, char* b) {
  *a = *b;
  *(a + 1) = *(b + 1);
  *(a + 2) = *(b + 2);
}

void horizontal_to_vertical(char* image, int width, int height, int comp) {
  char* vertical_image = (char*)malloc(width * height * comp);

  for(int i = 0; i < width; i++) {
    for(int j = 0; j < height; j++) {
        set_pixel((vertical_image + i * height * comp + j * comp), (image + i * comp + j * comp * width));
    }
  }
  
  for(int m = 0; m < width * height; m++) {
    set_pixel((image + m * comp), (vertical_image + m * comp));
  }

  free(vertical_image);
}


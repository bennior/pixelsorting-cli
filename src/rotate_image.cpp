#include <iostream>

#include "rotate_image.hpp"
#include "util.hpp"

void horizontal_to_vertical(char* image, int& width, int& height, int& channels) {
  char vertical_image[width * height * channels];

  for(int i = 0; i < width; i++) {
    for(int j = 0; j < height; j++) {
        set_pixel(vertical_image[i * height * channels + j * channels], image[i * channels + j * channels * width]);
    }
  }
  
  for(int m = 0; m < width * height; m++) {
    set_pixel(image[m * channels], vertical_image[m * channels]);
  }
}


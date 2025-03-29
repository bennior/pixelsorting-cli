#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

#include "hbs.hpp"
#include "pixelsort.hpp"
#include "quicksort.hpp"
#include "mask.hpp" 

struct pixel_streams {
  char* pixel_stream_start;
  int pixel_stream_length;
};

void pixelsort_image(std::string& input, std::string& output, void (*rotate_image)(char*, int&, int&, int&), float (*hbs)(char, char, char)) {
   int desired_channels = 3;
   int width = 0; int height = 0; int channels = 0;
  
   char* image = (char*) stbi_load(input.c_str(), &width, &height, &channels, desired_channels);
   char masked_image[width * height * desired_channels];  

   int rows = width; int columns = height;

   if(image == NULL) {
     std::cout << "Error in loading the image" << std::endl;
     exit(1);
   }

   if(rotate_image != NULL) {
     rotate_image(image, width, height, desired_channels);
     rows = height;
     columns = width;
   }

   mask(image, masked_image, width, height, desired_channels);

   for(int i = 0; i < columns; i++) {
     quicksort((image + i * rows * desired_channels), rows * desired_channels, hbs);
   }

   if(rotate_image != NULL) {
     rotate_image(image, height, width, desired_channels);
   }

   stbi_write_png(output.c_str(), width, height, desired_channels, image, width * desired_channels);
   stbi_write_png("images/masked_image.png", width, height, desired_channels, masked_image, width * desired_channels);
}


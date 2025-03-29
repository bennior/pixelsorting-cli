#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

#include "hbs.hpp"
#include "pixelsort.hpp"
#include "quicksort.hpp"

void pixelsort_image(std::string& input, std::string& output, void (*rotate_image)(char*, int&, int&, int&), float (*hbs)(char, char, char)) {
   int desired_channels = 3;
   int width = 0; int height = 0; int channels = 0;
  
   char* image = (char*) stbi_load(input.c_str(), &width, &height, &channels, desired_channels);

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

   for(int i = 0; i < columns; i++) {
     quicksort((image + i * rows * desired_channels), rows * 3, hbs);
   }

   if(rotate_image != NULL) {
     rotate_image(image, height, width, desired_channels);
   }

   stbi_write_png(output.c_str(), width, height, desired_channels, image, width * desired_channels);
}


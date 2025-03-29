#include <iostream>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

#include "hbs.hpp"
#include "pixelsort.hpp"
#include "quicksort.hpp"
#include "mask.hpp" 
#include "util.hpp"

void pixelsort_image(std::string& input, std::string& output, void (*mask_ptr)(char*, std::vector<pixel_stream>&, int&, int, int, int), void (*rotate_image)(char*, int&, int&, int&), float (*hbs)(char, char, char)) {
   int desired_channels = 3;
   int width = 0; int height = 0; int channels = 0;
  
   char* image = (char*) stbi_load(input.c_str(), &width, &height, &channels, desired_channels);

   std::vector<pixel_stream> px_str;
   px_str.reserve(1);
   px_str.emplace_back(pixel_stream{width, width});

   int columns = height;
   int rows = width;
 
   if(image == NULL) {
     std::cout << "Error in loading the image" << std::endl;
     exit(1);
   }

   if(assert_ptr(rotate_image)) {
     rotate_image(image, width, height, desired_channels);
     columns = width;
     rows = height;
     px_str[0] = pixel_stream{height, height};
   }

   if(assert_ptr(mask_ptr)) {
     mask_ptr(image, px_str, columns, rows, columns, desired_channels);
   }

   for(struct {int i; char* p;} def = {0, image + assert_ptr(mask_ptr) * px_str[def.i].offset * desired_channels}; def.i < columns; def.i++, def.p += px_str[def.i * assert_ptr(mask_ptr)].offset * desired_channels) {
        quicksort(def.p, px_str[def.i * assert_ptr(mask_ptr)].length * desired_channels, desired_channels, hbs); 
   }

   if(rotate_image) {
     rotate_image(image, height, width, desired_channels);
   }

   stbi_write_png(output.c_str(), width, height, desired_channels, image, width * desired_channels);
}


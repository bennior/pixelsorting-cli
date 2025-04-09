#include <iostream>
#include <vector>
#include <algorithm>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

#include "hbs.hpp"
#include "pixelsort.hpp"
#include "quicksort.hpp"
#include "mask.hpp" 
#include "util.hpp"
#include "image_format.hpp"



void pixelsort_image(std::string& input, std::string& output, void (*mask_ptr)(char*, std::vector<pixel_stream>&, int&, int, int, int), void (*rotate_image)(char*, int&, int&, int&), float (*hbs)(char, char, char)) {
   int desired_channels = 3;
   int width = 0; int height = 0; int channels = 0;

  
   char* image = (char*) stbi_load(input.c_str(), &width, &height, &channels, desired_channels);
   if(image == NULL) {
     std::cout << "Error in loading the image" << std::endl;
     exit(1);
   }

   std::string format = get_image_format(input);

   if(format.empty()) {
     std::cout << "Image format is not supported. Supported format are 'png', 'pgm', 'ppm', 'jpg', 'jpeg' and 'bmp'" << std::endl;
     exit(1);
   }

   if(format == "pgm" && hbs != get_brightness) {
     std::cout << "Option not available for image format 'pgm'. Use '--sort-by-brightness' or '-B' instead"<< std::endl;
     exit(1);
   }

   int columns = height;
   int rows = width;
 
   std::vector<pixel_stream> px_str;
   px_str.reserve(1);
   px_str.emplace_back(pixel_stream{width, width});

   if(rotate_image) {
     rotate_image(image, width, height, desired_channels);
     columns = width;
     rows = height;
     px_str[0] = pixel_stream{height, height};
   }

   if(mask_ptr) {
     mask_ptr(image, px_str, columns, rows, columns, desired_channels);
   }

   char* p = image + assert_ptr(mask_ptr) * px_str[0].offset * desired_channels;

   for(int i = 0; i < columns; i++, p += px_str[i * assert_ptr(mask_ptr)].offset * desired_channels) {
        quicksort(p, px_str[i * assert_ptr(mask_ptr)].length * desired_channels, desired_channels, hbs); 
   }

   if(rotate_image) {
     rotate_image(image, height, width, desired_channels);
   }

   switch(const_hash(format.c_str())) {
     case const_hash("png"): stbi_write_png(output.c_str(), width, height, desired_channels, image, width * desired_channels);
			     break;
     case const_hash("pgm"): stbi_write_png(output.c_str(), width, height, desired_channels, image, width * desired_channels);
			     break;
     case const_hash("ppm"): stbi_write_png(output.c_str(), width, height, desired_channels, image, width * desired_channels);
			     break;
     case const_hash("jpg"): stbi_write_jpg(output.c_str(), width, height, desired_channels, image, 100);
			     break;
     case const_hash("bmp"): stbi_write_bmp(output.c_str(), width, height, desired_channels, image);
			     break;
   }
}


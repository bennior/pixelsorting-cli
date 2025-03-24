#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

#include "hbs.hpp"
#include "util.hpp"
#include "pixelsort.hpp"

void partition(char* array, int low, int high, float (*hbs)(char, char, char)) {
    int i = low;
    int j = high;
    int pivot_index = high;
    float pivot = hbs(array[pivot_index], array[pivot_index + 1], array[pivot_index + 2]);

    while(i <= j) {
        while(hbs(array[i], array[i + 1], array[i + 2]) < pivot) 
            i += 3;
        while(hbs(array[j], array[j + 1], array[j + 2]) > pivot) 
            j -= 3;
        if(i <= j) {
            swap_pixel(array[i], array[j]);
	    i += 3;
	    j -= 3;
        }
    }

    if(j > low)
        partition(array, low, j, hbs);
    if(i < high) {
        partition(array, i, high, hbs);
    }
}

void quicksort(char* array, int length, float (*hbs)(char, char, char)) {
    partition(array, 0, length - 3, hbs);
}

void pixelsort(std::string& input, std::string& output, void (*rotate_image)(char*, int&, int&, int&), float (*hbs)(char, char, char)) {
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


#include "mask.hpp"
#include "hbs.hpp"

void mask(char* image, char* mask, int rows, int columns, int desired_channels) {
  
  float lower_threshold = -0.0;
  float upper_threshold = 0.2;

  for(int i = 0; i < rows * columns; i++) {
      if(get_brightness(image[i * desired_channels], image[i * desired_channels + 1], image[i * desired_channels + 2]) <= upper_threshold && get_brightness(image[i * desired_channels], image[i * desired_channels + 1], image[i * desired_channels + 2]) >= lower_threshold) {
       	  mask[i * desired_channels] = 0;
	  mask[i * desired_channels + 1] = 0;
	  mask[i * desired_channels + 2] = 0;
       }else {
       	  mask[i * desired_channels] = (char) 255;
	  mask[i * desired_channels + 1] = (char) 255;
	  mask[i * desired_channels + 2] = (char) 255;
       }
  }
}


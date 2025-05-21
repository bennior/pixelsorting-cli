#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "image_format.h"

const char* get_image_format(const char* filename) {

  if(strcmp(filename, "") == 0) {
    printf("Error processing user's input filename\n");
    exit(1);
  }

  FILE* file = fopen(filename, "rb"); 
  int ret = 0;

  if(!file) {
    printf("Error analysing image format\n");
    exit(1);
  }

  int buffer_size = 8;
  unsigned char buffer[buffer_size];

  ret = fread(buffer, 1, buffer_size, file);
  if(ret != buffer_size) {
    printf("Failed to read image file\n");
    exit(1);
  }

  ret = fclose(file);
  if(ret != 0) {
    printf("Failed to close image file\n");
    exit(1);
  }

  unsigned char png[8] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A}; //signature of png format to int 
  unsigned char pgm[3] = {0x50, 0x35, 0x0A};
  unsigned char ppm[3] = {0x50, 0x36, 0x0A};
  unsigned char jpg[4] = {0xFF, 0xD8, 0xFF, 0xE0};
  unsigned char jpeg[4] = {0xFF, 0xD8, 0xFF, 0xDB}; 
  unsigned char bmp[2] = {0x42, 0x4D};

  //png
  if(memcmp(png, buffer, 8) == 0)
    return "png";

  //jpg
  if(memcmp(jpg, buffer, 4) == 0 || memcmp(jpeg, buffer, 4) == 0)
    return "jpg";

  //ppm
  if(memcmp(ppm, buffer, 3) == 0)
    return "ppm";
 
  //pgm
  if(memcmp(pgm, buffer, 3) == 0)
    return "pgm";

  //bmp
  if(memcmp(bmp, buffer, 2) == 0)
    return "bmp";

  return ""; 
}


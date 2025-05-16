#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "image_format.h"

const char* get_image_format(const char* filename) {
  FILE* file = fopen(filename, "rb"); 

  if(!file) {
    printf("Error analysing image format\n");
    exit(1);
  }

  unsigned char buffer[8];
  fread(buffer, 1, 8, file);
  fclose(file);

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


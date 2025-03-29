#pragma once

#include <vector>

void swap_pixel(char& pixel_a, char& pixel_b);
void set_pixel(char& pixel_a, char& pixel_b);
struct pixel_stream {
  int offset;
  int length;
};
bool assert_ptr(void (*mask)(char*, std::vector<pixel_stream>&, int&, int, int, int));
bool assert_ptr(void (*rotate_image)(char*, int&, int&, int&));


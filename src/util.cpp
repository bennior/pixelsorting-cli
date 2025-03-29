#include <utility>
#include <vector>

#include "util.hpp"

void swap_pixel(char& pixel_a, char& pixel_b) {
  std::swap(pixel_a, pixel_b);
  std::swap(*(&pixel_a + 1), *(&pixel_b + 1));
  std::swap(*(&pixel_a + 2), *(&pixel_b + 2));
}

void set_pixel(char& pixel_a, char& pixel_b) {
  pixel_a = pixel_b;
  *(&pixel_a + 1) = *(&pixel_b + 1);
  *(&pixel_a + 2) = *(&pixel_b + 2);
}

bool assert_ptr(void (*rotate_image)(char*, int&, int&, int&)) {
  if(rotate_image) {
    return 1;
  }
  return 0;
}

bool assert_ptr(void (*mask)(char*, std::vector<pixel_stream>&, int&, int, int, int)) {
  if(mask) {
    return 1;
  }
  return 0;
}



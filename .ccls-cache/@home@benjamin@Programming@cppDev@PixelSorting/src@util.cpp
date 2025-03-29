#include <utility>

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

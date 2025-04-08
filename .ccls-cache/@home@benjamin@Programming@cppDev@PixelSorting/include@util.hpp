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
constexpr unsigned int const_hash(const char* c, int h = 0) {
    return c[h] ? (const_hash(c, h+1) * 33) ^ c[h] : 5381;
}

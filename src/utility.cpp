#include "utility.hpp"
#include <utility>

void swap(char& a, char& b) {
	char t = a;
	a = b;
	b = t;
}
void swap_pixel(char& pixel_a, char& pixel_b) {
	std::swap(pixel_a, pixel_b);
	std::swap(*(&pixel_a + 1), *(&pixel_b + 1));
	std::swap(*(&pixel_a + 2), *(&pixel_b + 2));
}

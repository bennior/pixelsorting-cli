#pragma once

#include <iostream>
#include <vector>

#include "util.hpp"

void pixelsort_image(std::string& input, std::string& output, void (*mask_ptr)(char*, std::vector<pixel_stream>&, int&, int, int, int), void (*rotate_image)(char*, int&, int&, int&), float (*hbs)(char, char, char));
void pixelsort_video(std::string& input, std::string& output, void (*mask_ptr)(char*, std::vector<pixel_stream>&, int&, int, int, int), void (*rotate_image)(char*, int&, int&, int&), float (*hbs)(char, char, char));

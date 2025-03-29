#pragma once

#include <iostream>

void pixelsort_image(std::string& input, std::string& output, void (*rotate_image)(char*, int&, int&, int&), float (*hbs)(char, char, char));

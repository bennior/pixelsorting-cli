#pragma once

#include <iostream>

void partition(char* array, int low, int high, float (*hbs)(char, char, char));
void quicksort(char* array, int length, float (*hbs)(char, char, char));
void pixelsort(std::string& input, std::string& output, void (*rotate_image)(char*, int&, int&, int&), float (*hbs)(char, char, char));

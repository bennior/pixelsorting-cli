#pragma once

void partition(char* array, int low, int high, int desired_channels, float (*hbs)(char, char, char));
void quicksort(char* array, int length, int desired_channels, float (*hbs)(char, char, char));

#pragma once

void partition(char* array, int low, int high, float (*hbs)(char, char, char));
void quicksort(char* array, int length, float (*hbs)(char, char, char));

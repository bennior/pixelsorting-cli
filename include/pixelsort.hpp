#pragma once

void partition(char* array, int low, int high, int (*hbs)(char, char, char));
void quicksort(char* array, int length, int (*hbs)(char, char, char));
void pixelsort(char* array, int length, int (*hbs)(char, char, char));

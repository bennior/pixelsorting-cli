#include "quicksort.hpp"
#include "util.hpp"

void partition(char* array, int low, int high, float (*hbs)(char, char, char)) {
    int i = low;
    int j = high;
    int pivot_index = high;
    float pivot = hbs(array[pivot_index], array[pivot_index + 1], array[pivot_index + 2]);

    while(i <= j) {
	while(hbs(array[i], array[i + 1], array[i + 2]) < pivot) 
	    i += 3;
	while(hbs(array[j], array[j + 1], array[j + 2]) > pivot) 
	    j -= 3;
	if(i <= j) {
	    swap_pixel(array[i], array[j]);
	    i += 3;
	    j -= 3;
	}
    }

    if(j > low)
	partition(array, low, j, hbs);
    if(i < high) {
	partition(array, i, high, hbs);
    }
}

void quicksort(char* array, int length, float (*hbs)(char, char, char)) {
    partition(array, 0, length - 3, hbs);
}


#include "quicksort.hpp"
#include "util.hpp"

void partition(char* array, int low, int high, int desired_channels, float (*hbs)(char, char, char)) {
    int i = low;
    int j = high;
    int pivot_index = high;
    float pivot = hbs(array[pivot_index], array[pivot_index + 1], array[pivot_index + 2]);

    while(i <= j) {
	while(hbs(array[i], array[i + 1], array[i + 2]) < pivot) 
	    i += desired_channels;
	while(hbs(array[j], array[j + 1], array[j + 2]) > pivot) 
	    j -= desired_channels;
	if(i <= j) {
	    swap_pixel(array[i], array[j]);
	    i += desired_channels;
	    j -= desired_channels;
	}
    }

    if(j > low)
	partition(array, low, j, desired_channels, hbs);
    if(i < high) {
	partition(array, i, high, desired_channels, hbs);
    }
}

void quicksort(char* array, int length, int desired_channels, float (*hbs)(char, char, char)) {
    partition(array, 0, length - desired_channels, desired_channels, hbs);
}


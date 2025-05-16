#include "quicksort.h"

void swap_pixel(char* a, char* b) {
  char temp = 0;

  temp = *a;
  *a = *b;
  *b = temp;

  temp = *(a + 1);
  *(a + 1) = *(b + 1);
  *(b + 1) = temp;

  temp = *(a + 2);
  *(a + 2) = *(b + 2);
  *(b + 2) = temp;
}

void partition(char* array, int low, int high, int desired_comp, float (*hbs)(char, char, char)) {
    int i = low;
    int j = high;
    int pivot_index = high;
    float pivot = hbs(array[pivot_index], array[pivot_index + 1], array[pivot_index + 2]);

    while(i <= j) {
	while(hbs(array[i], array[i + 1], array[i + 2]) < pivot) 
	    i += desired_comp;
	while(hbs(array[j], array[j + 1], array[j + 2]) > pivot) 
	    j -= desired_comp;
	if(i <= j) {
	    swap_pixel(array + i, array + j);
	    i += desired_comp;
	    j -= desired_comp;
	}
    }

    if(j > low)
	partition(array, low, j, desired_comp, hbs);
    if(i < high) {
	partition(array, i, high, desired_comp, hbs);
    }
}

void quicksort(char* array, int length, int desired_comp, float (*hbs)(char, char, char)) {
    partition(array, 0, length - desired_comp, desired_comp, hbs);
}


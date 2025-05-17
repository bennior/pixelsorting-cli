#include <stdlib.h>

#include "pixel_stream_context.h"

void init_arr(pixel_stream_context* a, int size) {
  a->array = (pixel_stream*) malloc(size * sizeof(pixel_stream));
  a->used = 0;
  a->size = size;
}

void insert_element(pixel_stream_context* a, pixel_stream element) {
  if(a->used == a->size) {
    a->size = a->size != 0 ? a->size * 2 : 1;
    a->array = (pixel_stream*)realloc(a->array, a->size * sizeof(pixel_stream));
  }
  a->array[a->used++] = element;
}

void free_arr(pixel_stream_context* a) {
  free(a->array);
  a->array = NULL;
  a->used = a->size = 0;
}


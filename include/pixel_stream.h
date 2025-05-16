#ifndef _PIXEL_STREAM_
#define _PIXEL_STREAM_

typedef struct {
  int offset;
  int length;
} pixel_stream;

typedef struct {
  pixel_stream* array;
  int used;
  int size;
} dynamic_array;

void init_arr(dynamic_array* a, int size);
void insert_element(dynamic_array* a, pixel_stream element);
void free_arr(dynamic_array* a);

#endif

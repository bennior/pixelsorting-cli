#ifndef _PIXEL_STREAM_CONTEXT_
#define _PIXEL_STREAM_CONTEXT_

typedef struct {
  int offset;
  int length;
} pixel_stream;

typedef struct {
  pixel_stream* array;
  int used;
  int size;
} pixel_stream_context;

void init_arr(pixel_stream_context* a, int size);
void insert_element(pixel_stream_context* a, pixel_stream element);
void free_arr(pixel_stream_context* a);

#endif

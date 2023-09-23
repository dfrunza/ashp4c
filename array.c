#include <memory.h>  /* memset */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>   /* exit */
#include "foundation.h"

void
array_extend(UnboundedArray* array, Arena* storage, int elem_size)
{
  assert(elem_size > 0);
  int segment_index = floor_log2(array->capacity + 1);
  if (segment_index >= array->segment_count) {
    printf("\nMaximum capacity has been reached.\n");
    exit(1);
  }
  int segment_capacity = (1 << segment_index);
  array->segment_table[segment_index] = arena_malloc(storage, segment_capacity * elem_size);
  array->capacity += segment_capacity;
}

void
array_init(UnboundedArray* array, Arena* storage, int elem_size, int max_capacity)
{
  assert(elem_size > 0);
  assert(max_capacity >= 15);
  array->segment_count = ceil_log2(max_capacity + 1);
  array->elem_count = 0;
  array->capacity = 15;
  array->segment_table = arena_malloc(storage, sizeof(void*) * array->segment_count);
  array->segment_table[0] = arena_malloc(storage, 16 * elem_size);
  array->segment_table[1] = array->segment_table[0] + 1 * elem_size;
  array->segment_table[2] = array->segment_table[0] + 3 * elem_size;
  array->segment_table[3] = array->segment_table[0] + 7 * elem_size;
}

static void
array_elem_at_i(UnboundedArray* array, int i, int elem_size,
      int* _segment_index, int* _elem_offset, void** _data_segment, void** _elem_slot)
{
  assert(elem_size > 0);
  int segment_index = floor_log2(i + 1);
  int elem_offset = i - ((1 << segment_index) - 1);
  void* data_segment = array->segment_table[segment_index];
  void* elem_slot = data_segment + elem_offset * elem_size;
  *_segment_index = segment_index;
  *_elem_offset = elem_offset;
  *_data_segment = data_segment;
  *_elem_slot = elem_slot;
}

void*
array_get(UnboundedArray* array, int i, int elem_size)
{
  assert(i >= 0 && i < array->elem_count);
  int segment_index, elem_offset;
  void* data_segment, *elem_slot;
  array_elem_at_i(array, i, elem_size, &segment_index, &elem_offset, &data_segment, &elem_slot);
  return elem_slot;
}

void*
array_set(UnboundedArray* array, int i, void* elem, int elem_size)
{
  assert(elem_size > 0);
  assert(i >= 0 && i < array->elem_count);
  int segment_index, elem_offset;
  void* data_segment, *elem_slot;
  array_elem_at_i(array, i, elem_size, &segment_index, &elem_offset, &data_segment, &elem_slot);
  memcpy(elem_slot, elem, elem_size);
  return elem_slot;
}

void*
array_append(UnboundedArray* array, Arena* storage, void* elem, int elem_size)
{
  if (array->elem_count >= array->capacity) {
    array_extend(array, storage, elem_size);
  }
  array->elem_count += 1;
  void* result = array_set(array, array->elem_count - 1, elem, elem_size);
  return result;
}


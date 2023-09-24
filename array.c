#include <memory.h>  /* memset */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>   /* exit */
#include "foundation.h"

void
array_extend(UnboundedArray* array, Arena* storage, int elem_size)
{
  assert(elem_size > 0);
  assert(array->elem_count >= array->capacity);
  int last_segment = floor_log2(array->capacity/16 + 1);
  if (last_segment >= array->segment_count) {
    printf("\nMaximum capacity has been reached.\n");
    exit(1);
  }
  int segment_capacity = 16 * (1 << last_segment);
  array->segment_table[last_segment] = arena_malloc(storage, elem_size * segment_capacity);
  array->capacity = 16 * ((1 << (last_segment + 1)) - 1);
}

void
array_init(UnboundedArray* array, Arena* storage, int elem_size, int max_capacity)
{
  assert(elem_size > 0);
  assert(max_capacity >= 16);
  array->elem_count = 0;
  array->capacity = 16;
  array->segment_count = ceil_log2(max_capacity/16 + 1);
  array->segment_table = arena_malloc(storage, sizeof(void*) * array->segment_count);
  array->segment_table[0] = arena_malloc(storage, 16 * elem_size);
}

void
array_elem_at_i(void* segment_table[], int i, void** _elem_slot, int elem_size)
{
  assert(elem_size > 0);
  int segment_index = floor_log2(i/16 + 1);
  int elem_offset = i - 16 * ((1 << segment_index) - 1);
  void* data_segment = segment_table[segment_index];
  void* elem_slot = data_segment + elem_offset * elem_size;
  *_elem_slot = elem_slot;
}

void*
array_get(UnboundedArray* array, int i, int elem_size)
{
  assert(elem_size > 0);
  assert(i >= 0 && i < array->elem_count);
  void* elem_slot;
  array_elem_at_i(array->segment_table, i, &elem_slot, elem_size);
  return elem_slot;
}

void*
array_set(UnboundedArray* array, int i, void* elem, int elem_size)
{
  assert(elem_size > 0);
  assert(i >= 0 && i < array->elem_count);
  void* elem_slot;
  array_elem_at_i(array->segment_table, i, &elem_slot, elem_size);
  memcpy(elem_slot, elem, elem_size);
  return elem_slot;
}

void*
array_append(UnboundedArray* array, Arena* storage, void* elem, int elem_size)
{
  assert(elem_size > 0);
  if (array->elem_count >= array->capacity) {
    array_extend(array, storage, elem_size);
  }
  void* elem_slot;
  array_elem_at_i(array->segment_table, array->elem_count, &elem_slot, elem_size);
  memcpy(elem_slot, elem, elem_size);
  array->elem_count += 1;
  return elem_slot;
}

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include "foundation.h"

void array_extend(Array* array, int elem_size)
{
  assert(elem_size > 0);
  assert(array->elem_count >= array->capacity);
  int last_segment;
  int segment_capacity;

  last_segment = floor(log2(array->capacity/16 + 1));
  if (last_segment >= array->data.segment_count) {
    printf("\nMaximum array capacity has been reached.\n");
    exit(1);
  }
  segment_capacity = 16 * (1 << last_segment);
  array->data.segments[last_segment] = arena_malloc(array->storage, elem_size * segment_capacity);
  array->capacity = 16 * ((1 << (last_segment + 1)) - 1);
}

Array* array_create(Arena* storage, int elem_size, int segment_count)
{
  assert(elem_size > 0);
  assert(segment_count >= 1 && segment_count <= 16);
  Array* array;

  array = arena_malloc(storage, sizeof(Array) + sizeof(void*) * segment_count);
  array->storage = storage;
  array_init(array->storage, array, elem_size, segment_count);
  return array;
}

void array_init(Arena* storage, Array* array, int elem_size, int segment_count)
{
  assert(elem_size > 0);
  assert(segment_count >= 1);

  array->storage = storage;
  array->elem_count = 0;
  array->capacity = 16;
  array->data.segment_count = segment_count;
  array->data.segments[0] = arena_malloc(array->storage, 16 * elem_size);
}

void* segment_locate_cell(SegmentTable* data, int i, int elem_size)
{
  assert(elem_size > 0);
  int segment_index, elem_offset;
  void* elem_slot;

  segment_index = floor(log2(i/16 + 1));
  elem_offset = i - 16 * ((1 << segment_index) - 1);
  elem_slot = data->segments[segment_index] + elem_offset * elem_size;
  return elem_slot;
}

void* array_get(Array* array, int i, int elem_size)
{
  assert(elem_size > 0);
  assert(i >= 0 && i < array->elem_count);
  void* elem_slot;

  elem_slot = segment_locate_cell(&array->data, i, elem_size);
  return elem_slot;
}

void* array_append(Array* array, int elem_size)
{
  assert(elem_size > 0);
  void* elem_slot;

  if (array->elem_count >= array->capacity) {
    array_extend(array, elem_size);
  }
  elem_slot = segment_locate_cell(&array->data, array->elem_count, elem_size);
  array->elem_count += 1;
  return elem_slot;
}

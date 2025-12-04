#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include "basic.h"
#include "foundation.h"

void* SegmentTable::locate_cell(int i, int elem_size)
{
  assert(elem_size > 0);
  int segment_index, elem_offset;
  void* elem_slot;

  segment_index = floor(log2(i/16 + 1));
  elem_offset = i - 16 * ((1 << segment_index) - 1);
  elem_slot = segments[segment_index] + elem_offset * elem_size;
  return elem_slot;
}

void Array::extend()
{
  assert(elem_size > 0);
  assert(elem_count >= capacity);
  int last_segment;
  int segment_capacity;

  last_segment = floor(log2(capacity/16 + 1));
  if (last_segment >= data.segment_count) {
    printf("\nMaximum array capacity has been reached.\n");
    exit(1);
  }
  segment_capacity = 16 * (1 << last_segment);
  data.segments[last_segment] = storage->malloc(elem_size * segment_capacity);
  capacity = 16 * ((1 << (last_segment + 1)) - 1);
}

Array* Array::create(Arena* storage, int elem_size, int segment_count)
{
  assert(elem_size > 0);
  assert(segment_count >= 1 && segment_count <= 16);
  Array* array;

  array = (Array*)storage->malloc(sizeof(Array) + sizeof(void*) * segment_count);
  array->storage = storage;
  array->init(array->storage, elem_size, segment_count);
  return array;
}

void Array::init(Arena* storage, int elem_size, int segment_count)
{
  assert(elem_size > 0);
  assert(segment_count >= 1);

  storage = storage;
  elem_count = 0;
  this->elem_size = elem_size;
  capacity = 16;
  data.segment_count = segment_count;
  data.segments[0] = storage->malloc(16 * elem_size);
}

void* Array::get(int i)
{
  assert(elem_size > 0);
  assert(i >= 0 && i < elem_count);
  void* elem_slot;

  elem_slot = data.locate_cell(i, elem_size);
  return elem_slot;
}

void* Array::append()
{
  assert(elem_size > 0);
  void* elem_slot;

  if (elem_count >= capacity) {
    extend();
  }
  elem_slot = data.locate_cell(elem_count, elem_size);
  elem_count += 1;
  return elem_slot;
}

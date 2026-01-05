#include "array.h"

void* ArrayElements::locate(int i)
{
  int segment_index = floor(log2(i/16 + 1));
  int element_offset = i - 16 * ((1 << segment_index) - 1);
  void* element_slot = (uint8_t*)segments[segment_index] + element_offset * element_size;
  return element_slot;
}

Array* Array::create(Arena* storage, int size, int segment_count)
{
  assert(segment_count >= 1 && segment_count <= 16);

  Array* array = (Array*)storage->allocate(sizeof(Array), 1);
  storage->allocate(sizeof(void**), segment_count);
  array->storage = storage;
  array->init(size, segment_count);
  return array;
}

void Array::init(int size, int segment_count)
{
  assert(segment_count >= 1 && segment_count <= 16);

  element_count = 0;
  capacity = 16;
  elements.segment_count = segment_count;
  elements.element_size = size;
  elements.segments[0] = storage->allocate(size, 16);
  memset(elements.segments[0], 0, sizeof(size) * 16);
}

void Array::extend()
{
  assert(element_count >= capacity);

  int last_segment = floor(log2(capacity/16 + 1));
  if (last_segment >= elements.segment_count) {
    printf("\nMaximum array capacity has been reached.\n");
    exit(1);
  }
  int segment_capacity = 16 * (1 << last_segment);
  elements.segments[last_segment] = storage->allocate(elements.element_size, segment_capacity);
  capacity = 16 * ((1 << (last_segment + 1)) - 1);
}

void* Array::get(int i)
{
  assert(i >= 0 && i < element_count);

  void* elem_slot = elements.locate(i);
  return elem_slot;
}

void* Array::append()
{
  if (element_count >= capacity) {
    extend();
  }
  void* elem_slot = elements.locate(element_count);
  element_count += 1;
  return elem_slot;
}
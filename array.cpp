#include <array.h>

template<class T>
Array<T>* Array<T>::create(Arena* storage, int segment_count)
{
  assert(segment_count >= 1 && segment_count <= 16);

  Array* array = storage->allocate<Array>(1);
  storage->allocate<T**>(segment_count);
  array->storage = storage;
  array->element_count = 0;
  array->capacity = 16;
  array->elements.segment_count = segment_count;
  array->elements.segments[0] = storage->allocate<T>(16);
  return array;
}

template<class T>
void Array<T>::extend()
{
  assert(element_count >= capacity);

  int last_segment = floor(log2(capacity/16 + 1));
  if (last_segment >= elements.segment_count) {
    printf("\nMaximum array capacity has been reached.\n");
    exit(1);
  }
  int segment_capacity = 16 * (1 << last_segment);
  elements.segments[last_segment] = storage->allocate<T>(segment_capacity);
  capacity = 16 * ((1 << (last_segment + 1)) - 1);
}

template<class T>
T* Array<T>::get(int i)
{
  assert(i >= 0 && i < element_count);

  T* elem_slot = elements.locate_cell(i);
  return elem_slot;
}

template<class T>
T* Array<T>::append()
{
  if (element_count >= capacity) {
    extend();
  }
  T* elem_slot = elements.locate_cell(element_count);
  element_count += 1;
  return elem_slot;
}

#include <type.h>
template struct Array<Type>;
template struct Array<Type*>;

#include <token.h>
template struct Array<Token>;
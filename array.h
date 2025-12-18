#pragma once
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <basic.h>
#include <array.h>
#include <arena.h>

/**
 * n  ...  segment count
 * C  ...  capacity
 *
 * n |  C
 * --+-----
 * 1 | 16
 * 2 | 48
 * 3 | 112
 * 4 | 240
 * 5 | 496
 * 6 | 1008
 * 7 | 2032
 * 8 | 4080
 * 9 | 8176
 * ...
 *
 * C(n) = (2^n - 1)*16
 **/

template<class T>
struct SegmentTable {
  int segment_count;
  T* segments[];

  T* locate_cell(int i)
  {
    int segment_index = floor(log2(i/16 + 1));
    int elem_offset = i - 16 * ((1 << segment_index) - 1);
    T* elem_slot = segments[segment_index] + elem_offset;
    return elem_slot;
  }
};

template<class T>
struct Array {
  Arena* storage;
  int elem_count;
  int capacity;
  SegmentTable<T> data;

  static Array* create(Arena* storage, int segment_count)
  {
    assert(segment_count >= 1 && segment_count <= 16);

    Array* array = storage->allocate<Array>();
    storage->allocate<T**>(segment_count);
    array->storage = storage;
    array->init(array->storage, segment_count);
    return array;
  }

  void init(Arena* storage, int segment_count)
  {
    assert(segment_count >= 1);

    this->storage = storage;
    elem_count = 0;
    capacity = 16;
    data.segment_count = segment_count;
    data.segments[0] = storage->allocate<T>(16);
  }

  void extend()
  {
    assert(elem_count >= capacity);

    int last_segment = floor(log2(capacity/16 + 1));
    if (last_segment >= data.segment_count) {
      printf("\nMaximum array capacity has been reached.\n");
      exit(1);
    }
    int segment_capacity = 16 * (1 << last_segment);
    data.segments[last_segment] = storage->allocate<T>(segment_capacity);
    capacity = 16 * ((1 << (last_segment + 1)) - 1);
  }

  T* get(int i)
  {
    assert(i >= 0 && i < elem_count);

    T* elem_slot = data.locate_cell(i);
    return elem_slot;
  }

  T* append()
  {
    if (elem_count >= capacity) {
      extend();
    }
    T* elem_slot = data.locate_cell(elem_count);
    elem_count += 1;
    return elem_slot;
  }
};

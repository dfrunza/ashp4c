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
  int element_count;
  int capacity;
  SegmentTable<T> elements;

  static Array* create(Arena* storage, int segment_count);
  void extend();
  T* get(int i);
  T* append();
};

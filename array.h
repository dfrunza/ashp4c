#pragma once
#include "arena.h"

/**
 * n  ...  segment count
 * C  ...  capacity (max. nr. of elements)
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

struct SegmentTable {
  int segment_count;
  void* segments[];

  void* locate_cell(int i, int elem_size);
};

struct Array {
  Arena* storage;
  int elem_count;
  int elem_size;
  int capacity;
  SegmentTable data;

  static Array* create(Arena* storage, int elem_size, int segment_count);
  void init(Arena* storage, int elem_size, int segment_count);
  void extend();
  void* get(int i);
  void* append();
};

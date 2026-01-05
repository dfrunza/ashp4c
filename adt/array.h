#pragma once
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include "basic.h"
#include "array.h"
#include "memory/arena.h"

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

struct ArrayElements {
  int segment_count;
  int element_size;
  void* segments[];

  void* locate(int i);
};

struct Array {
  Arena* storage;
  int element_count;
  int capacity;
  ArrayElements elements;

  static Array* allocate(Arena* storage, int element_size, int segment_count);
  void extend();
  void* get(int i);
  void* append();
};

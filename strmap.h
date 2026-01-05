#pragma once
#include <stdint.h>
#include "arena.h"
#include "array.h"

struct StrmapEntry {
  char* key;
  void* value;
  StrmapEntry* next_entry;
};

struct StrmapBucket {
  uint32_t h;
  StrmapEntry** entry_slot;
  int last_segment;
};

struct Strmap {
  Arena* storage;
  int entry_count;
  int capacity;
  ArrayElements entries;

  static Strmap* allocate(Arena* storage, int segment_count);
  void grow();
  void* lookup(char* key, StrmapEntry** entry_/*out*/, StrmapBucket* bucket/*out*/);
  StrmapEntry* insert(char* key, void* value, bool return_if_found);
  void DEBUG_occupancy();
};

struct StrmapIterator {
  Strmap* strmap;
  StrmapEntry* entry;
  int i;

  StrmapIterator();
  StrmapIterator(Strmap* strmap);
  void begin(Strmap* strmap);
  StrmapEntry* next();
};

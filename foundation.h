#pragma once
#include <stdint.h>
#include <stddef.h>

struct PageBlock {
  struct PageBlock* next_block;
  struct PageBlock* prev_block;
  uint8_t* memory_begin;
  uint8_t* memory_end;
};

struct Arena {
  PageBlock* owned_pages;
  void* memory_avail;
  void* memory_limit;

  static void reserve_memory(int amount);
  void* malloc(uint32_t size);
  void free();
  void grow(uint32_t size);
};

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

struct StrmapEntry {
  char* key;
  void* value;
  struct StrmapEntry* next_entry;
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
  SegmentTable entries;

  static Strmap* create(Arena* storage, int segment_count);
  void init(Arena* storage, int segment_count);
  void grow();
  void* lookup(char* key, StrmapEntry** entry, StrmapBucket* bucket);
  StrmapEntry* insert(char* key, void* value, bool return_if_found);
};

struct StrmapCursor {
  Strmap* strmap;
  int i;
  StrmapEntry* entry;

  void begin(Strmap* strmap);
  StrmapEntry* next();
};

struct MapEntry {
  struct MapEntry* next;
  struct MapEntry* left_branch;
  struct MapEntry* right_branch;
  void* key;
  void* value;
};

struct Map {
  Arena* storage; 
  MapEntry* first;
  MapEntry* root;

  MapEntry* search_entry(MapEntry* entry, void* key);
  MapEntry* insert_entry(MapEntry** branch, MapEntry* entry,
     void* key, void* value, bool return_if_found);
  MapEntry* insert(void* key, void* value, bool return_if_found);
  void* lookup(void* key, MapEntry** entry);
  int count();
};

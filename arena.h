#pragma once
#include <stdint.h>

struct PageBlock {
  PageBlock* next_block;
  PageBlock* prev_block;
  uint8_t* memory_begin;
  uint8_t* memory_end;

  static PageBlock* find_block_first_fit(int requested_memory_amount);
  static PageBlock* get_new_block_struct();
  void recycle_block_struct();
  PageBlock* block_insert_and_coalesce(PageBlock* new_block);
};

struct Arena {
  PageBlock* owned_pages;
  void* memory_avail;
  void* memory_limit;

  static void reserve_memory(int amount);
  void* _malloc(uint32_t size);
  void free();
  void grow(uint32_t size);

  template<class T> T* malloc(int count = 1)
  {
    T* t = (T*)_malloc(sizeof(T) * count);
    return t;
  }
};
#pragma once
#include <stdint.h>
#include <memory.h>
#include <basic.h>

#define ZMEM_ON_FREE  0
#define ZMEM_ON_ALLOC 1

struct PageBlock {
  PageBlock* next_block;
  PageBlock* prev_block;
  uint8_t* memory_begin;
  uint8_t* memory_end;

  static PageBlock* find_first_fit(int size);
  static PageBlock* new_block();
  void recycle();
  PageBlock* insert_and_coalesce(PageBlock* new_block);
};

struct Arena {
  PageBlock* owned_pages;
  void* memory_avail;
  void* memory_limit;

  void free();
  void grow(uint32_t size);

  template<class T>
  T* allocate(int count = 1)
  {
    assert(count > 0);

    uint8_t* user_memory = (uint8_t*)memory_avail;
    int size = sizeof(T) * count;
    if (user_memory + size >= (uint8_t*)memory_limit) {
      grow(size);
      user_memory = (uint8_t*)memory_avail;
    }
    memory_avail = user_memory + size;
    if (ZMEM_ON_ALLOC) {
      memset(user_memory, 0, size);
    }
    return (T*) user_memory;
  }
};

struct Memory
{
  int page_size;
  int total_page_count;
  uint8_t* page_memory_start;
  Arena storage;
  PageBlock* first_block;
  PageBlock* block_freelist_head;
  PageBlock* recycled_block_structs;

  static void reserve(int amount);
};

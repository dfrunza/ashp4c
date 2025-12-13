#pragma once
#include <stdint.h>
#include <memory.h>
#include <basic.h>
#include <list.h>

#define ZMEM_ON_FREE  0
#define ZMEM_ON_ALLOC 1

struct PageBlock {
  List<PageBlock> list;
  uint8_t* memory_begin;
  uint8_t* memory_end;

  static PageBlock* find_first_fit(int size);
  static PageBlock* new_block();
  void recycle();
  PageBlock* insert_and_coalesce(PageBlock* new_block);

  static inline PageBlock* owner_of(List<PageBlock>* list)
  {
    return ::owner_of(list, &PageBlock::list);
  }
};

struct Arena {
  PageBlock* owned_pages;
  uint8_t* memory_avail;
  uint8_t* memory_limit;

  void free();
  void grow(uint32_t size);

  template<class T>
  T* allocate(int count = 1)
  {
    assert(count > 0);

    uint8_t* user_memory = memory_avail;
    int size = sizeof(T) * count;
    if (user_memory + size >= memory_limit) {
      grow(size);
      user_memory = memory_avail;
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
  int page_count;
  uint8_t* page_memory;
  Arena block_storage;
  PageBlock* first_block;
  PageBlock* block_freelist;
  PageBlock* recycled_blocks;

  static void reserve(int amount);
};

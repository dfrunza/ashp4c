#pragma once
#include <stdint.h>
#include <memory.h>
#include "basic.h"
#include "list.h"

#define ZMEM_ON_FREE  0
#define ZMEM_ON_ALLOC 1

struct PageBlock {
  List link;
  uint8_t* memory_begin;
  uint8_t* memory_end;

  static PageBlock* find_first_fit(int size);
  static PageBlock* new_block();
  void recycle();
  PageBlock* insert_and_coalesce(PageBlock* block);
  static PageBlock* owner_of(List* list);
};

struct Arena {
  PageBlock* owned_pages;
  uint8_t* memory_avail;
  uint8_t* memory_limit;

  void grow(uint32_t size);
  void free();
  void* allocate(int size, int count);
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

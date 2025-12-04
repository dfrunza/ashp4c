#pragma once
#include <stdint.h>

struct PageBlock {
  PageBlock* next_block;
  PageBlock* prev_block;
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
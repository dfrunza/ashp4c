#pragma once
#include "basic.h"

struct Arena;

struct PageBlock
{
  struct PageBlock* next_block;
  struct PageBlock* prev_block;
  uint8_t* memory_begin;
  uint8_t* memory_end;
};

struct Arena 
{
  struct PageBlock* owned_pages;
  void* memory_avail;
  void* memory_limit;
};

struct ArenaUsage
{
  int total;
  int free;
  int in_use;
  int arena_count;
};

struct UnboundedArray
{
  void* index_table[24];
  int elem_size;
  int elem_count;
  int capacity;
  struct Arena* storage;
};

void init_memory(int memory_amount);
void* arena_push(struct Arena* arena, uint32_t size);
void arena_delete(struct Arena* arena);

struct ArenaUsage arena_get_usage(struct Arena* arena);
void arena_print_usage(struct Arena* arena, char* title);

void array_init(struct UnboundedArray* array, int elem_size, struct Arena* storage);
void* array_get(struct UnboundedArray* array, int i);
void array_set(struct UnboundedArray* array, int i, void* elem);
void array_append(struct UnboundedArray* array, void* elem);

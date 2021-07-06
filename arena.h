#pragma once
#include "basic.h"

struct Arena;

struct PageBlock
{
  struct PageBlock* next_page;
  struct Arena* arena_owning;

  struct PageBlock* next_block;
  struct PageBlock* prev_block;
  uint8_t* memory_begin;
  uint8_t* memory_end;
};

struct Arena 
{
  struct Arena* prev;
  void* memory;
  void* avail;
  void* limit;

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

struct Arena* arena_new(struct Arena* arena, uint32_t size);

void init_memory();
void* arena_push(struct Arena* arena, uint32_t size);
void arena_delete(struct Arena* arena);


struct ArenaUsage arena_get_usage(struct Arena* arena);
void arena_print_usage(struct Arena* arena, char* title);


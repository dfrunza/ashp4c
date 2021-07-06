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

#define init_memory  init_memory2
void init_memory1();
void init_memory2();

#define arena_push  arena_push3
void* arena_push1(struct Arena* arena, uint32_t size);
void* arena_push2(struct Arena* arena, uint32_t size);
void* arena_push3(struct Arena* arena, uint32_t size);

#define arena_delete  arena_delete2
void arena_delete1(struct Arena* arena);
void arena_delete2(struct Arena* arena);


struct ArenaUsage arena_get_usage(struct Arena* arena);
void arena_print_usage(struct Arena* arena, char* title);


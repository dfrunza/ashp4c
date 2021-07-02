#pragma once
#include "basic.h"

struct Arena;

struct Page
{
  struct Page* next_page;
};

struct Arena 
{
  struct Arena* prev;
  void* memory;
  void* avail;
  void* limit;

  struct Page* owned_pages;
  void* page_memory_start;
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
void* arena_push(struct Arena* arena, uint32_t size);

struct ArenaUsage arena_get_usage(struct Arena* arena);
void arena_print_usage(struct Arena* arena, char* title);


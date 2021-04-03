#pragma once
#include "basic.h"

struct Arena 
{
  struct Arena* prev;
  void* memory;
  void* avail;
  void* limit;
}
Arena;

struct ArenaUsage
{
  int total;
  int free;
  int in_use;
};

struct Arena* arena_new(struct Arena* arena, uint32_t size);
void* arena_push(struct Arena* arena, uint32_t size);

struct ArenaUsage arena_get_usage(struct Arena* arena);
void arena_print_usage(struct Arena* arena, char* title);

#define zero_struct(ptr, type) \
  *ptr = (struct type){};

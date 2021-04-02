#pragma once
#include "basic.h"

typedef struct Arena 
{
  void* memory;
  void* avail;
  void* limit;
}
Arena;

typedef struct
{
  int total;
  int free;
  int in_use;
}
ArenaUsage;

Arena* arena_new(Arena* arena, uint32_t size);
void* arena_push(Arena* arena, uint32_t size);

ArenaUsage arena_get_usage(Arena* arena);
void arena_print_usage(Arena* arena, char* title);

#define zero_struct(ptr, type) \
  *ptr = (struct type){};

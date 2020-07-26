#pragma once
#include "basic.h"

typedef struct Arena 
{
  uint8_t* memory;
  uint8_t* avail;
  uint8_t* limit;
  struct Arena* next;
  struct Arena* last;
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
Arena* arena_branch_new(Arena* arena, uint32_t size);
Arena* arena_branch_new_ratio(Arena* arena, float size_ratio);
void arena_free(Arena* arena);
void* arena_push(Arena* arena, uint32_t size);
#define arena_push_struct(arena, type)  (type*)arena_push((arena), sizeof(type))
#define arena_push_array(arena, type, count)  (type*)arena_push((arena), sizeof(type)*(count))
ArenaUsage arena_get_usage(Arena* arena);
void arena_print_usage(Arena* arena, char* title);
#define zero_struct(ptr, type) *ptr = (type){};

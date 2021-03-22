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
void arena_free(Arena* arena);
void* arena_push(Arena* arena, uint32_t size);

#define arena_push_struct(arena, type) ({\
  struct type* object = arena_push((arena), sizeof(struct type)); \
  *object = (struct type){}; \
  object;})

#define arena_push_array(arena, type, COUNT) \
  (type*)arena_push((arena), sizeof(type)*(COUNT))

ArenaUsage arena_get_usage(Arena* arena);
void arena_print_usage(Arena* arena, char* title);

#define zero_struct(ptr, type) \
  *ptr = (struct type){};

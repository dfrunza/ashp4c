#include "arena.h"

Arena*
arena_new(Arena* arena, uint32_t size)
{
  arena->memory = malloc(size);
  arena->avail = arena->memory;
  arena->limit = arena->memory + size;
  arena->next = 0;
  arena->last = arena;
  return arena;
}

void*
arena_push(Arena* arena, uint32_t size)
{
  void* object = arena->avail;
  arena->avail += size;
  assert (arena->avail < arena->limit);
  return object;
}

ArenaUsage
arena_get_usage(Arena* arena)
{
  ArenaUsage usage = {};
  usage.total = arena->limit - arena->memory;
  usage.in_use = arena->avail - arena->memory;
  usage.free = usage.total - usage.in_use;
  return usage;
}

void
arena_print_usage(Arena* arena, char* caption)
{
  ArenaUsage usage = arena_get_usage(arena);
  float free_ratio = usage.free / (float)usage.total;
  printf("%s\nfree: %d bytes, in_use: %d bytes, free: %.2f%%\n", \
         caption, usage.free, usage.in_use, free_ratio*100.f);
}


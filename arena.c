#include "arena.h"

internal int DEFAULT_SIZE_KB = 4*KILOBYTE;

void*
arena_push(struct Arena* arena, uint32_t size)
{
  uint8_t* object = arena->avail;
  if (object + size >= (uint8_t*)arena->limit) {
    int memory_amount = (size + sizeof(struct Arena) + DEFAULT_SIZE_KB - 1) & ~(DEFAULT_SIZE_KB - 1);
    uint8_t* memory = malloc(memory_amount);
    *(struct Arena*)memory = *arena;
    arena->prev = (struct Arena*)memory;
    arena->avail = memory + sizeof(struct Arena);
    arena->limit = memory + memory_amount;
    arena->memory = memory;
    object = arena->avail;
  }
  arena->avail = object + size;
  return object;
}

struct ArenaUsage
arena_get_usage(struct Arena* arena)
{
  struct ArenaUsage usage = {};
  usage.total = arena->limit - arena->memory;
  usage.in_use = arena->avail - arena->memory;
  usage.free = usage.total - usage.in_use;
  return usage;
}

void
arena_print_usage(struct Arena* arena, char* caption)
{
  struct ArenaUsage usage = arena_get_usage(arena);
  float free_ratio = usage.free / (float)usage.total;
  printf("%s\nfree: %d bytes, in_use: %d bytes, free: %.2f%%\n", \
         caption, usage.free, usage.in_use, free_ratio*100.f);
}


#include "arena.h"

#define DEBUG_ENABLED 0

internal int DEFAULT_SIZE_KB = 8*KILOBYTE;

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

void
arena_free(struct Arena* arena)
{
  struct Arena at_arena = *arena;
  int arena_count = 0;
  struct ArenaUsage usage = arena_get_usage(arena);
  while (1) {
    struct Arena prev_save = {};
    if (at_arena.prev) {
      prev_save = *(at_arena.prev);
    }
    if (at_arena.memory) {
      free(at_arena.memory);
    }
    arena_count += 1;
    if (!at_arena.prev) break;
    at_arena = prev_save;
  }
  if (DEBUG_ENABLED) {
    printf("\nfreed a total of %d bytes, in %d arenas.\n", usage.total, arena_count);
  }
}

struct ArenaUsage
arena_get_usage(struct Arena* arena)
{
  struct ArenaUsage usage = {};
  struct Arena* at_arena = arena;
  while (at_arena) {
    usage.total += at_arena->limit - at_arena->memory;
    usage.in_use += at_arena->avail - at_arena->memory;
    usage.arena_count += 1;
    at_arena = at_arena->prev;
  }
  usage.free = usage.total - usage.in_use;
  return usage;
}

void
arena_print_usage(struct Arena* arena, char* caption)
{
  struct ArenaUsage usage = arena_get_usage(arena);
  float free_fraction = 1.f;
  if (usage.total > 0) {
    free_fraction = usage.free / (float)usage.total;
  }
  printf("%s\nfree: %d bytes, in_use: %d bytes, free: %.2f%%\n", \
         caption, usage.free, usage.in_use, free_fraction*100.f);
}


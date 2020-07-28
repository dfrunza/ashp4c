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

Arena*
arena_branch_new(Arena* arena, uint32_t size)
{
  Arena* branch = arena_push_struct(arena, Arena);
  arena_new(branch, size);
  arena->last->next = branch;
  arena->last = branch;
  return branch;
}

Arena*
arena_branch_new_ratio(Arena* arena, float ratio)
{
  ArenaUsage trunk_usage = arena_get_usage(arena);
  uint32_t size = trunk_usage.total * ratio;
  Arena* result = arena_branch_new(arena, size);
  return result;
}

void
arena_free(Arena* arena)
{
  // FIXME: Finish this and test it.
  Arena* A = arena;
  while (A)
  {
    free(A->memory);
    A = A->next;
  }
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
  printf("%s free:%d, in_use:%d -> %.2f\n", caption, usage.free, usage.in_use, free_ratio);
}


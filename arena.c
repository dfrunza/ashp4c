#include "arena.h"
#include <memory.h>  // memset
#include <unistd.h>
#include <sys/mman.h>

#define DEBUG_ENABLED 1

internal int DEFAULT_SIZE_KB = 8*KILOBYTE;
internal int page_size = 0;
internal int page_count = 0;
internal int page_block_storage_size = 0;
internal int page_block_count = 0;
internal struct PageBlock* first_page_block = 0;
internal struct PageBlock* page_freelist = 0;
internal void* memory_start = 0;


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

uint8_t*
mprotect_enable_page(uint8_t* page_start, int page_count)
{
  if (mprotect(page_start, page_count * page_size, PROT_READ|PROT_WRITE) != 0) {
    perror("mprotect");
    exit(1);
  }
  return page_start + page_count * page_size;
}

uint8_t*
mprotect_disable_page(uint8_t* page_start, int page_count)
{
  memset(page_start, 0, page_count * page_size);
  if (mprotect(page_start, page_count * page_size, PROT_NONE) != 0) {
    perror("mprotect");
    exit(1);
  }
  return page_start + page_count * page_size;
}

void
init_memory()
{
  page_size = getpagesize();
  page_count = 32*MEGABYTE / page_size;
  memory_start = mmap(0, page_count * page_size, PROT_NONE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
  if (memory_start == MAP_FAILED) {
    perror("mmap");
    exit(1);
  }

  page_block_storage_size = (page_count * sizeof(struct PageBlock) + page_size - 1) & ~(page_size - 1);
  page_block_count = page_block_storage_size / page_size;
  mprotect_enable_page(memory_start, page_block_count);
  first_page_block = (struct PageBlock*)memory_start;
  page_freelist = first_page_block + page_block_count;

  struct PageBlock* p = page_freelist;
  int i = page_block_count;
  for (; i < page_count - 1; i++) {
    *p = (struct PageBlock){};
    p->next_block = p + 1;
    p++;
  }
  *p = (struct PageBlock){};  // last block
}

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
arena_delete(struct Arena* arena)
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
      memset(at_arena.memory, 0, at_arena.limit - at_arena.memory);
      free(at_arena.memory);
    }
    arena_count += 1;
    if (!at_arena.prev) break;
    at_arena = prev_save;
  }
  if (DEBUG_ENABLED) {
    printf("\nfreed a total of %d bytes in %d arenas.\n", usage.total, arena_count);
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

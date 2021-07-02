#include "arena.h"
#include <memory.h>  // memset
#include <unistd.h>
#include <sys/mman.h>

#define DEBUG_ENABLED 1

internal int DEFAULT_SIZE_KB = 8*KILOBYTE;
internal int page_size = 0;
internal int total_page_count = 0;
internal int freelist_storage_size = 0;
internal int freelist_storage_page_count = 0;
internal struct Page* first_page = 0;
internal struct Page* freelist_head = 0;
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
mprotect_allocate_page(uint8_t* page_start, int page_count)
{
  if (mprotect(page_start, page_count * page_size, PROT_READ|PROT_WRITE) != 0) {
    perror("mprotect");
    exit(1);
  }
  return page_start + page_count * page_size;
}

uint8_t*
mprotect_deallocate_page(uint8_t* page_start, int page_count)
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
  total_page_count = 32*MEGABYTE / page_size;
  memory_start = mmap(0, total_page_count * page_size, PROT_NONE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
  if (memory_start == MAP_FAILED) {
    perror("mmap");
    exit(1);
  }

  freelist_storage_size = (total_page_count * sizeof(struct Page) + page_size - 1) & ~(page_size - 1);
  freelist_storage_page_count = freelist_storage_size / page_size;
  mprotect_allocate_page(memory_start, freelist_storage_page_count);
  first_page = (struct Page*)memory_start;
  freelist_head = first_page + freelist_storage_page_count;

  struct Page* p = freelist_head;
  int i = freelist_storage_page_count;
  for (; i < total_page_count - 1; i++) {
    *p = (struct Page){};
    p->next_page = p + 1;
    p++;
  }
  *p = (struct Page){};  // last page
}

internal struct Page*
find_end_contiguous_sequence(struct Page* first_page)
{
  struct Page* last_page = first_page;
  struct Page* p = first_page;
  for (; p; p = p->next_page) {
    last_page = p;
    if (p->next_page && (p->next_page - p > 1)) {
      break;
    }
  }
  return last_page;
}

internal struct Page*
find_free_memory(int memory_amount)
{
  assert ((memory_amount % page_size) == 0);
  int requested_page_count = memory_amount / page_size;
  struct Page* start_page_seq = freelist_head;
  struct Page* end_page_seq = find_end_contiguous_sequence(start_page_seq);
  int page_count = end_page_seq - start_page_seq + 1;
  return start_page_seq;
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

void*
arena_push2(struct Arena* arena, uint32_t size)
{
  uint8_t* object = arena->memory_avail;
  if (object + size >= (uint8_t*)arena->memory_limit) {
    int memory_amount = (size + sizeof(struct Arena) + page_size - 1) & ~(page_size - 1);
    struct Page* start_free_block = find_free_memory(memory_amount);
  }
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

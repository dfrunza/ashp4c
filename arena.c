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
internal struct PageBlock* first_page = 0;
internal struct PageBlock* freelist_head = 0;
internal void* page_memory_start = 0;
internal struct Arena pageblock_storage = {};
internal struct PageBlock* first_block = 0;
internal struct PageBlock unused_block_structs = {};


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

void
init_memory()
{
  page_size = getpagesize();
  total_page_count = 200*KILOBYTE / page_size;
  page_memory_start = mmap(0, total_page_count * page_size, PROT_NONE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
  if (page_memory_start == MAP_FAILED) {
    perror("mmap");
    exit(1);
  }
  if (mprotect(page_memory_start, 1 * page_size, PROT_READ|PROT_WRITE) != 0) {
    perror("mprotect");
    exit(1);
  }
  first_block = page_memory_start;
  *first_block = (struct PageBlock){};
  first_block->memory_begin = (uint8_t*)page_memory_start;
  first_block->memory_end = first_block->memory_begin + (1 * page_size);

  freelist_head = first_block + 1;
  *freelist_head = (struct PageBlock){};
  freelist_head->memory_begin = first_block->memory_end;
  freelist_head->memory_end = freelist_head->memory_begin + ((total_page_count - 1) * page_size);

  pageblock_storage.owned_pages = first_block;
  pageblock_storage.memory_avail = first_block->memory_begin + sizeof(*first_block) + sizeof(*freelist_head);
  pageblock_storage.memory_limit = first_block->memory_end;
}

internal struct PageBlock*
find_block_first_fit(int requested_memory_amount)
{
  struct PageBlock* result = 0;
  struct PageBlock* b = freelist_head;
  while (b) {
    if ((b->memory_end - b->memory_begin) >= requested_memory_amount) {
      result = b;
      break;
    }
    b = b->next_block;
  }
  return result;
}

void*
arena_push(struct Arena* arena, uint32_t size)
{
  assert (size > 0);
  uint8_t* client_memory = arena->memory_avail;
  if (client_memory + size >= (uint8_t*)arena->memory_limit) {
    struct PageBlock* free_block = find_block_first_fit(size);
    if (!free_block) {
      printf("\nOut of memory.\n");
      exit(1);
    }
    uint8_t* alloc_memory_begin = 0, *alloc_memory_end = 0;
    int size_in_page_multiples = (size + page_size - 1) & ~(page_size - 1);
    if (size_in_page_multiples < (free_block->memory_end - free_block->memory_begin)) {
      alloc_memory_begin = free_block->memory_begin;
      alloc_memory_end = alloc_memory_begin + size_in_page_multiples;
      free_block->memory_begin = alloc_memory_end;
    } else if (size_in_page_multiples == (free_block->memory_end - free_block->memory_begin)) {
      alloc_memory_begin = free_block->memory_begin;
      alloc_memory_end = free_block->memory_end;
      free_block->memory_begin = alloc_memory_end;
    } else assert (0);

    if (mprotect(alloc_memory_begin, alloc_memory_end - alloc_memory_begin, PROT_READ|PROT_WRITE) != 0) {
      perror("mprotect");
      exit(1);
    }
    arena->memory_avail = alloc_memory_begin;
    arena->memory_limit = alloc_memory_end;

    struct PageBlock* alloc_block = arena_push(&pageblock_storage, sizeof(struct PageBlock));
    *alloc_block = (struct PageBlock){};
    alloc_block->memory_begin = alloc_memory_begin;
    alloc_block->memory_end = alloc_memory_end;
    alloc_block->prev_block = 0;
    alloc_block->next_block = arena->owned_pages;
    arena->owned_pages = alloc_block;

    client_memory = arena->memory_avail;
  }
  arena->memory_avail = client_memory + size;
  return client_memory;
}

void
arena_delete(struct Arena* arena)
{

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

#include <memory.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "foundation.h"

#define ZMEM_ON_FREE  0
#define ZMEM_ON_ALLOC 1

static int page_size = 0;
static int total_page_count = 0;
static void* page_memory_start = 0;
static Arena storage = {0};
static PageBlock* first_block = 0;
static PageBlock* block_freelist_head = 0;
static PageBlock* recycled_block_structs = 0;

void reserve_memory(int amount)
{
  page_size = getpagesize();
  total_page_count = ceil(amount / page_size);
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
  memset(first_block, 0, sizeof(PageBlock));
  first_block->memory_begin = (uint8_t*)page_memory_start;
  first_block->memory_end = first_block->memory_begin + (1 * page_size);

  block_freelist_head = first_block + 1;
  memset(block_freelist_head, 0, sizeof(PageBlock));
  block_freelist_head->memory_begin = first_block->memory_end;
  block_freelist_head->memory_end = block_freelist_head->memory_begin + ((total_page_count - 1) * page_size);

  storage.owned_pages = first_block;
  storage.memory_avail = first_block->memory_begin + 2*sizeof(PageBlock);
  storage.memory_limit = first_block->memory_end;
}

static PageBlock* find_block_first_fit(int requested_memory_amount)
{
  PageBlock* result = 0;
  PageBlock* b;

  b = block_freelist_head;
  while (b) {
    if ((b->memory_end - b->memory_begin) >= requested_memory_amount) {
      result = b;
      break;
    }
    b = b->next_block;
  }
  return result;
}

static void recycle_block_struct(PageBlock* block)
{
  memset(block, 0, sizeof(PageBlock));
  block->next_block = recycled_block_structs;
  recycled_block_structs = block;
}

static PageBlock* block_insert_and_coalesce(PageBlock* block_list, PageBlock* new_block)
{
  PageBlock* merged_list;
  PageBlock* left_neighbour = 0, *right_neighbour = 0;
  PageBlock* p;
  const int STITCH_NONE  = 0,
            STITCH_LEFT  = 1 << 1,
            STITCH_RIGHT = 1 << 2;
  int stitch_method;

  if (!block_list) {
    return new_block;
  }
  merged_list = block_list;
  p = block_list;
  while (p) {
    /* Find the left neighbour of 'new_block' in the ordered list of blocks. */
    if (p->memory_begin <= new_block->memory_begin) {
      left_neighbour = p;
      break;
    }
    p = p->next_block;
  }
  /* Insert the 'new_block' into the list. */
  if (left_neighbour) {
    right_neighbour = left_neighbour->next_block;
    left_neighbour->next_block = new_block;
    new_block->prev_block = left_neighbour;
    new_block->next_block = right_neighbour;
    if (right_neighbour) {
      right_neighbour->prev_block = new_block;
    }
  } else {
    new_block->next_block = block_list;
    block_list->prev_block = new_block;
    right_neighbour = new_block->next_block;
    merged_list = new_block;
  }

  /* Coalesce adjacent blocks. */
  stitch_method = STITCH_NONE;
  if (left_neighbour && (left_neighbour->memory_end == new_block->memory_begin)) {
    stitch_method |= STITCH_LEFT;
  }
  if (right_neighbour && (right_neighbour->memory_begin == new_block->memory_end)) {
    stitch_method |= STITCH_RIGHT;
  }
  if (stitch_method == (STITCH_LEFT | STITCH_RIGHT)) {
    left_neighbour->memory_end = right_neighbour->memory_end;
    left_neighbour->next_block = right_neighbour->next_block;
    if (right_neighbour->next_block) {
      right_neighbour->next_block->prev_block = left_neighbour;
    }
    recycle_block_struct(right_neighbour);
  } else if (stitch_method == STITCH_LEFT) {
    left_neighbour->memory_end = new_block->memory_end;
    left_neighbour->next_block = right_neighbour;
    if (right_neighbour) {
      right_neighbour->prev_block = left_neighbour;
    }
  } else if (stitch_method == STITCH_RIGHT) {
    right_neighbour->memory_begin = new_block->memory_begin;
    right_neighbour->prev_block = left_neighbour;
    if (left_neighbour) {
      left_neighbour->next_block = right_neighbour;
    } else {
      merged_list = right_neighbour;
    }
  }
  if (stitch_method != STITCH_NONE) {
    recycle_block_struct(new_block);
  }
  return merged_list;
}

static PageBlock* get_new_block_struct()
{
  PageBlock* block;

  block = recycled_block_structs;
  if (block) {
    recycled_block_structs = block->next_block;
  } else {
    block = arena_malloc(&storage, sizeof(PageBlock));
  }
  memset(block, 0, sizeof(PageBlock));
  return block;
}

static void arena_grow(Arena* arena, uint32_t size)
{
  PageBlock* free_block, *alloc_block;
  uint8_t* alloc_memory_begin = 0, *alloc_memory_end = 0;
  int size_in_page_multiples;

  free_block = find_block_first_fit(size);
  if (!free_block) {
    printf("\nOut of memory.\n");
    exit(1);
  }
  size_in_page_multiples = (size + page_size - 1) & ~(page_size - 1);
  if (size_in_page_multiples < (free_block->memory_end - free_block->memory_begin)) {
    alloc_memory_begin = free_block->memory_begin;
    alloc_memory_end = alloc_memory_begin + size_in_page_multiples;
    free_block->memory_begin = alloc_memory_end;
  } else if (size_in_page_multiples == (free_block->memory_end - free_block->memory_begin)) {
    alloc_memory_begin = free_block->memory_begin;
    alloc_memory_end = free_block->memory_end;
    free_block->memory_begin = alloc_memory_end;
  } else assert(0);

  if (mprotect(alloc_memory_begin, alloc_memory_end - alloc_memory_begin, PROT_READ|PROT_WRITE) != 0) {
    perror("mprotect");
    exit(1);
  }
  arena->memory_avail = alloc_memory_begin;
  arena->memory_limit = alloc_memory_end;

  alloc_block = get_new_block_struct();
  alloc_block->memory_begin = alloc_memory_begin;
  alloc_block->memory_end = alloc_memory_end;
  arena->owned_pages = block_insert_and_coalesce(arena->owned_pages, alloc_block);
}

void* arena_malloc(Arena* arena, uint32_t size)
{
  assert(size > 0);
  uint8_t* user_memory;

  user_memory = arena->memory_avail;
  if (user_memory + size >= (uint8_t*)arena->memory_limit) {
    arena_grow(arena, size);
    user_memory = arena->memory_avail;
  }
  arena->memory_avail = user_memory + size;
  if (ZMEM_ON_ALLOC) {
    memset(user_memory, 0, size);
  }
  return user_memory;
}

void arena_free(Arena* arena)
{
  PageBlock* p, *next_block;

  p = arena->owned_pages;
  while (p) {
    if (ZMEM_ON_FREE) {
      memset(p->memory_begin, 0, p->memory_end - p->memory_begin);
    }
    if (mprotect(p->memory_begin, p->memory_end - p->memory_begin, PROT_NONE) != 0) {
      perror("mprotect");
      exit(1);
    }
    next_block = p->next_block;
    block_freelist_head = block_insert_and_coalesce(block_freelist_head, p);
    p = next_block;
  }
  memset(arena, 0, sizeof(Arena));
}


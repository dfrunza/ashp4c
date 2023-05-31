#include <memory.h>  // memset
#include <unistd.h>
#include <sys/mman.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>   // exit
#include <math.h> // ceil
#include "arena.h"

#define ZERO_MEMORY_ON_FREE  0

internal int page_size = 0;
internal int total_page_count = 0;
internal void* page_memory_start = 0;
internal Arena pageblock_storage = {};
internal PageBlock* first_block = 0;
internal PageBlock* block_freelist_head = 0;
internal PageBlock* recycled_block_structs = 0;

void
alloc_memory(int memory_amount)
{
  page_size = getpagesize();
  total_page_count = ceil(memory_amount / page_size);
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
  memset(first_block, 0, sizeof(*first_block));
  first_block->memory_begin = (uint8_t*)page_memory_start;
  first_block->memory_end = first_block->memory_begin + (1 * page_size);

  block_freelist_head = first_block + 1;
  memset(block_freelist_head, 0, sizeof(*block_freelist_head));
  block_freelist_head->memory_begin = first_block->memory_end;
  block_freelist_head->memory_end = block_freelist_head->memory_begin + ((total_page_count - 1) * page_size);

  pageblock_storage.owned_pages = first_block;
  pageblock_storage.memory_avail = first_block->memory_begin + sizeof(*first_block) + sizeof(*block_freelist_head);
  pageblock_storage.memory_limit = first_block->memory_end;
}

internal PageBlock*
find_block_first_fit(int requested_memory_amount)
{
  PageBlock* result = 0;
  PageBlock* b = block_freelist_head;
  while (b) {
    if ((b->memory_end - b->memory_begin) >= requested_memory_amount) {
      result = b;
      break;
    }
    b = b->next_block;
  }
  return result;
}

internal void
recycle_block_struct(PageBlock* block)
{
  memset(block, 0, sizeof(*block));
  block->next_block = recycled_block_structs;
  recycled_block_structs = block;
}

internal PageBlock*
block_insert_and_coalesce(PageBlock* block_list, PageBlock* new_block)
{
  if (!block_list) {
    return new_block;
  }
  PageBlock* merged_list = block_list;
  PageBlock* left_neighbour = 0;
  PageBlock* right_neighbour = 0;
  PageBlock* p = block_list;
  while (p) {
    /* Find the left neighbour of 'new_block' in the ordered list of blocks. */
    if (p->memory_begin <= new_block->memory_begin) {
      left_neighbour = p;
      break;
    }
    p = p->next_block;
  }
  /* Insert the 'new_block' in the ordered list of blocks. */
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
  const int STITCH_LEFT = 1 << 1, STITCH_RIGHT = 1 << 2;
  int stitch_type = 0;
  if (left_neighbour && (left_neighbour->memory_end == new_block->memory_begin)) {
    stitch_type |= STITCH_LEFT;
  }
  if (right_neighbour && (right_neighbour->memory_begin == new_block->memory_end)) {
    stitch_type |= STITCH_RIGHT;
  }
  if (stitch_type == (STITCH_LEFT | STITCH_RIGHT)) {
    left_neighbour->memory_end = right_neighbour->memory_end;
    left_neighbour->next_block = right_neighbour->next_block;
    if (right_neighbour->next_block) {
      right_neighbour->next_block->prev_block = left_neighbour;
    }
    recycle_block_struct(right_neighbour);
  } else if (stitch_type == STITCH_LEFT) {
    left_neighbour->memory_end = new_block->memory_end;
    left_neighbour->next_block = right_neighbour;
    if (right_neighbour) {
      right_neighbour->prev_block = left_neighbour;
    }
  } else if (stitch_type == STITCH_RIGHT) {
    right_neighbour->memory_begin = new_block->memory_begin;
    right_neighbour->prev_block = left_neighbour;
    if (left_neighbour) {
      left_neighbour->next_block = right_neighbour;
    } else {
      merged_list = right_neighbour;
    }
  }
  if (stitch_type != 0) {
    recycle_block_struct(new_block);
  }
  return merged_list;
}

internal PageBlock*
get_new_block_struct()
{
  PageBlock* block = recycled_block_structs;
  if (block) {
    recycled_block_structs = block->next_block;
  } else {
    block = arena_push(&pageblock_storage, sizeof(*block));
  }
  memset(block, 0, sizeof(*block));
  return block;
}

void*
arena_push(Arena* arena, uint32_t size)
{
  assert (size > 0);
  uint8_t* client_memory = arena->memory_avail;
  if (client_memory + size >= (uint8_t*)arena->memory_limit) {
    PageBlock* free_block = find_block_first_fit(size);
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

    PageBlock* alloc_block = get_new_block_struct();
    alloc_block->memory_begin = alloc_memory_begin;
    alloc_block->memory_end = alloc_memory_end;
    arena->owned_pages = block_insert_and_coalesce(arena->owned_pages, alloc_block);

    client_memory = arena->memory_avail;
  }
  arena->memory_avail = client_memory + size;
  return client_memory;
}

void
arena_delete(Arena* arena)
{
  PageBlock* p = arena->owned_pages;
  while (p) {
    if (ZERO_MEMORY_ON_FREE) {
      memset(p->memory_begin, 0, p->memory_end - p->memory_begin);
    }
    if (mprotect(p->memory_begin, p->memory_end - p->memory_begin, PROT_NONE) != 0) {
      perror("mprotect");
      exit(1);
    }
    PageBlock* next_block = p->next_block;
    block_freelist_head = block_insert_and_coalesce(block_freelist_head, p);
    p = next_block;
  }
  memset(arena, 0, sizeof(*arena));
}


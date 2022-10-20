#include <memory.h>  // memset
#include <unistd.h>
#include <sys/mman.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>   // exit
#include "arena.h"

#define ZERO_MEMORY_ON_FREE  0

internal int page_size = 0;
internal int total_page_count = 0;
internal void* page_memory_start = 0;
internal struct Arena pageblock_storage = {};
internal struct PageBlock* first_block = 0;
internal struct PageBlock* block_freelist_head = 0;
internal struct PageBlock* recycled_block_structs = 0;

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

internal struct PageBlock*
find_block_first_fit(int requested_memory_amount)
{
  struct PageBlock* result = 0;
  struct PageBlock* b = block_freelist_head;
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
recycle_block_struct(struct PageBlock* block)
{
  memset(block, 0, sizeof(*block));
  block->next_block = recycled_block_structs;
  recycled_block_structs = block;
}

internal struct PageBlock*
block_insert_and_coalesce(struct PageBlock* block_list, struct PageBlock* new_block)
{
  if (!block_list) {
    return new_block;
  }
  struct PageBlock* merged_list = block_list;
  struct PageBlock* left_neighbour = 0;
  struct PageBlock* right_neighbour = 0;
  struct PageBlock* p = block_list;
  while (p) {
    // Find the left neighbour of 'new_block' in the ordered list of blocks.
    if (p->memory_begin <= new_block->memory_begin) {
      left_neighbour = p;
      break;
    }
    p = p->next_block;
  }
  // Insert the 'new_block' in the ordered list of blocks.
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

  // Coalesce adjacent blocks.
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

internal struct PageBlock*
get_new_block_struct()
{
  struct PageBlock* block = recycled_block_structs;
  if (block) {
    recycled_block_structs = block->next_block;
  } else {
    block = arena_push(&pageblock_storage, sizeof(*block));
  }
  memset(block, 0, sizeof(*block));
  return block;
}

void*
arena_push(struct Arena* arena, uint32_t size)
{
  assert (size > 0);
  uint8_t* client_memory = arena->memory_avail;
  if (client_memory + size >= (uint8_t*)arena->memory_limit) {
    struct PageBlock* free_block = find_block_first_fit(size);
    if (!free_block) {
      printf("\nERROR: Out of memory.\n");
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

    struct PageBlock* alloc_block = get_new_block_struct();
    alloc_block->memory_begin = alloc_memory_begin;
    alloc_block->memory_end = alloc_memory_end;
    arena->owned_pages = block_insert_and_coalesce(arena->owned_pages, alloc_block);

    client_memory = arena->memory_avail;
  }
  arena->memory_avail = client_memory + size;
  return client_memory;
}

void
arena_delete(struct Arena* arena)
{
  struct PageBlock* p = arena->owned_pages;
  while (p) {
    if (ZERO_MEMORY_ON_FREE) {
      memset(p->memory_begin, 0, p->memory_end - p->memory_begin);
    }
    if (mprotect(p->memory_begin, p->memory_end - p->memory_begin, PROT_NONE) != 0) {
      perror("mprotect");
      exit(1);
    }
    struct PageBlock* next_block = p->next_block;
    block_freelist_head = block_insert_and_coalesce(block_freelist_head, p);
    p = next_block;
  }
  memset(arena, 0, sizeof(*arena));
}

struct ArenaUsage
arena_get_usage(struct Arena* arena)
{
  struct ArenaUsage usage = {};
  return usage;
}

int
floor_log2(int x)
{
  int result = floor(log10(x) / log10(2));
  return result;
}

void
array_init(struct UnboundedArray* array, int elem_size, struct Arena* storage)
{
  memset(array->segment_table, 0, sizeof(array->segment_table));
  array->elem_size = elem_size;
  array->elem_count = 0;
  array->capacity = 0;
  array->storage = storage;
}

internal void
array_elem_at_i(struct UnboundedArray* array, int i, int* segment_index_, int* elem_offset_,
                void** data_segment_, void** elem_slot_)
{
  int segment_index = floor_log2(i + 1);
  int elem_offset = i - ((1 << segment_index) - 1);
  void* data_segment = array->segment_table[segment_index];
  void* elem_slot = data_segment + elem_offset * array->elem_size;

  *segment_index_ = segment_index;
  *elem_offset_ = elem_offset;
  *data_segment_ = data_segment;
  *elem_slot_ = elem_slot;
}

void*
array_get(struct UnboundedArray* array, int i)
{
  assert (i >= 0 && i < array->elem_count);
  int segment_index, elem_offset;
  void* data_segment, *elem_slot;
  array_elem_at_i(array, i, &segment_index, &elem_offset, &data_segment, &elem_slot);
  return elem_slot;
}

void*
array_set(struct UnboundedArray* array, int i, void* elem)
{
  assert (i >= 0 && i < array->elem_count);
  int segment_index, elem_offset;
  void* data_segment, *elem_slot;
  array_elem_at_i(array, i, &segment_index, &elem_offset, &data_segment, &elem_slot);
  memcpy(elem_slot, elem, array->elem_size);
  return elem_slot;
}

void*
array_append(struct UnboundedArray* array, void* elem)
{
  if (array->elem_count >= array->capacity) {
    int segment_index = floor_log2(array->elem_count + 1);
    if (segment_index >= ARRAY_MAX_SEGMENT) {
      printf("\nERROR: Maximum array capacity has been reached.\n");
      exit(1);
    }
    int segment_capacity = (1 << segment_index);
    array->segment_table[segment_index] = arena_push(array->storage, segment_capacity * array->elem_size);
    array->capacity += segment_capacity;
  }
  array->elem_count += 1;
  void* result = array_set(array, array->elem_count - 1, elem);
  return result;
}

void
list_init(struct List* list)
{
  assert(list->head == 0);
  assert(list->tail == 0);
  list->head = &list->sentinel;
  list->tail = list->head;
}

void
list_append_link(struct List* list, struct ListLink* link)
{
  assert(list->tail->next == 0);
  assert(link->prev == 0);
  list->tail->next = link;
  link->prev = list->tail;
  list->tail = link;
  list->link_count += 1;
}

struct ListLink*
list_first_link(struct List* list)
{
  struct ListLink* first = list->head->next;
  return first;
}

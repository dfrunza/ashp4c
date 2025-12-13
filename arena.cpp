#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include <memory.h>
#include <math.h>
#include <arena.h>

static Memory memory = {};

void Memory::reserve_memory(int amount)
{
  memory.page_size = getpagesize();
  memory.total_page_count = ceil(amount / memory.page_size);
  memory.page_memory_start = (uint8_t*)mmap(0, memory.total_page_count * memory.page_size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (memory.page_memory_start == MAP_FAILED) {
    perror("mmap");
    exit(1);
  }
  if (mprotect(memory.page_memory_start, 1 * memory.page_size, PROT_READ | PROT_WRITE) != 0) {
    perror("mprotect");
    exit(1);
  }
  memory.first_block = (PageBlock*)memory.page_memory_start;
  memset(memory.first_block, 0, sizeof(PageBlock));
  memory.first_block->memory_begin = (uint8_t*)memory.page_memory_start;
  memory.first_block->memory_end = memory.first_block->memory_begin + (1 * memory.page_size);

  memory.block_freelist_head = memory.first_block + 1;
  memset(memory.block_freelist_head, 0, sizeof(PageBlock));
  memory.block_freelist_head->memory_begin = memory.first_block->memory_end;
  memory.block_freelist_head->memory_end = memory.block_freelist_head->memory_begin + ((memory.total_page_count - 1) * memory.page_size);

  memory.storage.owned_pages = memory.first_block;
  memory.storage.memory_avail = memory.first_block->memory_begin + 2 * sizeof(PageBlock);
  memory.storage.memory_limit = memory.first_block->memory_end;
}

PageBlock* PageBlock::find_block_first_fit(int requested_memory_amount)
{
  PageBlock* result = 0;
  PageBlock* b = memory.block_freelist_head;
  while (b) {
    if ((b->memory_end - b->memory_begin) >= requested_memory_amount) {
      result = b;
      break;
    }
    b = b->next_block;
  }
  return result;
}

void PageBlock::recycle_block_struct()
{
  memset(this, 0, sizeof(PageBlock));
  this->next_block = memory.recycled_block_structs;
  memory.recycled_block_structs = this;
}

PageBlock* PageBlock::get_new_block_struct()
{
  PageBlock* block = memory.recycled_block_structs;
  if (block) {
    memory.recycled_block_structs = block->next_block;
  } else {
    block = memory.storage.allocate<PageBlock>();
  }
  memset(block, 0, sizeof(PageBlock));
  return block;
}

enum class BlockStitch : int {
  NONE  = 0,
  LEFT  = 1 << 1,
  RIGHT = 1 << 2,
};
inline BlockStitch operator | (BlockStitch lhs, BlockStitch rhs) {
  return (BlockStitch)((int)lhs | (int)rhs);
}
inline BlockStitch operator & (BlockStitch lhs, BlockStitch rhs) {
  return (BlockStitch)((int)lhs & (int)rhs);
}

PageBlock* PageBlock::block_insert_and_coalesce(PageBlock* new_block)
{
  if (!this) {
    return new_block;
  }

  PageBlock* left_neighbour = 0, *right_neighbour = 0;
  PageBlock* merged_list = this;
  PageBlock* p = this;

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
    new_block->next_block = this;
    prev_block = new_block;
    right_neighbour = new_block->next_block;
    merged_list = new_block;
  }

  /* Coalesce adjacent blocks. */
  enum BlockStitch stitch_op = BlockStitch::NONE;
  if (left_neighbour && (left_neighbour->memory_end == new_block->memory_begin)) {
    stitch_op = (stitch_op | BlockStitch::LEFT);
  }
  if (right_neighbour && (right_neighbour->memory_begin == new_block->memory_end)) {
    stitch_op = (stitch_op | BlockStitch::RIGHT);
  }
  if (stitch_op == (BlockStitch::LEFT | BlockStitch::RIGHT)) {
    left_neighbour->memory_end = right_neighbour->memory_end;
    left_neighbour->next_block = right_neighbour->next_block;
    if (right_neighbour->next_block) {
      right_neighbour->next_block->prev_block = left_neighbour;
    }
    right_neighbour->recycle_block_struct();
  } else if (stitch_op == BlockStitch::LEFT) {
    left_neighbour->memory_end = new_block->memory_end;
    left_neighbour->next_block = right_neighbour;
    if (right_neighbour) {
      right_neighbour->prev_block = left_neighbour;
    }
  } else if (stitch_op == BlockStitch::RIGHT) {
    right_neighbour->memory_begin = new_block->memory_begin;
    right_neighbour->prev_block = left_neighbour;
    if (left_neighbour) {
      left_neighbour->next_block = right_neighbour;
    } else {
      merged_list = right_neighbour;
    }
  }
  if (stitch_op != BlockStitch::NONE) {
    new_block->recycle_block_struct();
  }
  return merged_list;
}

void Arena::grow(uint32_t size)
{
  uint8_t* alloc_memory_begin = 0, *alloc_memory_end = 0;

  PageBlock* free_block = PageBlock::find_block_first_fit(size);
  if (!free_block) {
    printf("\nOut of memory.\n");
    exit(1);
  }
  int size_in_page_multiples = (size + memory.page_size - 1) & ~(memory.page_size - 1);
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
  memory_avail = alloc_memory_begin;
  memory_limit = alloc_memory_end;

  PageBlock* alloc_block = PageBlock::get_new_block_struct();
  alloc_block->memory_begin = alloc_memory_begin;
  alloc_block->memory_end = alloc_memory_end;
  owned_pages = owned_pages->block_insert_and_coalesce(alloc_block);
}

void Arena::free()
{
  PageBlock* p = owned_pages;
  while (p) {
    if (ZMEM_ON_FREE) {
      memset(p->memory_begin, 0, p->memory_end - p->memory_begin);
    }
    if (mprotect(p->memory_begin, p->memory_end - p->memory_begin, PROT_NONE) != 0) {
      perror("mprotect");
      exit(1);
    }
    PageBlock* next_block = p->next_block;
    memory.block_freelist_head = memory.block_freelist_head->block_insert_and_coalesce(p);
    p = next_block;
  }
  memset(this, 0, sizeof(Arena));
}

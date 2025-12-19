#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include <memory.h>
#include <math.h>
#include <arena.h>

static Memory memory = {};

void Memory::reserve(int amount)
{
  memory.page_size = getpagesize();
  memory.page_count = ceil(amount / memory.page_size);
  memory.page_memory = (uint8_t*)mmap(0, memory.page_count * memory.page_size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (memory.page_memory == MAP_FAILED) {
    perror("mmap");
    exit(1);
  }
  if (mprotect(memory.page_memory, 1 * memory.page_size, PROT_READ | PROT_WRITE) != 0) {
    perror("mprotect");
    exit(1);
  }
  memory.first_block = (PageBlock*)memory.page_memory;
  memset(memory.first_block, 0, sizeof(PageBlock));
  memory.first_block->memory_begin = (uint8_t*)memory.page_memory;
  memory.first_block->memory_end = memory.first_block->memory_begin + (1 * memory.page_size);

  memory.block_freelist = memory.first_block + 1;
  memset(memory.block_freelist, 0, sizeof(PageBlock));
  memory.block_freelist->memory_begin = memory.first_block->memory_end;
  memory.block_freelist->memory_end = memory.block_freelist->memory_begin + ((memory.page_count - 1) * memory.page_size);

  memory.block_storage.owned_pages = memory.first_block;
  memory.block_storage.memory_avail = memory.first_block->memory_begin + 2 * sizeof(PageBlock);
  memory.block_storage.memory_limit = memory.first_block->memory_end;
}

PageBlock* PageBlock::find_first_fit(int size)
{
  PageBlock* result = 0;
  PageBlock* b = memory.block_freelist;
  while (b) {
    if ((b->memory_end - b->memory_begin) >= size) {
      result = b;
      break;
    }
    b = PageBlock::owner_of(b->link.next);
  }
  return result;
}

void PageBlock::recycle()
{
  memset(this, 0, sizeof(PageBlock));
  this->link.next = &memory.recycled_blocks->link;
  memory.recycled_blocks = this;
}

PageBlock* PageBlock::new_block()
{
  PageBlock* block = memory.recycled_blocks;
  if (block) {
    memory.recycled_blocks = PageBlock::owner_of(block->link.next);
  } else {
    block = memory.block_storage.allocate<PageBlock>();
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

PageBlock* PageBlock::insert_and_coalesce(PageBlock* block)
{
  if (!this) {
    return block;
  }

  PageBlock* left_neighbour = 0, *right_neighbour = 0;
  PageBlock* merged_list = this;
  PageBlock* p = this;

  while (p) {
    /* Find the left neighbour of block in the ordered list of blocks. */
    if (p->memory_begin <= block->memory_begin) {
      left_neighbour = p;
      break;
    }
    p = PageBlock::owner_of(p->link.next);
  }

  /* Insert the block into the list. */
  if (left_neighbour) {
    right_neighbour = PageBlock::owner_of(left_neighbour->link.next);
    left_neighbour->link.next = &block->link;
    this->link.insert_in_between(&left_neighbour->link, &right_neighbour->link);
  } else {
    block->link.insert_before(&this->link);
    right_neighbour = PageBlock::owner_of(block->link.next);
    merged_list = block;
  }

  /* Coalesce adjacent blocks. */
  enum BlockStitch stitch_op = BlockStitch::NONE;
  if (left_neighbour && (left_neighbour->memory_end == block->memory_begin)) {
    stitch_op = (stitch_op | BlockStitch::LEFT);
  }
  if (right_neighbour && (right_neighbour->memory_begin == block->memory_end)) {
    stitch_op = (stitch_op | BlockStitch::RIGHT);
  }

  if (stitch_op == (BlockStitch::LEFT | BlockStitch::RIGHT)) {
    left_neighbour->memory_end = right_neighbour->memory_end;
    left_neighbour->link.next= right_neighbour->link.next;
    if (right_neighbour->link.next) {
      right_neighbour->link.next->prev = &left_neighbour->link;
    }
    right_neighbour->recycle();
  } else if (stitch_op == BlockStitch::LEFT) {
    left_neighbour->memory_end = block->memory_end;
    left_neighbour->link.next= &right_neighbour->link;
    if (right_neighbour) {
      right_neighbour->link.prev = &left_neighbour->link;
    }
  } else if (stitch_op == BlockStitch::RIGHT) {
    right_neighbour->memory_begin = block->memory_begin;
    right_neighbour->link.prev= &left_neighbour->link;
    if (left_neighbour) {
      left_neighbour->link.next = &right_neighbour->link;
    } else {
      merged_list = right_neighbour;
    }
  }
  if (stitch_op != BlockStitch::NONE) {
    block->recycle();
  }
  return merged_list;
}

void Arena::grow(uint32_t size)
{
  uint8_t* alloc_memory_begin = 0, *alloc_memory_end = 0;

  PageBlock* free_block = PageBlock::find_first_fit(size);
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

  PageBlock* alloc_block = PageBlock::new_block();
  alloc_block->memory_begin = alloc_memory_begin;
  alloc_block->memory_end = alloc_memory_end;
  owned_pages = owned_pages->insert_and_coalesce(alloc_block);
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
    PageBlock* next_block = PageBlock::owner_of(p->link.next);
    memory.block_freelist = memory.block_freelist->insert_and_coalesce(p);
    p = next_block;
  }
  memset(this, 0, sizeof(Arena));
}

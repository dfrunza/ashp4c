#pragma once
#include "basic.h"


struct Arena;

struct PageBlock {
  struct PageBlock* next_block;
  struct PageBlock* prev_block;
  uint8_t* memory_begin;
  uint8_t* memory_end;
};

struct Arena {
  struct PageBlock* owned_pages;
  void* memory_avail;
  void* memory_limit;
};

struct ArenaUsage {
  int total;
  int free;
  int in_use;
  int arena_count;
};

// 1,048,575 elements
#define ARRAY_MAX_SEGMENT 20

struct UnboundedArray {
  void* segment_table[ARRAY_MAX_SEGMENT];
  int elem_size;
  int elem_count;
  int capacity;
  struct Arena* storage;
};

struct ListLink {
  struct ListLink* prev;
  struct ListLink* next;
  void* object;
};

struct List {
  struct ListLink sentinel;
  struct ListLink* head;
  struct ListLink* tail;
  int link_count;
};


void init_memory(int memory_amount);
void* arena_push(struct Arena* arena, uint32_t size);
void arena_delete(struct Arena* arena);

struct ArenaUsage arena_get_usage(struct Arena* arena);
void arena_print_usage(struct Arena* arena, char* title);

void array_init(struct UnboundedArray* array, int elem_size, struct Arena* storage);
void* array_get(struct UnboundedArray* array, int i);
void array_set(struct UnboundedArray* array, int i, void* elem);
void array_append(struct UnboundedArray* array, void* elem);

void list_init(struct List* list);
void list_append_link(struct List* list, struct ListLink* link);
struct ListLink* list_first_link(struct List* list);


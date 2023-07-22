#include <memory.h>  // memset
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>   // exit
#include "foundation.h"

static void
array_extend(UnboundedArray* array, int elem_count)
{
  int segment_index = floor_log2(elem_count);
  if (segment_index >= array->segment_length) {
    printf("\nMaximum array capacity has been reached.\n");
    exit(1);
  }
  int segment_capacity = (1 << segment_index);
  array->segment_table[segment_index] = arena_malloc(array->storage, segment_capacity * array->elem_size);
  array->capacity += segment_capacity;
}

void
array_create(UnboundedArray* array, Arena* storage, int elem_size, int max_array_length)
{
  array->segment_length = ceil_log2(max_array_length);
  array->segment_table = arena_malloc(storage, sizeof(void*) * array->segment_length);
  array->elem_size = elem_size;
  array->elem_count = 0;
  array->capacity = 0;
  array->storage = storage;
  for (int i = 0; i < 3; i++) {
    array_extend(array, 1 << i);
  }
}

static void
array_elem_at_i(UnboundedArray* array, int i, int* segment_index_, int* elem_offset_,
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
array_get(UnboundedArray* array, int i)
{
  assert(i >= 0 && i < array->elem_count);
  int segment_index, elem_offset;
  void* data_segment, *elem_slot;
  array_elem_at_i(array, i, &segment_index, &elem_offset, &data_segment, &elem_slot);
  return elem_slot;
}

void*
array_set(UnboundedArray* array, int i, void* elem)
{
  assert(i >= 0 && i < array->elem_count);
  int segment_index, elem_offset;
  void* data_segment, *elem_slot;
  array_elem_at_i(array, i, &segment_index, &elem_offset, &data_segment, &elem_slot);
  memcpy(elem_slot, elem, array->elem_size);
  return elem_slot;
}

void*
array_append(UnboundedArray* array, void* elem)
{
  if (array->elem_count >= array->capacity) {
    array_extend(array, array->elem_count + 1);
  }
  array->elem_count += 1;
  void* result = array_set(array, array->elem_count - 1, elem);
  return result;
}

void
list_create(List* list, Arena* storage)
{
  list->last_item = &list->sentinel;
  list->sentinel.next = list->sentinel.prev = 0;
  list->item_count = 0;
  list->storage = storage;
}

ListItem*
_list_create_item(List* list, int item_size)
{
  assert(item_size >= sizeof(ListItem));
  ListItem* li = arena_malloc(list->storage, item_size);
  return li;
}

ListItem*
_list_first_item(List* list)
{
  ListItem* li = list->sentinel.next;
  return li;
}

void
list_append_item(List* list, ListItem* item, int count)
{
  assert(item->prev == 0);
  ListItem* tail = list->last_item;
  tail->next = item;
  item->prev = tail;
  list->last_item = item;
  list->item_count += count;
}

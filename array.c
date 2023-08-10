#include <memory.h>  /* memset */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>   /* exit */
#include "foundation.h"

void
array_extend(UnboundedArray* array)
{
  int segment_index = floor_log2(array->capacity + 1);
  if (segment_index >= array->segment_length) {
    printf("\nMaximum array capacity has been reached.\n");
    exit(1);
  }
  int segment_capacity = (1 << segment_index);
  array->segment_table[segment_index] = arena_malloc(array->storage, segment_capacity * array->elem_size);
  array->capacity += segment_capacity;
}

void
array_create(UnboundedArray* array, Arena* storage, int elem_size, int max_capacity)
{
  assert(elem_size > 0);
  assert(max_capacity >= 7);
  array->segment_length = ceil_log2(max_capacity + 1);
  array->segment_table = arena_malloc(storage, sizeof(void*) * array->segment_length);
  array->elem_size = elem_size;
  array->elem_count = 0;
  array->capacity = 0;
  array->storage = storage;
  for (int i = 0; i < 3; i++) {
    array_extend(array);
  }
}

static void
array_elem_at_i(UnboundedArray* array, int i, int* _segment_index, int* _elem_offset,
                void** _data_segment, void** _elem_slot)
{
  int segment_index = floor_log2(i + 1);
  int elem_offset = i - ((1 << segment_index) - 1);
  void* data_segment = array->segment_table[segment_index];
  void* elem_slot = data_segment + elem_offset * array->elem_size;

  *_segment_index = segment_index;
  *_elem_offset = elem_offset;
  *_data_segment = data_segment;
  *_elem_slot = elem_slot;
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
    array_extend(array);
  }
  array->elem_count += 1;
  void* result = array_set(array, array->elem_count - 1, elem);
  return result;
}

void
list_create(List* list, Arena* storage, int item_size)
{
  assert(item_size >= sizeof(void*));
  list->last_item = &list->sentinel;
  list->sentinel.next = list->sentinel.prev = 0;
  list->item_size = item_size;
  list->item_count = 0;
  list->storage = storage;
}

void*
list_cursor_begin(List* list)
{
  ListItem* item = list->sentinel.next;
  list->cursor.item = item;
  return item ? &item->elem : 0;
}

void*
list_cursor_next(List* list)
{
  ListItem* item = list->cursor.item;
  item = item->next;
  list->cursor.item = item;
  return item ? &item->elem : 0;
}

void*
list_cursor_prev(List* list)
{
  ListItem* item = list->cursor.item;
  item = item->prev;
  list->cursor.item = item;
  return item ? &item->elem : 0;
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

void*
list_append(List* list, void* elem)
{
  ListItem* item = arena_malloc(list->storage, sizeof(ListItem) + list->item_size);
  memcpy(item->elem, elem, list->item_size);
  list_append_item(list, item, 1);
  return &item->elem;
}

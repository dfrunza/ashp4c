#include <memory.h>  // memset
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>   // exit
#include <stdarg.h>   // va_list, va_start, va_end
#include <math.h> // floor, log10
#include "arena.h"

void
assert_(char* message, char* file, int line)
{
  printf("%s:%d: ", file, line);
  if(!message || message[0] == '\0') {
    message = "";
  }
  printf("assert(%s)\n", message);
  exit(2);
}

bool
cstr_is_letter(char c)
{
  return ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z');
}

bool
cstr_is_digit(char c, int base)
{
  if (base == 10) {
    return '0' <= c && c <= '9';
  } else if (base == 16) {
    return ('0' <= c && c <= '9') || ('a' <= c && c <= 'f') || ('A' <= c && c <= 'F');
  } else if (base == 8) {
    return '0' <= c && c <= '7';
  } else if (base == 2) {
    return c == '0' || c == '1';
  } else assert(0);
  return false;
}

bool
cstr_is_ascii_printable(char c)
{
  return ' ' <= c && c <= '~';
}

bool
cstr_is_whitespace(char c)
{
  return c == ' ' || c == '\n' || c == '\r' || c == '\t';
}

int
cstr_len(char* str)
{
  int len = 0;
  while(*str++ != 0)
    len++;
  return len;
}

char*
cstr_copy(char* dest_str, char* src_str)
{
  do
    *dest_str++ = *src_str++;
  while(*src_str);
  return dest_str;
}

void
cstr_copy_substr(char* dest_str, char* begin_char, char* end_char)
{
  char* src_str = begin_char;

  do
    *dest_str++ = *src_str++;
  while(src_str <= end_char);
}

bool
cstr_start_with(char* str, char* prefix)
{
  while(*str == *prefix) {
    str++;
    prefix++;
    if(*prefix == '\0')
      break;
  }
  bool result = (*prefix == '\0');
  return result;
}

bool
cstr_match(char* str_a, char* str_b)
{
  while (*str_a == *str_b) {
    str_a++;
    str_b++;
    if (*str_a == '\0')
      break;
  }
  bool result = (*str_a == *str_b);
  return result;
}

void
cstr_print_substr(char* begin_char, char* end_char)
{
  char* c = begin_char;
  while (c <= end_char) {
    printf("%c", *c);
    c++;
  }
}

void
error_(char* file, int line, char* message, ...)
{
  printf("ERROR: ");
  if (!message) {
    printf("at %s:%d\n", file, line);
  } else {
    va_list args;
    va_start(args, message);
    vprintf(message, args);
    va_end(args);
    printf("\n");
  }
  exit(1);
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

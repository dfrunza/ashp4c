#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>  /* exit */
#include <memory.h>  /* memset */
#include <math.h>  /* floor, ceil, log2 */
#include "foundation.h"

static const uint32_t P = 257, Q = 4294967029;
static const uint32_t SIGMA = 2654435769;

static uint32_t hash_string(char* string, uint32_t m)
{
  assert(m > 0 && m <= 32);
  uint32_t K = 0, h;
  uint64_t KxSigma;

  for (uint8_t* s = (uint8_t*)string; (*s); s++) {
    K = (P * K + (*s)) % Q;
  }
  KxSigma = (uint64_t)K * (uint64_t)SIGMA;
  h = ((uint32_t)KxSigma) >> (32 - m);  /* 0 <= h < 2^m */
  return h;
}

static uint32_t hash_key(char* key, int m, int capacity)
{
  uint32_t h;
  h = hash_string(key, m) % capacity;
  return h;
}

Strmap* strmap_create(Arena* storage, int segment_count)
{
  assert(segment_count >= 1 && segment_count <= 16);
  Strmap* strmap;

  strmap = arena_malloc(storage, sizeof(Strmap) + sizeof(StrmapEntry**) * segment_count);
  strmap->storage = storage;
  strmap_init(strmap->storage, strmap, segment_count);
  return strmap;
}

void strmap_init(Arena* storage, Strmap* strmap, int segment_count)
{
  assert(segment_count >= 1);

  strmap->storage = storage;
  strmap->entry_count = 0;
  strmap->capacity = 16;
  strmap->entries.segment_count = segment_count;
  strmap->entries.segments[0] = arena_malloc(strmap->storage, sizeof(StrmapEntry*) * 16);
  memset(strmap->entries.segments[0], 0, sizeof(StrmapEntry*) * 16);
}

static void strmap_grow(Strmap* strmap)
{
  int last_segment;
  StrmapCursor it = {0};
  StrmapEntry* first_entry, *last_entry;
  StrmapEntry* entry, *next_entry;
  StrmapEntry** segment, **entry_slot;
  int entry_count;
  int segment_capacity; 
  uint32_t h;

  last_segment = floor(log2(strmap->capacity/16 + 1));
  if (last_segment >= strmap->entries.segment_count) {
    printf("\nMaximum capacity has been reached.\n");
    exit(1);
  }
  strmap_cursor_begin(&it, strmap);
  first_entry = strmap_cursor_next(&it);
  last_entry = first_entry;
  entry_count = first_entry ? 1 : 0;
  for (entry = strmap_cursor_next(&it);
       entry != 0; entry = strmap_cursor_next(&it)) {
    last_entry->next_entry = entry;
    last_entry = entry;
    entry_count += 1;
  }
  assert(entry_count == strmap->entry_count);
  segment_capacity = 16 * (1 << last_segment);
  strmap->entries.segments[last_segment] = arena_malloc(strmap->storage, sizeof(StrmapEntry*) * segment_capacity);
  strmap->capacity = 16 * ((1 << (last_segment + 1)) - 1);
  for (int i = 0; i <= last_segment; i++) {
    segment_capacity = 16 * (1 << i);
    for (int j = 0; j < segment_capacity; j ++) {
      segment = strmap->entries.segments[i];
      segment[j] = 0;
    }
  }
  for (entry = first_entry; entry != 0; ) {
    next_entry = entry->next_entry;
    h = hash_key(entry->key, 4 + (last_segment + 1), strmap->capacity);
    entry_slot = segment_locate_cell(&strmap->entries, h, sizeof(StrmapEntry*));
    entry->next_entry = *entry_slot;
    *entry_slot = entry;
    entry = next_entry;
  }
}

void* strmap_lookup(Strmap* strmap, char* key, StrmapEntry** entry_/*out*/, StrmapBucket* bucket/*out*/)
{
  int last_segment;
  StrmapEntry** entry_slot, *entry;
  uint32_t h;

  last_segment = floor(log2(strmap->capacity/16));
  h = hash_key(key, 4 + (last_segment + 1), strmap->capacity);
  entry_slot = segment_locate_cell(&strmap->entries, h, sizeof(StrmapEntry*));
  entry = *entry_slot;
  while (entry) {
    if (cstr_match(entry->key, key)) {
      break;
    }
    entry = entry->next_entry;
  }
  if (entry_) { *entry_ = entry; }
  if (bucket) {
    bucket->h = h;
    bucket->entry_slot = entry_slot;
    bucket->last_segment = last_segment;
  }
  if (entry) { return entry->value; }
  return 0;
}

StrmapEntry* strmap_insert(Strmap* strmap, char* key, void* value, bool return_if_found)
{
  StrmapEntry* entry;
  StrmapBucket bucket = {0};
 
  strmap_lookup(strmap, key, &entry, &bucket);
  if (entry) {
    if (return_if_found) { return entry; } else { return 0; }
  }

  if (strmap->entry_count >= strmap->capacity) {
    strmap_grow(strmap);
    bucket.last_segment = floor(log2(strmap->capacity/16));
    bucket.h = hash_key(key, 4 + (bucket.last_segment + 1), strmap->capacity);
    bucket.entry_slot = segment_locate_cell(&strmap->entries, bucket.h, sizeof(StrmapEntry*));
  }
  entry = arena_malloc(strmap->storage, sizeof(StrmapEntry));
  entry->key = key;
  entry->value = value;
  entry->next_entry = *bucket.entry_slot;
  *bucket.entry_slot = entry;
  strmap->entry_count += 1;
  return entry;
}

void strmap_cursor_begin(StrmapCursor* cursor, Strmap* strmap)
{
  cursor->strmap = strmap;
  cursor->i = -1;
  cursor->entry = 0;
}

StrmapEntry* strmap_cursor_next(StrmapCursor* cursor)
{
  Strmap* strmap;
  StrmapEntry* entry = 0;
  StrmapEntry** entry_slot;

  strmap = cursor->strmap;
  entry = cursor->entry;
  if (entry) {
    entry = entry->next_entry;
    if (entry) {
      cursor->entry = entry;
      return cursor->entry;
    }
  }
  cursor->i++;
  while (cursor->i < strmap->capacity) {
    entry_slot = segment_locate_cell(&strmap->entries, cursor->i, sizeof(StrmapEntry*));
    entry = *entry_slot;
    if (entry) {
      cursor->entry = entry;
      break;
    }
    cursor->i++;
  }
  return entry;
}

void Debug_strmap_occupancy(Strmap* strmap)
{
  StrmapEntry** entry_slot;
  StrmapEntry* entry;
  int empty_buckets = 0;
  int total_entry_count = 0,
      entry_count = 0,
      max_bucket_length = 0;

  for (int i = 0; i < strmap->capacity; i++) {
    entry_slot = segment_locate_cell(&strmap->entries, i, sizeof(StrmapEntry*));
    entry = *entry_slot;
    entry_count = 0;
    if (entry) {
      while (entry) {
        entry_count += 1;
        entry = entry->next_entry;
      }
      if (entry_count > max_bucket_length) {
        max_bucket_length = entry_count;
      }
    } else {
      empty_buckets += 1;
    }
    total_entry_count += entry_count;
    printf("[%d] -> %d\n", i, entry_count);
  }
  printf(
    "Entry count: %d\n" \
    "Empty buckets: %d\n" \
    "Max. bucket length: %d\n", total_entry_count, empty_buckets, max_bucket_length);
}

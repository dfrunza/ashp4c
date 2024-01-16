#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>  /* exit */
#include <memory.h>  /* memset */
#include <math.h>  /* floor, ceil, log2 */
#include "foundation.h"

static const uint32_t P = 257, Q = 4294967029;
static const uint32_t SIGMA = 2654435769;

static uint32_t
hash_string(char* string, uint32_t m)
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

static uint32_t
hashmap_hash_key(char* key, int m, int capacity)
{
  uint32_t h;
  h = hash_string(key, m) % capacity;
  return h;
}

Hashmap*
hashmap_create(Arena* storage, int max_capacity)
{
  assert(max_capacity >= 16);
  int segment_count;
  Hashmap* hashmap;

  segment_count = ceil(log2(max_capacity/16 + 1));
  hashmap = arena_malloc(storage, sizeof(Hashmap) + sizeof(HashmapEntry**) * segment_count);
  hashmap_init(hashmap, storage, segment_count);
  return hashmap;
}

void
hashmap_init(Hashmap* hashmap, Arena* storage, int segment_count)
{
  assert(segment_count >= 1);

  hashmap->entry_count = 0;
  hashmap->capacity = 16;
  hashmap->entries.segment_count = segment_count;
  hashmap->entries.segments[0] = arena_malloc(storage, sizeof(HashmapEntry*) * 16);
  memset(hashmap->entries.segments[0], 0, sizeof(HashmapEntry*) * 16);
}

static void
hashmap_grow(Hashmap* hashmap, Arena* storage)
{
  int last_segment;
  HashmapCursor it = {};
  HashmapEntry* first_entry, *last_entry;
  HashmapEntry* entry, *next_entry;
  HashmapEntry** segment, **entry_slot;
  int entry_count;
  int segment_capacity; 
  uint32_t h;

  last_segment = floor(log2(hashmap->capacity/16 + 1));
  if (last_segment >= hashmap->entries.segment_count) {
    printf("\nMaximum hashmap capacity has been reached.\n");
    exit(1);
  }
  hashmap_cursor_begin(&it, hashmap);
  first_entry = hashmap_cursor_next_entry(&it);
  last_entry = first_entry;
  entry_count = first_entry ? 1 : 0;
  for (entry = hashmap_cursor_next_entry(&it);
       entry != 0; entry = hashmap_cursor_next_entry(&it)) {
    last_entry->next_entry = entry;
    last_entry = entry;
    entry_count += 1;
  }
  assert(entry_count == hashmap->entry_count);
  segment_capacity = 16 * (1 << last_segment);
  hashmap->entries.segments[last_segment] = arena_malloc(storage, sizeof(HashmapEntry*) * segment_capacity);
  hashmap->capacity = 16 * ((1 << (last_segment + 1)) - 1);
  for (int i = 0; i <= last_segment; i++) {
    segment_capacity = 16 * (1 << i);
    for (int j = 0; j < segment_capacity; j ++) {
      segment = hashmap->entries.segments[i];
      segment[j] = 0;
    }
  }
  for (entry = first_entry; entry != 0; ) {
    next_entry = entry->next_entry;
    h = hashmap_hash_key(entry->key, 4 + (last_segment + 1), hashmap->capacity);
    entry_slot = segment_locate_cell(&hashmap->entries, h, sizeof(HashmapEntry*));
    entry->next_entry = *entry_slot;
    *entry_slot = entry;
    entry = next_entry;
  }
}

HashmapEntry*
hashmap_lookup_entry(Hashmap* hashmap, char* key)
{
  int last_segment;
  HashmapEntry** entry_slot, *entry;
  uint32_t h;

  last_segment = floor(log2(hashmap->capacity/16));
  h = hashmap_hash_key(key, 4 + (last_segment + 1), hashmap->capacity);
  entry_slot = segment_locate_cell(&hashmap->entries, h, sizeof(HashmapEntry*));
  entry = *entry_slot;
  while (entry) {
    if (cstr_match(entry->key, key)) {
      break;
    }
    entry = entry->next_entry;
  }
  return entry;
}

HashmapEntry*
hashmap_insert_entry(Hashmap* hashmap, Arena* storage, char* key, uint64_t value)
{
  int last_segment;
  HashmapEntry* entry, **entry_slot;
  uint32_t h;

  last_segment = floor(log2(hashmap->capacity/16));
  h = hashmap_hash_key(key, 4 + (last_segment + 1), hashmap->capacity);
  entry_slot = segment_locate_cell(&hashmap->entries, h, sizeof(HashmapEntry*));
  entry = *entry_slot;
  while (entry) {
    if (cstr_match(entry->key, key)) {
      break;
    }
    entry = entry->next_entry;
  }
  if (entry) {
    return 0;
  }

  if (hashmap->entry_count >= hashmap->capacity) {
    hashmap_grow(hashmap, storage);
    last_segment = floor(log2(hashmap->capacity/16));
    h = hashmap_hash_key(key, 4 + (last_segment + 1), hashmap->capacity);
  }
  entry_slot = segment_locate_cell(&hashmap->entries, h, sizeof(HashmapEntry*));
  entry = arena_malloc(storage, sizeof(HashmapEntry));
  entry->key = key;
  entry->value = value;
  entry->next_entry = *entry_slot;
  *entry_slot = entry;
  hashmap->entry_count += 1;
  return entry;
}

HashmapEntry*
hashmap_insert_or_lookup_entry(Hashmap* hashmap, Arena* storage, char* key, uint64_t value)
{
  int last_segment;
  HashmapEntry** entry_slot, *entry;
  uint32_t h;

  last_segment = floor(log2(hashmap->capacity/16));
  h = hashmap_hash_key(key, 4 + (last_segment + 1), hashmap->capacity);
  entry_slot = segment_locate_cell(&hashmap->entries, h, sizeof(HashmapEntry*));
  entry = *entry_slot;
  while (entry) {
    if (cstr_match(entry->key, key)) {
      break;
    }
    entry = entry->next_entry;
  }
  if (entry) {
    return entry;
  }

  if (hashmap->entry_count >= hashmap->capacity) {
    hashmap_grow(hashmap, storage);
    last_segment = floor(log2(hashmap->capacity/16));
    h = hashmap_hash_key(key, 4 + (last_segment + 1), hashmap->capacity);
  }
  entry_slot = segment_locate_cell(&hashmap->entries, h, sizeof(HashmapEntry*));
  entry = arena_malloc(storage, sizeof(HashmapEntry));
  entry->key = key;
  entry->value = value;
  entry->next_entry = *entry_slot;
  *entry_slot = entry;
  hashmap->entry_count += 1;
  return entry;
}

void
hashmap_cursor_begin(HashmapCursor* cursor, Hashmap* hashmap)
{
  cursor->hashmap = hashmap;
  cursor->i = -1;
  cursor->entry = 0;
}

HashmapEntry*
hashmap_cursor_next_entry(HashmapCursor* cursor)
{
  Hashmap* hashmap;
  HashmapEntry* entry = 0;
  HashmapEntry** entry_slot;

  hashmap = cursor->hashmap;
  entry = cursor->entry;
  if (entry) {
    entry = entry->next_entry;
    if (entry) {
      cursor->entry = entry;
      return cursor->entry;
    }
  }
  cursor->i++;
  while (cursor->i < hashmap->capacity) {
    entry_slot = segment_locate_cell(&hashmap->entries, cursor->i, sizeof(HashmapEntry*));
    entry = *entry_slot;
    if (entry) {
      cursor->entry = entry;
      break;
    }
    cursor->i++;
  }
  return entry;
}

void
Debug_hashmap_occupancy(Hashmap* hashmap)
{
  HashmapEntry** entry_slot;
  HashmapEntry* entry;
  int empty_buckets = 0;
  int total_entry_count = 0,
      entry_count = 0,
      max_bucket_length = 0;

  for (int i = 0; i < hashmap->capacity; i++) {
    entry_slot = segment_locate_cell(&hashmap->entries, i, sizeof(HashmapEntry*));
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

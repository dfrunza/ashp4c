#include <memory.h>  /* memset */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>  /* exit */
#include <stdarg.h>  /* va_list, va_start, va_end */
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
hash_uint32(uint32_t i, uint32_t m)
{
  assert(m > 0 && m <= 32);
  uint32_t K, h;
  uint64_t KxSigma;

  K = i;
  KxSigma = (uint64_t)K * (uint64_t)SIGMA;
  h = ((uint32_t)KxSigma) >> (32 - m);  /* 0 <= h < 2^m */
  return h;
}

static uint64_t
hash_uint64(uint64_t i, uint32_t m)
{
  assert(m > 0 && m <= 32);
  uint32_t upper_half, lower_half;
  uint32_t K, h;
  uint64_t KxSigma;

  upper_half = (uint32_t)(i >> 32);
  lower_half = (uint32_t)(0x00000000ffffffffl & i);
  K = upper_half ^ lower_half;
  KxSigma = (uint64_t)K * (uint64_t)SIGMA;
  h = ((uint32_t)KxSigma) >> (32 - m);  /* 0 <= h < 2^m */
  return h;
}

static int
hashmap_hash_key(enum HashmapKeyType key_type, HashmapKey* key, int m, int capacity)
{
  int h;

  if (key_type == HKEY_STRING) {
    h = hash_string(key->str_key, m) % capacity;
  } else if (key_type == HKEY_UINT32) {
    h = hash_uint32(key->u32_key, m) % capacity;
  } else if (key_type == HKEY_UINT64) {
    h = hash_uint64(key->u64_key, m) % capacity;
  } else assert(0);
  return h;
}

static bool
key_equal(enum HashmapKeyType key_type, HashmapKey* key_A, HashmapKey* key_B)
{
  if (key_type == HKEY_STRING) {
    return cstr_match(key_A->str_key, key_B->str_key);
  } else if (key_type == HKEY_UINT32) {
    return key_A->u32_key == key_B->u32_key;
  } else if (key_type == HKEY_UINT64) {
    return key_A->u64_key == key_B->u64_key;
  } else assert(0);
  return false;
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
  HashmapEntry** segment;

  hashmap->entry_count = 0;
  hashmap->capacity = 16;
  hashmap->entries.segment_count = segment_count;
  hashmap->entries.segments[0] = arena_malloc(storage, sizeof(HashmapEntry*) * 16);
  for (int j = 0; j < 16; j ++) {
    segment = hashmap->entries.segments[0];
    segment[j] = 0;
  }
}

static void
hashmap_grow(Hashmap* hashmap, Arena* storage, enum HashmapKeyType key_type)
{
  int last_segment;
  HashmapCursor it = {};
  HashmapEntry* first_entry, *last_entry;
  HashmapEntry* entry, *next_entry;
  int entry_count;
  int segment_capacity; 
  HashmapEntry** segment, **entry_slot;
  int h;

  last_segment = floor(log2(hashmap->capacity/16 + 1));
  if (last_segment >= hashmap->entries.segment_count) {
    printf("\nMaximum capacity has been reached.\n");
    exit(1);
  }
  hashmap_cursor_begin(&it);
  first_entry = hashmap_cursor_next_entry(&it, hashmap);
  last_entry = first_entry;
  entry_count = first_entry ? 1 : 0;
  for (entry = hashmap_cursor_next_entry(&it, hashmap);
       entry != 0; entry = hashmap_cursor_next_entry(&it, hashmap)) {
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
    h = hashmap_hash_key(key_type, &entry->key, 4 + (last_segment + 1), hashmap->capacity);
    entry_slot = array_elem_at_i(&hashmap->entries, h, sizeof(HashmapEntry*));
    entry->next_entry = *entry_slot;
    *entry_slot = entry;
    entry = next_entry;
  }
}

HashmapEntry*
hashmap_lookup_entry(Hashmap* hashmap, enum HashmapKeyType key_type, ...)
{
  va_list args;
  HashmapKey key = {};
  int last_segment;
  int h;
  HashmapEntry** entry_slot, *entry;

  va_start(args, key_type);
  if (key_type == HKEY_STRING) {
    key = (HashmapKey){ .str_key = va_arg(args, char*) };
  } else if (key_type == HKEY_UINT32) {
    key = (HashmapKey){ .u32_key = va_arg(args, uint32_t) };
  } else if (key_type == HKEY_UINT64) {
    key = (HashmapKey){ .u64_key = va_arg(args, uint64_t) };
  } else assert(0);
  va_end(args);
  last_segment = floor(log2(hashmap->capacity/16));
  h = hashmap_hash_key(key_type, &key, 4 + (last_segment + 1), hashmap->capacity);
  entry_slot = array_elem_at_i(&hashmap->entries, h, sizeof(HashmapEntry*));
  entry = *entry_slot;
  while (entry) {
    if (key_equal(key_type, &entry->key, &key)) {
      break;
    }
    entry = entry->next_entry;
  }
  return entry;
}

HashmapEntry*
hashmap_get_entry(Hashmap* hashmap, Arena* storage, int value_size, enum HashmapKeyType key_type, ...)
{
  assert(value_size > 0);
  va_list args;
  HashmapKey key = {};
  int last_segment;
  int h;
  HashmapEntry** entry_slot, *entry;

  va_start(args, key_type);
  if (key_type == HKEY_STRING) {
    key = (HashmapKey){ .str_key = va_arg(args, char*) };
  } else if (key_type == HKEY_UINT32) {
    key = (HashmapKey){ .u32_key = va_arg(args, uint32_t) };
  } else if (key_type == HKEY_UINT64) {
    key = (HashmapKey){ .u64_key = va_arg(args, uint64_t) };
  } else assert(0);
  va_end(args);
  last_segment = floor(log2(hashmap->capacity/16));
  h = hashmap_hash_key(key_type, &key, 4 + (last_segment + 1), hashmap->capacity);
  entry_slot = array_elem_at_i(&hashmap->entries, h, sizeof(HashmapEntry*));
  entry = *entry_slot;
  while (entry) {
    if (key_equal(key_type, &entry->key, &key)) {
      break;
    }
    entry = entry->next_entry;
  }
  if (entry) {
    return entry;
  }
  if (hashmap->entry_count >= hashmap->capacity) {
    hashmap_grow(hashmap, storage, key_type);
    last_segment = floor(log2(hashmap->capacity/16));
    h = hashmap_hash_key(key_type, &key, 4 + (last_segment + 1), hashmap->capacity);
  }
  entry = arena_malloc(storage, sizeof(HashmapEntry) + value_size);
  entry->key = key;
  entry_slot = array_elem_at_i(&hashmap->entries, h, sizeof(HashmapEntry*));
  entry->next_entry = *entry_slot;
  *entry_slot = entry;
  hashmap->entry_count += 1;
  return entry;
}

void
hashmap_cursor_begin(HashmapCursor* cursor)
{
  cursor->i = -1;
  cursor->entry = 0;
}

HashmapEntry*
hashmap_cursor_next_entry(HashmapCursor* cursor, Hashmap* hashmap)
{
  HashmapEntry* entry = 0;
  HashmapEntry** entry_slot;

  if (cursor->entry) {
    entry = cursor->entry->next_entry;
    if (entry) {
      cursor->entry = entry;
      return cursor->entry;
    }
  }
  cursor->i++;
  while (cursor->i < hashmap->capacity) {
    entry_slot = array_elem_at_i(&hashmap->entries, cursor->i, sizeof(HashmapEntry*));
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

  for (int i = 0; i < hashmap->capacity; i++) {
    entry_slot = array_elem_at_i(&hashmap->entries, i, sizeof(HashmapEntry*));
    entry = *entry_slot;
    int entry_count = 0;
    if (entry) {
      while (entry) {
        entry_count += 1;
        entry = entry->next_entry;
      }
    }
    printf("[%d] -> %d\n", i, entry_count);
  }
}

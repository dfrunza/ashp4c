#include <memory.h>  /* memset */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>  /* exit */
#include <stdarg.h>  /* va_list, va_start, va_end */
#include <math.h>  /* floor, ceil, log10 */
#include "foundation.h"

static const uint32_t P = 257, Q = 4294967029;
static const uint32_t SIGMA = 2654435769;

static uint32_t
hash_string(char* string, uint32_t m)
{
  assert(m > 0 && m <= 32);
  uint32_t K = 0;
  for (uint8_t* s = (uint8_t*)string; (*s); s++) {
    K = (P * K + (*s)) % Q;
  }
  uint64_t KxSigma = (uint64_t)K * (uint64_t)SIGMA;
  uint32_t h = ((uint32_t)KxSigma) >> (32 - m);  /* 0 <= h < 2^m */
  return h;
}

static uint32_t
hash_uint32(uint32_t i, uint32_t m)
{
  assert(m > 0 && m <= 32);
  uint32_t K = i;
  uint64_t KxSigma = (uint64_t)K * (uint64_t)SIGMA;
  uint32_t h = ((uint32_t)KxSigma) >> (32 - m);  /* 0 <= h < 2^m */
  return h;
}

static uint64_t
hash_uint64(uint64_t i, uint32_t m)
{
  assert(m > 0 && m <= 32);
  uint32_t upper_half = (uint32_t)(i >> 32);
  uint32_t lower_half = (uint32_t)(0x00000000ffffffffl & i);
  uint32_t K = upper_half ^ lower_half;
  uint64_t KxSigma = (uint64_t)K * (uint64_t)SIGMA;
  uint32_t h = ((uint32_t)KxSigma) >> (32 - m);  /* 0 <= h < 2^m */
  return h;
}

static void
hashmap_hash_key(enum HashmapKeyType key_type, /* in/out */ HashmapKey* key, int m, int capacity)
{
  if (key_type == HKEY_STRING) {
    key->h = hash_string(key->str_key, m) % capacity;
  } else if (key_type == HKEY_UINT32) {
    key->h = hash_uint32(key->u32_key, m) % capacity;
  } else if (key_type == HKEY_UINT64) {
    key->h = hash_uint64(key->u64_key, m) % capacity;
  } else assert(0);
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
  int segment_count = ceil_log2(max_capacity/16 + 1);
  Hashmap* hashmap = arena_malloc(storage, sizeof(Hashmap) + sizeof(HashmapEntry**) * segment_count);
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
  for (int j = 0; j < 16; j ++) {
    HashmapEntry** segment = hashmap->entries.segments[0];
    segment[j] = 0;
  }
}

static void
hashmap_grow(Hashmap* hashmap, Arena* storage, HashmapKey* key, enum HashmapKeyType key_type)
{
  int last_segment = floor_log2(hashmap->capacity/16 + 1);
  if (last_segment >= hashmap->entries.segment_count) {
    printf("\nMaximum capacity has been reached.\n");
    exit(1);
  }
  HashmapCursor it = {};
  hashmap_cursor_begin(&it);
  HashmapEntry* first_entry = hashmap_cursor_next_entry(&it, hashmap);
  HashmapEntry* last_entry = first_entry;
  int entry_count = first_entry ? 1 : 0;
  for (HashmapEntry* entry = hashmap_cursor_next_entry(&it, hashmap);
       entry != 0; entry = hashmap_cursor_next_entry(&it, hashmap)) {
    last_entry->next_entry = entry;
    last_entry = entry;
    entry_count += 1;
  }
  assert(entry_count == hashmap->entry_count);
  int segment_capacity = 16 * (1 << last_segment);
  hashmap->entries.segments[last_segment] = arena_malloc(storage, sizeof(HashmapEntry*) * segment_capacity);
  hashmap->capacity = 16 * ((1 << (last_segment + 1)) - 1);
  for (int i = 0; i <= last_segment; i++) {
    int segment_capacity = 16 * (1 << i);
    for (int j = 0; j < segment_capacity; j ++) {
      HashmapEntry** segment = hashmap->entries.segments[i];
      segment[j] = 0;
    }
  }
  for (HashmapEntry* entry = first_entry; entry != 0; ) {
    HashmapEntry* next_entry = entry->next_entry;
    hashmap_hash_key(key_type, &entry->key, 4 + (last_segment + 1), hashmap->capacity);
    HashmapEntry** entry_slot = array_elem_at_i(&hashmap->entries, entry->key.h, sizeof(HashmapEntry*));
    entry->next_entry = *entry_slot;
    *entry_slot = entry;
    entry = next_entry;
  }
  hashmap_hash_key(key_type, key, 4 + (last_segment + 1), hashmap->capacity);
}

HashmapEntry*
hashmap_lookup_entry(Hashmap* hashmap, HashmapKey* key, enum HashmapKeyType key_type)
{
  HashmapEntry** entry_slot = array_elem_at_i(&hashmap->entries, key->h, sizeof(HashmapEntry*));
  HashmapEntry* entry = *entry_slot;
  while (entry) {
    if (key_equal(key_type, &entry->key, key)) {
      break;
    }
    entry = entry->next_entry;
  }
  return entry;
}

void*
hashmap_lookup(Hashmap* hashmap, enum HashmapKeyType key_type, ...)
{
  int last_segment = floor_log2(hashmap->capacity/16);
  va_list args;
  va_start(args, key_type);
  HashmapKey key = {};
  if (key_type == HKEY_STRING) {
    key = (HashmapKey){ .str_key = va_arg(args, char*) };
    hashmap_hash_key(HKEY_STRING, &key, 4 + (last_segment + 1), hashmap->capacity);
  } else if (key_type == HKEY_UINT32) {
    key = (HashmapKey){ .u32_key = va_arg(args, uint32_t) };
    hashmap_hash_key(HKEY_UINT32, &key, 4 + (last_segment + 1), hashmap->capacity);
  } else if (key_type == HKEY_UINT64) {
    key = (HashmapKey){ .u64_key = va_arg(args, uint64_t) };
    hashmap_hash_key(HKEY_UINT64, &key, 4 + (last_segment + 1), hashmap->capacity);
  } else assert(0);
  va_end(args);
  HashmapEntry* entry = hashmap_lookup_entry(hashmap, &key, key_type);
  return entry ? entry->value : 0;
}

HashmapEntry*
hashmap_get_entry(Hashmap* hashmap, Arena* storage, int value_size,
        HashmapKey* key, enum HashmapKeyType key_type)
{
  assert(value_size > 0);
  HashmapEntry* entry = hashmap_lookup_entry(hashmap, key, key_type);
  if (entry) {
    return entry;
  }
  if (hashmap->entry_count >= hashmap->capacity) {
    hashmap_grow(hashmap, storage, key, key_type);
  }
  entry = arena_malloc(storage, sizeof(HashmapEntry) + value_size);
  entry->key = *key;
  HashmapEntry** elem_slot = array_elem_at_i(&hashmap->entries, key->h, sizeof(HashmapEntry*));
  entry->next_entry = *elem_slot;
  *elem_slot = entry;
  hashmap->entry_count += 1;
  return entry;
}

void*
hashmap_get(Hashmap* hashmap, Arena* storage, int value_size, enum HashmapKeyType key_type, ...)
{
  assert(value_size > 0);
  int last_segment = floor_log2(hashmap->capacity/16);
  va_list args;
  va_start(args, key_type);
  HashmapKey key = {};
  if (key_type == HKEY_STRING) {
    key = (HashmapKey){ .str_key = va_arg(args, char*) };
    hashmap_hash_key(HKEY_STRING, &key, 4 + (last_segment + 1), hashmap->capacity);
  } else if (key_type == HKEY_UINT32) {
    key = (HashmapKey){ .u32_key = va_arg(args, uint32_t) };
    hashmap_hash_key(HKEY_UINT32, &key, 4 + (last_segment + 1), hashmap->capacity);
  } else if (key_type == HKEY_UINT64) {
    key = (HashmapKey){ .u64_key = va_arg(args, uint64_t) };
    hashmap_hash_key(HKEY_UINT64, &key, 4 + (last_segment + 1), hashmap->capacity);
  } else assert(0);
  va_end(args);
  HashmapEntry* entry = hashmap_get_entry(hashmap, storage, value_size, &key, key_type);
  return entry->value;
}

void
hashmap_set(Hashmap* hashmap, Arena* storage, void* value, int value_size, enum HashmapKeyType key_type, ...)
{
  assert(value_size > 0);
  int last_segment = floor_log2(hashmap->capacity/16);
  va_list args;
  va_start(args, key_type);
  HashmapKey key = {};
  if (key_type == HKEY_STRING) {
    key = (HashmapKey){ .str_key = va_arg(args, char*) };
    hashmap_hash_key(HKEY_STRING, &key, 4 + (last_segment + 1), hashmap->capacity);
  } else if (key_type == HKEY_UINT32) {
    key = (HashmapKey){ .u32_key = va_arg(args, uint32_t) };
    hashmap_hash_key(HKEY_UINT32, &key, 4 + (last_segment + 1), hashmap->capacity);
  } else if (key_type == HKEY_UINT64) {
    key = (HashmapKey){ .u64_key = va_arg(args, uint64_t) };
    hashmap_hash_key(HKEY_UINT64, &key, 4 + (last_segment + 1), hashmap->capacity);
  } else assert(0);
  va_end(args);
  HashmapEntry* entry = hashmap_get_entry(hashmap, storage, value_size, &key, key_type);
  memcpy(entry->value, value, value_size);
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
  if (cursor->entry) {
    entry = cursor->entry->next_entry;
    if (entry) {
      cursor->entry = entry;
      return cursor->entry;
    }
  }
  cursor->i++;
  while (cursor->i < hashmap->capacity) {
    HashmapEntry** entry_slot = array_elem_at_i(&hashmap->entries, cursor->i, sizeof(HashmapEntry*));
    entry = *entry_slot;
    if (entry) {
      cursor->entry = entry;
      break;
    }
    cursor->i++;
  }
  return entry;
}

void*
hashmap_cursor_next(HashmapCursor* cursor, Hashmap* hashmap)
{
  HashmapEntry* entry = hashmap_cursor_next_entry(cursor, hashmap);
  return entry ? entry->value : 0;
}

void
Debug_hashmap_occupancy(Hashmap* hashmap)
{
  for (int i = 0; i < hashmap->capacity; i++) {
    HashmapEntry** entry_slot = array_elem_at_i(&hashmap->entries, i, sizeof(HashmapEntry*));
    HashmapEntry* entry = *entry_slot;
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

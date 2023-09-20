#include <memory.h>  /* memset */
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>  /* va_list, va_start, va_end */
#include "foundation.h"

static const uint32_t P = 257, Q = 4294967029;
static const uint32_t SIGMA = 2654435769;
static HashmapEntry*  NULL_ENTRY = 0;

static uint32_t
fold_string(char* string)
{
  uint32_t K = 0;
  for (uint8_t* s = (uint8_t*)string; (*s); s++) {
    K = (P * K + (*s)) % Q;
  }
  return K;
}

static uint32_t
fold_bytes(uint8_t* bytes, int length)
{
  uint32_t K = 0;
  for (int i = 0; i < length; i++) {
    K = (P * K + bytes[i]) % Q;
  }
  return K;
}

static uint64_t
fold_uint64(uint64_t i)
{
  uint32_t upper_half = (uint32_t)(i >> 32);
  uint32_t lower_half = (uint32_t)(0x00000000ffffffffl & i);
  uint32_t K = upper_half ^ lower_half;
  return K;
}

static uint32_t
multiply_hash(uint32_t K, uint32_t m)
{
  assert(m > 0 && m <= 32);
  uint64_t KxSigma = (uint64_t)K * (uint64_t)SIGMA;
  uint32_t h = ((uint32_t)KxSigma) >> (32 - m);  /* 0 <= h < 2^m */
  return h;
}

static uint32_t
hash_string(char* string, uint32_t m)
{
  assert(m > 0 && m <= 32);
  uint32_t h = fold_string(string);
  h = multiply_hash(h, m) % ((1 << m) - 1);  /* 0 <= h < 2^m - 1 */
  return h;
}

static uint32_t
hash_bytes(uint8_t* bytes, int length, uint32_t m)
{
  assert(m > 0 && m <= 32);
  uint32_t h = fold_bytes(bytes, length);
  h = multiply_hash(h, m) % ((1 << m) - 1);  /* 0 <= h < 2^m - 1 */
  return h;
}

static uint32_t
hash_uint32(uint32_t i, uint32_t m)
{
  assert(m > 0 && m <= 32);
  uint32_t h = i;
  h = multiply_hash(h, m) % ((1 << m) - 1);  /* 0 <= h < 2^m - 1 */
  return h;
}

static uint64_t
hash_uint64(uint64_t i, uint32_t m)
{
  assert(m > 0 && m <= 32);
  uint32_t h = fold_uint64(i);
  h = multiply_hash(h, m) % ((1 << m) - 1);  /* 0 <= h < 2^m - 1 */
  return h;
}

void
hashmap_hash_key(enum HashmapKeyType key_type, /* in/out */ HashmapKey* key, int length_log2)
{
  if (key_type == HKEY_STRING) {
    key->h = hash_string(key->str_key, length_log2);
  } else if (key_type == HKEY_BYTES) {
    assert(key->keylen > 0);
    key->h = hash_bytes(key->bytes_key, key->keylen, length_log2);
  } else if (key_type == HKEY_UINT32) {
    key->h = hash_uint32(key->u32_key, length_log2);
  } else if (key_type == HKEY_UINT64) {
    key->h = hash_uint64(key->u64_key, length_log2);
  } else assert(0);
}

static bool
key_equal(enum HashmapKeyType key_type, HashmapKey* key_A, HashmapKey* key_B)
{
  if (key_type == HKEY_STRING) {
    return cstr_match(key_A->str_key, key_B->str_key);
  } else if (key_type == HKEY_BYTES) {
    assert((key_A->keylen > 0) && (key_B->keylen > 0));
    bool result = (key_A->keylen == key_B->keylen);
    if (!result) {
      return result;
    }
    uint8_t *p_a = key_A->bytes_key,
            *p_b = key_B->bytes_key;
    int at_i = 0;
    while (*p_a == *p_b) {
      p_a++;
      p_b++;
      if (++at_i == key_A->keylen) {
        break;
      }
    }
    result = (at_i == key_A->keylen);
    return result;
  } else if (key_type == HKEY_UINT32) {
    return key_A->u32_key == key_B->u32_key;
  } else if (key_type == HKEY_UINT64) {
    return key_A->u64_key == key_B->u64_key;
  } else assert(0);
  return false;
}

void
hashmap_create(Hashmap* hashmap, Arena* storage, enum HashmapKeyType key_type, int value_size,
               int capacity, int max_capacity)
{
  assert(max_capacity >= 7 && capacity <= max_capacity);
  assert(value_size >= sizeof(void*));
  hashmap->capacity_log2 = ceil_log2(capacity + 1);
  hashmap->capacity = (1 << hashmap->capacity_log2) - 1;
  /*hashmap->key_type = key_type;*/
  hashmap->value_size = value_size;
  hashmap->entry_count = 0;
  array_create(&hashmap->entries, storage, sizeof(HashmapEntry*), max_capacity);
  for (int i = 0; i < hashmap->capacity; i++) {
    array_append(&hashmap->entries, storage, &NULL_ENTRY);
  }
}

static void
hashmap_grow(Hashmap* hashmap, Arena* storage, HashmapKey* key, enum HashmapKeyType key_type)
{
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
  hashmap->capacity = (1 << ++hashmap->capacity_log2) - 1;
  for (int i = hashmap->entry_count; i < hashmap->capacity; i++) {
    array_append(&hashmap->entries, storage, &NULL_ENTRY);
  }
  for (int i = 0; i < hashmap->capacity; i++) {
    array_set(&hashmap->entries, i, &NULL_ENTRY);
  }
  for (HashmapEntry* entry = first_entry; entry != 0; ) {
    HashmapEntry* next_entry = entry->next_entry;
    hashmap_hash_key(key_type, &entry->key, hashmap->capacity_log2);
    entry->next_entry = *(HashmapEntry**)array_get(&hashmap->entries, entry->key.h);
    array_set(&hashmap->entries, entry->key.h, &entry);
    entry = next_entry;
  }
  hashmap_hash_key(key_type, key, hashmap->capacity_log2);
}

HashmapEntry*
hashmap_lookup_entry(Hashmap* hashmap, HashmapKey* key, enum HashmapKeyType key_type)
{
  HashmapEntry* entry = *(HashmapEntry**)array_get(&hashmap->entries, key->h);
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
  va_list args;
  va_start(args, key_type);
  HashmapKey key = {};
  if (key_type == HKEY_STRING) {
    key = (HashmapKey){ .str_key = va_arg(args, char*) };
    hashmap_hash_key(HKEY_STRING, &key, hashmap->capacity_log2);
  } else if (key_type == HKEY_BYTES) {
    key = (HashmapKey){ .bytes_key = va_arg(args, uint8_t*),
                        .keylen = va_arg(args, int) };
    hashmap_hash_key(HKEY_BYTES, &key, hashmap->capacity_log2);
  } else if (key_type == HKEY_UINT32) {
    key = (HashmapKey){ .u32_key = va_arg(args, uint32_t) };
    hashmap_hash_key(HKEY_UINT32, &key, hashmap->capacity_log2);
  } else if (key_type == HKEY_UINT64) {
    key = (HashmapKey){ .u64_key = va_arg(args, uint64_t) };
    hashmap_hash_key(HKEY_UINT64, &key, hashmap->capacity_log2);
  } else assert(0);
  va_end(args);
  HashmapEntry* entry = hashmap_lookup_entry(hashmap, &key, key_type);
  return entry ? entry->value : 0;
}

HashmapEntry*
hashmap_get_entry(Hashmap* hashmap, Arena* storage, HashmapKey* key, enum HashmapKeyType key_type)
{
  HashmapEntry* entry = hashmap_lookup_entry(hashmap, key, key_type);
  if (entry) {
    return entry;
  }
  if (hashmap->entry_count >= hashmap->capacity) {
    hashmap_grow(hashmap, storage, key, key_type);
  }
  entry = arena_malloc(storage, sizeof(HashmapEntry) + hashmap->value_size);
  entry->key = *key;
  entry->next_entry = *(HashmapEntry**)array_get(&hashmap->entries, key->h);
  array_set(&hashmap->entries, key->h, &entry);
  hashmap->entry_count += 1;
  return entry;
}

void*
hashmap_get(Hashmap* hashmap, Arena* storage, enum HashmapKeyType key_type, ...)
{
  va_list args;
  va_start(args, key_type);
  HashmapKey key = {};
  if (key_type == HKEY_STRING) {
    key = (HashmapKey){ .str_key = va_arg(args, char*) };
    hashmap_hash_key(HKEY_STRING, &key, hashmap->capacity_log2);
  } else if (key_type == HKEY_BYTES) {
    key = (HashmapKey){ .bytes_key = va_arg(args, uint8_t*),
                        .keylen = va_arg(args, int) };
    hashmap_hash_key(HKEY_BYTES, &key, hashmap->capacity_log2);
  } else if (key_type == HKEY_UINT32) {
    key = (HashmapKey){ .u32_key = va_arg(args, uint32_t) };
    hashmap_hash_key(HKEY_UINT32, &key, hashmap->capacity_log2);
  } else if (key_type == HKEY_UINT64) {
    key = (HashmapKey){ .u64_key = va_arg(args, uint64_t) };
    hashmap_hash_key(HKEY_UINT64, &key, hashmap->capacity_log2);
  } else assert(0);
  va_end(args);
  HashmapEntry* entry = hashmap_get_entry(hashmap, storage, &key, key_type);
  return entry->value;
}

void
hashmap_set(Hashmap* hashmap, Arena* storage, void* value, enum HashmapKeyType key_type, ...)
{
  va_list args;
  va_start(args, key_type);
  HashmapKey key = {};
  if (key_type == HKEY_STRING) {
    key = (HashmapKey){ .str_key = va_arg(args, char*) };
    hashmap_hash_key(HKEY_STRING, &key, hashmap->capacity_log2);
  } else if (key_type == HKEY_BYTES) {
    key = (HashmapKey){ .bytes_key = va_arg(args, uint8_t*),
                        .keylen = va_arg(args, int) };
    hashmap_hash_key(HKEY_BYTES, &key, hashmap->capacity_log2);
  } else if (key_type == HKEY_UINT32) {
    key = (HashmapKey){ .u32_key = va_arg(args, uint32_t) };
    hashmap_hash_key(HKEY_UINT32, &key, hashmap->capacity_log2);
  } else if (key_type == HKEY_UINT64) {
    key = (HashmapKey){ .u64_key = va_arg(args, uint64_t) };
    hashmap_hash_key(HKEY_UINT64, &key, hashmap->capacity_log2);
  } else assert(0);
  va_end(args);
  HashmapEntry* entry = hashmap_get_entry(hashmap, storage, &key, key_type);
  memcpy(entry->value, value, hashmap->value_size);
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
  while (cursor->i < hashmap->entries.elem_count) {
    entry = *(HashmapEntry**)array_get(&hashmap->entries, cursor->i);
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
    HashmapEntry* entry = *(HashmapEntry**)array_get(&hashmap->entries, i);
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

#include "hashmap.h"
#include <memory.h>  // memset
#include <math.h>


static const uint32_t P = 257, Q = 4294967029;
static const uint32_t SIGMA = 2654435769;
static struct HashmapEntry* null_entry = 0;


internal uint32_t
fold_string(uint8_t* string)
{
  uint32_t K = 0;
  uint8_t* s;
  for(s = string; (*s); s++) {
    K = (P * K + (*s)) % Q;
  }
  return K;
}

internal uint32_t
fold_bytes(uint8_t* bytes, int length)
{
  uint32_t K = 0;
  if (length <= sizeof(uint32_t)) {
    K = *(uint32_t*)bytes;
  } else {
    uint8_t* b;
    int i;
    for(i = 0; i < length; i++) {
      K = (P * K + bytes[i]) % Q;
    }
  }
  return K;
}

internal uint32_t
multiply_hash(uint32_t K, uint32_t m)
{
  uint64_t Ksigma = (uint64_t)K * (uint64_t)SIGMA;
  uint32_t h = ((uint32_t)Ksigma) >> (32 - m);  // 0 <= h < 2^{m}
  return h;
}

uint32_t
hash_string(uint8_t* string, uint32_t m)
{
  uint32_t K = fold_string(string);
  uint32_t h = multiply_hash(K, m) % ((1 << m) - 1);  // 0 <= h < 2^{m} - 1
  return h;
}

uint32_t
hash_bytes(uint8_t* bytes, int length, uint32_t m)
{
  uint32_t K = fold_bytes(bytes, length);
  uint32_t h = multiply_hash(K, m) % ((1 << m) - 1);  // 0 <= h < 2^{m} - 1
  return h;
}

uint32_t
hashmap_hash_key(int capacity_log2, uint8_t* key, int keylen)
{
  uint32_t h = 0;
  if (keylen == 0) {
    h = hash_string(key, capacity_log2);
  } else {
    h = hash_bytes(key, keylen, capacity_log2);
  }
  return h;
}

internal bool
key_match(uint8_t* bytes_a, int len_a, uint8_t* bytes_b, int len_b)
{
  if (len_a == 0 && len_b == 0) {
    return cstr_match(bytes_a, bytes_b);
  }
  assert ((len_a > 0) && (len_b > 0));
  bool result = (len_a == len_b);
  if (!result) {
    return result;
  }
  uint8_t *p_a = bytes_a,
          *p_b = bytes_b;
  int at_i = 0;
  while (*p_a == *p_b) {
    p_a++;
    p_b++;
    if (++at_i == len_a) {
      break;
    }
  }
  result = (at_i == len_a);
  return result;
}


void
hashmap_init(struct Hashmap* hashmap, int capacity_log2, struct Arena* storage)
{
  array_init(&hashmap->entries, sizeof(null_entry), storage);
  hashmap->capacity = (1 << capacity_log2) - 1;
  hashmap->entry_count = 0;
  {int i;
  for (i = 0; i < hashmap->capacity; i++) {
    array_append(&hashmap->entries, &null_entry);
  }}
  hashmap->capacity_log2 = capacity_log2;
}

struct HashmapEntry*
hashmap_find_entry(struct Hashmap* hashmap, uint8_t* key, int keylen)
{
  assert (keylen >= 0);
  uint32_t h = hashmap_hash_key(hashmap->capacity_log2, key, keylen);
  struct HashmapEntry* entry = *(struct HashmapEntry**)array_get(&hashmap->entries, h);
  while (entry) {
    if (key_match(entry->key, entry->keylen, key, keylen))
      break;
    entry = entry->next_entry;
  }
  return entry;
}

struct HashmapEntry*
hashmap_get_or_create_entry(struct Hashmap* hashmap, uint8_t* key, int keylen)
{
  struct HashmapEntry* entry = hashmap_find_entry(hashmap, key, keylen);
  if (entry) {
    return entry;
  }
  if (hashmap->entry_count >= hashmap->capacity) {
    struct Arena temp_storage = {};
    struct HashmapEntry** entries_array = arena_push(&temp_storage, hashmap->capacity);
    int i, j = 0;
    for (i = 0; i < hashmap->capacity; i++) {
      struct HashmapEntry* entry = *(struct HashmapEntry**)array_get(&hashmap->entries, i);
      while (entry) {
        entries_array[j] = entry;
        struct HashmapEntry* next_entry = entry->next_entry;
        entry->next_entry = 0;
        entry = next_entry;
        j++;
      }
    }
    assert (j == hashmap->entry_count);
    hashmap->capacity = (1 << ++hashmap->capacity_log2) - 1;
    for (i = hashmap->entry_count; i < hashmap->capacity; i++) {
      array_append(&hashmap->entries, &null_entry);
    }
    for (i = 0; i < hashmap->capacity; i++) {
      array_set(&hashmap->entries, i, &null_entry);
    }
    for (i = 0; i < hashmap->entry_count; i++) {
      uint32_t h = hashmap_hash_key(hashmap->capacity_log2, entries_array[i]->key, entries_array[i]->keylen);
      entries_array[i]->next_entry = *(struct HashmapEntry**)array_get(&hashmap->entries, h);
      array_set(&hashmap->entries, h, &entries_array[i]);
    }
    arena_delete(&temp_storage);
  }
  int h = hashmap_hash_key(hashmap->capacity_log2, key, keylen);
  entry = arena_push(hashmap->entries.storage, sizeof(*entry));
  memset(entry, 0, sizeof(*entry));
  entry->key = key;
  entry->keylen = keylen;
  entry->next_entry = *(struct HashmapEntry**)array_get(&hashmap->entries, h);
  array_set(&hashmap->entries, h, &entry);
  hashmap->entry_count += 1;
  return entry;
}


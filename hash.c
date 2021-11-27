#include "hash.h"
#include <memory.h>  // memset
#include <math.h>


static const uint32_t P = 257, Q = 4294967029;
static const uint32_t SIGMA = 2654435769;
static struct HashtableEntry* null_entry = 0;


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
hashtable_hash_key(int capacity_log2, uint8_t* key, int keylen)
{
  uint32_t h = 0;
  if (keylen == 0) {
    h = hash_string(key, capacity_log2);
  } else {
    h = hash_bytes(key, keylen, capacity_log2);
  }
  return h;
}

void
hashtable_init(struct Hashtable* hashtable, int capacity_log2, struct Arena* storage)
{
  array_init(&hashtable->entries, sizeof(null_entry), storage);
  hashtable->capacity = (1 << capacity_log2) - 1;
  hashtable->entry_count = 0;
  int i;
  for (i = hashtable->entry_count; i < hashtable->capacity; i++) {
    array_append(&hashtable->entries, &null_entry);
  }
  hashtable->capacity_log2 = capacity_log2;
}

struct HashtableEntry*
hashtable_find_entry(struct Hashtable* hashtable, uint8_t* key, int keylen)
{
  assert (keylen >= 0);
  uint32_t h = hashtable_hash_key(hashtable->capacity_log2, key, keylen);
  struct HashtableEntry* entry = *(struct HashtableEntry**)array_get(&hashtable->entries, h);
  while (entry) {
    if (bytes_match(entry->key, entry->keylen, key, keylen))
      break;
    entry = entry->next_entry;
  }
  return entry;
}

struct HashtableEntry*
hashtable_get_or_create_entry(struct Hashtable* hashtable, uint8_t* key, int keylen)
{
  struct HashtableEntry* entry = hashtable_find_entry(hashtable, key, keylen);
  if (entry) {
    return entry;
  }
  if (hashtable->entry_count >= hashtable->capacity) {
    struct Arena temp_storage = {};
    struct HashtableEntry** entries_array = arena_push(&temp_storage, hashtable->capacity);
    int i, j = 0;
    for (i = 0; i < hashtable->capacity; i++) {
      struct HashtableEntry* entry = *(struct HashtableEntry**)array_get(&hashtable->entries, i);
      while (entry) {
        entries_array[j] = entry;
        struct HashtableEntry* next_entry = entry->next_entry;
        entry->next_entry = 0;
        entry = next_entry;
        j++;
      }
    }
    assert (j == hashtable->entry_count);
    hashtable->capacity = (1 << ++hashtable->capacity_log2) - 1;
    for (i = hashtable->entry_count; i < hashtable->capacity; i++) {
      array_append(&hashtable->entries, &null_entry);
    }
    for (i = 0; i < hashtable->capacity; i++) {
      array_set(&hashtable->entries, i, &null_entry);
    }
    for (i = 0; i < hashtable->entry_count; i++) {
      uint32_t h = hashtable_hash_key(hashtable->capacity_log2, entries_array[i]->key, entries_array[i]->keylen);
      entries_array[i]->next_entry = *(struct HashtableEntry**)array_get(&hashtable->entries, h);
      array_set(&hashtable->entries, h, &entries_array[i]);
    }
    arena_delete(&temp_storage);
  }
  int h = hashtable_hash_key(hashtable->capacity_log2, key, keylen);
  entry = arena_push(hashtable->entries.storage, sizeof(*entry));
  memset(entry, 0, sizeof(*entry));
  entry->key = key;
  entry->keylen = keylen;
  entry->next_entry = *(struct HashtableEntry**)array_get(&hashtable->entries, h);
  array_set(&hashtable->entries, h, &entry);
  hashtable->entry_count += 1;
  return entry;
}


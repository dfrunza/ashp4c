#include <memory.h>  // memset
#include <stdint.h>
#include "arena.h"

internal const uint32_t P = 257, Q = 4294967029;
internal const uint32_t SIGMA = 2654435769;
internal HashmapEntry* NULL_ENTRY = 0;

internal uint32_t
fold_string(uint8_t* string)
{
  uint32_t K = 0;
  for (uint8_t* s = string; (*s); s++) {
    K = (P * K + (*s)) % Q;
  }
  return K;
}

internal uint32_t
fold_bytes(uint8_t* bytes, int length)
{
  uint32_t K = 0;
  for (int i = 0; i < length; i++) {
    K = (P * K + bytes[i]) % Q;
  }
  return K;
}

internal uint32_t
multiply_hash(uint32_t K, uint32_t m)
{
  assert(m > 0 && m <= 32);
  uint64_t Ksigma = (uint64_t)K * (uint64_t)SIGMA;
  uint32_t h = ((uint32_t)Ksigma) >> (32 - m);  // 0 <= h < 2^{m}
  return h;
}

internal uint32_t
hash_string(uint8_t* string, uint32_t m)
{
  assert(m > 0 && m <= 32);
  uint32_t K = fold_string(string);
  uint32_t h = multiply_hash(K, m) % ((1 << m) - 1);  // 0 <= h < 2^{m} - 1
  return h;
}

internal uint32_t
hash_bytes(uint8_t* bytes, int length, uint32_t m)
{
  assert(m > 0 && m <= 32);
  uint32_t K = fold_bytes(bytes, length);
  uint32_t h = multiply_hash(K, m) % ((1 << m) - 1);  // 0 <= h < 2^{m} - 1
  return h;
}

internal uint32_t
hash_uint32(uint32_t i, uint32_t m)
{
  assert(m > 0 && m <= 32);
  uint32_t h = multiply_hash(i, m) % ((1 << m) - 1);  // 0 <= h < 2^{m} - 1
  return h;
}

void
hashmap_hash_key(enum HashmapKeyType key_type, /*in/out*/ HashmapKey* key, int capacity_log2)
{
  if (key_type == HASHMAP_KEY_STRING) {
    key->h = hash_string(key->str_key, capacity_log2);
  } else if (key_type == HASHMAP_KEY_BIT) {
    assert (key->keylen > 0);
    key->h = hash_bytes(key->bit_key, key->keylen, capacity_log2);
  } else if (key_type == HASHMAP_KEY_UINT32) {
    key->h = hash_uint32(key->int_key, capacity_log2);
  } else assert(0);
}

internal bool
key_equal(enum HashmapKeyType key_type, HashmapKey* key_A, HashmapKey* key_B)
{
  if (key_type == HASHMAP_KEY_STRING) {
    return cstr_match((char*)key_A->str_key, (char*)key_B->str_key);
  } else if (key_type == HASHMAP_KEY_BIT) {
    assert ((key_A->keylen > 0) && (key_B->keylen > 0));
    bool result = (key_A->keylen == key_B->keylen);
    if (!result) {
      return result;
    }
    uint8_t *p_a = key_A->bit_key,
            *p_b = key_B->bit_key;
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
  } else if (key_type == HASHMAP_KEY_UINT32) {
    return key_A->int_key == key_B->int_key;
  } else assert(0);
  return false;
}

void
hashmap_init(Hashmap* hashmap, enum HashmapKeyType key_type, int capacity_log2, Arena* storage)
{
  array_init(&hashmap->entries, sizeof(HashmapEntry*), storage);
  hashmap->key_type = key_type;
  hashmap->capacity = (1 << capacity_log2) - 1;
  hashmap->entry_count = 0;
  for (int i = 0; i < hashmap->capacity; i++) {
    array_append(&hashmap->entries, &NULL_ENTRY);
  }
  hashmap->capacity_log2 = capacity_log2;
}

HashmapEntry*
hashmap_get_entry(Hashmap* hashmap, HashmapKey* key)
{
  HashmapEntry* entry = *(HashmapEntry**)array_get(&hashmap->entries, key->h);
  while (entry) {
    if (key_equal(hashmap->key_type, &entry->key, key)) {
      break;
    }
    entry = entry->next_entry;
  }
  return entry;
}

HashmapEntry*
hashmap_create_entry(Hashmap* hashmap, HashmapKey* key)
{
  HashmapEntry* entry = hashmap_get_entry(hashmap, key);
  if (entry) {
    return entry;
  }
  if (hashmap->entry_count >= hashmap->capacity) {
    HashmapCursor entry_it = {};
    hashmap_cursor_init(&entry_it, hashmap);
    HashmapEntry* first_entry = hashmap_move_cursor(&entry_it);
    HashmapEntry* last_entry = first_entry;
    int entry_count = first_entry ? 1 : 0;
    for (HashmapEntry* entry = hashmap_move_cursor(&entry_it);
         entry != 0; entry = hashmap_move_cursor(&entry_it)) {
      last_entry->next_entry = entry;
      last_entry = entry;
      entry_count += 1;
    }
    assert (entry_count == hashmap->entry_count);
    hashmap->capacity = (1 << ++hashmap->capacity_log2) - 1;
    for (int i = hashmap->entry_count; i < hashmap->capacity; i++) {
      array_append(&hashmap->entries, &NULL_ENTRY);
    }
    for (int i = 0; i < hashmap->capacity; i++) {
      array_set(&hashmap->entries, i, &NULL_ENTRY);
    }
    for (HashmapEntry* entry = first_entry; entry != 0;) {
      HashmapEntry* next_entry = entry->next_entry;
      hashmap_hash_key(hashmap->key_type, &entry->key, hashmap->capacity_log2);
      entry->next_entry = *(HashmapEntry**)array_get(&hashmap->entries, entry->key.h);
      array_set(&hashmap->entries, entry->key.h, &entry);
      entry = next_entry;
    }
    hashmap_hash_key(hashmap->key_type, key, hashmap->capacity_log2);
  }
  entry = arena_push_struct(hashmap->entries.storage, HashmapEntry);
  entry->key = *key;
  entry->next_entry = *(HashmapEntry**)array_get(&hashmap->entries, key->h);
  array_set(&hashmap->entries, key->h, &entry);
  hashmap->entry_count += 1;
  return entry;
}

HashmapEntry*
hashmap_create_entry_uint32(Hashmap* map, uint32_t int_key)
{
  assert(map->key_type == HASHMAP_KEY_UINT32);
  HashmapKey key = { .int_key = int_key };
  hashmap_hash_key(HASHMAP_KEY_UINT32, &key, map->capacity_log2);
  HashmapEntry* he = hashmap_create_entry(map, &key);
  return he;
}

HashmapEntry*
hashmap_get_entry_uint32(Hashmap* map, uint32_t int_key)
{
  assert(map->key_type == HASHMAP_KEY_UINT32);
  HashmapKey key = { .int_key = int_key };
  hashmap_hash_key(HASHMAP_KEY_UINT32, &key, map->capacity_log2);
  HashmapEntry* he = hashmap_get_entry(map, &key);
  return he;
}

void
hashmap_cursor_init(HashmapCursor* it, Hashmap* hashmap)
{
  it->hashmap = hashmap;
  it->i = -1;
  it->entry = 0;
}

HashmapEntry*
hashmap_move_cursor(HashmapCursor* it)
{
  HashmapEntry* next_entry = 0;
  if (it->entry) {
    next_entry = it->entry->next_entry;
    if (next_entry) {
      it->entry = next_entry;
      return it->entry;
    }
  }
  it->i++;
  while (it->i < it->hashmap->entries.elem_count) {
    next_entry = *(HashmapEntry**)array_get(&it->hashmap->entries, it->i);
    if (next_entry) {
      it->entry = next_entry;
      break;
    }
    it->i++;
  }
  return next_entry;
}


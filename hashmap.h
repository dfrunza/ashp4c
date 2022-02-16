#pragma once
#include "basic.h"
#include "arena.h"


enum HashmapKeyType {
  HASHMAP_KEY_NONE,
  HASHMAP_KEY_STRING,
  HASHMAP_KEY_BLOB,
  HASHMAP_KEY_INT,
};

struct Hashmap {
  struct UnboundedArray entries;
  enum HashmapKeyType key_type;
  int capacity_log2;
  int capacity;
  int entry_count;
};

struct HashmapKey {
  union {
    uint8_t* s_key;
    uint8_t* b_key;
    uint8_t i_key;
  };
  int keylen;
};

struct HashmapEntry {
  struct HashmapKey key;
  void* object;
  struct HashmapEntry* next_entry;
};

struct HashmapEntryIterator {
  struct Hashmap* hashmap;
  int i;
  struct HashmapEntry* entry;
};


void hashmap_init(struct Hashmap* hashmap, enum HashmapKeyType type, int capacity_log2, struct Arena* storage);
struct HashmapEntry* hashmap_get_or_create_entry(struct Hashmap* hashmap, struct HashmapKey key);
struct HashmapEntry* hashmap_get_entry(struct Hashmap* hashmap, struct HashmapKey key);
void hashmap_iter_init(struct HashmapEntryIterator* it, struct Hashmap* hashmap);
struct HashmapEntry* hashmap_iter_next(struct HashmapEntryIterator* it);


#pragma once
#include "basic.h"
#include "arena.h"


struct Hashmap {
  struct UnboundedArray entries;
  int capacity_log2;
  int capacity;
  int entry_count;
};

struct HashmapEntry {
  uint8_t* key;
  int keylen;
  void* object;
  struct HashmapEntry* next_entry;
};


uint32_t hash_string(uint8_t* string, uint32_t m);
uint32_t hash_bytes(uint8_t* bytes, int length, uint32_t m);

struct HashmapEntry* hashmap_find_entry(struct Hashmap* hashmap, uint8_t* key, int keylen);
struct HashmapEntry* hashmap_get_or_create_entry(struct Hashmap* hashmap, uint8_t* key, int keylen);


#pragma once
#include "basic.h"
#include "arena.h"


struct Hashtable {
  struct UnboundedArray entries;
  int capacity_log2;
  int capacity;
  int entry_count;
};

struct HashtableEntry {
  uint8_t* key;
  int keylen;
  void* object;
  struct HashtableEntry* next_entry;
};


uint32_t hash_string(uint8_t* string, uint32_t m);
uint32_t hash_bytes(uint8_t* bytes, int length, uint32_t m);

struct HashtableEntry* hashtable_find_entry(struct Hashtable* hashtable, uint8_t* key, int keylen);
struct HashtableEntry* hashtable_get_or_create_entry(struct Hashtable* hashtable, uint8_t* key, int keylen);


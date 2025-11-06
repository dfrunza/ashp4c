#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include "foundation.h"

static const uint32_t P = 257, Q = 4294967029;
static const uint32_t SIGMA = 2654435769;

static uint32_t hash_string(char* string, uint32_t m)
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

static uint32_t hash_key(char* key, int m, int capacity)
{
  uint32_t h;
  h = hash_string(key, m) % capacity;
  return h;
}

Strmap* Strmap::create(Arena* storage, int segment_count)
{
  assert(segment_count >= 1 && segment_count <= 16);
  Strmap* strmap;

  strmap = (Strmap*)arena_malloc(storage, sizeof(Strmap) + sizeof(StrmapEntry**) * segment_count);
  strmap->storage = storage;
  strmap->init(strmap->storage, segment_count);
  return strmap;
}

void Strmap::init(Arena* storage, int segment_count)
{
  assert(segment_count >= 1);

  this->storage = storage;
  this->entry_count = 0;
  this->capacity = 16;
  this->entries.segment_count = segment_count;
  this->entries.segments[0] = arena_malloc(this->storage, sizeof(StrmapEntry*) * 16);
  memset(this->entries.segments[0], 0, sizeof(StrmapEntry*) * 16);
}

static void strmap_grow(Strmap* strmap)
{
  int last_segment;
  StrmapCursor it = {0};
  StrmapEntry* first_entry, *last_entry;
  StrmapEntry* entry, *next_entry;
  StrmapEntry** segment, **entry_slot;
  int entry_count;
  int segment_capacity; 
  uint32_t h;

  last_segment = floor(log2(strmap->capacity/16 + 1));
  if (last_segment >= strmap->entries.segment_count) {
    printf("\nMaximum capacity has been reached.\n");
    exit(1);
  }
  it.begin(strmap);
  first_entry = it.next();
  last_entry = first_entry;
  entry_count = first_entry ? 1 : 0;
  for (entry = it.next();
       entry != 0; entry = it.next()) {
    last_entry->next_entry = entry;
    last_entry = entry;
    entry_count += 1;
  }
  assert(entry_count == strmap->entry_count);
  segment_capacity = 16 * (1 << last_segment);
  strmap->entries.segments[last_segment] = arena_malloc(strmap->storage, sizeof(StrmapEntry*) * segment_capacity);
  strmap->capacity = 16 * ((1 << (last_segment + 1)) - 1);
  for (int i = 0; i <= last_segment; i++) {
    segment_capacity = 16 * (1 << i);
    for (int j = 0; j < segment_capacity; j ++) {
      segment = (StrmapEntry**)strmap->entries.segments[i];
      segment[j] = 0;
    }
  }
  for (entry = first_entry; entry != 0; ) {
    next_entry = entry->next_entry;
    h = hash_key(entry->key, 4 + (last_segment + 1), strmap->capacity);
    entry_slot = (StrmapEntry**)segment_locate_cell(&strmap->entries, h, sizeof(StrmapEntry*));
    entry->next_entry = *entry_slot;
    *entry_slot = entry;
    entry = next_entry;
  }
}

void* Strmap::lookup(char* key, StrmapEntry** entry_/*out*/, StrmapBucket* bucket/*out*/)
{
  int last_segment;
  StrmapEntry** entry_slot, *entry;
  uint32_t h;

  last_segment = floor(log2(this->capacity/16));
  h = hash_key(key, 4 + (last_segment + 1), this->capacity);
  entry_slot = (StrmapEntry**)segment_locate_cell(&this->entries, h, sizeof(StrmapEntry*));
  entry = *entry_slot;
  while (entry) {
    if (cstr_match(entry->key, key)) {
      break;
    }
    entry = entry->next_entry;
  }
  if (entry_) { *entry_ = entry; }
  if (bucket) {
    bucket->h = h;
    bucket->entry_slot = entry_slot;
    bucket->last_segment = last_segment;
  }
  if (entry) { return entry->value; }
  return 0;
}

StrmapEntry* Strmap::insert(char* key, void* value, bool return_if_found)
{
  StrmapEntry* entry;
  StrmapBucket bucket = {0};
 
  this->lookup(key, &entry, &bucket);
  if (entry) {
    if (return_if_found) { return entry; } else { return 0; }
  }

  if (this->entry_count >= this->capacity) {
    strmap_grow(this);
    bucket.last_segment = floor(log2(this->capacity/16));
    bucket.h = hash_key(key, 4 + (bucket.last_segment + 1), this->capacity);
    bucket.entry_slot = (StrmapEntry**)segment_locate_cell(&this->entries, bucket.h, sizeof(StrmapEntry*));
  }
  entry = (StrmapEntry*)arena_malloc(this->storage, sizeof(StrmapEntry));
  entry->key = key;
  entry->value = value;
  entry->next_entry = *bucket.entry_slot;
  *bucket.entry_slot = entry;
  this->entry_count += 1;
  return entry;
}

void StrmapCursor::begin(Strmap* strmap)
{
  this->strmap = strmap;
  this->i = -1;
  this->entry = 0;
}

StrmapEntry* StrmapCursor::next()
{
  Strmap* strmap;
  StrmapEntry* entry = 0;
  StrmapEntry** entry_slot;

  strmap = this->strmap;
  entry = this->entry;
  if (entry) {
    entry = entry->next_entry;
    if (entry) {
      this->entry = entry;
      return this->entry;
    }
  }
  this->i++;
  while (this->i < strmap->capacity) {
    entry_slot = (StrmapEntry**)segment_locate_cell(&strmap->entries, this->i, sizeof(StrmapEntry*));
    entry = *entry_slot;
    if (entry) {
      this->entry = entry;
      break;
    }
    this->i++;
  }
  return entry;
}

void Debug_strmap_occupancy(Strmap* strmap)
{
  StrmapEntry** entry_slot;
  StrmapEntry* entry;
  int empty_buckets = 0;
  int total_entry_count = 0,
      entry_count = 0,
      max_bucket_length = 0;

  for (int i = 0; i < strmap->capacity; i++) {
    entry_slot = (StrmapEntry**)segment_locate_cell(&strmap->entries, i, sizeof(StrmapEntry*));
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

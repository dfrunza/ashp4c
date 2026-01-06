#include <memory.h>
#include "adt/strmap.h"
#include "adt/cstring.h"

static const uint32_t P = 257, Q = 4294967029;
static const uint32_t SIGMA = 2654435769;

static uint32_t hash_string(char* string, uint32_t m)
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

static uint32_t hash_key(char* key, int m, int capacity)
{
  uint32_t h = hash_string(key, m) % capacity;
  return h;
}

Strmap* Strmap::allocate(Arena* storage, int segment_count)
{
  assert(segment_count >= 1 && segment_count <= 16);

  Strmap* strmap = (Strmap*)storage->allocate(sizeof(Strmap), 1);
  storage->allocate(sizeof(StrmapEntry*), segment_count);
  strmap->storage = storage;
  strmap->entries.element_size = sizeof(StrmapEntry*);
  strmap->entries.segment_count = segment_count;
  strmap->entries.segments[0] = (StrmapEntry**)storage->allocate(sizeof(StrmapEntry*), 16);
  memset(strmap->entries.segments[0], 0, sizeof(StrmapEntry*) * 16);
  strmap->entry_count = 0;
  strmap->capacity = 16;
  return strmap;
}

void Strmap::grow()
{
  int last_segment = floor(log2(capacity/16 + 1));
  if (last_segment >= entries.segment_count) {
    printf("\nMaximum capacity has been reached.\n");
    exit(1);
  }
  StrmapIterator it(this);
  StrmapEntry* first_entry = it.next();
  StrmapEntry* last_entry = first_entry;
  int entry_count = first_entry ? 1 : 0;
  for (StrmapEntry* entry = it.next();
       entry != 0; entry = it.next()) {
    last_entry->next_entry = entry;
    last_entry = entry;
    entry_count += 1;
  }
  assert(entry_count == this->entry_count);
  int segment_capacity = 16 * (1 << last_segment);
  entries.segments[last_segment] = (StrmapEntry**)storage->allocate(sizeof(StrmapEntry*), segment_capacity);
  capacity = 16 * ((1 << (last_segment + 1)) - 1);
  for (int i = 0; i <= last_segment; i++) {
    segment_capacity = 16 * (1 << i);
    for (int j = 0; j < segment_capacity; j ++) {
      StrmapEntry** segment = (StrmapEntry**)entries.segments[i];
      segment[j] = 0;
    }
  }
  for (StrmapEntry* entry = first_entry; entry != 0; ) {
    StrmapEntry* next_entry = entry->next_entry;
    uint32_t h = hash_key(entry->key, 4 + (last_segment + 1), capacity);
    StrmapEntry** entry_slot = (StrmapEntry**) entries.locate(h);
    entry->next_entry = *entry_slot;
    *entry_slot = entry;
    entry = next_entry;
  }
}

void* Strmap::lookup(char* key, StrmapEntry** entry_/*out*/, StrmapBucket* bucket/*out*/)
{
  int last_segment = floor(log2(capacity/16));
  uint32_t h = hash_key(key, 4 + (last_segment + 1), capacity);
  StrmapEntry** entry_slot = (StrmapEntry**) entries.locate(h);
  StrmapEntry* entry = *entry_slot;
  while (entry) {
    if (cstring::match(entry->key, key)) {
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
  StrmapBucket bucket = {};

  lookup(key, &entry, &bucket);
  if (entry) {
    if (return_if_found) { return entry; } else { return 0; }
  }

  if (entry_count >= capacity) {
    grow();
    bucket.last_segment = floor(log2(capacity/16));
    bucket.h = hash_key(key, 4 + (bucket.last_segment + 1), capacity);
    bucket.entry_slot = (StrmapEntry**) entries.locate(bucket.h);
  }
  entry = (StrmapEntry*)storage->allocate(sizeof(StrmapEntry), 1);
  entry->key = key;
  entry->value = value;
  entry->next_entry = *bucket.entry_slot;
  *bucket.entry_slot = entry;
  entry_count += 1;
  return entry;
}

void Strmap::DEBUG_occupancy()
{
  int empty_buckets = 0;
  int total_entry_count = 0,
      entry_count = 0,
      max_bucket_length = 0;

  for (int i = 0; i < capacity; i++) {
    StrmapEntry** entry_slot = (StrmapEntry**) entries.locate(i);
    StrmapEntry* entry = *entry_slot;
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
      "Max. bucket length: %d\n", total_entry_count, empty_buckets, max_bucket_length
  );
}

StrmapIterator::StrmapIterator()
{
  strmap = 0;
  entry = 0;
  i = 0;
}

StrmapIterator::StrmapIterator(Strmap* strmap)
{
  begin(strmap);
}

void StrmapIterator::begin(Strmap* strmap)
{
  this->strmap = strmap;
  i = -1;
  entry = 0;
}

StrmapEntry* StrmapIterator::next()
{
  Strmap* strmap = this->strmap;
  StrmapEntry* entry = this->entry;
  if (entry) {
    entry = entry->next_entry;
    if (entry) {
      this->entry = entry;
      return this->entry;
    }
  }
  i++;
  while (i < strmap->capacity) {
    StrmapEntry** entry_slot = (StrmapEntry**) strmap->entries.locate(i);
    entry = *entry_slot;
    if (entry) {
      this->entry = entry;
      break;
    }
    i++;
  }
  return entry;
}

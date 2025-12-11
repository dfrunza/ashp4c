#pragma once
#include <stdint.h>
#include <memory.h>
#include <cstring.h>
#include <arena.h>
#include <array.h>

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

template<class V>
struct StrmapEntry {
  char* key;
  V* value;
  StrmapEntry* next_entry;
};

template<class V>
struct StrmapBucket {
  uint32_t h;
  StrmapEntry<V>** entry_slot;
  int last_segment;
};

template<class V> struct StrmapIterator;

template<class V>
struct Strmap {
  Arena* storage;
  int entry_count;
  int capacity;
  SegmentTable<StrmapEntry<V>*> entries;

  Strmap* create(Arena* storage, int segment_count)
  {
    assert(segment_count >= 1 && segment_count <= 16);

    Strmap* strmap = storage->allocate<Strmap>();
    storage->allocate<StrmapEntry<V>*>(segment_count);
    strmap->storage = storage;
    strmap->init(strmap->storage, segment_count);
    return strmap;
  }

  void init(Arena* storage, int segment_count)
  {
    assert(segment_count >= 1);

    this->storage = storage;
    entry_count = 0;
    capacity = 16;
    entries.segment_count = segment_count;
    entries.segments[0] = storage->allocate<StrmapEntry<V>*>(16);
    memset(entries.segments[0], 0, sizeof(StrmapEntry<V>*) * 16);
  }

  void grow()
  {
    int last_segment = floor(log2(capacity/16 + 1));
    if (last_segment >= entries.segment_count) {
      printf("\nMaximum capacity has been reached.\n");
      exit(1);
    }
    StrmapIterator<V> it = {};
    it.begin(this);
    StrmapEntry<V>* first_entry = it.next();
    StrmapEntry<V>* last_entry = first_entry;
    int entry_count = first_entry ? 1 : 0;
    for (StrmapEntry<V>* entry = it.next();
         entry != 0; entry = it.next()) {
      last_entry->next_entry = entry;
      last_entry = entry;
      entry_count += 1;
    }
    assert(entry_count == this->entry_count);
    int segment_capacity = 16 * (1 << last_segment);
    entries.segments[last_segment] = storage->allocate<StrmapEntry<V>*>(segment_capacity);
    capacity = 16 * ((1 << (last_segment + 1)) - 1);
    for (int i = 0; i <= last_segment; i++) {
      segment_capacity = 16 * (1 << i);
      for (int j = 0; j < segment_capacity; j ++) {
        StrmapEntry<V>** segment = entries.segments[i];
        segment[j] = 0;
      }
    }
    for (StrmapEntry<V>* entry = first_entry; entry != 0; ) {
      StrmapEntry<V>* next_entry = entry->next_entry;
      uint32_t h = hash_key(entry->key, 4 + (last_segment + 1), capacity);
      StrmapEntry<V>** entry_slot = entries.locate_cell(h);
      entry->next_entry = *entry_slot;
      *entry_slot = entry;
      entry = next_entry;
    }
  }

  V* lookup(char* key, StrmapEntry<V>** entry_/*out*/, StrmapBucket<V>* bucket/*out*/)
  {

    int last_segment = floor(log2(capacity/16));
    uint32_t h = hash_key(key, 4 + (last_segment + 1), capacity);
    StrmapEntry<V>** entry_slot = entries.locate_cell(h);
    StrmapEntry<V>*entry = *entry_slot;
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

  StrmapEntry<V>* insert(char* key, V* value, bool return_if_found)
  {
    StrmapEntry<V>* entry;
    StrmapBucket<V> bucket = {};

    lookup(key, &entry, &bucket);
    if (entry) {
      if (return_if_found) { return entry; } else { return 0; }
    }

    if (entry_count >= capacity) {
      grow();
      bucket.last_segment = floor(log2(capacity/16));
      bucket.h = hash_key(key, 4 + (bucket.last_segment + 1), capacity);
      bucket.entry_slot = entries.locate_cell(bucket.h);
    }
    entry = storage->allocate<StrmapEntry<V>>();
    entry->key = key;
    entry->value = value;
    entry->next_entry = *bucket.entry_slot;
    *bucket.entry_slot = entry;
    entry_count += 1;
    return entry;
  }

  void DEBUG_occupancy()
  {
    int empty_buckets = 0;
    int total_entry_count = 0,
        entry_count = 0,
        max_bucket_length = 0;

    for (int i = 0; i < capacity; i++) {
      StrmapEntry<V>** entry_slot = entries.locate_cell(i);
      StrmapEntry<V>* entry = *entry_slot;
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
};

template<class V>
struct StrmapIterator {
  Strmap<V>* strmap;
  StrmapEntry<V>* entry;
  int i;

  void begin(Strmap<V>* strmap)
  {
    this->strmap = strmap;
    i = -1;
    entry = 0;
  }

  StrmapEntry<V>* next()
  {
    Strmap<V>* strmap = this->strmap;
    StrmapEntry<V>* entry = this->entry;
    if (entry) {
      entry = entry->next_entry;
      if (entry) {
        this->entry = entry;
        return this->entry;
      }
    }
    i++;
    while (i < strmap->capacity) {
      StrmapEntry<V>** entry_slot = strmap->entries.locate_cell(i);
      entry = *entry_slot;
      if (entry) {
        this->entry = entry;
        break;
      }
      i++;
    }
    return entry;
  }
};

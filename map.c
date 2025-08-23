#include <stdio.h>
#include <stdint.h>
#include "foundation.h"

static MapEntry* search_entry(MapEntry* entry, void* key)
{
  if (!entry) {
    return 0;
  } else if (entry->key == key) {
    return entry;
  } else if (key < entry->key) {
    return search_entry(entry->left_branch, key);
  } else {
    return search_entry(entry->right_branch, key);
  }
  assert(0);
  return 0;
}

static MapEntry* insert_entry(Map* map, MapEntry** branch, MapEntry* entry,
    void* key, void* value, bool return_if_found)
{
  if (!entry) {
    entry = arena_malloc(map->storage, sizeof(MapEntry));
    *branch = entry;
    entry->key = key;
    entry->value = value;
    entry->left_branch = 0;
    entry->right_branch = 0;
    entry->next = map->first;
    map->first = entry;
    return entry;
  } else if (entry->key == key) {
    if (return_if_found) { return entry; } else { return 0; }
  } else if (key < entry->key) {
    return insert_entry(map, &entry->left_branch, entry->left_branch,
                key, value, return_if_found);
  } else {
    return insert_entry(map, &entry->right_branch, entry->right_branch,
                key, value, return_if_found);
  }
  assert(0);
  return 0;
}

void* map_lookup(Map* map, void* key, MapEntry** entry)
{
  MapEntry* m;
  void* value;

  m = search_entry(map->root, key);
  value = 0;
  if (m) { value = m->value; }
  if (entry) { *entry = m; }
  return value;
}

MapEntry* map_insert(Map* map, void* key, void* value, bool return_if_found)
{
  return insert_entry(map, &map->root, map->root, key, value, return_if_found);
}

int
map_count(Map* map)
{
  int c = 0;
  MapEntry* m;

  for (m = map->first; m != 0; m = m->next) {
    c += 1;
  }
  return c;
}

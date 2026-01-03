#include <map.h>

MapEntry* Map::search_entry(MapEntry* entry, void* key)
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

MapEntry* Map::insert_entry(MapEntry** branch, MapEntry* entry,
             void* key, void* value, bool return_if_found)
{
  if (!entry) {
    entry = (MapEntry*)storage->allocate(sizeof(MapEntry), 1);
    *branch = entry;
    entry->key = key;
    entry->value = value;
    entry->left_branch = 0;
    entry->right_branch = 0;
    entry->next = first;
    first = entry;
    return entry;
  } else if (entry->key == key) {
    if (return_if_found) { return entry; } else { return 0; }
  } else if (key < entry->key) {
    return insert_entry(&entry->left_branch, entry->left_branch,
                        key, value, return_if_found);
  } else {
    return insert_entry(&entry->right_branch, entry->right_branch,
                        key, value, return_if_found);
  }
  assert(0);
  return 0;
}

void* Map::lookup(void* key, MapEntry** entry)
{
  MapEntry* m = search_entry(root, key);
  void* value = 0;
  if (m) { value = m->value; }
  if (entry) { *entry = m; }
  return value;
}

MapEntry* Map::insert(void* key, void* value, bool return_if_found)
{
  return insert_entry(&root, root, key, value, return_if_found);
}

int Map::count()
{
  int c = 0;
  for (MapEntry*  m = first; m != 0; m = m->next) {
    c += 1;
  }
  return c;
}

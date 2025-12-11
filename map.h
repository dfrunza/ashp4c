#pragma once
#include <arena.h>

template<class K, class V>
struct MapEntry {
  MapEntry* next;
  MapEntry* left_branch;
  MapEntry* right_branch;
  K* key;
  V* value;
};

template<class K, class V>
struct Map {
  Arena* storage;
  MapEntry<K,V>* first;
  MapEntry<K,V>* root;

  MapEntry<K,V>* search_entry(MapEntry<K,V>* entry, K* key)
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

  MapEntry<K,V>* insert_entry(MapEntry<K,V>** branch, MapEntry<K,V>* entry,
                              K* key, V* value, bool return_if_found)
  {
    if (!entry) {
      entry = storage->allocate<MapEntry<K,V>>();
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

  V* lookup(K* key, MapEntry<K,V>** entry)
  {

    MapEntry<K,V>* m = search_entry(root, key);
    V* value = 0;
    if (m) { value = m->value; }
    if (entry) { *entry = m; }
    return value;
  }

  MapEntry<K,V>* insert(K* key, V* value, bool return_if_found)
  {
    return insert_entry(&root, root, key, value, return_if_found);
  }

  int count()
  {
    int c = 0;
    for (MapEntry<K,V>*  m = first; m != 0; m = m->next) {
      c += 1;
    }
    return c;
  }
};

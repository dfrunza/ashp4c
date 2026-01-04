#pragma once
#include "arena.h"

struct MapEntry {
  MapEntry* next;
  MapEntry* left_branch;
  MapEntry* right_branch;
  void* key;
  void* value;
};

struct Map {
  Arena* storage;
  MapEntry* first;
  MapEntry* root;

  MapEntry* search_entry(MapEntry* entry, void* key);
  MapEntry* insert_entry(MapEntry** branch, MapEntry* entry,
               void* key, void* value, bool return_if_found);
  void* lookup(void* key, MapEntry** entry);
  MapEntry* insert(void* key, void* value, bool return_if_found);
  int count();
};

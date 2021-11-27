#include "basic.h"
#include "arena.h"
#include "token.h"
#include "symtable.h"
#include <memory.h>  // memset

#define DEBUG_ENABLED 0

internal struct Arena* symtable_storage;
internal struct UnboundedArray scope_stack = {};
internal struct SymtableEntry* null_entry = 0;
internal struct Hashmap map_child_scope = {};


struct Scope*
get_root_scope()
{
  struct Scope* scope = *(struct Scope**)array_get(&scope_stack, 0);
  return scope;
}

struct Scope*
get_current_scope()
{
  struct Scope* scope = *(struct Scope**)array_get(&scope_stack, scope_stack.elem_count - 1);
  return scope;
}

struct Scope*
new_scope(int capacity_log2)
{
  struct Scope* new_scope = arena_push(symtable_storage, sizeof(*new_scope));
  memset(new_scope, 0, sizeof(*new_scope));
  scope_init(new_scope, capacity_log2);
  return new_scope;
}

struct Scope*
push_scope()
{
  assert (scope_stack.elem_count > 0);
  struct Scope* current_scope = get_current_scope();
  struct Scope* new_scope = arena_push(symtable_storage, sizeof(*new_scope));
  memset(new_scope, 0, sizeof(*new_scope));
  array_append(&scope_stack, &new_scope);
  scope_init(new_scope, 4);
  new_scope->scope_level = current_scope->scope_level + 1;
  if (DEBUG_ENABLED) {
    printf("push scope %d\n", new_scope->scope_level);
  }
  new_scope->parent_scope = current_scope;
  struct HashmapEntry* entry = hashmap_get_or_create_entry(&map_child_scope,
                (uint8_t*)&current_scope, sizeof(current_scope));
  struct Scope* last_child_scope = entry->object;
  if (last_child_scope) {
    assert (last_child_scope->scope_level == new_scope->scope_level);
    last_child_scope->right_sibling_scope = new_scope;
  }
  entry->object = new_scope;
  if (!current_scope->first_child_scope) {
    current_scope->first_child_scope = new_scope;
  }
  return new_scope;
}

struct Scope*
pop_scope()
{
  assert (scope_stack.elem_count > 0);
  struct Scope* current_scope = get_current_scope();
  assert (current_scope->scope_level > 0);
  if (DEBUG_ENABLED) {
    printf("pop scope %d\n", current_scope->scope_level);
  }
  scope_stack.elem_count -= 1;
  current_scope = get_current_scope();
  return current_scope;
}

struct SymtableEntry*
find_symtable_entry(struct Scope* scope, char* name)
{
  uint32_t h = hash_string(name, scope->capacity_log2);
  struct SymtableEntry* entry = *(struct SymtableEntry**)array_get(&scope->symtable, h);
  while (entry) {
    if (cstr_match(entry->name, name))
      break;
    entry = entry->next_entry;
  }
  return entry;
}

struct SymtableEntry*
get_symtable_entry(struct Scope* scope, char* name)
{
  struct SymtableEntry* entry = find_symtable_entry(scope, name);
  if (!entry) {
    if (scope->entry_count >= scope->capacity) {
      struct Arena temp_storage = {};
      struct SymtableEntry** entries_array = arena_push(&temp_storage, scope->capacity);
      int i, j = 0;
      for (i = 0; i < scope->capacity; i++) {
        struct SymtableEntry* entry = *(struct SymtableEntry**)array_get(&scope->symtable, i);
        while (entry) {
          entries_array[j] = entry;
          struct SymtableEntry* next_entry = entry->next_entry;
          entry->next_entry = 0;
          entry = next_entry;
          j++;
        }
      }
      assert (j == scope->entry_count);
      scope->capacity = (1 << ++scope->capacity_log2) - 1;
      for (i = scope->entry_count; i < scope->capacity; i++) {
        array_append(&scope->symtable, &null_entry);
      }
      for (i = 0; i < scope->capacity; i++) {
        array_set(&scope->symtable, i, &null_entry);
      }
      for (i = 0; i < scope->entry_count; i++) {
        uint32_t h = hash_string(entries_array[i]->name, scope->capacity_log2);
        entries_array[i]->next_entry = *(struct SymtableEntry**)array_get(&scope->symtable, h);
        array_set(&scope->symtable, h, &entries_array[i]);
      }
      arena_delete(&temp_storage);
    }
    int h = hash_string(name, scope->capacity_log2);
    entry = arena_push(symtable_storage, sizeof(*entry));
    memset(entry, 0, sizeof(*entry));
    entry->name = name;
    entry->next_entry = *(struct SymtableEntry**)array_get(&scope->symtable, h);
    array_set(&scope->symtable, h, &entry);
    scope->entry_count += 1;
  }
  return entry;
}

struct SymtableEntry*
scope_resolve_name(struct Scope* scope, char* name)
{
  struct SymtableEntry* entry = 0;
  while (scope) {
    entry = get_symtable_entry(scope, name);
    if (entry->id_type || entry->id_ident) {
      break;
    }
    scope = scope->parent_scope;
  }
  return entry;
}

struct SymtableEntry*
register_type(struct Scope* scope, struct ObjectDescriptor* descriptor, int line_nr)
{
  struct SymtableEntry* entry = get_symtable_entry(scope, descriptor->name);
  descriptor->next_in_scope = entry->id_type;
  entry->id_type = (struct ObjectDescriptor*)descriptor;
  if (DEBUG_ENABLED) {
    printf("new type `%s` at line %d.\n", descriptor->name, line_nr);
  }
  return entry;
}

struct SymtableEntry*
register_identifier(struct Scope* scope, struct ObjectDescriptor* descriptor, int line_nr)
{
  struct SymtableEntry* entry = get_symtable_entry(scope, descriptor->name);
  descriptor->next_in_scope = entry->id_ident;
  entry->id_ident = (struct ObjectDescriptor*)descriptor;
  if (DEBUG_ENABLED) {
    printf("new identifier `%s` at line %d.\n", descriptor->name, line_nr);
  }
  return entry;
}

struct SymtableEntry*
register_keyword(struct Scope* scope, struct ObjectDescriptor* descriptor)
{
  struct SymtableEntry* entry = get_symtable_entry(scope, descriptor->name);
  entry->id_kw = (struct ObjectDescriptor*)descriptor;
  return entry;
}

void
scope_init(struct Scope* scope, int capacity_log2)
{
  struct SymtableEntry* null_entry = 0;
  array_init(&scope->symtable, sizeof(null_entry), symtable_storage);
  scope->capacity = (1 << capacity_log2) - 1;
  scope->entry_count = 0;
  int i;
  for (i = scope->entry_count; i < scope->capacity; i++) {
    array_append(&scope->symtable, &null_entry);
  }
  scope->capacity_log2 = capacity_log2;
}

void
symtable_init(struct Arena* symtable_storage_)
{
  symtable_storage = symtable_storage_;
  hashmap_init(&map_child_scope, 5, symtable_storage);
  struct Scope* root_scope = arena_push(symtable_storage, sizeof(*root_scope));
  memset(root_scope, 0, sizeof(*root_scope));
  scope_init(root_scope, 5);
  array_init(&scope_stack, sizeof(root_scope), symtable_storage);
  array_append(&scope_stack, &root_scope);
}


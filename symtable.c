#include "basic.h"
#include "arena.h"
#include "token.h"
#include "hashmap.h"
#include "symtable.h"
#include <memory.h>  // memset


internal struct Arena *symtable_storage;
internal struct Arena local_storage = {};
internal struct UnboundedArray scope_stack = {};


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
  struct Scope* scope = new_scope(4);
  array_append(&scope_stack, &scope);
  scope->scope_level = current_scope->scope_level + 1;
  scope->parent_scope = current_scope;
  return scope;
}

struct Scope*
pop_scope()
{
  assert (scope_stack.elem_count > 0);
  struct Scope* current_scope = get_current_scope();
  assert (current_scope->scope_level > 0);
  scope_stack.elem_count -= 1;
  current_scope = get_current_scope();
  return current_scope;
}

struct SymtableEntry*
symtable_get_or_create_entry(struct Hashmap* declarations, char* name)
{
  struct HashmapKey key = { .s_key = (uint8_t*)name };
  hashmap_hash_key(HASHMAP_KEY_STRING, &key, declarations->capacity_log2);
  struct HashmapEntry* hmap_entry = hashmap_get_or_create_entry(declarations, &key);
  if (hmap_entry->object) {
    return (struct SymtableEntry*)hmap_entry->object;
  }
  struct SymtableEntry* entry = arena_push(symtable_storage, sizeof(*entry));
  hmap_entry->object = entry;
  memset(entry, 0, sizeof(*entry));
  entry->strname = name;
  return entry;
}

struct SymtableEntry*
symtable_get_entry(struct Hashmap* declarations, char* name)
{
  struct HashmapKey key = { .s_key = (uint8_t*)name };
  hashmap_hash_key(HASHMAP_KEY_STRING, &key, declarations->capacity_log2);
  struct HashmapEntry* hmap_entry = hashmap_get_entry(declarations, &key);
  if (hmap_entry) {
    return (struct SymtableEntry*)hmap_entry->object;
  }
  return 0;
}

struct SymtableEntry*
scope_lookup_name(struct Scope* scope, enum Namespace ns, char* name)
{
  struct SymtableEntry* entry = 0;
  while (scope) {
    entry = symtable_get_or_create_entry(&scope->declarations, name);
    if (entry->ns_type && (ns & NAMESPACE_TYPE)) {
      break;
    }
    if (entry->ns_general && (ns & NAMESPACE_GENERAL)) {
      break;
    }
    if (entry->ns_keyword && (ns & NAMESPACE_KEYWORD)) {
      break;
    }
    scope = scope->parent_scope;
  }
  return entry;
}

struct SymtableEntry*
declare_object_in_scope(struct Scope* scope, enum Namespace ns, struct NamedObject* descriptor, int line_nr)
{
  struct SymtableEntry* entry = symtable_get_or_create_entry(&scope->declarations, descriptor->strname);
  if (ns == NAMESPACE_TYPE) {
    descriptor->next_in_scope = entry->ns_type;
    entry->ns_type = (struct NamedObject*)descriptor;
  } else if (ns == NAMESPACE_GENERAL) {
    descriptor->next_in_scope = entry->ns_general;
    entry->ns_general = (struct NamedObject*)descriptor;
  } else if (ns == NAMESPACE_KEYWORD) {
    struct SymtableEntry* entry = symtable_get_or_create_entry(&scope->declarations, descriptor->strname);
    entry->ns_keyword = (struct NamedObject*)descriptor;
  } else assert (0);
  return entry;
}

void
scope_init(struct Scope* scope, int capacity_log2)
{
  scope->scope_level = 0;
  scope->parent_scope = 0;
  hashmap_init(&scope->declarations, HASHMAP_KEY_STRING, capacity_log2, symtable_storage);
}

void
symtable_begin_build(struct Arena* symtable_storage_)
{
  symtable_storage = symtable_storage_;
  struct Scope* root_scope = arena_push(symtable_storage, sizeof(*root_scope));
  memset(root_scope, 0, sizeof(*root_scope));
  scope_init(root_scope, 4);
  array_init(&scope_stack, sizeof(&root_scope), symtable_storage);
  array_append(&scope_stack, &root_scope);
}

void
symtable_end_build()
{
  arena_delete(&local_storage);
}

#include "basic.h"
#include "arena.h"
#include "token.h"
#include "hashmap.h"
#include "symtable.h"
#include <memory.h>  // memset

#define DEBUG_ENABLED 0


internal struct Arena *symtable_storage, *temp_storage;
internal struct UnboundedArray scope_stack = {};
internal struct SymtableEntry* null_entry = 0;
internal struct Hashmap child_scope_map = {};


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
  struct HashmapEntry* entry = hashmap_get_or_create_entry(&child_scope_map,
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
symtable_get_or_create_entry(struct Scope* scope, char* name)
{
  struct HashmapEntry* hmap_entry = hashmap_get_or_create_entry(&scope->symtable, (uint8_t*)name, 0);
  if (hmap_entry->object) {
    return (struct SymtableEntry*)hmap_entry->object;
  }
  struct SymtableEntry* entry = arena_push(symtable_storage, sizeof(*entry));
  hmap_entry->object = entry;
  memset(entry, 0, sizeof(*entry));
  entry->name = name;
  return entry;
}

struct SymtableEntry*
scope_resolve_name(struct Scope* scope, char* name)
{
  struct SymtableEntry* entry = 0;
  while (scope) {
    entry = symtable_get_or_create_entry(scope, name);
    if (entry->ns_type || entry->ns_general) {
      break;
    }
    scope = scope->parent_scope;
  }
  return entry;
}

struct SymtableEntry*
declare_object_in_scope(struct Scope* scope, enum Namespace ns, struct ObjectDescriptor* descriptor, int line_nr)
{
  struct SymtableEntry* entry = symtable_get_or_create_entry(scope, descriptor->name);
  if (ns == NAMESPACE_TYPE) {
    descriptor->next_in_scope = entry->ns_type;
    entry->ns_type = (struct ObjectDescriptor*)descriptor;
    if (DEBUG_ENABLED) {
      printf("new type `%s` at line %d.\n", descriptor->name, line_nr);
    }
  } else if (ns == NAMESPACE_GENERAL) {
    descriptor->next_in_scope = entry->ns_general;
    entry->ns_general = (struct ObjectDescriptor*)descriptor;
    if (DEBUG_ENABLED) {
      printf("new identifier `%s` at line %d.\n", descriptor->name, line_nr);
    }
  } else if (ns == NAMESPACE_KEYWORD) {
    struct SymtableEntry* entry = symtable_get_or_create_entry(scope, descriptor->name);
    entry->ns_keyword = (struct ObjectDescriptor*)descriptor;
  }
  return entry;
}

void
scope_init(struct Scope* scope, int capacity_log2)
{
  scope->scope_level = 0;
  scope->parent_scope = 0;
  scope->first_child_scope = 0;
  scope->right_sibling_scope = 0;
  hashmap_init(&scope->symtable, capacity_log2, symtable_storage);
}

void
symtable_init(struct Arena* symtable_storage_, struct Arena* temp_storage_)
{
  symtable_storage = symtable_storage_;
  temp_storage = temp_storage_;
  hashmap_init(&child_scope_map, 5, temp_storage);
  struct Scope* root_scope = arena_push(symtable_storage, sizeof(*root_scope));
  memset(root_scope, 0, sizeof(*root_scope));
  scope_init(root_scope, 5);
  array_init(&scope_stack, sizeof(root_scope), symtable_storage);
  array_append(&scope_stack, &root_scope);
}


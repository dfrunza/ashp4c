#include "basic.h"
#include "arena.h"
#include "token.h"
#include "hashmap.h"
#include "symtable.h"
#include <memory.h>  // memset


internal struct Arena *m_symtable_storage;
internal struct Arena m_local_storage = {};
internal struct UnboundedArray m_scope_stack = {};


struct Scope*
get_root_scope()
{
  struct Scope* scope = *(struct Scope**)array_get(&m_scope_stack, 0);
  return scope;
}

struct Scope*
get_current_scope()
{
  struct Scope* scope = *(struct Scope**)array_get(&m_scope_stack, m_scope_stack.elem_count - 1);
  return scope;
}

struct Scope*
new_scope(int capacity_log2)
{
  struct Scope* new_scope = arena_push(m_symtable_storage, sizeof(*new_scope));
  memset(new_scope, 0, sizeof(*new_scope));
  scope_init(new_scope, capacity_log2);
  return new_scope;
}

struct Scope*
push_scope()
{
  assert (m_scope_stack.elem_count > 0);
  struct Scope* current_scope = get_current_scope();
  struct Scope* scope = new_scope(4);
  array_append(&m_scope_stack, &scope);
  scope->scope_level = current_scope->scope_level + 1;
  scope->parent_scope = current_scope;
  return scope;
}

struct Scope*
pop_scope()
{
  assert (m_scope_stack.elem_count > 0);
  struct Scope* current_scope = get_current_scope();
  assert (current_scope->scope_level > 0);
  m_scope_stack.elem_count -= 1;
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
  struct SymtableEntry* entry = arena_push(m_symtable_storage, sizeof(*entry));
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
scope_lookup_name(struct Scope* scope, char* name)
{
  struct SymtableEntry* entry = 0;
  while (scope) {
    entry = symtable_get_or_create_entry(&scope->declarations, name);
    if (entry->ns_type || entry->ns_var || entry->ns_keyword) {
      break;
    }
    scope = scope->parent_scope;
  }
  return entry;
}

struct SymtableEntry*
declare_object_in_scope(struct Scope* scope, enum Namespace ns, struct NameDecl* decl)
{
  struct SymtableEntry* entry = symtable_get_or_create_entry(&scope->declarations, decl->strname);
  if (ns == NAMESPACE_TYPE) {
    decl->next_in_scope = entry->ns_type;
    entry->ns_type = decl;
  } else if (ns == NAMESPACE_VAR) {
    decl->next_in_scope = entry->ns_var;
    entry->ns_var = decl;
  } else if (ns == NAMESPACE_KEYWORD) {
    struct SymtableEntry* entry = symtable_get_or_create_entry(&scope->declarations, decl->strname);
    entry->ns_keyword = decl;
  } else assert(0);
  return entry;
}

struct NameRef*
nameref_get_entry(struct Hashmap* map, uint32_t id)
{
  struct HashmapKey key = { .i_key = id };
  hashmap_hash_key(HASHMAP_KEY_INT, &key, map->capacity_log2);
  struct HashmapEntry* he = hashmap_get_entry(map, &key);
  struct NameRef* nameref = 0;
  if (he) {
    nameref = he->object;
  }
  return nameref;
}

void
nameref_add_entry(struct Hashmap* map, struct NameRef* nameref, uint32_t id)
{
  struct HashmapKey key = { .i_key = id };
  hashmap_hash_key(HASHMAP_KEY_INT, &key, map->capacity_log2);
  struct HashmapEntry* he = hashmap_get_or_create_entry(map, &key);
  assert(!he->object);
  he->object = nameref;
}

struct Type*
type_get_entry(struct Hashmap* map, uint32_t id)
{
  struct HashmapKey key = { .i_key = id };
  hashmap_hash_key(HASHMAP_KEY_INT, &key, map->capacity_log2);
  struct HashmapEntry* he = hashmap_get_entry(map, &key);
  struct Type* type = 0;
  if (he) {
    type = he->object;
  }
  return type;
}

void
type_add_entry(struct Hashmap* map, struct Type* type, uint32_t id)
{
  struct HashmapKey key = { .i_key = id };
  hashmap_hash_key(HASHMAP_KEY_INT, &key, map->capacity_log2);
  struct HashmapEntry* he = hashmap_get_or_create_entry(map, &key);
  assert(!he->object);
  he->object = type;
}

void
scope_init(struct Scope* scope, int capacity_log2)
{
  scope->scope_level = 0;
  scope->parent_scope = 0;
  hashmap_init(&scope->declarations, HASHMAP_KEY_STRING, capacity_log2, m_symtable_storage);
}

void
symtable_init(struct Arena* symtable_storage)
{
  m_symtable_storage = symtable_storage;
  struct Scope* root_scope = arena_push(m_symtable_storage, sizeof(*root_scope));
  memset(root_scope, 0, sizeof(*root_scope));
  scope_init(root_scope, 4);
  array_init(&m_scope_stack, sizeof(struct Scope*), m_symtable_storage);
  array_append(&m_scope_stack, &root_scope);
}

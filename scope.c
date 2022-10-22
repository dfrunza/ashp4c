#include <memory.h>  // memset
#include <stdint.h>
#include "arena.h"
#include "ast.h"

internal struct Arena *scope_storage;
internal struct Scope* current_scope;
internal int scope_level;

struct Scope*
push_scope()
{
  struct Scope* scope = arena_push_struct(scope_storage, struct Scope);
  hashmap_init(&scope->decls, HASHMAP_KEY_STRING, 4, scope_storage);
  scope->scope_level = scope_level++;
  scope->parent_scope = current_scope;
  current_scope = scope;
  return scope;
}

struct Scope*
pop_scope()
{
  assert (scope_level > 0);
  current_scope = current_scope->parent_scope;
  scope_level -= 1;
  return current_scope;
}

struct NameEntry*
namedecl_get_or_create(struct Hashmap* decls, char* name)
{
  struct HashmapKey key = { .s_key = (uint8_t*)name };
  hashmap_hash_key(HASHMAP_KEY_STRING, &key, decls->capacity_log2);
  struct HashmapEntry* he = hashmap_get_or_create_entry(decls, &key);
  if (he->object) {
    return (struct NameEntry*)he->object;
  }
  struct NameEntry* entry = arena_push_struct(scope_storage, struct NameEntry);
  he->object = entry;
  memset(entry, 0, sizeof(*entry));
  entry->strname = name;
  return entry;
}

struct NameEntry*
namedecl_get(struct Hashmap* decls, char* name)
{
  struct HashmapKey key = { .s_key = (uint8_t*)name };
  hashmap_hash_key(HASHMAP_KEY_STRING, &key, decls->capacity_log2);
  struct HashmapEntry* hmap_entry = hashmap_get_entry(decls, &key);
  struct NameEntry* entry = 0;
  if (hmap_entry) {
    entry = hmap_entry->object;
  }
  return entry;
}

struct NameEntry*
scope_lookup_name(struct Scope* scope, char* name)
{
  struct NameEntry* ne = 0;
  while (scope) {
    ne = namedecl_get_or_create(&scope->decls, name);
    if (ne->ns_type || ne->ns_var || ne->ns_keyword) {
      break;
    }
    scope = scope->parent_scope;
  }
  return ne;
}

struct NameEntry*
declare_name_in_scope(struct Scope* scope, enum Namespace ns, struct NameDecl* decl)
{
  struct NameEntry* ne = namedecl_get_or_create(&scope->decls, decl->strname);
  if (ns == NAMESPACE_TYPE) {
    decl->nextdecl_in_scope = ne->ns_type;
    ne->ns_type = decl;
  } else if (ns == NAMESPACE_VAR) {
    decl->nextdecl_in_scope = ne->ns_var;
    ne->ns_var = decl;
  } else if (ns == NAMESPACE_KEYWORD) {
    assert(!decl->nextdecl_in_scope);
    ne->ns_keyword = decl;
  } else assert(0);
  return ne;
}

void
scope_init(struct Arena* scope_storage_)
{
  scope_storage = scope_storage_;
  scope_level = 0;
  current_scope = 0;
}


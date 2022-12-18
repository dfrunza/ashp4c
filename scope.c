#include <memory.h>  // memset
#include <stdint.h>
#include "arena.h"
#include "ast.h"

internal Arena *scope_storage;
internal Scope* current_scope;
internal int scope_level;

Scope*
push_scope()
{
  Scope* scope = arena_push_struct(scope_storage, Scope);
  hashmap_init(&scope->decls, HASHMAP_KEY_STRING, 4, scope_storage);
  scope->scope_level = scope_level++;
  scope->parent_scope = current_scope;
  current_scope = scope;
  return scope;
}

Scope*
pop_scope()
{
  assert (scope_level > 0);
  current_scope = current_scope->parent_scope;
  scope_level -= 1;
  return current_scope;
}

NameEntry*
namedecl_get_or_create(Hashmap* decls, char* name)
{
  HashmapKey key = { .s_key = (uint8_t*)name };
  hashmap_hash_key(HASHMAP_KEY_STRING, &key, decls->capacity_log2);
  HashmapEntry* he = hashmap_get_or_create_entry(decls, &key);
  if (he->object) {
    return (NameEntry*)he->object;
  }
  NameEntry* entry = arena_push_struct(scope_storage, NameEntry);
  he->object = entry;
  memset(entry, 0, sizeof(*entry));
  entry->strname = name;
  return entry;
}

NameEntry*
namedecl_get(Hashmap* decls, char* name)
{
  HashmapKey key = { .s_key = (uint8_t*)name };
  hashmap_hash_key(HASHMAP_KEY_STRING, &key, decls->capacity_log2);
  HashmapEntry* hmap_entry = hashmap_get_entry(decls, &key);
  NameEntry* entry = 0;
  if (hmap_entry) {
    entry = hmap_entry->object;
  }
  return entry;
}

void
declare_type_name(Scope* scope, Ast_Name* name, Ast* ast)
{
  NameDecl* ndecl = arena_push_struct(scope_storage, NameDecl);
  ndecl->ast = ast;
  ndecl->strname = name->strname;
  ndecl->line_no = name->line_no;
  ndecl->column_no = name->column_no;
  NameEntry* ne = namedecl_get_or_create(&scope->decls, name->strname);
  ndecl->next_decl = ne->ns_type;
  ne->ns_type = ndecl;
}

void
declare_var_name(Scope* scope, Ast_Name* name, Ast* ast)
{
  NameDecl* ndecl = arena_push_struct(scope_storage, NameDecl);
  ndecl->ast = ast;
  ndecl->strname = name->strname;
  ndecl->line_no = name->line_no;
  ndecl->column_no = name->column_no;
  NameEntry* ne = namedecl_get_or_create(&scope->decls, name->strname);
  ne->ns_var = ndecl;
}

void
declare_keyword(Scope* scope, char* strname, enum TokenClass token_class)
{
  NameDecl* ndecl = arena_push_struct(scope_storage, NameDecl);
  ndecl->strname = strname;
  ndecl->token_class = token_class;
  NameEntry* ne = namedecl_get_or_create(&scope->decls, strname);
  ne->ns_keyword = ndecl;
}

NameEntry*
scope_lookup_name(Scope* scope, char* name)
{
  NameEntry* ne = 0;
  while (scope) {
    ne = namedecl_get_or_create(&scope->decls, name);
    if (ne->ns_type || ne->ns_var || ne->ns_keyword) {
      break;
    }
    scope = scope->parent_scope;
  }
  return ne;
}

void
scope_init(Arena* scope_storage_)
{
  scope_storage = scope_storage_;
  scope_level = 0;
  current_scope = 0;
}


#include <memory.h>  // memset
#include <stdint.h>
#include "arena.h"
#include "frontend.h"

internal Arena *scope_storage;
internal Scope* current_scope;
internal int scope_level;

Scope*
push_scope()
{
  Scope* scope = arena_push_struct(scope_storage, Scope);
  hashmap_init(&scope->name_decls, HASHMAP_KEY_STRING, 4, scope_storage);
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
namedecl_create(Hashmap* decls, char* name)
{
  HashmapKey key = { .str_key = (uint8_t*)name };
  hashmap_hash_key(HASHMAP_KEY_STRING, &key, decls->capacity_log2);
  HashmapEntry* name_he = hashmap_create_entry(decls, &key);
  if (name_he->object) {
    return (NameEntry*)name_he->object;
  }
  NameEntry* entry = arena_push_struct(scope_storage, NameEntry);
  name_he->object = entry;
  memset(entry, 0, sizeof(*entry));
  entry->strname = name;
  return entry;
}

NameEntry*
namedecl_get(Hashmap* decls, char* name)
{
  HashmapKey key = { .str_key = (uint8_t*)name };
  hashmap_hash_key(HASHMAP_KEY_STRING, &key, decls->capacity_log2);
  HashmapEntry* name_he = hashmap_get_entry(decls, &key);
  NameEntry* entry = 0;
  if (name_he) {
    entry = (NameEntry*)name_he->object;
  }
  return entry;
}

void
declare_type_name(Scope* scope, Ast_Name* name, Ast* ast)
{
  NameDecl* decl = arena_push_struct(scope_storage, NameDecl);
  decl->ast = ast;
  decl->strname = name->strname;
  decl->line_no = name->line_no;
  decl->column_no = name->column_no;
  NameEntry* ne = namedecl_create(&scope->name_decls, name->strname);
  decl->next_decl = ne->ns_type;
  ne->ns_type = decl;
}

void
declare_var_name(Scope* scope, Ast_Name* name, Ast* ast)
{
  NameDecl* decl = arena_push_struct(scope_storage, NameDecl);
  decl->ast = ast;
  decl->strname = name->strname;
  decl->line_no = name->line_no;
  decl->column_no = name->column_no;
  NameEntry* ne = namedecl_create(&scope->name_decls, name->strname);
  ne->ns_var = decl;
}

void
declare_keyword(Scope* scope, char* strname, enum TokenClass token_class)
{
  NameDecl* decl = arena_push_struct(scope_storage, NameDecl);
  decl->strname = strname;
  decl->token_class = token_class;
  NameEntry* ne = namedecl_create(&scope->name_decls, strname);
  ne->ns_keyword = decl;
}

NameEntry*
scope_lookup_name(Scope* scope, char* name)
{
  NameEntry* ne = 0;
  while (scope) {
    ne = namedecl_create(&scope->name_decls, name);
    if (ne->ns_type || ne->ns_var || ne->ns_keyword) {
      break;
    }
    scope = scope->parent_scope;
  }
  return ne;
}

void
symbol_table_init(Arena* scope_storage_)
{
  scope_storage = scope_storage_;
  scope_level = 0;
  current_scope = 0;
}


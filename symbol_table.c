#include <memory.h>  // memset
#include <stdint.h>
#include "arena.h"
#include "frontend.h"

internal Arena *scope_storage;
internal Scope* current_scope;
internal int scope_level;
internal NameEntry NULL_NAME_ENTRY = {0};

Scope*
push_scope()
{
  Scope* scope = arena_push_struct(scope_storage, Scope);
  hashmap_init(&scope->sym_table, HASHMAP_KEY_STRING, 4, scope_storage);
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

void
declare_type_name(Scope* scope, Ast_Name* name, Ast* ast)
{
  NameDecl* decl = arena_push_struct(scope_storage, NameDecl);
  decl->ast = ast;
  decl->strname = name->strname;
  decl->line_no = name->line_no;
  decl->column_no = name->column_no;
  HashmapEntry* name_he = hashmap_create_entry_string(&scope->sym_table, name->strname);
  NameEntry* ne = arena_push_struct(scope_storage, NameEntry);
  ne->strname = name->strname;
  ne->ns_type = decl;
  name_he->object = ne;
  decl->next_decl = ne->ns_type;
}

void
declare_var_name(Scope* scope, Ast_Name* name, Ast* ast)
{
  NameDecl* decl = arena_push_struct(scope_storage, NameDecl);
  decl->ast = ast;
  decl->strname = name->strname;
  decl->line_no = name->line_no;
  decl->column_no = name->column_no;
  HashmapEntry* name_he = hashmap_create_entry_string(&scope->sym_table, name->strname);
  NameEntry* ne = arena_push_struct(scope_storage, NameEntry);
  ne->strname = name->strname;
  ne->ns_var = decl;
  name_he->object = ne;
}

void
declare_keyword(Scope* scope, char* strname, enum TokenClass token_class)
{
  NameDecl* decl = arena_push_struct(scope_storage, NameDecl);
  decl->strname = strname;
  decl->token_class = token_class;
  HashmapEntry* name_he = hashmap_create_entry_string(&scope->sym_table, strname);
  NameEntry* ne = arena_push_struct(scope_storage, NameEntry);
  ne->strname = strname;
  ne->ns_keyword = decl;
  name_he->object = ne;
}

NameEntry*
scope_lookup_name(Scope* scope, char* strname)
{
  NameEntry* ne = &NULL_NAME_ENTRY;
  while (scope) {
    HashmapEntry* name_he = hashmap_get_entry_string(&scope->sym_table, strname);
    if (name_he && name_he->object) {
      ne = (NameEntry*)name_he->object;
      if (ne->ns_type || ne->ns_var || ne->ns_keyword) {
        break;
      }
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


#include <memory.h>  // memset
#include <stdint.h>
#include "foundation.h"
#include "frontend.h"

internal Arena *scope_storage;
internal Scope* current_scope;
internal int scope_level;

Scope*
push_scope()
{
  Scope* scope = arena_push_struct(scope_storage, Scope);
  hashmap_create(&scope->decls, HASHMAP_KEY_STRING, 3, scope_storage);
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
declare_type_name(Scope* scope, char* strname, int line_no, int column_no, Ast* ast)
{
  NameDecl* decl = arena_push_struct(scope_storage, NameDecl);
  decl->ast = ast;
  decl->strname = strname;
  decl->line_no = line_no;
  decl->column_no = column_no;
  HashmapEntry* he = hashmap_create_entry_string(&scope->decls, strname);
  NamespaceEntry* ns = arena_push_struct(scope_storage, NamespaceEntry);
  ns->strname = strname;
  ns->scope = scope;
  ns->ns_type = decl;
  he->object = ns;
  decl->next_decl = ns->ns_type;
}

void
declare_var_name(Scope* scope, char* strname, int line_no, int column_no, Ast* ast)
{
  NameDecl* decl = arena_push_struct(scope_storage, NameDecl);
  decl->ast = ast;
  decl->strname = strname;
  decl->line_no = line_no;
  decl->column_no = column_no;
  HashmapEntry* he = hashmap_create_entry_string(&scope->decls, strname);
  NamespaceEntry* ns = arena_push_struct(scope_storage, NamespaceEntry);
  ns->strname = strname;
  ns->scope = scope;
  ns->ns_var = decl;
  he->object = ns;
}

void
declare_keyword(Scope* scope, char* strname, enum TokenClass token_class)
{
  NameDecl* decl = arena_push_struct(scope_storage, NameDecl);
  decl->strname = strname;
  decl->token_class = token_class;
  HashmapEntry* he = hashmap_create_entry_string(&scope->decls, strname);
  NamespaceEntry* ns = arena_push_struct(scope_storage, NamespaceEntry);
  ns->strname = strname;
  ns->scope = scope;
  ns->ns_keyword = decl;
  he->object = ns;
}

NamespaceEntry*
scope_lookup_name(Scope* scope, char* strname)
{
  NamespaceEntry* ns = 0;
  while (scope) {
    HashmapEntry* he = hashmap_get_entry_string(&scope->decls, strname);
    if (he && he->object) {
      ns = (NamespaceEntry*)he->object;
      if (ns->ns_type || ns->ns_var || ns->ns_keyword) {
        break;
      }
    }
    scope = scope->parent_scope;
  }
  return ns;
}

void
scope_reset(Arena* scope_storage_)
{
  scope_storage = scope_storage_;
  scope_level = 0;
  current_scope = 0;
}


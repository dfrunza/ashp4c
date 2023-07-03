#include <memory.h>  // memset
#include <stdint.h>
#include "foundation.h"
#include "frontend.h"

Scope*
push_scope(Arena* storage, Scope* parent_scope)
{
  Scope* scope = arena_push_struct(storage, Scope);
  hashmap_create(&scope->decls, HASHMAP_KEY_STRING, 3, storage);
  scope->scope_level = parent_scope->scope_level + 1;
  scope->parent_scope = parent_scope;
  return scope;
}

Scope*
pop_scope(Scope* scope)
{
  Scope* parent_scope = scope->parent_scope;
  return parent_scope;
}

NameDecl*
declare_type_name(Arena* storage, Scope* scope, char* strname, int line_no, int column_no)
{
  NameDecl* decl = arena_push_struct(storage, NameDecl);
  decl->strname = strname;
  decl->line_no = line_no;
  decl->column_no = column_no;
  ListItem* decl_li = arena_push_struct(storage, ListItem);
  decl_li->object = decl;
  HashmapEntry* he = hashmap_get_entry_string(&scope->decls, strname);
  NamespaceEntry* ns = he->object;
  if (!ns) {
    ns = arena_push_struct(storage, NamespaceEntry);
    list_reset(&ns->ns_type);
  }
  ns->strname = strname;
  ns->scope = scope;
  list_append_item(&ns->ns_type, decl_li, 1);
  he->object = ns;
  return decl;
}

NameDecl*
declare_var_name(Arena* storage, Scope* scope, char* strname, int line_no, int column_no)
{
  NameDecl* decl = arena_push_struct(storage, NameDecl);
  decl->strname = strname;
  decl->line_no = line_no;
  decl->column_no = column_no;
  HashmapEntry* he = hashmap_get_entry_string(&scope->decls, strname);
  NamespaceEntry* ns = he->object;
  if (!ns) {
    ns = arena_push_struct(storage, NamespaceEntry);
    list_reset(&ns->ns_type);
  }
  ns->strname = strname;
  ns->scope = scope;
  if (ns->ns_var) {
    error("At line %d, column %d: redeclaration of name `%s`.", line_no, column_no, strname);
  } else { ns->ns_var = decl; }
  he->object = ns;
  return decl;
}

NameDecl*
declare_keyword(Arena* storage, Scope* scope, char* strname)
{
  NameDecl* decl = arena_push_struct(storage, NameDecl);
  decl->strname = strname;
  HashmapEntry* he = hashmap_get_entry_string(&scope->decls, strname);
  NamespaceEntry* ns = he->object;
  if (!ns) {
    ns = arena_push_struct(storage, NamespaceEntry);
    list_reset(&ns->ns_type);
  }
  ns->strname = strname;
  ns->scope = scope;
  assert(!ns->ns_keyword);
  ns->ns_keyword = decl;
  he->object = ns;
  return decl;
}

NamespaceEntry*
scope_lookup_name(Scope* scope, char* strname)
{
  NamespaceEntry* ns = 0;
  while (scope) {
    HashmapEntry* he = hashmap_lookup_entry_string(&scope->decls, strname);
    if (he && he->object) {
      ns = (NamespaceEntry*)he->object;
      if (ns->ns_type.item_count > 0 || ns->ns_var || ns->ns_keyword) {
        break;
      }
    }
    scope = scope->parent_scope;
  }
  return ns;
}


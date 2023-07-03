#include <memory.h>  // memset
#include <stdio.h>
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
  HashmapEntry* he = hashmap_get_entry_string(&scope->decls, strname);
  NamespaceEntry* ns = he->object;
  if (!ns) { ns = arena_push_struct(storage, NamespaceEntry); }
  ns->strname = strname;
  ns->scope = scope;
  decl->next_in_scope = ns->ns_type;
  ns->ns_type = decl;
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
  if (!ns) { ns = arena_push_struct(storage, NamespaceEntry); }
  ns->strname = strname;
  ns->scope = scope;
  decl->next_in_scope = ns->ns_var;
  ns->ns_var = decl;
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
  if (!ns) { ns = arena_push_struct(storage, NamespaceEntry); }
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
      if (ns->ns_type || ns->ns_var || ns->ns_keyword) {
        break;
      }
    }
    scope = scope->parent_scope;
  }
  return ns;
}

void
Debug_print_scope_declarations(Scope* scope)
{
  int count = 0;
  HashmapCursor entry_it = {};
  hashmap_cursor_reset(&entry_it, &scope->decls);
  printf("Names in scope 0x%x\n\n", scope);
  for (HashmapEntry* entry = hashmap_move_cursor(&entry_it);
       entry != 0; entry = hashmap_move_cursor(&entry_it)) {
    NamespaceEntry* ns = entry->object;
    if (ns->ns_type) {
      NameDecl* decl = ns->ns_type;
      while (decl) {
        printf("%s  ...  at %d:%d\n", decl->strname, decl->line_no, decl->column_no);
        decl = decl->next_in_scope;
        count += 1;
      }
    }
    if (ns->ns_var) {
      NameDecl* decl = ns->ns_var;
      printf("%s  ...  at %d:%d\n", decl->strname, decl->line_no, decl->column_no);
      count += 1;
    }
  }
  printf("\nTotal: %d\n", count);
}


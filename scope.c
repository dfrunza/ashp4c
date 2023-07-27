#include <stdio.h>
#include <stdint.h>
#include "foundation.h"
#include "frontend.h"

Scope*
scope_push(Scope* scope, Scope* parent_scope)
{
  scope->scope_level = parent_scope->scope_level + 1;
  scope->parent_scope = parent_scope;
  return scope;
}

Scope*
scope_pop(Scope* scope)
{
  Scope* parent_scope = scope->parent_scope;
  return parent_scope;
}

ScopeEntry*
scope_lookup_any(Scope* scope, char* strname)
{
  ScopeEntry* ns_entry = 0;
  while (scope) {
    ns_entry = hashmap_lookup_entry_stringk(&scope->name_table, strname, ScopeEntry);
    if (ns_entry) {
      if (ns_entry->ns[NS_TYPE] || ns_entry->ns[NS_VAR] || ns_entry->ns[NS_KEYWORD]) {
        break;
      }
    }
    scope = scope->parent_scope;
  }
  return ns_entry;
}

ScopeEntry*
scope_lookup_namespace(Scope* scope, char* strname, enum NameSpace ns)
{
  ScopeEntry* ns_entry = 0;
  while (scope) {
    ns_entry = hashmap_lookup_entry_stringk(&scope->name_table, strname, ScopeEntry);
    if (ns_entry) {
      if (ns_entry->ns[ns]) {
        break;
      }
    }
    scope = scope->parent_scope;
  }
  return ns_entry;
}

ScopeEntry*
scope_push_decl(Scope* scope, NameDecl* decl, enum NameSpace ns)
{
  ScopeEntry* ns_entry = hashmap_get_entry_stringk(&scope->name_table, decl->strname, ScopeEntry);
  decl->next_in_scope = ns_entry->ns[ns];
  ns_entry->ns[ns] = decl;
  return ns_entry;
}

void
Debug_scope_decls(Scope* scope)
{
  int count = 0;
  HashmapCursor entry_it = {};
  hashmap_cursor_reset(&entry_it, &scope->name_table);
  printf("Names in scope 0x%x\n\n", scope);
  for (ScopeEntry* ns_entry = hashmap_move_cursor(&entry_it, ScopeEntry);
       ns_entry != 0; ns_entry = hashmap_move_cursor(&entry_it, ScopeEntry)) {
    for (int i = 0; i < NameSpace_COUNT; i++) {
      NameDecl* decl;
      decl = ns_entry->ns[i];
      while (decl) {
        if (i == NS_KEYWORD) {
          printf("%s  ...  at 0:0, ns[%d]\n", decl->strname, i);
        } else {
          Ast* ast = decl->ast;
          printf("%s  ...  at %d:%d, ns[%d]\n", decl->strname, ast->line_no, ast->column_no, i);
        }
        decl = decl->next_in_scope;
        count += 1;
      }
    }
  }
  printf("\nTotal: %d\n", count);
}


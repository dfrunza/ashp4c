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
  return scope->parent_scope;
}

NameEntry*
scope_lookup_any(Scope* scope, char* strname)
{
  NameEntry* ns_entry = 0;
  while (scope) {
    ns_entry = hashmap_lookup(&scope->name_table, HASHMAP_KEY_STRING, strname);
    if (ns_entry) {
      if (ns_entry->ns[NS_TYPE] || ns_entry->ns[NS_VAR] || ns_entry->ns[NS_KEYWORD]) {
        break;
      }
    }
    scope = scope->parent_scope;
  }
  return ns_entry;
}

NameEntry*
scope_lookup_namespace(Scope* scope, char* strname, enum NameSpace ns)
{
  NameEntry* ns_entry = 0;
  while (scope) {
    ns_entry = hashmap_lookup(&scope->name_table, HASHMAP_KEY_STRING, strname);
    if (ns_entry && ns_entry->ns[ns]) {
        break;
    }
    ns_entry = 0;
    scope = scope->parent_scope;
  }
  return ns_entry;
}

NameEntry*
scope_push_decl(Scope* scope, Arena* storage, NameDecl* decl, enum NameSpace ns)
{
  NameEntry* ns_entry = hashmap_get(&scope->name_table, storage, HASHMAP_KEY_STRING, decl->strname);
  decl->next_in_scope = ns_entry->ns[ns];
  ns_entry->ns[ns] = decl;
  return ns_entry;
}

void
Debug_scope_decls(Scope* scope)
{
  int count = 0;
  HashmapCursor it = {};
  hashmap_cursor_begin(&it);
  printf("Names in scope 0x%x\n\n", scope);
  for (NameEntry* ns_entry = hashmap_cursor_next(&it, &scope->name_table);
       ns_entry != 0; ns_entry = hashmap_cursor_next(&it, &scope->name_table)) {
    for (int i = 1; i < NameSpace_COUNT; i++) {
      NameDecl* decl;
      decl = ns_entry->ns[i];
      while (decl) {
        if (i == NS_KEYWORD) {
          printf("%s, ns[%d]\n", decl->strname, i);
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


#include <stdio.h>
#include <stdint.h>
#include <math.h>  /* floor, ceil, log2 */
#include "foundation.h"
#include "frontend.h"

Scope*
scope_create(Arena* storage, int max_capacity)
{
  assert(max_capacity >= 16);
  int segment_count;
  Scope* scope;

  segment_count = ceil(log2(max_capacity/16 + 1));
  scope = arena_malloc(storage, sizeof(Scope) + sizeof(HashmapEntry**) * segment_count);
  hashmap_init(&scope->name_table, storage, segment_count);
  return scope;
}

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
  NameEntry* name_entry = 0;
  HashmapKey hkey;
  HashmapEntry* he;

  while (scope) {
    hkey.str_key = strname;
    he = hashmap_lookup_entry(&scope->name_table, &hkey, HKEY_STRING);
    if (he) {
      name_entry = (NameEntry*)he->value;
      if (name_entry->ns[NS_TYPE] || name_entry->ns[NS_VAR] || name_entry->ns[NS_KEYWORD]) {
        break;
      }
    }
    name_entry = 0;
    scope = scope->parent_scope;
  }
  return name_entry;
}

NameEntry*
scope_lookup_namespace(Scope* scope, char* strname, enum NameSpace ns)
{
  NameEntry* name_entry = 0;
  HashmapKey hkey;
  HashmapEntry* he;

  while (scope) {
    hkey.str_key = strname;
    he = hashmap_lookup_entry(&scope->name_table, &hkey, HKEY_STRING);
    if (he) {
      name_entry = (NameEntry*)he->value;
      if (name_entry->ns[ns]) {
        break;
      }
    }
    name_entry = 0;
    scope = scope->parent_scope;
  }
  return name_entry;
}

NameEntry*
scope_push_decl(Scope* scope, Arena* storage, NameDecl* decl, enum NameSpace ns)
{
  HashmapKey hkey;
  HashmapEntry* he;
  NameEntry* name_entry;

  hkey.str_key = decl->strname;
  he = hashmap_lookup_entry(&scope->name_table, &hkey, HKEY_STRING);
  if (he) {
    name_entry = (NameEntry*)he->value;
  } else {
    name_entry = arena_malloc(storage, sizeof(NameEntry));
    hashmap_insert_entry(&scope->name_table, storage, &hkey, HKEY_STRING, (uint64_t)name_entry);
  }
  decl->next_in_scope = name_entry->ns[ns];
  name_entry->ns[ns] = decl;
  return name_entry;
}

void
Debug_scope_decls(Scope* scope)
{
  int count = 0;
  HashmapCursor it = {};
  HashmapEntry* he;
  NameEntry* name_entry;
  NameDecl* decl;

  hashmap_cursor_begin(&it);
  printf("Names in scope 0x%x\n\n", scope);
  he = hashmap_cursor_next_entry(&it, &scope->name_table);
  while (he) {
    name_entry = (NameEntry*)he->value;
    for (int i = 1; i < NameSpace_COUNT; i++) {
      decl = name_entry->ns[i];
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
    he = hashmap_cursor_next_entry(&it, &scope->name_table);
  }
  printf("\nTotal names: %d\n", count);
}


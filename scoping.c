#include <stdio.h>
#include <stdint.h>
#include <math.h>  /* floor, ceil, log2 */
#include "foundation.h"
#include "frontend.h"

static NameEntry null_entry = {};

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
scope_lookup(Scope* scope, char* strname)
{
  NameEntry* name_entry = 0;
  HashmapEntry* he;

  while (scope) {
    he = hashmap_lookup_entry(&scope->name_table, strname);
    if (he) {
      name_entry = (NameEntry*)he->value;
      if (name_entry->ns[NAMESPACE_TYPE] || name_entry->ns[NAMESPACE_VAR] || name_entry->ns[NAMESPACE_KEYWORD]) {
        break;
      }
    }
    name_entry = 0;
    scope = scope->parent_scope;
  }
  if (name_entry) {
    return name_entry;
  }
  return &null_entry;
}

NameEntry*
scope_lookup_in_namespace(Scope* scope, char* strname, enum NameSpace ns)
{
  assert(NAMESPACE_NONE < ns && ns < NameSpace_COUNT);
  NameEntry* name_entry = 0;
  HashmapEntry* he;

  while (scope) {
    he = hashmap_lookup_entry(&scope->name_table, strname);
    if (he) {
      name_entry = (NameEntry*)he->value;
      if (name_entry->ns[ns]) {
        break;
      }
    }
    name_entry = 0;
    scope = scope->parent_scope;
  }
  if (name_entry) {
    return name_entry;
  }
  return &null_entry;
}

NameEntry*
scope_lookup_current(Scope* scope, char* strname)
{
  NameEntry* name_entry = 0;
  HashmapEntry* he;

  he = hashmap_lookup_entry(&scope->name_table, strname);
  if (he) {
    name_entry = (NameEntry*)he->value;
  }
  return name_entry;
}

NameDeclaration*
scope_bind(Scope* scope, Arena* storage, char*strname, enum NameSpace ns)
{
  assert(NAMESPACE_NONE < ns && ns < NameSpace_COUNT);
  NameDeclaration* name_decl;
  NameEntry* name_entry;
  HashmapEntry* he;

  name_decl = arena_malloc(storage, sizeof(NameDeclaration));
  name_decl->strname = strname;

  he = hashmap_lookup_or_insert_entry(&scope->name_table, storage, strname, 0);
  if (he->value == 0) {
    he->value = arena_malloc(storage, sizeof(NameEntry));
  }
  name_entry = (NameEntry*)he->value;
  name_decl->next_in_scope = name_entry->ns[ns];
  name_entry->ns[ns] = name_decl;
  return name_decl;
}

void
Debug_scope_decls(Scope* scope)
{
  NameEntry* name_entry;
  NameDeclaration* decl;
  int count = 0;
  HashmapCursor it = {};
  HashmapEntry* he;

  hashmap_cursor_begin(&it, &scope->name_table);
  printf("Names in scope 0x%x\n\n", scope);
  he = hashmap_cursor_next_entry(&it);
  while (he) {
    name_entry = (NameEntry*)he->value;
    for (int i = 1; i < NameSpace_COUNT; i++) {
      decl = name_entry->ns[i];
      while (decl) {
        if (i == NAMESPACE_KEYWORD) {
          printf("%s, ns[%d]\n", decl->strname, i);
        } else {
          Ast* ast = decl->ast;
          printf("%s  ...  at %d:%d, ns[%d]\n", decl->strname, ast->line_no, ast->column_no, i);
        }
        decl = decl->next_in_scope;
        count += 1;
      }
    }
    he = hashmap_cursor_next_entry(&it);
  }
  printf("\nTotal names: %d\n", count);
}


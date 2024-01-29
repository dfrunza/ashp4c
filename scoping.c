#include <stdio.h>
#include <stdint.h>
#include <math.h>  /* floor, ceil, log2 */
#include "foundation.h"
#include "frontend.h"

static NameEntry entry_none = {};

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
  HashmapEntry* e;

  while (scope) {
    e = hashmap_lookup_entry(&scope->name_table, strname);
    if (e) {
      name_entry = (NameEntry*)e->value;
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
  return &entry_none;
}

NameEntry*
scope_lookup_namespace(Scope* scope, char* strname, enum NameSpace ns)
{
  NameEntry* name_entry = 0;
  HashmapEntry* e;

  while (scope) {
    e = hashmap_lookup_entry(&scope->name_table, strname);
    if (e) {
      name_entry = (NameEntry*)e->value;
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
  return &entry_none;
}

NameEntry*
scope_lookup_local(Scope* scope, char* strname)
{
  NameEntry* name_entry = 0;
  HashmapEntry* e;

  e = hashmap_lookup_entry(&scope->name_table, strname);
  if (e) {
    name_entry = (NameEntry*)e->value;
  }
  return name_entry;
}

NameEntry*
scope_push_decl(Scope* scope, Arena* storage, NameDeclaration* decl, enum NameSpace ns)
{
  NameEntry* name_entry;
  HashmapEntry* e;

  e = hashmap_lookup_or_insert_entry(&scope->name_table, storage, decl->strname, 0);
  if (e->value == 0) {
    e->value = arena_malloc(storage, sizeof(NameEntry));
  }
  name_entry = (NameEntry*)e->value;
  decl->next_in_scope = name_entry->ns[ns];
  name_entry->ns[ns] = decl;
  return name_entry;
}

void
Debug_scope_decls(Scope* scope)
{
  NameEntry* name_entry;
  NameDeclaration* decl;
  int count = 0;
  HashmapCursor it = {};
  HashmapEntry* e;

  hashmap_cursor_begin(&it, &scope->name_table);
  printf("Names in scope 0x%x\n\n", scope);
  e = hashmap_cursor_next_entry(&it);
  while (e) {
    name_entry = (NameEntry*)e->value;
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
    e = hashmap_cursor_next_entry(&it);
  }
  printf("\nTotal names: %d\n", count);
}


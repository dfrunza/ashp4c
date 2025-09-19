#include <stdint.h>
#include <stdio.h>
#include "foundation.h"
#include "frontend.h"

static NameEntry NULL_ENTRY = {0};

Scope* scope_create(Arena* storage, int segment_count)
{
  assert(segment_count >= 1 && segment_count <= 16);
  Scope* scope;

  scope = arena_malloc(storage, sizeof(Scope) + sizeof(StrmapEntry**) * segment_count);
  strmap_init(storage, &scope->name_table, segment_count);
  return scope;
}

Scope* scope_push(Scope* scope, Scope* parent_scope)
{
  scope->scope_level = parent_scope->scope_level + 1;
  scope->parent_scope = parent_scope;
  return scope;
}

Scope* scope_pop(Scope* scope)
{
  return scope->parent_scope;
}

NameDeclaration* builtin_lookup(Scope* scope, char* strname, enum NameSpace ns)
{
  NameEntry* name_entry;
  assert (ns == NAMESPACE_VAR || ns == NAMESPACE_TYPE);

  name_entry = scope_lookup(scope, strname, ns);
  return name_entry->ns[ns >> 1];
}

NameEntry* scope_lookup(Scope* scope, char* strname, enum NameSpace ns)
{
  NameEntry* name_entry;

  while (scope) {
    name_entry = strmap_lookup(&scope->name_table, strname, 0, 0);
    if (name_entry) {
      if ((ns & NAMESPACE_VAR) != 0 && name_entry->ns[NAMESPACE_VAR >> 1]) break;
      if ((ns & NAMESPACE_TYPE) != 0 && name_entry->ns[NAMESPACE_TYPE >> 1]) break;
      if ((ns & NAMESPACE_KEYWORD) != 0 && name_entry->ns[NAMESPACE_KEYWORD >> 1]) break;
    }
    name_entry = 0;
    scope = scope->parent_scope;
  }
  if (name_entry) return name_entry;
  return &NULL_ENTRY;
}

NameEntry* scope_lookup_current(Scope* scope, char* strname)
{
  return strmap_lookup(&scope->name_table, strname, 0, 0);
}

NameDeclaration* scope_bind(Arena* storage, Scope* scope, char*strname, enum NameSpace ns)
{
  assert(0 < ns);
  NameDeclaration* name_decl;
  NameEntry* name_entry;
  StrmapEntry* he;

  name_decl = arena_malloc(storage, sizeof(NameDeclaration));
  name_decl->strname = strname;
  he = strmap_insert(&scope->name_table, strname, 0, 1);
  if (he->value == 0) {
    he->value = arena_malloc(storage, sizeof(NameEntry));
  }
  name_entry = (NameEntry*)he->value;
  name_decl->next_in_scope = name_entry->ns[ns >> 1];
  name_entry->ns[ns >> 1] = name_decl;
  return name_decl;
}

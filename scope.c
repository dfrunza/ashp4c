#include <stdint.h>
#include "foundation.h"
#include "frontend.h"

static NameEntry NULL_ENTRY = {0};

Scope* scope_create(Arena* storage, int segment_count)
{
  assert(segment_count >= 1 && segment_count <= 16);
  Scope* scope;

  scope = (Scope*)arena_malloc(storage, sizeof(Scope) + sizeof(StrmapEntry**) * segment_count);
  scope->name_table.init(storage, segment_count);
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

NameDeclaration* scope_builtin_lookup(Scope* scope, char* strname, enum NameSpace ns)
{
  NameEntry* name_entry;
  assert (ns == NameSpace::VAR || ns == NameSpace::TYPE);

  name_entry = scope_lookup(scope, strname, ns);
  return name_entry->ns[(int)ns >> 1];
}

NameEntry* scope_lookup(Scope* scope, char* strname, enum NameSpace ns)
{
  NameEntry* name_entry;

  while (scope) {
    name_entry = (NameEntry*)scope->name_table.lookup(strname, 0, 0);
    if (name_entry) {
      if (((int)ns & (int)NameSpace::VAR) != 0 && name_entry->ns[(int)NameSpace::VAR >> 1]) break;
      if (((int)ns & (int)NameSpace::TYPE) != 0 && name_entry->ns[(int)NameSpace::TYPE >> 1]) break;
      if (((int)ns & (int)NameSpace::KEYWORD) != 0 && name_entry->ns[(int)NameSpace::KEYWORD >> 1]) break;
    }
    name_entry = 0;
    scope = scope->parent_scope;
  }
  if (name_entry) return name_entry;
  return &NULL_ENTRY;
}

NameEntry* scope_lookup_current(Scope* scope, char* strname)
{
  return (NameEntry*)scope->name_table.lookup(strname, 0, 0);
}

NameDeclaration* scope_bind(Arena* storage, Scope* scope, char*strname, enum NameSpace ns)
{
  assert((int)ns > 0);
  NameDeclaration* name_decl;
  NameEntry* name_entry;
  StrmapEntry* he;

  name_decl = (NameDeclaration*)arena_malloc(storage, sizeof(NameDeclaration));
  name_decl->strname = strname;
  he = scope->name_table.insert(strname, 0, 1);
  if (he->value == 0) {
    he->value = arena_malloc(storage, sizeof(NameEntry));
  }
  name_entry = (NameEntry*)he->value;
  name_decl->next_in_scope = name_entry->ns[(int)ns >> 1];
  name_entry->ns[(int)ns >> 1] = name_decl;
  return name_decl;
}

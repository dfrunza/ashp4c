#include "foundation.h"
#include "frontend.h"

static NameEntry NULL_ENTRY = {};

Scope* Scope::create(Arena* storage, int segment_count)
{
  assert(segment_count >= 1 && segment_count <= 16);
  Scope* scope;

  scope = (Scope*)storage->malloc(sizeof(Scope) + sizeof(StrmapEntry**) * segment_count);
  scope->name_table.init(storage, segment_count);
  return scope;
}

Scope* Scope::push(Scope* parent_scope)
{
  this->scope_level = parent_scope->scope_level + 1;
  this->parent_scope = parent_scope;
  return this;
}

Scope* Scope::pop()
{
  return this->parent_scope;
}

NameEntry* Scope::lookup(char* strname, enum NameSpace ns)
{
  NameEntry* name_entry;
  Scope*  scope;

  scope = this;
  while (scope) {
    name_entry = (NameEntry*)scope->name_table.lookup(strname, 0, 0);
    if (name_entry) {
      if ((ns & NameSpace::VAR) != (NameSpace)0 && name_entry->ns[(int)NameSpace::VAR >> 1]) break;
      if ((ns & NameSpace::TYPE) != (NameSpace)0 && name_entry->ns[(int)NameSpace::TYPE >> 1]) break;
      if ((ns & NameSpace::KEYWORD) != (NameSpace)0 && name_entry->ns[(int)NameSpace::KEYWORD >> 1]) break;
    }
    name_entry = 0;
    scope = scope->parent_scope;
  }
  if (name_entry) return name_entry;
  return &NULL_ENTRY;
}

NameDeclaration* Scope::builtin_lookup(char* strname, enum NameSpace ns)
{
  NameEntry* name_entry;
  assert (ns == NameSpace::VAR || ns == NameSpace::TYPE);

  name_entry = this->lookup(strname, ns);
  return name_entry->ns[(int)ns >> 1];
}

NameDeclaration* Scope::bind(Arena* storage, char*strname, enum NameSpace ns)
{
  assert((int)ns > 0);
  NameDeclaration* name_decl;
  NameEntry* name_entry;
  StrmapEntry* he;

  name_decl = (NameDeclaration*)storage->malloc(sizeof(NameDeclaration));
  name_decl->strname = strname;
  he = this->name_table.insert(strname, 0, 1);
  if (he->value == 0) {
    he->value = storage->malloc(sizeof(NameEntry));
  }
  name_entry = (NameEntry*)he->value;
  name_decl->next_in_scope = name_entry->ns[(int)ns >> 1];
  name_entry->ns[(int)ns >> 1] = name_decl;
  return name_decl;
}

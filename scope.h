#pragma once
#include <namespace.h>
#include <strmap.h>

struct Scope {
  static NameEntry NULL_ENTRY;
  int scope_level;
  Scope* parent_scope;
  Strmap<NameEntry> name_table;

  static Scope* create(Arena* storage, int segment_count)
  {
    assert(segment_count >= 1 && segment_count <= 16);
    Scope* scope = storage->allocate<Scope>();
    storage->allocate<StrmapEntry<NameEntry>*>(segment_count);
    scope->name_table.init(storage, segment_count);
    return scope;
  }

  Scope* push(Scope* parent_scope)
  {
    scope_level = parent_scope->scope_level + 1;
    this->parent_scope = parent_scope;
    return this;
  }

  Scope* pop()
  {
    return parent_scope;
  }

  NameEntry* lookup(char* strname, enum NameSpace ns)
  {
    NameEntry* name_entry = 0;
    Scope*  scope = this;

    while (scope) {
      name_entry = scope->name_table.lookup(strname, 0, 0);
      if (name_entry) {
        if ((ns & NameSpace::Var) != (NameSpace)0 && name_entry->get_declarations(NameSpace::Var)) break;
        if ((ns & NameSpace::Type) != (NameSpace)0 && name_entry->get_declarations(NameSpace::Type)) break;
        if ((ns & NameSpace::Keyword) != (NameSpace)0 && name_entry->get_declarations(NameSpace::Keyword)) break;
      }
      name_entry = 0;
      scope = scope->parent_scope;
    }
    if (name_entry) return name_entry;
    return &NULL_ENTRY;
  }

  NameDeclaration* lookup_builtin(char* strname, enum NameSpace ns)
  {
    assert (ns == NameSpace::Var || ns == NameSpace::Type);
    NameEntry* name_entry = lookup(strname, ns);
    return name_entry->get_declarations(ns);
  }

  NameDeclaration* bind_name(Arena* storage, char* strname, enum NameSpace ns)
  {
    assert((int)ns > 0);

    NameDeclaration* name_decl = NameDeclaration::create(storage, strname);
    StrmapEntry<NameEntry>* he = name_table.insert(strname, (NameEntry*)0, 1);
    if (he->value == 0) {
      he->value = storage->allocate<NameEntry>();
    }
    NameEntry* name_entry = he->value;
    name_entry->new_declaration(name_decl, ns);
    return name_decl;
  }
};

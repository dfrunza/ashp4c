#pragma once
#include "namespace.h"

struct Scope {
  int scope_level;
  Scope* parent_scope;
  Strmap name_table;

  static Scope* create(Arena* storage, int segment_count);
  Scope* push(Scope* parent_scope);
  Scope* pop();
  NameEntry* lookup(char* name, enum NameSpace ns);
  NameDeclaration* builtin_lookup(char* strname, enum NameSpace ns);
  NameDeclaration* bind(Arena* storage, char* strname, enum NameSpace ns);
};

#pragma once
#include "namespace.h"
#include "strmap.h"

struct Scope {
  static NameEntry NULL_ENTRY;
  int scope_level;
  Scope* parent_scope;
  Strmap* name_table;

  static Scope* allocate(Arena* storage, int segment_count);
  void init();
  Scope* push(Scope* parent_scope);
  Scope* pop();
  NameEntry* lookup(char* strname, enum NameSpace ns);
  NameDeclaration* lookup_builtin(char* strname, enum NameSpace ns);
  NameDeclaration* bind_name(Arena* storage, char* strname, enum NameSpace ns);
};

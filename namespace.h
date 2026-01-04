#pragma once
#include "lexer.h"
#include "type.h"

enum class NameSpace : int {
  Var = 1 << 0,
  Type = 1 << 1,
  Keyword = 1 << 2,
};
#define NameSpace_COUNT 3
inline NameSpace operator | (NameSpace lhs, NameSpace rhs) {
  return (NameSpace)((int)lhs | (int)rhs);
}
inline NameSpace operator & (NameSpace lhs, NameSpace rhs) {
  return (NameSpace)((int)lhs & (int)rhs);
}
char* NameSpace_to_string(enum NameSpace ns);

struct NameDeclaration {
  char* strname;
  NameDeclaration* next_in_scope;
  Type* type;

  union {
    Ast* ast;
    enum TokenClass token_class;
  };

  static NameDeclaration* create(Arena* storage, char* strname);
};

struct NameEntry {
  NameDeclaration* declarations[NameSpace_COUNT];

  NameDeclaration* get_declarations(enum NameSpace ns);
  void new_declaration(NameDeclaration* name_decl, enum NameSpace ns);
};

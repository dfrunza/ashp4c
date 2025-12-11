#pragma once
#include <lexer.h>
#include <type.h>

struct NameEntry;
struct NameDeclaration;

enum class NameSpace : int {
  VAR = 1 << 0,
  TYPE = 1 << 1,
  KEYWORD = 1 << 2,
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
};

struct NameEntry {
  NameDeclaration* declarations[NameSpace_COUNT];

  NameDeclaration* get_declarations(enum NameSpace ns)
  {
    NameDeclaration* decls = this->declarations[(int)ns >> 1];
    return decls;
  }

  void new_declaration(NameDeclaration* name_decl, enum NameSpace ns)
  {
    name_decl->next_in_scope = this->declarations[(int)ns >> 1];
    this->declarations[(int)ns >> 1] = name_decl;
  }
};
